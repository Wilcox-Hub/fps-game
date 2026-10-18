"""IRON SUN's file-based project memory. Python standard library; gh for publication.

Read: search. Maintain: checkpoint, render, check. Publish: preview, then publish.
GitHub mutations use an isolated Git data commit; the checkout/index are untouched.
"""
from __future__ import annotations

import argparse
import base64
import datetime as dt
import hashlib
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
REPO = "Wilcox-Hub/fps-game"
BEGIN = "<!-- IRON-SUN-MEMORY:BEGIN -->"
END = "<!-- IRON-SUN-MEMORY:END -->"
POINTERS = {"AGENTS.md", "unreal/AGENTS.md", "docs/MEMORY.md"}
EXTRAS = {"tools/project_memory.py", "tools/tests/test_project_memory.py"}


def read(root, path):
    return json.loads((root / path).read_text(encoding="utf-8"))


def write(root, path, value):
    p = root / path
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def allowed(path):
    p = pathlib.PurePosixPath(path)
    return (not p.is_absolute() and ".." not in p.parts and "\\" not in path
            and (path in POINTERS or path in EXTRAS or
                 ((path.startswith("docs/memory/") or path.startswith("docs/design-decisions/"))
                  and p.suffix in {".md", ".json", ".jsonl"} and not p.name.startswith("."))))


def block(content):
    if content.count(BEGIN) != 1 or content.count(END) != 1:
        raise ValueError("Expected one complete project-memory instruction block")
    return content[content.index(BEGIN):content.index(END)+len(END)]


def merge_pointer(remote, local):
    replacement = block(local)
    if BEGIN in remote or END in remote:
        old = block(remote)
        return remote.replace(old, replacement)
    return (remote.rstrip() + "\n\n" if remote.strip() else "") + replacement + "\n"


def interviews(root):
    for p in sorted((root / "docs/design-decisions/interviews").glob("*.json")):
        yield p.relative_to(root).as_posix(), json.loads(p.read_text(encoding="utf-8"))


def check(root):
    errors = []
    current = read(root, "docs/design-decisions/current.json")
    records = current["decisions"]
    ids = [r["id"] for r in records]
    if len(ids) != len(set(ids)): errors.append("Duplicate decision IDs")
    by_id = {r["id"]: r for r in records}
    archive_ids = set()
    answered_ids = set()
    for path, archive in interviews(root):
        for entry in archive["entries"]:
            if entry["id"] in archive_ids: errors.append("Duplicate interview entry: " + entry["id"])
            archive_ids.add(entry["id"])
            if entry.get("answer"):answered_ids.add(entry["id"])
            if entry.get("answer_status") == "answered" and not entry.get("answer"):
                errors.append("Answered entry lacks its original answer: " + entry["id"])
            for field in ("question", "answer"):
                value, source = entry.get(field), entry.get(field + "_source")
                if value is not None and source and source.get("text_sha256") != hashlib.sha256(value.encode()).hexdigest():
                    errors.append("Provenance hash mismatch: " + entry["id"] + ":" + field)
    for item in records:
        if item["status"] not in {"approved", "provisional", "superseded", "open", "deferred"}:
            errors.append("Invalid decision status: " + item["id"])
        if item["status"] == "superseded" and not item.get("superseded_by"):
            errors.append("Superseded decision needs replacement: " + item["id"])
        for ref in item.get("supersedes", []) + ([item["superseded_by"]] if item.get("superseded_by") else []):
            if ref not in by_id: errors.append("Unknown decision reference: " + ref)
        if item.get("superseded_by") and item["id"] not in by_id.get(item["superseded_by"], {}).get("supersedes", []):
            errors.append("Nonreciprocal replacement: " + item["id"])
        for ref in item.get("sources", []):
            if ref not in archive_ids and ref not in current["sources"]: errors.append("Unknown evidence source: " + ref)
    # Replacement edges must remain acyclic.
    def visit(ident, trail):
        if ident in trail: raise ValueError("Replacement cycle at " + ident)
        for prior in by_id[ident].get("supersedes", []):
            if prior in by_id: visit(prior, trail | {ident})
    for ident in by_id:
        try: visit(ident, set())
        except ValueError as exc: errors.append(str(exc))
    graph = read(root, "docs/memory/graph.json")
    node_ids = [n["id"] for n in graph["nodes"]]
    if len(node_ids) != len(set(node_ids)): errors.append("Duplicate memory graph nodes")
    for node in graph["nodes"]:
        path = node.get("path", "").split("#", 1)[0]
        if path and not (root / path).is_file(): errors.append("Missing memory target: " + path)
    for edge in graph["edges"]:
        if edge["from"] not in node_ids or edge["to"] not in node_ids: errors.append("Dangling memory edge")
    for path in POINTERS:
        try: block((root / path).read_text(encoding="utf-8"))
        except (OSError, ValueError) as exc: errors.append(path + ": " + str(exc))
    for line in (root / "docs/memory/work-log.jsonl").read_text(encoding="utf-8").splitlines():
        try: validate_checkpoint(json.loads(line))
        except (ValueError, KeyError) as exc: errors.append("Invalid checkpoint: " + str(exc))
    conflict_path=root/"docs/memory/conflicts.json"
    if conflict_path.exists():
        conflicts=read(root,"docs/memory/conflicts.json")["conflicts"]
        if len({c["id"] for c in conflicts})!=len(conflicts):errors.append("Duplicate conflict IDs")
        for c in conflicts:
            if c["status"] not in {"unresolved","resolved"}:errors.append("Invalid conflict status: "+c["id"])
            if c.get("decision_id") and c["decision_id"] not in by_id:errors.append("Unknown conflict decision: "+c["id"])
            if c["status"]=="resolved":
                if not c.get("owner_confirmed") or not c.get("resolution") or c.get("resolution_entry_id") not in answered_ids:
                    errors.append("Conflict resolution needs archived owner confirmation: "+c["id"])
    return errors


def source_link(root, ident):
    for path, archive in interviews(root):
        if any(e["id"] == ident for e in archive["entries"]):
            return path[:-5] + ".md#" + ident
    return "docs/design-decisions/sources.md#" + ident


def render(root):
    data = read(root, "docs/design-decisions/current.json")
    topics = data["topics"]
    source_lines=["# Supporting owner statements and evidence", "", "Statements outside numbered interviews, approved-summary provenance and current evidence pointers. Local paths identify unpublished source material; they are not claims that the file is on GitHub.", ""]
    for ident,source in data["sources"].items():
        source_lines += [f'<a id="{ident}"></a>',f"## {ident}","",f"Kind: {source['kind']}",""]
        for key,value in source.items():
            if key=="kind":continue
            source_lines += [(f"[Repository evidence](../../{value})" if key=="path" else f"**{key}:** {value}"),""]
    (root/"docs/design-decisions/sources.md").write_text("\n".join(source_lines),encoding="utf-8")
    for topic, title in topics.items():
        lines = [f"# {title}", "", "Generated from [current.json](../current.json). Read current rules before historical interviews.", ""]
        for r in data["decisions"]:
            if r["topic"] != topic: continue
            lines += [f"## {r['id']} — {r['title']}", "", f"**{r['status'].upper()}** · Recorded {r['recorded_on']}", "", r["rule"], ""]
            if r.get("reason"): lines += ["Reason: " + r["reason"], ""]
            if r.get("superseded_by"): lines += ["Replaced by: **" + r["superseded_by"] + "**.", ""]
            if r.get("supersedes"): lines += ["Replaces: " + ", ".join(r["supersedes"]) + ".", ""]
            lines += ["Sources: " + ", ".join(f"[{s}](../../../{source_link(root,s)})" for s in r["sources"]) + ".", ""]
        out = root / f"docs/design-decisions/topics/{topic}.md"
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text("\n".join(lines), encoding="utf-8")
    coverage = []
    for path, archive in interviews(root):
        entries = archive["entries"]
        answered = sum(e["answer_status"] == "answered" for e in entries)
        coverage.append({"id": archive["id"], "path": path, "questions":len(entries), "answers": answered,
                         "missing_answer_ids": [e["id"] for e in entries if e["answer_status"] != "answered"]})
        lines = [f"# {archive['id']} — interview archive", "", archive["scope"], "", archive["completeness"], "",
                 "These are historical questions and answers. [Current rules](../README.md) resolve later corrections. Question text can contain unaccepted proposals.", ""]
        for e in entries:
            lines += [f"<a id=\"{e['id']}\"></a>", f"## {e['id']}", "", "### Question", "", e["question"], ""]
            if e["options"]: lines += ["Options shown: " + " | ".join(e["options"]), ""]
            lines += ["### Original answer", "", e.get("answer") or ("**Awaiting answer.**" if e.get("answer_status")=="pending" else "**Not recovered. No answer is inferred.**"), "",
                      "Source: saved game conversation, " + e["question_source"]["timestamp_utc"] + ".", ""]
            for previous in e.get("prior_question_versions", []):
                lines += ["<details><summary>Earlier wording before this answer</summary>", "",previous["question"], "",previous["status"],"", "</details>",""]
        (root / path).with_suffix(".md").write_text("\n".join(lines), encoding="utf-8")
    write(root,"docs/design-decisions/coverage.json", {
        "recovered_on":"2026-10-18", "question_count":sum(x["questions"] for x in coverage),
        "updated_on":dt.datetime.now(dt.timezone.utc).date().isoformat(),
        "answer_count":sum(x["answers"] for x in coverage), "interviews":coverage,
        "limits":"Historical backfill comes from two saved game-conversation segments; future rounds use incremental live capture. Not a complete transcript export. Pending/missing answers and deferred choices remain explicit; source dates and validation scopes are retained."})
    lines = ["# IRON SUN — Design Decisions", "", "[Project memory tree](../memory/README.md) · [Machine-readable rules](current.json) · [Recovery coverage](coverage.json)", "",
             "Current rules below are the design baseline. Historical interviews preserve what was asked and answered; later user corrections take precedence. Approved direction is separate from implemented, tested and published work.", "", "## Current direction", ""]
    lines += [f"- [{title}](topics/{topic}.md)" for topic,title in topics.items()]
    lines += ["", "## Original interview answers", "", "| Interview | Questions | Answers saved |", "| --- | ---: | ---: |"]
    lines += [f"| [{x['id']}](interviews/{x['id']}.md) | {x['questions']} | {x['answers']} |" for x in coverage]
    lines += ["", "## Source notes", "", "[Supporting owner statements and evidence](sources.md) preserve direct choices and explicit approvals outside numbered interviews, with original wording where recovered. Initial numerical balance choices are provisional. A named concept image without its original sheet is a visual-reference gap, not permission to invent its appearance.", "", "## Maintenance", "", "Follow [the memory workflow](../memory/WORKFLOW.md). Edit current.json and interview JSON, then run `python tools/project_memory.py render` and `python tools/project_memory.py check`. Generated Markdown is a readable view, not a separate source of truth.", ""]
    (root / "docs/design-decisions/README.md").write_text("\n".join(lines), encoding="utf-8")


def search(root, query, history=False, limit=6):
    terms = query.lower().split()
    hits = []
    current = read(root,"docs/design-decisions/current.json")
    for r in current["decisions"]:
        if not history and r["status"] == "superseded": continue
        body = " ".join([r["title"],r["rule"],r.get("reason", ""),r["topic"]]).lower()
        score = sum(len(re.findall(r"\b"+re.escape(t)+r"\b",body)) for t in terms)
        if score:
            hits.append((score + (20 if r["status"]=="approved" else 0),r["id"],r["status"],r["rule"],f"docs/design-decisions/topics/{r['topic']}.md"))
    for path in ["docs/memory/active-work.json", "docs/memory/open-issues.json"]:
        data = read(root,path)
        for item in data.get("issues",[data]):
            body=json.dumps(item,ensure_ascii=False)
            score=sum(len(re.findall(r"\b"+re.escape(t)+r"\b",body.lower())) for t in terms)
            if score:
                hits.append((40+score,item.get("id",item.get("task",path)),item.get("status",item.get("phase","work/evidence")),body,path))
    if history:
        for path,a in interviews(root):
            for e in a["entries"]:
                body=e["question"]+" "+(e.get("answer") or "")
                score=sum(body.lower().count(t) for t in terms)
                if score:hits.append((score,e["id"],"historical answer",e.get("answer") or "Not recovered",path))
    return sorted(hits,key=lambda h:(-h[0],h[1]))[:limit]


def validate_checkpoint(record):
    required = {"task", "phase", "outcome", "topics", "decisions", "evidence", "publication", "next_action"}
    missing = required - record.keys()
    if missing: raise ValueError("Checkpoint missing: " + ", ".join(sorted(missing)))
    if record["phase"] not in {"started", "checkpoint", "completed", "blocked"}: raise ValueError("Invalid task phase")
    for k in ("topics", "decisions", "evidence"):
        if not isinstance(record[k],list):raise ValueError(k+" must be a list")


def checkpoint(root, record):
    validate_checkpoint(record)
    record = dict(record)
    record["recorded_at"] = dt.datetime.now(dt.timezone.utc).isoformat()
    log = root / "docs/memory/work-log.jsonl"
    with log.open("a",encoding="utf-8") as f:f.write(json.dumps(record,ensure_ascii=False)+"\n")
    write(root,"docs/memory/active-work.json",record)


def pending_conflicts(root, decisions=None):
    path=root/"docs/memory/conflicts.json"
    if not path.exists():return []
    return [c for c in read(root,"docs/memory/conflicts.json")["conflicts"] if c["status"]=="unresolved"
            and (not decisions or not c.get("decision_id") or c["decision_id"] in decisions)]


def git_blob_sha(content):
    if isinstance(content,str):content=content.encode("utf-8")
    return hashlib.sha1(b"blob "+str(len(content)).encode()+b"\0"+content).hexdigest()


def capture_round(root, record, local_only=False):
    """Capture the exact round/reply, detect declared/structured conflicts, then sync.

    Semantic comparison is performed by the assistant against the relevant decisions;
    this helper requires that review and catches changed existing rules/answers itself.
    It never promotes a proposal into an approved rule.
    """
    if record.get("project")!="iron-sun":raise ValueError("Capture is scoped to IRON SUN")
    ident=record.get("interview_id","")
    if not re.fullmatch(r"\d{4}-\d{2}-\d{2}-[a-z0-9-]+",ident):raise ValueError("Use a dated, safe interview_id")
    entries=record.get("entries")
    if not isinstance(entries,list) or not entries:raise ValueError("Round needs entries")
    review=record.get("review",{})
    if review.get("status") not in {"pending","clear","conflict"}:raise ValueError("Review status is required")
    checked=review.get("checked_decisions")
    if not isinstance(checked,list):raise ValueError("Review must name the checked current decisions")
    current=read(root,"docs/design-decisions/current.json")
    by_id={d["id"]:d for d in current["decisions"]}
    if any(d not in by_id for d in checked):raise ValueError("Unknown checked decision")
    declarations=list(review.get("conflicts",[]))
    if review["status"]=="conflict" and not declarations:raise ValueError("Conflict review needs a reason and decision")
    for proposal in review.get("proposed_rules",[]):
        old=by_id.get(proposal.get("decision_id"))
        if old is None:raise ValueError("Unknown proposed-rule decision")
        if old["status"] in {"approved","provisional"} and old["rule"]!=proposal["rule"]:
            declarations.append({"decision_id":old["id"],"current_rule":old["rule"],"proposed_rule":proposal["rule"],"reason":"Proposed rule differs from the current rule; clarify whether and how to replace it."})
    path=f"docs/design-decisions/interviews/{ident}.json"
    archive=read(root,path) if (root/path).exists() else {
        "schema_version":1,"id":ident,"captured_on":dt.datetime.now(dt.timezone.utc).date().isoformat(),
        "source_kind":"live-interview-capture","scope":"IRON SUN design interview; exact questions, options and original answers.",
        "completeness":"Live incremental capture. Pending questions and unresolved conflicts are retained explicitly.","entries":[]}
    existing={e["id"]:e for e in archive["entries"]}
    normalized=[]
    seen=set()
    for item in entries:
        eid=item.get("id","")
        if not re.fullmatch(re.escape(ident)+r"-[a-z0-9-]+",eid) or eid in seen:raise ValueError("Each entry needs a unique interview-prefixed ID")
        seen.add(eid)
        if not isinstance(item.get("question"),str) or not item["question"].strip():raise ValueError("Original question is required")
        if not isinstance(item.get("options",[]),list):raise ValueError("Options must be an array")
        answer=item.get("answer")
        if answer is not None and (not isinstance(answer,str) or not answer.strip()):raise ValueError("Answer must be original nonempty text or null")
        if answer is not None and review["status"]=="pending":raise ValueError("Review answered rounds before capture")
        previous=existing.get(eid)
        if previous and (previous["question"]!=item["question"] or previous["options"]!=item.get("options",[])):
            raise ValueError("Preserve the original prompt; use a new entry ID for a different question or options")
        updated=dict(previous or {})
        def provenance(timestamp,value):
            parsed=dt.datetime.fromisoformat(timestamp.replace("Z","+00:00"))
            if parsed.tzinfo is None:raise ValueError("Source timestamps need a timezone")
            return {"kind":"live-game-conversation","timestamp_utc":parsed.astimezone(dt.timezone.utc).isoformat(),"text_sha256":hashlib.sha256(value.encode()).hexdigest()}
        updated.update(id=eid,question_number=item.get("question_number"),question=item["question"],options=item.get("options",[]))
        updated["question_source"]=previous["question_source"] if previous else provenance(item["question_timestamp_utc"],item["question"])
        if previous and previous.get("answer") is not None and answer is None:raise ValueError("An answered question cannot be reverted to pending")
        if previous and previous.get("answer") is not None and previous["answer"]!=answer:
            updated.setdefault("answer_revisions",[]).append({"answer":previous["answer"],"answer_source":previous["answer_source"]})
            declarations.append({"decision_id":checked[0] if checked else None,"reason":"An existing answer was revised; clarify and retain both versions."})
        updated.update(answer=answer,answer_status="answered" if answer is not None else "pending",
                       answer_source=provenance(item["answer_timestamp_utc"],answer) if answer is not None else None,
                       review={"status":review["status"],"checked_decisions":checked},capture="live")
        normalized.append(updated)
    for conflict in declarations:
        if conflict.get("decision_id") is not None and conflict["decision_id"] not in by_id:raise ValueError("Unknown conflicting decision")
        if not conflict.get("reason"):raise ValueError("Conflict needs a reason")
    if declarations:
        for item in normalized:item["review"].update(status="conflict",agent_status=review["status"])
    # Save before doing network work. A failed sync must not lose the user's answer.
    tracked=["docs/design-decisions/README.md","docs/design-decisions/coverage.json",path,path[:-5]+".md","docs/memory/graph.json","docs/memory/conflicts.json"]
    queue_path="docs/memory/.sync-state.local.json"
    queue=read(root,queue_path) if (root/queue_path).exists() else {"expected":{}}
    before={p:queue["expected"][p] if p in queue["expected"] else (git_blob_sha((root/p).read_text(encoding="utf-8")) if (root/p).exists() else None) for p in tracked}
    for item in normalized:
        if item["id"] in existing:archive["entries"][archive["entries"].index(existing[item["id"]])]=item
        else:archive["entries"].append(item)
    write(root,path,archive)
    conflicts=read(root,"docs/memory/conflicts.json") if (root/"docs/memory/conflicts.json").exists() else {"schema_version":1,"conflicts":[]}
    for declaration in declarations:
        identity={"interview":ident,"conflict":declaration,"answers":[(x["id"],x.get("answer"),len(x.get("answer_revisions",[]))) for x in normalized]}
        key=hashlib.sha256(json.dumps(identity,sort_keys=True,ensure_ascii=False).encode()).hexdigest()[:16]
        if not any(c["id"]==key for c in conflicts["conflicts"]):
            conflicts["conflicts"].append({"id":key,"status":"unresolved","interview_id":ident,"entry_ids":list(seen),**declaration,"action":"Activate grill-me; clarify and obtain explicit owner confirmation before changing the affected rule."})
    write(root,"docs/memory/conflicts.json",conflicts)
    queue["expected"].update(before)
    write(root,queue_path,queue)
    graph=read(root,"docs/memory/graph.json")
    if not any(n["id"]==ident for n in graph["nodes"]):
        graph["nodes"].append({"id":ident,"kind":"interview","path":path[:-5]+".md"})
        graph["edges"].append({"from":"direction","to":ident,"relation":"supported-by"})
        write(root,"docs/memory/graph.json",graph)
    render(root)
    errors=check(root)
    if errors:raise ValueError("Saved locally; fix archive validation before syncing: "+"; ".join(errors))
    pending=pending_conflicts(root)
    result={"saved":path,"conflicts":[c["id"] for c in pending],"next_action":"activate-grill-me" if pending else "continue-interview-or-approved-work","sync":"local-only"}
    if not local_only:
        try:
            base=gh(f"repos/{REPO}/git/ref/heads/main")["object"]["sha"]
            result["publication"]=publish(root,base,True,paths=tracked,expected=before)
            result["sync"]="verified" if publication_plan(root,result["publication"]["commit"],paths=tracked)==[] else "verification-pending"
            if result["sync"]=="verified":
                for path in tracked:queue["expected"].pop(path,None)
                write(root,queue_path,queue)
        except (OSError,ValueError,RuntimeError) as exc:
            result["sync"]="pending"
            result["sync_error"]=str(exc)
    return result


def gh(endpoint, data=None, method=None):
    args=["gh","api",endpoint]
    if method:args += ["--method",method]
    if data is not None:args += ["--input","-"]
    result=subprocess.run(args,input=json.dumps(data) if data is not None else None,
                          encoding="utf-8",capture_output=True,check=False)
    if result.returncode:raise RuntimeError("GitHub operation failed: " + endpoint + ". " + result.stderr[-1200:])
    return json.loads(result.stdout) if result.stdout.strip() else None


def publication_plan(root, base, paths=None, expected=None):
    ref=gh(f"repos/{REPO}/git/ref/heads/main")
    if ref["object"]["sha"]!=base:raise ValueError("Remote main changed; reread it before publication")
    tree=gh(f"repos/{REPO}/git/trees/{base}?recursive=1")
    if tree.get("truncated"):raise ValueError("Incomplete remote tree; publication stopped")
    remote={x["path"]:x for x in tree["tree"] if x["type"]=="blob"}
    files=sorted(set(paths) if paths is not None else ({p.relative_to(root).as_posix() for folder in ["docs/memory","docs/design-decisions"] for p in (root/folder).rglob("*") if p.is_file() and allowed(p.relative_to(root).as_posix())} | POINTERS | EXTRAS))
    updates=[]
    for path in files:
        if not allowed(path):raise ValueError("Out of memory scope: "+path)
        content=(root/path).read_text(encoding="utf-8")
        if path in POINTERS:
            prior=""
            if path in remote:
                blob=gh(f"repos/{REPO}/git/blobs/{remote[path]['sha']}")
                prior=base64.b64decode(blob["content"]).decode("utf-8")
            content=merge_pointer(prior,content)
        encoded=content.encode("utf-8")
        blob_sha=hashlib.sha1(b"blob "+str(len(encoded)).encode()+b"\0"+encoded).hexdigest()
        if path not in remote or remote[path]["sha"]!=blob_sha:
            if expected is not None and path in expected and remote.get(path,{}).get("sha")!=expected[path]:
                raise ValueError("Remote memory differs from the pre-capture version; reconcile before syncing: "+path)
            updates.append({"path":path,"mode":"100644","type":"blob","content":content})
    return updates


def publish(root, base, apply=False, paths=None, expected=None):
    errors=check(root)
    if errors:raise ValueError("Memory validation failed: "+"; ".join(errors))
    updates=publication_plan(root,base,paths,expected)
    if not apply:return {"base":base,"paths":[x["path"] for x in updates],"mutation":False}
    if not updates:return {"commit":base,"paths":[],"changed":False}
    parent=gh(f"repos/{REPO}/git/commits/{base}")
    tree=gh(f"repos/{REPO}/git/trees",{"base_tree":parent["tree"]["sha"],"tree":updates},"POST")
    commit=gh(f"repos/{REPO}/git/commits",{"message":"Document project memory tree and approved design decisions","tree":tree["sha"],"parents":[base]},"POST")
    # force:false rejects races without touching the local dirty checkout or index.
    gh(f"repos/{REPO}/git/refs/heads/main",{"sha":commit["sha"],"force":False},"PATCH")
    return {"commit":commit["sha"],"paths":[x["path"] for x in updates],"changed":True}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=pathlib.Path,default=ROOT)
    sub=parser.add_subparsers(dest="command",required=True)
    sub.add_parser("check");sub.add_parser("render")
    s=sub.add_parser("search");s.add_argument("query");s.add_argument("--history",action="store_true");s.add_argument("--limit",type=int,default=6)
    c=sub.add_parser("checkpoint");c.add_argument("--record",required=True,type=pathlib.Path)
    c=sub.add_parser("capture-round");c.add_argument("--record",required=True,type=pathlib.Path);c.add_argument("--local-only",action="store_true")
    c=sub.add_parser("gate");c.add_argument("--decisions",nargs="*")
    for name in ("preview","publish"):
        p=sub.add_parser(name);p.add_argument("--base",required=True,help="Freshly read remote main commit SHA")
    args=parser.parse_args();root=args.root.resolve()
    if args.command=="render":render(root);print("Rendered current rules, interview pages and coverage.")
    elif args.command=="check":
        errors=check(root)
        if errors:print("\n".join(errors));return 1
        coverage=read(root,"docs/design-decisions/coverage.json")
        print(f"PASS: memory graph, decisions, source links, original-answer hashes and checkpoints. {coverage['answer_count']}/{coverage['question_count']} answers saved; gaps remain explicit.")
    elif args.command=="search":
        for _,ident,status,value,path in search(root,args.query,args.history,args.limit):print(f"[{status}] {ident}\n{value[:2200]}\nSource: {path}\n")
    elif args.command=="checkpoint":checkpoint(root,json.loads(args.record.read_text(encoding="utf-8")));print("Task checkpoint saved.")
    elif args.command=="capture-round":print(json.dumps(capture_round(root,json.loads(args.record.read_text(encoding="utf-8")),args.local_only),indent=2))
    elif args.command=="gate":
        conflicts=pending_conflicts(root,args.decisions)
        print(json.dumps({"allowed":not conflicts,"next_action":"activate-grill-me" if conflicts else "continue","conflicts":conflicts},indent=2))
        if conflicts:return 2
    else:print(json.dumps(publish(root,args.base,args.command=="publish"),indent=2))
    return 0


if __name__=="__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    try:sys.exit(main())
    except (OSError,ValueError,RuntimeError,KeyError) as exc:print(str(exc),file=sys.stderr);sys.exit(1)
