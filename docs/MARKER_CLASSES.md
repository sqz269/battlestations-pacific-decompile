# Marker class loading and ownership

This packet reconstructs the global MarkerClasses loader006DBEB0, the20h
descriptor constructor006D8FC0 and lifetime, and the registry lookup006DBC10.
Names are descriptive hypotheses. Descriptor field offsets are verified for
Win32; native vtable addresses remain identity words, not executable host
vtables. Registry storage is an explicit standard-map projection.

## Binding the correct Lua owner

006DBED1 loads CURRENT[00E188A8], then006DBEDF adds1A0C before calling the
Lua owner GetGlobals helper00B67980. This is an embedded game Lua owner,
distinct from the mission Lua pointer at game+1A08. The context therefore
requires `MarkerClassHost::current_game_lua_1a0c()` returning that already-open
state. The loader invokes the getter once, borrows the interpreter and captures
the `MarkerClasses` value once. It does not create/close a Lua interpreter,
load a script, or substitute the mission Lua state. The temporary globals
pseudo-object is released before iteration begins.

For each table entry, the key supplies the descriptor name. The value is read
in this order: `Mesh` string, `Anim` integer, `Synchronized` boolean. Literal
bytes were verified at00CE5FD0 and00CF8FD0..FF5. Mesh conversion creates a pooled
temporary and captures its data and length BEFORE the Lua Mesh reference is
released at006DC022. Anim uses existing00B66290, including its float32 spill
and REQUIRED live CRT SSE2/x87 mode reference. Each field reference is released
before looking up the next field. Lua truthiness is retained: numeric zero is
true. Missing/non-string Mesh values have no invented empty default.

The loader allocates20h, constructs the descriptor, looks up the name in fixed
registry00E19974 and only then publishes the pointer at006DC0E3. It subsequently
releases the captured Mesh data/size, then the CURRENT name header, before
advancing iteration. Final cleanup is iteration value, key, MarkerClasses;
the borrowed Lua state remains open. String-key iteration uses the established
registry-reference GuiLua51Host adaptation. Native stack tracking/GC timing and
numeric-key coercion of the original lua_next cursor are not reproduced.

## Descriptor and resource ownership

Native20h layout: vtable0, NativeString name+4, NativeString mesh+C, intrusive
resource pointer+14, signed Anim+18, synchronized byte+1C. The remaining three
bytes are padding. `MarkerClassDescriptor` has compile-time size/offset checks.
Construction writes00CF8F78, independently copies both strings through the
existing00426060 implementation, then stores Anim and only the synchronized
byte. Resource+14 and padding are not given fabricated constructor defaults.

The resource request uses a NEW temporary string constructed from the descriptor
mesh data. Only a null mesh DATA POINTER selects the empty literal00E19980;
non-null data is consumed as a C string. The concrete007188A0 wrapper captures
factory007175D0 first, then resolves manager004C1400 and calls00B80720 with that
captured factory and the same mutable name object. Individual factory/manager
resolution and load/cache services remain required implementations. There is
no fake cache or success result. The returned pointer owns one intrusive
reference; it is stored at+14 before the temporary name is released.

006D8BC0 restores the vtable identity and captures resource+14. For a nonnull
resource it performs real InterlockedDecrement at resource+4; zero dispatches
the resource's CURRENT virtual+0 with no stack arguments through the required
resource service. The descriptor pointer is set null AFTER that callback,
including when the callback changed it. Mesh then name are destroyed through
the existing pooled-string body, leaving their headers untouched.

006D90E0 always destroys the descriptor and tests only flag bit0 before concrete
CRT free. It returns the original pointer even when freed. The existing correct
`CG_scalar_deleting_dtor_006d90e0` identity is retained. Host deleting calls
require placement construction in `singleton_lifetime_allocate` storage.
The constructor's string cleanup and raw-allocation failure cleanup use C++
exceptions; native SEH transport, compiler tracking and allocation-fault parity
are not claimed. Resource implementations must supply real ownership effects.

## Registry identity, duplicates and clear

006D8460 is lower_bound using the existing length-zero gates and CRT `_stricmp`
ordering.006DBC10 returns the existing pointer cell or inserts a new owning
key with pointer value NULL. The native1Ch node has links0/4/8, key+C, mapped
pointer+14, color18 and sentinel19. On insertion the temporary key and node key
are independent copies.006D94B0 proves the node key copy;006DA710 provides the
native link/rebalance work. Its partial STL_xlen_throw classification is a
library helper, not a reason to port the complete tree. Standard map operations
serve that library boundary here.

006DBC76 captures temporary key data before insertion;006DBCBC reloads its
length for cleanup. The pointer cell
is retained across that release. New `count_08` is incremented before temporary
cleanup; existing cells, keys and values retain identity.

006DC0E3 performs a RAW POINTER OVERWRITE. It does not delete, decrement or
retain the previous descriptor. Repeated case-insensitive names therefore
abandon the previous pointer from this registry. The reconstruction preserves
that behavior; it does not introduce an ownership collection or replacement
callback. A caller retaining an overwritten pointer can explicitly destroy it,
but the native registry clear cannot discover such displaced descriptors.

006DB2F0 walks current entries in ascending key order. Each nonnull descriptor
is deleted, then its CAPTURED mapped cell is set null after deletion returns.
Its pointer is not cleared beforehand. Once descriptor callbacks finish,
006DA6B0 destroys registry keys in reverse key order (right/current/left) and
the native count is finally reset to zero. The projection preserves that order
and maintains separate `count_08`; std::map size is not the native count during
teardown. The key helper is reconstructed only for this full-root use.

Typed registry entries must be descriptors constructed by006D8FC0; arbitrary
replacement vtables/subclasses are not represented. Callbacks must preserve
captured nodes. Native raw-node layout, debug iterator validation, STL maximum
size/exception ABI, allocation-time registry mutation and freed-link traversal
are outside the standard-map projection. Its host sentinel belongs to the C++
container lifetime. No validation failures are replaced by successful results.

## ABI, bounds and analysis

| Address | Native ABI | Last instruction / inclusive end |
|---|---|---|
|006DBEB0|no arguments; RET|006DC19D RET length1, end006DC19D|
|006D8FC0|ECX descriptor; name*,mesh*,Anim,sync stack; EAX=this; RET10|006D90DC RET10 length3, end006D90DE|
|006DBC10|ECX registry; name* stack; EAX=mapped pointer cell; RET4|006DBCFB RET4 length3, end006DBCFD|
|006D8BC0|ECX descriptor; RET|006D8C67 RET length1, end006D8C67|
|006D90E0|ECX descriptor; flags stack; EAX=this; RET4|006D90FB RET4 length3, end006D90FD|
|006D8460|ECX registry; name* stack; EAX=node; RET4|006D84B0 RET4 length3, end006D84B2|
|006DB2F0|no arguments; RET|006DB3A2 RET length1, end006DB3A2|
|006DA6B0|ECX registry; subtree pointer stack; RET4; full-root projection only|006DA6FF RET4 length3, end006DA701|
|007188A0|ECX mutable resource name; EAX=resource; RET|007188BB RET length1, end007188BB|

Complete loader207, constructor91 and lookup86 instructions were inspected,
plus all lifetime/wrapper listings and bounded direct library dependencies.
Ghidra project/program verification preceded live queries and refreshed exports.
All function starts exist live. This worker performed no Ghidra mutations.

Two flow repairs are pending with root: free006D90F0 has missing ADD ESP,4 at
006D90F5..F7 (3 bytes); free006DA6EC has missing006DA6F1..FB (11 bytes: stack
adjust, left sentinel test, current-node update and back edge). Disk decode
establishes those instructions and the complete RET bounds above. No other
call gaps were found in the nine owned functions. Root owns central annotation
and reconstruction ledger integration; correct library/compiler names remain.

## Validation

Standalone `./scripts/build.ps1` passed MSVC Win32 Release and configured
reconstructed_math test1/1; log `local/marker-classes-build.log`. One ignored
fixture uses TWO actual Lua states, the real Lua owner/runtime over an empty
fixture VFS and concrete resource allocations. It verifies current-state
borrowing, no file reads or state close, float32 Anim conversion, boolean
truthiness, captured factory identity, descriptor publication after loading,
case-insensitive key/cell identity, duplicate overwrite without old-reference
release, callback-sensitive resource and registry-cell nulling, and balanced
pooled ownership after explicitly disposing the retained displaced descriptor.

Fixture `local/marker-classes-probe.cpp` passed with `/W4 /WX` against library
SHA256 B261BC8C8673DAD9C5C36EF22DDC705462235ECDC911086E40211029DEB14F65.
Recipe: `./local/run-marker-classes-probe.ps1 -SourceRoot <checkout>`; optional
`-CoreLibrary` and `-LuaLibrary` select matching built artifacts. The recipe
uses live headers/libraries without copying reconstructed source. Log:
`local/marker-classes-probe.log`. Initial fixture setup lacked the Lua owner's
required DoFile binding; using the existing LuaScriptRuntime resolved it.
No source fix was needed. Full resource loading, startup integration, native
binary ABI and gameplay remain unvalidated.
