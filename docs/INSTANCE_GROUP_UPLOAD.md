# Instance group upload and borrowed frame entries

`upload_instance_groups_00b1e990_fragment` implements the established two-category
upload/queue path using actual `LogicalVertexStream` locks and retained geometry
objects. `InstanceRenderQueue` stores the actual caller-owned output entry pointers.
The generator and model hooks are required adapters to real borrowed scene inputs.
They have no default success callback. Geometry allocation remains in
`instance_geometry.cpp`; the group producer `00b1dff0` is a separate contract.

This is a checked success-domain C++ interface, not a binary-compatible native
context/model/container overlay. The four primary native functions are `00b1e990`,
`00b85590`, `00b51a20`, and `00b51cb0`. See the paired JSON audit for full byte-range
hashes, installed-PE comparisons, annotation proposals and validation status.

## Observed storage and call order

The native context contains group pointers/count at `+38/+3C`. Each group holds
the generator through `[[group+0]+0C]`, counts at `+0C/+10`, retained output entry
pointers at `+14/+18`, generated model pointers at `+1C/+20`, and source pointer
arrays/counts at `+24/+28` and `+30/+34`. The loop visits groups in list order and
category zero before category one. A zero category count skips every operation,
even when its source list or other fields are nonempty.

For each active category, the instruction sequence is:

1. `00b1e9cb..00b1e9e5`: attach the generated model to the scene binding through
   model virtual `+50`, with arguments `(scene_binding, false)`.
2. `00b1e9e7..00b1ea0d`: category one sorts its source pointer list using
   `00b1dce0` and comparator `00b51ab0`; category zero retains source order.
3. `00b1ea12..00b1ea36`: select model geometry zero, stream one, and invoke its
   virtual `+10` lock with `(instance_count, 0, 0)`.
4. `00b1ea53..00b1ea89`: call generator virtual `+8` for each source entry with
   `(entry, destination)` and advance by its declaration's `+CC` stride. The
   declaration getter is read after every generator call.
5. `00b1ea8b..00b1eaa6`: get geometry zero/stream one again and invoke unlock
   virtual `+14`. The typed implementation holds the same stable stream identity.
6. `00b1eaa8..00b1eac7`: get geometry zero/section zero and write count to section
   `+1C` through `00b85590`.
7. `00b1eacc..00b1eb17`: initialize the category's retained output entry through
   `00b51a20`, passing `(0, section, geometry, model, camera, visibility, 0, 555h)`.
   The camera is `[[context+28]+8]`. Original PE/live constants at `00d7a24c` and
   `00ce3800` are float `1.0` and `0.5`, respectively.
8. `00b1eb1c..00b1eb45`: category one appends to queue `[context+10]`. Category zero
   reads the cloned material effect's `+AC` selector via `00b17300` and appends to
   `[context+0C + selector*4]`. No selector-zero or distinct-queue assumption is made.

The native upload routine uses ECX=context and plain `RET`; it has no stack args.
`00b85590` uses ECX=section, one stack DWORD, and `RET4`. Generator virtual `+8`
uses ECX=generator, stack source entry/output, and `RET8` for the building writer.
The typed generator receives a checked byte extent in addition to these values.

## Getter arguments and sphere depth

The decompiler incorrectly makes the `PUSH0` at `00b1e9d4` an argument of
`00b72110`. Assembly shows that getter returns `[ECX+1C]` with plain `RET`, so zero
remains on the stack as the second argument of virtual `+50`. The generated model
table `00d62de8` resolves `+50` to `00b6ed80`, whose `RET8` and second-argument
recursion gate establish `attach(scene_binding, false)`. Attachment can alter the
scene's real retained model relationships; this layer does not replace it with
an invented owner or skip it.

`00b51a20` uses ECX=entry and eight stack words, `RET20h`. It writes fields
`+00/+04/+08/+0C/+10/+18/+1C`; it does not initialize sort-key words `+20/+24`.
If depth override is greater than float zero (`00d7a218`), it stores that value at
`+14` and returns. Otherwise, including unordered comparison, it gets camera view
`00b6fcb0`, calls model virtual `+48`, transforms the returned sphere center by
`004142e0`, and stores float32 `(section+34) + transformed_z`.

Virtual `+48` resolves to `00b6e8c0`. This is a zero-stack-argument cached WORLD
SPHERE getter; it calls `007c1180` on model `+8` with model world `+F0` when cache
bits `+138 & 30h` are absent. It is not a raw translation getter. The prior pushes
of view and temporary output survive its plain `RET` as `004142e0`'s arguments.
The typed model callback supplies the actual world sphere center; native sphere
cache ownership and radius work are not reimplemented here.

The full XYZ x87 product/add/store sequence of `004142e0` is retained in the local
point kernel, although entry construction consumes only Z. The final bias add
also uses x87 and float32 spill. No native exception-state or NaN-payload claim is
made for the surrounding C++ and callback interface.

## Queue ownership and ordering boundary

`00b51cb0` uses ECX=queue, one stack entry pointer, `RET4`. Queue storage is
`+0C/+10/+14` = pointer/count/capacity. A full queue calls `00b51b50` with
`max(256, capacity*2)`, then writes the entry pointer and increments count. It
does not retain, clone, dispatch or destroy the entry. Host queue storage owns
only the vector of pointers; all entries, models, cameras and geometry must
survive frame consumption. Explicit host `clear()` retains queue capacity.

Ghidra originally omitted the tail after `CALL00b51b9f -> 00bf6989` because that
free routine was marked no-return. Original/live bytes `00b51ba4..00b51bac`
decode as `ADD ESP,4; MOV [ESI],EBX; MOV [ESI+8],EDI; POP EBX`. These commit the
new array and capacity before the shared `POP EDI; POP ESI; RET4` at `00b51bad`.
The complete allocator range is `00b51b50..00b51bb1`, 98 bytes. Reconstruction
uses this assembly tail, not the incomplete pseudocode. Ghidra changes are
coordinated by the primary agent; this worker only records proposals/evidence.

Category-one inputs use signed material order ascending and depth descending.
For at most32 entries, `00b1dce0` selects `00b1d420`, whose strict comparisons
and single-element rotation `00b1c210` preserve equivalent-item input order.
The upload now calls the complete recovered sort, including median/ninther pivot
selection, equal-band partition movement and heap fallback. Larger lists retain
the native tie permutations; they are not generally stable. The isolated native
comparison passed90 runs and5652 pointer positions, including duplicate pointers
and forced heap paths. See `INSTANCE_CATEGORY_SORT.md`. Nonfinite depths remain
outside the typed sort domain.

## Valid domain and verification

Counts must fit valid nonnegative native containers, source list size must match
its active category count, and total stream bytes must fit a DWORD and the
remaining physical buffer capacity. Output entry
must be distinct from source entries. Model/generator/geometry identities,
declarations and list storage remain stable during callbacks. Callback failures
and HRESULTs are host diagnostics; the native routine does not handle them.

Failure is intentionally not transactional: previously queued categories remain,
already performed scene changes and mapped writes remain, and a successful lock
is unlocked during unwinding. Capacity rejection precedes attachment and locking.
The physical lock helper increments depth even on an attempted COM failure;
upload now balances that pair only when depth actually changed, leaving preflight
failures alone and preserving the native cursor/dynamic-lock counters. Section count changes after upload; entry fields
can be partly updated if the world-sphere callback fails. Queue append stores
the original pointer. Native malformed containers, byte-size overflow and
allocation-failure corruption paths are outside this interface.

Full bytes for upload(494), setter(10), entry constructor(139), append(60), reserve
helper(98), and point transform(132) compare equal to the installed PE. The source
compiles with MSVC Win32 `/std:c++17 /W4 /WX /fp:strict`; primary integration owns
the full build and existing checks. No permanent test target was added. Final
installed-model upload, both queued stream-offset/frequency checks, the focused
capacity rejection and draw validation are recorded in `INSTANCE_INTEGRATION.md`
and `reports/instance_integration_validation.json`. Native object ABI and full
scene/gameplay behavior remain separate.
