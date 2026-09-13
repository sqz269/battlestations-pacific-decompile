# Actual renderer material state caches

Addresses: 00B26500, 00B265C0, 00B26680, 00B22650, 00B226B0, 00B22710, 00B5EA10, 00B5EAA0, 00B5EB40

The three cache entry points now operate on the original renderer's pointer
vectors and actual material-state owners. Descriptive names are hypotheses,
not recovered symbols. These C++ interfaces are not binary replacements.

| Entry | Renderer vector | Row width | Miss statistic |
| --- | --- | --- | --- |
| B26500 render | +1AE8 | 8 bytes | +1CEC |
| B265C0 third state | +1ADC | 12 bytes | +1CF0 |
| B26680 sampler | +1AF4 | 12 bytes | none |

Each vector is the actual 12-byte data/count/capacity header. The original ABI
passes the renderer in ECX and a held input owner on the stack, returns the
held canonical owner in EAX, and removes four stack bytes. Comparators receive
the cached owner in ECX and incoming owner on the stack, returning AL; reserve
helpers receive the vector in ECX and signed requested capacity on the stack.

The comparators capture incoming count before cached count. Unequal counts
fail; equal nonpositive counts succeed without touching row data. Each cached
row searches all incoming rows. Matches are not consumed: `[A,A]` matches
`[A,B]`, while the reverse does not. Reordering is accepted. The first cached
DWORD is captured before the inner search; later DWORDs are read at comparison.

A hit consumes the incoming reference before reloading the current cache row,
retains that current owner, and reloads the returned pointer after increment.
The canonical owner helper resolves a terminal only when decrement reaches
zero. A miss doubles signed capacity with native DWORD wrapping and a minimum
of one, grows if count equals capacity, publishes the pointer and count, updates
only the applicable statistic, then adds the cache reference.

The reserve helpers reuse BF55BE/BF681B allocation and BF6989 freeing through
the existing CRT layer. They reload source/count after allocation, free the old
array, then publish data/capacity. Three saved Ghidra CALL_RETURN overrides at
B2269C, B226FC and B2275C hid reachable post-free publication; the official
flow-repair tool cleared those overrides under the write lock, saved the
project, and refreshed all three exports. The journal is retained at
`local/native-material-state-cache-flow.json`.

`NativeMaterialStateCacheAcquired` records partial publication and reference
consumption. Callers must retain this one-shot frame after a host failure;
there is no replay or rollback. Accessible, valid native extents are required.
Host checks for overflow and corrupt extents are explicit source boundaries,
not recovered exception behavior. A retained unpublished allocation is not
automatically freed or retried.

Validation captured all nine complete machine bodies (1,277 bytes), matched
fresh Ghidra reads to the installed executable, and checked all 12 direct call
rows. Indirect reference operations are recorded separately. One focused
fixture relocates those bodies and compares all three profiles with the exact
production C++ on reordered and duplicate rows, capacity growth, hit reference
transfer and statistic writes. It passed, including 36 canonical terminal
deletions. The initial fixture compiled the new production translation unit
directly beside the existing libraries; combined library validation is recorded
separately in the report when completed. No new permanent test was added.

## Follow-up packets

Connect these actual cache functions to the persistent material-program child
adapter alongside the actual descriptor reader and VFS resolver. Shader compiler
B3C3A0 and a complete cold effect load remain separate dependencies. This packet
does not establish full shader loading or gameplay validation.
