# Raw Object particle resource loading

`native_particle_object_resources_raw.hpp/.cpp` composes all three complete
AF8940, AF9660 and B80D70 bodies with the application's existing raw resource
container, VFS and cache services. The earlier host interfaces remain available.
The three bodies total 1,488 bytes and match the installed PE and live Ghidra.
Names are descriptive hypotheses; these C++ interfaces add explicit contexts
and retained invocation frames and are not binary replacements.

## Actual resource and publication domains

Object definition +8C/+90/+94 is a pointer/count/capacity array of actual
resource **containers**. It does not contain model-owner wrappers. AF8940 walks
the CURRENT count backwards, captures the current last cell and its resource,
and decrements the resource's actual +4 reference count exactly once. At zero
it captures the current profile and slot0 and invokes the concrete
`NativeResourceContainerReferences` chain. Only a normal terminal return clears
the captured cell. The loop then decrements and reloads the CURRENT count.
An interrupted terminal must never retry the already-consumed decrement.

`NativeParticleObjectResourcesRawContext` borrows the real manager context,
B80720 cache context, container reference chain, current F8D31C factory alias
and native empty-stem literal. The manager, cache, resolver, hierarchy reader,
item dispatch and raw strings must share their actual application domains.
Numeric profile data must be readable as required by the existing providers;
function words are dispatch identities. The animation item chain can use
[the completed animation dispatch adapter](NATIVE_SKINNED_ANIMATION_DISPATCH_ORCH4.md).
Unimplemented current providers retain their explicit source boundaries.

AF9660 first clears old resources, constructs the filename and resolves it
through CURRENT 0109CEEC, ignoring this first Boolean result while preserving
filename mutations. It splits the last dot, scans signed trailing digit bytes
while index > 0, and generates numbered prefix/number/extension candidates when
digits were found. Position zero is not scanned. Each candidate gets a fresh
actual VFS resolution. False stops the sequence; true loads and appends without
an extra retain. The unnumbered arm performs one candidate lookup.

On each load, F8D31C is tested before 4C1400. The explicit arm rereads that alias
after the getter. The other arm calls B80D70, which captures CURRENT manager+4
and calls B80720 with the same name and manager. Append grows only at count ==
capacity, uses signed max(2*capacity,1), skips a computed null destination store,
then increments CURRENT count. Already-cleared or appended resources are not
rolled back, and an acquired resource has no invented cleanup on append failure.

## Temporary storage and exception ownership

The immovable acquired frame contains native locals +10..+847, including the
reused eight-byte headers and both 1,024-byte formatting buffers. Optional VFS
and cache children follow that storage. A new child replaces only a successfully
completed invocation. A failed child and its parent must remain alive according
to the existing provider contract; diagnostics may contain natively freed data
pointers, which must not be reused. No replay or reset is supported.

Handler CBAFA3 uses FuncInfo DF2DF0 and the eleven-row map at DF2E14. The source
preserves each predecessor and header identity. State 7 exists in the map but
is not installed: after the second concatenate, state 8 owns the completed name
and suffix while the head is returned; state 9 owns the name while the captured
suffix is returned. Numbered candidate cleanup then returns to state 4 (prefix),
and unnumbered state 10 owns its separate +20 header. Final extension and stem
returns disarm before calling the pool. The delimiter has no caller unwind state.
Nine distinct cleanup actions tail-jump to 41DD20. A second C++ cleanup exception
terminates; this does not establish original Windows FH3 or fault transport.

## Validation and limits

The full Release MSVC Win32 build and all three existing CTests passed. One
ignored probe executes all three complete copied original bodies and compares
four filename cases through the genuine resolver with empty actual mount/search
structures. It covers extension splitting, numbered construction, backslash
normalization and complete pooled-byte accounting. A repeated/null-reference
clear comparison checks reverse traversal, count and exact decrements. A B80D70
comparison enters the genuine B80720 hot-cache path and checks resource identity,
factory/work publication and one retain. The report records bridge boundaries.

AF9660's successful candidate/cache-miss path and AF8940's zero-reference terminal
were reviewed and built but not executed by this probe. It does not establish
real file loading, original child bodies/CRT/FH3, application-chain installation,
rendering or gameplay. No permanent tests were added.
