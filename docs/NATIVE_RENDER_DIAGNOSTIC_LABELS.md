# Actual diagnostic command labels

`B13030` and `B13510` are implemented as complete bodies over the diagnostic service's actual eight-byte pooled-string header at `+684`. The supplied service/global slot and `NativeStringStorage` are borrowed; this does not construct the service, initialize its singleton, or implement its statistics/system-time responsibilities. The source base is `3c62b2261e42b11f5bbd8e66a7481ae684d5a549`, with discovery pinned at `4b51af45dcc71b97fb39cb27d76ebfa43e7a3066`.

The API is in [native_render_diagnostic_labels.hpp](../include/bsp/native_render_diagnostic_labels.hpp) and implementation in [native_render_diagnostic_labels.cpp](../src/native_render_diagnostic_labels.cpp). These are new C++ interfaces, not drop-in original register/stack replacements. Neither entry is `noexcept`; allocation exceptions propagate with native preceding effects retained.

| Original entry | Original ABI | Implemented body |
|---|---|---|
| `B13030`, `[B13030,B13069)`, 57 bytes | `ECX=actual service`, stack actual source-header pointer, `RET4` | Exact header self-copy skip; actual destination resize; current source/destination field reads and copy |
| `B13510`, `[B13510,B135B5)`, 165 bytes | No inputs, `ECX` unused, `RET` | Initial global gate, literal temporary construction, current global reload, captured copy/normal release, guarded current-header exception cleanup |

## Setter ordering

`set_native_render_diagnostic_label_00b13030(actual_service, actual_source_header, storage)` computes destination `service+684` using Win32 pointer arithmetic. Identical header addresses return before any header read or storage call. Otherwise it reads source length and calls `resize_native_string_header_0041dd40(destination, storage, length, true)`.

After resize, it rereads source length for the nonzero gate, then reads current destination length, source data and destination data in that order. It copies **destination's current length** bytes, without copying a terminator. It does not substitute the originally requested length, capture source data before allocation, or replace the actual headers with `std::string`. There is no setter cleanup guard or rollback.

## Reset ordering and cleanup

`reset_native_render_diagnostic_label_00b13510(actual_global_00f8d39c, storage)` borrows a `void* const volatile&` to the actual publication slot. Initially null returns. Otherwise it constructs one fresh actual eight-byte `NativeString` through `assign_0041e870` from fixed bytes `58 00` (`CE9A38`, `X`). The constructor runs before the reset's guard is armed.

Only after construction does the function reload the global service, then capture temporary data and length. It forms destination `service+684`, compares that header with the temporary, and arms cleanup around the following copy branch (`B1355C`). The branch resizes using **captured temporary length**; if that captured length is nonzero, it copies current destination length bytes from **captured temporary data**. The captured destination is retained if a later allocation changes the global. There is no second native null-global check after construction.

Normal cleanup disarms first (`B13586`) and releases **captured data with captured length+1**, with DWORD wrap, leaving the temporary header untouched. Exception cleanup instead calls `destroy_native_string_header_0041dd20(&temporary, storage)` and rethrows: it reads the temporary header's **current** pointer and length. Evidence is handler `CBC278`, EH info `DF4360`, state-0 map at `DF4358` with successor -1/action `CBC270`, and `CBC270: LEA ECX,[EBP-14]; JMP 41DD20`. A construction failure has no reset-temporary cleanup action. Normal release is outside the guard and is not retried.

`NativeString` has no automatic buffer cleanup. A compile-time trivial-destructor assertion prevents this use from silently acquiring an additional destructor release later. The explicit copy-only `try` region provides no cleanup before the native arm point and no second release on normal completion. Existing `NativeStringStorage::release` and actual `41DD20` are `noexcept`; that host boundary cannot represent a native pool release throwing. This restriction is recorded separately from the observed native disarming.

## Native evidence and validation

All live queries used the guarded `bsp.py ghidra` client, verifying project `bsp`, `C:/Users/sqz269/bsp.gpr`, and `/battlestationspacific.exe`. **12 full-function/EH/literal/hook-preimage spans, 582 bytes, match the installed executable.** Its SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Complete prior names/comments, source hashes, native ranges and fixture artifact hashes are in the [audit](../reports/native_render_diagnostic_labels_audit.json). No Ghidra metadata or bytes were changed.

`python tools/ghidra_export.py verify-seeds` passed the existing eight seed comparisons. `./scripts/build.ps1` passed the registered Win32 build and both existing checks (`reconstructed_math`, `native_math_differential`). The new module is not yet registered in shared CMake; it was separately compiled and linked by the private fixture with **MSVC Win32 `/std:c++17 /EHsc /fp:strict /O2 /Oy- /MD /W4 /WX`**. Thus the existing build is a baseline check, while the private compile verifies this new source.

One ignored original-caller fixture passed **eight focused scenarios and 7,096 normalized words**. It compares ordered storage calls/sizes, complete guarded service storage, headers, global identity, source buffers and allocated buffers. Scenarios cover setter header aliasing; allocation callbacks replacing source length/data; a current source length becoming zero; initially null reset; global replacement during temporary and destination allocation; allocation failure before the reset guard, inside its guarded resize, and in the unguarded setter. Explicit expected-result assertions accompany original-versus-port comparison. There were **13 native pool-getter boundaries**. The checked original handler searched and unwound once with state 0 and once with state -1; only state 0 released the temporary.

The fixture executes complete original setter/reset/constructor/resize/destructor bodies in a private sparse image. It relocates the two global operands, literal pointer, EH handler/info/map/action pointers, and connects the original handler to the host `__CxxFrameHandler3` through a registration trampoline in the fixture executable. Native `419CC0`, `BD1120`, `BD1510` and `BF7680` hook sites are checked against installed bytes before being connected to the same fixture storage/copy boundaries used by the port. `/SAFESEH:NO` is fixture-only. The real game allocator and complete historical CRT runtime are not exercised.

Two zero-byte original memcpy calls were omitted at the existing actual-string boundary. The production functions likewise omit zero-byte copy while retaining the preceding field reads. Reset self-aliasing and an externally modified temporary header were not injected into the production code: their branches and captured-versus-current cleanup distinction are established by native assembly and source review. No extra fixture hook was added to the public API. Null becoming published during construction, invalid pointers, concurrent mutations, process faults, throwing pool release, binary replacement, and game/render behavior were not tested or claimed.

## Reproduction and integration

The ignored fixture remains in `J:/PROG/battlestations-pacific-decompile-native-render-diagnostic-labels-20260910/local/`. Run there after obtaining the four-file implementation:

```powershell
python local/prepare_native_render_diagnostic_labels.py
python tools/ghidra_export.py verify-seeds
./scripts/build.ps1
./local/build_native_render_diagnostic_labels_check.ps1
```

The preparation script emits `native_render_diagnostic_labels_spans.hpp` and a range audit; the driver compiles `native_render_diagnostic_labels_check.cpp`, this new source, the existing `native_string.cpp`, and the registered core library. The audit records absolute artifact paths and hashes so the integrator can reproduce the same private fixture without adding it to the repository's test suite.

Only the dedicated header, source, this document and audit are committed. The primary integrator owns CMake registration and later name/reconstruction records. Suggested descriptive names are `BSP_RenderDiagnostics_SetCommandLabel` and `BSP_RenderDiagnostics_ResetCommandLabel`; preserve the recorded provisional names/comments before applying any annotation. The full queue/command execution chain and diagnostic service remain separate work.
