# LandConvoy retained F payloads

Packet `cc11_scene_convoy_float_payload`, baseline `e036b2964`, owns
`00743450` and the scene-contents source plus this doc/report. The result is a
bounded adoption in `retain_land_convoy_roster`, not a new whole-function port.
Full receipts are in `reports/scene_convoy_float_payload_cc11.json`.

The actual class descriptor identifies LandConvoy as class `1Ah` in
`scene_entity_factory.cpp`. The scene dispatch calls this reader for that class
after library groups fill missing properties and authored properties overwrite.
Previously its five float fields reparsed `values.back()` even when a successful
explicit F already supplied an owning binary32 payload. They now use that
payload when the property exists, its letter is F, and `has_float` is true.
The existing raw scalar lambda handles every other case. Reverse still calls
that original lambda; Rows, Columns, Path and the Type1..4 roster are unchanged.

## Native scalar fragment

Read-only queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Live assembly was read for the complete setup
`00743450..007434A1` and scalar fragment `007434A4..007435DE`, with containing
function checks. `ESI` receives `ECX` at `00743474`; holder `+C0h` must exist and
its kind at `+4h` must equal 1. Its `+8h` supplies the property bag. `EDI` is 1.
Each row below calls `008F2260`, then compares the returned record's type at
`+4h` with 1. The F arm directly loads float32 at `+0Ch`; the other arm converts
the record's integer dword using CVTSI2SS. No diagnostic text is read here.

| Key / verified literal | Find call | F load | Integer conversion | Convoy store |
| --- | --- | --- | --- | --- |
| Speed / `00CE69A4` | `007434C7` | `007434D1` | `007434D8` | `007434E3`, `+368h` |
| ColumnGap / `00CFF338` | `0074352B` | `00743535` | `0074353C` | `00743547`, `+360h` |
| HP / `00CE6750` | `00743557` | `00743561` | `00743568` | `00743573`, `+364h` |
| RowGap / `00CFF330` | `00743583` | `0074358D` | `00743594` | `0074359F`, `+35Ch` |
| Offset / `00CED1B0` | `007435AF` | `007435B9` | `007435C0` | `007435DE`, `+3ACh` |

The other three scalar keys have distinct storage: Reverse's find at
`007434AB` is followed by byte load/store `007434B0/B3`; Columns' find
`007434F3` and Rows' `0074350F` copy integer dwords. All eight call rows carry
`00743450` as the containing function and mechanically verify.

The complete supporting `008F2260` listing was read. These undotted keys take
its native-string/map lookup at `008F235C..008F23D4`, returning the node's
record pointer or zero, with RET 4. Native string allocation, map
`0043B8B0`, dotted namespace/scratch behavior and original ABI are separate
providers. The scalar fragment dereferences found records without a per-key
null check. The host's existing missing/malformed fallback is preserved source
policy, not proof of native graceful failure.

The containing body ends with RET at `00743B6F`. Primary repaired the
15-byte returning-free continuation at `00743B4B..00743B59`: three additional
instructions restore the stack and publish the +3BCh field from +3B8h. The
stored listing now has 522 instructions and zero call gaps; two three-byte
alignment spans after unconditional jumps remain untouched. The project was
saved and exports refreshed. Receipt: `convoy_scalar_containing_flow_recovery_cc11.json`. Later
allocation, roster creation, EH, holder ABI and whole-body behavior remain
outside this scalar adoption. The preceding `0077E830` Lua/self attachment is
also external. Prior group-merge tail/body qualifications remain unchanged.

## Parser, ownership and admission

This consumes the accepted successful **nonempty explicit F** parser contract
from `SCENE_FLOAT_NONEMPTY_PAYLOAD_CC11.md`. Native type1 records retain float32
at `+0Ch`: parser `008F5BE0..008F5C91`, heap producer `008F3770` and scalar
constructor `008EF170`. The latter is a constructor, not an existing-record
copy helper. Clone `008F4F60` and assignment `008F0700` are separate supporting
contracts. This packet adds no parser, allocator or provider reconstruction.

The source provider is modern MSVC `std::sscanf("%f")` directly into float,
used once for an admitted C-locale, finite ordinary decimal-prefix token in
closed, NUL-free input under the verified `400h` bound. Historical VS2005 CRT
`BF7533`, numerical/FP-status/error/extended-ST0 parity remain unverified.
Partial/unsupported tokens, nonfinite/overflow, locale, aliases, allocator,
fault, reentry and ABI behavior are not newly admitted.

`has_float` is source availability, not native declaration identity or a
persistent parse-success/action flag. Ordinary source group copies and
compatible successful authored overwrites copy the whole `SceneProperty`,
including this owning value. No shared child or parent storage is introduced;
the existing ordered group capture and entity flag1 fill-missing remain intact.
Duplicate/implicit/type-conflicting parsing, enum declaration identity at
`+28h`, and Lua/context providers retain their separate integration limits.

Empty F is a distinct unresolved parser contract. Native guard failure has a
positive-zero temporary, but an existing type1 record is written only on
successful read; an absent record can be created with zero. A zero record
later copied from a group is real data, not a replayable failed-read action.
The source parser lacks that existing-declaration context. Bare F semicolon
still has no retained payload and takes the old host fallback. No empty-F
default or permanent replay flag was added.

## Focused source evidence

One ignored probe includes the actual `game_hosts_scene_contents.cpp` and
freshly compiles `scene_file.cpp`. The only active enlarged-property paths are
fresh parser/library/convoy code. Three project libraries and 74 Game object
link dependencies were copied to uniquely leased local paths, with matching
source pre/post-copy hashes and unchanged pinned pre/post-link hashes.
Uninvoked Game dependencies provide linkage, not game execution evidence.
The Win32 probe has an embedded RT_MANIFEST type24/id1 with `asInvoker`.

Both compile and final probe exit 0 under `/W4 /WX /fp:strict`. It loads the
same bounded 15 installed library inputs into 22 ordinary source groups using
the actual parser and PropertyLibrary merge code. Installed LandConvoy's
RowGap/ColumnGap/HP/Speed/Offset are `10,10,0,1,0`; all five payload bits match
the prior raw source path. The probe's discovery does not load enum tables or
exercise VFS/Lua. Scalar checks therefore make no roster-provider claim.

The same scenario applies successful authored values, clears the five
diagnostic arrays, and still obtains `7.25,-8.5,-0.0,2.5,14.75` bitwise from
the actual reader. Prefix-token conversion and negative-zero retention are
source proof. The original group remains unchanged. Reverse B, integer fields
and Path retain their old paths. Raw source-authored F, integer conversion,
malformed/missing/present-raw-empty fallbacks also retain their old results;
these permissive fallback checks are explicitly source-only.

Fresh JM06 parsing yields 96 authored entities, zero class1Ah entities. An
initial fixture assertion confused the fourteen SubmarineGen subset with the
whole scene; it was corrected without a production change. JM06 does not
exercise convoy admission, mode policy or gameplay here. Downstream source
formation uses the retained gaps/speed/offset in `game_hosts_units.cpp`.
HP remains retained scene data; existing member-creation/HP projection limits
are not repaired or claimed by this packet.

`git diff --check` and the eight direct-call verifier rows pass. No tracked
test, full build, game run, native replacement/ABI test or Ghidra mutation was
performed. Primary owns the integrated build and independent probe rerun.

Primary integration 80f59d1500079e8852d97c2afd803c1509fcd35a: full MSVC Win32 build and all three existing CTests passed; independent actual-main source probes passed. Executable SHA256 4a241ef9e5fe16e8cb058fbd07b7876d682a614e6f11d24f74e4b9cb1f7fc2cf. Build log local/cc11_copy_convoy_integrated_build.log. Source/caller/provider qualification, original full task/game ABI and game validation limits remain.
