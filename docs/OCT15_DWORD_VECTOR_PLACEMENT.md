This packet supplies two actual ordinary placement-copy wrappers and records
two compiler placement-unwind effects as elided fragments. It preserves the
existing `STL_ConstructWordVector` identities and WordVectorHooks/typed APIs.
New C++ descriptions are hypotheses. No parent range core is included.

00697D40 spans 00697D40..00697D7F, 64 bytes/17 instructions. Destination is
ECX, source is EDX, there are no stacked inputs, and the normal return is RET.
The wrapper registers its exception node, saves destination twice, tests it,
and arms state0 before the null branch. Only nonnull pushes the original source
address and enters the accepted actual constructor. The normal epilogue
restores the previous FS0 node and discards its 14h private bytes.

00697EC0 spans 00697EC0..00697F07, 72 bytes/19 instructions. Destination and
source are TWO stacked words, entry ESP+4/+8; the normal return removes both
with RET8. Incoming ECX is used only to reserve scratch space before overwrite;
EDX is not a semantic input. The wrapper reads destination, redundantly writes
its native argument slot, saves it locally, tests it and arms state0. It reads
the source ARGUMENT only on the nonnull path. It does not read the source
object. The native argument-slot self-store belongs to its private frame
contract and is not translated into an invented public memory operation.

Both Source wrappers are void. Native null leaves EAX equal to the previous
FS exception-chain value; nonnull leaves the actual child's original
destination result. Neither epilogue synthesizes a return value. Source does
not invent a null result or an ambient EAX input. The register form uses
fastcall; the two-stack-word form uses stdcall. Those new declarations and
their emitted shape are not proof of original mangling or binary ABI.

Nonnull forwards the same destination/source addresses to
`construct_native_checked_dword_vector_copy_00557590`. Zero defines only that
accepted Source callee's ignored EDX slot. No eager source-object read,
snapshot, validation, allocator, extra guard, catch, free, rollback or provider
is added. The child preserves its count-before-reset, returning-diagnostic
captures/current reloads, alias behavior and actual ownership policy.

The child may propagate its uncaught length/allocation exceptions, or tidy
the CURRENT destination header and rethrow from its protected copy/diagnostic
scope. The wrapper adds no public cleanup. A returning diagnostic remains a
returning diagnostic under the accepted child contract; it is not converted
to failure or normalized storage. Termination and native faults remain
qualified by that accepted Current C++/CRT boundary.

The complete raw map at DAB984 has state0 previous=-1/action C7EBD0 followed
by FuncInfo DAB98C: magic19930522, maxState1, no try blocks and EHFlags1.
The handler C7EBE1 loads that exact FuncInfo. DAB9B0 and FuncInfo DAB9B8 have
the same structure with action C7EBF0 and handler C7EC01. Together these are
88 raw bytes, verified against installed PE and live memory. The two wrappers
arm those exact states; this is an unwind action, not a local catch.

Let S denote native entry ESP. The register form saves two placement words
at S-14h/S-10h; both hold destination. The stack form retains destination at
S+4 and saves it at S-10h. Both have previous FS0 at S-0Ch, handler at S-8,
and state at S-4. The private runtime action frame supplies the native EBP
base; these offsets are evidence about that frame, not a Source EBP argument.

C7EBD0 spans C7EBD0..C7EBE0, 17 bytes/seven instructions. It reads [EBP-14h],
pushes it, reads [EBP-10h], pushes it, invokes the literal no-op, balances eight
stack bytes and returns. C7EBF0 spans C7EBF0..C7EC00, also 17 bytes/seven
instructions; its first read is [EBP+4], with the rest identical. These are
compiler effects fragments, not normal standalone Source interfaces.

All actual transfers, including read-only handler support, are checked:

- 00697D6C CALL 00557590, actual constructor.
- 00697EF2 CALL 00557590, actual constructor.
- 00C7EBD8 CALL 00401130, literal no-op.
- 00C7EBF8 CALL 00401130, literal no-op.
- 00C7EBE6 JMP 00BF6B43, original frame-handler support.
- 00C7EC06 JMP 00BF6B43, original frame-handler support.

The COMPLETE PE/live/current body of 00401130 is one byte, C3 RET: it consumes
no argument, performs no destruction/free/callback and has no public memory
effect. In valid private frames/stacks, the actions therefore add only inert
private reads and balanced argument temporaries. Source can effect-elide them
and propagate the same C++ exception. That argument does not reproduce private
frame aliases, invalid-frame faults, ambient registers/flags or original FH3.
No public native EBP/frame helper, destructor, generic no-op stub or original
handler bridge is created. The two handlers and literal RET are read-only
support, 21 bytes/five starts; proposed owned coverage is 170 bytes/50 starts,
split 136/36 ordinary and 34/14 logical actions.

Historical EC0 wording saying “same body as 00697D40” is physically incorrect.
The report retains its old evidence and comments before appending correction.
The existing WordVectorHooks header already records distinct fastcall versus
stdcall/RET8 shapes; that correct statement and both callback projections
remain unchanged. They do not provide the new callback-free ordinary binding.

Source inherits the accepted constructor's current CRT allocation/free,
new-handler and owning legacy exception policy. Original CRT/heap, FH3/RTTI,
SEH/fault, FS chain, private frame/argument aliases, ambient registers/flags,
mangling, binary ABI and game equivalence remain unproved. All reached raw
storage must satisfy the actual constructor's valid/writable and finite-copy
preconditions. Invalid native stack/argument behavior is not repaired.

The exact Source built with MSVC Win32; all three existing suites passed in
9.06 seconds. The emitted register wrapper is 13 bytes/six instructions:
test destination, skip all child work on null, push the original source, define
ignored EDX0, invoke the accepted constructor and return with plain RET.
The stack wrapper is 24 bytes/ten instructions with a balanced frame and RET8.
Its source-argument push occurs only after the destination test and null branch;
the null path skips that read and all child work. Both external relocations
name the actual accepted constructor. Complete object inspection found no
local C++ EH metadata, other child, action/frame helper, free or callback.
Constructor C++ exceptions therefore have no wrapper interception or cleanup;
this structural proof is not an exception runtime fixture or original FH3 proof.
The existing unrelated Lua duplicate-symbol linker warning remains recorded.
No new test or native/game execution is claimed.
