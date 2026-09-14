# Raw structured-node release and CRT prerequisite discovery BJ

Base `eaa1f87cc5594484494824afe5d1fa7f2efb5545`. Discovery only; no C++ changes,
Ghidra mutations, tests, build or game execution. The companion report retains
all 19 direct call sites, three separately qualified indirect sites, source
availability, and the complete SHA256/SHA512 local evidence inventory.
Eleven complete code/data spans, 402 bytes, match live Ghidra and the installed
PE with SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Release contracts

| Entry | Complete span | Native interface |
|---|---|---|
| BE9ED0 | BE9ED0..BE9EF8, 41 bytes | ECX valid four-byte wrapper; no stack arguments; RET |
| BD30E0 | BD30E0..BD30ED, 14 bytes | ECX actual node; no incoming flags; RET |
| BE9FC0 | BE9FC0..BE9FDD, 30 bytes | ECX actual node; one flags DWORD; EAX original node; RET4 |

BE9ED0 captures `[wrapper]` once. A null node returns without any store. For a
nonnull node it invokes actual KERNEL32 `InterlockedDecrement(node+4)` through
IAT CE2220. Only a result exactly zero dispatches current node vslot00, with
ECX the captured node and no deleting flags. Any nonzero result, including a
negative count, skips dispatch. Both returning paths then clear the wrapper.
The final clear overwrites any wrapper changes made by a returning callback.
It does not preclear, check underflow, retain, retry or validate node storage.
There is no semantic return value established for this routine.

Verified D68BB4 bytes contain slot00=BD30E0 and slot04=BE9FC0. BD30E0 has a
null guard; otherwise it **reloads the current profile** and its slot04, pushes
flags1, and calls it. The earlier slot00 lookup must not be reused to select
slot04. Native address words remain profile evidence, not callable pointers
inside the reconstructed executable.

BE9FC0 captures ECX in ESI, calls BE9DF0, and only after it returns tests bit0
of the stacked flags. A set bit calls BF65AC with the captured pointer and
cleans that one cdecl argument. It returns the original pointer even after
free. It adds no null guard, array-delete branch, duplicate detach, generic raw
destructor or host C++ object destructor.

None of these three complete bodies creates an EH frame. If the zero callback
propagates an exception/fault, BE9ED0 has not cleared its wrapper. If BE9DF0
propagates, BE9FC0 has not tested flags or freed. Existing BE9DF0 source already
preserves its base-stamp cleanup with `__try/__finally`; original FH3 handler
identity remains separate from that qualified source interface.

## Source readiness and concrete missing providers

Existing `native_structured_node_destruction.cpp` supplies the actual BE9DF0
schedule and `native_ref_counted.cpp` supplies BD30F0. Its BD30E0 source still
delegates to abstract `NativeRefCountedDeleteCalls`; no concrete D68BB4 resolver,
BE9FC0 source, or BE9ED0 source was found at this base. Supplying a new generic
delete/free callback would not establish their actual lifetime contract.

The smallest prerequisite is a concrete raw24h node allocation/free domain,
then concrete D68BB4 slot routing through BE9FC0 and BE9DF0. The constructor
worker owns BF681B/BF65AC/BF9DC8/BF9F1A/C055B1 evidence. The existing
`singleton_lifetime_allocate` uses host `malloc(host_bytes)`, host `_callnewh`
and `std::bad_alloc`; `native_bytes` is informational. Its free uses host
`std::free`. These can support an explicitly qualified rebuilt-process domain;
they do not establish original CRT heap/newmode/handler/exception identity.

There is useful existing exception code: internal `copy_base` in
`native_legacy_exception_owner.cpp` models BF63A6, including nullable allocation
and the post-allocation source-message reload. Its public consumers own 28h
logic/length-error storage. It is not an exported 0Ch bad_alloc lifetime or
native throw provider.

## Additional BF681B failure-path prerequisite

While the release addresses were owned by the constructor worker, this packet
independently inspected the unowned atexit path. BF6FF5 forwards its callback to
BF6FB9 and maps nonnull to0, null to-1. BF6FB9 acquires actual CRT lock8 through
BFBA68/C11C21 before arming state0, calls BF6ED1, saves its result, disarms to-2
and invokes BF6FEF/BFBA71/C11B31 to unlock. The retained E02CF0 scope table
also selects BF6FEF on unwind. SEH frame helpers and lock ownership are external.

The complete 185-byte BF6ED1 backend decodes encoded start109FED0 and end109FECC;
rejects reversed/wrapped used ranges; queries C07B5C allocation size; and grows
through C048A2 with `old_size + min(old_size, 800h)`, then `old_size + 10h` on
overflow/failure, rejecting further overflow. It preserves used bytes through
the observed signed SAR2 pointer calculation, encodes/publishes a new base,
encodes/stores the callback, and encodes/publishes end+4. It returns the original
callback; a null callback does not become artificial success.

Qualified C04FDE decoder and C04F67 encoder sources already exist, including
their observable caller argument writeback. They borrow actual TLS indices,
IAT, getter/PTD, encoder/decoder and owning-CRT state; they do not initialize
that state. The module fallback gate exists, with explicit owning errno and
invalid-parameter services. Current `std::atexit` adapters and shutdown-token
interfaces do not provide native encoded table ownership, lock8, original
C07B5C msize/C048A2 realloc, or the bad_alloc cleanup identity CE1122.

This is a bounded provider frontier, not a claim that original CRT, native ABI,
EH continuation, executable startup or gameplay reconstruction is complete.
