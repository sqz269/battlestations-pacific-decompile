# Native DWORD-vector assignment

This packet reconstructs the complete `00697BD0..00697CF4` body: 293 bytes,
133 current instruction starts and seven direct transfers. The library name
`STL_Vector_Assign_00697bd0` is preserved as a descriptive hypothesis. The new
Source directly composes the accepted raw forward-copy, uninitialized-copy
dispatcher, clear, Buy and matching current-CRT free services. No callback,
container/header model, element-count adapter or generic assignment is used.

Declarations and Source are in `native_dword_vector_assign.hpp/.cpp`. This is
a new MSVC Win32 interface, not an installed binary replacement. The existing
`assign_input_checked_word_storage` fragment in
`native_input_configuration_storage.cpp`, its narrower storage/fixture domain,
and its reconstruction record remain intact.

## ABI and storage

Native ECX is the destination header; incoming EDX is unconsumed. One stacked
word points to the source header. Every normal return uses `RET4` and returns
the original destination in EAX. The new Source expresses this physical input
shape with an explicit unused EDX argument and a raw pointer result. It does
not put an exception barrier around the entry.

Both actual headers occupy `10h` bytes. Begin, end and capacity end are at +4,
+8 and +C; the opaque +0 word is not written. The elements are raw four-byte
DWORDs. The accepted Buy producer establishes these fields, and the actual
`00698010` producer advances headers by `10h`. All address arithmetic is
unsigned DWORD wrapping. Size calculations use arithmetic right shift by two
on the wrapped byte difference, then unsigned comparisons of the resulting
DWORD bits. No typed pointer subtraction or validation is substituted.

## Complete branch schedule

Exact header self assignment returns before reading any header field. Other
overlaps do not get an extra guard or source snapshot. The body captures
source begin first; a null begin skips the source-end read and enters actual
`00697890` clear. Otherwise it captures source end and computes SAR2 size.
A zero result also uses that clear, including its actual diagnostic and
captured-end behavior.

For nonempty source it captures destination begin. A null destination begin
gives size zero without reading destination end. Otherwise size is computed
from the current end and captured begin. The first unsigned comparison selects
shortening when source size is no larger than destination size.

Shortening passes the captured source endpoints and destination begin to
actual `00697080`. That child's EAX is ignored. After it returns, assignment
reloads current source begin and, only if nonnull, current source end. It then
reloads destination begin before the final SAR2 calculation and publishes
`current_destination_begin + recomputed_source_size*4` as end. Reusing the
initial source size would lose header/backing aliases exposed by the copy.

If source is larger, capacity is computed using captured destination begin
and current capacity end, with the same null-begin skip. When source fits,
destination size is read and computed again. A captured source middle is
formed from original source begin plus this repeated size times four.
`00697080` copies the prefix into captured destination begin. Assignment then
reloads destination end first and source end second. The actual `006974B0`
dispatcher copies from captured source middle to that fresh source end, into
the fresh destination end. Only after return is its actual advanced EAX
published as the new end.

When capacity is insufficient, captured old destination begin is freed first
if nonnull. This happens before Buy clears any fields. Source begin and its
conditional source end are then read again, and the requested SAR2 count is
recomputed after free. Actual `00557200` receives that same count. It clears
+4, +8 and +C before the zero/length/allocation branches. Assignment consumes
only Buy AL. AL zero skips all further copying. Otherwise it reloads
destination begin, source end, then source begin, in that order, and uses
`006974B0` with this post-Buy range/output. Returned EAX is published as end
only after the copy returns.

The incoming EDX slots of clear and Buy are unconsumed; Source passes zero
without claiming their original dead register residue. For the two
uninitialized-copy calls it forwards the known native EDX bits: current tail
destination and post-Buy source end respectively. The child still treats that
incoming slot as unconsumed. Its actual owner is the destination header, with
the accepted dispatcher's private metadata construction unchanged.

## Ownership, failures and actual bindings

There is no local FS/FH3 frame, owned temporary, catch, unwind action,
element destruction, fill, reserve growth formula, source copy or rollback.
The parent releases old backing through `singleton_lifetime_free`, the
accepted actual current `std::free` service at native `00BF65AC`. It does not
add a custom allocator or change the accepted allocation/free domain.

Clear uses `clear_native_checked_dword_vector_00697890`, which was separately
accepted with its actual erase and returning/throwing current SDK diagnostic
policy. Forward copying uses `copy_native_dword_range_forward_00697080`;
there is no null-output skip in that body. Uninitialized copying uses
`copy_native_dword_assignment_uninitialized_dispatch_006974b0`, whose actual
`00696D40` core skips data accesses for null output while advancing cursors.
Neither progressive alias schedule is replaced by memmove or a snapshot.

Buy uses `buy_native_checked_dword_vector_00557200`, directly bound to the
accepted `00553260` allocation and full `00556C20` length-error services.
Its current new-handler may reenter. The parent therefore rereads the source
range after Buy, rather than keeping the earlier range across allocation.
Allocation or length exceptions propagate after old backing was freed and
the header was cleared by Buy. Normal copy-end publication remains after
the copy. Source does not promise original machine-fault/SEH behavior for
inaccessible or dangling storage.

## Fresh native evidence and accounting

| Verified transfer | Scope |
| --- | --- |
| `00697BF5 CALL 00697890` | Owned empty-source clear |
| `00697C1D CALL 00697080` | Owned shortening copy |
| `00697C86 CALL 00697080` | Owned prefix copy |
| `00697C99 CALL 006974B0` | Owned current-end suffix copy |
| `00697CAF CALL 00BF65AC` | Owned free before Buy |
| `00697CCD CALL 00557200` | Owned recomputed-count Buy |
| `00697CE4 CALL 006974B0` | Owned post-Buy copy |
| `00698026 CALL 00697BD0` | Whole native fill-producer reference |
| `006987C8 CALL 00697BD0` | Bounded configuration-clear producer |
| `00BF65AC JMP 00BF9DC8` | Separate actual CRT thunk identity reference |

All owned bytes and all 133 instruction starts were checked against the
installed PE and current Ghidra listing. Every selected transfer has its
current owning function, default/effective target and REL32 bytes verified.
The whole native `00698010..00698035` producer adds 38 bytes/18 starts. The
bounded `006987C0..006987CF` producer adds 16 bytes/five starts, including
argument setup, transfer and following `10h` advance; it does not cover whole
`00698730`. The separate free thunk adds five bytes/one start and one tail JMP.

The union is 352 bytes/157 current starts, nine CALLs and one reference tail
JMP. Owned coverage remains 293 bytes/133 starts/seven CALLs, without an owned
external tail JMP or local EH frame. The JSON report keeps these scopes,
full bytes, hashes, instruction starts and provider Source fingerprints
separate. No Ghidra repair or mutation was performed by this packet.

## Validation boundary

The MSVC Win32 build and all three existing suites passed in 7.40 seconds.
All ten JSON transfer rows and ten documentation claims pass, with no failures
or unchecked claims. No new tests or native execution were added. The existing
suites exercise established math, seeds and tools; they do not prove this
entry's original process behavior.

The focused Release COFF check passed eight checks over the compiled Source's
637 bytes/247 instructions. It verifies physical RET4/receiver return,
self/empty paths, unsigned sizing, shortening reloads and final SAR2, repeated
size/prefix/current-tail order, free before fresh-count Buy, AL-only result
use, post-Buy range loads and delayed publication. All seven compiled external
sites bind the five accepted services. These Source counts are separate from
the original 293 bytes/133 starts and do not establish original frame or ABI
identity. Frozen Source/header/provider and compiled-object hashes are in the
JSON report. Registration was appended under a successful FIFO lease and
released before the build.

The Source requires accessible reached raw storage and the accepted current
allocation/free domain. It preserves progressive aliases and public header
reloads, while original private argument/frame aliases, ambient registers and
flags, native CRT/heap/new-handler state, Watson/fault/SEH/FH3/RTTI identity,
binary ABI and gameplay remain unproved. Existing qualified Source and
fixtures retain their own provenance.
