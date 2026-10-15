# Actual DWORD in-place ranges

This packet reconstructs complete ordinary entries `006970B0` and `00697440`.
Established STL names are retained. Source names describe their proven
behavior and remain hypotheses, not recovered symbols. Existing qualified
interfaces and fixtures remain separate.

| Entry | Native interface | Complete body |
| --- | --- | --- |
| 006970B0 | cdecl(first,last,value address), plain RET; EAX terminates at last | 30B/13 instructions |
| 00697440 | cdecl(first,last,destination end), plain RET; independently computed EAX | 53B/24 instructions |

Incoming ECX/EDX are unconsumed. Source returns the observed stable EAX
addresses, including fill's result which its producers discard. Complete
typed bodies, installed/live bytes and all37 owned export/current instruction
starts agree. The supporting complete `0043F9E0` fill matches all30 bytes of
`006970B0` exactly; no masked address or partial-body comparison is used.
Source still supplies its own direct loop and introduces no helper dependency.

Fill captures last, skips the value-address capture and pointed accesses for
an empty range, otherwise captures the value ADDRESS once. It reads the
current DWORD immediately before each store, then advances by4 until equality
with captured last. A destination or value pointer is not given an extra null
guard. Source/value overlap retains the progressive reads and writes.

Backward copy captures first/last/destination end and computes its return
BEFORE any pointed operation: `destination_end - (SAR2(last-first)*4)`, all in
DWORD arithmetic. The explicit x86 SAR preserves arithmetic rounding and wrap;
there is no signed-overflow C++ expression or byte-distance substitution.
On nonempty input it captures `destination_end-last`, then reads
`current_last-4`, decrements the source cursor, compares it with captured first,
and stores through the displacement plus decremented source. It repeats using
the current source word on each iteration. Progressive right overlap is
preserved without a memmove/std::copy model, snapshot or overlap repair.

Neither complete body has any call, external tail transfer or local EH/catch.
All three consumed producer sites are in the second normal range of `00697990`,
whose older snapshot graph incorrectly assigned some children to a catch.
Current instruction strings and installed rel32 targets agree:

```text
00697B61 CALL 006970B0
00697BB0 CALL 006970B0
00697BA0 CALL 00697440
```

The packet report separates37 owned starts,13 supporting fill starts and the
three incoming calls. It records the actual raw bytes and all current query
matches. It does not promote the general insertion parent, allocation/error
dependencies, wrapper or append.

Reached raw4B storage must support all ordered accesses and equality traversal
must terminate. The new Source/private argument/helper frame conventions do
not establish native register/flag residue, private-frame alias, asynchronous
mutation, fault/SEH, binary ABI or gameplay identity. No allocator, callback,
guard, catch, retry, rollback or new test framework is introduced. Existing
native math differential tests exercise older seeds, not these new entries.

Win32 build, existing suite results and the independent fourteen-check
Source/native review are recorded in the report separately from byte evidence.
