# Project cleanup review

Reviewed 2026-09-08. Sizes below were measured before cleanup and represent local
file sizes, not Git history or Git LFS storage savings.

## Completed cleanup

At the user's request, deleted these seven folders after checking that Unreal
and build processes were closed:

```text
MetaHumans/DerivedDataCache/
MetaHumans/Intermediate/
SOTM/DerivedDataCache/
SOTM/Intermediate/
SOTM/Binaries/
MetaHumans/Saved/
SOTM/Saved/
```

Before deletion, archived both complete `Saved/` folders under
`.cleanup-backups/20260908-155456/` as `MetaHumans-Saved.zip` and `SOTM-Saved.zip`.
Verified all 968 saved files against the archives using SHA-256. To restore,
extract the respective archive into that project's `Saved/` folder.

Removed 3,829,294,470 bytes; retained 111,289,469 bytes of recovery archives,
for approximately **3.46 GiB net local file-size savings**. The backup directory
is ignored by Git. Confirmed all seven target folders are absent. The `SOTM/`
project and its content remain intact.

## Original cleanup candidates

Close Unreal Editor and any build processes before cleaning generated data.

| Path | Approximate size | Condition |
| --- | ---: | --- |
| `MetaHumans/DerivedDataCache/` | 2,032 MiB | Regenerable cache; subsequent loading/shader processing can take longer. |
| `SOTM/DerivedDataCache/` | 52 MiB | Regenerable cache. |
| `SOTM/Intermediate/` | 500 MiB | Regenerable build/editor intermediates. |
| `MetaHumans/Intermediate/` | 76 MiB | Regenerable editor intermediates. |
| `SOTM/Binaries/` | 648 MiB | Packaged/compiled game output; remove if you do not need this existing executable. Rebuild/package to replace it. |
| `SOTM/Saved/` | 342 MiB | Review first: includes about 117 MiB of autosaves, local settings and save-game data as well as disposable logs/caches. Preserve any recovery/save files you need. |
| `MetaHumans/Saved/` | 2.3 MiB | Same review for autosaves, local settings and save-game data. |
| `MetaHumans/Binaries/Win64/UnrealEditor-MetaHumanSample.pdb` | 7.1 MiB | Debug symbols; remove if you do not need to debug this precompiled module. |
| `SOTM/Build/Windows/FileOpenOrder/EditorOpenOrder.log` | 0.28 MiB | Generated file-open-order log. |
| `Content/VaultCache/` | 0.16 MiB | Local Fab catalog/download metadata; only a manifest and catalog database are present here. |
| `Content/MenuSystemPro/ExampleContent/Designs/Design_Silence/Levels/desktop.ini` | Less than 1 KiB | Windows folder metadata. |

The four cache/intermediate directories alone total approximately **2.60 GiB**.
Most of this generated data was already ignored; deleting it frees local disk
space rather than reducing the tracked project.

## Larger candidates that need project/asset review

- **`SOTM/`** contains another `SOTM1.uproject` and approximately 5.60 GiB of
  content. Comparing tracked Git object IDs against the root project found
  2,606 identical files, 2,876 differing files at corresponding paths, and 1,029
  files present only under `SOTM/` (counts before index cleanup). Its startup map
  also differs from the root project's. Do not delete it as a duplicate: first
  decide which project is authoritative and migrate any unique work in Unreal.
- **`MetaHumans/`** is a separate `MetaHumanSample.uproject`, with approximately
  2.31 GiB of content. It is a candidate for removal only if you no longer need
  that sample and have migrated any assets you use. Keep the separate root
  project's MetaHuman content when reviewing this sample.
- **`AtrisLA-7745294f/metahuman/oa_atrisla.mhpkg`** is a roughly 209 MiB source
  package. Archive/remove it only if the import is complete and you do not need
  the original for later work.
- Large asset packs worth reviewing include `Content/Fab/` (2.10 GiB),
  `Content/Forest/` (1.93 GiB), `Content/Iceland_Environment/` (1.17 GiB), and
  `Content/LazyDevAac990ce745b2V1/` (1.08 GiB). Size and sample/demo names do not
  establish that an asset is unused. Inspect references and delete assets through
  Unreal's Content Browser, then validate your maps and a packaged build.

This review inspected the filesystem, project/config files and Git index. It did
not load Unreal's asset registry or validate Blueprint, soft-reference or runtime
dependencies, so no game content is certified unused.

## Keep these files

Keep `.uproject`, `Config/`, game content, source artwork and any future source or
plugin code. `__ExternalActors__/` and `__ExternalObjects__/` are level data.
Map `*_BuiltData.uasset` files are ignored at the user's request; local copies
remain available. Preserve `Build/` configuration, icons and packaging
rules; it is not exclusively generated output.

Do not delete all of `MetaHumans/Binaries/`: its module manifest references
`UnrealEditor-MetaHumanSample.dll`, and this checkout contains no source for that
module. At the user's request, all `Binaries/` and `Build/` folders are now
ignored, including this sample's DLL, manifest, receipt and build configuration.
Their local copies remain intact.

## Git changes made

Updated `.gitignore` to cover nested caches/intermediates/saved data, all
`Binaries/` and `Build/` folders, local Fab metadata, IDE state, debug symbols and
OS metadata. Removed extension-wide rules that hid source artwork and arbitrary
libraries outside these folders. Map build data remains ignored at the user's
request using `*_BuiltData.uasset` across all project folders.

Removed these eight files from Git's index **while keeping their local copies**:

```text
Content/MenuSystemPro/ExampleContent/Designs/Design_Silence/Levels/desktop.ini
Content/VaultCache/FabLibrary/Abandoned_Tunnel-e6b2165a/unreal-engine/manifest
Content/VaultCache/FabLibrary/listings_v1.db
MetaHumans/Binaries/Win64/UnrealEditor-MetaHumanSample.pdb
MetaHumans/Binaries/Win64/MetaHumanSampleEditor.target
MetaHumans/Binaries/Win64/UnrealEditor.modules
MetaHumans/Build/UnrealGameSync.ini
SOTM/Build/Windows/FileOpenOrder/EditorOpenOrder.log
```

These index removals are staged. The `.gitignore` edit and this report are not
staged. Commit them together with the removals to apply the cleanup for other
checkouts. No commit or push was performed; existing Git/LFS history is unchanged.

The **44 map build-data assets (542 MiB)** and **46 KiB MetaHuman DLL** remain
ignored. They already existed locally; this review did not create or stage them.
`.uasset` files already have a Git LFS rule in `.gitattributes`.

Validation: the initial 29 representative ignore/keep checks passed. Following
the user's changes, root and nested project/plugin `Binaries/` and `Build/`
paths were verified ignored, along with map build data. Git diff whitespace
checks passed. No Unreal build was run for this Git configuration change.

References: Epic's [directory structure documentation](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-directory-structure),
[derived data cache documentation](https://dev.epicgames.com/documentation/unreal-engine/using-derived-data-cache-in-unreal-engine),
and [One File Per Actor documentation](https://dev.epicgames.com/documentation/unreal-engine/one-file-per-actor-in-unreal-engine?lang=en-US).
