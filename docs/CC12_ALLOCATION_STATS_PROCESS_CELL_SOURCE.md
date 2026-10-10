# CC12 allocation-statistics process cell Source

`GameNativeStringProcess` now owns a distinct, permanent, zero-initialized
`void* volatile allocation_stats_0109cefc_` immediately after the existing pending
registry publication. Its out-of-line `allocation_stats_0109cefc() noexcept`
returns that actual cell by reference. This packet adds storage only. It does
not construct, publish, register, dispatch, sample or destroy a statistics owner.

The two existing Source files were already compiled on `bsp_game`; no CMake or
Core registration changed. The exact owned outputs are this document, its JSON
report, `include/bsp/game_native_string_process.hpp`, and
`src/game_native_string_process.cpp`. The address lease contains no Native
addresses or ranges. There are seven appended Source lines.

## Lifetime and identity

The existing function-local static pointer still retains one
`new GameNativeStringProcess` through process termination. No process destructor,
`atexit` registration or reset was added. First process access still may allocate
the retained process object. Accessing the new member merely returns its cell;
it does not load a publication, call a manager getter, create the future
12-byte statistics receiver, or bind a cache/host/deletion context.

The actual x86 MSVC layout diagnostics establish the following offsets. These
are compiler observations, not values assumed from the readiness audit:

| Member | Before | After |
| --- | ---: | ---: |
| manager publication | 0 | 0 |
| pool publication | 4 | 4 |
| disabled flag | 8 | 8 |
| raw string context | 12 | 12 |
| actual string storage | 24 | 24 |
| property empty byte | 44 | 44 |
| query empty byte | 45 | 45 |
| pending-registry publication | 48 | 48 |
| allocation-statistics publication | absent | 52 |
| complete process size | 52 | 56 |

Thus manager, string and empty-byte identities retain their established offsets.
The four-byte append is a Source process-cell layout change; it is not evidence
about the layout or activation of the separate Native statistics receiver.

## Frozen preimages and actual build context

The baseline is `898224d3c0c206cc30b04343f4ed8e74dace3de9`. Before editing Source, the packet retained the
complete accepted Source822 checkpoint: 822 Source inputs plus its library,
executable, map and existing-test log. Its exact selected artifact set remains
139 Core objects plus five App objects. The prior process App object is a
**separately newly selected provider**; it was not among those five and is never
retroactively counted as their previous proof.

The actual process command comes from `bsp_game.vcxproj` and its App CL tlog.
The genuine storage provider has its separate Core project/tlog command. Both
use x86 MSVC 14.51.36231 Release `/O2 /Ob2 /Oy- /EHsc /MD /fp:strict /std:c++17
/W4 /WX`. The packet freezes both projects, CL command/read tlogs, App link
command/read tlogs, 290 prior dependency occurrences, compiler executables/DLLs,
20 actual external link libraries, and 190 unique fresh dependency/tool records.
The complete provider Source closure contains 16 roots, 45 files and 57 local
quoted-include edges, with full current Git preimages. Initial failed preparation
and the compiler's single-layout-selector behavior are retained explicitly in
`execution_notes.json`; neither supplies any positive layout or build claim.

## Complete object comparison

The full fresh baseline process object matches the separately retained prior
App object across every non-debug/non-checksum section, including raw bytes,
characteristics, symbol identities and relocation targets. The fresh genuine
`ActualNativeStringPoolStorage` provider likewise matches its accepted full
Source822 archive member. Anonymous-namespace path qualification is explicit;
COFF `.sxdata` words are resolved through their actual symbol-table indexes.
Headers, timestamps, debug/checksum records and all original object bytes remain
retained and physically indexed rather than being presented as identical.

The new accessor is exactly `8D 41 34 C3`: `lea eax,[ecx+52]; ret`. It is four
bytes, has no relocation, and performs no cell read or write. Of existing code:

- The emitted default process constructor grows 64 to 71 bytes solely by inserting
  `mov dword ptr [esi+52],0` at offset 0x3E.
- The retained-process factory grows 216 to 223 bytes. Its allocation immediate
  at 0x6A changes 52 to 56, the same zero store is inserted at 0xBA, and only two
  branch displacements adjust. The exact remaining instructions and all
  relocation identities are preserved with the required seven-byte shift.
- Both existing string-storage destructors, pending-cell and empty-byte
  accessors, exception funclets/handler, `.xdata`, static process pointer and
  initialization state remain unchanged. No additional allocation/deletion
  wrapper or exit cleanup appears.

The process object has 7 then 8 ordinary code symbols and 2 EH code symbols in
both versions; full code is 344 then 362 bytes, 107 then 111 decoded instructions.
Physical sections are 22 then 24 and symbol/AUX records 71 then 76. The extra
metadata relocation belongs to the new debug record; the new leaf has none.

The before/after complete provider graphs retain 15 genuine full Core members
each. They cover 244 then 245 reachable sections, 626 relocation edges and 219
explicit CRT/Win32 frontiers in each version. Complete code decode totals are
15,341/5,521 then15,359/5,525 bytes/instructions with zero undecoded code bytes.
Every entry in the exact provider map links its whole archive member to its
current/Git/Source822 Source witnesses. No frontier is replaced by a fake
provider or silently treated as behavioral proof.

The ordinary/physical inventory is `physical_inventory.json`: 57 complete COFF
files, 1,796,025 bytes, a disjoint byte partition and zero unaccounted/gap bytes.
Each entry records exact code-symbol indexes, section/value/type/storage class,
full COFF header, section/raw/relocation regions, complete symbol+AUX and string
tables. `before/indexes` and `after/indexes` retain all raw section bytes and
every indexed relocation; the graph is rooted in all process code sections.

## Validation and remaining boundary

`./scripts/build.ps1` completed its normal Win32 Release build and both configured
CTest checks (`reconstructed_math`, `tool_tests`); no tests were added. This clean
worktree lacks `local/seed_reference.hpp`, so CMake did not register the conditional
`native_math_differential` check. The old Source822 Root3/3 result remains historical;
Root can run its already-configured third check during integration. This packet
performs no new Native seed read/generation/execution. The actual normally built process
object matches the focused candidate comparison. All 15 normal Core provider
objects match the retained provider graph under the recorded namespace and one
explicit accepted lambda-name qualification. Among the five prior selected App
objects, the VFS application and shader process match the narrow comparator.
`game_main` and `game_hosts_vfs` contain broader path-qualified lambda names and
section ordering; the renderer contains three anonymous RTTI name strings. Their
whole objects/indexes and negative comparison results remain retained; this
packet does not claim all five are unchanged. Root's same-worktree integration
audit is pending. The full normal-object replay decodes all 21 retained objects:
208,373 code bytes and 71,033 instructions, with no undecoded bytes.

The final link map includes the existing retained-process factory. It omits the
new unused accessor, whose proof is the complete App COFF object; no new caller
or activation was added. No game process was executed as validation.

`verify_evidence.py` is a read-only offline replay of every retained local pin,
65 full Git preimages, 826 accepted checkpoint pins, 32 complete graph COFF/decode
records, every graph relocation/target, layout and precise code changes, and the
physical byte inventory. It also replays the immutable ZIP/manifest and, with
`--commit`, checks the exact four committed outputs against their frozen bytes.
Final receipts remain outside the bundle to avoid self-hash cycles.

Host-owned constructor context, mandatory scalar deletion binding, the two
finite D685E0/D685F4 routes, startup allocation/publication/registration,
cache/report/predicate composition and failure-retention/fatal policy remain
separate unimplemented work. There is no rollback, retry, safe failed-construction
drain or runtime/Native ABI/OS exception-delivery claim. No new Native queries,
installed-PE reads, Ghidra writes or ledger entries occurred.
