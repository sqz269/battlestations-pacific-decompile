# Raw particle type base lifetime

The six routines in `native_particle_type_lifetime.cpp` connect the actual 80h
particle type image to the application's existing string and parameter pools.
The older explicit host-binding overloads remain available. This context borrows
the actual string publication/gate/manager cells and the same F8D344 parameter
pool used by parameter producers; it creates no replacement storage or callback.

| Address | Complete bytes | Native interface | Behavior |
|---|---:|---|---|
| AFFDF0 | 42 | ECX parameter; RET | Free nonnull captured +4 only for current WORD+A type 1/2, then clear current +4. |
| B00090 | 12 | ECX parameter; RET | Return the actual slot through 924420 into the F8D344 pool. |
| B00C20 | 133 | ECX descriptor, stack signed capacity; RET4 | Minimum one, signed capacity check, forward seven-DWORD copies, current count/source reads, fixed CRT storage. |
| B00F70 | 61 | ECX descriptor; RET | Reserve for negative capacity, reduce count, capture data before final count-zero store, free; leave pointer/capacity stale. |
| B00FB0 | 384 | ECX actual 80h type; RET | Ordered parameter, descriptor, raw name and reference-base cleanup. |
| B01130 | 30 | ECX owner, stack flags; RET4/EAX owner | Run the base destructor, free iff bit zero is set. |

The base stamps D5DDC0 and releases offsets 1C, 2C, 30, 48, 34, 38, 3C,
40, 44, 5C in that order. It clears only 48, and only after a nonnull return.
Its normal inlined descriptor cleanup stores count zero before capturing data;
the genuine B00F70 unwind body captures data first. The two schedules remain
separate. No invalid-input checks, capacity normalization, or rollback is added.

## Exception evidence

The complete handler/funclet span CBB420..CBB447 and FuncInfo/map
DF3468..DF34A3 match the installed image. Handler CBB43E uses DF3480.
State 2 invokes CBB433 -> B00F70(owner+68), state 1 invokes CBB428 ->
41DD20(owner+8), and state 0 invokes CBB420 -> BD30F0(owner). The C++ guard
is `noexcept`: a second exception during unwind terminates. Normal state changes
occur before the corresponding potentially throwing cleanup, exactly as listed.

## Evidence and validation

`reports/native_particle_type_lifetime_raw_orch4.json` retains complete byte
hashes, pre-edit Ghidra documentation, earlier reconstruction records, original
ABI, direct call sites, probe receipt and limitations. B00F70 had a returning-free
flow override that omitted its final five bytes. After whole-body live/PE
comparison, the override was removed and the function recreated through its
actual RET at B00FAC. The shared repair reports retain the old body and mutation
receipts; all affected exports were forced to refresh.

The strict Win32 build and all three existing CTests pass. One ignored local
probe runs all six complete copied original bodies (662 bytes), redirecting
external calls to the genuine shared pool/base/CRT providers. Internal calls use
the copied original bodies. Source and original runs check all ten parameter
returns in order, payload types 1/2, complete 80h owner images, actual string-ring
reuse, descriptor copy/growth and stale fields, negative capacity, scalar flags
2/3 and owner returns. An independent assembly review found no discrepancy in
the normal or unwind state/capture schedules.

These are source interfaces with an explicit lifetime context, not binary
replacements. The copied-original probe does not execute native FH3 exceptions,
SEH, allocation failure or invalid-memory behavior. Unwind behavior is supported
by the complete state map and listing, not original exception execution.
Renderer ownership, emitter definition destruction and gameplay remain separate.
