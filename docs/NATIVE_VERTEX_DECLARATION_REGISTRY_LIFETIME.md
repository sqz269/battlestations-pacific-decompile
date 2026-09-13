# Actual declaration registry lifetime

Addresses: `00B30270`, `00B31630`, `00B316A0`, `00B31D40`, `00B32030`,
`00B32210`, and placement fragment `00B32534..00B32555` within `00B32410`.

The module completes lifetime operations on the existing actual declaration
registry used by `native_vertex_declaration_cache.cpp`. It borrows that cache's
actual string and allocator domains and the application's existing canonical
`NativeRenderActualOwners`. No second registry, semantic record overlay, owner
map, or reference word is introduced. Names are descriptive hypotheses; these
C++ interfaces do not replace original x86 ABI or the native exception runtime.

| Routine | Native ABI, inclusive end and last instruction | Coverage |
|---|---|---|
| B30270 | ECX vector; DWORD requested count on stack; B3033B, RET4 at B30339 (3 bytes) | Complete normal and source exception schedule |
| B31630 | ECX registry; B31698, RET (1 byte) | Complete, current D5F024/D5F060 release profiles |
| B316A0 | ECX vector; B316B6, RET (1 byte) | Complete, including returning-free tail |
| B31D40 | Incoming ECX unused; declaration on stack; B31D5E, RET4 at B31D5C (3 bytes) | Complete, canonical actual-owner terminal boundary |
| B32030 | ECX registry; B3208E, RET (1 byte) | Complete normal and source exception schedule |
| B32210 | ECX registry; flags on stack; EAX original pointer even after free; B3222D, RET4 at B3222B (3 bytes) | Complete |
| B32534..B32555 | In B32410; ESI renderer, EBX zero; final MOV at B3254C (10 bytes); no return | Complete five-store fragment; partial renderer constructor, all other B32410 ranges excluded |

## Producer and current storage

The existing `NativeRenderResourceRecord` producer establishes actual size2Ch,
name length/data `+00/+04`, preserved `+08`, alias sentinel/count `+0C/+10`,
five payload words `+14..+24`, and unretained declaration pointer `+28`.
B30270's default construction zeroes the name, allocates a real alias sentinel,
zeroes alias count, then zeroes payload words in descending offset order. It
deliberately leaves `+08` and `+28` untouched. Default records therefore do not
implicitly contain null declarations.

B32534, B3253A, B32540 and B32546 clear renderer `+1A64/+1A68/+1A6C/+1A70`;
B3254C publishes D5F060 at `+1A60`. The constructor fragment receives the actual
registry address directly. Whole-listing register filtering found ESI assigned
from incoming ECX only at B3242D, and EBX zeroed only at B32439; intervening calls
preserve both registers under the original x86 convention. This supplies the
actual vector header and stride accumulator without claiming full renderer
construction or replacing its other fields.

B30270 uses signed comparisons for reserve, growth and shrink; its index and
2Ch byte products wrap as DWORDs. Growth publishes the requested count only
after all constructors complete. Shrink decrements the current count first,
reloads it and the data pointer, and destroys that record. B316A0 resizes to0
then frees the current data, leaving data/capacity unchanged. No resource
AddRef or Release occurs merely because a record is constructed or destroyed.

## Release, flushing and destruction

B31D40 performs one atomic decrement at the actual declaration's `+04`. Only
zero loads its current native virtual0. The existing shared release helper
resolves and validates the companion only at zero. `NativeVertexDeclarationReference`
then checks its current actual profile and slots, performs actual scalar
destruction and returns the slot to the same pool, before retiring its binding.
Neither lookup nor a private reference operation is performed for nonzero.

B31630 reads the current last record's resource, unconditionally reads its
current `+CC` stride, and subtracts that value from registry `+10`. It then
reloads count, data, resource, registry profile and virtual `+10`, in native
order. Both D5F024 and D5F060 have B31D40 at that slot. After release, it reloads
count; nonzero destroys the *current* last record and then decrements the
*current* count. The final B30270(0) occurs even when the initial count was0.
The source retains DWORD underflow and adds no null-resource guard. In
particular, an earlier loader's null cached record is not made safe to flush
by this lifetime reconstruction.

B32030 publishes D5F024 before flushing. Its state0 cleanup is
`DF6600 -> CBDC80 -> B316A0(registry+4)`. After successful flush it disarms that
cleanup, calls B30270(0), and frees the current vector buffer. B32210 calls this
destructor and frees the registry itself through the same lifetime allocation
domain only when flags bit0 is set. Other flag bits do not affect the decision.
The returning-free continuation explicitly returns the saved original pointer.

## Native calls and exception ownership

The JSON report records every direct call/tail site and the three indirect
sites, including containing-function attribution. Native direct dependencies
are existing B2FE20 reserve, B2F910 record destruction, 4C3020 sentinel allocation,
41DD20 string cleanup, no-op placement delete401130, and the host CRT free/FH3
boundaries. Each callee body was inspected before assigning these meanings.
InterlockedDecrement is the real Windows atomic boundary at IAT CE2220.
The shared cache's current tables and original identity tokens are read at the
virtual call, not cached across release. Unsupported profiles/slots throw a
source-domain error after the preceding stride write; this error is not native
recovered behavior.

B30270's `DF6370` FuncInfo references the two-state map at DF6360: state1 first
destroys the current name through CBDA79, then state0 invokes CBDA60's no-op
placement delete with its two stack arguments (`ADD ESP,8` at CBDA75). Earlier
completed default rows and their sentinels remain allocated outside the old
count if a later constructor fails. No blanket rollback is added. Reserve has
its already documented no-rollback behavior. B32030 only arms vector cleanup
during flush; a later failure cannot repeat vector destruction.

The worker did not mutate Ghidra. Its initial B32030 body ended at B3207B,
omitting B3207C..B3208E. B316A0 ended at B316B1, omitting B316B2..B316B6.
B32225..B32227 was a three-byte ADD ESP,4 gap inside the scalar destructor.
All three continuations were verified with disk disassembly and matching live
bytes. Parent integration owns local flow overrides, expanded bodies and
refreshed exports; the CRT functions' global no-return properties are not
changed by this packet.

## Verification and remaining boundaries

Sixteen complete native/code/data spans match the installed PE and live Ghidra
image with SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The six routines contain488 bytes and the constructor fragment34 bytes. The
private reproducible probe and complete inputs live under
`local/declaration_lifetime/`; `evidence.py` records exact spans, lengths, hashes,
12 preimage-checked absolute relocations and native call sites. The fresh Win32
repository build and its one enabled existing CTest pass. The new source and
probe compile with MSVC Win32 `/MD /W4 /WX /O2 /fp:strict`, linked against the
primary checkout's current libraries. New-source CMake registration and the
final library-only replay belong to primary integration. The call verifier
passes15 direct/tail rows; three indirect sites remain explicit evidence rows.

The probe passes13 original/source comparisons: initialization; grow/shrink;
three allocation failures; reverse flush with one externally retained owner;
destructor and scalar flags0/3; exception during flush with native vector
cleanup; terminal callback changing count; and independent reference1/2 release.
It compares allocation/free/retirement events, count/stride/reference/return
state and all arena bytes except the actual string-pool lock's24 Windows-owned
bytes. The separate recursion counter remains compared. This supplies a real
already-constructed actual string-pool publication; manager creation is outside
the lifetime fixture.

The fixture copies the installed image into private executable storage and
runs the six original entry bodies, original cleanup actions and original FH3
metadata. The standalone constructor fragment gets a return at its end. Its
allocator is a deterministic observed arena on both sides; reserve, record
cleanup, sentinel allocation, string cleanup and the actual declaration
destruction/return remain explicit existing-source callee boundaries. Native
profile addresses are relocated for original virtual dispatch. The declaration
virtual0 bridge restores the original identity token before entering the same
canonical companion; it does not decrement again. Registered host handlers
enter the original metadata through the current CRT's CxxFrameHandler3.

This proves only the documented fixture paths and source cleanup schedule.
It does not prove arbitrary native virtual profiles, compatibility with every
native SEH/FH3 exception, CRT allocation ABI parity, or gameplay behavior.
The registry is not wired into a complete actual renderer owner in this packet.
Full renderer/device recreation and application registration of decoded
declaration companions remain their respective owners' integration work.

## AU saved-analysis limitation

Names, original signatures and evidence were saved and read back. The returning
free-call tails for B316A0 and B32030 are decoded and their complete
native byte spans were checked, but Ghidra still stores shorter function bodies.
The script API refused the narrow stored-body extension because script execution
is disabled. Complete source and original-byte fixture coverage does not imply
that these two stored Ghidra bodies are repaired. The constructor placement
fragment B32534..B32555 is an EOL comment within B32410; its parent name remains.
