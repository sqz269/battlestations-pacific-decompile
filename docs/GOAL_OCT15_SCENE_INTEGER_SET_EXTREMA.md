# Scene integer-set extrema

Addresses: 00466A40, 00466A60.

| Native address | Source API | Functional coverage | Envelope |
|---|---|---|---|
| 00466A40 | maximum_native_scene_integer_set_node_00466a40 | complete, 25 executable bytes | 28 bytes, excludes unreachable A4D..A4F |
| 00466A60 | minimum_native_scene_integer_set_node_00466a60 | complete, 27 executable bytes | 27 bytes, includes neutral six-byte LEA |

The maximum body reads node.right at +8 before testing that child's nil byte
at11h. While zero it advances current to child and reads the next child and nil.
The minimum body uses left at +0 with the same schedule. Each returns current,
not the nil child, through EAX. Neither tests the initial node's nil byte.
Native inputs are ECX node, unused incoming EDX, no stack arguments and plain
RET. The Source fastcall interfaces expose the result consumed by actual
callers; current void pseudocode is not a reliable return-type inference. No
live signature/prototype change is made. The correct native STL names remain.

The actual14h producer 004681B0 writes links0/4/8, key0Ch, color10h and nil11h.
Extrema only read links and nil. Both raw Source bodies are separate from the
accepted nil19h record-map providers. No semantic owner, comparator, callback,
STL replacement, added validation or initial-sentinel shortcut is introduced.

All live callsites belong to single erase 0046A740. The minimum call atA823
receives EDI through ECX and stores its EAX result into the captured head.left.
The maximum call atA845 receives EDI through ECX and stores its EAX result into
the captured head.right. Both have no stack arguments and plain RET. The JSON
report records exact call/callee/containing-function rows and original bytes.

Typed live metadata confirms maximum has exactly two executable body ranges,
A40..A4C and A50..A5B, totalling25B. AtA4B an unconditional jump with no default
or effective fallthrough skips the three-byte `8D 49 00` alignment atA4D..A4F;
there is no instruction atA4D. These bytes are retained as original envelope
evidence and excluded from functional coverage. Minimum's27B includes reachable
six-byte `LEA EBX,[EBX]` atA6A, a register/flags/memory identity operation. Thus
the packet covers52 executable bytes in55 original envelope bytes, not55
functional bytes. Both functions are returning, non-thunk, with complete typed
body ranges. No analysis or listing repair is needed or performed.

Guarded live batches verify C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe, x86 image base00400000. Locked annotation preserves
existing names/comments, records old values, saves the project and refreshes
owned exports. The Source draft includes full assembly, producer/caller/typed
metadata and live-versus-installed PE bytes; Win32 build and existing checks are
pending. No new routine tests are added. Valid backing is required for each
taken access; compilation and existing checks do not prove private-frame,
mangling, fault/SEH identity, native binary ABI, direct native differential,
application adoption or game behavior.
