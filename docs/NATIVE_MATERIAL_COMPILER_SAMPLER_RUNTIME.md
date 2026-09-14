# Cached compiler with one source-zero sampler

This fixture extends the completed cached compiler runtime with one root
descriptor sampler: pixel `Diffuse`, source0, index0, and empty sampler/state
lists. The mode descriptor remains empty. It changes no production source,
header, native annotation or reconstruction row. Its base is `09184cc3`; the
earlier runtime tree and immutable archive remain unchanged.

## Stack contract established before execution

Let B be B3B280's ESP after its five entry pushes. The call at B3B328
pushes two arguments, so the callee B5F100 enters with ESP=B-12. B5F10B writes
only the stage byte at B-16. B5F13C subsequently loads the whole DWORD, and
B5F142 copies it to the second word of the eight-byte pass+24 row. Those upper
bytes are supplied input, not boolean padding. The native routines both RET8.

The root call receives `7B2954AA`; the pixel stage changes only its low byte,
producing `7B295400`. The separate empty mode call receives `192EF0C3` and leaves
it unchanged. These are explicit fixture-entry preimages, not captures from an
executing original whole compiler. The source contexts preserve their exact
independent DWORD storage and remain alive with the canonical compiler frame.

Source0 requires no acquired sampler texture or post-release scratch record.
Source1 would additionally require its real loader/acquisition/release and an
identity-ordered post-release B-16 capture. That branch is outside this case.
No missing callback, zero residue or fake release is supplied to reach a pass.

## Actual source compiler execution

The same B3C3A0 wrapper, persistent builder/local name and B3B536 cached
continuation execute through normal completion. The actual sampler uses the
existing B57B50 allocation-initialization fragment and actual string producer.
Its two distinct state-list headers come from the canonical 0108FEE4 pool and
B621A0. Descriptor C4/count/capacity are explicit one-entry fixture inputs;
the Lua sampler parser is not executed. Original numeric profile identities
remain in storage; no host callback replaces a table token.

Real D3DXCompileShader creates the cache input PS containing `Diffuse` at s0
and `Tint`. The actual compiler performs cache lookup, reflection, COM shader
creation, canonical publication, assignment and temporary releases. Reflection
writes mask1, so native sampler pruning retains slot0 and its six default
states while removing the other fifteen slots. The source B5F100 grows the
initially empty actual pass array and appends the row `0/7B295400`. The builder
counters at8C/90/94 finish at1/0/1. The child operation retains the exact sampler
identity and address of the supplied root scratch word.

The existing checks still pass: nine canonical owners, real hot white-texture
acquisition with count1→2, returned pass and shader counts1, cache cursor2,
VS semantic5/register0/count4, PS material `Tint`, root/mode render-state
override, normal builder cleanup, and COM bytecode readback matching the cache
inputs. No cold resource/VFS route is manufactured. The white owner is acquired
by the existing pass constructor; source0 adds a reference route and does not
perform a source1 sampler texture load.

## Separate original sampler observation

The full 306 bytes of B3B280 and 81 bytes of B5F100 were reverified live against
the read-only installed PE. Private executable copies redirect only the reached
B3B328 relative CALL to the copied original B5F100. All consumed instructions
execute as original machine code, without a host callback or atomic bridge.
The original and relocated images are retained as evidence.

This narrow observation uses explicit pass/builder storage preimages and the
same produced sampler. Only the consumed twelve-byte pair header is initialized;
the existing B40C80 source reserve supplies capacity before the original call.
The original no-growth branch then preserves data/capacity, publishes count1,
stores `0/7B295400`, and advances builder counters13/17/19→14/17/20. Sentinel
bytes outside the consumed pass/header and builder-counter fields stay unchanged.
This footprint is not presented as a constructed complete original pass owner.

The original row agrees with the full source compiler's row, including all
upper scratch bytes. Original allocation growth is not covered: the source
compiler starts with zero capacity, while the original observation deliberately
has reserved capacity. Neither this observation nor the empty-mode call executes
the original whole compiler, its imported child schedule or FH3 exception path.

## Evidence and limits

The focused fixture and exact commands are under
`local/verification/material_compiler_sampler_runtime`. The final probe links
directly against fresh Win32 libraries; its map identifies the compiler archive
member. Original bytes, the relocated executable image, cache-input blobs,
compiler/read/link records, loaded modules and the immutable closure are pinned
in `reports/native_material_compiler_sampler_runtime.json`.

The inherited platform adapter has a borrowed zeroed receiver and takes the
online-null BECB20 source guard before platform/input/profile reads. It proves
no platform construction or publication replacement. All contexts and acquired
objects remain held to process exit, with no acknowledgement or fake retirement.
Source1/source3 samplers, cold loading, generated-source compilation, original
growth/FH3, terminal teardown, drawing and gameplay remain unvalidated.
