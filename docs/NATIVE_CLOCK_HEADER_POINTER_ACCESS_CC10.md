# Actual clock header pointer access, CC10

Nine existing reads of the actual vector data pointer now use its typed volatile
member instead of a uint32_t lvalue. Three source files changed: particle record
resize, clock shutdown and sampler cache operation. Scalar count/capacity,
profiles, raw records/strings, allocation, cleanup and ownership are unchanged.
No caller or runtime case is activated.

The integrated canonical 1Ch clock has one live 0Ch
NativeResourceRecordVectorStorage subobject at complete+8. B1B4D0 forwards
complete+4 as the cache/secondary receiver; secondary+4 is that same header.
The resize API already takes a live typed header reference. The new helpers
read `data_00` through const volatile header access each time. They construct
nothing, introduce no snapshot/credit/default, and do not adopt arbitrary bytes.
Existing nonnull publications and standalone header callers must already satisfy
the canonical live-type contract.

## Recovered access sequence

| Route | Actual pointer access and order |
| --- | --- |
| 4DC410 growth | Fresh postreserve count at 4DC43F; each record reads current data at 4DC455 before placement/sentinel construction. |
| 4DC410 shrink | Single memory count decrement 4DC4B0, fresh count 4DC4B4, fresh data 4DC4BA, record destruction. |
| Resize source catch | Retained name cleanup, fresh current data, captured-index DWORD-wrap calculation, volatile placement-address store/read, rethrow. Existing CY recovery identifies the native state0 placement action/RET-only401130; no new private FH3 claim. |
| 4DDA40 drain | Current count 4DDA50 then data 4DDA53, sink capture, current profile/slot and release; fresh count 4DDA67 then data 4DDA6E before record destruction and count decrement. |
| 4DDAA0 | Resize0 returns, fresh vector data at 4DDAAA, existing free. |
| 4DE290 | Release-all returns, disarm array guard before second resize0, fresh data 4DE2D4, existing free. |
| B1A4F0 first scan | Requested-name normalization returns; count B1A56C then data B1A56F. |
| B1A4F0 second scan | Resolved-name normalization/comparison returns; count B1A6DB then data B1A6DE. |

These are three resize, four shutdown and two cache pointer reads. The catch
read at old resize line93 was in the same documented raw header helper and is
included. No name cleanup, placement cleanup behavior or array rollback is added.
DWORD products and sums retain their existing Win32 wrap. Header data/capacity
remain stale after native array free; no clearing or later access is introduced.

## Build and complete emitted evidence

Baseline d3abd087546eb77bd721511147bcf8fe8011c99c was safely fast-forwarded from
published main. Before one build,28 source/config pins,27 tool/SDK identities,
the exact Git tree, three exact old core-library members and1,414 frozen prior
artifacts were sealed. Eighteen bounded Ghidra CLI queries verify bsp.gpr,
/battlestationspacific.exe, x86:LE:32 and base00400000 per query. Five complete
native ranges total1,744 bytes and equal the exact installed PE.

One Release Win32 build completed2026-09-26 09:24:26.862259 to09:24:41.524761
UTC, exit0. Actual /MD, /EHsc, /O2, /W4, /WX and /fp:strict flags were verified.
All three existing CTests passed; no test or additional runtime case was added.
The actual launch and terminal wait tool-return objects are preserved.

Three exact new objects match the frozen core-library members. Complete bounded
COFF code sections and relocations cover89 old/91 new sections,6,236/6,250 bytes;
85 existing sections and5,569 bytes are identical. Complete raw metadata/data
sections are retained too. Decoder coverage is supplementary; raw sections are
authoritative and no native byte credit is added.

All50 existing cache code sections and all existing shutdown code sections except4DE290 remain
raw-byte/relocation equal. The public resize adapter is still the identical21B
body and retains its public requested-count address/RET4 lowering. New typed
read helpers are10/10/11B, each one current pointer-member read with no relocation.

Resize's complete private main/catch section changes317->315B. Its frame reserve
changes10h->14h, saved header/index/bound locals move, and the private EH cookie
offset changes-20h->-24h. Normal current count/data accesses remain ordered.
Catch name cleanup CALL is at110h, typed reload CALL118h, wrapped index product
11Dh, placement store128h/read12Bh, then rethrow CALL130h. The one-argument
typed reload removes the old raw offset argument. The volatile placement store
remains; its local lowering is not native stack/FH3 proof.

4DE290 changes307->306B: the first data read uses EBX, the actual secondary+4
header address, instead of ESI+4. Current count/data/sink/profile/slot ordering
and fresh postrelease data are retained. Guard disarm precedes the second resize,
then fresh data read and free. All REL32 call symbols retain their order. Full
section bytes and relocations are in compiled_sections.json; the instruction
schedule and exact offsets are in compiled_schedule.json.

## Boundaries and archive

Only these header-pointer lvalue accesses are corrected. Broader raw pointer
fields in records, aliases, strings, tables and other providers keep their prior
limits. Passing emitted comparison/build tests is not fully ISO-safe composition,
native ABI/private FH3/SEH/hardware-fault, or runtime proof. No full linked entry
or application admission is claimed.

C30570 genuine positive children/rate, platform5030/SDK, caller MSG preimage,
source0/W, full sampler/material/application/startup/draw and the original game
remain separate. No synthetic count/FPS/header/postmessage or fallback appears.
All previous code/build/runtime archives remain immutable.

Report: reports/native_clock_header_pointer_access_cc10.json. Archive:
local/cc10_clock_header_pointer_access_evidence.zip. Manifest, exact member/code
and metadata schemas, raw tool receipts and external freeze receipt are under
local/cc10_clock_header_pointer_access_implementation.
