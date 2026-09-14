# Actual resource loader and cache composition

Addresses: `00B80720`, `00B7E8F0`, `00B7E910`, `00BE2700`; unwind evidence `00CC2170`, `00CC217B`, `00CC2183`, `00CC218E`, `00CC2199`, `00CC21A4`.

The new `native_resource_load_cache` module composes the existing actual raw cache, resource factory, VFS name resolution, reader, root and item dispatch bodies. B80720 is811 bytes; the two name-only pair destructors are29 bytes each; the statistics method is30 bytes. All899 ordinary bytes,284 ordinary instruction starts and12 support instruction starts match the installed PE and saved Ghidra owners. The report records all38 direct transfers (including support tails) and7 indirect calls. Names are descriptive hypotheses. The original game was neither executed nor modified.

## Inputs, publication and ownership

B80720 consumes ECX actual28h manager, stacked actual8h name-header pointer and factory, returns EAX resource and RET8. The source adds borrowed service contexts and an immovable invocation frame. It preserves these stages:

1. Publish factory at manager+20 before looking up the ORIGINAL name in cache+14. A hit validates captured iterator identity, publishes its resource at+24, atomically increments resource+4, and returns CURRENT+24. It leaves factory+20 set.
2. On a miss, read CURRENT0109CEFC and its current slot4, then CURRENT manager+20 factory/slot4. Publish the factory result at+24. There is no resource-null gate or rollback.
3. Copy the input name and call BDF4C0 through CURRENT0109CEEC. Ignore its boolean result; reload CURRENT0109CEEC/table/slot4 and open the copied name with flags2. Construct actual70h reader, assign the stream, copy the resolved name into reader+68, then decrement the captured open result without a null guard. A zero count dispatches its current slot0. No owning stream slot is invented.
4. Create the actual root and run B7F430. Capture CURRENT manager+24 only afterward. Copy ORIGINAL input name into the native by-value pair constructor, copy that completed pair into the insertion pair, and call B803B0. Ignore duplicate/inserted output; neither cache insertion nor pair construction adds a resource reference.
5. Disarm and return the insertion pair's captured data with its current length; disarm and return the first pair's captured data. Clear factory+20, query CURRENT statistics owner, and write wrapping starting-minus-ending DWORD into CURRENT resource+40. Capture CURRENT resource for return BEFORE root/reader/resolved-name cleanup, so a cleanup callback can change manager+24 without changing the return.

BE2700 zeroes a34h stack block through memset and subtracts its DWORD+1C from40000000. The whole30-byte listing contains no OS memory query. The ordinary source result is therefore40000000, and two unchanged calls yield a zero resource metric. Its finite binding is based on actual D685F4 slot4=BE2700, not an assumed live-memory statistic. Other current metric identities remain explicit calls. Known factory slots B88340 and71B870 compose their existing source constructors; opening delegates the captured target to NativeVfsRuntimeBindings. Numeric targets are never executed directly.

## Cleanup evidence and limits

Handler CC21A4 loads FuncInfo DFB3BC and tail-jumps to BF6B43. Its five-entry map at DFB3E0 is state0->-1/CC2170 (resolved name EBP-B8),1->0/CC217B (reader EBP-7C),2->1/CC2183 (root EBP-A4),3->2/CC218E (first pair EBP-94),4->3/CC2199 (insertion pair EBP-B0). The previously undefined complete10-byte handler was defined under the write lock; no byte deletion or no-return edits were needed. Every prior plate comment is retained.

The source applies these five states to ordinary C++ exceptions and disarms each action before a normal cleanup call. A second cleanup exception terminates. Initial copies are unarmed until the original state transitions; there is no added partial-name, completed-resource or cache rollback. B7E8F0/B7E910 return current name storage through the current pool, leaving stale headers and the borrowed resource word intact.

The nested VFS resolver retains its existing failure-frame contract: if resolution itself fails by exception, its invocation and caller storage must remain retained; destroying that failed provider frame terminates. This source integration does not convert that boundary into native FH3 recovery. Native exception-object/handler identity, original private-stack aliases, hardware faults, arbitrary callback mutations and whole-loader binary ABI compatibility are not certified.

## Validation

The strict MSVC Win32 build and both existing CTests pass. One ignored controlled-child fixture uses actual string pools, resource factories, cache nodes, memory streams, reader/root/child owners and unknown-resource item dispatch. It checks three misses and one hit: original backslash-containing cache key after name normalization; continuing after false resolution; refcount/factory persistence on hit; a factory changed by the initial metric callback; separate resource changes at renderer begin, ending metric and stream cleanup; wrapping5-8 metric; and parser failure after an actual DWORD read. The failure case closes child/root/reader/name ownership, omits renderer-end, and retains the factory and created resource. Stream counters return to zero. Both pair cleanup entries retain their stale header and borrowed value. A relocated original30-byte BE2700 body, using canonical memset, agrees with its source result.

The empty VFS lookup tree and the open/parser callbacks are explicit fixture boundaries. Production VFS open dispatch is compiled but not exercised by this fixture. There is no original B80720/FH3 oracle, new repository test suite or gameplay claim. The fixture explicitly dismantles resources afterward; that is not a reconstruction of the full successful-resource destructor. Concrete parser construction/bodies, actual manager bootstrap, resource destruction and loading-queue shutdown remain follow-up work.
