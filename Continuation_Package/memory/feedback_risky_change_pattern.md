---
name: feedback-risky-change-pattern
description: "For risky changes to shipped/production Blueprint content, duplicate it, verify the change on the duplicate in a safe test level, then apply the identical change to the real asset and playtest again"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: bf986c9d-5fd6-4d8e-b888-497a3214ab0f
  modified: 2026-09-09T23:43:13.865Z
---

When a change touches Blueprint content that's already placed/shipped in real game levels (e.g. reparenting an enemy Blueprint used in 30+ placements), don't edit the real asset first. Instead: duplicate the asset (and any Blueprint it depends on, e.g. its AI controller), apply and verify the full change on the duplicate in an isolated test level, and only once verified via an actual PIE playtest, apply the identical change to the real asset and playtest again in its real placement context.

**Why:** User's own words when asked to reparent BP_AI in CH1 (30+ placements): "Path A, do it on a duplicate first, test it, then go for the real BT_AI and play test again" — in response to "I don't want to break things, list the steps then I'll decide." This is the user's explicit risk-management preference for irreversible-feeling changes to production content, distinct from [[feedback-delegated-authority]]'s general "don't ask permission" rule — the *pattern* is pre-approved, but it must still be followed every time, not skipped.

**How to apply:** For any Blueprint reparent/graph-surgery affecting content already placed in a real (non-test) level: (1) duplicate the target Blueprint(s) into a scratch location (this session used `/Game/SOTM/Tests/`), (2) place the duplicate in the test lab level (`L_ArchitectureLab`), (3) apply the full change and verify via an actual `StartPIE` session with concrete telemetry (actor transforms, log errors, physical interactions) — not just "it compiled", (4) only then apply the identical change to the real asset, (5) playtest again in the real asset's actual level (found via `AssetTools.get_referencers`, since it may differ from the test level), (6) clean up the test duplicates once the real port is verified. Used successfully for both the CruelDoll (BP_AI/BP_AI_Controller) and Creation/Freddy (Creation/BP_Creation_Controller) ports.
