---
name: project-sotm-repo-state
description: SOTM Unreal repo has ~8.1GB of foreign projects + a 3.6GB LFS gap already baked into pushed GitHub history — pending team decisions before onboarding other devs
metadata: 
  node_type: memory
  type: project
  originSessionId: 1a989c24-290e-4661-a002-6abe3e37d179
  modified: 2026-09-16T12:26:01.756Z
---

As of 2026-09-16, `origin/main` on github.com/CJTechnology21/unreal-project-with-Parth already contains, fully committed (verified via `git ls-files`, not just present on disk):

- `/SOTM/` — a second, older, ~5.6 GB copy of this same game (6,510 tracked files). A prior diff (see [[reference_cleanup_md]]) found 2,606 files byte-identical to the root project, 2,876 differing, and 1,029 that exist only under `/SOTM/`. Not yet resolved which is authoritative.
- `/MetaHumans/` — Epic's own MetaHuman Sample project (~2.3 GB, 748 files), not part of this game.
- `/AtrisLA-7745294f/` — a raw `.mhpkg` MetaHuman import package (~209 MB, 3 files).
- A Git LFS gap: 10,357 tracked `.uasset`/`.umap`/`.fbx`/`.mhpkg` files (3.59 GB, confirmed by diffing `git lfs ls-files` against the full tracked list) are plain git blobs instead of LFS pointers, despite `.gitattributes` covering those extensions — likely committed before the LFS rule existed. Adding/editing `.gitattributes` never retroactively converts already-committed history; only `git lfs migrate import` (a history rewrite) does.

**Why:** Discovered while building a git push/cleanup plan (published as an artifact, "Push Manifest," https://claude.ai/artifact/MXz5cuVKPN9aUYEWRnxpjD) ahead of other developers joining the repo. A prior session's `CLEANUP.md` had already found the foreign projects sitting on disk but hadn't checked whether they were committed to git history — they are, which changes the fix from "just gitignore it" to "needs a deliberate history rewrite."

**How to apply:** Before recommending or running any git history rewrite (`git rm -r --cached`, `git lfs migrate import`, `git filter-repo`/BFG, force-push) for this repo, confirm with the user (a) whether anyone besides this machine has already cloned `origin/main` — changes the urgency/safety math — and (b) which of `/SOTM/` vs. the root project is authoritative, since `/SOTM/` has 1,029 unique files nowhere else. Don't assume this is still unresolved without checking current repo state first — re-verify with `git ls-files SOTM/ MetaHumans/ AtrisLA-7745294f/` and `git lfs ls-files` counts, since this is exactly the kind of point-in-time finding that goes stale once the user acts on it. Also worth re-checking: as of this session, the entire `Source/` C++ tree and the new `Content/SOTM/` scaffold (the actual current milestone work) were 100% uncommitted/unpushed — confirm they landed before assuming they're in history.
