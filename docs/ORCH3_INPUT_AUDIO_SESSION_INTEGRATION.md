# Input, audio and session follow-up integration

Addresses: 00425850 004da6c0 0076d0e0 00772610 007727a0 007728b0 00772990 008d5430 00a7a3f0 00a7a440 00a7b230 00a91a50 00a91e80 00a926f0 00a92aa0 00a92c40

This batch uses `agent/orch3-20260910` and three separate worker worktrees.
Leases reserve each packet's addresses and outputs independently of the other
orchestrators' renderer and persistence work. The workers use read-only Ghidra;
the primary reviews their evidence and handles saved annotations at integration.

The input manager now calls the reconstructed listener classifier directly.
Its confirmation flags require negated auxiliary latches. Timing globals remain
explicit state, initialized to their verified image values. The injected modifier
block is a native string length/data header, with length gating dispatch and CRT
case-insensitive comparisons selecting `fastRelease` and `holdPress`.
See [classifier and rebind evidence](INPUT_ACTION_CLASSIFIER.md) and the correction
appended to [the original input packet](GAME_INPUT_TICK.md).

The sound follow-up separates class descriptors from active entries, reconstructs
level updates and dirty masks, and checks settings application. Its evidence also
corrects mission entry: `00f889a0` is the mutable settings master volume, so the
entry sequence now receives `AudioSettings` and reads `master_20` at the native
point. See [sound evidence](SOUND_MANAGER_LEVELS.md) and the appended correction in
[mission entry](MISSION_STATE_ENTRY.md).

The session packet recovers the disconnection latches and ordered native call
contracts. Its names describe observed behavior and remain hypotheses; platform
and transport calls stay explicit host dependencies. See
[session teardown evidence](SESSION_TEARDOWN_LATCHES.md).

Exact build, focused-check and annotation outcomes are tracked in
`reports/orch3_input_audio_session_integration.json`. None of these new C++
interfaces establishes original ABI compatibility or original-game validation.
