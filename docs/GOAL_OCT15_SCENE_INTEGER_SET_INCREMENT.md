# Scene integer-set increment

Addresses: 00466AE0, 00467B30, 00468340.

| Native address | Source API | Executed span | Coverage |
|---|---|---|---|
| 00466AE0 | increment_native_scene_integer_set_iterator_00466ae0 | 00466AE0..00466B42, 99 bytes | complete |
| 00467B30 | increment_native_scene_integer_set_iterator_00467b30 | 00467B30..00467B3B, 12 bytes | complete |
| 00468340 | increment_native_scene_integer_set_iterator_00468340 | 00468340..0046834B, 12 bytes | complete |

Native ECX points to actual eight-byte backing: owner DWORD+0, live node DWORD+4.
Incoming EDX is unused; the core has no defined EAX result, plain returns and
one tail validation transfer. The wrappers capture original ECX, call increment,
then return that same backing address in EAX with plain RET. Correct library
names are preserved; the Source names describe their body/layout behavior.

If owner is zero, the concrete current-CRT entry is invoked and may return.
Only then is CURRENT iterator.node loaded. If its nil11h is nonzero, the native
tail reports and returns; Source likewise makes no subsequent publication.
Otherwise the body reads current.right. A real right child descends left links
until its next child is nil, then publishes the last real node. With nil right,
the body captures current.parent, tests that parent's nil, reads CURRENT
iterator.node and parent.right, and publishes parent when equal. Only after that
store does it reload parent.parent, preserving aliases. It finally publishes
the captured parent unconditionally, including a sentinel parent. There is no
decrement-style final-node validation, owner reload, snapshot or added guard.

The actual14h node producer 004681B0 writes links0/4/8, key0Ch, color10h and
nil11h. This separate Source does not invoke the accepted nil19h record-map
increment. Its full38-op native schedule matches that provider with the actual
nil displacement; native rel32 transfers are separately verified. The core's
99 bytes include a reachable three-byte neutral LEA ECX,[ECX]. Both wrappers
add24 bytes; no interfunction padding is counted.

All live callers were read. Single erase 0046A740 reaches
`0046A7BE CALL 00466AE0` with its actual stacked iterator through ECX=ESP+6Ch;
it ignores child EAX and continues reading the captured original node. Range
erase 0046B170 reaches `0046B209 CALL 00466AE0` with actual first-iterator
backing through ECX=ESP+20h, ignores EAX, then reloads current owner/node after
the following erase. Both wrappers pass unchanged incoming ECX and return
its captured address; neither has a live incoming xref. The JSON report has
every exact call/callee/containing-function row and argument/result evidence.

The only dependency is concrete accepted invoke_native_invalid_parameter_00bf6713
over the actual current SDK _invalid_parameter_noinfo API. The owner report is
a CALL atAE8 with fallthrough AED. The nil-node report bytes atAF7 are a JMP
with existing CALL_RETURN override, effective CALL_TERMINATOR and no default or
effective fallthrough. Typed metadata confirms that existing state; no flow or
prototype repair is performed. The core and both wrappers are returning,
non-thunk functions with complete one-range bodies. Five CALL edges and one
explicit tail-JMP edge are recorded. Current CRT handler selection and delivery
remain qualified against native encoded globals, Watson, debugger and FH3.

Guarded live batches verify C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe, x86 image base00400000. Locked annotation records
prior values, preserves existing names/comments, saves the project and refreshes
owned exports. Full native assembly, typed metadata, all caller shapes and
live/installed123B equality accompany the Source draft; Win32 build and existing
checks are pending. No new routine tests are added. Every taken access requires
valid backing. Compilation/existing checks do not establish private-frame,
mangling, flags/fault/SEH identity, native binary ABI, dedicated native differential,
application adoption or game behavior.
