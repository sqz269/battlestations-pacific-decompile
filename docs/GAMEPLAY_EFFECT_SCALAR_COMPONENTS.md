# Scalar effect component readers and destruction

Addresses: 00868DE0, 00868EC0, 00868FA0, 00869080, 00869160,
008694C0, 00869FD0, 0086B2B0, 0086B7E0, 0086CD40,
0086D0A0, 0086D0C0, 0086D0E0, 0086D100, 0086D120,
0086D140, 0086D160; helper contracts 00B67D40 and 00B685C0;
acquisition integration 008700E0.

These descriptive names are hypotheses, not recovered symbols. The seven
compiler scalar-deleting-destructor names remain intact. Existing Ghidra
project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, was
verified before each batch. Full-span disk/live hashes, original prototype
values, repaired flow, validation hashes and integration revisions are in
`reports/gameplay_effect_scalar_components*.json`.

## Concrete call chain and limits

Acquisition008700E0 now requires `GameplayEffectScalarComponentDispatcher`.
The existing concrete definition loader00870400 constructs actual component
allocations, and this dispatcher reads their current vtable word. Seven
read-only image tables dispatch directly to the readers below. Their slot0
entries are00BD30E0, which invokes current slot4(1); the dispatcher therefore
uses the verified scalar cleanup. Reuse this dispatcher in the definition
destruction context. Other current tables require the supplied remaining
services; there is no default successful reader or cleanup.

This implements seventeen complete normal-flow bodies and two tracked-Lua
helper projections. It does not implement effect ticks, renderer/texture
internals, or the remaining Sound, Particle, Tracer, WaterTracer, Flare and
ThunderStorm virtual readers/destructors. The texture service must resolve
the current renderer and current texture virtuals at the specified call site.
The fixture provides controlled resources and a Sound fallback; those are
fixture bindings, not game implementations.

## Field reads

Every reader first calls concrete00868BF0 (through00868EC0 for rumble
derivatives). Fields below are relative to the actual component. A field is
looked up and retained, converted, stored to the owner, then released before
the next lookup. Bare numbers accept Lua numeric strings and nil converts
to zero. Defaulted numbers accept only Lua Number. Integer conversion uses
the existing binary32/late-current-CRT conversion, including hardware
indefinite results, rather than a C++ out-of-range cast.

| Reader | Actual field order and conversion |
|---|---|
| Shake00868DE0 | Persistent byte20 truthiness; Radius float24, Strength float28 bare |
| Rumble base00868EC0 | PowerType int20 bare; TimeTotal float24, Radius float28 bare |
| ConstRumble00868FA0 | Rumble base, Magnitude float2C bare |
| SlopeRumble00869080 | Rumble base, StartMagnitude float2C bare |
| SquareRumble00869160 | Rumble base, MaxStart byte2C truthiness; MinMagnitude30, MaxMagnitude34, MinTime38, MaxTime3C bare floats; padding2D..2F untouched |
| Splash008694C0 | Radius20 default1; Amplitude24 default0.1; Speed28 default1 |
| Light00869FD0 | Duration20 default2; Radius24 default10; DiffuseColor28..34 default(0,1,0,1); SpecularColor38..44 default(0,0,1,1) |
| Waterdrops0086B2B0 | MinLifeTime20 default0.5, MaxLifeTime24 default4, MinSize28 default0.1, MaxSize2C default0.25, Gravity30 default0.95, Radius34/Range38/StickyPercent3C default10 |

Waterdrops next reads boolean-only ApplyAtBottomOnly40, StartTopOffScreen41,
ApplyAtTopOnly42 and NoCameraMove43, all defaultfalse. WaterdropCount44 is
number-only, default50. Lua number0 is truthy for Shake/Square but rejected
by these boolean-only accessors. Read-only image constants were checked
against live bytes; 0.1 is3DCCCCCD and 0.95 is3F733333.

### Light vector helper00B67D40

For retained field references the value must be a Table and its index5 must
be Nil. This is not a table-length check. Release the index5 reference before
copying fallback or accessing1..4. On acceptance, lookup1, convert with bare
lua_tonumber to binary32 and release; repeat2,3,4. Only after release4 publish
all four local words. The Light caller copies these words into the actual
owner before releasing the outer color field. Missing1..4 become zero;
numeric strings qualify; sparse index6 does not invalidate the value;
index5=false does invalidate it. Four-word order is preserved without an
inferred color-channel convention. Native non-kind2 borrowed-alias paths
are analyzed but are outside this private tracked-field implementation.

### Waterdrops texture and string helper00B685C0

Construct an actual pooled eight-byte temporary string from Texture only
when the retained reference is a Lua String; otherwise use `white.tga`.
This is construction through0041E870, not assignment with old-buffer cleanup.
Release Texture's Lua reference first. Then resolve renderer[00F8D394] and
its current vtable+64h, passing the temporary's address and flags0. Store
the returned owned texture directly to actual48: no release of the previous
slot and no extra retain. Destroy the temporary string afterward. The
renderer call remains a required resource boundary.

## Destruction and ABI evidence

Common0086B7E0 writes D0D570, captures actual name dataC, and for nonnull data
releases it using current length8 plus1 with DWORD wrap. It leaves the name
header untouched, including callback changes. Base00BD30F0 then writes
CEB130. Other component bytes and reference counts remain unchanged.

Waterdrops0086CD40 first writes D0D634 and captures actual texture48. If
nonnull, InterlockedDecrement(texture+4) precedes current texture slot0 only
on zero. After that callback it clears actual48 even if the callback
published a replacement; that replacement gets no extra release. Then it
runs the common destructor. A null texture goes straight to common cleanup.

| Component | Current table | Scalar deletion | Lua reader |
|---|---|---|---|
| Shake | D0D5F4 | 0086D0A0 | 00868DE0 |
| ConstRumble | D0D76C | 0086D0E0 | 00868FA0 |
| SlopeRumble | D0D78C | 0086D100 | 00869080 |
| SquareRumble | D0D7AC | 0086D120 | 00869160 |
| Light | D0D654 | 0086D140 | 00869FD0 |
| Splash | D0D674 | 0086D160 | 008694C0 |
| Waterdrops | D0D634 | 0086D0C0 | 0086B2B0 |

Six scalar wrappers invoke common0086B7E0; Waterdrops invokes0086CD40. Only
flags bit0 calls free. They return the original pointer in EAX even when
freed. Seven three-byte fall-through gaps after free were repaired with the
write lock, saved and re-exported; scalar compiler tags remain correct.

Assembly establishes ECX component plus one stack LuaObject pointer/RET4
for readers; ECX component/RET for ordinary destruction; ECX component plus
stack flags/EAX original pointer/RET4 for scalar deletion. Both Lua helpers
consume ECX reference plus stack output/fallback, return output in EAX and
RET8. Ghidra prototypes explicitly occupy unconsumed EDX to model the stack
arguments. This is an analysis storage model, not recovered source syntax.
The bridge rejected `const void *`; the saved helper fallback uses `void *`
with read-only intent documented here. Prior values and the rejection are
retained. Waterdrops pseudocode still misidentifies the string cleanup
registers; its actual stack header and cleanup order come from assembly.

## Validation and follow-up packets

MSVC Win32 Release and both existing CTests passed. One focused local fixture
uses real Lua5.1.1 and concrete acquisition of all seven types plus a required
Sound fallback, checks coercion/defaults, sparse color and index order,
Waterdrops reload without old release, texture callback replacement,
eight actual name releases and full definition teardown. A flags2 check
confirms the scalar wrapper does not free. The existing full startup fixture
also passed with the new mandatory dispatcher. Exact source/library hashes
and combined integration results are separate in the report.

No native reader/destructor differential execution, native ABI/SEH parity,
effect runtime behavior, rendered output or game validation is claimed.
Prior native constructor comparison results remain in the preceding packet.

Ready follow-ups, subject to fresh leases: Sound reader0086EF60 and scalar
0086FB00; remaining Particle/Tracer/WaterTracer/Flare/ThunderStorm readers
and their current scalar targets listed in `GAMEPLAY_EFFECT_COMPONENTS.md`;
startup effect-vector append004D9C00. A renderer resource binding requires
coordination with its existing owner. Borrowed Lua vector/string coverage
should be implemented only when a caller needs those paths.

## Correction from docs/GAMEPLAY_EFFECT_SOUND.md

The dispatcher now also handles SoundD0DA18 through concrete0086EF60 and
0086FB00. Its mandatory context includes Sound sample-cache and current-owner
bindings. Five component families remain outside concrete dispatch. The new
Sound/cache documents distinguish component behavior from the required7Ch
sample constructor and resource lifetime boundary.
