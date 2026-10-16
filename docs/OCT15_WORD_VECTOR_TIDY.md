This packet reconstructs two complete ordinary cleanup entries, 004D29D0 and
004CDCE0, through the accepted whole ordinary raw tidy service. It preserves
their correct `STL_Vector_Tidy` library names. New C++ names describe observed
effects and are hypotheses. No parent range core or compiler EH action is
included.

004D29D0 spans 004D29D0..004D29FD, 46 bytes and 13 instructions. After saving
ESI it reads the sole stack argument at ESP+8, which is entry ESP+4. Incoming
ECX can carry a caller owner but is unread; EDX is unread. It returns with
`RET 4`. Its new Source interface uses ignored ECX/EDX slots followed by the
stack header. Those dead registers do not recover an original C++ declaration,
owner type or mangled ABI.

004CDCE0 spans 004CDCE0..004CDD09, 42 bytes and 13 instructions. It captures
the actual header from ECX, ignores EDX, consumes no stack arguments and uses
plain `RET`. Both entries save and restore ESI.

Each reads DWORD[header+4] exactly once. A nonzero captured pointer is passed
to the actual free service. The null path skips that operation only. Both
returning paths store zero at the captured header's +4, then +8, then +0Ch.
There is no second backing-pointer load, direct +0 access, element/key cleanup,
owner read, validation, diagnostic, allocation, rollback or extra null guard.
The header address remains the reset target even if a returning free changes
its fields. The header must remain valid and writable through those stores;
its nonzero backing must be admitted to the current CRT free domain.

The Source wrappers directly use `tidy_native_dword_vector_004cb130`, which
directly delegates to `tidy_native_vector_storage_00468840`. That accepted
service captures a volatile raw DWORD, conditionally uses
`singleton_lifetime_free`, then performs the three ordered volatile stores.
The accepted free adapter uses current `std::free`. No replacement allocator,
provider, callback, typed/count bridge or duplicate body is introduced.

The complete 004CDCE0 body differs from each accepted reference only in the
four-byte relative free operand at offsets 0Ch..0Fh. Each displacement was
resolved independently from PE and live bytes. The 004D29D0 comparison uses
all 13 ordered instructions: only its stack-header load, that resolved operand
and `RET 4` differ. Every other instruction byte is identical.
It is not the same physical body as the receiver-only entries.

All actual transfers, including read-only accepted references, are checked:

- 004D29DD CALL 00BF65AC, captured backing free.
- 004CDCEB CALL 00BF65AC, captured backing free.
- 004CB13B CALL 00BF65AC, accepted reference.
- 0046884B CALL 00BF65AC, accepted reference.

Native EAX remains zero after a null capture; otherwise it retains incidental
void-free residue because the subsequent immediate stores do not replace it.
The Source interfaces are void and promise no ambient EAX/flags. The two known
stack-header consumers discard EAX while walking their completed prefix and
rethrow afterward. The receiver-only entry is used by private unwind actions.
Those callers' frames, handlers and parent completion are outside this packet.

Neither complete leaf contains a local EH frame, handler, action, try or catch.
No native flow or listing repair is needed. Current C++/CRT and admitted raw
storage behavior do not establish original CRT/heap identity, fault/SEH,
FH3/RTTI, private frame/argument aliases, mangling, binary ABI or gameplay
equivalence. No element type is inferred from this raw cleanup schedule.

The report retains fresh project/program verification, full installed/live
bytes and current listing starts, all provider fingerprints and complete
comparison provenance. The exact Source built with MSVC Win32; all three
existing suites passed in 7.80 seconds. The emitted stack-header wrapper is
15 bytes/six instructions: read its stack header into ECX, invoke the accepted
service, and return with one stack word removed. The receiver wrapper is a
five-byte direct delegation branch with ECX and entry ESP unchanged. Both
external relocations name the accepted service; neither emits local C++ EH
metadata or an additional child. The existing unrelated Lua duplicate-symbol
linker warning remains recorded. No new test or native/game execution is
part of this packet.
