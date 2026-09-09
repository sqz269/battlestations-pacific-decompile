# Shader RenderStates table conversion

Read-only assembly-backed investigation, 2026-09-09. Client.verify() verified
project bsp, program /battlestationspacific.exe before each analysis batch.
No C++, installation files, Ghidra state/names or shared metadata changed.
Raw exports are ignored under exports/bsp/shader_descriptor. No runtime fixture
was executed. Descriptive function names below are hypotheses.

## Lookup, order and ABI

00b579b0 takes three stack pointers (destination state-list header, definition
registry header, RenderStates Lua reference), ends RET Ch at00b57b3f, and
does not use incoming ECX as an object. The decompiler's float third parameter
is a stack-slot reuse artifact: the incoming argument is a Lua reference
pointer, later overwritten with a converted DWORD.

Shader reader00b43b00 first tests Shader.RenderStates for table type. If absent
or non-table, it skips00b579b0, leaving existing destination unchanged. It
passes descriptor+B8h and singleton0108fe90+4h as destination and registry.

00b579b0 walks the **definition registry in its stored order**, not lua_next.
Each definition is10h bytes: native key string at+0/+4, state ID+8, conversion
tag+Ch. It looks up that exact key through00b68100 ->00b67800, checks the
returned Lua value with00b66050, then destroys the temporary reference. Only
exact Lua NUMBER type3 passes. Missing values, Boolean values and numeric
strings are skipped, rather than coerced or assigned zero.

For a passing key it looks up the same key a second time, performs conversion
selected by the definition tag, then calls00b567b0. The second lookup is real
native behavior; an adapter should preserve it if Lua lookup can have side
effects. The path reaches00a67c20 ->00a6d420, a VM table lookup path; it is not
a raw lookup of the table array. No uppercase normalization or unknown-key
reporting is performed. Unregistered keys are never visited.

## Conversion and insertion

Tag0 calls00b66290: obtain Lua numeric value through00a67770, round to
float32 by FSTP/FLD, then00bf7420 truncates to signed integer, preserving its
EAX bits as the DWORD payload. Tag1 calls00b66270 and spills ST0 to float32
at00b57ac4; those **float bits** become the DWORD payload, not an integer
conversion. Tag1 therefore preserves ordinary signed-zero float encoding.

The integer SSE path in00bf7420 uses CVTTSD2SI after a double spill. Its
alternate CRT path and all exceptional input behavior are not re-established
here; use the previously documented conversion boundary in SHADER_FIELD_TABLES.
Do not replace float32 rounding followed by signed conversion with direct
Lua double-to-unsigned conversion.

There is no second type guard after the second lookup. Both conversion helpers
use Lua numeric coercion on that second result: numeric strings can convert,
failed conversions produce zero. A metatable returning a number first and a
numeric string second must therefore not be treated as failing the first gate.

Both known tags are exhaustively represented by this registry. The converter
has no defined fallback assignment for other tags: it would reuse a stack
slot (initially the Lua reference pointer, subsequently a previous payload).
A supplied-registry host adapter should reject unknown tags explicitly.

00b567b0 / proposed BSP_StateList_AppendUnique: ECX=destination header,
stack stateID then DWORD payload, RET8. It searches existing 8-byte records
(ID,payload); finding the same ID returns without updating it. Otherwise it
appends, growing capacity to max(oldCapacity+5,10). **First ID wins**, including
records that were present before conversion. The helper neither clears nor
sorts its output, and allocation failure is not a recovered safe error API.

## Verified render registry

Constructor00b585a0 registers the singleton through00b56610, installs vtable
00d62260, and initializes the render definition array at+4h. Each registration
uses00b58200 ->00b58050, which appends without sorting. Assembly keeps ECX
at manager+4 through the last render definition00b591ff. At00b5925e the
constructor switches to manager+10h for sampler definitions, and later+1Ch
for texture-stage definitions. Those are separate tables and are excluded.

Machine-readable evidence: reports/shader_render_state_registry.json. All
31 keys, IDs, conversion tags and registration call sites follow, in order.
IDs are D3D9 render-state values; the conversion performs no intermediate
engine-ID remapping.

| Order | Exact key | D3D state ID (decimal) | Tag | Registration call |
|---:|---|---:|---:|---|
| 0 | ZWRITE | 14 | 0 | 00b5863f |
| 1 | ZTEST | 7 | 0 | 00b586a2 |
| 2 | ZFUNC | 23 | 0 | 00b58705 |
| 3 | ALPHABLEND | 27 | 0 | 00b58768 |
| 4 | ALPHATEST | 15 | 0 | 00b587cb |
| 5 | ALPHAFUNC | 25 | 0 | 00b5882e |
| 6 | ALPHAREF | 24 | 0 | 00b58891 |
| 7 | CULLMODE | 22 | 0 | 00b588f4 |
| 8 | BLENDOP | 171 | 0 | 00b5895a |
| 9 | SRCBLEND | 19 | 0 | 00b589bd |
| 10 | DESTBLEND | 20 | 0 | 00b58a20 |
| 11 | COLORWRITEENABLE | 168 | 0 | 00b58a86 |
| 12 | COLORWRITEENABLE1 | 190 | 0 | 00b58aec |
| 13 | COLORWRITEENABLE2 | 191 | 0 | 00b58b52 |
| 14 | COLORWRITEENABLE3 | 192 | 0 | 00b58bb8 |
| 15 | MULTISAMPLEANTIALIAS | 161 | 0 | 00b58c1e |
| 16 | BLENDOPALPHA | 209 | 0 | 00b58c84 |
| 17 | SEPARATEALPHABLENDENABLE | 206 | 0 | 00b58cea |
| 18 | SRCBLENDALPHA | 207 | 0 | 00b58d50 |
| 19 | DESTBLENDALPHA | 208 | 0 | 00b58db6 |
| 20 | STENCILENABLE | 52 | 0 | 00b58e19 |
| 21 | STENCILREF | 57 | 0 | 00b58e7c |
| 22 | STENCILMASK | 58 | 0 | 00b58edf |
| 23 | STENCILWRITEMASK | 59 | 0 | 00b58f42 |
| 24 | STENCILZFAIL | 54 | 0 | 00b58fa5 |
| 25 | STENCILFAIL | 53 | 0 | 00b59008 |
| 26 | STENCILPASS | 55 | 0 | 00b5906b |
| 27 | STENCILFUNC | 56 | 0 | 00b590ce |
| 28 | FILLMODE | 8 | 0 | 00b59131 |
| 29 | DEPTHBIAS | 195 | 1 | 00b59198 |
| 30 | SLOPESCALEDEPTHBIAS | 175 | 1 | 00b591ff |

## Installed debug shader and existing renderer integration

shaderfx/common/debugshader.shfx supplies ZENABLE=0, ZTEST=0 and ZWRITE=0.
There is **no ZENABLE registry entry**. ZTEST maps directly to state7,
D3DRS_ZENABLE; ZWRITE maps directly to state14, D3DRS_ZWRITEENABLE. Thus
on an initially empty output, conversion yields exactly this ordered pair:

```text
{ state=14, value=0 }  // ZWRITE: D3DRS_ZWRITEENABLE
{ state= 7, value=0 }  // ZTEST:  D3DRS_ZENABLE
```

The unrecognized ZENABLE entry contributes nothing. Do not invent an alias
for it, map ZTEST to ZFUNC, or add a third state. Lua declaration/hash order
has no effect on this output.

The existing include/bsp/d3d9_states.hpp RenderStateValue and RenderStateBlock
can carry these IDs/payloads directly. D3D9StateCache::set_render_state_00b24460
accepts the corresponding D3DRENDERSTATETYPE and DWORD. The existing block
application in src/d3d9_states.cpp loops its stored records and calls that
setter. Feeding the recovered pair to that API is a concrete host integration
route; it does not require a new Boolean-depth abstraction or inferred state
mapping. Full material ownership, override stacking and restoration remain
separate from this descriptor conversion.

No claim of full descriptor ABI compatibility, rendering parity or game runtime
validation follows from this analysis alone.

## Adapter integration follow-up

The parent added the verified31 definitions to src/shader_render_state_registry.inc
and uses them in the Lua adapter. It preserves definition order, the two distinct
lookups, second-value numeric coercion, float payload bits and first-ID-wins.
Unsupported signed conversions fail explicitly instead of claiming native error
parity. The installed debug table produces [{14,0},{7,0}], then feeds the recovered
state-block binder. Both device values read back0 before the expected pixel draw.
Build, existing tests and full probe pass; float-tag exceptional values and
metatable side effects have not been fixture-tested. See SHADER_COMBINERS.md
and reports/shader_combiner_states.json.
