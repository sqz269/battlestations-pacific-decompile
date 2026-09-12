# Native pooled text suffix helpers

Addresses: `00AF4450`, `00AF44C0`.

These two complete bodies extend the actual four-byte pooled text owner from
`native_pooled_text.hpp`. They use the application's same `NativeStringStorage`
for every allocation and release. This owner is not the eight-byte NativeString.
Descriptive names below are hypotheses, not recovered symbols. The exported C++
interfaces add an explicit storage argument and are not binary replacements.

| Routine | Inclusive body | Final instruction | Original ABI | Coverage |
|---|---|---|---|---|
| `assign_native_pooled_text_suffix_00af4450` | `00AF4450..00AF44BE` | `00AF44BC: RET 4` | ECX header; stack text; EAX count | complete |
| `get_native_pooled_text_suffix_00af44c0` | `00AF44C0..00AF45CF` | `00AF45CE: JMP 00AF457D` | ECX line; stack output/index; EAX output; RET 8 at AF4512/AF4590 | complete |

The live Ghidra signatures still say `undefined FUN_...(void)`. Proposed annotation
prototypes omit the implicit ECX argument:

```cpp
uint __thiscall BSP_NativePooledText_AssignSuffix(char * text);
void * __thiscall BSP_NativePooledText_GetSuffix(void * output, int index);
```

No Ghidra names, signatures, comments, function bodies or saved analysis were
changed by this worker. There are no missing instructions in either body.

AF4450 scans first, accepting TAB or signed bytes `20h..7Eh`. Null input or a
zero-length accepted prefix returns zero without changing an existing header.
Otherwise it releases the captured old pointer using its current `strlen+1`,
allocates scanned-count+1, publishes the new pointer, copies exactly the scanned
count through genuine CRT `strncpy`, reloads the header pointer, and writes NUL.
The count is not recomputed if a storage callback changes the source. The CRT
therefore pads with NUL when the input becomes shorter after scanning.

AF44C0 captures the line pointer in ESI before scanning. Its search accepts only
signed `20h..7Eh`, increments its token counter once per space run, and compares
the requested index before skipping spaces. After selecting a start, AF4450
copies the remaining accepted suffix, including TAB. Null input, no selection,
negative indices and an initially rejected byte construct a null output.

| Input | Index 0 | Index 1 | Index 2 |
|---|---|---|---|
| `A B C` | `A B C` | `B C` | `C` |
| `  A B` | `  A B` | `A B` | `B` |
| `A B\tC D` | `A B\tC D` | `B\tC D` | null |
| `A   ` | `A   ` | null | null |
| `\tA B` | null | null | null |

Output is constructor storage: AF44C0 does not release an old output pointer.
A successful selection allocates its temporary, copy-constructs output through
AEE2E0, then destroys the temporary through AEE2A0. The line and output headers
may alias; the line pointer was already captured. Inherited AEE2E0 publication
and source reload order is retained. The C++ catch cleans a temporary when a
throwing host allocator fails; original MSVC exception dispatch is not ported.

The original BF9280 library body uses source alignment and DWORD copies. It
remains the genuine `_strncpy` dependency, not reconstructed project code. Source
and destination byte-range overlap is outside the host CRT provider contract;
it must not be confused with supported line/output header aliasing or use of
the same storage pool. No memmove or copy-through-temporary repair is added.

| Containing function | Call site | Callee and checked contract |
|---|---|---|
| AF4450 | AF448D | AEE1E0: release bytes with current strlen+1 |
| AF4450 | AF4492 | 419CC0: pool singleton, no arguments |
| AF4450 | AF449F | BD1120: pool allocation, count+1 and 1, RET 8 |
| AF4450 | AF44A9 | BF9280: CRT strncpy, three stack arguments; ADD ESP,0Ch at AF44B0 |
| AF44C0 | AF44FC | AEE2E0: null-input output construction, RET 4 |
| AF44C0 | AF4578 | AEE2E0: missing-index output construction, RET 4 |
| AF44C0 | AF45A3 | AF4450: selected-suffix assignment to temporary, RET 4 |
| AF44C0 | AF45B3 | AEE2E0: selected output construction, RET 4 |
| AF44C0 | AF45C9 | AEE2A0: temporary destruction, RET |

All 39 direct incoming AF44C0 sites in 11 containing functions were inspected.
They request indices 1 or 2, including index 1 on an earlier suffix returned in
EAX. Five index-2 pushes precede conditional branches rather than the adjacent
linear block: AF506E, AFAE23, AF8D53, B066B2 and B0B064. The report carries every
incoming address/native row and its owning function. AF4450 has one direct caller,
AF45A3 inside AF44C0; its ECX is the zero-initialized four-byte stack temporary.

Verification uses `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; every live
query verifies that pair. Both complete bodies and the four original dependencies
used in the scratch probe match the installed executable at
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
The report records the installed executable and per-body SHA-256 values.

The optimized MSVC Win32 scratch build passed `/O2 /MD /W4 /WX /fp:strict` and
embedded a manifest. Its executable exited 0 after matching 386 observations and
449 pool events between C++ and six actual original bodies: both suffix helpers,
AEE1E0/AEE2A0/AEE2E0, and original BF9280. Only pool allocation/release/singleton
calls were redirected to returning hooks. The probe covers all 256 first-byte
values, null/empty input, requested indices -1..4, space runs, tabs/control/high
bytes, source alignment and post-scan NUL padding, release/allocate callback
changes, line/output aliasing, and copy-before-temporary-release order. A separate
C++ second-allocation failure verified temporary cleanup. Native exception
dispatch, native allocation failure, overlapping CRT byte ranges and gameplay
remain unvalidated. No game installation was modified.

Scratch evidence and repeatable probe sources are in
`C:/Users/sqz269/bsp-ap-suffix`. `scripts/build.ps1` validates the existing worktree
target; this worker's new source is intentionally not yet registered with CMake.
Its separate strict object/probe compile is the evidence for the new C++ code.
After `verify-seeds`, the baseline Win32 build and both existing tests passed:
`reconstructed_math` and `native_math_differential`. Integration registration and
the combined build belong to the primary agent.
