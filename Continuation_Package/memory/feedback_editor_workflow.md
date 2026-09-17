---
name: feedback-editor-workflow
description: "After C++ edits, tell the user to reopen the editor via Visual Studio rather than building via CLI; diagnose compile errors from Log.txt directly"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: bf986c9d-5fd6-4d8e-b888-497a3214ab0f
  modified: 2026-09-09T23:42:53.394Z
---

After making C++ source changes, do not run a build via command line. Instead, tell the user to close the Unreal Editor and reopen the project via Visual Studio, which builds and launches in one step. If the user reports a compile error, diagnose it by directly reading `C:\Users\ParthSikka\AppData\Local\UnrealBuildTool\Log.txt` rather than asking the user to paste the error text.

**Why:** User's own words: "once done just tell me to re open editor with code so that it auto compiles, because if I close the editor and you compile its a time taking process, I'll just re open the editor using IDE and if it doesn't compile it means there is an error then only I'll open claude outside unreal." Building via IDE is faster in their workflow than a CLI build initiated by the assistant, and the user already knows to come back if (and only if) the reopen fails to compile.

**How to apply:** After any Source/ edit, end the turn by telling the user to close the editor and reopen via Visual Studio. Don't proactively run Build.bat or similar. When they report a compile error afterward, go straight to reading Log.txt for the actual error rather than asking "what does the error say."
