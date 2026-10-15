# GOAL15C avoidance cache cleanup and captured hull data

Names are hypotheses; the correct existing `DYN_ConvexHull_Destroy_00C37450`
library name and its historical 719-case proof are retained.

| Routine | Inclusive native body | Native ABI | Coverage |
| --- | --- | --- | --- |
| `0041E960` | `0041E960-0041EA23` (196 bytes) | ECX manager; no stack arguments; plain RET | complete owned cleanup schedule through concrete queue/free/hull providers |
| `00C37450` | `00C37450-00C37480` (49 bytes) | ECX handle; no stack arguments; plain RET | existing complete typed reconstruction repaired to capture data once; historical finite-input algorithm/data proof retained |

Complete listings and live bounds were read through the target-verifying
`bsp.py` bridge: `C:/Users/sqz269/bsp.gpr`, project `bsp`, program
`/battlestationspacific.exe`, x86 LE32, base `00400000`. No flow gaps or repair
were found. This worker makes no Ghidra, CMake or shared packet-registry writes.

## Manager schedule and actual publication

The new header reuses the existing exact 78h `Cc13AvoidZoneManagerNative`, its
body-pointer vector at +60h/+64h/+68h, and +70h/+74h hull list. Its 10h-byte
`Cc13AvoidZoneHullListNode` contains the existing 8-byte `AvoidZoneDynHullHandle`
at +08h. No duplicate native ownership or owner-class schema is introduced.
The sole caller `0041F600` clears the group slots then tail-jumps here at
`0041F635`; its complete listing was read. This is static caller evidence,
not a gameplay-path claim.

The Source API is `__fastcall(manager, actual_publication_cell_address)`: ECX is
the manager and EDX explicitly borrows the ADDRESS of the actual `00E188A8`
publication cell. A Source-only PUSH/POP retains that address across calls. At
each native publication-load site the kernel reloads the actual cell value,
then `[current_game+18h]`, and invokes the concrete release queue. It creates no
substitute global, shadow publication or cached game-pointer snapshot. POP
discards Source context without changing final arithmetic flags. Source frame,
argument and provider identities remain qualified; this is not a production ABI binding.

The complete original ordering remains:

1. Reload vector first/last each queue iteration. Counts use DWORD subtraction
   followed by signed SAR2, then the original unsigned index comparison.
   The second check can report an invalid parameter and return; first is reloaded
   before fetching the current body. Each body is queued through the current publication.
2. Cache old last, validate order, reload first, validate again, and preserve
   that cached endpoint across a returning handler. Tail length uses SAR2;
   only a signed-positive count invokes `memmove_s`. The computed last is stored
   even when the copy is skipped. Vector first/end/capacity are retained.
3. Capture head->next, self-link the current head's next, reload head before
   self-linking previous, compare against the current head and zero list size.
   For every node capture next before destroying its embedded handle and freeing
   the node, then compare next against a newly loaded head. The sentinel remains
   allocated and unrelated fields are untouched. No EH rollback is added.

`00C34F70` is the concrete `native_dyn_release_queue.hpp/.cpp` implementation
from independent commit `8a0a5c625d946c37d0f85053cf697f21c35d0609`. Its complete native
body was read; it appends a DWORD to the actual service's +438h/+43Ch/+440h queue,
using the existing allocator/free. It does not destroy or drain bodies here.
Node frees use `singleton_lifetime_free`; the hull bridge calls the existing
typed destructor with the matched genuine singleton allocator/free pair through
the existing memory contract. There are no abstract success/delegation stubs.
Invalid reports use the existing qualified returning Source entry; secure copying
uses the genuine current SDK `memmove_s`. Original CRT/heap/error/exception,
callee-frame and fault/return-address identity is not claimed.

## Captured-data repair

Native `00C37451` captures `[handle]` once into ESI. It reads data+0Ch and frees
adjacency; after that return `00C37467` reads vertices from the SAME captured
data, then frees that captured data itself. The old typed implementation instead
read `handle.data` in each of its three release expressions. A callback that
republishes the handle therefore caused it to free the alternate object's
vertices and data. A handle's shorter lifetime also made those later handle
reads an extra requirement absent from the native body.

The repaired function caches `data=handle.data` once, checks it once, then
releases data->adjacency, reloads data->vertices AFTER the first release, and
finally releases cached data. It does not store handle words. The optimized
listing confirms one handle read into EDI, +0Ch read, callback, +00h read,
callback, final cached EDI release. Existing custom memory providers keep their
qualified Source contract; their callbacks may republish/end the handle while
the captured data must remain live until its final release.

## Verification and integration

- Both translation units pass strict MSVC Win32 `/std:c++17 /EHsc /MD /W4 /WX
  /fp:strict /O2 /c`. Ignored reproduction: `local/goal15c_avoid_cache_compile.cmd`.
- Static COFF comparison passes: Source cleanup is 198 bytes vs Native196.
  Exact exclusions are one added context PUSH byte, one context POP byte,
  the six-byte absolute publication load replaced by six bytes of actual-cell
  load/dereference, and seven CALL rel32 operands. Every other byte, branch,
  arithmetic/read/store and cleanup matches. The ignored comparator accepts an
  optional registered-object path: `local/goal15c_compare_cache_bytes.py`.
- The report contains all seven cleanup calls and three native hull free calls
  for live mechanical checking. It distinguishes the caller's tail JMP from CALL.
- The focused real-provider Source fixture is
  `local/goal15c_hull_publication_probe.cpp/.obj`; strict compilation passes.
  Its first genuine adjacency free republishes to a valid alternate record.
  Expected frees are original adjacency/vertices/data (indices0,1,2), with the
  republished handle and field04 preserved. Both outcomes clean remaining known
  heap blocks safely, without dereferencing a freed alternate. The primary
  reproduced the retained baseline failure: exit1, indices0,4,5. Candidate run
  against the rebuilt registered archive remains primary integration work.
- One ordinary Source cleanup composition fixture is being prepared separately
  in ignored local storage, using real list/hull/body/queue backing and the actual
  passed Source publication cell. No new test target/framework is added.
- Primary owns registered `scripts/build.ps1`, existing CTest and fixture links
  with `/MD`, actual Release core/dependency archives and `/MANIFEST:EMBED`.
  Safe probe names contain `probe`, not installer-detection words.
- Historical relevant checks remain `reports/avoid_zone_dyn_hull.json` and
  `docs/AVOID_ZONE_DYN_HULL.md`: 27 byte spans, 719 original-byte finite cases,
  including706 Marshall pieces and719 retained copies after source destruction.
  The historical ignored `local/dyn_hull_probe.cpp` linked the actual Release
  archive with an embedded manifest. Its receipt is preserved, not represented
  as freshly rerun. Existing mechanical check:
  `python tools/verify_report_calls.py reports/avoid_zone_dyn_hull.json`.
- No original Native execution, production binding, malformed graph/OOM/SEH,
  physics-body destruction, gameplay or drop-in binary ABI proof is added.
  All reached native spans, Source context/control slots, heap/provider and
  publication backing must remain live. Only the two leased address records are
  changed; C37450 receives appended name evidence and a repair fragment, leaving
  its original complete-function proof/status untouched.

## Primary integration result

Registered Win32 build and all three existing CTests passed. registered-archive cleanup composition passed; 4 node/hull blocks freed, retained vector/sentinel/body/queue intact, all11 fixture blocks reclaimed. Hull publication regression: old registered archive failed0,4,5; corrected registered archive passed0,1,2 with genuine heap releases and valid alternate publication. The primary reproduced the documented registered COFF comparisons where applicable, checked all report call rows, saved Ghidra annotations and refreshed exports. Actual archive/fixture hashes, exclusions, rollback artifacts and logs are in `reports/goal15c_reconstruction_batch.json`. Historical Native evidence and current Source checks remain separate; no production/native ABI or gameplay validation is claimed.
