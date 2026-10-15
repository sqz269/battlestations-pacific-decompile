# Actual native string and scalar-key ordering

This packet adds complete ordinary Source for the two raw storage contracts
below. `00443CB0` had no indexed Source; `0076EAA0` had a semantic projection
whose reversed string comparison required `MpSeg43Hooks`. The new entry points
use the existing concrete actual-header predicate from
`native_vfs_date_leaf_providers.cpp`. That provider and the legacy semantic
Source remain unchanged.

| Routine | Native range | Coverage | Original ABI | New C++ symbol |
| --- | --- | --- | --- | --- |
| `00443CB0` | `[00443CB0,00443CFD)` | complete | ECX left header; right header stack; RET4; AL boolean, upper EAX not normalized | `less_native_string_headers_00443cb0` |
| `0076EAA0` | `[0076EAA0,0076EAF7)` | complete | ECX left record; EDX right record; RET; EAX0/1; ESI/EDI preserved | `less_native_string_scalar_keys_0076eaa0` |

`00443CB0` tests the recorded length at header+0. Length zero is empty regardless
of data+4; empty sorts before nonempty. When both lengths are nonzero, it reads
right data then left data, performs the original CRT comparison, and returns
whether its signed result is negative. There is no length-bounded byte compare,
data-pointer null ordering, or string-header validation. The prior name evidence
described null-data ordering; assembly corrects that description. The native
name is retained as a descriptive hypothesis with appended evidence.

The complete assembly of accepted `00443D00` was compared with `00443CB0`.
Their predicate and read order agree after their different native argument
setup (`RET8` versus `RET4`). Source therefore forwards to the existing complete
`less_native_string_headers_00443d00` implementation. It does not reconstruct or
duplicate that accepted provider again.

`0076EAA0` first inlines the forward prefix predicate. If that is not less,
the native setup for `0076EACF` places right in ECX and pushes left. If this current
reverse comparison is also not less, it reads left+Ch then right+Ch and compares
unsigned. Only on equality does it read left+8h then right+8h and compare
unsigned. The new Source preserves both predicate evaluations, current raw
header reads, and the scalar read order, with no record copies or cached string
result across comparisons. The prefix uses the established native8h length/data
layout. The suffix is described only by its observed low/high words: no caller
or producer evidence establishes a stronger LAN/host meaning for this record.

| Containing function | Site | Native callee | Contract |
| --- | --- | --- | --- |
| `00443CB0` | `00443CE9` | `00BF7FBF` | Original CRT `__stricmp(left.data,right.data)`; caller ADD ESP,8 at `00443CF0`; sign only. |
| `0076EAA0` | `0076EAC0` | `00BF7FBF` | Forward string comparison; caller ADD ESP,8 at `0076EAC5`; negative returns true. |
| `0076EAA0` | `0076EACF` | `00443CB0` | Current reverse string predicate, ECX right and stack left; callee RET4. |

All native callees' bodies were inspected. The original `00BF7FBF` is VS2005
CRT code with ASCII/locale paths and invalid-parameter behavior. This packet
inherits the accepted provider's current CRT `_stricmp` boundary; it does not
claim private original CRT locale, errno or invalid-parameter-handler parity.
No new host callback substitutes for either candidate. Live xrefs found the
single `00443CB0` caller above and no references to `0076EAA0`.

Live queries verify the configured `C:/Users/sqz269/bsp.gpr` project and
`/battlestationspacific.exe` target. Original complete byte ranges are compared
with the installed PE, and annotations preserve previous values under the
Ghidra write lock before project save/export. The report records those values,
call verification, and build status. CMake registration is a unique source append
after the foreign lease was released; no exclusive registry lease was taken.
The strict MSVC Win32 `scripts/build.ps1` run passed for Source revision
`df1f39680cc95e21c569c5d149d7f6b96ac5fdfb`, with both configured existing CTests
(`reconstructed_math` and `tool_tests`) passing. Original native math differential
tests were not enabled in this worktree. The build log is recorded in the report.
No new tests are added. The new C++ interfaces are not original ABI replacements;
no candidate-specific original-byte differential or gameplay validation is claimed.
