# Mission record picture ownership

Addresses: 005C6A70, 00AA2660, 005C89B0, 005C81F0, 005C7B80, 005C8AE0;
supporting container routes 005C8170, 005C8880, 005C8F90, 005C9030,
005C9250, 005C9290, 005C92F0, 005C9530, 005C9C10, 005C9E30.

`MissionRecordData::picture` owns the actual native texture reference and UV
rectangle of that same record. `extra.picture` remains the authored Lua name.
There is no extra mission record, GUI tree, pointer-to-record lookup, COM
texture cast, shared ownership control block, or fabricated texture identity.

The required services borrow the existing `GuiTextureCallbacks`, canonical
`NativeRenderActualOwners`, the concrete GUI-manager getter boundary, and an
explicit frame float2 preimage. The complete reader path cannot run without
these bindings. The current game host uses the explicitly named metadata
loader until the integrator supplies its real native texture binding.

| Routine | Coverage in this packet | Original ABI |
| --- | --- | --- |
| 005C6A70 | Picture branch 005C75A2..005C763B added to the existing Lua reader; other Lua/string/map/SEH operations remain their previous semantic reconstruction | ECX record, stack reader, RET4; EDI captures record, ESI reader |
| 005C89B0 | Picture initialization stores 005C8A49..005C8A96; other record construction remains outside this owner | ECX record, EAX record, RET |
| 005C81F0 | Picture copy 005C85A9..005C860D; remaining strings, vectors, sides and settings tree are outside this owner | ECX destination, stack source, EAX destination, RET4 |
| 005C7B80 | Picture assignment 005C7E2F..005C7E9C; other member assignments remain outside this owner | ECX destination, stack source, EAX destination, RET4 |
| 005C8AE0 | Picture release 005C8B58..005C8B82, recovered from raw continuation; other record cleanup and SEH are not ported here | ECX record, RET at005C8DAC |
| 00AA2660 | Reuse existing complete semantic resolver in `gui_texture.cpp`; no new renderer/atlas implementation | Stack name-header, inoutUV, inoutSize, scale; RET10h, EAX acquired texture; incoming ECX is unused |
| Container helpers above | Dependency evidence for retain-copy and destruction; no port of native STL storage/growth/validation/SEH | Native record stride434h; host `std::vector<MissionRecordData>` remains a new interface |

## Producer ordering

005C6D8E reads `picture` into a frame native string. The reconstruction keeps
that name in a local owning string through the intervening map-size branches;
the extra metadata string is not the resolver's source lifetime.

At005C75A2 the native length is tested. A nonempty name calls004C12B0 at
005C75A9. Its complete body was read: it double-checks the GUI singleton under
the singleton-manager guard, allocates88h, constructs00AA5D70, and registers the
result. These side effects remain an explicit required getter boundary even
though AA2660 ignores incoming ECX. This packet does not replace that singleton.

The actual resolver CALL is **005C75C7**, not005C75CC. FLD1/FSTP supplies scale1;
the four pushes supply name header at frame+30h, record+A4, frame float2 at+48h,
and scale. The frame float2 has no initialization in the reader. The caller
must supply its preimage; zero size is not assumed. The temporary is discarded
after resolution, as in native code.

AA2660 gets a borrowed atlas item through00AEFB20. A hit writes the SAME record
UV storage, preserves negative U/V orientation, optionally queries current
texture+3C/+40 when both size values equal zero, then retains texture+04. A miss
calls current renderer+64(name,0), preserves UV/size and returns its acquired
reference without another retain. The existing resolver's callback, x87 and
new-interface limits remain; this change makes no additional ABI-fidelity claim.

After AA2660 returns,005C75CC captures the old record+A0. It decrements old+04
at005C75DE, calls old CURRENT virtual0 only when zero at005C75EF, clears A0,
and publishes the acquired result at005C75FF. Thus zero callbacks observe the
old texture and the newly written atlas UV. Same-identity rereads still acquire
and release one reference. An empty name takes005C7607: release old, clear A0,
preserve UV, and call neither the manager nor the resolver. Native null returns
are published as null; missing host bindings are errors before reading begins.

## Copy, assignment, destruction, and vector use

005C89B0 writes A0=0 and UV={0,0,1,1}. The one at00D7A24C was checked as
3F800000 in read-only `.rdata`. No texture count is changed by construction.

005C81F0 publishes copied A0 and increments its actual +04 at005C85C7 before
copying all four UV DWORDs.005C7B80 compares identities; on change it publishes
new, retains new at005C7E4D, decrements old at005C7E5B, and dispatches old current0
at005C7E6B only on zero. UV copying follows, including the identity-equal path.

The destructor captures A0 at005C8B58, decrements at005C8B6B, dispatches current0
at005C8B7B if zero, then clears A0. The reconstruction reuses the established
raw-native retain leaf004DDB20 and `release_native_render_actual_owner`. That
helper decrements the SAME atomic and resolves the canonical companion only
on zero, validating that the companion borrows that actual atomic. Its terminal
method must select the owner's current profile and must not throw. The provider
and canonical companions must outlive all record references. A raw D3D COM
pointer does not satisfy this contract.

005C9E30 uses005C9030->005C81F0 for spare-capacity insertion. Reallocation uses
005C9C10->005C9530: retain-copy old records through005C8F90->005C81F0, fill the
new range through005C9250->005C9030, destroy old records through005C9290->005C8AE0,
then free old storage. Erase uses005C8880->005C8170->005C7B80 and destroys the
tail via005C8AE0. No native move-steal operation was found on these routes.
`MissionPictureOwner` declares copy/destruction and no move operation, so C++
rvalues also retain-copy the picture. Host vector allocation policy and moves
of other semantic record members are not the native STL implementation.

Whole-record destructor ordering of other semantic members, native SEH and
allocator failure behavior remain outside this picture fragment. Reached
native reference operations require a valid registered domain and normal return;
invalid-domain exceptions during C++ destruction terminate. No source claim is
made about calling an absent native profile or translating native SEH faults.

## Partial metadata route and integration

`read_mission_record_metadata`, `read_mission_group_metadata`, and
`load_mission_tree_metadata` parse authored fields but leave the optional picture
disengaged. If rereading an already bound record through this deliberately
partial route, they release/reset its old owner at the picture branch. This is
an explicit metadata adaptation, not native empty-picture behavior. Copies of
metadata records do not create or retain a picture.

An engaged picture after a successfully returning complete read may hold null
from an empty name or null renderer result. Consumers must distinguish that
state from disengaged metadata. The parent integrates the A0 consumer;5966F0
still supplies its own unit rectangle to AB2690 and does not read stored+A4.
This packet leaves menu headers and selection behavior unchanged.

The complete overloads take `const MissionPictureTextureServices&`. The game
host's single existing call and two existing map-size fixtures were changed to
the explicit metadata names. No new permanent test was added.

## Ghidra boundaries and verification

All live batches used the verified read-only bsp.py wrappers for
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, port8089. No annotations,
prototypes, body definitions or saves were changed by the worker.

Stored005C8AE0 currently ends005C8B2F after the incorrect no-return free call.
`ghidra disasm 005C8AE0 --start005C8B30` rejects the continuation as outside the
stored body. Disk decoding and live bytes recover005C8B30..005C8DAC inclusive;
005C8DAD begins INT3 padding. This is a continuation, not a new function entry.
Supporting005C9530 has gaps005C96C7..005C96C9 (ADD ESP,4) and005C9701..005C970C
(ADD ESP,4; PUSH0; PUSH0; CALL BF6885). These remain explicit raw-flow evidence
for the integrator to repair; no missing-body inference uses an interior proto.

`./scripts/build.ps1` passes MSVC Win32 Release and the existing reconstructed_math
CTest. The ignored fixture `local/mission_picture_probe.cpp`, compiled as the
only fixture translation unit against the resulting libraries with
`/link /MANIFEST:EMBED`, passes actual+04 count, canonical zero callback, UV
publication, copy/rvalue/vector, same-identity reread, empty/null and missing
binding checks. It uses local native-layout fixture objects and callback
boundaries; it is not an installed texture/renderer or game ABI differential.

The isolated game command in the JSON report exits C0000005 during settings
initialization before mission loading. The last log reports initial fullscreen
2560x1440 settings. This provides no picture-path runtime evidence. The parent
requested no further game/settings investigation. Build, fixture, source
evidence, unbound application providers and that pre-entry failure stay separate.
