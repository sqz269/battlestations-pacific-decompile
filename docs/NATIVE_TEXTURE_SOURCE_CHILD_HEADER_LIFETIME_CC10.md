# Canonical texture-source child header and slots

The existing C30470 producer creates one live NativeTextureSourcePayload at
actual+08. This packet routes C304A0 source cleanup through that payload's nested
NativeRenderPointerArrayStorage at actual+10, with typed volatile pointer and
count accesses. New reference overloads of 735FF0 reserve and 737390 resize
start individual void* slot lifetimes only at their reached copy/zero stores.
The existing raw void* signatures/bodies and unrelated callers remain intact.
These are explicit Win32 C++ source interfaces, not original ABI replacements.

| Original routine | Complete original body | Original ABI |
| --- | --- | --- |
| 735FF0 reserve | 735FF0..73604E, 95B | ECX actual12B header, stacked signed capacity, RET4 |
| 737390 resize | 737390..7373DF, 80B | ECX actual12B header, stacked signed count, RET4 |
| C304A0 cleanup | C304A0..C3054B, 172B | ECX complete source receiver, RET |

Only the real C30470-created live payload/header is admitted. Launder at owner+8
accesses that existing object; it does not create a payload or adopt raw image
bytes. The same payload/header lifetime must survive all callbacks and cleanup.
Every reached source/child-slot read requires an aligned, backed, live void*
object. Child identities require the real same +04 atomic and canonical owner/
zero provider; null children are not made safe. No payload/header/atomic is
constructed twice. Raw base/profile/derived lifetime assumptions remain separate.
Selected/rate/flag/padding and the separately live owner atomic are unchanged.

Typed reserve retains the signed clamp/capacity gate, DWORD-wrapped request*4
allocation, fresh signed-count loop tests and current-data/source-slot loads.
Each nonnull destination default-placement-starts one scalar Pointer without
parentheses/braces, then receives one typed volatile store. Inactive capacity
remains unwritten. Current data is freshly read for free, then replacement data
and requested capacity are published in order; count is untouched. There is
no buffer snapshot, bulk construction, extra owner credit or failure rollback.

Typed resize retains the capacity gate/reserve, captured count after reserve,
fresh data per wrapped slot address and the null-address skip. Each reached
zero store default-placement-starts one Pointer; an existing same-type scalar
pointer can be transparently replaced without retaining old slot references.
Shrink retains fresh count reads and wrapped decrement stores followed by the
requested-count publication. It neither destroys old slots nor clears stale
data/capacity. Freed storage ends slot lifetimes. A skipped null-address store
does not establish a slot lifetime despite the unchanged count behavior.

C304A0 still stamps D79B54 first. Each iteration reads current count, data and
last pointer, uses the unchanged genuine atomic-decrement/zero provider, then
rereads and conditionally decrements current count. Reentrant child callbacks
may redirect the next data/count/child access while retaining the active
payload/header lifetime and reached slot validity. Normal cleanup disarms the
existing source guard, performs typed resize0, freshly loads current data for
free and calls the base stamp provider. Stale data/capacity and source cleanup
diagnostics remain. Catch-path array cleanup and base cleanup retain their
order and existing secondary-exception terminate boundary. Static FH3 map/
actions are pinned separately; private FH3/SEH and hardware-fault execution
are not established by source C++ exception reconstruction.

Existing companion admission remains explicit: actual BBC6F0/BBC810-created
storage, the same +04 atomic/canonical binding and current recovered profile/
zero/scalar providers. This packet adds no registration, retained count or
profile check. The historical procedural differential fixture cited by
NATIVE_PROCEDURAL_RESOURCE_LIFETIME.md at local/native-procedural-resource-final
has no currently established immutable path. Its pre-canonical constructor and
copied-native byte stores do not themselves establish this C++ payload/header/
slot admission. Any additional lifetime preparation is unverified; none of that
old runtime is credited to these new typed paths. No fixture was rerun.

The four-file diff exactly matches the frozen reviewed design. The worker
safely synced 4993f9954 to newer disjoint fe858cae3 after preserving selected
source and complete old core/member artifacts. The old full library is the
historical a40922b3 payload build (its reviewed patch later committed63a4e31),
not a coherent image of all newer disjoint providers. Both selected old source
files match current prechange source and their full library members exactly.
The new complete library/members come from this packet's strict Win32 build.
That single build and all three existing CTests passed; no test was added.

| Selected object | Old/new code sections | Old/new code bytes |
| --- | --- | --- |
| cube_texture_owner_array_reserve | 4 / 6 | 163 / 307 |
| procedural_resource_lifetime | 70 / 74 | 3488 / 3598 |

Across both objects, 72 existing code sections (3136B) retain exact bytes and
relocations, including the raw reserve136B, raw resize83B, existing companion,
scalar/terminal and source exception-handler code. Six added code sections
total254B, including typed reserve136B, typed resize83B and small compiler
placement/launder/address helpers. No linked-body or native-byte credit is added.
All170old/176new nonempty raw COFF sections, symbols and relocations are archived.

Typed reserve's full instructions/control flow/relocations match raw reserve
after its declared ECX/EDX index/destination register exchange at69..102. Typed
resize83B has identical bytes; its reserve-call relocation at19 names the typed
overload. At source level, destroy_array calls typed resize, which calls typed
reserve. MSVC inlines resize in destroy_array157B and the normal cleanup within
destroy_body358B: their emitted relocation substitutions at46 and150 name typed
reserve. Array cleanup retains exact bytes. The broader destroy_body array is
not byte-identical: owner/op registers exchange, header LEA moves70->66 before
the first count test66->69, and selected count accesses use equivalent owner+14
or header+4 addresses. All18 actual field events and normalized control flow
correspond; callback-relative current loads/stores and free publication remain.
Its exception tail279..357 retains exact bytes/relocations. No extra placement
call, native store or FP operation appears in reached typed bodies.

The initial ignored seal helper refused Windows HostX86/Hostx86 path spelling
despite exact size/hash agreement. That refusal and diagnosis are preserved;
the helper verified the requested path's content and resumed sealing without
reapplying source. Actual build launch/completion return objects are archived.
Named24source/27tool pins are bounded coverage, not all compilation dependencies.

Full byte/relocation evidence is in compiled_sections.json and
complete_coff_sections.json; instruction/field/control correspondence is in
compiled_schedule.json under local/cc10_texture_source_child_header_implementation.
Archive local/cc10_texture_source_child_header_evidence.zip and its manifest
freeze the packet plus the separate non-reconstructed C30570 read-only audit.
Three fragments add zero native bytes/functions. No Ghidra writes were made.

Other raw atlas/renderer headers and the uncovered C303F8 reset caller remain
outside typed admission. C30570 must separately start its reached append-slot
lifetime and resolve its Lua/error/resource obligations. Unchecked FPS, null
children and returned-but-unappended credits remain unresolved. No producer,
Lua, record/string, platform/MSG/XLive, source0/W, full sampler/application/game
activation, new fixture runtime, full ISO-safe graph or original ABI proof is
claimed.
