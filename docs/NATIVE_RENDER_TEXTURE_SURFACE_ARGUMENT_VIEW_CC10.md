# Raw B4E020 argument view

The complete 283-byte constructor at `[00B4E020,00B4E13B)` now accepts six
initialized, address-stable DWORD cells through
`NativeRenderTextureSurfaceOwnerArgumentView`. The existing typed argument API
is preserved. Both entries build addresses-only metadata and invoke the same
constructor engine; neither snapshots argument values. The old heterogeneous
struct remains live typed storage, including its pointer member. The raw view
does not start an aggregate lifetime over its caller's DWORD backing.

This supports later raw initializer composition without replacing actual
texture/surface/renderer ownership or introducing a dispatch callback.

| Native body | Coverage | Original ABI |
|---|---|---|
| B4E020..B4E13A | Complete normal body, existing valid-provider domain | ECX actual18h owner; six stack words; EAX same owner; RET18h |
| CBFAB0..CBFAB7 | Boundary evidence; existing source base-cleanup projection | `[EH EBP-20h]` receiver; tail BD30F0 |
| CBFAB8..CBFAC1 | Boundary evidence, missing live function | DF862C descriptor then tail BF6B43 |

The six cells contain width, height, format, multisample, mode, and external
surface pointer bits. Metadata is immutable and disjoint from all mutable
native storage. Each cell has a real initialized DWORD lifetime and remains
available through synchronous provider callbacks. Native saved-register,
return-address and unexposed nested-provider stack aliases remain excluded.

Native E04F reads the mode **byte**, E053 reads format, E057 reads height,
E060 stores owner14, and only then E063 reads initial width. Captured height
and format survive subsequent calls. After texture and level construction,
E0A4 rereads the external pointer. If separate mode still requires a new
target, E0DC rereads multisample and E0E0 rereads width. The actual owner-mode
comparison precedes texture publication and the level call. At the end, the
existing implementation reloads current owner8/profile54 and invokes the
genuine B3D640 RET4 no-op.

All renderer/texture/surface factories, imported increments/decrements, pool
identities, current table read points, reference credits and owner field
stores remain in the shared engine. Only base cleanup is armed; no partial
resource rollback is added. Source exception cleanup does not establish native
FH3/SEH or private-stack fault behavior. Existing admitted profiles and valid
allocation/provider contracts still apply; this is a source interface rather
than a drop-in native ABI replacement.

Strict Win32 build passed all three existing CTests. Compiled-object inspection
pins BYTE mode at shared engine+6A, format+75, height+89, owner14 store+94,
initial width+9F, level call+15F, current external+182, multisample+279,
width+289 and target factory+2B3. The two overloads share that engine.

One ignored focused comparison executes copied-original283B and source with
real hidden-window D3D9 HAL, genuine texture/surface constructors, pools,
registrations, actual counts and cleanup. Its renderer is an explicit zeroed
1D94h field fixture with real device/resource domains; it does not establish
full renderer construction or application startup.

Immediately after a genuine Win32 increment retaining the level surface in
the texture cache, an explicit harness callback changes the live argument
cells: width64 to32, height32 to17, format22 to113, multisampleFFFFFFFF to0,
modeABCD0001 to0, and a genuinely constructed external surface pointer to null.
Both lanes produce the original64x32 texture and a new32x32 format22 target,
retain captured mode1/height32/format22, and do not retain the stale external
surface. Whole18h outputs match after normalizing only the three resource
identities; bytes15..17 preserve CCh. Real retirement restores registrations
and tracking, the genuine singleton pool/support domain drains, and final
device/API references are0/0. The original argument wrapper exposes its actual
six pushed words; no dead private-stack result is compared.

The harness borrows the earlier R75 fixture, linking 70 current project objects
and three current project libraries with `/MD /MANIFEST:EMBED`. Original
constructor modifications are one rel32 factory operand, two renderer
publication operands, two IAT operands, and the unreached EH immediate to a
fail-fast trap. Indirect call instructions are unchanged. Admitted virtual
targets reach ABI adapters for genuine existing providers. The old fixture's
copied destructor/scalar bodies remain allocated but are not invoked by this
comparison; retirement uses the existing genuine source lifetime. The explicit
callback mutation is a test mechanism, not observed game behavior.

Fresh live/PE evidence pins301 code bytes and44 descriptor/map bytes. The
report includes all eight normal calls and both compiler tail jumps. The
report verifier checks three direct/tail rows: two pass; its sole expected
failure is CBFABD because CBFAB8 lacks a live function. Root definition must
cover inclusive CBFAC1; final JMP atCBFABD is5 bytes. Its row remains
`kind=tail_jump`, independently of missing-function metadata. Workers made
no Ghidra writes.

Evidence is under ignored `local/cc10_render_texture_surface_argument_view/`;
the report pins its exact source, compiled object, fixture/link inputs and
logs. No tracked test, application run, initializer admission or B107F0
activation was added. Native failure/EH paths, unsupported profiles, full
post20 construction and source0 sampler preimages remain unproved.

The three frozen initializer-readiness files are hash-pinned in the report and
included unchanged in this packet's evidence archive. They retain the full
B4F560 physical stack/state audit and explicit unknown34/38/3C preimage hazard;
this supporting view does not resolve that separate initializer contract.
