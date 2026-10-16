# Actual checked DWORD vector copy constructor

This packet reconstructs whole `00557590` (196 bytes, 70 instructions) and
the effects of native catch `00557654` (17 bytes, five instructions) as a
lexical Current C++ catch. The catch has no public standalone Source ABI.
Correct library name `STL_CheckedWordVector_CopyConstruct_00557590` and the
native catch identity remain intact. New Source names are hypotheses.

Native ECX is destination, EDX is unused, and one stacked word is source.
RET4 returns the original destination in EAX. The new fastcall interface has
explicit unused EDX so source keeps its physical stacked position. This shape
does not establish original binary ABI or private-frame compatibility.

The actual16B header has opaque/begin/end/capacity at 0/4/8/C; elements are
unowned four-byte DWORDs. Capture sourcebegin first. Nullbegin selects count0
without reading initial sourceend; otherwise derive count with wrapped DWORD
subtraction and actual x86 SAR2. Capture count BEFORE clearing destination+4,
then+8, then+C. Count0 returns destination without allocation or further checks.
Initial old destination backing is not freed, including when reset loses it.

For nonzero count, compare its DWORD bitpattern unsigned against 3FFFFFFF.
Actual length error and allocation occur AFTER clears but OUTSIDE state0 and
the Source try scope. Pass count unchanged with EDX0 to actual allocation.
Publish allocated begin, end=begin, then wrapped capacity=begin+count*4.
There is no Buy helper, null-allocation guard or rollback.

Capture CURRENT sourceend and read/compare CURRENT sourcebegin against it
BEFORE arming state0. The Source performs that read/comparison outside its
lexical try. Inside the protected scope, first report invalid order and retain
captured end if reporting returns. Capture a fresh sourcebegin, compare it to
fresh sourceend, and retain that SAVED first pointer across the second report.
Only AFTER that report read CURRENT destinationbegin.

Pass full savedfirst and original capturedend as the copy endpoints. Both dead
metadata/tag words receive the defined savedfirst bits with low byte cleared;
owner is the original destination. These are bit values, not an invented tag
type or allocation model. Actual uninitialized copy preserves progressive
DWORD aliases and null-output skip. Publish its advanced output as destination
end; do not derive end from the captured count, allocated pointer or memmove.
On normal completion return original destination.

State0 catch calls the actual tidy routine on the original destination header,
then barethrows. Tidy captures CURRENT destinationbegin once, conditionally
frees that pointer, then clears+4/+8/+C in order. It does not necessarily free
the saved allocation if a returning diagnostic changed the header. No leak
repair, saved-allocation free, source selfalias repair or extra guard is added.
In particular source==destination observes the clears and later publications;
it is not converted to an ordinary safe self-copy or snapshot.

All native transfers in the complete ordinary/catch bodies and read-only
handler are independently checked:

- `005575E3 CALL 00556C20`: accepted owning length error, outside state0.
- `005575EC CALL 00553260`: actual count allocation, outside state0.
- `0055760C CALL 00BF6713`: first order report after state0 is armed.
- `0055761C CALL 00BF6713`: second order report with saved first.
- `00557637 CALL 00555000`: actual progressive copy, advanced EAX consumed.
- `00557657 CALL 004CB130`: catch cleans the current header.
- `00557660 CALL 00BF6885`: native zero/zero rethrow maps lexical barethrow.
- `00C6D815 JMP 00BF6B43`: read-only native FH3 handler dispatch.

Raw installed/live EH evidence at D97C00..D97C57 is 88 bytes: catch descriptor
`[40,0,0,557654]`; two null-action unwind entries `[-1,0]`; try map
`[0,0,1,1,D97C00]`; FuncInfo
`[19930522,2,D97C10,1,D97C20,0,0,0,1]`. Handler C6D810 loads D97C34.
Native catch reads original destination from private EBP-14. Source uses its
lexically captured parameter, without a fabricated native frame or handler.
Native faults/SEH and compiler-private state/flag timing are not Current C++
exception proof. Source preserves the specified raw access and exception-call
ordering while retaining its own compiler frame and EH representation.

All reached raw accesses and frees must remain valid, copy equality termination
finite, and destination writable through cleanup. Public aliases preserve the
specified captures/current reloads; original/private Source argument and frame
aliases and asynchronous mutation are excluded. Accepted current CRT
allocation/free/new-handler and owning legacy length-error transport apply.
Original FH3, RTTI, fault/SEH, heap/CRT, ambient registers/flags, binary ABI and
game/runtime equivalence remain unproved. Existing qualified memmove/callback
APIs retain their original scope and identities.

The report separates 213 owned bytes/75 starts from the read-only10-byte/two-op
handler, pins all dependency and native fingerprints, and records the required
Win32 build, existing three suites and focused compiled-interface/EH inspection.
The exact Source built with MSVC Win32; all three existing suites passed in
7.94 seconds. The 427-byte compiled function section includes its generated
catch, length-error branch and padding. Its normal return is the original
destination with one stack word removed. Focused inspection confirms the full
saved first pointer alongside two masked dead words, the late current-header
load, the advanced-end publication, and current-header tidy followed by rethrow.
Current C++ metadata contains two null unwind actions and one catch-all try.
The existing unrelated Lua duplicate-symbol linker warning remains recorded.
No new test, native execution or gameplay check is claimed. Placement wrappers,
their no-op actions and the historical same-body/ABI correction remain solely
in the separate unchanged readiness; this packet promotes none of those scopes.
