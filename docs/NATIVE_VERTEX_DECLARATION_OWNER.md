# Native CPU vertex declaration owner

Nine complete native bodies, **790 bytes**, are reconstructed in
`src/native_vertex_declaration_owner.cpp`. They operate on the actual declaration
and array storage and return raw slots to a borrowed initialized pool. This CPU
declaration supplies element/stride information; it does not own a COM declaration.

| Original | End exclusive | Bytes | Original ABI and reconstructed behavior |
| --- | --- | ---: | --- |
| `00B48AF0` | `00B48B68` | 120 | ECX raw owner, EAX same address, RET. Initialize base/derived profiles, reference word, main header and 15 usage headers, then stride. |
| `00B47910` | `00B4791D` | 13 | ECX header, EAX same address, RET. Write data/count/capacity zero in order. |
| `00B488E0` | `00B4894F` | 111 | ECX owner, RET. Resize main then 15 usage arrays to 0, zero stride, then run the live-count/type-table pass. |
| `00B48AD0` | `00B48AE7` | 23 | ECX header, RET. Resize0, then unconditionally free the current data, preserving pointer/capacity. |
| `00B480F0` | `00B48161` | 113 | ECX header, signed count on stack, RET4. Reserve if needed, initialize exposed 20-byte records, decrement shrinking count, publish requested count. |
| `00B47A30` | `00B47ABE` | 142 | ECX header, signed capacity on stack, RET4. Clamp request to 1, grow only, copy raw records, free old data, then publish replacement/capacity. |
| `00B48B70` | `00B48BF4` | 132 | ECX owner, RET. Clear, destroy 15 usage arrays in reverse, destroy main, restore base profile. |
| `00B48CA0` | `00B48CC0` | 32 | ECX owner, stack flags, EAX original address, RET4. Destroy, then return slot to canonical pool iff flags bit0 is set. |
| `00B47950` | `00B479B8` | 104 | ECX pool, stack slot, RET4. Enter real lock, append the raw slot index, update first-free slab, leave lock. |

The new C++ interfaces take raw actual storage. They do not install callable
native vtables or binary replacements. Descriptive names are reconstruction
hypotheses; the address, field order and observed effects are the evidence.

## Actual fields and array operations

The owner occupies exactly `D0h` bytes: profile at `+00`, reference word at `+04`,
scalar at `+08`, main array at `+0C`, fifteen usage-array headers at `+18+i*0C`,
and stride at `+CC`. Each header is `{data, signed count, signed capacity}`.
The constructor writes base profile `CEB130`, reference 1, derived profile
`D61D1C`, scalar 0, the 16 zero headers, then stride 0. The pool's trailing slab
index at `+D0` survives construction and destruction.

Reserve calls the existing shared `singleton_lifetime_allocate/free` boundaries
corresponding to `BF55BE/BF6989`. The 20-byte product and pointer offsets use
DWORD wrap; comparisons remain signed. Each row copies five DWORDs in order,
with a live source data pointer and count. A computed null destination skips
that row's stores. Free receives the current old data even when null; only after
it returns are the captured replacement and requested capacity published. The
count remains whatever the actual header then contains. No rollback is added.

Resize captures the post-reserve old count and growth distance, reloads data
for each new row, and writes offsets **`+04=11h`, `+08=0`, `+0C=0Eh`,
`+00=FFFFFFFFh`, `+10=FFFFFFFFh` in that order**. Shrink decrements the actual
count until the requested signed count is reached; the final store publishes
that request. Raw records have no element destructor or retain/release action.

Clear calls resize0 on the main header and each usage header before zeroing
stride. It then observes the current main count. A returning allocation handler
can change that count while a later header is cleared, so the stride loop is
retained. It captures the main record cursor once, reloads the count each
iteration, and indexes the borrowed actual `D61CC0` type-size table with native
DWORD arithmetic. Unknown or negative type words receive no new validation.
All addresses actually accessed must be readable/writable in the supplied state.

## Cleanup and raw pool return

Constructor C++ EH states1->0 destroy the main header then restore the base;
the original constructor iterator first destroys any initialized usage prefix
in reverse. The current concrete header initializer has no throwing operation
on valid ordinary storage.

Destructor states2->1->0 mean usage arrays, main array, base. State2 covers
clear; state1 begins before the reverse usage destructor iterator; state0 begins
before main cleanup. The original iterator decrements its remaining count before
calling each element destructor. If that call throws, its cleanup skips the
failed element and destroys only the remaining lower indices. The reconstruction
preserves this sequence. Private unwind helpers terminate on a second C++
cleanup exception, matching the native `BF7C10` C++ exception filter. No public
destructor or scalar-delete API adds `noexcept`.

`B47950` borrows the already initialized actual `0108FD38` pool. Its real Win32
`CRITICAL_SECTION` is at `+0C`, recursion word at `+24`, slab table at `+28`, and
first-free index at `+34`. It reads the slot's live `+D0` slab index after entering
the lock. A slab contains 32 `D4h` slots, the WORD free-index stack at `1A80h`, and
its WORD count at `1AC0h` (`1AC4h` bytes total). The slot index uses signed
division of the wrapped address difference by `D4h`, then narrows to WORD.
The count is reloaded after the index store before incrementing. First-free
selection is unsigned. There is no replacement pool, new lock, implicit pool
initialization, allocator-list registration or automatic unlock policy.

## Verification

The [audit](../reports/native_vertex_declaration_owner_audit.json) pins the
source, all nine complete spans, prior annotations, original EH metadata and
private reproduction artifacts. **26 live Ghidra/installed-PE spans totaling
1,587 bytes matched**, including the 790 owned bytes. Every live read first
verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`.

One private MSVC Win32 fixture compiled the actual production source separately
with `/std:c++17 /EHsc /fp:strict /O2 /W4 /WX`. It compared original native callers
with rebuilt calls in six focused checkpoints, matching **43,032 normalized
words** covering callbacks, full owner state, raw array records and pool state:

- Construction, default rows, clear, reverse destruction, flags2/flags1 and two
  exact slot returns; one return executes under an existing recursive Win32 lock.
- Signed shrink/growth, min1 reserve behavior and the wrapped20-byte allocation
  product for capacity `40000001h`.
- A returning allocation changes the previously cleared main count. The stride
  pass uses live data, including raw type `FFFFFFFFh`, which reads the captured
  word immediately before `D61CC0` instead of sanitizing the index.
- Allocation exceptions at destructor states2,1 and 0. Each original owner state
  received one C++ search and unwind; the original reverse iterator received one
  SEH4 search and unwind and skipped the failed element correctly. No slot was
  returned after a failed destructor.

The original `BF7C10/BF7C6E/BF7CD1` iterator bodies, SEH4 prolog/epilog and
cleanup actions execute in the private mapped image. Its 28 explicit address
relocations adapt absolute pointers. Personality transfers use actual host
`__CxxFrameHandler3` and `_except_handler4_common`; the original prolog uses the
host security cookie and real cookie-check function. The two Win32 IAT cells
call real lock functions. Existing allocation/free boundaries are controlled
only inside the fixture to record/mutate state and raise one allocation failure;
production continues to use the repository's existing allocation domain.
`/SAFESEH:NO` applies only to this private mapped-code executable.

The eight existing native seed comparisons, registered Win32 build and both
existing CTests passed. The source is now registered in the main CMake target.
Primary integration repeated the same 43,032-word comparison against the main
library after verifying the 26 native spans, source pins and 13 artifacts.
Eight standalone definitions are identified in the link map; the initializer
is inlined into the constructor. Its standalone return value is source-reviewed
rather than independently compared. No tracked tests were added. The original
private comparison can be reproduced in the worker checkout:

```powershell
python local/prepare_native_vertex_declaration_owner.py
./local/build_native_vertex_declaration_owner_check.ps1
python tools/ghidra_export.py verify-seeds
./scripts/build.ps1
```

Primary integration created `B47910..B4791C`, extended the complete `B48AD0`
and `B48B70` bodies to the exclusive ends above, and decoded the returning-free
gap `B47AAF..B47AB8`. Prior comments were preserved, all nine functions were
annotated and saved, and their exports were refreshed.
This proof covers the stated raw-storage and C++ exception contracts. It does
not establish full pool initialization, original CRT-runtime identity, concurrent
mutation behavior, hardware-fault unwinding, secondary-exception termination at
runtime, a callable whole resource hierarchy or gameplay validation.

## Cleanup search follow-up

The complete owner constructor/destructor now use armed cleanup guards instead
of synthetic catch/rethrow maps. Their native FH3 metadata has cleanup actions
but no catches. The array constructor/destructor iterators use scalar
`__try/__finally` with completed/remaining state; unwind-only calls terminate
on a second C++ exception during SEH search. This preserves the native SEH4
array counter behavior as well as cleanup order.

The existing nine-entry fixture was linked to the corrected primary library
and again matches 43,032 words across six checkpoints. Seven standalone map
providers are present; the trivial initializer and owner destructor are
optimized within the constructor/scalar-delete translation unit. The hardware
fixture additionally matches both nested-exception terminal snapshots, but its
array pool-entry failure occurs after the CPU declaration destructor returns,
so that case does not establish a failure inside CPU array clear. Original
static CRT filter code is byte-audited; host CRT adapters execute. The exact
source pins, build, maps and boundaries are in
[the cleanup audit](../reports/native_cleanup_search_fidelity.json).
