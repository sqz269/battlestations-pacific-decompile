# The fixed step's callback list: the record, who registers, and the per-step call

Addresses: 00874DE0, 00875A80, 006F7360, 00CFAFE0; read as contracts 00875E3F, 006F5610,
007AC000, 00875340, 00D051E8.

Packet `cc_dyn_step`, 2026-09-11. Reconstructed in `include/bsp/fixed_step_callbacks.hpp`
and `src/fixed_step_callbacks.cpp`; semantic C++ interfaces for MSVC Win32, not drop-in
binary replacements. Descriptive names are hypotheses, not recovered symbols. The saved
project is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. This worker made
no Ghidra mutation; the ledger records the new names.

`docs/FIXED_STEP_FANOUT.md` row 5 declares the host method
`run_fixed_step_callbacks_00874de0` and leaves the list unread. This document is that
method's body.

## The list

An intrusive doubly-linked list with two sentinels, both in static data.

| address | what |
| --- | --- |
| `00E0B748` | the head sentinel's `next` field; `00874DE1` starts the walk here |
| `00E0B76C` | the tail sentinel node; `00874DE7` and `00874E1F` stop there |
| `00E0B778` | the tail sentinel's `prev` field, i.e. the current last node; `00875A80` reads and writes it |

`00E0B778 - 00E0B76C = 0Ch`, which is the `prev` offset, so the two addresses are one node.
Two sentinels make the unlink at `00874E13`/`00874E1C` total: a node that is first or last
patches a sentinel rather than a null.

| node | field |
| --- | --- |
| `+00h` | vtable |
| `+0Ch` | prev |
| `+10h` | next |
| `+18h` | byte, linked |
| `+19h` | byte, repeating |

The step callback is vtable slot 4, `[[node]+10h]` (`00874DF8`).

## `00875A80`, the registration

`__thiscall void(node, bool repeating)`, `RET 4` at `00875ADB`, body `00875A80..00875ADD`,
under the critical section the singleton at `00875340` owns. `ECX` is the node and the bool
arrives at `[ESP+0Ch]` (`00875AC1`). When `node+18h` is clear (`00875A9B`) it appends at the
tail (`00875AA1..00875AB9`); then it writes `node+19h = repeating` (`00875AC5`) and
`node+18h = 1` (`00875AC8`) **whether or not** the node was already linked, so re-registering
an already-linked node only changes its repeating flag.

Both call sites, which is the whole of the contract:

| site | containing function | `this` | repeating | node vtable |
| --- | --- | --- | --- | --- |
| `006F5767` | `006F5610` | `ESI+310h` (`LEA EBP,[ESI+310h]` at `006F5676`) | `1` (`PUSH 1` at `006F5735`) | `00CFAFE0`, written at `006F567C` |
| `007AC095` | `007AC000` | `ESI+3E4h` (`LEA ECX,[ESI+3E4h]` at `007AC088`) | `0` (`PUSH 0` at `007AC086`) | not found |

Both forms are real, so the run has to handle both. `007AC000` is slot 3 of the vtable at
`00D051E8` and registers only on the branch where `006F2C30` returned something
(`007AC07C..007AC095`); it also sets `owner+490h` at `007AC08E`. The node's own vtable is
written by that object's constructor, which this packet did not find.

`00CFAFE0` slot 4 is `006F7360`, which has **no Ghidra function** (table below). Its own
body reads `[00E188A8]+1FE4h` at `006F737D`, the same mission-state word `007AC000` tests,
and is not otherwise read.

## `00874DE0`, the run

`__cdecl void(void)`, `RET` at `00874E29`, body `00874DE0..00874E29`, sole caller
`00875E3F`, which pushes nothing.

```
for (n = [00E0B748]; n != 00E0B76C; n = next) {
    [[n]+10h](n, [00D0DE84]);              ; __thiscall, RET 4
    next = n->next;                        ; read AFTER the call, 00874E07
    if (n->repeating == 0) {
        n->linked = 0;
        n->prev->next = next;
        next->prev = n->prev;
    }
}
```

Three properties worth stating because they are the contract a callback can rely on.

**The step is not the fan-out's.** The site passes nothing; the callee loads the image float
at `00D0DE84` itself (`00874DF2`) and pushes it per callback. It is `0.05f`, the same value
every other row of the fan-out receives, but the coupling is inside this routine, not at the
call site.

**`next` is read after the callback returns.** A callback may unlink itself, or register
something, and this same walk sees it.

**The walk takes no lock.** `00875A80` takes the critical section from `00875340`; this does
not. A registration from another thread during the walk is a race the image does not guard,
which is consistent with both registrants being on the simulation thread.

**A one-shot registration fires exactly once.** `007AC095`'s node is unlinked immediately
after its callback returns, on the same step.

## Corrections

None. The plate comment already on `00874DE0` from packet `cc_fixed_step` describes the same
walk; this packet read the two registrants, the sentinel geometry and the vtable slot, which
that comment did not have, and found nothing in it to correct.

## Coverage

| routine | state | coverage |
| --- | --- | --- |
| `00874DE0` | reconstructed as a sequence over `FixedStepCallbackHost`, build-tested | complete |
| `00875A80` | reconstructed as a pure rule over the list, build-tested | complete for the list surgery; the critical-section acquire and its recursion counter are host-side |
| `006F5610`, `007AC000` | read at their registration sites only | partial: the two call sites and the registers feeding them, not the bodies |
| `006F7360` | not read | none; contract is "the repeating registrant's per-step callback" |

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `fixed_step_callback_owners` | `006F5610`, `006F7360`, `007AC000`, `00D051E8` | what the two registrants are. `006F5610` is in the bullet/projectile segment and `007AC000` in the path/paratrooper segment, so the list looks like a per-step service for short-lived gameplay objects, but neither owner class is identified |
| `bsp_critical_section_00875340` | `00875340`, `00BD1860` | the singleton whose critical section guards the registration, and whether anything else shares it |

## no_ghidra_function

| start | inclusive end | evidence |
| --- | --- | --- |
| `006F7360` | `006F7665` | Start: it is the target of slot 4 of the vtable at `00CFAFE0` (`[00CFAFF0] = 006F7360`), and the bytes there are a fresh SEH prologue (`64 A1 00 00 00 00` / `PUSH -1` / `PUSH 00C83690`). The previous Ghidra function `006F6760` ends with `RET` at `006F7352` (byte `C3`) and `006F7353..006F735F` is `CC` padding. End: `C2 04 00` (`RET 4`) at `006F7663..006F7665`, matching the `__thiscall void(node, float)` the caller uses, followed by `CC` padding `006F7666..006F766F` and the next Ghidra function `FUN_006F7670` at `006F7670` (bytes `64 A1`). The integrator should run `python tools/ghidra_define_function.py 006f7360 006f7666` before the ledger name `BSP_FixedStepCallback_RepeatingStep` can be applied. |
