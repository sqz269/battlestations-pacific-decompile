# Raw DWORD-vector push-back

| Native entry | Source API | Span | Coverage |
| --- | --- | --- | --- |
| `0046C450` | `push_back_native_scene_dword_vector_0046c450` | `[0046C450,0046C4B9)` 105B/44ops | complete |

Original ABI is ECX vector16, one stacked live value-pointer DWORD at entry
P+4, and RET4. Neither returning branch establishes a common semantic EAX
result; the new ordinary Source API is void. The correct native STL name is
retained. This packet does not adapt the existing semantic scene factory hook
or invent a nominal element class, ownership rule or enclosing collection type.

The body captures begin+4 once. Size is zero for null begin, otherwise raw
SAR2 of current end-begin. Null begin skips the capacity read and takes the
fallback. Nonnull begin reads current capacity+Ch and compares the two SAR2
bit patterns unsigned. No signed-positive, alignment, count or ordering guard
narrows this decision. Source uses unsigned shift and explicit sign-fill bits.

On the fast path, capture current end BEFORE reading the current actual
parameter pointer. Load its current DWORD, store that value at captured end,
then publish captured end+4 to vector+8. There is no end reload after the
destination store, even when described valid aliases affect vector fields.
No child metadata or private output is inspected on this path.

The fallback captures current end before comparing it against the original
captured begin. If begin>end, the actual current-CRT diagnostic can return;
the captured end still supplies the child position. Only after the diagnostic
returns is the current value-pointer parameter read. Child argument stores
follow native push order: value pointer, captured end, captured tree owner,
then the private output pointer. Real complete One/N insertion handles all
reached paths; this parent ignores its EAX. No added rollback, validation,
callback, stub, host collection or fallback-only projection replaces behavior.

`NativeSceneDwordVectorPushBackFrame` binds the actual parameter cell, separate
private output8 and a lazy One-frame pointer. Metadata is stable/disjoint from
described cells/payload, whose valid aliases remain admitted. Fast path permits
absent child metadata and unbacked private output. On fallback, child metadata
is accessed only after the returning diagnostic and late value capture. The
child owns its nested reached N backing and the full argument-cell mutations
already documented by that real provider.

With P native push-back entry ESP, private output occupies P-8..P-1. One entry
Q is P-24h and its four argument words occupy P-20h..P-11h. Nested N entry is
P-4Ch, with argument words P-48h..P-39h and scalar-value cell P-3Ch. The One
provider's unused Q-4 needs no backing. These are native evidence coordinates;
the new Source interfaces do not impose hidden physical private-stack aliases.

| Containing routine | Native listing | Contract |
| --- | --- | --- |
| `0046C450` | `0046C499 CALL 00BF6713` | Fallback captured begin/end diagnostic; may return. |
| `0046C450` | `0046C4AC CALL 0046B4A0` | One actual output/owner/position/value cells, RET10h; ignored result. |
| `0046C550` | `0046C64F CALL 0046C450` | Receiver+24h, address of current pointer slot, RET4. |
| `0046C550` | `0046CC01 CALL 0046C450` | Receiver+24h, address of another current pointer slot, RET4. |

A third actual incoming transfer is at 0046C548 in the raw 16B wrapper span
0046C540..0046C54F. It prepares address(entry+4), pushes it and advances ECX
by24h, then invokes this helper and executes RET4. Ghidra defines no containing
function for that span. JSON records this separately as no_ghidra_function;
it is not falsely attributed to 0046C550. This packet neither owns/annotates
nor defines/repairs the orphan wrapper. Its call is checked only to the extent
supported by the tool's explicit raw-caller category.

The actual One/N Source and header were already integrated on main18ccd65d4,
with their complete growth, in-place and returning diagnostic paths. Full
listings and Source were independently reviewed earlier; current hashes match.
No dependency is named-but-incomplete at the accepted current CRT/owning throw
boundary. Current heap/CRT/handler policy and Source C++ EH do not prove native
private-frame/fault/register/stack/RTTI/FH3/SEH/binary ABI or game identity.

Live batches verify C:/Users/sqz269/bsp.gpr, /battlestationspacific.exe,
x86 language and image base. Complete live/installed bytes and typed edges,
prior names/comments and Source hashes are recorded in JSON. Annotation uses
the shared write lock, preserves old values, saves and refreshes the owned
export. The primary independently reviewed the complete 44-op contract and
unchanged Source/header at `fa9f7532c`; reviewed hashes are recorded in JSON.
Required `scripts/build.ps1` passed at that Source revision, targeting MSVC
Win32. Both enabled existing checks passed in 7.64 seconds: math and tool tests.
The optional native differential fixtures are not enabled in this worker build.
The existing unrelated LNK4006 duplicate `spawn_request_id_matches` warning
remains. Four full typed rows and the raw orphan's callee-only check passed;
the four documentation claims passed, with one expected no-function warning
for the explicitly unowned raw wrapper span. No new routine tests, original
native ABI, candidate differential or game claim is made.
