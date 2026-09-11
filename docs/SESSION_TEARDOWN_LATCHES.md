# Session lifecycle messages and teardown latches

Addresses: 0076d0e0 007727a0 007728b0 00772990 00772610 0075b430 00770b50 00770af0 007848f0 004d7d50

The four writers of `game+1EE4h` do not perform the same disconnect. They cover
end-scene and game-closed messages, their receive handlers, and a conditional
game-closed payload. `00772610` starts mission reload notification: it caches
network mode, clears `+1EE5h`, and sends message `0Bh`. None of these five frees
the session. Their common `007848F0` call flushes pending peer buffers.

All descriptive names below are hypotheses, not recovered symbols. The five
anchors are reconstructed as valid-state C++ sequences; five direct callees
are analyzed contracts. Every live query verified `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Ghidra was read-only; ten exports were
written to the shared ignored `exports/bsp/functions/` tree. All ten already
have Ghidra functions; no function definitions or flow repairs are requested.

## Native ABI and route evidence

| Address / provisional name | ABI from listing | Route |
| --- | --- | --- |
| `0076D0E0` `BSP_Session_ReceiveGameClosed` | Callee cleans 8 stack bytes (`RET 8`, `0076D133`); first argument is message, second is unused output/context pointer. ECX is supplied as session by caller but unused in body. | Dispatcher `00777C4F..00777C81` tests message virtual `+Ch` with `13h`; with session `+F4h != 1`, invokes this handler. |
| `007727A0` `BSP_Session_BroadcastEndScene` | ECX=session, low byte of one stack slot, `RET 4` at `007728A3`. | `004D7A4E..004D7A78`: EndScene in states `0Dh/0Fh`, mode 1, non-abort argument, passes 1. State drain `004E45D0..004E45FC`: cached network mode set, mode 1, `+1EE1h=0`, passes 0. |
| `007728B0` `BSP_Session_BroadcastGameClosed` | ECX=session, low byte of one stack slot, `RET 4` at `00772987`. | The same state drain passes 0 when `+1EE1h!=0`. Also called with 0 at `005D9F01`; accepted-invite callback `004DB27C` is already documented in `GAME_SESSION_POLLS.md`. |
| `00772990` `BSP_Session_ReceiveEndScene` | ECX=session, two unused stack slots; `RET 8` at `00772A31`. Decompiler omits these slots. | Dispatcher `00777C18..00777C45` tests type `12h`; session `+F4h != 1` invokes this handler with message and output/context pointers. |
| `00772610` `BSP_Session_BeginMissionReload` | ECX=session, no stack arguments; `RET` at `00772702`. | `004D7940..004D7966` enqueues game state request `0Ah` then tail-calls this routine on `game+1EF0h`. |

The dispatcher, request queue, EndScene body, and callers remain external.
The dispatcher reads above were bounded call-site inspection, not reconstruction.
`session+F4h` values are not assigned recovered host/client enum names here.

## The five ordered sequences

`0076D0E0`, receive game closed:

1. Read incoming message byte `+18h`. If nonzero, write `game+624h=2` and
   return (`0076D0E4..0076D0FA`). This branch does not write the drop latch.
2. Otherwise compute a Boolean from `[00E198AC]`, its pointer `+60h`, and
   the pointee byte `+4h`; all must be present/nonzero.
3. Call `004D7D50(game, boolean)` at `0076D122`, then set `game+1EE4h=1`
   at `0076D12C`. Only AL of the pushed EAX matters; its upper bytes are
   residual pointer bits, not a second semantic parameter.

`007727A0`, broadcast end scene:

1. Capture `session+188h`, then write raw argument byte to `session+290h`
   (`007727C0..007727C6`).
2. Walk that transport's list (`transport+Ch` sentinel, node `+0h` next,
   node `+8h` peer). The first peer receives no event. When its `peer+D50h`
   is non-null, write the raw argument to that metadata object's `+1Ah`.
3. For every subsequent peer, construct a type `12h` event through `0075B430`;
   overwrite event `+4h=1`, vtable `00D03160`; call `00770B50(session,peer,event)`.
   Restore stack event vtable `00CE4974` without a destructor call. Reload
   `peer+D50h` after dispatch, and write metadata `+1Ah` if it exists.
4. Re-read `session+188h`, falling back to `+18Ch` if null; call `007848F0`
   at `0077287F`. Set `game+1EE4h=1` at `0077288C`, after the flush.

`007728B0`, broadcast game closed:

1. Require a first peer in the `session+188h` transport list and skip it.
2. For each subsequent peer construct type `13h`, overwrite `+4h=1`,
   vtable `00CE74DC`, and **one byte** at event `+18h` with the raw argument.
   Dispatch through `00770B50`; restore vtable `00CE4974`.
3. Select `+188h` or fallback `+18Ch`, then flush at `0077295F`.
4. Only if the raw argument equals zero, set `game+1EE4h=1` (`00772967..72`).
   A nonzero byte preserves the previous latch, including an already-set one.

`00772990`, receive end scene:

1. Set `session+274h=1` at `007729B1`.
2. Construct type `12h`, overwrite `+4h=1` and vtable `00D03160`.
3. Call `00770AF0(session,event)` at `007729DC`, then restore base vtable.
4. Flush selected `+188h/+18Ch` transport at `00772A03`.
5. Call `004D7970(game,0)` at `00772A10`, then set `game+1EE4h=1` at `00772A1B`.
   The end-scene host call therefore observes the old drop-latch value.

`00772610`, begin mission reload:

1. Write `game+1EE3h = (game+1FE4h != 0)` at `0077263C`.
2. Clear `game+1EE5h` at `00772648`.
3. Ensure `[session+188h]+9Ch=4` at `00772659..63`, avoiding a store if already 4.
4. Require and skip the transport's first peer. For every subsequent peer,
   construct type `0Bh`, overwrite `+4h=1`, vtable `00D0314C`, and a **dword**
   at event `+18h=0`. Dispatch through `00770B50`, then restore base vtable.
5. Flush selected `+188h/+18Ch` transport at `007726EB`. No write to `+1EE4h`.

## Direct callee contracts, not ported implementations

| Address / proposed descriptive name | Verified contract |
| --- | --- |
| `0075B430` `BSP_SessionMessage_ConstructBase` | ECX=message; low stack byte=type; `RET 4`. Writes vtable `00D02C68`, `+4h=3`, `+8h=0`, `+Ch=0`, `+10h=type`; `+14h` gets `game+18CCh[index]` iff signed index `game+18ECh` is 0..7, otherwise null. Does not initialize event `+18h`. |
| `00770B50` `BSP_Session_SendMessageToNonlocalPeer` | ECX=session; stack(peer,event); `RET 8`. With `+188h`, compares peer to its first list entry (null if count `+10h` is zero); dispatches other targets to `00783DC0` on that transport. Without `+188h`, uses non-null `+18Ch` only if event virtual `+Ch(29h)` returns nonzero. Assembly `00770BB3` reads the peer stack argument correctly; pseudocode's `unaff_retaddr` is wrong. |
| `00770AF0` `BSP_Session_SendMessageToSecondaryFirstPeer` | ECX=session, stack(event), `RET 4`. If `+18Ch` exists, passes its first peer, or null when list count is zero, to `00783DC0`. Existing `STL_inst_00770af0` label is an automated classification, contradicted by session-field reads and this transport call. No STL code is reconstructed. |
| `007848F0` `BSP_SessionTransport_FlushPeerBuffers` | ECX=transport, plain `RET`. For each peer: enter critical section `peer+4h`, increment its `+18h` depth, visit three pointers at `peer+D40h`, dispatch nonempty buffers through transport virtual `+20h(peer,buffer,index)`, reset buffer `+8h` to base, `+Ch=0`, and first byte to 0; decrement depth and leave. Nonempty test is 32-bit `end-base + ((bits&7)!=0) != 0`. This is queue flushing, not session destruction. |
| `004D7D50` `BSP_Game_ShowGameClosed` | ECX=game, low stack byte controls branch; `RET 4`. Sets `game+608h=1`, calls cinematic mode `(1,0,1)`, conditionally dismisses prompt slot 4 and pause interface. False argument sets `00F1B038=1` and raises `ingame.multi_gameclosed` with callback `004D7B50`; true sets `00F1B038=2` and skips that prompt. Native strings and prompt internals remain external. |

## Reconstruction and limits

`include/bsp/session_teardown_latches.hpp` and `src/session_teardown_latches.cpp`
reuse `MissionOneShots`. The host has one method for each native external call
contract. The list projection retains first-peer exclusion, next-node traversal,
nullable/reloaded metadata, and transport reselection after sends. Message
projections record whether `+18h` is untouched, a byte, or a dword; they do not
claim a wire format. The host constructor owns source-slot resolution.

The three broadcasters require non-null `+188h`. The `007728B0` and `00772610`
lists must contain the first/local entry; the originals call `00BF6713` on
checked-iterator failure. `007727A0` accepts an empty list. The selected flush
transport must exist. Native checked-iterator failure, invalid/dangling nodes,
SEH unwind, reentrant list invalidation and globals changing identity are outside
this valid-state projection. Peer metadata may change during dispatch; it is
read afterwards. No CRT/STL/Win32 networking or XLive routine is ported or stubbed.

All five sequences are build-tested; the existing core and native differential
tests pass after all eight seed byte comparisons. No new tests were added.
Those existing tests do not exercise this module's session routes. These are
new C++ interfaces, not ABI-compatible replacements, and no gameplay or network
runtime validation was performed.

## Corrections and open follow-ups

- `MISSION_STATE_ENTRY.md` identifies `game+624h` as having no known writer;
  `0076D0F0` is now a concrete writer of value 2, on nonzero message `13h/+18h`.
  The semantic meaning of 2 remains unproven.
- Its broad “teardown paths raise +1EE4h” description needs the guards above:
  nonzero `13h/+18h` preserves the latch on both broadcast and receive routes.
- `+1EE4h` is also raised after ordinary end scene; “session dropped” is the
  existing shared field name, not proof that a transport failure occurred.
- Message `12h/13h` serializers, type tests and remote-side handling for
  `session+F4h==1` (`0076D260`, `0076D140`) remain untraced.
- `session+274h`, `session+290h`, peer metadata `+1Ah`, and transport `+9Ch=4`
  retain raw meanings. Follow their consumers before assigning lifecycle enums.
- Transport virtual `+20h` delivery guarantees and `00783DC0` serialization
  remain external; a flush call is not proof of remote receipt.

One disk disassembly attempt started at old documentation address `004E45E8`,
which lies inside the instruction at `004E45E6`, and was discarded. The caller
branch above was rechecked against the stored listing `004E45D0..004E4603`.
No analysis or code depends on the misaligned decode.
