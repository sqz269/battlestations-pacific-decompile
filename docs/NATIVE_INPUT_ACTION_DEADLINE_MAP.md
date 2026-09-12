# Native input action deadline map lifetime

Addresses: 00CC9E30 and 00CD9EC0. Source names are descriptive hypotheses.
This packet completes the static lifetime of the actual 12-byte header at
E18A7C. It does not implement the map subscript or the game deadline callback.

| Entry | Native ABI and coverage |
| --- | --- |
| CC9E30 | No inputs, no stack arguments, EAX actual CRT atexit result, RET at CC9E6F. Complete valid static-instance initialization. |
| CD9EC0 | No inputs or stack arguments; EAX0 on normal return, RET at CD9EFF. Complete valid static-instance teardown. The source void CRT callback does not expose incidental EAX. |

The host binds the sole canonical header before initialization and keeps it
alive through CRT atexit processing. The binding stores only its borrowed
address. There is no second header, map, node owner, callback collector, manager
or game-clock state. Call initialization and destruction once; the native body
has no idempotence or null-head guard. An automatic application member that dies
before CRT shutdown does not meet this lifetime contract.

The original header occupies E18A7C..E18A87. Its opaque leading DWORD is not
written by either routine; head and unsigned count occupy+4/+8. The analysis
image has all12 bytes zero before the initializer. The source consumes the
caller's preimage instead of inventing a second static instance. CE2828 contains
the initializer pointer CC9E30 in the CRT initializer table.

CC9E35 calls4C27A0 to allocate the actual18h sentinel. That library contract
initializes links+0/+4/+8 to null, color+14 to1 and nil+15 to0; key/value words
at+C/+10 and padding+16/+17 remain allocation preimages. CC9E3A publishes the
returned allocation as head. The initializer sets nil1, reloads head between
self-links at+4,+0,+8, clears count, then passes the actual destructor to the
returning CRT atexit routine BF6FF5. Registration returns0/-1. A registration
failure leaves the initialized allocation in the header; no rollback is added.
Allocation failure occurs before publication. Neither failure was injected.

CD9EC0 captures head and its minimum link before calling4D2000 with the actual
full-range iterators and an8-byte stack output. That call takes ECX=tree and
five stack DWORDs, with RET14h. After it returns, CD9EE0 reloads current head,
CD9EE7 frees that address through BF65AC, CD9EEC cleans its one stack argument,
and CD9EF1/CD9EF6 clear head/count. Values are scalar words and are never freed
as pointers. The header's first DWORD is preserved.

The source reuses existing raw-tree contracts rather than porting STL:

| Native contract | Existing source provider | Byte evidence |
| --- | --- | --- |
| 4C27A0 sentinel allocation | 86AC00 raw18h allocation |55 bytes identical except CALL displacement |
| 4D2000 range erasure |86EE50 raw18h range erasure |201 bytes identical except CALL displacements |
| 4C18D0 subtree cleanup |86AA60 raw18h subtree cleanup |53 bytes identical except CALL displacements |

Each compared span also matched live analysis bytes and the installed image.
The reachable full-range branch frees right subtree, captures left, frees the
current node and continues left, then resets the sentinel links/count. The
range helper's alternate partial-erasure branch calls4CFB70; that instantiation
has a different native EH prolog from86E8A0 and is not claimed equivalent here.
It is unreachable for the valid captured full range supplied by this destructor.
Concurrent header corruption, native FH3/SEH and arbitrary mutable iterator
aliases are outside this static-instance source contract.

Primary repaired the destructor's previously excluded CD9EEC..CD9EFF tail,
including the head/count clears after returning CRT free. The final saved body
has23 instructions and zero gaps. The worker made no Ghidra writes. The
unowned4C18D0 listing still omits4C18F4..4C18FE; its complete disk/live53-byte
span and established86AA60 provider supply the explicit evidence boundary.
Correct recognized library names are retained, even when an inventory name is
misleading: CD9EC0 performs teardown despite its old `CG_static_init` name.

Validation: strict Win32 Release build and both existing CTests passed; all8
native seeds matched disk. One ignored manifested fixture linked only selected
worktree headers/archives. It used real std::atexit registration, one actual18h
sentinel and three manually populated valid raw nodes, then verified head/count
were zero and the opaque DWORD unchanged after the registered destructor ran.
This proves source static lifetime, not insertion, native differential, failure
handling, SDK, application or game behavior. The runner is
`local/run_native_input_action_deadline_map_probe.ps1 -CoreWorktree <tree>`.

4D6900 remains an explicit library-storage dependency. It is a signed-int to
float map subscript over an actual header supplied in ECX, with one stack key
pointer, EAX mapped address, RET4. It performs lower-bound search, copies the
key and positive-zero value before missing-key insertion, then executes two
real returning-capable invalid-parameter checks. It may create an entry even
when its caller only reads the resulting float. A future source adapter must
preserve native address identity, key capture/reloads, insertion publication,
length-error/returning-handler effects and mapped-value bits.

The caller audit found two separate storage instances: E18A7C for4D8CD0's16
calls and4D92B0's two edge/repeat calls; actual game+5C8 for the two calls each in
4D9420 and4D9480. A global-only adapter would be wrong. The missing-key path
requires4D3CD0 hinted insertion,4D1F40 ordinary insertion,4CF010 node insertion
and balancing,4C27E0 allocation/copy, iterator helpers4B7520/4B75B0/4B5AC0,
and the existing CRT/native length-error providers. There is no available raw
18h insertion adapter to reuse without another bounded library-contract review.
No empty-table substitute, typed std::map or fake successful insertion is added.
The exact follow-up contract and spans are in the report and ignored
`local/native_input_deadline_map_lookup_next_ag.md`.

The CALL audit checked23 rows with0 failures: two owned destructor calls,
18 comparison-helper calls and three deferred lookup calls. CC9E30 remains an
undefined Ghidra start at worker closeout, so its two calls are separately
recorded with exact disk bytes under `no_ghidra_function`; they are not attributed
to an unrelated enclosing function. Its complete64-byte span ends in the
one-byte RET at CC9E6F (exclusive CC9E70). Primary will define that entry and
rerun its call rows after the worker releases ownership.
