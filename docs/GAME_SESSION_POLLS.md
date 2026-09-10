# Game session polls

Addresses: 004db290, 004caa90, 004d80d0, 006840f0, 00776230, 004c40f0, 00778560,
004e4b91, 004e4b98, 004e5036, 004e5434, 004e5442, 004e5455, 00a40110, 00a409f0,
00a3e3b0, 00a3e420, 00a3e440, 00a3e460, 00a3e470, 00a3e4b0, 00a3eac0

Packet `game_session_polls`, proposed by `docs/GAME_ON_MOVE_MAP.md`. Read-only analysis of the
Ghidra project plus a reconstruction in `include/bsp/session_polls.hpp` and `src/session_polls.cpp`.
Every name below is a hypothesis, not a recovered symbol.

`BSP_Game_OnMove` (004e4a40) reaches these routines at four points in one frame:

| Site | Callee | Position in the frame |
| --- | --- | --- |
| 004e4b91 | 004db290 | step 8, right after the blocking-screen fast path returns |
| 004e4b98 | 004caa90 | step 8 |
| 004e5036 | 00778560 | step 16, unless the in-game simulation will advance |
| 004e5434 | 004d80d0 | step 23, post-simulation |
| 004e5442 | 006840f0 | step 23, the first request poll |
| 004e5455-004e5488 | 00776230, 004c40f0, 006840f0 | the do/while Ghidra removed as unreachable |

The stored body of 004e4a40 is eight bytes, so every statement about the call sites comes from
`python tools/bsp.py disasm-raw 004e4a40 --length 2808`, not from `show`.

## Calling conventions

| Address | Signature | Return |
| --- | --- | --- |
| 004db290 | `__fastcall(GGame*)`, ECX only; ESI is loaded from 00f8abe8 | `ret`, no value |
| 004caa90 | `__fastcall(GGame*)` | `ret` at 004cac2a |
| 004d80d0 | `__fastcall(GGame*)` | `ret` at 004d8606 / 004d861f; EAX carries the last callee's value and no caller reads it |
| 006840f0 | `__fastcall(bool *out)`, ECX holds the out pointer | `ret` at 006841e6 |
| 00776230 | `__fastcall(void* peer)`, ECX = game+1EF0h | `ret` at 0077648a |
| 004c40f0 | `__fastcall(GGame*)` | `ret` at 004c42f8 |
| 00778560 | `__fastcall(void* peer, float seconds)`, ECX = game+1EF0h | `ret 4` at 00778582 |

004db290, 004caa90 and 004d80d0 each open an SEH frame (handlers 00c668e8, 00c65598, 00c66498).
Those frames make the decompiler alias the pushed arguments onto stack locals, so the argument lists
in `exports/bsp/functions/*/decompiled.c` for these three are wrong; the control flow is not. Every
argument list quoted here comes from the listing.

## 004db290, the platform session poll

### The notification drain it sits on

The first call, 004db2bc, is `00a409f0(00f8abe8)` — one pump of the platform manager
(`XenonSystemManager`, `docs/APP_INIT_AUDIO_ONLINE.md`). That pump calls 00a40110, which is the only
`XNotifyGetNext` loop reached from the frame (the two other call sites of 00a4d5ae are 00a40161 in
the same routine and 00a49562). The loop re-creates the listener with `qwAreas = 2Fh` when
manager+1Ch is -1, logs `" ...   NotifyGet %x %x"` for every notification, and dispatches:

| Id | Provisional XNOTIFY name | Arm | Effect |
| --- | --- | --- | --- |
| 00000009h | XN_SYS_UI | switch | calls the hook in 00f8abec (004ceb40, installed at 004e555e), then `manager+3E8h = parameter != 0` |
| 0000000Ah | XN_SYS_SIGNINCHANGED | switch | 00a3f440 |
| 0000000Bh | XN_SYS_STORAGEDEVICESCHANGED | switch | calls the hook in 00f8abf0; nothing in the image writes that pointer, so on this build the arm is a no-op |
| 0000000Eh | XN_SYS_PROFILESETTINGCHANGED | switch | 00a3e600(parameter) |
| 00000015h | XN_SYS_XLIVETITLEUPDATE | 00a40416 | builds a path, compares it against `\setup.exe` |
| 00000016h | XN_SYS_XLIVESYSTEMUPDATE | 00a404a9 | `XLiveUpdateSystem` then `_exit(0)`; the process does not come back |
| 02000001h | XN_LIVE_CONNECTIONCHANGED | 00a40353 | logs; parameter 001510F0h calls 00a3fa70(1) at 00a40391; parameter 80151005h sets `manager+128h` at 00a403b1 |
| 02000002h | XN_LIVE_INVITE_ACCEPTED | 00a4031e | `XInviteGetAcceptedInfo(parameter, &info)` (00a4d5a8), `manager+31h = 1` at 00a4033c, then `REP MOVSD` of 15h dwords into manager+32h at 00a4034c |
| 02000003h | XN_LIVE_LINK_STATE_CHANGED | 00a403bd | log only |
| 02000007h | XN_LIVE_CONTENT_INSTALLED | 00a40400 | `manager+30h = 1`, log `"DLC: downloaded content installed"` |
| 04000002h, 04000003h | XN_FRIENDS_* | 00a403e6 | 00f8a2fc virtual +28h when that object exists |

Ids outside this list fall out of every compare and are dropped. The id names come from the public
`XNOTIFY_*` set; 02000001h and 02000003h are corroborated by their own log strings. The numeric ids
and the effects are recovered; the names are provisional.

`REP MOVSD` of 15h dwords is 84 bytes, which is exactly `XINVITE_INFO`
(`XUID xuidInvitee; XUID xuidInviter; DWORD dwTitleID; XSESSION_INFO hostInfo; BOOL
fFromGameInvite`, with `XSESSION_INFO` = 8 + 36 + 16). Every offset 004db290 reads agrees:

| Offset in the block | Field | Read at |
| --- | --- | --- |
| +00h, 8 bytes | `xuidInvitee` | 00a3e470 compares it against the local slot key |
| +08h, 8 bytes | `xuidInviter` | 00a3e4b0 compares it against the same key |
| +10h | `dwTitleID` | 004db574, against 534307FAh |
| +14h, 8 bytes | `hostInfo.sessionID` | 004db54f, against the current session key |
| +1Ch, 36 bytes | `hostInfo.hostAddress` | 004db58d, passed to `XNetXnAddrToPlatform` |
| +50h | `fFromGameInvite` | 004db805 |

### The manager accessors

Five one- and two-instruction accessors on the manager gate the poll. They are a family: the flag
bytes sit at +2Ch..+31h and each accessor takes a "clear" argument in the stack slot.

| Address | Behaviour |
| --- | --- |
| 00a3e3b0 | returns `manager+2Ch != 0 && manager+3C0h != 3E5h`. No clear argument; the flag stays set |
| 00a3e3e0 | test-and-clear of +2Dh (not used by this packet) |
| 00a3e400 | test-and-clear of +2Eh (not used by this packet) |
| 00a3e420 | test-and-clear of +2Fh, the storage-removed flag |
| 00a3e440 | test-and-clear of +31h, the accepted-invite flag |
| 00a3e460 | `lea eax, [ecx+32h]` — the address of the copied `XINVITE_INFO` |
| 00a3e470 | index of the local slot whose 8-byte key at manager+110h equals info+00h, else 1 |
| 00a3e4b0 | the same search against info+08h |
| 00a3eac0 | `manager+11Ch`, the active local slot index |

3E5h is `ERROR_IO_PENDING`: 00a3e3b0 suppresses the profile-changed report while an asynchronous
operation is still outstanding. The slot loop in 00a3e470 and 00a3e4b0 runs over exactly one entry
(`cmp eax, 1; jb` at 00a3e49d), so both return 0 or 1, and 1 means "no local slot matches". 00a40020
sets manager+11Ch to 1 by default, so `00a3e470 == 00a3eac0` reads as "the invitee is the active
local profile".

### The three arms

**Profile changed, 004db2d8-004db40c.** Runs when 00a3e3b0 reports the flag and `game+5D4h != 2`;
it ends with `ret`, so nothing else in the routine runs this frame.

- `game+620h` non-zero calls 004d94f0(game, 0); `game+61Fh` non-zero calls 004d95f0(game, 0).
  Both are SEH-wrapped and take a byte argument; what they do is not established.
- In state 4, 004d7f90: drains the state-request queue, then either sets state 2 (when 00e198c4 is
  null) or enqueues request 10h.
- Otherwise 004db190 (releases 00e198b4 and 00e198b8, calls 004d7970(0) and 004da780 when 00e198c4
  exists, sets 00e08874), then 004cccc0 (leaves the session through 0076fad0(0) when game+1FE4h is
  set, releases 00e18678), `game+2180h = 0`, and `BSP_Game_OnInitTitle` (004c9a70).
- Then the notification prompt `FE_xbox.xsm_profilechanged`.

**Storage removed, 004db40d-004db4ea.** `00a3e420(1)` consumes the flag whether or not the prompt
appears; the prompt is raised only when `*(int*)(0109cecc + 4) == 0`. It calls 00bd3450 on 0109cecc,
raises `FE_xbox.xsm_storageremovedauto`, and **falls through** into the invite arm rather than
returning.

**Accepted invite, 004db4eb-004db8f0.** Gated by `game+5D4h >= 2` first, so an invite that arrives
during the logo sequence is not consumed and stays pending. Then `00a3e440(1)` takes the flag.

1. 004db509: unless `game+610h` is set, if the player is signed in (004b44f0 = `00a3e510` and
   `00a3ead0 == 2`), the invitee slot equals the active slot, `game+1FE4h != 0`, and
   `info.hostInfo.sessionID` equals the current session key from `00a43560(00f8a2fc)`, the routine
   returns: the player is already in the invited session.
2. 004db561: 00425d10 then 00530650, which runs 00532a20 over seven layers — the dialog layers are
   dismissed before anything is shown.
3. 004db574: `info.dwTitleID != 534307FAh` returns without a prompt.
4. 004db596: `XNetXnAddrToPlatform(&platform, &info.hostInfo.hostAddress)`. When the call succeeds
   and reports anything other than 2, the prompt is `FE.crossplatform_notsupported`. A failed call
   falls through to the normal path.
5. 004db620: when 00a3e4b0 finds a local slot for `xuidInviter`, the invite came from this console
   and the prompt is `FE_xbox.xsm_invitesameconsole`.
6. 004db69d: in state 2 the invite is accepted straight through 004d8000, with no prompt.
7. 004db6c0: signed in and the invitee is the active slot. The message is built by concatenating
   `FE_xbox.xsm_joinwarning|.\n|`, then `globals.willendsession|.\n|` when 004bb8a0 reports that
   leaving would end the session for the others, then `globals.areyousure`. Callback 004db260.
8. 004db7fe: otherwise the invitee is a different local profile.
   `info.fFromGameInvite != 0` gives `FE_xbox.xsm_inviteotherplayer`, zero gives
   `FE_xbox.xsm_joinotherplayer`. Callback 004db240.

Step 7 has one dead branch. 004db6e7 assigns `FE_xbox.xsm_invitewarning|.\n|` and 004db718 replaces
it with the join variant when 00a3e470 and 00a3eac0 agree — but reaching 004db6e7 already required
them to agree (004db6df), so the invite variant is unreachable and `FE_xbox.xsm_invitewarning|.\n|`
(00ce7bb8) never reaches a prompt.

### The prompt call

Every prompt goes through `00531b00` with ECX from 00425d10 and eight arguments, recovered from the
push order at 004db39d, 004db497, 004db602, 004db686, 004db7da and 004db86f:

`00531b00(kind, const String& message, style, callback, flag, const String& title, 0.0f, 0)`

The title is always the empty string at 00ce3a0c and the last two arguments are always `0.0f` and 0.
`kind` is 2 everywhere except the storage prompt (004db497 passes 0). `style` is 2 for the
notification-only prompts and 1 for the two that carry a callback. `flag` is 1 only at 004db7cc, on
the join warning; its meaning is unresolved.

The two callbacks are `__fastcall(int result)` and both act only on result 1:

- 004db240: tail-jumps to `004d8000(game)`.
- 004db260: when `game+1FE4h == 1`, calls `007728b0(game+1EF0h, 0)` first, then falls into the same
  `004d8000(game)`.

Both sit inside the Ghidra body of 004db220, which belongs to another packet; they are read here,
not analysed or renamed.

## 004caa90, the online stats write

Two gates: `game+22DDh` must be set (a write is in flight) and `game+7038h` must not be 3E5h. The
second is `XOVERLAPPED.InternalLow` still holding `ERROR_IO_PENDING`; `game+7038h` is the overlapped
itself, passed by address to `XGetOverlappedResult(&ov, &result, FALSE)` and
`XGetOverlappedExtendedError(&ov)`. Both values go into
`" - - ONLINE - - Update Stats Write Status code %x and %x"` and `game+22DDh` is cleared.

The pending writes live in a 12-byte MSVC list at `00f8a2fc+2200h` (proxy, head at +2204h, count at
+2208h). The step is:

- `result == 0` and the list is empty: `" - - ONLINE - - WriteStats kaput"`.
- `result == 0` and the list is not empty: erase the front through 004c4cc0, then re-read the count.
- Either way, if the count is now non-zero, copy 0Eh dwords (56 bytes) out of the front node's
  payload, log `" - - ONLINE - - WriteStats userleft %d"` with the remaining count, and resubmit
  through 004c04c0 with twelve fields.
- A failed write does not erase, so the same front entry is submitted again — this is a retry.

On the drained path, when `game+1FE4h == 2`, `004c9780(*(00e188a8+18C8h), 4)` feeds 00770af0 and
004c5f10 runs.

## 004d80d0, the multiplayer interface update

Unconditional ticks first:

1. 004d8100: when 00f8a2fc exists, `client->vtbl[0Ch]((float)game+21ECh)`. The delta is the field at
   game+21ECh, not the scaled clock at game+21F0h.
2. 004d811b: when 00e198b4 exists, `006890b0(00e198b4)`.
3. 004d8126: `0076a750(game+1EF0h)`.

Then the session state machine. `game+1FE4h` is the session mode (0 = none) and `game+624h` is the
termination reason, which the routine sets to 3 once it has handled reason 1 or 2. `00f1b038` is a
three-valued phase latch shared with the front end, and `game+610h` suppresses the whole arm.

- Mode 0: `00f1b038 = 0` and return.
- Mode 2 with reason 0 (004d8167): 004b61c0 must return true, `game+610h` must be clear, and
  `*(*(game+207Ch) + 9Ch) == 3` is latched. In state 0Dh or 0Ch it either runs 004d7ba0 with that
  latch (phase 0) or logs `"GGame::MultiInterfaceUpdate() -> SCENE_TERM"` and enqueues state
  request 10h (phase other than 1). Outside those states it runs 00688c70, and unless the phase is
  2 also 00688ad0(1Bh) and 005d2e70 with a code chosen from the latch, `00f8abe8+128h`, and two
  virtuals of 00f8a2fc (+19Ch and +40h); then `00f1b038 = 0` and 004bdf50.
- Reason 1 or 2 (004d82ce onward): the same shape, with dialogs instead of a silent teardown.
  Phase 0 in state 0Dh or 0Ch raises `FE.multi_terminated_client` (reason 1) or
  `ingame.multi_gameclosed_xbox` (reason 2) with callback 004d7b10, sets `game+608h`, calls
  004cd0f0(1,0,1) and clears `00e19698+4`; otherwise, in state 0Dh, it sets `+288h` on the 00425d10
  object and raises `FE.multi_terminated`. A phase other than 0 or 1 logs
  `"GGame::MultiInterfaceUpdate() -> SCENE_AUTO_TERM"` and enqueues state request 10h.
- Tail: in state 0Dh a HUD bookkeeping block clears `00e19698+4` and sets `+18h` on the active
  player record at `game+18CCh + game+18ECh*4`; in states 0Ch, 0Dh and 12h, 0076c500 runs last.

Request 10h is the teardown arm documented in `docs/GAME_FRAME_CONTROL.md`; both enqueue sites
(004d8290, 004d8514) are already listed there.

## 006840f0 and the drain loop at 004e5455

006840f0 walks four globals in order — 00e198ac, 00e198b4, 00e198b8, 00e198c4 — and for each:

```
if (obj != 0 && *(char*)(obj + 3Ch) != 0
    && (*(int*)(obj + 04h) != *(int*)(obj + 20h)
        || *(int*)(obj + 1Ch) != *(int*)(obj + 38h))) {
    (*(obj->vtbl + 10h))(*(int*)(obj + 20h), *(int*)(obj + 38h));
    *out = 1;
}
```

The pseudocode reaches that shape through a duplicated compare (`if ((obj[1] != iVar1) || (obj[7] !=
obj[0xe]))` inside a branch where the first half is already false); the listing collapses to the
predicate above. The out byte is written only when a channel actually needed servicing, so it means
"work was done", not "a channel is active".

The caller does not use the return value; it passes the address of a stack byte in ECX, zeroes the
byte first, and 006840f0 only ever stores 1.

The loop Ghidra dropped is 004e5455-004e5488. `WARNING: Removing unreachable block (ram,0x004e5455)`
appears at the head of the export because the decompiler cannot see that 006840f0 writes through its
pointer argument, so it proves the `je` at 004e5453 always taken. The listing:

```
004e5439  lea  ecx, [esp+1Bh]
004e543d  mov  byte ptr [esp+1Bh], 0
004e5442  call 006840f0
004e5447  cmp  byte ptr [esp+1Bh], 0
004e544c  mov  byte ptr [00e18cdc], 0
004e5453  je   004e548a
004e5455  lea  edi, [esi+1EF0h]        ; hoisted out of the loop
004e5460  mov  ecx, edi
004e5462  call 00776230
004e5467  mov  ecx, esi
004e5469  call 004c40f0
004e546e  lea  ecx, [esp+1Bh]
004e5472  mov  byte ptr [esp+1Bh], 0
004e5477  call 006840f0
004e547c  cmp  byte ptr [esp+1Bh], 0
004e5481  mov  byte ptr [00e18cdc], 0
004e5488  jne  004e5460
```

So: poll once; while the poll reported work, pump the peer queues, run the interface-only update,
and poll again. The termination rule is entirely 006840f0's predicate — the loop ends when a whole
pass over the four channels finds no active channel with a mismatched index pair. There is no
iteration bound; the loop relies on the virtual at +10h advancing the current indices toward the
targets. 00e18cdc is cleared after every poll, including the first; it is written by the front-end
screen updates 004f8390..004f8830, which belong to another packet.

00776230 appears in the call graph and in no line of the pseudocode, because this loop is its only
reachable call site from OnMove.

The redundant `mov byte ptr [esp+1Bh], 0` at 004e548a and the `cmp`/`jne` at 004e54b5 that follow
are dead: both paths into 004e548a already have the byte clear, and nothing between 004e548a and
004e54b5 writes it, so the jump over the render block at 004e54ba is never taken. That is a
compiler artefact of a shared epilogue, not a second flag.

## 00776230, the peer message pump

`__fastcall(void* peer)` with ECX = game+1EF0h. Returns immediately when 00e188a8 is null. The
network object is `peer+188h` if set, otherwise `peer+18Ch` — the host and client halves of the
same interface. It then drains two locked queues on that object:

| Container | List head | Count | Critical section | Per-node work |
| --- | --- | --- | --- | --- |
| net+28h | +2Ch | +30h | +34h | 00774ab0, 007752d0, 007717a0 on `node[2]` |
| net+18h | +1Ch | +20h | +24h | 007702e0 on `node[2]` when `node[2]+D50h` is set, then 00772ec0 |

Each queue is drained fully: the pseudocode shows a `return` after the first node's `_free`, but the
listing at 007762ff-00776309 falls into `mov esi, ebp; jmp 00776286`, so it is a loop over the whole
list and the `return` is a decompiler artefact of the MSVC debug-iterator checks (00bf6713). When a
queue empties, 0076e850 runs on the container and the critical section is released; the manual
`+18h` increment and decrement around `Enter`/`LeaveCriticalSection` is a recursion shadow counter.

Between the two queues, when `peer+F4h` is set, a throttle reads the platform timer through
`01090ab0` virtual +20h (a `{counter, frequency}` pair), computes `counter / frequency` on the x87
stack, and when the gap since `00e0af18` exceeds `00d7a348` it stores the new time and calls
00784c40 (host half) and 007850e0 (client half) with `00ce3918`.

## 004c40f0, the interface-only update

```
bool guide_path = (00e188ae != 0 || 00e18b34 != 0) && game+719Eh == 0;
game+719Eh = 0;
00e18b34 = 00e188ae;                 // previous-frame latch
```

00e188ae is the system-UI byte 004ceb40 maintains, so `guide_path` is "the guide is up now or was up
last frame". On that path the routine walks 00e198c4's sub-objects at +C8h, +9Ch and +A4h; for each
non-null one whose `+5` byte is set it calls 004b6e50 when `+4` is clear and 004f71f0(game+21F0h)
otherwise. Off that path it goes through the 00425d10 object: `+25Ch` clear runs
004f8830 with game+21F0h, or game+21ECh when `game+635h` is set; `+25Ch` set branches on `+4` into
either a virtual `+1Ch` teardown plus 004f83b0, or 004f71f0(game+21F0h). The same two-way test then
runs on 00e19698. Finally, when `game+624h != 0` and `00e198c4+A4h` exists, that sub-object is
forced active (`+4` and `+5` set), 004f83b0 runs, its virtual `+18h` is called, and 005b6960 closes
out.

004b6e50, 004f71f0 and 004f8830 belong to the front-end-states packet and are external contracts
here.

## 00778560, the multiplayer tick

`00778450(peer, seconds)` then `0076ffc0(peer, seconds, 0)`, with ECX restored from ESI for the
second call. Both take the raw frame delta the caller loaded from `[esp+24h]`; OnMove skips the
whole call when the state is 0Dh, `00f876b0 >= 1` and the scaled delta is positive.

## Reconstruction

`include/bsp/session_polls.hpp` and `src/session_polls.cpp` hold:

- `kPlatformNotificationArms`, the twelve-entry id-to-effect table above, with
  `apply_platform_notification_00a40110` reproducing the four arms that write a manager flag.
- `classify_accepted_invite_004db4eb`, the decision tree of steps 1-8, over an explicit
  `InviteDecisionInputs`, plus `compose_join_warning_text_004db6e7` for the three-part message.
- `online_stats_write_step_004caa90`, the pop-on-success / retry-on-failure rule.
- `service_pending_menu_requests_006840f0` over four `MenuRequestChannel` projections, and
  `run_menu_interface_drain_004e5434`, which reproduces the native loop and its termination rule
  without an iteration bound.
- `SessionPollHost`, one virtual per native call site, and the sequence routines
  `poll_platform_session_events_004db290`, `run_session_polls` (step 8),
  `run_multiplayer_tick_00778560` (step 16) and `run_menu_interface_drain_004e5434` (step 23),
  in the style of `bsp::run_application_frame`.

None of it is ABI-compatible; the structures are projections and every offset is in a comment. One
test case was added to `tests/math_tests.cpp` for the drain loop's termination rule.

## State reached

| Address | State |
| --- | --- |
| 004db290 | exported, analysed, reconstructed (decision tree and prompt shape), build-tested |
| 004caa90 | exported, analysed, reconstructed (step rule), build-tested |
| 004d80d0 | exported, analysed; gates and ticks documented, dialog arms not reconstructed |
| 006840f0 | exported, analysed, reconstructed, build-tested |
| 00776230 | exported, analysed; loop shape corrected against the listing, not reconstructed |
| 004c40f0 | exported, analysed; not reconstructed (its callees belong to another packet) |
| 00778560 | exported, analysed, reconstructed, build-tested |
| 004e5455-004e5488 | recovered from the listing, reconstructed, fixture-tested |

Nothing here is game-validated.

## Callers and callees

- 004db290: callers 004e4b91 and one other; callees 00a409f0, 004ceb40, 00a3e3b0, 004d94f0,
  004d95f0, 004d7f90, 004db190, 004cccc0, 004c9a70, 0041e870, 0041e350, 0041dd20, 00425d10,
  00425e10, 00531b00, 00419cc0, 00bd1510, 00a3e420, 00bd3450, 00a3e440, 004b44f0, 00a3e470,
  00a3eac0, 00a3e460, 00a43560, 00530650, 00a4d434, 00a3e4b0, 004d8000, 004bb8a0.
- 004caa90: callers 004e4b98 and one other; callees 00a4d42e, 00a4d422, 004254b0, 004c4cc0,
  004c04c0, 004c9780, 00770af0, 004c5f10.
- 004d80d0: sole caller 004e5434; callees 006890b0, 0076a750, 004b61c0, 004d7ba0, 00688c70,
  00688ad0, 005d2e70, 004bdf50, 004254b0, 004d3ed0, 0041e870, 0041dd20, 00425d10, 00531b00,
  004cd0f0, 004c3a80, 006529e0, 0076c500.
- 006840f0: callers 004e5442 and 004e5477 only.
- 00776230: callers 004e5462 and one other; callees 00774ab0, 007752d0, 007717a0, 0076e850,
  007702e0, 00772ec0, 00784c40, 007850e0, 00bf65ac, 00bf6713.
- 004c40f0: sole caller OnMove, reached from three places (the non-pause in-game branch at 004e5259,
  the simulation-gate fallback at 004e53b6, and the drain loop at 004e5469); callees 00425d10,
  004b6e50, 004f71f0, 004f83b0, 004f8830, 005b6960.
- 00778560: sole caller 004e5036; callees 00778450, 0076ffc0.

## Uncertainties and what remains

1. **Who sets manager+2Ch and +2Fh.** 00a3e3b0 and 00a3e420 read them, but a scan of the whole
   XenonSystemManager segment (00a371c0-00a46930) finds writes only to +30h and +31h, both inside
   00a40110. The producers of the profile-changed and storage-removed flags are outside that
   segment and were not found.
2. **004d94f0 and 004d95f0** take a byte argument and are called with 0 from the profile-changed
   arm. Their bodies were not analysed.
3. **The fifth argument of 00531b00** is 1 only on the join warning. Its meaning is unresolved, as
   is the first argument's 0 on the storage prompt.
4. **004d80d0's dialog arms** are documented from the pseudocode's control flow, which is reliable,
   but the argument lists inside them were not each checked against the listing. The two log strings
   are pinned: 00ce7a44 `SCENE_TERM` is on the mode-2/reason-0 path and 00ce79c0 `SCENE_AUTO_TERM`
   on the reason-1-or-2 path, which is the opposite of what the names suggest.
5. **00f1b038** is a three-valued latch shared between 004d80d0 and code outside this packet; the
   producer of values 1 and 2 was not traced.
6. **`FE_xbox.xsm_invitewarning|.\n|` (00ce7bb8) is unreachable** through the compare at 004db6df.
   Whether the source intended a second entry point into 004db6e7 is not established.
7. **004b44f0** is called with ECX = the platform manager, but Ghidra types it as taking no
   arguments and its two callees (00a3e510, 00a3ead0) are manager accessors. The other three callers
   were not checked, so its parameter is inferred from this call site only.
