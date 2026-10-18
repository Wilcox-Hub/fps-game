"""Focused regressions for provenance, direction and memory-only publication."""
import base64
import importlib.util
import json
import pathlib
import shutil
import tempfile
import unittest
from unittest.mock import patch

REPO=pathlib.Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location("project_memory",REPO/"tools/project_memory.py")
memory=importlib.util.module_from_spec(spec)
spec.loader.exec_module(memory)


class MemoryTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(prefix="iron-sun-memory-test-")
        self.addCleanup(self.tmp.cleanup)
        self.root=pathlib.Path(self.tmp.name)
        for folder in ("docs/memory","docs/design-decisions"):
            shutil.copytree(REPO/folder,self.root/folder)
        for name in memory.POINTERS | memory.EXTRAS | {"docs/beta-readiness/latest.json"}:
            target=self.root/name;target.parent.mkdir(parents=True,exist_ok=True)
            shutil.copyfile(REPO/name,target)

    def test_actual_archive_validates(self):
        self.assertEqual(memory.check(self.root),[])

    def test_original_answer_change_is_detected(self):
        path=next((self.root/"docs/design-decisions/interviews").glob("*wildcard*.json"))
        data=json.loads(path.read_text(encoding="utf-8"));data["entries"][0]["answer"]="A fabricated replacement"
        path.write_text(json.dumps(data),encoding="utf-8")
        self.assertTrue(any("Provenance hash mismatch" in e for e in memory.check(self.root)))

    def test_duplicate_decision_is_rejected(self):
        path="docs/design-decisions/current.json";data=memory.read(self.root,path)
        data["decisions"].append(data["decisions"][0]);memory.write(self.root,path,data)
        self.assertIn("Duplicate decision IDs",memory.check(self.root))

    def test_unknown_source_is_rejected(self):
        path="docs/design-decisions/current.json";data=memory.read(self.root,path)
        data["decisions"][0]["sources"].append("invented-question");memory.write(self.root,path,data)
        self.assertIn("Unknown evidence source: invented-question",memory.check(self.root))

    def test_replacement_cycle_is_rejected(self):
        path="docs/design-decisions/current.json";data=memory.read(self.root,path)
        by_id={r["id"]:r for r in data["decisions"]}
        by_id["controls-lean-enabled"]["supersedes"]=["controls-lean-removed"]
        memory.write(self.root,path,data)
        self.assertTrue(any("Replacement cycle" in e for e in memory.check(self.root)))

    def test_default_search_returns_current_lean_removal(self):
        hits=memory.search(self.root,"lean")
        self.assertIn("controls-lean-removed",[h[1] for h in hits])
        self.assertNotIn("controls-lean-enabled",[h[1] for h in hits])

    def test_open_ammo_failure_is_retrieved_before_tuning(self):
        hits=memory.search(self.root,"ammo")
        self.assertEqual(hits[0][1],"host-ammo-natural")

    def test_publisher_rejects_source_and_traversal(self):
        for path in ("unreal/Ridgefire/Source/Shooter.cpp","../outside.md","/absolute.md","docs/memory/../../key.json","docs\\memory\\x.json","docs/memory/.secret.json","docs/memory/key.exe"):
            with self.subTest(path=path):self.assertFalse(memory.allowed(path))

    def test_pointer_merge_preserves_unrelated_guidance(self):
        remote="# Existing guidance\nUnrelated rule.\n"
        local="Unpublished local changes that must not be copied\n"+memory.BEGIN+"\nMemory only\n"+memory.END
        merged=memory.merge_pointer(remote,local)
        self.assertIn("Unrelated rule.",merged)
        self.assertNotIn("Unpublished local changes",merged)
        self.assertEqual(memory.merge_pointer(merged,local),merged)

    def test_checkpoint_retains_full_work_context(self):
        r={"task":"scenario","phase":"completed","outcome":"Controlled case only","topics":["combat-movement"],"decisions":["controls-e-interact"],"evidence":["dated scoped report"],"publication":{"game":"unchanged"},"next_action":"Human acceptance"}
        before=(self.root/"docs/memory/work-log.jsonl").read_text(encoding="utf-8").splitlines()
        memory.checkpoint(self.root,r)
        after=(self.root/"docs/memory/work-log.jsonl").read_text(encoding="utf-8").splitlines()
        self.assertEqual(len(after),len(before)+1)
        self.assertEqual(memory.read(self.root,"docs/memory/active-work.json")["evidence"],r["evidence"])
        with self.assertRaises(ValueError):memory.validate_checkpoint({"task":"Incomplete"})

    def test_remote_race_stops_before_mutation(self):
        with patch.object(memory,"gh",return_value={"object":{"sha":"new-remote-head"}}) as api:
            with self.assertRaisesRegex(ValueError,"Remote main changed"):memory.publication_plan(self.root,"old-head")
            self.assertEqual(api.call_count,1)

    def test_publication_plan_is_scoped_and_does_not_stage_checkout(self):
        def api(endpoint,data=None,method=None):
            self.assertIsNone(method)
            self.assertIsNone(data)
            if "git/ref/" in endpoint:return {"object":{"sha":"base"}}
            if "git/trees/" in endpoint:return {"truncated":False,"tree":[]}
            self.fail(endpoint)
        with patch.object(memory,"gh",side_effect=api):updates=memory.publication_plan(self.root,"base")
        self.assertTrue(updates)
        self.assertTrue(all(memory.allowed(x["path"]) for x in updates))
        self.assertFalse(any("Ridgefire/Source" in x["path"] for x in updates))
        self.assertTrue(all(x["content"].startswith(memory.BEGIN) for x in updates if x["path"] in memory.POINTERS))

    def test_publish_uses_tree_sha_and_nonforced_ref(self):
        calls=[]
        def api(endpoint,data=None,method=None):
            calls.append((endpoint,data,method))
            if endpoint.endswith("git/commits/base"):return {"tree":{"sha":"base-tree"}}
            if endpoint.endswith("git/trees"):return {"sha":"new-tree"}
            if endpoint.endswith("git/commits"):return {"sha":"new-commit"}
            if endpoint.endswith("git/refs/heads/main"):return {}
            self.fail(endpoint)
        updates=[{"path":"docs/memory/active-work.json","content":"{}","type":"blob","mode":"100644"}]
        with patch.object(memory,"check",return_value=[]),patch.object(memory,"publication_plan",return_value=updates),patch.object(memory,"gh",side_effect=api):
            result=memory.publish(self.root,"base",True)
        self.assertEqual(result["commit"],"new-commit")
        self.assertEqual(calls[1][1]["base_tree"],"base-tree")
        self.assertEqual(calls[2][1]["parents"],["base"])
        self.assertFalse(calls[3][1]["force"])


if __name__=="__main__":unittest.main()
