# Whole preset-tree range erasure

`erase_native_input_preset_tree_range_006a1aa0` reconstructs the whole ordinary
entry: 200 defined bytes and 76 instruction starts in `006A1AA0..006A1B1E`
and `006A1B20..006A1B68`. The outside NOP at `006A1B1F` is excluded.
`STL_Tree_EraseRange_StrKeyCI_N2C_006a1aa0` retains its established library
identity. The descriptive Source name remains provisional. The published
NativeKeyboardTreeIterator/NativeStringStorage AV overload and its fragment
record remain separate and unchanged.

Native uses ECX=tree, unconsumed EDX and five stacked DWORDs: output,
FIRST.owner/node and LAST.owner/node. Both returns produce the actual output
pointer in EAX and pop 14h. Source uses the accepted trivial eight-byte
`NativeInputPresetTreeIterator` carriers by value and adds the actual borrowed
RawCtx reference, so it pops 18h. The mutable FIRST argument and private
eight-byte ignored output occupy new C++ frame storage. No original C++
declaration or original private-frame/argument-alias equivalence is asserted.

FIRST.owner is cached before CURRENT head/head.left. A returning initial
diagnostic does not refresh those captures; FIRST.node is read after it.
For a prospective full range, LAST.owner is read before CURRENT head is
captured, both before the next diagnostic. CURRENT LAST.node after that
diagnostic is compared with the retained head. The three diagnostic sites
use the published actual BF6713 Source with current SDK invalid-parameter
policy. It may return, throw or terminate; original encoded globals and
Watson/frame policy remain qualified.

The full path reads a fresh CURRENT head/root and invokes the existing whole
actual `006A0770` subtree service with the same RawCtx, including a nil root.
After success it reloads headA and resets root to self; captures headB BEFORE
clearing tree.count and resetting left to self; reloads headC AFTER the left
store and resets right to self. It then reads a fresh head.left result before
reading output and storing owner followed by node. Opaque+0 and the head
allocation are retained. A subtree exception prevents those resets/output.

The whole subtree is already published actual Source, not an AV fragment.
Its 82-byte/34-instruction body uses nil29, forwards opaque tree through right
recursion, then captures key data at node+10h BEFORE CURRENT left. Only nonnull
data reads CURRENT length at node+0Ch plus one, with DWORD wrap, and resolves
the actual getter/return services. Captured left survives pool work. Original
node free precedes the captured-left nil test. Getter failure leaves prior
right-subtree progress and skips current-node free/later-left work. There is
no SBO assumption, pair delegate, mapped+20h cleanup, header clear or key guard.

The partial path checks cached owner against CURRENT LAST.owner, with a
short-circuit for zero owner, before comparing cached node with CURRENT
LAST.node. It preserves OLD owner/node before advancing the actual mutable
FIRST through the published fixed-nil29 successor. The actual preset eraser
then receives OLD by value and a private eight-byte ignored output; that
child separately advances its private OLD copy. Only after success does the
range reload FIRST.node BEFORE FIRST.owner and continue. Empty comparison
still follows the diagnostic and writes the cached owner/node. Both output
paths capture the output and result values before owner-then-node stores.
There is no extra ownership/endpoint guard, revalidation, repair or rollback.
Child exceptions retain prior FIRST/tree/pool progress; no local EH is added.

The six native external transfers are:

- `006A1ABA CALL 00BF6713` — initial owner diagnostic.
- `006A1AD6 CALL 00BF6713` — full-range end-owner diagnostic.
- `006A1AEA CALL 006A0770` — existing actual whole subtree.
- `006A1B2A CALL 00BF6713` — partial-range owner diagnostic.
- `006A1B39 CALL 0069C0F0` — mutate actual FIRST.
- `006A1B47 CALL 006A04A0` — erase saved OLD through the actual preset eraser.

All 201 envelope bytes normalize to published whole `006A1B70` after only six
individually resolved E8 operand fields: 24 mask bytes, 176 untouched DEFINED
bytes, and 177 untouched envelope bytes including the excluded NOP. Eleven
bytes actually differ. No opcode, branch, field or broad address mask is used.
Source instantiates that proved full algorithm with actual preset services;
it does not delegate to the int/string range or introduce a nil/layout adapter.
Fresh PE/live/current-listing proof covers the 200 owned bytes/76 starts plus
read-only reference 200/76, subtree 82/34 and bounded producer 52/23: 534
defined bytes/209 starts, with two excluded NOPs. All 18 support-inclusive
external targets match the current listing. The producer confirms five
physical words and private output; it is not promoted to reconstructed Source.

The default MSVC Win32 build passed all three existing suites in 8.06 seconds
of CTest time. No new tests, ad-hoc executable, native fixture or game run was
added. The existing unrelated duplicate spawn-helper LNK4006 warning remained
nonfatal. The report verifier checks six JSON transfers and six Markdown claims.

The complete actual CMake object contains five functions: 267 code bytes and
115 instructions. Public range is 220 bytes/90 instructions, including one
two-byte alignment NOP; four raw integer helpers total 47 bytes/25 instructions.
Every code byte and all sections/relocations were inspected. There is no
local EH/FS/cookie code, hidden call, unwind state or catch. Both public returns
are RET18h. All six REL32/addend-zero bindings name the actual published preset
subtree/successor/eraser and diagnostic APIs. The full compiled reference has
identical 267 code bytes/115 instructions without masks; explicit five-function
and six-callee symbol bijections establish bindings independently of placeholder
equality. Twelve focused checks pass.

The emitted code preserves both returning-diagnostic snapshots, headB before
count/left stores, OLD owner/node in nonvolatile registers through FIRST's
advance, node-before-owner recaches, and output store ordering. Its added RawCtx
push precedes the full root read; private frame/stack and fault timing differ
from Native. Valid reached tree/sentinel/iterator/output storage and compatible
current-CRT ownership/actual RawCtx cells are required. Added context, private
frames/argument aliases, dead EDX canonicalization, recursive/callee origins,
register flags, CurrentCpp/noexcept/termination, original heap/CRT/FH3/SEH/RTTI/
fault behavior, binary replacement and game equivalence remain unproved. No
x87 operation or new NaN policy occurs.

The baseline is published Main `44b08ee2f426ca45ec791e4c4d0804d9f809082d`.
All 30 direct/transitive/reference/preserved-AV provider files retain published
LF content; the report distinguishes physical and blob hashes. Registration
is one local bsp_core append, with no shared registry lease. Root preliminary
full Source/header and actual COFF review passed 12 checks with no findings;
primary review of the immutable pin remains pending. No caller, member, or
`006AA460` owner is promoted by this packet.
