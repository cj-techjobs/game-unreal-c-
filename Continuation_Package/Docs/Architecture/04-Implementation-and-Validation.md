# Part 4 — Implementation, Validation, and Chapter Mapping

[Handbook index](README.md) · [Foundation](01-Foundation-and-Code-Setup.md) · [Gameplay contracts](02-Gameplay-System-Contracts.md) · [Authoring](03-Authoring-and-Team-Workflows.md)

## 1. Build in dependency order

Do not use the client's production checklist as the programming dependency order. Build a complete small loop, prove its failure paths, then make it reusable and give it to content authors. Each milestone below produces something concrete to inspect and has an exit gate. No duration estimate is implied without inspecting the existing Blueprints and team capacity.

| Milestone | Work and deliverable | Exit gate |
| --- | --- | --- |
| M0 — Baseline and decisions | Authoritative project/engine manifest; recorded current behavior; baseline packaged build; known-bug list; migration branch | Another programmer can open/build the same project; old saves and unique content are preserved |
| M1 — Native foundation | Modules, framework classes, logging, native tags/IDs, typed results, project settings, lab map | Editor/game targets compile; frontend/lab enter and exit; no editor dependency in runtime |
| M2 — Lifecycle and player | Explicit readiness flow, controller/character/PlayerState ASC, input/camera policy, movement definition/state policy, snapshot HUD | Cold start, pause, pawn replacement, and map travel recover input/camera; multiple PIE sessions isolate state |
| M3 — Persistence and transactions | Run ledger, five lives, stable world IDs, save generations, migration fixture, checkpoint restoration, fake reward/purchase commands | Duplicate commands, crash-recovery fixtures, retained progression, lives, and blocked spawn tests pass |
| M4 — First complete gameplay loop | One real coin, chest/key/gate, station, upgrade, objective, save, death and retry in a small room | Collect → purchase → open → complete → die → retry → reload works without duplicate grants or lost state |
| M5 — Player powers and enemy | Speed, Lightning, one enemy with sight/hearing/chase/search/attack/stun, animation contract, cancellation | AI loses sight correctly; attacks respect geometry; all interruption/reset tests pass |
| M6 — Presentation integration | CommonUI screens, pause/settings, objective HUD, dialogue/subtitles, music states, one dynamic jumpscare and one skippable story cinematic | Keyboard/gamepad-only flow works; skip/failure/timeout restores input/camera and preserves outcomes |
| M7 — Authoring product | Factories/templates, validators, chapter inspector, scenario launcher, preview fixtures, recipes, actionable errors | Designer and artist independently complete the self-service exercise through enemy, objective, chest, upgrade, and cinematic recipes; boss-phase authoring is gated by M9 |
| M8 — Chapter content | Forest routes, cousin variants, marker reveals, Timmy guide, objectives, keys/gates, upgrade economy, cinematic chain | Exploration route through the boss entrance is reachable and affordable; every checkpoint can resume it |
| M9 — Boss and chapter end | Phase coordinator, approved win mechanic, summons, retry recipe, chapter completion, Silken Mother teaser | All phases can be tested directly; repeat/skip/restore cannot duplicate or lose victory |
| M10 — Demo and full profiles | Separate content/save profiles, demo boundary, cook dependencies, entry/exit UX | Both packaged profiles boot from clean settings; excluded content is absent and no required reference is missing |
| M11 — Polish and release | Accessibility, final art/audio, target-hardware profiling, regression, clean-machine install, save upgrade compatibility | Release matrix passes; build and known limitations are documented |

Keep M1–M6 inside laboratories until reliable. Establish definitions and validation alongside each new feature; M7 makes the scattered tools into a coherent team-facing workflow. Do not wait until the forest is finished to discover that retry or packaging does not work.

## 2. Concrete first implementation checklist

These instructions describe work to perform in the selected implementation checkout. They were not executed while creating the handbook.

1. Add native game/editor targets and the modules from Part 1. Build a clean Development Editor target with Unreal closed for structural reflected-type changes; use Live Coding for suitable implementation iteration after the clean baseline works.
2. Create `L_ArchitectureLab` and Blueprint children of the native GameInstance, ChapterGameMode, GameState, PlayerState, PlayerController, player character, and camera manager. Set the GameMode's framework class defaults, and verify no unintended map override selects an old mode.
3. Introduce GameInstance and input setting changes in the isolated migration branch/checkpoint. These are project-wide settings, not safely isolated merely by a test map. Provide an adapter for legacy menu entry points until the new frontend replaces them.
4. Configure CommonUI viewport and action data; choose the baseline standard UI action path and disable the bridge there, or document the verified bridge choice. Restart after changing its support setting. Confirm the old menu's required actions before migrating it.
5. Add native Gameplay Tags and centralized collision channels/profiles. Publish their meanings, owners, and allowed writers. Include Interaction, AI visibility, projectile, and cinematic staging collision tests.
6. Create native Data Asset instance factories with immutable content IDs, primary type registration, required bundle/cook validation, and a trivial `DA_Chapter_ArchitectureLab`.
7. Implement readiness services and a visible development overlay before adding complex gameplay. Deliberately fail one dependency to verify timeout and recovery UI.
8. Implement the progression ledger and SaveGame records using a test slot namespace. Add a coin, a fake upgrade, and an opened gate record to verify save/load and unique placement identity.
9. Add the player ASC/attribute initialization and an input-to-ability adapter. Verify rebuilding the avatar produces exactly one copy of each initial or purchased grant.
10. Build the thin M4 loop and its Functional Test. Only then connect migrated game content through supported templates and adapters.

Useful initial contracts to create, in order:

```text
FRunId / FWorldGeneration / FOperationId
FCommandResult / FProgressionSnapshot / FWorldStateRecord
USOTMGameInstance, UGameFlowSubsystem, UProgressionSubsystem, USaveSubsystem
ASOTMChapterGameMode, ASOTMGameStateBase, ASOTMPlayerState
ASOTMPlayerController, ASOTMPlayerCharacter, ASOTMPlayerCameraManager
UChapterDefinition, UChapterSubsystem, UWorldStateSubsystem
USOTMPersistenceComponent, ISOTMWorldStateParticipant
UPresentationSubsystem, UPlayerHUDPresenter
UMovementDefinition, USOTMMovementPolicyComponent
ISOTMInteractable, USOTMInteractionComponent
UUpgradeDefinition, UObjectiveDefinition, UObjectiveSubsystem
```

Names are design proposals. Avoid proliferating wrappers for simple engine types: for example, `FRunId` can be a small strongly typed wrapper around `FGuid` if it prevents mixing IDs; it does not require another service.

## 3. Migrate existing assets without breaking the project

The project already contains marketplace/sample systems. Inspect them in the Editor before deciding what to keep. Directory names such as `SkillTree`, `Skill_Tree`, `SuperPowers`, `CombatSystem`, and `MenuSystemPro` do not establish their compatibility or redundancy.

| Existing area | Migration approach | Proof before cutover |
| --- | --- | --- |
| MenuSystemPro menus | Keep useful art/layout and route commands through the new presentation/flow adapter | Start/Continue/Pause/Settings/Quit and controller navigation work in a package |
| Existing player and powers | Extract useful mesh/animation/VFX and approved behaviors; replace ownership gradually | Same input behavior, collision, action cancellation and saved grants |
| Existing AI | Compare behavior and references; adapt presentation and useful task logic to one controller contract | Perception/chase/attack and checkpoint reset pass in the enemy lab |
| Skill-tree assets | Reuse art/layout where suitable; import definitions/ranks into the progression owner | Price/prerequisite/UI values match and rapid input cannot duplicate purchase |
| Chest/key assets | Wrap supported interaction/persistence interfaces | Repeated interaction and consumed-key restore pass |
| Cinematics/audio | Replace fixed actor bindings and progression Event Track dependencies | Dynamic roles, skip, timeout, missing voice and travel cleanup |
| Old saves | Versioned read/import path or explicitly labeled legacy profile | Fixture loads/imports coherently; originals remain recoverable |

Migrate one ownership boundary at a time. Never run old and new wallet/death/objective mutation logic together. During a transition, a narrow adapter is the single route to the current owner. Search references in Unreal, fix redirectors after reviewed moves, compile affected Blueprints, then cook the affected profile. Do not bulk rename assets on disk.

## 4. What the supplied material contributes

The following mapping is based on direct reading of the two local PDFs and the user's overview. Page references identify narrative evidence; paraphrases avoid freezing dialogue into code. The documents remain in Downloads and are not duplicated into this repository.

| Input and location | Element identified | Architectural requirement | Treatment |
| --- | --- | --- | --- |
| Overview and feature list | Five lives, death screen, objectives, upgrades, coins, boss, pause/menu polish | Run/death flow, economy transactions, state-driven UI, encounter phases | Required supported systems |
| Voice lines pp. 1–2 | Timmy speaker identity, story/hint/tutorial lines | Stable cue/speaker IDs, subtitle fallback, cue priorities | Narrative data |
| Voice lines p. 2 | Speed described through fatigue | Separate movement stamina and ability policy | Conflicts with cooldown phrasing; configurable decision |
| Lost Path p. 1 | Mansion introduction and apparently empty powers | Initial ability/grant loadout and intro outcome | Avoid ability logic in the cinematic |
| Lost Path p. 2 | Isabel appearance, power theft, scripted defeat, forest transition | Scripted story outcome distinct from normal death; travel and camera control | Default scripted loss does not consume life |
| Lost Path pp. 2–3 | Timmy hologram | Speaker/guide interface independent of physical pawn | Hologram and physical presentation supported |
| Lost Path pp. 3–4 | Crawling/twitching/sprinting/laughing/hovering cousins and detection barks | Enemy definitions, animation families, perception, bark concurrency | Hover presentation does not automatically require flight AI |
| Lost Path p. 4 | Speed unlocked from fragments; cooldown | Upgrade grant, economy threshold, tutorial cue | Cost/timing configurable |
| Lost Path pp. 4–5 | More fragments revealed on minimap; hidden chest and key | Marker reveal policy, stable spawned/hidden world state, objective gating | Authorable predicates and world roles |
| Lost Path p. 5 | Lightning stuns rather than kills; cousins become angry | GAS stun, target policy, bounded encounter reaction | Core example; no omniscient global aggro |
| Lost Path p. 5 | Chest contains key; both powers needed later | Atomic chest rewards/access; required ability checks | Validate mandatory route and affordability |
| Lost Path pp. 6–7 | Main Isabel boss, summons, stun reactions, low-health line | Phase coordinator, spawn ownership, boss-progress presenter | Defeat mechanic must be chosen |
| Lost Path pp. 7–8 + overview | Spider Mother reveal, named Silken Mother by user | Chapter completion independent of teaser; small teaser bundle | Chapter 1 content only; no Chapter 2 gameplay now |
| Lost Path p. 8 | This draft focuses on Speed and Lightning | Limited Chapter 1 content scope | Framework supports future powers |
| Voice lines p. 2 | Discord role-level notes | No game-system requirement established | Exclude from runtime scope |

### 4.1 Decisions left configurable

These are missing or inconsistent product details. They do not block documentation or foundation work. Use the proposed defaults until a design owner approves alternatives, then record the decision before content lock.

| Decision | Recommended provisional policy | Why it matters |
| --- | --- | --- |
| Isabel / Isabella / Isabelle | Stable ID `Character.Isabel`; approved localized display name and recording name sheet | Renames must not invalidate saves or recordings |
| Coins / golden fragments | One currency ID with configurable presentation | Prevent duplicate wallets unless multiple currencies are intentional |
| Initial recovered power | Chapter initial grants or first tutorial outcome, chosen in data | Overview and script differ on recovery order |
| Speed fatigue/cooldown | Normal sprint may use stamina; Speed Boost uses timed effect/cooldown | UI/voice must accurately explain the selected mechanic |
| Scripted intro loss | Story transition without spending a life | Five-life difficulty should not depend on cinematic replay |
| Death progression | Retain linked ledger; reset player/encounter transients | Keeps upgrades while preventing coin farming |
| Game Over | Exhausted run; New Run or Main Menu | Continue cannot silently reset lives |
| Boss victory | Explicit approved phase mechanic; ward example in Part 2 is only a proposal | Stun-only Lightning and low health do not define a complete fight |
| Upgrade-station safety | Safe stations pause gameplay | Opening UI during a chase needs a deliberate threat policy |
| Demo boundary | A build-profile milestone chosen after the complete thin slice | Demo must remain completable without unavailable content |
| Next-chapter carryover | Future chapter entry imports approved recovered-power records | Chapter advancement must not discard the premise of recovering powers |
| Difficulty/accessibility | Separate gameplay difficulty profile from camera/audio comfort settings | Reduced flashes should not silently alter currency or enemy rules |

### 4.2 Future chapter seam

Distinguish **New Campaign**, **Replay/Restart Chapter**, and **Advance to Next Chapter**. New Campaign creates initial progression; replay can create an isolated chapter run; advancement imports approved recovered abilities and story facts into the next run through `FChapterCarryoverPolicy` (proposed type). Currency, temporary items, and lives carry only when explicitly specified.

Store completed chapter outcomes and the approved carryover record in a coherent completion transaction. A future Chapter 2 definition declares required input schema and allowed carried IDs. Chapter 1 only needs to produce that record and play the Silken Mother teaser. Do not implement Chapter 2 AI, levels, or a playable spider boss for this milestone.

## 5. Validation architecture

Use Unreal's Data Validation framework for asset/schema checks and project C++ validators for rules that CI must run reliably. Epic's DataValidation commandlet runs C++ rules by default; Blueprint/Python validation needs deliberate extension. Graph reachability, loaded-map actor checks, runtime navigation, and packaged dependency checks need additional project validation/test passes. A green asset validation result alone does not prove gameplay is valid. [Data Validation](https://dev.epicgames.com/documentation/unreal-engine/data-validation-in-unreal-engine).

| Validation layer | Examples | When |
| --- | --- | --- |
| Local field/type | Missing ID, invalid cost/radius, absent required asset | Edit/save; authoring form |
| Definition relationships | Duplicate IDs, prerequisite cycles, unavailable grants, illegal reset combinations | Validate definition/dependencies; CI |
| Loaded map/placement | Duplicate persistent IDs, missing role anchors, blocked checkpoint exit, bad collision | Chapter inspector; map validation command |
| Scenario behavior | Death duplication, path failure, purchase/retry, skip cleanup | Targeted Functional Tests and author playtests |
| Cooked/runtime | Missing soft dependencies, demo contamination, packaged input/audio/settings | Build-profile package tests |
| Human assessment | Chase readability, horror timing, animation/camera comfort, route clarity | Representative playtest and art review |

Validation output format is specified in Part 3. Make fixes navigable and safe; unsupported automatic fixes must remain explicit author actions. Gate shipping on errors. Assign warning owners and expiry/version justification for intentional exceptions so ignored warnings do not become permanent noise.

## 6. Risk-based test matrix

The tests below validate behavior and contracts, not implementation internals. Pure record/transaction tests should be quick. World interactions use Functional Tests. UI/focus, cinematic rendering, navigation, audio, and performance need real world/rendering execution; `-NullRHI` cannot certify them.

| ID | Scenario | Required result | Test level |
| --- | --- | --- | --- |
| FND-01 | Cold start frontend → New Game → chapter | One run, one pawn, correct initial state, input available only after ready | Packaged smoke |
| FND-02 | PIE start/stop twice; multiple PIE worlds | No state leakage, duplicate listeners, or wrong-world service access | Editor functional |
| FND-03 | Missing required chapter asset / streaming timeout | Stable error/retry/menu path; no infinite loading | Functional fault injection |
| FND-04 | Travel while asset load/AI task is pending | Old callback cannot mutate new world/run | Functional |
| MOV-01 | Sprint exhaustion, crouch under ceiling, fall/land | Correct actual stance/gait; no invalid capsule or stuck speed | Player lab |
| MOV-02 | Stun while Speed Boost active; expiry during pause | Owned modifiers clean up; documented game-time behavior | Player lab |
| GAS-01 | Respawn and travel with purchased powers | Exactly one valid grant per source; correct rebuilt rank | Functional |
| GAS-02 | Cancel cast before/after cost commitment | Documented refund/cooldown; no lingering projectile/input lock | Ability lab |
| AI-01 | Seen, then behind occluder; later hearing stimulus | Last-known search; no continuous hidden-position tracking | Enemy lab |
| AI-02 | Door closes / destination unreachable / navlink interrupted | Bounded recovery and no hot retry loop | Enemy lab |
| AI-03 | Two enemies hit/capture in one frame | At most one accepted death and one life spent | Functional |
| AI-04 | Stun cousin with nearby/far cousins | Only eligible bounded reaction; expiry/reset correct | Encounter lab |
| ANI-01 | Interrupt attack before/during/after hit window | No stranded hitbox, damage repeat, movement lease, or root motion | Animation lab |
| ANI-02 | Attack offscreen or under animation throttling | Required gameplay timing follows supported contract | Rendered stress lab |
| INT-01 | Interact then move away/destroy target/open modal | Cancelled or rejected; no stale target mutation | Interaction lab |
| ECO-01 | Duplicate overlap/click/request ID | One collection/purchase/result mutation | Transaction automation |
| ECO-02 | Insufficient funds / max rank / stale quote | Typed rejection, unchanged wallet and grants | Transaction automation |
| ECO-03 | Grant asset missing or travel during preparation | No debit/partial ownership; stale request rejected | Functional |
| ECO-04 | Buy, open chest, consume key, die, reload | Purchases/open gate persist; consumed sources do not return | Functional + saved fixture |
| ECO-05 | Spend currency after a collection objective | Wallet changes; historical collection remains correct | Automation |
| ECO-06 | Mandatory path with worst allowed optional spending | Still solvable or invalid content flagged | Graph analysis + playthrough |
| SAVE-01 | Write failure/corrupt newest generation | Last valid complete revision loads; failure visible | IO abstraction/fault test |
| SAVE-02 | Older autosave finishes after newer mutation queued | Durable revision truthful; no newer state overwritten | Save queue automation |
| SAVE-03 | Old schema/renamed ID/removed required content | Migration or explicit recovery; no silent progress wipe | Fixture automation |
| SAVE-04 | Retained record loads before/after streamed actor registers | Same final state; no replayed reward/event | Functional |
| SAVE-05 | Duplicate actor or repeated level instance IDs | Validator catches collision; existing identity not silently replaced | Editor validation |
| LIFE-01 | Five accepted deaths | 5→4→3→2→1→0; zero disables retry | Functional |
| LIFE-02 | Skip/fail jumpscare; repeat completion/Retry click | One decrement; stable death screen and one restore | Functional |
| LIFE-03 | Continue a save made during death / exhausted run | No second decrement; no free life reset | Saved fixture |
| OBJ-01 | Chest opened / upgrade owned before objective activates | Defined retroactive/current-state policy; no softlock | Objective lab |
| OBJ-02 | Duplicate/out-of-order objective events and reward callbacks | One outcome/reward; bounded evaluation | Automation |
| OBJ-03 | Boss attempt reset with persistent chapter objectives | Attempt counters reset; retained progress remains consistent | Functional |
| NAR-01 | Timmy absent, stuck, audio missing, hint interrupted | Objective remains possible; subtitles/marker fallback; no spam | Narrative lab |
| MAP-01 | Reveal hidden chest, collect markers, stream area/reload | Correct discovery and consumed marker state | Map lab |
| CIN-01 | Every cutscene completes, skips, times out, aborts on travel | Terminal policy correct; all owned restrictions released | Cinematic lab |
| CIN-02 | Missing attacker role / obstructed capture staging | Safe fallback; death flow remains correct | Cinematic lab |
| UI-01 | Pause → Settings → Back → Resume on gamepad only | Correct focus, pause state, cursor, no click-through | UI functional/manual |
| UI-02 | Long text, larger subtitles, remapped input, device change | Readable layout, updated glyphs, accessible actions | UI review |
| BOSS-01 | Retry each phase and interrupt a transition | No duplicate boss/summons/hazards; valid encounter entry | Boss lab |
| BOSS-02 | Simultaneous boss/player terminal outcomes | Documented precedence; one stable result | Functional |
| END-01 | Victory then skip/missing teaser; repeat completion | Chapter/carryover committed once; correct frontend | Functional + save fixture |
| PKG-01 | Demo/full cold startup from clean user data | Correct profile, reachable exit, valid save namespace | Packaged smoke |
| PKG-02 | Demo content audit and unavailable offer/marker | No excluded dependency needed; no dead-end objective | Cook audit + playthrough |
| PERF-01 | Dense forest chase → capture → retry repeatedly | Meets approved frame/memory/hitch budgets; no growth/leaks | Target hardware |
| REL-01 | Clean checkout and clean-machine install | Reproducible build, necessary assets/config/prerequisites present | Release gate |

Unreal provides automation and level-oriented Functional Testing. Implement tests around the selected contracts and run them at the appropriate level. [Automation Test Framework](https://dev.epicgames.com/documentation/unreal-engine/automation-test-framework-in-unreal-engine?lang=en-US), [Functional Testing](https://dev.epicgames.com/documentation/unreal-engine/functional-testing-in-unreal-engine?lang=en-US).

### 6.1 Test scenarios and save isolation

Each scenario declares map, build profile, ledger fixture, checkpoint, encounter attempt, random seed, and expected result. Use slots under a visibly labeled test namespace and never write normal user saves. A test resets state between cases and cleans up even on failure; test order must not matter.

Expose approved developer commands through a scenario launcher: fresh run, spawn compatible enemy, set test currency, grant test ability, start phase, simulate death, restore checkpoint, and inject missing asset/save failure. These are proposed tools, not existing console commands. Compile/guard dangerous test mutations out of Shipping.

## 7. CI and packaging

### 7.1 Pipeline

1. Verify engine/plugin manifest, source availability, Git LFS objects, and required generated/imported inputs.
2. Build editor and game targets. Compile relevant Blueprints and run native data validators; load/validate production maps through a project-specific map pass.
3. Run quick transaction/schema/identity tests, then affected Functional Tests.
4. Cook/package the selected build profile with explicit entry maps and primary asset inclusion rules.
5. Audit cooked dependency/catalog output, startup map, localization, audio, required sequence roles/content, and exclusion of test/editor assets.
6. Run packaged startup/Continue/pause/death/exit scenarios with clean and migration-fixture user data.
7. Archive executable/content, symbols as appropriate for debugging, validation/test reports, engine/build manifest, and change list. Distribution packages and internal diagnostic artifacts can differ.

Do not rely on Editor success to establish packaged availability of soft-referenced assets. A Development package is useful early; the release candidate must also pass in its intended Shipping configuration. Epic documents packaging as a separate build/cook/stage/package workflow and requires a valid game default map. [Packaging](https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project).

### 7.2 Illustrative Windows commands

These examples are for the future implementation checkout. Paths, targets, tests, and profiles must exist first. They were not run for this documentation task. Execute each separately; capture its exit code and logs.

```powershell
$engineRoot = 'C:\Program Files\Epic Games\UE_5.8'
$projectFile = 'C:\Path\To\AuthoritativeProject\SOTM1.uproject'

# After native targets/modules exist:
& "$engineRoot\Engine\Build\BatchFiles\Build.bat" SOTM1Editor Win64 Development "-Project=$projectFile" -WaitMutex

# Engine commandlet; project C++ validators must be implemented:
& "$engineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" $projectFile -run=DataValidation -unattended -nop4 -log

# Example registered test prefix; project tests named SOTM.* must exist:
& "$engineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" $projectFile '-ExecCmds=Automation RunTests SOTM.' '-TestExit=Automation Test Queue Empty' -unattended -nop4 -log

# Package using the selected, validated project/build-profile configuration:
& "$engineRoot\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun "-project=$projectFile" -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -iostore -archive '-archivedirectory=C:\BuildArtifacts\SOTM-Development'
```

Add CI timeouts and explicit failure-report parsing. Confirm automation invocation behavior against the pinned engine before adopting these as CI scripts; a process exiting is not proof every test passed. A `UBuildProfileDefinition` is project data: it does not automatically configure cooking. Implement profile-to-cook configuration/validation in tooling and pass the resulting configuration to UAT or Project Launcher. [Running Automation Tests](https://dev.epicgames.com/documentation/unreal-engine/run-automation-tests-in-unreal-engine).

### 7.3 Demo policy

A demo is a profile of the same mechanics. Its definition selects available chapter content, initial grants, upgrade availability, objective roots, exit milestone, menu text, and save namespace. Do not duplicate the player/economy/AI implementations into a separate demo project.

If the demo stops before the boss, its objective graph ends cleanly and never instructs the player to obtain excluded powers or enter an absent arena. If it includes the teaser, label only its needed assets for the demo cook. UI hiding alone does not remove assets or prevent soft-reference failures.

Test clean install, repeat launch, Continue, exhausted run, settings persistence, demo completion, controller-only exit, unavailable full-save handling, and restart. Treat demo-to-full import as a feature with explicit fixtures rather than assuming matching save class names ensure compatibility.

## 8. Performance and polish acceptance

Record minimum/recommended hardware, target resolution/scalability, target frame rate, memory limit, acceptable cold-start/load time, and hitch thresholds before performance sign-off. These values are not available from the client material and should not be invented as final promises.

Measure a representative forest route, multi-cousin pursuit, lightning reaction, boss summons, capture, checkpoint retry, and menu transitions. Track game/render/GPU time, memory/high-water marks, texture/mesh/animation residency, audio concurrency, streaming stalls, and shader/PSO preparation. Use Unreal Insights and platform GPU tooling; compare packaged builds with fixed scenarios and seeds.

Polish acceptance includes readable enemy telegraphs, consistent collision, no camera penetration, comfortable optional camera effects, audible/non-audio guidance, subtitle timing, responsive pause/focus, clear upgrade errors, honest save status, and correct return to control after every cinematic. Art quality does not replace these behavioral checks.

## 9. Team ownership and handoff

| Role | Owns | Ready-to-hand-off evidence |
| --- | --- | --- |
| Architecture/programming owner | State/lifecycle contracts, migration policy, native extension interfaces | Contracts, validators, regression tests, example content |
| Gameplay designer | Definitions, tuning, objective/encounter graphs, checkpoint/economy layout | Local validation and assigned scenarios |
| Level designer | Placement/role bindings, routes, navigation, streaming boundaries, checkpoint safety | Map validation and representative route playthrough |
| Character/animation artist | Compatible meshes, layers/montages, sockets, timings, root-motion/camera requirements | Animation lab contract and interruption pass |
| UI artist/designer | Layout/styles, focus order, long-text/accessibility behavior | Presenter-fixture preview and device navigation pass |
| Audio/narrative | Cue IDs, localized subtitles, voice/music assets, mix/priority intent | Missing-audio and interruption fallback checks |
| QA/build owner | Scenarios, fixtures, regression/packaged matrices, reproducible bug reports | Reports tied to build/profile/seed/save revision |

Each new template includes purpose, permitted variations, forbidden mutations, required fields, example asset/map, reset/save behavior, troubleshooting, and owner. Add a change log for contract versions so an artist can see why a previously valid asset now needs an update.

### 9.1 Self-service acceptance exercise

Give a designer and artist this handbook plus the implemented templates. Without private engineering instructions, they must:

1. Create a visually distinct cousin using supported behavior and animation contracts.
2. Place a coin route, chest/key/gate, and an affordable upgrade offer.
3. Create an objective chain with a Timmy hint and a minimap reveal.
4. Bind a compatible jumpscare/intro cinematic and test skip/failure.
5. Validate content, fix deliberately seeded errors, run a checkpoint retry, and submit a useful bug report for a real unsupported condition.

The boss-phase step is completed after M9, when the phase coordinator and boss laboratory exist:

6. Create a phase variant using supported boss mechanics and launch directly into it.

Passing the first five steps proves the authoring foundation is usable before boss implementation; passing the final step proves the boss authoring extension is usable.

Record every point where they require undocumented help. Fix the template, validator, message, or recipe; declare new-mechanic requests as engineering extensions. Passing this exercise is a stronger definition of "plug and play" than having many Blueprint-exposed variables.

### 9.2 Bug report template

```text
Build/engine/profile:
Map / scenario / seed:
Run ID / checkpoint / save revision:
Asset or actor persistent ID:
Expected behavior:
Actual behavior:
Smallest reproduction steps:
Frequency and first known affected version:
Relevant reason code / log / screenshot or video:
Does fresh run, checkpoint retry, or packaged build differ?
```

No framework removes all future engineering work. This architecture makes supported content predictable and makes failures diagnosable, so routine iteration can belong to the team rather than to the original programmer's memory.

## 10. Documentation verification boundary

This handbook was checked for internal links, document structure, consistency of key ownership/reset rules, and source coverage. Engine-specific setup was cross-checked with Epic documentation and selected installed 5.8.2 headers/plugin descriptors. The proposed game classes, validators, editor tools, and scenarios have not been compiled or executed. Their acceptance gates describe the evidence the implementation must produce.

Markdown is the maintained source. HTML and Word are generated reading/editing artifacts; source changes should be applied to Markdown and re-exported to avoid divergent specifications.
