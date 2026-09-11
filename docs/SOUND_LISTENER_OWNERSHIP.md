# Sound listener records and ownership

Addresses: 00A7D380, 00A7BCE0, 00A7BD50, 00A7F9F0, 00A7F050,
00A7D750, 00A7D910, 00A7FE00.

The configuration now allocates actual listener records and retains table
references. The previous `vector<string>` append projection had the right name
and duplicate policy but could not implement sound shutdown's reference release.
This document supersedes that listener-ownership limitation in
`SOUND_CONFIGURATION.md`; its group/type and selected-ordinal limitations remain.
Descriptive names are hypotheses, not recovered symbols.

## Layout and original ABIs

Producer 00A7D380 stores base vtable CEB130, refcount 1, derived vtable D5ABB0,
and zeros the two NativeString fields at +8/+C. Allocation at 00A7FA4D requests
10h bytes. Live image D5ABB0 contains `00BD30E0,00A7BD50`, establishing the
zero-ref path through vslot0 to scalar destruction with flag 1.

| Address | Native ABI / inclusive end | Coverage |
| --- | --- | --- |
| 00A7D380 | ECX=10h destination; stack source*; EAX=this; RET4 at A7D40C..0E | complete normal behavior |
| 00A7BCE0 | ECX=record; no stack args; RET at A7BD46 | complete normal behavior |
| 00A7BD50 | ECX=record; stack flags; EAX=input address; RET4 at A7BD6B..6D | complete, raw gap included |
| 00A7F9F0 | ECX=manager+A4; stack source record*; RET4 at A7FABF..C1 | complete normal behavior |
| 00A7F050 | ECX=pointer-array header; stack record**; RET4 at A7F0C0..C2 | complete normal behavior |
| 00A7D750 | ECX=pointer-array header; stack signed capacity; RET4 at A7D841..43 | complete, raw gap included |
| 00A7D910 | ECX=pointer-array header; stack signed count; RET4 at A7D98B..8D | complete normal behavior |
| 00A7FE00 | ECX=manager+A4; no stack args; RET at A7FE5C | complete, raw tail included |

The table's native pointer/count/capacity occupy manager+AC/+B0/+B4. The
listener owner starts at manager+A4: its +0 vtable and +4 refcount remain in
`SoundSystemOwner.listener`. The new raw array owner keeps string-storage and
allocation-liveness bookkeeping outside the 10h listener record. This is a new
C++ interface; neither the whole table owner nor its vtable is a binary replacement.

## Ordering and native quirks

The copy constructor resets the destination before checking source/destination
identity. Self-copy loses the former buffer and leaves an empty name/refcount 1;
it does not release that former buffer. Nonself copy reuses the existing native
string copy helper, including its post-allocation reloads and preserve=true resize.

The destructor writes D5ABB0, captures the current nonnull string buffer and its
length+1, and releases it through the supplied `NativeStringStorage`. The name
header and refcount are untouched by destruction, including header modifications
made by that callback. Only afterwards does BD30F0 write CEB130. Scalar destruction
always runs that body, frees the record only when flags bit0 is set, and returns
the original address on both paths. Record allocations and frees use the host CRT;
the native operator-new retry and exception machinery are not reimplemented.

Reserve clamps to at least 1 and grows only. It copies and retains every live
slot before its forward old-slot release pass, then frees old storage and installs
the new pointer and capacity. Unused capacity is not initialized. Append zeroes
its destination slot before rereading the pointer-reference argument, retains
the resulting nonnull pointer, then increments count. A reference aliasing that
same destination slot consequently appends null. A reference into storage that
is invalidated by growth is outside the usable native contract.

Resize grows with null slots. Shrink decrements count before each release, walks
backwards, captures the removed slot before the callback, then zeroes that slot
after the callback. Raw slots remain allocated during the callback; vector
`pop_back` would end that slot's lifetime prematurely.

Owner destruction calls resize(0), frees the table, then writes base vtable
CEB130. It does not first install the derived vtable. The freed data pointer,
capacity, selected ordinal/pointer and other embedded fields stay untouched.
Host-only liveness bookkeeping prevents C++ automatic cleanup from freeing the
already destroyed table again. Destruction is single-use; the native dead owner
cannot be used for another append. Without explicit native destruction, the
table's C++ destructor provides fallback ref cleanup through its bound storage.

Configuration creates the temporary listener name, appends the copied record,
finds/selects using the still-live temporary buffer, then destroys the temporary
at A80C2F..A80C6A. Comparisons in A7F9F0 are deliberately ignored: duplicates
remain separate allocated records. The table owns one reference to each appended
copy after the constructor's temporary reference drops. Additional external
references must be retained and eventually released with the same string storage.

## Direct call evidence

Each row gives native CALL site, callee, and containing function. IAT and virtual
calls are separately described in the report. Callee bodies were inspected before
assigning their contracts; no unresolved dependency was replaced with a no-op.

| Site | Callee | Containing function | Contract |
| --- | --- | --- | --- |
| A7D3DC | 0041DD40 | A7D380 | Existing NativeString resize(length,true) |
| A7D3F1 | 00BF7680 | A7D380 | CRT memcpy; caller ADD ESP,0Ch |
| A7BD1C | 00419CC0 | A7BCE0 | Pool singleton, no release arguments consumed |
| A7BD23 | 00BD1510 | A7BCE0 | Pool return(block,length+1,1), RET0Ch |
| A7BD32 | 00BD30F0 | A7BCE0 | Base vtable store |
| A7BD53 | 00A7BCE0 | A7BD50 | Listener destructor |
| A7BD60 | 00BF65AC | A7BD50 | CRT free, caller ADD ESP,4 in raw gap |
| A7D791 | 00BF55BE | A7D750 | Pointer-array allocation, caller ADD ESP,4 |
| A7D81A | 00BF6989 | A7D750 | CRT free, caller ADD ESP,4 in raw gap |
| A7D91E | 00A7D750 | A7D910 | Reserve requested capacity |
| A7F080 | 00A7D750 | A7F050 | Reserve max(2*capacity,1) |
| A7FA3C | 00BF7FBF | A7F9F0 | CRT case-insensitive compare; ADD ESP,8 |
| A7FA4D | 00BF681B | A7F9F0 | Allocate 10h listener; ADD ESP,4 |
| A7FA68 | 00A7D380 | A7F9F0 | Copy construction |
| A7FA86 | 00A7F050 | A7F9F0 | Append retained pointer |
| A7FE2D | 00A7D910 | A7FE00 | Reverse release to count zero |
| A7FE35 | 00BF6989 | A7FE00 | Free table, leave dead pointer/capacity |

Raw disk CALL A7FE47 -> BD30F0 is in **no Ghidra function** as checked with
`ghidra proto` on the worker snapshot. It is the independently decoded A7FE00
tail, inclusive A7FE3A..A7FE5C, and is not attributed as a live function call until
the integrator repairs that boundary. Disk and live bytes agree. Two additional
holes: A7D81F..A7D831 (install pointer/capacity) and A7BD65..A7BD67 (ADD ESP,4).
The worker did not mutate Ghidra. Existing comments and names remain available
for the integrator's saved-project annotation pass.

## Verification boundaries

Validation results are recorded in `reports/sound_listener_ownership.json`.
Target is MSVC Win32 /W4 /WX. A focused local probe checks actual allocation,
duplicate identity, refcounts across relocation, reverse release callback timing,
dead fields, destructive self-copy, and cleanup without double release. It is not
an installed-game differential test. Existing tests and compilation are distinct
from game validation; no gameplay or sound playback claim is made.

The supplied string storage must outlive every record and the table owner and
must match the table's explicitly bound storage. Production uses CRT storage.
Reference arithmetic is atomic; structural mutation is serialized. Counts must
be nonnegative and allocation products fit signed 32-bit range. Foreign virtual
destructors, allocation failure/new-handler equivalence, native SEH frames,
reentrant structural mutation, malformed headers and use after explicit owner
destruction are outside the equivalence claim. The selected field remains the
existing ordinal projection; no retained reference was invented for that weak field.
