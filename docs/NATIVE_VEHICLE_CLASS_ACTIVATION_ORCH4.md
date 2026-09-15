# Native vehicle class activation and platform cleanup

Addresses: `009598D0`, `007F6E50`. Required existing source: `00879AA0`;
shared dependency: `005471B0`; unresolved terminal: `00443490` -> `00443090`.

## Result and evidence boundary

The complete ordinary bodies of `009598D0` (112 bytes) and
`007F6E50` (297 bytes) are represented in
`native_vehicle_class_activation.cpp`. The outer body composes the existing
damageable-class model activation and the shared native vehicle pointer-array
resize implementation. The platform body exposes `00443490` as a mandatory,
address-named call interface with no default implementation.

This is complete source coverage for the two owned bodies, with one explicit
native terminal still required. It is not an actual end-to-end vehicle
activation path: `00443490` and its large class resolver `00443090` have no
source implementation, and the real model slot `+20h` binders are owned by
separate packets. No ABI bridge, native FH3 behavior, executable reachability
or gameplay result is claimed.

Live Ghidra was verified against `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, with 64,122 functions, equal to the current
snapshot. All Ghidra work was read-only.

## `009598D0`: class activation

ABI: `__thiscall`, ECX actual vehicle class, one stacked enemy flag, `RET 4`.
Coverage: complete `009598D0` through `0095993F`. The count, data and
slot reads are volatile raw accesses so the compiler retains both repeated
bounds checks and their `005471B0` branches.

1. A nonzero byte at class `+44h` returns immediately.
2. `009598DF` calls full `00879AA0` with the original class and enemy flag.
   That source loads/publishes class `+50h`, calls the current model binder when
   nonnull, and only then writes byte `+44h = 1`.
3. The signed index starts at zero and is compared with the current class
   `+98h` count before and after every iteration.
4. The two repeated bounds checks call `005471B0(index + 1)` when a callback has
   made the current count too small. Each access reloads the current `+94h`
   data pointer. The current nonnull entry is passed as ECX to `007F6E50`.

The apparently redundant bounds checks are retained because `00879AA0` and
`007F6E50` cross external call boundaries. A source projection must not cache
the pointer or count across those calls.
The index itself is a DWORD: `index + 1`, `index * 4` and increment wrap in
32 bits, while each `JL`/`JGE` decision casts those same bits to signed.

The caller owns `NativeDamageableClassModelContext` and one fresh
`NativeDamageableClassModelAcquired` frame for this invocation. If class byte
`+44h` is already set, the frame remains fresh because `00879AA0` is not called.
Once called, the existing model API determines complete/failed phase and unwind
ownership; this wrapper neither replays nor releases that frame.

## `007F6E50`: platform activation and arc cleanup

ABI: ECX actual 98h-byte platform descriptor, no stacked arguments, plain
`RET`. Coverage: complete `007F6E50` through `007F6F78`.

The first pass walks the intrusive list whose header begins at platform `+1Ch`
and whose sentinel pointer is at `+20h`. For each node, node `+8h` is the
retained device-class reference and reference `+6Ch` is passed in ECX to
`00443490`; DL is explicitly zero. The producer proves the layout:
`009614F0` reads an authored integer id from the list at platform `+10h`, calls
`00443090` with DL=1, and inserts the returned retained reference into the
platform `+1Ch` list at `00961518`.

The four `00BF6713` checks are preserved through the required
`SingletonLifetimeCallbacks` validation input. The list-owner address is
checked before each comparison with the captured termination sentinel and
again before dereferencing a nonterminal node. The current sentinel is reloaded
before and after `00443490`; a returning validation callback or the activation
call may mutate it. The captured sentinel still controls termination.

The second pass edits the firing-arc array at `+3Ch`, count `+40h`, 14h-byte
records. One bounded MSVC x86 assembly helper covers the complete
`007F6EA7..007F6F70` tail. It loads a local double constant whose exact bits
were copied from `00D08B88`, then keeps it resident on x87 across the whole
prune loop. This is not a binding to the original live cell. A nonzero flag byte
skips every FP instruction. A zero-flag record is removed only when the native
`FLD32`, `FSUB32`, `FXCH`, `FCOMI`, `FSTP ST(1)`, `JBE` schedule says the
resident threshold is ordered-greater than `max_horz - min_horz`. The threshold
bytes are `00 00 00 A0 46 DF 81 3F` (`0.008726646192371845`). Equality and
unordered values are retained.

Removal uses the listing's five ordered DWORD read/write pairs for each shifted
14h-byte record, reloads current data/count after each row, decrements count and
re-examines the shifted record. It does not substitute `memmove`. A concrete
precision discriminator is `max_horz = threshold` and `min_horz = 2^-65` under
x87 PC=64: native extended subtraction removes that zero-flag arc, while a
binary64 expression rounds the span back to threshold and keeps it.

Finally `007F6F33..007F6F70` scans bit 0 of every remaining arc and tracks each
transition in DL. The value is never published, but the raw assembly retains
every read. It starts by reading the first arc even when count is zero, exactly
as the listing does; source does not convert that reached invalid access into a
successful no-op. Constructor `007F7110` normally seeds a full-span arc.

## Call and boundary audit

| Containing function | Call site | Native | Contract and coverage |
| --- | --- | --- | --- |
| `009598D0` | `009598DF` | `00879AA0` | complete existing source; class model activate/bind/latch |
| `009598D0` | `00959900`, `00959920` | `005471B0` | complete shared source; resize actual `{data,count,capacity}` ref array |
| `009598D0` | `0095992A` | `007F6E50` | complete owned source; current nonnull platform entry |
| `007F6E50` | `007F6E6E`, `007F6E7D`, `007F6E87`, `007F6E9E` | `00BF6713` | required returning validation callback; current sentinel reloads retained |
| `007F6E50` | `007F6E94` | `00443490` | complete 42-byte callee read; required terminal, no source implementation |

`00443490` forwards ECX and DL to `00443090`. If the result is nonnull and its
byte `+44h` is zero, it invokes current vtable slot `+10h` with stacked zero,
then current slot `+14h`; otherwise it returns the result unchanged. Because
the full `00443090` factory/resolver family and concrete slot bodies remain
unsourced, the source interface is deliberately named `call_00443490` rather
than assigning it a broader semantic contract.

## Verification

The strict MSVC Win32 build passes with the exact x87 tail and volatile outer
reloads compiled into `bsp_core`. Both registered CTests pass:
`reconstructed_math` and `tool_tests`. The report call verifier checks nine
fixed native call rows with zero failures. No new test suite was added.

The parent then linked the exact source/header bytes from this packet into a
combined candidate at `2ccb5609` and ran one ignored paired original/source
fixture for **the 297-byte `007F6E50` platform body only**. The original span
hash was `b591ae4bc496e4d0d476596e4a2f63d290ac09b30d4293d5524e252dbcb94c82`;
the eight-byte `00D08B88` input hash was
`b8ae6633c93899b397f0a5e12a2a0943adf7639f4ea8b834a72c38bbcedd7a4f`.
Both spans matched live Ghidra and disk.

Original and source outcomes were identical: remaining arc count 3, one
fixture device call, one returning validator call, repaired sentinel true,
x87 status 0 and control word 895 (`037Fh`). Under precision-control 64, the
`max=threshold, min=2^-65` zero-flag arc was removed. A flagged signaling-NaN
payload was retained without setting FP invalid, and an exact-threshold arc was
retained. The source and header pins were respectively
`2ee253a22479097d11660ec79ab8d14bf567029206b826d0939a4b87820e6eeb` and
`829d5f01a957abc119264570506ec2fc6a1213ccfff1f406cd9c5a0982a5da7a`.
The fixture linked the actual `bsp_core.lib` and pinned `bsp_zlib121.lib`;
the device call and returning validator were fixture-provided boundaries.

The combined strict Win32 build and all three registered CTests passed,
including `native_math_differential`. This does not dynamically compare the
112-byte `009598D0` wrapper, exercise real `00443490`/`00443090`, reach the
model-admission path or validate gameplay.

## Continuation scope

A follow-up device-class packet should own `00443090` and `00443490`, audit all
call sites for the DL selector, enumerate every construction branch and its
current slots `+10h`/`+14h`, and produce a real terminal for
`NativeVehicleEntryActivationCalls`. It must verify the resolver's Lua/global
dependencies before making an end-to-end activation claim. The base and ship
model binders (`0095F500`, `0082FE30`) remain separate ownership.

Evidence report: `reports/native_vehicle_class_activation_orch4.json`.
