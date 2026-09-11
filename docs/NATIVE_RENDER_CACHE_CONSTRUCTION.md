# Native render-cache construction

`B27E80` and `B29430` are implemented over actual borrowed cache bytes. The full constructor composes the existing `B659D0` plane helper with the **proved all-null construction path** of `B241C0`. General nonnull cache release is not implemented by this packet. Constructor closure is conditional on ordinary unpublished storage and the existing plane helper's finite-CRT contract; it does not extend that helper's exceptional-input or returning-diagnostic behavior.

Source base: `4973646` (full commit in the [audit](../reports/native_render_cache_construction_audit.json)). The discovery input is `reports/native_renderer_stop_pipeline_next.json`, specifically `cache_construction` and the `B241C0` contract. Public interfaces are in [native_render_cache_construction.hpp](../include/bsp/native_render_cache_construction.hpp); implementation is [native_render_cache_construction.cpp](../src/native_render_cache_construction.cpp). There is no cache allocator, full renderer class, terminal callback provider or public general reset interface.

| Entry | Native ABI and span | Scope |
|---|---|---|
| `B27E80`, `[B27E80,B27F11)`, 145 bytes | ECX first bank; EAX returns same address; RET | Full twenty-bank constructor in unpublished storage |
| `B29430`, `[B29430,B295B6)`, 390 bytes | ECX cache; EAX returns same address; RET | Full cache constructor, inheriting the existing plane helper contract |
| `B241C0`, `[B241C0,B24458)`, 664 bytes inspected | ECX cache; RET | Private control-flow fragment: all 26 reset-owned reference cells are null; nonnull release edges excluded |

These are new C++ interfaces rather than original register/stack ABI replacements. `B27E80` is nonthrowing in its proved domain. `B29430` does not add an exception guard or `noexcept` to the existing plane-helper call.

## Actual storage and lifetime

The bank helper touches twenty records of stride `ACh`, for a minimum span `D70h`. In the complete cache they start at `+428`, ending immediately before twenty `48h` records at `+1198`. The complete constructor requires a minimum **`193Ch`** cache span. Its actual `CameraPlaneSet` is `144h` bytes at `+178C`; gamma is the separate DWORD at `+1938`. These are minimum touched extents and an embedded type, not a recovered complete cache class definition.

Before calling the new C++ constructor, its embedded `CameraPlaneSet` must have a real C++ lifetime without changing the intended native entry preimage. For example, a caller preparing an unused aligned raw slot can use the same save/placement/restore pattern as `NativeCameraOwner`:

```cpp
// actual_cache already denotes an unused, aligned span of at least 0x193C.
auto* plane_address = static_cast<std::byte*>(actual_cache) + 0x178C;
std::array<std::byte, sizeof(bsp::CameraPlaneSet)> saved;
std::memcpy(saved.data(), plane_address, saved.size());
::new (plane_address) bsp::CameraPlaneSet;
std::memcpy(plane_address, saved.data(), saved.size());
// All intended native preimage bytes are restored before this entry.
bsp::construct_native_render_cache_00b29430(actual_cache, live_one_00d7a24c);
```

This host preparation occurs **outside** `B29430`. The implementation itself neither default-initializes a replacement plane set nor inserts extra initialization stores before the native plane call. The raw slot and actual live constant must remain valid, unpublished and free from outside writes throughout construction. Arbitrary old bit patterns in an unused slot are allowed; they are not live owned references requiring destruction.

## Store order and the null-reference proof

`B27E80` loops twenty times. For each bank it writes owner `+A8=0` **first**, then eight DWORDs `+00..+1C=0`, then byte `+20=0`. Bytes `+21..+A7` are preserved. The original then reads `+A8` and conditionally decrements/releases it. That read must observe zero in the supported domain: every intervening write is disjoint from `+A8`, with no intervening call. The unreachable release branch needs no terminal implementation or success stub. The original's unused decrement-IAT load is not an ownership operation.

`B29430` writes first-owner cells `+00/+04/+08=0`, calls the bank helper on `+428`, then initializes twenty records at `+1198` with three DWORD stores at `+00/+04/+08` and one WORD store at `+0C` each. It preserves each record's `+0E..+47` gap. It calls the existing plane constructor on the **actual** `+178C` object with the same live `D7A24C` binding.

After the plane call, it writes gamma `+1938=positive zero`, DWORDs `+1738/+173C`, owner cells `+1780/+1784`, then each of four streams at `+1740+i*10h` (owner, `+04`, `+08`; the stream's `+0C` is not initialized until reset). It writes target `+18D4`, then `+18D0`, then the 24 tail words in native order: `18D8,18E8,18DC,18E0,18E4,18EC..1934`. Only then does it invoke the private null-reset fragment.

Exactly **26** cells are read by `B241C0` release branches: three initial owners; layout/index `1780/1784`; four stream owners; sixteen bank owners `4D0+i*ACh`; and target `18D4`. The constructor initializes **30** because it prepares all twenty bank owners. The last four (`F90,103C,10E8,1194`) are not visited by reset. These are distinct addresses; the record area and plane set do not overlap any of them.

The plane helper writes only `178C..18CF`. Its supported implementation has no returning user/provider/diagnostic callback and delegates to the established finite nonnegative square-root core. The inspected native `D7A24C` word is `3F800000` (1.0); existing helper evidence and this fixture also cover stable 2.0. Exceptional native CRT diagnostics remain outside that contract. A future supported returning diagnostic path capable of mutating owner cells would require this proof to be reviewed again; this packet does not assume such a path is harmless.

## Private `B241C0` fragment

All null reference branches skip both release and owner-slot writes. The sole production caller proves this precondition, so the private fragment contains only the reached write schedule:

1. `memset(cache+0C,0,D2h)`; zero DWORDs `1738/173C`; write `1788=FFFFFFFF`.
2. For four streams, zero DWORDs `+04/+08/+0C`, leaving the already-null owner untouched.
3. For **sixteen** banks, zero eight DWORDs `+00..+1C` and byte `+20`, leaving owner `+A8` untouched.
4. Initialize the twenty `48h` records with the same three-DWORD/one-WORD schedule.
5. Write the 24 positive-zero tail words in the native `18D8,18E8,18DC,18E0,18E4,18EC..1934` order.

Gamma `1938`, plane storage `178C..18CF`, DWORD `18D0`, bank/record gaps and bytes outside the exact flag memset remain untouched by reset. There is no whole-cache memset, twenty-owner release loop or semantic `D3D9StateCache::invalidate` substitution. Nonnull `B241C0` still requires actual refcounts, current terminal profiles, callback rereads and exception behavior; no general release claim is made here.

## Evidence and validation

Guarded live Ghidra queries verified `C:/Users/sqz269/bsp.gpr`, project `bsp`, program `/battlestationspacific.exe` before each batch/read. **20 spans, 2,446 bytes**, match the installed PE or its declared zero-fill. These include the complete three cache listings, existing plane/extraction/normalization and finite CRT instruction ranges, data words and the memset hook preimage. Binary SHA-256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Existing eight math-seed byte comparisons passed. Prior names/comments and all range/source/artifact hashes are preserved in the audit. No Ghidra mutation occurred.

`./scripts/build.ps1` passed the registered Win32 build and existing `reconstructed_math` plus `native_math_differential` checks, **2/2**. Shared CMake is unchanged, so the new source was separately compiled and linked with **MSVC Win32 `/std:c++17 /EHsc /fp:strict /O2 /Oy- /MD /Gy /Gw /W4 /WX`** in one ignored original-caller fixture.

The fixture passed four checkpoints: bank construction on poisoned preimage; complete construction with live 1.0; private all-null reset with poisoned gaps/planes, distinct gamma/18D0, and four nonzero extra-bank owner cells; and complete construction with live 2.0. All native return pointers matched. **6,508 snapshot words and 1,866 ordered store records matched**. The snapshots include the complete minimum cache, canaries, x87 status/control, MXCSR and live constant. The write observer uses private-page write protection and one-instruction stepping; it records cache offsets and resulting words, including writes whose values do not change. The 210-byte memset range is normalized into one bulk record because its internal library store widths are not a cache-function contract. Bank construction produced exactly 200 stores, with each owner store first.

Both public production constructors are compiled as their own source translation unit. A second fixture-only translation unit includes a renamed copy solely to reach the private reset slice; no private reset interface or test hook is exposed in production. Native cache/plane/math calls are not replaced: the fixture executes the original finite CRT square-root route with the installed zero-filled CRT mode retained. Standard memset is the only replaced callable. The original decrement IAT is read but never called under the proved null paths; no terminal provider exists in the fixture. No general nonnull cleanup, exceptional CRT path, arbitrary floating-point environment, concurrent mutation, binary replacement or game/render validation is claimed.

Reproduce in `J:/PROG/battlestations-pacific-decompile-native-render-cache-construction-20260910`:

```powershell
python local/prepare_native_render_cache.py
python tools/ghidra_export.py verify-seeds
./scripts/build.ps1
./local/build_native_render_cache_check.ps1
```

Only this document, its audit and the dedicated header/source are committed. Primary integration owns CMake, Ghidra annotations and ledgers. Suggested descriptive names are `BSP_RenderCache_ConstructBanks` and `BSP_RenderCache_Construct`; preserve the recorded previous names/comments. Record `B241C0` solely as the private construction-path fragment.
