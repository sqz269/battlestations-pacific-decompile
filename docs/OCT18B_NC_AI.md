# OCT18B nC segment 66 leaves

The exact assigned 21 rows in `codex_new_nC.txt` segment 66 are accounted for:
four game-owned ContentPackageList no-ops are reconstructed, and 17 MSVC C++
library members are classified `library_instantiation_out_of_scope`. Segment 67
is excluded. The library bodies receive no reconstructed game-code credit and
no replacement implementation.

The Ghidra client verified project `bsp`, program `/battlestationspacific.exe`,
x86 little-endian 32-bit and image base `00400000` against `config/target.json`
before analysis/export and annotation. That config selects the existing
`C:/Users/sqz269/bsp.gpr` project. The original game and program signatures are
unchanged. Reviewed names are descriptive hypotheses with address suffixes.

The four game rows occupy vtable `00D1D75C`, already identified by the existing
`00994F10` constructor and `00994E10` destructor as ContentPackageList. Their
implementations extend the existing `engine_seg11_35_66_cc13` source/header
family, alongside `ContentPackageListLayout`; no new source registration is
needed.

| Entry | Vtable slot / pointer | Original body | C++ behavior |
| --- | --- | --- | --- |
| `00994EB0` | 7 / `00D1D778` | `RET` | No side effects |
| `00994EC0` | 8 / `00D1D77C` | `RET` | No side effects |
| `00994ED0` | 9 / `00D1D780` | `RET` | No side effects |
| `00994EE0` | 10 / `00D1D784` | `RET 4` | No side effects; ignores one stack word |

The member context supplies ECX, which none of these bodies reads. None writes
EAX: its incoming value survives in the native body, and no defined result is
established. The C++ interfaces return `void` and do not reproduce machine
register preservation or stack cleanup. Slot purposes and slot 10's argument
type are unknown; its C++ `uint32_t` represents only the ignored stack word.

The library evidence is in `reports/oct18b_nc_ai.json`, including exact native
cleanup and vtable references per row. The codecvt table `00D1D510` has the
standard identity-conversion members (out/in/unshift return `noconv` = 3 while
preserving input/output cursors), the bounded length member and a scalar
deleting destructor. Its three constant virtuals are shared with `00D1D1F4`.
Two constant-one members retain slot-based names because their individual
standard method identities are not established here.

Streambuf table `00D1D214` is written by the existing `0098E4E0` destructor;
the assigned rows implement EOF pbackfail, inert setbuf/sync/imbue, secure
buffered reading and invalid seek results. The reader at `0098E6D0` uses
`0098DF50` get-area availability and `_memcpy_s`, then uflow at vtable +14h.
The filebuf table `00D1D434` reuses that reader and installs `00992330` at
slot 13; the latter selects a codecvt facet with `00991D60`, checks its
always-noconv virtual and updates the facet pointer at +3Ch. Ctype table
`00D1D48C` is installed by the existing `00990C90` constructor; `00990E10`
is its range-to-secure-range virtual dispatch. These are library behavior,
not game algorithms.

Validation: all assigned native bodies were exported and inspected; assembly
confirmed RET cleanup, ECX use and undefined EAX behavior. The four C++ no-ops
await the integrator's combined Win32 build through `scripts/build.ps1`.
No new tests or game runs were requested. This is static reconstruction and
library classification evidence, not ABI or gameplay validation. Annotation
uses `ghidra_annotate.py --apply`, preserves existing comments and records prior
values in its local annotation log before saving. Exports are refreshed after
annotation. The JSON report records the completed verification receipts.
