---
name: feedback-avoid-casting
description: "In new C++ gameplay code, prefer interfaces/tags/component-lookups over casting to concrete game classes; don't refactor existing casts out yet"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: bf986c9d-5fd6-4d8e-b888-497a3214ab0f
  modified: 2026-09-10T00:15:11.067Z
---

When writing new C++ for the Blueprint-to-C++ conversion, avoid `Cast<>` to a *concrete gameplay class* for behavior dispatch (e.g. "is this actor a specific enemy/interactable Blueprint class"). Prefer the project's existing interfaces (`ISOTMInteractable`, `ISOTMDamageReceiver`, etc. in `SOTMCore`), tags, or a plain component lookup (`FindComponentByClass`) instead. If avoiding a *specific existing* cast would require a large surrounding refactor, leave it as-is for now and note it — don't do the big refactor yet.

**Why:** User's own words: "avoid casting in cpp and use interfaces or low cost approaches if possible... note that if its a big change (to remove casting in cpp) avoid it for now we'll re factor later after shifting and testing everything in cpp." The priority order is: finish shifting everything to C++ and get it tested first, then circle back to decouple casts as a dedicated refactor pass.

**How to apply:** Casts to *engine* base classes required by the API itself (`Cast<APlayerController>`, `Cast<UEnhancedInputComponent>`, `Cast<APawn>`) are NOT what this rule targets — those are normal, unavoidable engine patterns and don't need justification. The rule targets casts used to identify *specific game content* (a particular enemy class, a particular interactable Blueprint) to decide behavior. When porting a new system, check first whether an interface/tag/component-based approach is a small, natural fit (e.g. this was trivial for the Creation/player port since neither needed any game-specific-class casts at all); if the only way to avoid an existing cast is a wide-reaching change touching multiple other classes, leave it and mention it as deferred-refactor work rather than doing it inline. See [[feedback-cpp-first]] and [[project-sotm-cpp-conversion]] for the surrounding conversion effort this applies to.
