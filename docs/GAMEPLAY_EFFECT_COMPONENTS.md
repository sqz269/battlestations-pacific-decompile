# Gameplay effect component construction and loading

Addresses: 00870400, 00868bf0, 0086bc80, 0086b8b0, 0086bcc0, 0086bd30,
0086bd70, 0086bdc0, 0086be00, 0086be40, 0086cf90, 0086cfe0, 0086d030,
0086fec0, 008700e0. Sound inline constructor: 00870733..00870763.

Packet `orch4_effect_components_n` reconstructs the full normal definition loader,
all twelve called constructors, Sound's inline constructor, and the shared base
Lua reader. ID acquisition now calls the concrete loader. Individual component
virtual readers and zero-reference dispatch remain required services. Names are
hypotheses; these are new C++ interfaces, not binary replacement vtables.

## Allocation and actual storage

Every constructor uses ECX=allocation and returns that same base in EAX, RET.
Sound instead begins inside00870400 with EAX=allocation and EBP=0. No constructor
calls an unresolved helper or allocates additional payload storage. Allocation
remains the existing native-size CRT/new-handler service. Untouched bytes are
preserved; no blanket zero initialization or shadow component container exists.

| Type | Allocation | Constructor | Component offset | Final component vtable |
|---|---:|---|---:|---|
|Sound|3Ch|00870733 inline|0|00D0DA18|
|Particle|34h|0086BC80|0|00D0D5B4|
|Tracer|A0h|0086B8B0|80h|00D0D590|
|WaterTracer|C4h|0086BCC0|0|00D0D5D4|
|Shake|2Ch|0086BD30|0|00D0D5F4|
|Flare|34h|0086BD70|0|00D0D614|
|ThunderStorm|58h|0086FEC0|0|00D0DA38|
|Waterdrops|4Ch|0086BDC0|0|00D0D634|
|ConstRumble|30h|0086CF90|0|00D0D76C|
|SlopeRumble|30h|0086CFE0|0|00D0D78C|
|SquareRumble|40h|0086D030|0|00D0D7AC|
|Light|48h|0086BE00|0|00D0D654|
|Splash|2Ch|0086BE40|0|00D0D674|

Ordinary bases write CEB130 then references4=1, name8/C=0, byte10=0,
Delay14=0 and NoFilterDist18=+0, followed by the derived vtable. Padding11..13,
word1C and derived payload fields remain untouched unless listed here:

- Sound: zero20/24/28.
- Particle: zero20/24/28/2C/30.
- WaterTracer: zero48/4C/50/54/64/68/6C/70/8C.
- Flare: zero28/2C/30.
- ThunderStorm: zero40/44/48/4C/50.
- Shake, Waterdrops, all three rumbles, Light and Splash: no additional writes.

Tracer has a distinct primary base and a secondary component at+80. Its primary
vtable changes D0D4A4->D0D5B0; zeros4C/50/54,18/1C,8/C,40/44, byte4=0, byte5=1,
and word20=3. Five verified read-only image words supply10=3F800000,
14=3DCCCCCD,28=7F7FFFFF,2C=41200000 and secondary98=453B8000 (3000.0f).
Their original addresses, section flags and disk/live byte checks are recorded.
Secondary vtable80 changes CEB130->D0D570->D0D590; refs84=1, name88/8C=0,
byte90=0 and Delay94=0. Other bytes, including padding and9C, are untouched.

The C++ constructors begin a real eight-byte NativeString subobject at component+8
while performing exactly the corresponding two zero stores. This lets the already
linked0041E350 API assign the actual header without a copied temporary. No string
allocation occurs during construction. A separately exported raw helper in the
physical-file packet was not linked into bsp_core; this packet does not pull that
unrelated module in or modify its leased function.

## Definition loader00870400

Original ECX=actual24h definition, one stack LuaObject pointer, RET4 at00870AB1.
Iterate the supplied table in native Lua order. For string keys, construct separate
pooled copies for case-insensitive Name, then Type, then Comment comparisons,
stopping at the first match. All constructed key temporaries die in reverse order
before processing the row. Non-string keys skip these comparisons.

For an unreserved row, retain its Type Lua field, copy the Lua string to a pooled
temporary, and release the Type reference before looking up Platform. Type uses
the existing nonnull-string constructor contract; nil/non-string values are not
silently turned into unknown types. Platform defaults true unless its Lua type is
BOOLEAN, in which case use its actual boolean value. A numeric0 therefore enables
the component. Release Platform before any allocation.

Compare the thirteen type names in the order above, case-insensitively. Unknown
types and allocation-null branches append nothing. Tracer alone adjusts the
constructor result by80h. Call CURRENT component vtable+14 with the retained row;
this remains the explicit real-reader service. Afterwards, a string key replaces
the actual component name8/C via0041E350. Numeric keys preserve the reader's name.

Append the actual component pointer through concrete0086EB60 on definition+8;
the existing pointer/count/capacity array is not cleared. The append retains its
stored reference. Then decrement the originally captured component+4 atomically
and dispatch CURRENT slot0 if it reaches zero. The captured component and mutable
temporary source slot are kept distinct. Type string cleanup happens after this
release, followed by advancing Lua iteration. Value/key references die at exit.
Constructor/read/name failures are not given invented rollback beyond the native
normal sequence. C++ exception cleanup remains an adaptation, not SEH proof.

## Shared base reader00868BF0

ECX=component, stack LuaObject pointer, RET4 at00868CC5. Read Autostart and store
Lua truthiness as byte10 before releasing its reference. Read Delay, accept only
Lua NUMBER, spill through the existing binary32/late CRT integer conversion, and
store signed14; other types store0. Numeric strings are not accepted as numbers.

Look up NoFilterDist BEFORE loading the current component18 default. The native
x87 FLD/FSTP round trip is retained. Then accept only Lua NUMBER through the
existing float32 accessor; other types use that captured current default. Store18
before releasing the retained reference. This matters when an __index metamethod
changes component18 during lookup; the fixture observes that changed default.

## Ghidra and validation

Fourteen stored no-input/undefined-return prototypes were corrected under the
write lock. Constructors now expose their verified ECX input and EAX pointer
return. The two loaders use an explicit fastcall analysis model with unused EDX
occupancy so the Lua argument stays on the stack; EDX is not a consumed native
input. Prior values are retained, and all readbacks were verified. The first
save met an active analysis transaction; retry saved the same changes without
discarding the original before-state. No body, flow override or inventory tag
changed in this packet.

MSVC Win32 Release and both existing tests passed. Native seed verification also
passed. One focused fixture compares all13 constructor outputs byte-for-byte with
the original x86 instructions using patterned incoming storage and guard bytes.
The12 leaf bodies execute directly in isolated scratch allocations; Tracer's five
absolute read-only constants are redirected to verified local words. Sound's
inline stores use a tiny adapter establishing EAX/EBP and preserving EBP. This
proves isolated constructor storage behavior, not full binary ABI compatibility.

The same fixture runs concrete ID acquisition through all13 factories with real
Lua5.1.1, base-field reads, a metamethod-driven current-default change, reserved
key allocation counts, Platform filtering, string/numeric key naming, actual
reference growth, a cache hit and full definition teardown through fixture
component-release services. The existing startup fixture also passed after its
service binding was updated. Exact source/library/combined-build provenance is
in `reports/gameplay_effect_components.json`. Component-specific fields, runtime
virtual behavior and gameplay remain unvalidated.

## Follow-up packets

- Component virtual+14 readers. Sound0086EF60 now has a concrete base-reader
  dependency; continue with sample acquisition00A83FD0 and sample assignment005B9AF0.
- Concrete virtual slot0/scalar destructors and component resource cleanup;
  preserve Tracer secondary-base adjustment in both dispatch directions.
- Startup reference-vector append004D9C00 and remaining acquisition consumers.
- Stored cleanup-body extents remain pending behind the documented script gate.

## Verified virtual targets for follow-up ownership

All13 current slot0 entries resolve to00BD30E0. It invokes the current slot4
with flags1; these are payload-specific scalar targets, not newly reconstructed
destructors. The slots below were checked against both disk and live Ghidra.

| Component | Current slot4 | Current Lua slot14 |
|---|---|---|
|Sound|0086FB00|0086EF60|
|Particle|0086BC60|00871D00|
|Tracer|0086B9A0|00858700|
|WaterTracer|0086D080|0086D180|
|Shake|0086D0A0|00868DE0|
|Flare|0086E870|0086C240|
|ThunderStorm|0086FF00|0086F3B0|
|Waterdrops|0086D0C0|0086B2B0|
|ConstRumble|0086D0E0|00868FA0|
|SlopeRumble|0086D100|00869080|
|SquareRumble|0086D120|00869160|
|Light|0086D140|00869FD0|
|Splash|0086D160|008694C0|

## Correction from docs/GAMEPLAY_EFFECT_SCALAR_COMPONENTS.md

Seven current component tables now have concrete Lua readers and scalar
cleanup: Shake, ConstRumble, SlopeRumble, SquareRumble, Light, Splash and
Waterdrops. Acquisition requires their dispatcher, also usable for definition
destruction. The remaining six table families still require services.
Waterdrops retains a required current-renderer texture binding. See the new
document for exact field/default rules, tracked color/string helper limits,
texture callback order, flow repair and validation provenance.
