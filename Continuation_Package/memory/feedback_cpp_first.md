---
name: feedback-cpp-first
description: User wants all gameplay logic in C++; Blueprints reduced to empty pass-through shells
metadata: 
  node_type: memory
  type: feedback
  originSessionId: bf986c9d-5fd6-4d8e-b888-497a3214ab0f
  modified: 2026-09-09T23:42:36.704Z
---

All gameplay logic belongs in C++. When porting a Blueprint that has a native C++ equivalent available, the Blueprint should end up as an empty pass-through shell: parent class reparented to the native class, asset references (meshes, animations, montages, behavior trees) assigned on the class defaults, and zero remaining event-graph logic.

**Why:** Stated repeatedly and directly by the user across the whole Blueprint-to-C++ conversion effort ("I want all the logic in cpp"), not just as a one-off preference for a single system.

**How to apply:** When a Blueprint's graph logic has a native C++ counterpart, delete the old graph nodes (using precise node-by-node deletion via `get_connected_subgraph`/`delete_node` after fully inspecting the chain, never a blind wipe) rather than leaving old and new logic coexisting. A dangling/dead custom event with no callers left behind on an unrelated Blueprint (e.g. the player character) is acceptable and does not need to be proactively cleaned up unless it's actually causing a compile error — see [[feedback-risky-change-pattern]] for how to roll changes out safely. See [[project-sotm-cpp-conversion]] for the overall project this rule governs.
