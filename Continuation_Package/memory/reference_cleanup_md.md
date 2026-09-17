---
name: reference-cleanup-md
description: CLEANUP.md at repo root has a prior disk-space/asset cleanup audit for this Unreal project — check it before re-deriving folder cleanup findings from scratch
metadata: 
  node_type: memory
  type: reference
  originSessionId: 1a989c24-290e-4661-a002-6abe3e37d179
  modified: 2026-09-16T12:26:08.950Z
---

`CLEANUP.md` at the project root (written 2026-09-08, per its own header) documents a prior cleanup pass: which generated folders (DerivedDataCache/Intermediate/Saved/Binaries under `/SOTM/` and `/MetaHumans/`) were already deleted from disk with SHA-256-verified backups under `/.cleanup-backups/`, which large asset packs and duplicate-project folders still need a human decision (`/SOTM/`, `/MetaHumans/`, `/AtrisLA-7745294f/`, several `Content/` marketplace packs like Fab/Forest/Iceland_Environment/LazyDevAac990ce745b2V1), and what `.gitignore`/git-index changes were already made at that time. See [[project_sotm_repo_state]] for what that investigation missed — those "candidate" folders turned out to already be fully committed to git history, not just sitting on disk, which changes the required fix.

**How to apply:** Read this file first before re-auditing the project's folder structure or disk usage — it has real measured sizes and a documented decision trail, so don't re-derive it from scratch or contradict it without re-checking current state.
