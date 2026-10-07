# Retained land-task hook (cc11)

Packet `cc11_land_retained_hook`, 2026-10-06. **Conditional source
reconstruction, unbound.** No GameUnitsHost retirement/dequeue wiring changed.
The previous audit is `LANDING_TASK_RETIREMENT_CC11.md`.

`plane_squadron_host.hpp/.cpp` now project the complete conditional control
flow of `009B33F0 -> 007EFB60 -> 007EEE50`. They reuse `PlaneSquadronEntity`
and the existing `CommandQueueState` singleton-token getter. Caller-owned
views carry a particular old task's retained squadron and the two writable
native bytes. Required host providers supply actual plane facts, game/session
state, and the recovered deferred message route. There are no fallback
providers, no-op adapters, synchronous promotion, or live game binding.

## Native scope and target

Every live Ghidra query verified project `bsp`, program
`/battlestationspacific.exe`, x86 little endian 32-bit, image base `00400000`,
using the configured `C:/Users/sqz269/bsp.gpr`. Ghidra, ledgers, packet metadata,
and CMake were read-only. Undefined getter/message entries remain raw fragments;
the worker did not create functions or change analysis.

| Entry | Exact body / instructions | ABI and coverage |
| --- | --- | --- |
| `009B33F0` | `009B33F0-009B3433` | ECX=old task, plain RET or tail-jump; complete conditional source projection |
| `007EFB60` | `007EFB60-007EFB9A` | ECX=squadron, plain RET; complete conditional source projection |
| `007EEE50` | `007EEE50-007EEF2D`, RET 4 at `007EEF2B` | `__thiscall(squadron, candidate_plane)`; complete conditional source projection with external construction/routing contract |
| `009F9CE0` producer | `009F9CE0-009F9D77`, RET 8 at `009F9D75` | ECX=approach, plane/reference-speed on stack; whole cache producer read |
| `009B3240` task producer | `009B3240-009B3306`, RET 8 at `009B3304` | ECX=task, bot/block on stack; whole construction listing read |
| `009F9980` bind-to-task | `009F9980-009F99A5` | ECX=approach, task on stack; whole body read |
| `007ED580` command thunk | raw `007ED580-007ED58A`, INT3 begins `007ED58B` | ECX=squadron; undefined Ghidra fragment, not reconstructed as an ABI entry |
| `0071BE40` command getter | `0071BE40-0071BE5A` | ECX=controller; full getter read and existing typed provider reused |
| `0077C2A0` route | `0077C2A0-0077C469`, RET 12 at `0077C467` | ECX=sender, message/flags/status on stack; complete listing read; routing remains an external provider |
| `0076E520` local-copy enqueue | `0076E520-0076E70C` | receive-queue object in ECX; message and sender are distinct stack arguments; copy/queue head read, allocation tail remains an existing external contract |
| `007F0030` receiver | `007F0030-007F01E5` | ECX=squadron, message on stack; partial evidence: BE arm `007F0077-007F0089` |
| `007ED610` rotation | `007ED610-007ED64E`, RET 4 at `007ED64C` | ECX=squadron, index on stack; complete body read; existing typed rotation omits the reindex callee |
| `007EF6F0` BE writer | raw `007EF6F0-007EF70F`, RET 4 at `007EF70D` | ECX=message, write stream on stack; complete raw fragment, no translated ABI profile supplied |
| `007EF710` BE reader | raw `007EF710-007EF730`, RET 4 at `007EF72E` | ECX=message, read wrapper on stack; complete raw fragment, no translated ABI profile supplied |

Body limits include operand bytes after the final instruction address. Names
and semantic labels remain hypotheses. The C++ references/views/virtual host
interface are different from the native layouts and calling conventions.

## Producer and reference lifetime

`009B3272` sets the approach address to task+3F8h, and `009B3283` calls its
constructor with the bot's plane and chosen block. `009AFE8A` calls
`009F9CE0`. The latter directly stores:

- `009F9CEA`: plane at approach+4h = task+3FCh.
- `009F9CF6/009F9CFC`: plane+9D4h copied to approach+0Ch = task+404h.

Neither this cache producer nor `009F9980` registers an observer or adds a
reference count for those two fields. `009F9980` only binds approach+18h,
+1Ch and +20h to the task's command/controller components. The derived land
constructor's state-registry work is separate; the approach's state cleanup
does not establish automatic nulling of these cached fields.

This proves a cache of the old squadron, not ownership of its lifetime. The
new views therefore borrow an explicit caller-owned old instance and do not
invent refcounting, observer invalidation, or a current-membership fallback.
The old task/view, squadron, byte references, command controller, member
planes, and host provider must remain alive through the hook. The caller must
provide a different view for a replacement task. Registry vector growth does
not satisfy these obligations: a raw pointer into `records()` can move.

Death-time invalidation and the actual bot/task deletion chain remain open.
This packet does not turn `departed_units` into native pointer retention and
does not preserve a stale back pointer to force the later destructor dequeue.

## Real current-command comparison

The squadron constructor stores vtable `00D087C0` at `007F2CAD`.
Its `+174h` dword at `00D08934` is `007ED580`. Raw bytes are
`8B 89 48 03 00 00 E9 B5 E8 F2 FF`:

```text
007ED580 MOV ECX,[ECX+348h]
007ED586 JMP 0071BE40
```

`0071BE40` returns the **native interned command pointer token**, not a
host command class or task kind:

| Controller mode +30h | Native value | Existing typed value |
| --- | --- | --- |
| 1 | controller+54h | `state.slots[0].command` |
| 2 | controller+188h | `state.override_slot.command` |
| other | zero | zero |

`CommandSlot::command` explicitly carries the native singleton addresses.
`command_queue_current_command` implements these three exact reads. The hook
uses it twice and compares the second result to the interned `land` pointer
`00E08FA0`, matching `009B33FF-009B3420`. It reloads the old task's view at
each native field read; it does not collapse the two probes.

The required `command_queue_348` provider must expose this retained
squadron's actual typed controller state, including its override. Existing
`GameCommandsHost::current_command_0071be40` instead supplies zero for the
override argument; the plane's `attack_command_class` latch and RTB census
also do not supply this state. None was wired as a substitute.

## Exact hook order and promotion request

`009B33F0` does nothing when its retained pointer is null. A non-null,
nonzero first command probe followed by `land` on the second probe also
returns. Otherwise it reloads task+404h and calls `007EFB60` when non-null.

`007EFB62` reads the member count **before** both unconditional byte clears
at `007EFB69` (+3B0h) and `007EFB6F` (+3B8h). With that count greater than
one, it reads current member zero; only its landed +904h byte and mode
+900h == 5 allow the second member to reach `007EFB95 -> 007EEE50`.
The typed projection preserves this order, including clear-before-plane-facts.

`007EEE50` refuses fewer than two members, the current leader as candidate,
and a **present** current game with session mode +1FE4h == 2. It then searches
backwards from count-1 through index 1, never accepting index zero. A found
candidate produces a BEh message request. The source projection leaves the
member array untouched.

Admitted source object domain: native five-wing array, live count 0..5,
valid occupied member pointers, valid borrowed controller/byte storage, and
providers for actual facts. Unknown/corrupt object layouts are excluded;
there is no added clamp or synthetic plane fact.

## Deferred BE routing is a separate contract

At `007EEEDE`, base `0075B430` initializes the session-message fields using
the real game's selected-owner slot. The call then sets:

- vtable `00D02E5C` at +0h; delivery +4h = 1;
- sender WORD +18h = 0, relay byte +1Ah = 0;
- candidate index at +1Ch.

At `007EEF16`, `0077C2A0` receives ECX=the retained squadron, the message,
routing flags **5**, and status pointer **null**. Sender identity must be
preserved. The base constructor requires a valid current game when reached,
although `007EEE50`'s preceding mode gate permits a null game; null-world
emission is therefore an excluded caller domain, not a fabricated success.
Router readiness requires game+5D4h >= 10; a route can produce no local copy
when its native gates reject it. The source API returns no invented success.

`00D02E5C` contains writer `007EF6F0` and reader `007EF710`. Their recovered
raw fragments call extended-header write/read `0075B480`/`0075B4C0`, then
write/read the +1Ch index in **3 bits** via `00429090`/`00428D30`. Header,
payload, padding, and native profile ABI are not replaced with an integer-only
wire message in this source projection.

For a local route, `0077C44D` calls `0076E520` with message and sender.
`0076E575` invokes the message writer; `0076E5A4` rebuilds through
`00768530`. `0076E5B2/0076E5B9` stamps the **copy's** +18h from the separate
sender+174h argument, and `0076E5CB/0076E5CE` carries the original message's
+14h. The copy is stored in the receive queue at +24Ch. It is not a call to
the squadron message receiver.

Later receipt reaches `007F0077-007F0089`, which reads message+1Ch and calls
`007ED610` at `007F007B`. That rotates the member prefix and calls reindex
`007ED260` at `007ED645`. Existing `plane_squadron_promote_flight_leader_007ed610`
projects the rotation but explicitly omits that reindex call. It must not be
invoked synchronously by the new hook's routing provider.

The required `construct_and_route_promotion_be` provider owns this full
construction/routing/copy domain. No provider implementation, native BE
profile, or receive-stage adapter is supplied here. Those remain unfinished
dependencies for a live binding; the source fixture supplies a diagnostic
queue only and does not validate native serialization.

## Retirement schedule and next packets

This source hook must eventually run on the **old task record** at
`00999EE0`'s +58h dispatch, before that record's scalar destructor. It does
not itself dequeue. The previous audit's drain-before-install (+7Ch) and
drain-before-finished-head-retirement (`0099ADC2` before `0099AE7E`) order
must remain intact. No FIFO was added to the Boolean-only GameUnitsHost.

Next bounded work:

1. Recover/define the raw `007ED580`, `007EF6F0`, `007EF710` fragments under
   the primary agent's Ghidra write lock; implement the actual BE native
   profile/factory and deferred routing/receive binding using the existing
   message/bit-cursor providers. Preserve selected-owner/sender identities
   and the 3-bit payload; finish reindex at receipt.
2. Recover bot ownership/destruction of active and retired task objects and
   stable old task identities. Add a per-bot retired FIFO and explicit
   borrowed-reference lifetime contracts before connecting this hook and
   `009B3F50` to GameUnitsHost. The separate death chain is still uncertain.

## Validation

A single ignored local source probe uses an old task whose head slot contains
`land` but whose **mode 2 override** contains the native `retreat` token.
It verifies two command probes, both clears before plane-fact/routing calls,
one deferred index-1 request, unchanged member order, and untouched state
for a distinct newly installed task. It also checks the admitted mode-1,
mode-2, and idle getter values against the native listing.

MSVC x86 compiled the changed squadron-host object and the probe, then linked
the existing main `bsp_core.lib` with `/MANIFEST:EMBED` and no incremental
link. Result:

```text
PASS mode2 interned-token read; old task identity; both clears before deferred BE(index1); members unchanged
```

This is **source fixture evidence**, not a native runtime test. The report's
direct-call rows pass live `verify_report_calls.py`; raw/indirect dispatch
evidence is separately labeled. Worker whitespace checks passed. The primary
agent owns the full MSVC main build/integration. No ABI compatibility,
native message/profile execution, original-game task lifetime, death repair,
or improvement to the USNOS 202 landing loop is claimed.
