# Automatic IRON SUN interviews

The owner requests automatic capture and GitHub updates for every IRON SUN grill-me question and answer. This is part of active assistant work: save and sync without waiting for “remember this” or “save.” Other sessions must load this repository's guidance. No background service reads unrelated chats.

## Before asking

1. Read the relevant current decisions, pending conflicts and original supporting answers. Compare scope, dependencies, intended behavior and numerical values across related topics.
2. Use the grill-me/grilling skill's numbered frontier rounds with recommendations. Save the exact prompts and options as pending using `capture-round` before presenting them. Keep each question's stable ID and source time.

## Immediately after each reply

1. Save original answers and source times to the same question IDs. Preserve corrections and earlier answers. A “use suggested” answer must retain the exact proposal/options it accepted. Capture final plan confirmation as its own entry.
2. Perform a semantic conflict review against the relevant current rules and answers in this round. Include behavior, control bindings, costs, timing, ownership, modes, related systems and earlier pending answers. The helper checks structured rule/answer changes; it does not infer every natural-language contradiction itself.
3. Run `python tools/project_memory.py capture-round --record <round.json>`. It saves locally, renders the archive, validates it and automatically publishes only that round's memory files, then reads GitHub back. No extra save or publication approval is needed under the owner's standing request.
4. Inspect the result. `sync: verified` confirms publication. `sync: pending` keeps the answers and retry baseline locally; report the pending sync and reconcile any remote change before retrying. `--local-only` is for an unavailable connection or controlled tests, not routine deferral. Do this before another interview round, dependent implementation or ending the turn.

## Any unresolved conflict triggers grill-me

Record both competing statements and the affected decision in `docs/memory/conflicts.json`. Automatically apply grill-me and ask the current clarification frontier, with the competing rules and a recommendation. Pause implementation of affected decisions until the owner clarifies and explicitly confirms the agreed replacement. Independent work may continue.

`python tools/project_memory.py gate --decisions <decision-id> ...` blocks affected work while a matching conflict is unresolved. Without decision IDs it reports all unresolved conflicts. No rule is promoted by capturing an answer or a proposed replacement.

Save the clarification questions, answers and final confirmation too. Resolve the conflict with `status: resolved`, `owner_confirmed: true`, an explanation in `resolution` and `resolution_entry_id` pointing to the answered owner-confirmation entry. Then update current rules and reciprocal supersession links, render, check and publish through the normal memory workflow. Previously resolved supersessions remain history; reading them does not reopen the question. If an archived answer itself conflicts with another active rule, reopen a concrete conflict rather than guessing from timestamps.

## Round input

```json
{
  "project": "iron-sun",
  "interview_id": "2026-10-18-topic",
  "entries": [{
    "id": "2026-10-18-topic-q1",
    "question_number": 1,
    "question": "Exact question and recommendation as presented",
    "options": ["Exact option A", "Exact option B"],
    "question_timestamp_utc": "2026-10-18T09:00:00Z",
    "answer": null
  }],
  "review": {
    "status": "pending",
    "checked_decisions": ["existing-decision-id"],
    "conflicts": []
  }
}
```

After the real reply, supply its exact `answer` and `answer_timestamp_utc`; review status becomes `clear` or `conflict`. Use actual timestamps and IDs, not the placeholders above. A conflict includes `decision_id` and `reason`; `proposed_rules` may include `decision_id` and a proposed `rule`, which automatically raises a conflict if it differs from an active approved/provisional rule. An amended answer preserves its earlier version and also triggers clarification.

Keep secrets, credentials, private invites, authentication material and unrelated chat out of the public archive. Memory publication preserves the working checkout and does not deploy the game.
