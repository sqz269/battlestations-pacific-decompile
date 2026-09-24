# Actual scene constructor argument word (CC10)

`B724E0 [B724E0,B72576)` is the existing complete 150-byte actual24h scene
constructor. This packet adds a borrowed `const volatile uint32_t&` incoming
name-word overload to `native_gui_scene_storage.*`. The existing
`NativeGuiSceneConstructFrame&` overload forwards its member by reference
without reading it. Both execute the same body; no frame aggregate is started
over caller-owned scratch. Existing scene/context/weak/string lifetime and
failure contracts remain in force.

Native ECX is the actual24h destination, the stacked DWORD points to an actual8h
name header, EAX returns the destination, and B72573 is `RET4` (three bytes).
Weak construction at B72500 precedes the destination profile, root and name
initialization. The incoming DWORD is captured at B7251C only after both
destination-name DWORDs are zeroed. The captured header survives B72536 resize;
its current length and data and the current destination length/data are read
before B7254A overlap copy. Final B72552 MOVSS reads the independently current
one-cell binding before storing destination+18. The overload does not copy a
header or convert it to `NativeString`.

Caller backing must be live, initialized and address-stable. Context and
acquired metadata remain disjoint; aliases through unexposed private stacks,
saved registers, return addresses or the destination's lifetime preparation
are excluded. The existing byte-preserving actual24h object preparation is
unchanged. Adding this argument interface does not admit the full distortion
initializer or its unwritten +34/+38/+3C preimages.

The report pins 179 code bytes and 52 descriptor/map bytes against live Ghidra
and the installed PE. All six normal/cleanup call rows are explicit. Descriptor
DFAB08 uses map DFAAF8: state0 -> -1/CC1B60 destroys the weak base; state1 ->
0/CC1B68 destroys the name header; CC1B73 is the existing complete ten-byte
FH3 handler. The existing consume-before-call C++ cleanup projection remains;
native FH3/SEH and exceptional paths were not exercised here.

Strict Win32 build and all three existing CTests passed. Compiled evidence
shows the frame overload tail-jumping without an argument read, and the raw
body's argument load after the weak call and name-zero stores. The current
source/destination reads and final scalar-store helper are pinned separately
in `local/cc10_gui_scene_argument_view/compiled_checks.json`.

One ignored standalone original/source pair uses genuine isolated weak
pool/allocator/singleton lifetime, actual raw string pool and actual24h scene
storage. The source receives a live DWORD adjacent to a two-DWORD `MARS`
header and three canaries; no `NativeString` or overlapping frame is formed.
The current one-cell binding aliases destination name length, so final MOVSS
stores raw bits4. Both lanes preserve the backing/canaries, produce matching
whole24h bytes after normalizing only weak and name-data pointer identities,
and complete genuine scalar retirement, weak-handle invalidation, pool return
and support drain. The copied original receives the corresponding header
pointer on its own actual stack; the probe does not establish external
incoming-cell address equivalence or callback mutation of that cell.

The copied150B input equals the pinned live/PE bytes. Three rel32 calls use
genuine weak construction/resize and `memmove` adapters; one D7A24C operand is
relocated to the current owner length; the unreachable handler immediate is a
fail-fast trap. There are no indirect instructions in this normal body. The
probe links the three current project libraries with `/MD` and an embedded
manifest; its inputs, source, object, binary and logs are hashed. No application
or original game ran, no tracked test was added, and no allocation failure,
native exception transport, binary replacement ABI or gameplay claim is made.

The preceding initializer readiness files remain byte-for-byte frozen and are
included in the packet archive. The separate B4E470/535320 incoming-cell
proposal remains read-only under
`local/cc10_post_effect_argument_readiness/README.md`; it is not implemented by
this packet. Prior names, comments and reconstruction records are preserved.
