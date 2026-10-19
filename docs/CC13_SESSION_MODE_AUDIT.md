# CC13 session-mode audit: `kSessionModeSinglePlayer = 1` in the commands host

A read-only audit (cc13-subdive, 2026-10-19, at cc9's request).
`src/game_hosts_commands.cpp:155` declares `constexpr int kSessionModeSinglePlayer = 1`, with
the comment "[00e188a8]+1fe4h. The single-player value". The question is whether 1 is the right
single-player value, and whether the value matters on any path.

## Which field, and its single-player value

Every use below mirrors the same field, `[[00E188A8]+1FE4h]`. One site reaches it as
`[ECX+F4h]` with ECX = game+1EF0h:

- 0071E7BD `MOV ECX,[00E188A8]`
- 0071E7C3 `ADD ECX,1EF0h`
- 0071E7C9 `CALL 006E38E0`
- 006E38E0 `MOV EAX,[ECX+F4h]`

That is an `ADD`, not a load, so 1EF0h + F4h = 1FE4h is the same dword. No use reads a
different field.

The values of game+1FE4h:

| Value | Meaning | Evidence |
| --- | --- | --- |
| 0 | single player | docs/GAME_EXECUTABLE.md (LobbySettings: the 005E2F93 early-out runs, "single player, game+1FE4h = 0"); docs/CONSTRUCT_WORLD.md (0077F090 Pass D does nothing); docs/GAME_SIMULATION_GATE.md (004DB0D6); docs/CC13_FIX_5.md, docs/DIFFICULTY_MULTIPLIERS.md, docs/CC13_AUDIT_MG_AA_LETHALITY.md |
| 1 | session host (server) | 0077C7B0 broadcasts to peers only in mode 1 (0077C7B6); 0090ABB0 and 008BDEC0 send only in mode 1; include/bsp/cc13_small2.hpp `kSessionModeServer = 1` |
| 2 | client | include/bsp/cc13_small2.hpp `kSessionModeClient = 2`; 00772A40 sends the local score only in mode 2; the "no simulation" arms (director_update_arms.hpp) |

So the single-player value is 0. The 1 at :155 is the session-host value. A likely source of
the "SP is 1" reading is the byte 00E0C978, which 005E2F99..005E2FAB *sets to 1* in single
player when game+1FE4h is 0 (docs/CC13_FIX14.md). That is a different global.

## The uses

Every image site compares the field only against 2, or tests it for the set {0, 1}. Under
either test 0 and 1 give the same answer, so the host's 1 takes the same arm as the image's 0
at every use.

| Host line | Use | Image site and bytes | Test | 0 vs 1 |
| --- | --- | --- | --- | --- |
| :1091 | `director_set_command_008358d0(..., mode)`, then `src/cruise_command.cpp:273` | 00835915 `MOV EAX,[EAX+1FE4h]`, 0083591B `TEST EAX,EAX`, `JZ 00835924`, 0083591F `CMP EAX,1`, `JNZ 00835935` | mode in {0,1} sets the fire target (00835928 `00521EA0(1)`) | same |
| :1325 | `session_field_f4h_is_0_or_1()` (006E38E0) | 006E38E6 `TEST`, `JZ`, 006E38EA `CMP EAX,1`, `JZ`: 1 for {0,1}, else 0 | {0,1} | same |
| :1337 | `echo_command` (0071E2E0) returns in mode 2 | 0071E30E `CMP [ECX+1FE4h],EAX` (EAX = 2), `JZ 0071E338` | == 2 | same |
| :1758, :2243, :2256 | `gameunit_apply_set_command_00721a40(host, mode, flag)`, `src/cruise_command.cpp:242` | 00721B01 `CMP dword ptr [EAX+1FE4h],2`, `JNZ 00721B36` | == 2 (the resolve-or-drop test) | same |
| :2567, :2962, :2974, :2982, :3765, :3966 | `stage_raise_sends_message(stage, mode)` (`include/bsp/command_completion.hpp:91`) | 0071D841 / 0071DA11 `CMP dword ptr [EAX+1FE4h],2`, `JZ` past the message | != 2 | same |
| :2877 | `session_mode()` override for `CommandExecutionHost`; its only readers are `src/command_execution.cpp:317` (008358D0, {0,1}) and `:350` (00835C70's `!= 2`) | as :1091 and 00835C70 | {0,1} / != 2 | same |

The two "tests" the lead listed (:1325, :1337) are those host methods, not unit tests.
:1091 is a use that was not on the lead's list. The lines are from origin/main at 81e083b63.

## Verdict

- (a) Every use mirrors game+1FE4h. Its single-player value is 0, so `kSessionModeSinglePlayer = 1`
  is mislabelled: 1 is the session-host value.
- (b) No single-player path changes. All thirteen uses go through a `== 2`, `!= 2` or {0,1}
  test, and 0 and 1 agree on each of them.

No OFF switch is proposed, and lua70 has no pairs to run. Optionally, change the constant to 0
and its comment to match the rest of the tree, in one comment-and-value commit. That commit
would be behaviour-identical by the table above. Any future reader of this constant that tests
`== 1` or `== 0` (for example a port of 0077C7B0, 0090ABB0 or 008BDEC0) would go wrong with the
current value.
