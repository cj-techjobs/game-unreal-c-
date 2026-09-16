# SOTM Unreal Game Architecture Handbook

**Architecture and implementation specification · Revision 1 · 9 September 2026**

Build a reusable foundation for a chapter-based horror game with exploration, pursuit, survival, abilities, objectives, cinematics, and optional surveillance gameplay. *The Secrets of the Mansion* Chapter 1 provides the worked example; the framework must remain usable when its story, characters, and tuning change.

This is a proposed architecture and implementation guide. The project-specific classes, tools, test rooms, and commands described in these documents must be implemented unless explicitly identified as existing. No gameplay code or Unreal assets were changed to produce this handbook.

## Read and use

1. [Foundation and code setup](01-Foundation-and-Code-Setup.md): decisions, module dependencies, framework classes, initialization, data, event contracts, persistence, and migration.
2. [Gameplay system contracts](02-Gameplay-System-Contracts.md): player, movement, abilities, AI, animation, interaction, economy, objectives, death, cinematics, UI, audio, world design, and horror encounter patterns.
3. [Authoring and team workflows](03-Authoring-and-Team-Workflows.md): templates, content recipes, art handoffs, editor tooling, diagnostics, and independent testing.
4. [Implementation and validation](04-Implementation-and-Validation.md): build order, milestone acceptance, source requirements, test matrix, packaging, and project decisions.

For a formatted, searchable version of the complete handbook, open [Architecture-Handbook.html](Architecture-Handbook.html). It includes a table of contents, diagrams, and print styling. An editable Word version is available as [Architecture-Handbook.docx](Architecture-Handbook.docx). Markdown is the maintained source; regenerate the reading versions after changes.

## Recommended foundation

- C++ owns lifecycle, state changes, transactions, validation, and supported extension points.
- Blueprints compose actors, implement approved behavior extensions, and present art and audio.
- Primary Data Assets define reusable content; Data Tables handle homogeneous bulk tuning and dialogue rows.
- Unreal's Character Movement, Gameplay Ability System, Behavior Trees, AI Perception, navigation, Enhanced Input, CommonUI, and Sequencer provide the main engine foundations.
- Runtime data has one owner. UI and animation observe snapshots and events; they do not maintain competing game state.
- Designer independence comes from validated templates, clear diagnostics, test scenarios, and complete recipes.

The practical target is **independent creation and testing of supported content**. A new rendering technique, movement mechanic, persistence rule, or AI capability still needs engineering. The foundation is ready when another team member can complete the authoring acceptance exercise without private instructions from its programmer.

## Project and source review

The root `SOTM1.uproject` declares Unreal 5.8; the installed `UE_5.8/Engine/Build/Build.version` reports **5.8.2**. The nested `SOTM/SOTM1.uproject` declares 5.6 and uses different startup/gameplay configuration. This handbook uses 5.8.2 as the proposed implementation baseline, pending selection of the authoritative project. Do not treat either project as disposable.

Reviewed: both project descriptors, selected root and nested configuration, content directory names, the previous cleanup report, selected installed engine headers/plugin descriptors, and these local documents:

- `Downloads/CHAPTER 1 — Voice Actor Lines.pdf`, 2 pages.
- `Downloads/The Lost Path_ Chapter 1.pdf`, 8 pages.
- The Chapter 1 overview supplied in the conversation.

The PDFs are narrative inputs, not finalized gameplay specifications. Their differences and architectural implications are recorded in Part 4. The review did not open Unreal Editor, inspect Blueprint graphs, compile code, or execute gameplay. Engine references are linked beside the relevant technical guidance; project-specific policies are recommendations, not engine defaults.

## Document maintenance

Each system owner maintains its contract, authoring recipe, validators, and regression scenarios together. Record changes to stable IDs, save schemas, input ownership, and reset policies as architecture decisions. A template that needs an undocumented manual fix is incomplete.

The export script is [build_handbook.py](build_handbook.py); its header documents the optional Python dependencies and regeneration command. Document-generation dependencies are not Unreal runtime dependencies.
