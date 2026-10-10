# CC12 locale pool section cleanup contract

`00402F70` drains a signed-positive CURRENT DWORD at receiver `+18h`, decrementing
before every Leave call, then deletes the actual section at the receiver address.
Zero or negative depth skips Leave but still reaches Delete. The accepted locale
action supplies `pool+0Ch`, so the tracked depth is at `pool+24h`. Existing Source
already contains a generic borrowed-section helper with this normal schedule;
no duplicate leaf C++ is needed. Complete owner construction and activation are
not established by this packet.

The full record is [the section report](../reports/cc12_locale_pool_section_cleanup_contract.json).
The accepted [action forwarding](CC12_LOCALE_UNWIND_PRIMARY.md),
[table cleanup](CC12_LOCALE_TABLE_CLEANUP_PRIMARY.md),
[normal lifecycle](CC12_LOCALE_LIFECYCLE_PRIMARY.md), and
[profile](CC12_LOCALE_PROFILE_PRIMARY.md) retain their earlier qualifications.

## Body and metadata gate

Live prototype and typed ownership identify one complete stored range,
`00402F70..00402F96`: both body and hull are 39 bytes, within the approved
128-byte limit. The exact range was leased before assembly and byte reads.
Sixteen instructions occupy every byte; there are no gaps, padding or
out-of-hull continuations. All bytes match retained original PE SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The existing name `BSP_RecursiveCriticalSection_Destroy` and stored
`undefined ...(void)` prototype remain unchanged. Complete metadata records for
entry and six control-flow sites are equal before/after at modification 88.
Both import calls are `COMPUTED_CALL`, with override `NONE` and normal listing
fallthrough. Their effective external targets are LeaveCriticalSection and
DeleteCriticalSection, each with `no_return=false`. These are stored metadata
facts; the normal behavioral graph separately assumes the calls return with
their expected Win32 calling convention. No metadata or listing repair is made.

## Exact access and call schedule

1. Save ESI and capture incoming ECX in ESI. Read the DWORD at `[ESI+18h]` and
   compare it to zero with signed `JLE`. This read is unconditional.
2. Only if it is positive, save EDI and read the Leave function pointer from
   cell `00CE2210` once into EDI. The skipped path never reads that cell.
3. At `402F80`, subtract one from the CURRENT DWORD at `[ESI+18h]` using a
   32-bit memory ADD. The update wraps modulo 2^32 and has no LOCK prefix.
   Push the captured section address ESI, then `CALL EDI` at `402F85`.
4. After normal return, reread the CURRENT DWORD and use signed `JG` to repeat.
   The loop has no saved initial count or fixed iteration budget. It keeps the
   cached Leave target even if its IAT cell changes later.
5. After normal loop completion, restore EDI. Both that path and the initially
   nonpositive path push ESI and call through the CURRENT `00CE2214` cell at
   `402F8F`. After normal return, restore ESI and use plain RET.

The code passes the receiver itself as the section address, not an embedded
pointer loaded from it. It never enters the section. It does not force depth to
zero, clear a negative depth, zero section bytes, free storage, touch pool list
links or inspect table/count/capacity. The only explicit object-memory update is
the pre-Leave depth decrement. If a Leave fails to return normally, that update
has already occurred and this body supplies no rollback or local cleanup.

Under normally returning imports that do not independently change the tracked
depth, an initial positive value drains to zero. The actual instructions always
use current memory; external changes cannot be replaced by that simpler model.
After normal loop exit, Delete is not conditional on a return value from Leave.
The normal-return graph has 17 internal edges across all 16 instructions.

## Import identity, stack and failure-prefix limits

The frozen original PE import descriptor independently maps `CE2210` to
`KERNEL32.dll!LeaveCriticalSection` and `CE2214` to
`KERNEL32.dll!DeleteCriticalSection`. The proof retains exactly 104 supporting
data bytes: the import directory entry, one descriptor, lookup/IAT DWORDs and
the DLL/function names. These are newly interpreted slices of the already
retained PE, separate from the 39 fresh GPR body bytes. No live IAT value, loaded
OS binding, installed image or OS-import body was read.

The native receiver is ECX; the stored void prototype does not express it. There
are no explicit caller stack arguments. Each import receives one pushed
section pointer, with no caller-side argument adjustment. Balanced normal paths
therefore require the expected one-argument callee-cleaned Win32 convention.
ESI is saved/restored on all normal paths, EDI only on the positive-depth path;
the local body does not touch EBX or EBP. Callee preservation of ESI/EDI is also
required while they hold the receiver and cached Leave target. EAX, ECX, EDX
and flags after Delete are unclassified; no meaningful API return value is
established. No compiled ABI replacement or runtime behavior is demonstrated.

There is no null or initialization guard. A null receiver still attempts the
depth read at address `18h`. Zero depth is not evidence that section storage was
initialized: it merely skips Leave and proceeds to Delete. The caller must
supply an actual initialized section and valid recursive-ownership conditions,
as the existing Source helper's header also requires.

Retained constructor bytes arm state 0 before the section initialization call,
then write zero to section `+18h` after its normal return, before arming state 2.
The declared state-0 unwind row does not select section cleanup. The declared
state-2 chain can reach this section action after table cleanup, with zero depth
on the ordinary allocation-failure prefix. This does not prove actual exception
dispatch, safe cleanup of an uninitialized prefix, or SEH/double-failure coverage.
Upstream action `C5EEA8` still requires dispatcher `EBP=S=R+0Ch`; actual frame
setup remains unproved. No handler or EH data is expanded here.

## Existing Source observations

Eight bounded excerpts from four frozen Source files are retained and tied to
their actual Git blobs. The public
`destroy_native_embedded_tracked_section_00402f70(void*)` in
`src/native_renderer_parent_member_cleanup.cpp` uses volatile DWORD reads,
signed-positive tests, unsigned wrapping decrement before a captured Leave
call, current-depth reread and final Delete. Its header requires an actual
embedded 1Ch tracked section and matching recursive acquisitions on the current
thread, and promises neither storage freeing nor negative-depth clearing.
This existing generic helper is a reuse candidate under those contracts.

`DirectionalLightPool::destroy_critical_section_00402f70` has the ordinary
drain/Delete schedule as a private `noexcept` method. Its typed storage and
layout checks belong to the actual `01090154` directional pool; this is not
locale `E1768C` owner identity evidence. Its direct API calls and typed signed
field are not a fresh proof of the native IAT-capture/access schedule. Neither
Source observation supplies new compilation, exception/fault timing, ABI,
binding, quiescence or application integration evidence.

## Portable proof and next bounded scope

Capture HEAD is `69535cbf7aa6d5a7d4407389426eacea09dbee89`. All 4,217 frozen
Source files are physically equal to their committed Git blobs and exactly
match the preceding table-cleanup Source epoch. Commit/tree objects prove a
closed census of 4,210 `include/`/`src/` paths plus seven explicit inputs.
Sixteen dependency and four tool/config blobs are retained. Historical artifact
and Native epochs remain separate; no new artifact credit is claimed.

Six read-only BSP query outputs verify the existing project/program. Native
captures were closed at epoch 88 before Root's later annotation work. Fresh body
bytes total 39; prior reviewed Native bytes total 1,379; the separately identified
104 import-data bytes come from the retained PE. All saved analysis is preserved.

`local/cc12_locale_pool_section_cleanup_contract/evidence.zip` contains 4,281
payload files / 90,260,493 uncompressed bytes. ZIP size is 60,044,589 bytes;
SHA-256 is `79b9712d3190b8e1b1d6a32543cede5f1b3296ca8b5460c5eaa47e8f5d2f6263`.
The original and independent physical copy both pass guarded ZIP-only replay of
nested prior proofs, complete manifests, closed Git paths, PE body/import data,
full decoding/CFG, typed equality and frozen Source excerpts. External reads,
directory access, process/network calls and dynamic loading are denied after
guard installation. No build, test, probe, OS call or game execution is added.

The next proposed finite packet is `00403970` base unlink: metadata/prototype/
typed ownership first, then a separately approved exact body/hull. Preserve its
existing `BSP_AllocatorListElement_UnlinkBase` name and the known
`AllocatorListDomain::unlink_base_element_00403970` Source provider; the task is
to connect its native effects and actual list-domain contract to the locale
action, not to presume missing leaf C++. Dispatcher frame setup and canonical
storage/lifetime remain separate. This packet accepts a bounded cleanup contract
and Source reuse candidate; it does not establish whole-owner GO or activation.
