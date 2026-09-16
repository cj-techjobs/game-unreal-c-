# Part 3 — Authoring and Team Workflows

This handbook specifies the proposed tools and content contracts for the architecture. These classes, templates, editor tools, validators, and test scenarios still need to be implemented; their names do not imply they already exist in the repository. The primary project target is Unreal Engine 5.8.2.

Read alongside [Foundation and Code Setup](01-Foundation-and-Code-Setup.md), [Gameplay System Contracts](02-Gameplay-System-Contracts.md), and [Implementation and Validation](04-Implementation-and-Validation.md).

## 1. The self-service contract

The target is that a designer or artist can create, validate, run, and diagnose a supported content recipe without modifying code. New mechanics, new persistence semantics, and changes to shared contracts require engineering. Document that boundary so a failed content setup is distinguishable from an unsupported feature.

Every shipped authoring template must include a working example, editable fields with useful descriptions, validation, a preview or test scenario, reset behavior, and a named owner. An undocumented template is unfinished infrastructure.

| Work | Owner and permitted extension |
| --- | --- |
| State, transactions, interruption, persistence | Native implementation; engineering owns its guarantees |
| Speeds, senses, prices, rewards, existing phase rules | Designers configure validated definition assets |
| Meshes, animation, VFX, materials, sound | Artists supply assets that satisfy documented compatibility contracts |
| Environment dressing and supported interactions | Level designers compose provided actors and definitions |
| Widget appearance and animation | UI authors use presentation snapshots and commands |
| A new kind of ability, objective predicate, or boss mechanic | Engineer adds a typed extension, validator, example, and test recipe |

Use C++ for invariants and reusable behavior; use Blueprints for supported composition and presentation. Avoid level Blueprint gameplay, arbitrary cross-level actor references, unchecked event names, and widgets that directly alter progression records.

## 2. Dependency and ownership boundaries

| Module | Authoring consequence |
| --- | --- |
| `SOTMCore` | Defines shared identifiers, definition schemas, contracts, typed results, and events; contains no chapter-specific assumptions |
| `SOTMGameplay` | Owns runtime state/services, interactions, progression, objectives, enemies, encounters, and their implementations |
| `SOTMPresentation` | Presents snapshots through UI, audio, camera, animation, and effects |
| `SOTMEditor` | Supplies inspectors, validators, placement tools, previews, and content-report generation; excluded from runtime builds |
| `SOTM1` | Bootstraps the game and installs the chosen implementation/configuration |

`UProgressionSubsystem` owns transactional wallet, inventory, upgrades, and persistent objective records. `UObjectiveSubsystem` evaluates conditions and submits progression commands. `UChapterSubsystem` owns chapter world lifecycle, while `UWorldStateSubsystem` registers placed state participants.

`UGameFlowSubsystem` owns death/load transitions. `UPresentationSubsystem` owns local UI/camera/input restriction leases. `UAudioNarrativeSubsystem` owns world dialogue and audio coordination. Authors request operations from the owner and react to delegates; they do not maintain shadow gameplay state.

## 3. Definitions, identities, and dependencies

Reusable definitions describe defaults and permitted configuration. Runtime objects hold current state. Never write the current wallet, objective count, alertness, health, or cooldown into a shared definition asset.

| Common definition field | Authoring rule |
| --- | --- |
| `ContentId` | Immutable `FName`, assigned when the definition is created; unique within its primary asset type |
| Primary asset identity | Explicit `FPrimaryAssetId` derived from asset type and persistent `ContentId`; do not derive persistent identity from the file name |
| Display text | Localizable text; never use displayed text as a gameplay identifier |
| Schema version | Engineering-controlled; changes that affect saved meaning need migration rules |
| Content ownership | Chapter/content-set association used for validation and package inclusion |
| Dependencies | Typed references to required definitions and soft references to presentation assets |
| Compatibility | Archetype, skeleton, supported actions, or other requirements relevant to the definition |

Asset renames and directory moves preserve `ContentId`. Duplicating a definition generates a new ID by default. Reusing an existing identity requires an explicit replacement workflow with validation and, where needed, migration.

Persistent placed actors receive project-owned cooked GUIDs. Moving or renaming the actor preserves its GUID; ordinary content duplication creates a new GUID while PIE duplication preserves it. Scope identities with stable chapter/map and, for repeated Level Instances, placement identity as defined in Part 1. Replacing a saved actor must preserve or migrate its identity deliberately.

| Definition | Required author-facing information |
| --- | --- |
| `UChapterDefinition` | Entry map/spawn role, objective and encounter roots, checkpoint policy, required content dependencies |
| `UEnemyDefinition` | Supported behavior archetype, movement, animation set, senses, attacks, interaction responses, kill presentation |
| `UMovementDefinition` | Permitted locomotion settings, acceleration/braking, stance parameters, ability movement limits |
| `UAnimationSetDefinition` | Compatible skeleton/archetype, locomotion assets, action slots, required markers/sockets, fallbacks |
| `UAbilityDefinition` | Native implementation, input role, costs/cooldowns, permitted targets, tuning, presentation |
| `UUpgradeDefinition` | Ability/tier granted, prerequisite IDs, purchase price, limits, availability |
| `UObjectiveDefinition` | Localized text, typed condition and parameters, prerequisites, activation/retirement, completion outcome |
| `UEncounterDefinition` | Participants, required placement roles, activation/completion/failure, reset policy, permitted phase rules |
| `UCinematicDefinition` | Sequence, role bindings, entry constraints, camera/input policy, skip, timeout, terminal outcome |
| `UDialogueSetDefinition` | Stable cue IDs, speakers, localized subtitles, voice references, priority, interruption, fallback durations |
| `UBuildProfileDefinition` | Included chapters/maps/content, demo boundary, supported entry, shipping feature policy |

Use Data Tables for uniform bulk data such as price curves, movement tuning tiers, or reward rows. Use definition assets for identity, structured configuration, compatibility, and dependencies. A table row is not automatically a safe save identifier; if its identity is persisted, it needs the same stability and migration discipline.

Do not build a general-purpose gameplay scripting language inside data. Provide bounded native objective predicates, encounter operations, and effects. A new behavior gets a typed implementation and authoring contract.

## 4. Content organization and editable surfaces

Proposed content layout:

```text
Content/SOTM/Core/                  # Shared definitions, tuning and defaults
Content/SOTM/Player/                # Player art, animation, input and powers
Content/SOTM/Enemies/Common/        # Shared enemy art and contracts
Content/SOTM/World/                 # Reusable placement and interaction templates
Content/SOTM/Chapters/CH01/         # Chapter-owned maps, definitions, sequences, dialogue
Content/SOTM/UI/                    # Widgets, styles, input display assets
Content/SOTM/Tests/                 # Test rooms, scenario definitions, deliberately invalid fixtures
```

Use `DA_` for definition assets, `BP_` for actor templates, `WBP_` for widgets, and `L_` for maps. These names aid navigation; persistent identity remains separate.

Prefer definition assets plus shallow Blueprint composition. Avoid long Blueprint inheritance chains. Expose instance overrides only when their purpose is clear; visibly label an override and show its inherited default. Keep transaction and persistence invariants outside instance overrides.

Every editable field needs units, a useful description, a valid range where applicable, and conditional visibility when irrelevant. Definition pickers should filter incompatible types. Required references should fail validation rather than disappear into runtime defaults.

The details panel should separate identity, definition, placement, supported overrides, and preview/debug controls. Provide a reset-to-definition action. Debug controls operate on transient test state and must not mutate production assets.

## 5. Placement tools and the chapter inspector

| Template | Visible placement helpers and checks |
| --- | --- |
| `BP_CoinPickup` | Collection radius, reward preview, unique persistent identity |
| `BP_Door` / `BP_LockedChest` | Interaction bounds, access requirement, opening clearance, retained state |
| `BP_UpgradeStation` | Interaction point, offered upgrades, focus/camera region, exit path |
| `BP_EnemySpawn` / `BP_PatrolRoute` | Spawn facing, route links, reachable points, compatible enemy definition |
| `BP_EncounterAnchor` | Arena bounds, participant roles, activation region, reset area |
| `BP_ObjectiveTrigger` | Condition volume, objective ID, permitted source, repeat-event behavior |
| `BP_Checkpoint` / `BP_PlayerSpawn` | Safe capsule placement, facing, chapter entry role, threat clearance |
| `BP_CinematicTrigger` | Trigger bounds, binding roles, camera preview, terminal destination |
| `BP_TeddyGuideAnchor` / `BP_AudioZone` | Speaker/hologram position, hint association, attenuation/zone bounds |

Bindings express semantic roles such as `Player.Entry`, `Isabel.AttackEntry`, `Teddy.HintPosition`, and `Camera.Jumpscare`. Avoid lookup by actor label. Each role declares whether it is required, whether multiple actors are allowed, and its compatibility requirements.

The proposed chapter inspector lists objectives, encounters, gates, checkpoints, cinematic bindings, and persistent actors. Selecting a row selects the asset or actor. It shows unresolved references, dependency paths, and retained/reset state classification.

Provide actions to validate the chapter, preview supported content, run a scenario, show dependencies, and apply a safe fix. Auto-fixes must show their affected assets; identity migrations and destructive replacements require deliberate editor actions rather than silent repair.

World state must be classified as `ProgressionRetained`, `CheckpointReset`, or `Transient`. The baseline retains the entire progression ledger across deaths: wallet, purchases, keys/inventory, consumed pickups, opened chests/gates, completed objectives, and story outcomes.

Checkpoints restore safe player/encounter entry and transient encounter state; they do not rewind lives or the ledger. Critical world puzzles use retained progression. Transaction bundles cannot mix arbitrary retention rules. A consumed key must remain consistent with its already-open gate after retry.

## 6. Recipe: add an enemy or cousin variant

1. Duplicate a supported enemy visual template and create `DA_Enemy_<Variant>` with a fresh identity.
2. Select an existing behavior archetype; reference its movement and animation definitions.
3. Assign compatible mesh, materials, action animations, sockets, sounds, and effects.
4. Configure supported sense, speed, attack, and ability-response values within the validated limits.
5. Place the enemy through an encounter/spawn template and bind patrol and encounter roles.
6. Run asset validation and the enemy laboratory; then test the actual encounter space.

Acceptance: detection, loss of sight, unreachable target, attack miss/hit, interruption, player death, and retry all terminate correctly. A reskin must pass gameplay and pose compatibility, not merely look correct in the viewport.

For a lightning response, select supported response behavior explicitly: a target may be stunned while another cousin becomes enraged. Do not encode this distinction in an asset name or duplicate the spell logic into each enemy Blueprint. A new response mechanic needs a typed engineering extension.

## 7. Recipe: add a chest, key, and gate chain

1. Place the chest and gate templates; assign their generated persistent instance IDs.
2. Select the chest reward and the gate's typed access requirement.
3. Choose the supported key-consumption policy and opening presentation.
4. Bind objective conditions to committed chest/access outcomes, not to an opening sound or animation.
5. Validate required references, collision clearance, dependency order, and retained world state.
6. Test locked interaction, obtaining the key, opening the gate, repeated interaction, death, reload, and new run.

Acceptance: one reward is granted; one key is consumed when required; an opened gate remains open after retry; a consumed key does not make the critical path impossible. A failed presentation cannot roll back or duplicate a successful transaction.

Chest opening, reward issuance, and associated consumed state form one supported transaction bundle. Content authors do not choose separate checkpoint behavior for its individual fields.

## 8. Recipe: add an objective, teddy hint, and map reveal

1. Create `DA_Objective_<Task>` using an existing typed condition; supply localized title/detail and explicit prerequisites.
2. Bind the source through a placed interaction, encounter, or world-state contract.
3. Add a dialogue cue in a `UDialogueSetDefinition`, with speaker, subtitle, voice reference, and fallback duration.
4. Place a teddy guide anchor and select the supported hint trigger, repetition, and interruption policies.
5. If needed, bind coin/area/chest marker reveals to a supported progression condition; the minimap reads committed discovery state.
6. Validate prerequisite reachability and dependencies; run the objective/dialogue laboratory and chapter path.

Acceptance: out-of-order and repeated events do not duplicate rewards; hints respect current objective state; missing voice still produces subtitles; completion during speech cancels or updates the hint according to policy. Revealed map regions remain consistent after death and reload.

Timmy as a physical companion, hologram, or stationary guide should be selected through supported presentation/guide templates. Objective authority remains the same. Avoid assuming a full autonomous follower is required merely because the story includes a companion.

## 9. Recipe: add an upgrade or supported boss phase

For an upgrade, create `DA_Upgrade_<AbilityTier>`, select an existing ability implementation, configure price/prerequisites, and add the offer to a station. Preview its icon, description, locked reason, purchased state, and controller navigation.

Acceptance: insufficient funds produce a useful reason; repeated purchase requests do not double-charge; a purchase updates wallet and ownership together; death retains the acquired power. Validate that mandatory upgrades are affordable under the declared chapter economy assumptions.

For a boss phase, select an existing phase pattern in `UEncounterDefinition`, configure transition conditions, attacks, arena roles, and summoned-enemy definitions, and bind the permitted phase presentation.

Acceptance: direct start-at-phase, summoned-enemy cleanup, phase interruption, player death, restart, and repeated defeat notifications behave correctly. Spawn caps and cleanup ownership are explicit. A novel boss mechanic requires engineering before the phase becomes a supported recipe.

## 10. Recipe: add a cinematic or jumpscare

1. Create `DA_Cinematic_<Moment>` and select the supported sequence/presentation template.
2. Declare required participant roles and bind compatible actors; preview placement and camera framing.
3. Configure permitted entry state, skip policy, timeout, and terminal outcome.
4. Use supported presentation ownership leases for camera and input restrictions.
5. Connect gameplay outcomes through the authoritative flow contract, including a fallback when presentation cannot start.
6. Test natural completion, skip, missing binding, interruption, timeout, death/load overlap, and replay after retry.

Acceptance: input/camera/audio ownership is released on every exit; the terminal result is handled once; lives are consumed by the death flow once. Required chapter outcomes must not depend solely on a sequence event marker.

The Silken Mother teaser uses this same contract. It can be authored as a chapter-completion presentation without implementing Chapter 2 gameplay. Missing optional teaser presentation must not prevent Chapter 1 completion from being recorded.

## 11. Animation handoff and compatibility

Each player and enemy archetype needs a versioned handoff sheet linked from `UAnimationSetDefinition`:

| Contract | Required detail |
| --- | --- |
| Skeleton/retargeting | Supported skeletons, expected scale and facing, accepted retarget workflow |
| Required bones/sockets | Names, intended attachments, transform conventions, missing-socket severity |
| Locomotion | Required states, stance transitions, movement limits, blend expectations |
| Action presentation | Required slots/layers, attack/reaction/death assets, interruption and blending |
| Motion ownership | Root motion policy per action and reconciliation with the movement owner |
| Timing | Required markers, supported hit windows, skipped-marker fallback, maximum action duration |
| Camera | First-person body rules, clipping limits, cinematic attachment and restoration |
| Fallback | Safe behavior for absent optional assets or failed presentation startup |

Animation notifies may request footsteps, sound, and effects. Damage, rewards, death, and final action completion require native validation and duplicate suppression. If a supported attack uses an animation-driven hit window, its interruption and timeout handling are part of the native contract.

Artist acceptance requires a neutral-pose/scale check, locomotion transitions, stairs/uneven ground, camera clipping, attack contact alignment, interruption, missing optional presentation, and low-frame-rate playback. Test the real mesh with its actual animation set and supported actions.

Do not approve a character using only a turntable render. Verify that cancelled jumpscares, interrupted attacks, and failed montages return control and leave no residual transform, collision, or animation state.

## 12. Voice, music, and VFX handoff

The dialogue handoff records stable cue ID, speaker ID, localized subtitle, language, voice asset, fallback duration, priority, and interruption classification. The project must publish recording/import, naming, loudness, and looping targets; do not invent these independently per chapter.

Dialogue completion may advance presentation, but it must not be the only mechanism capable of unlocking a required objective. Missing audio runs subtitles and the declared fallback timing. Subtitles must remain readable when the speaker is off-screen, when audio is muted, and when a line is interrupted.

Music content supplies supported exploration, suspicion, chase, boss, and resolution states. `UAudioNarrativeSubsystem` arbitrates requests and transitions. Authors configure transition timing, fade behavior, priority, pause policy, and missing-layer fallback; placed enemies do not independently start competing chase music.

VFX handoff declares attachment role, coordinate space, expected duration, pooling/reset behavior, visibility/accessibility alternatives, and cancellation. Cosmetic particles do not grant damage or rewards merely because they overlap an actor.

When audio or effects are missing, validation distinguishes optional cosmetic omissions from required contract failures. Production builds log the failure once and use a documented fallback rather than repeatedly spamming errors.

## 13. UI authoring without gameplay ownership

Widgets bind to typed UObject presenter snapshots and subscribe to change delegates. On the game thread, subscribe then read a complete snapshot and reconcile notifications by revision as specified in Part 1; an event that happened before widget creation must not leave the screen stale.

Presenters expose commands such as request purchase, select upgrade, retry, or resume. The owning gameplay service returns a typed result and reason. A widget never subtracts coins, consumes a life, unlocks an ability, or marks an objective complete itself.

Use the chosen CommonUI standard action-data routing for UI and Enhanced Input for gameplay with explicit ownership routing. Do not make the CommonUI/Enhanced Input experimental bridge a baseline dependency. MVVM remains an optional spike because its documented status is Beta; the typed snapshot approach works without it.

UI acceptance includes mouse/controller transitions, focus recovery, stacked dialogs, pause/resume, disabled command reasons, five-life display, objective updates, currency/purchase states, subtitles, long localized text, and safe-area/font scaling. Closing a screen releases its subscriptions and presentation ownership.

## 14. Actionable validation

Every validation result uses a consistent shape:

```text
Severity | Rule ID | Asset/Actor | Property | Problem | Consequence | Exact fix | Action
Error | CIN-004 | DA_Cinematic_IsabelKill | Bindings.Player
Required player role has no compatible binding.
The cinematic cannot start safely.
Assign the Player role from the supported Isabel Kill template.
Actions: Select actor / Open definition / Open recipe
```

Errors block content acceptance; warnings describe a real limitation that can be reviewed; advice suggests optional improvements. Provide stable rule IDs and documentation. A generic "invalid data" message is not sufficient.

| Rule family | Examples |
| --- | --- |
| Identity | Duplicate definition IDs, duplicate placed GUIDs, missing migration for changed identity |
| Definitions | Missing required reference, incompatible class/skeleton, unsupported parameter combination |
| Progression | Unreachable mandatory objective, prerequisite cycle, impossible required purchase |
| Persistence | Retention mismatch inside a transaction, consumed-key/open-gate contradiction |
| World | Unbound encounter role, unsafe spawn, invalid route point, unresolved persistent participant |
| Presentation | Missing required slot/socket, absent cinematic exit policy, voice without subtitle fallback |
| Packaging | Required soft-referenced asset omitted, demo dependency crossing, accidental test content inclusion |

Asset validation, map checks, scenario execution, and packaged integration each find different failures. A green asset validator does not establish navigation reachability or prove a complete objective chain is playable.

## 15. Test rooms and scenario launcher

The scenario launcher selects a map, spawn role, encounter state, initial progression, lives, deterministic seed where supported, and expected outcome. Use transient overrides and isolated test save slots; never rewrite production definitions or the user's normal run.

| Room | Required scenarios |
| --- | --- |
| Player laboratory | Movement transitions, input remap, ability interruption, camera recovery |
| Enemy laboratory | Sense boundaries, target loss, unreachable player, stun/enrage, chase, attack |
| Economy laboratory | Insufficient funds, repeated requests, reward duplication, purchase/reload |
| Interaction laboratory | Key/chest/gate chain, repeat interaction, death, reload, new run |
| Objective/dialogue laboratory | Out-of-order events, repeat events, missing voice, hint cancellation, map reveal |
| Cinematic laboratory | Binding failure, skip, timeout, interruption, camera/input release |
| Boss laboratory | Each phase, summons, transition interruption, death/restart, repeated defeat events |
| UI laboratory | Controller/mouse, focus, pause, subtitles, long text, scaling |

Provide start fresh, restore checkpoint, restart encounter, and enter a supported test state. Use real gameplay/reset commands where possible; clearly identify fixtures that directly construct a test state so they are not mistaken for full integration coverage.

Checkpoint retry restores safe entry, leaves the retained ledger and remaining lives intact, and resets supported cooldown state at safe spawn under the game policy. Do not permit checkpoint spam to bypass encounter or cooldown restrictions. New Run resets the chapter ledger and lives to five while preserving separate profile settings/chapter completions.

## 16. Acceptance and maintenance

A supported content item is ready when its local validation passes, its recipe scenario passes, interruption/reset/persistence behavior is verified, missing optional presentation has a usable fallback, localization/accessibility fields are complete, and its required assets work in a packaged chapter build.

Record the content owner, template version, changed dependencies, and evidence of relevant checks. A material change does not require rerunning the whole story; a progression identity or transaction change does require broader save/reload coverage.

Chapter acceptance additionally covers economy feasibility, every mandatory objective, all five lives, retry after consumed keys/purchases, boss resolution, demo boundaries, and durable save/load. A runtime transaction being committed does not mean it has reached disk; durable saves contain the coherent current ledger, checkpoint, and remaining lives together.

Before declaring the tools self-service, ask a designer unfamiliar with the implementation to complete an enemy variant, objective/hint chain, chest, upgrade offer, and cinematic binding using only this handbook and examples. Track every interruption. Fix unclear tools, messages, and documentation, or explicitly declare an unsupported extension.

Changes to a shared template require validation against its registered dependents and representative scenarios. Deprecate old contracts with migration notes and a replacement recipe; do not silently alter persistent identity or saved meaning. Keep a small catalog of known-good examples that ships with each supported template version.

Engineering remains responsible for engine upgrades, new behavior contracts, migration failures, and shared-system bugs. The architecture succeeds when routine content work stays inside documented, observable contracts and exceptions produce a precise issue report with reproduction steps.
