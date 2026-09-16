# SkinedMeshAnimation source dispatch composition

`native_skinned_animation_dispatch.hpp/.cpp` binds the reconstructed animation
item reader and lifetime into the existing resource dispatch interfaces.
It completes the item-family dispatch gap identified in the Object model audit;
application bootstrap must still install these adapters in its actual chain.

## Established targets and ownership

The live `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` profile bytes
establish `D630D8+8 = B92F20` and `D63700+0/+4 = BD30E0/B92FA0`.
`NativeSkinnedAnimationResourceCalls` routes exactly B92F20 to the actual item
parser, borrowing the existing structured stream and raw string contexts.
That native parser does not use its parser receiver. Other parsers, renderer
hooks and resource appends forward their original arguments to the next binding.

`NativeSkinnedAnimationReferences` receives the zero-reference terminal after
the caller decrements the item's actual reference count at +4. It adds no
decrement, retain, registry, pool or replacement owner. For captured D63700 and
target BD30E0 it preserves BD30E0's null guard, reads the CURRENT item profile,
and reads the borrowed profile's CURRENT +4 target. The supported terminal is
B92FA0 with flags 1. The complete 14-byte BD30E0 listing establishes this call
and contains no reference-count operation. Other captured domains forward.
A changed reached profile or scalar-delete slot is an explicit unsupported
source boundary; the adapter does not execute numeric original addresses.

The parser and destructor retain the publication and exception ownership of
the [thirteen reconstructed bodies](NATIVE_SKINNED_ANIMATION_ITEM_ORCH4.md).
No cleanup, failed-parse rollback or failed-reference retry is added here.
The same raw string publications must remain alive for reading and destruction.

## Validation and limits

The full Release MSVC Win32 build and all three existing CTests passed.
An ignored focused probe reused the item packet's fixture and linked the final
`bsp_core` library. It entered through the registered parser adapter, checked
two records, outer relocation, 24 float values, an embedded-NUL name and full
stream-budget consumption. A second parsed item reached scalar deletion after
one actual interlocked +4 decrement; the terminal null guard also returned.
The fixture retained the earlier flags-2, vector differential and partial-read
failure checks. Free-ring size totals accounted for all raw-pool arena bytes
after complete teardown. No permanent test was added.

Only the two earlier inner-vector bodies are copied-original comparisons.
The dispatch and parser checks execute reconstructed source with a fixture
stream provider. They do not execute the original parser or Windows FH3.
Application-chain installation, real VFS/resource input, arbitrary profile
changes, original ABI compatibility, rendering and gameplay remain unvalidated.
No new native body is counted for these host composition classes.
