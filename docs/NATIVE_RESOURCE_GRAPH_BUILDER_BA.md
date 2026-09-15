# Native resource-instance graph builder

Addresses: 00b891a0, 007137f0, 00b88b60, 004fcbe0, 00b74310, 00b8e600, 00b8e6f0, 00b8f0f0

The complete 1280-byte B891A0 caller and 21-byte7137F0 wrapper now have C++
implementations. The builder creates the resource instance, constructs all nodes,
publishes the first node as the root, invokes the associated item services, then
runs the complete B79BC0 and B87E80 postprocessors recovered in AZ and AS.
This supersedes the analyzed-only status recorded in AQ/AR. It closes the caller
dependency; it does not register arbitrary game factories or admit a populated
resource graph into the executable. Names remain descriptive hypotheses.

## Native contract and sequencing

B891A0 receives resource in ECX, two words on the stack, and returns instance
in EAX with RET8. The source additionally borrows its context in EDX. The first
word is forwarded unchanged to every item slot18; its semantic meaning remains
unresolved. The second is unused by B891A0. The7137F0 wrapper nevertheless
executes FLD/FSTP on that second word before forwarding it, preserving the x87
effect even though the builder never reads the result.

Resource+1C/+20 is the actual record-pointer array; records contain parent index0,
name4, matrixC, item-index array4C/count50 and flags58. The existing hierarchy
producers and AP/AQ evidence establish this layout. Instance+10 is a checked
pointer-vector header with begin14/end18/capacity1C. No replacement graph or
container is introduced.

The default factory is the local D63218 model-base factory. Each current
associated item's slot1C can return a replacement; the last non-null result
wins. After name construction, current record flag58 bit0 overrides selection
with the local D63220 group factory. D63210 is a constructed local factory,
but is not the default. Factory objects are borrowed for this call only.

The unnamed path builds `<node N>` through the existing integer-string and
concatenation bodies, preserving its five temporary strings, copies and reverse
release order. Temporary construction becomes cleanup-owned only after the
corresponding call succeeds. The final name survives through node construction,
transform, parenting and bounds setup.

The vector destination cell is captured before the factory call. The returned
node is written to that cell even if a callback replaces current backing. The
builder then validates and reloads the current vector before setting slot38.
The first constructed/current node becomes the root. Later negative parent
indices select that root; nonnegative indices use another checked current cell.
The full canonical B6E680 parenting implementation uses the same existing
NativeNodeBinding companions and hierarchy storage.

Type dispatch captures the table before reading each current model/group token.
Model bounds preserve all six forward x87 copies. Groups choose automatic
aggregation for flag58 bit1, or explicit box followed by explicit sphere.
B8E6F0 is a literal56-byte x87 implementation: its pseudocode incorrectly
suggested integer copies. B8F0F0 sets actual byte175 then calls the complete
existing B8EBE0 implementation. That implementation retains its established
group companion/attachment-array projection; no raw binary ABI is asserted.

The second pass rereads each record's item list and the resource's item backing,
captures the corresponding current node once per record, and dispatches slot18
with instance, record, node and the original first creation word. Both full
postprocessors run after all item callbacks. No missing virtual operation has
a successful fallback; complete provider bindings are required.

## Vector and exception evidence

B88B60's complete179-byte resize grows through005058E0, whose recursively
verified canonical implementation is BD0700 (AI). It preserves unsigned counts,
captured begin/end values and returning invalid-parameter checks. Shrink uses
the complete90-byte4FCBE0 erase-range body. Erase validates iterator-owner
equality, moves the current tail with the actual CRT memmove_s, publishes its
captured resulting end, and returns the first iterator. It does not free memory.

The14-entry unwind map at DFBC48 has predecessors
`-1,0,1,2,3,4,5,6,7,6,5,4,3,2`. CC2690..CC2725 clean the three factory locals
and armed temporary strings. Factory cleanup B86B50/B86B60/B86B70 only writes
the common D63208 profile. The map does not release the instance or completed
nodes on a later failure. Source cleanup retains that partial graph and leaves
instance root0C untouched until the first pass completes. Native FH3 metadata,
private-frame aliases and fault-site equivalence remain unproved.

## Validation and limits

Strict MSVC Win32 and both existing CTests pass. Three paired executions of the
original caller bytes compare allocation/free events, native storage, names,
factory/item dispatch, parent links, root publication, pool state and FP status:
populated three-node construction, the game wrapper with a signaling-NaN second
word, and an empty graph. The populated cases use the complete existing native
ModelBase factory, actual pooled names and canonical parenting. Their fixture
item performs complete base-instance publication through B89E90. Published item
types do not select animation/mesh ranges; the full postprocessors execute their
empty-range paths here, while their populated paths have separate AZ/AS proof.

One paired vector sequence covers growth, equal-size preservation, shrink and
interior-tail erase. One source factory failure checks temporary-name cleanup,
retained partial construction and unpublished root. The56-byte compiled group
box helper matches the original bytes. All comparison images repeat exactly.
All45 address/native transfer rows pass, including B8F0F7 explicitly marked
as a tail JMP. The initial verifier rejection of that JMP as a CALL is retained.

Group-factory and automatic-group execution are not exercised by these caller
fixtures. Their complete external factory binding is still required, and the
existing group projection limits apply. The explicit providers and fixtures
are not an executable admission layer or proof of arbitrary item contracts.
Current class+50 population, graph admission, original exception integration
and gameplay validation remain open. No workers were dispatched.

## Retained evidence

Implementation commit `06891b527bdd0315725786efde13d7de4c6fcf0b` is integrated in `f9c2873031b81d2314f4918aee97500c27051f2c`, which was built as a clean checkout. The immutable local manifest retains 822 exact inputs and 124 artifacts, including 112 linked production objects, source/header/compiler/library inputs, eight original spans, paired comparison images, unwind records and saved Ghidra annotations/exports. The provider and game-validation boundaries above remain unchanged.
