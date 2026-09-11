# Actual shader sampler parsing and Lua traversal

Addresses: 00B57B50 00B41830 00B34620 00B66A60 00B662B0 00B66000 00B661B0 00B662F0 00B66420 00B67080 00B67190

The full sampler reader and sampler-table append now operate on the actual
44-byte sampler, 272-byte descriptor, 40-byte definition manager, 20-byte Lua
objects and 1224-byte Lua owner. They use the existing native string domain,
state-list pool and state reader. Names are descriptive hypotheses, not recovered
symbols. Source is `src/native_shader_sampler_reader.cpp`,
`src/native_lua_objects.cpp` and the shared class binding in
`src/native_shader_sampler_owner.cpp`; evidence is
`reports/native_shader_sampler_reader.json`.

## Entry reader B57B50

Original ABI: ECX definition manager, one stack pointer to the actual Lua entry,
EAX newly allocated sampler, RET4. The complete range is B57B50..B57FCC.
The 2Ch allocation initializes names04/18, vertex byte0C, source14, index20 and
holders24/28. Type10 and padding0D..0F retain their allocation preimage until
Type is read; padding is never initialized by this function.

| Field | Native read and publication |
| --- | --- |
| Name | Required lookup, Lua string/number coercion, temporary NativeString, copy to04, release temporary name then Lua object |
| Type | Required lookup, float32 then live CRT integer conversion; store10 before Lua cleanup |
| TextureSource | First lookup must pass the float32 integral-number predicate; release it, perform fresh integer lookup, release it, then store14 |
| TextureSourceName | Required string/number coercion only when converted source is1 or3; same temporary-name lifetime as Name |
| Index | Integral-number gate and fresh coercion; store20 before second Lua cleanup |
| VertexSampler | Exact Boolean gate and fresh Boolean-or-default conversion with fallback0; store byte0C before cleanup |
| SamplerStates | Table gate, release, always allocate/init a pooled0Ch header; if present read a fresh table through B579B0 using manager+10; publish24 after cleanup |
| TextureStageStates | Same sequence using manager+1C; publish28 after cleanup |

The two state headers are allocated even when their tables are absent. Nil,
Boolean and table values do not acquire invented string defaults for required
names. Number-to-string coercion borrows the live Lua string before copying it;
the subsequent native C-string constructor uses the first NUL. Numeric strings
fail the integral-number gate but can be accepted by a fresh conversion after a
different gate result. Metatables can therefore change a field between reads.

FuncInfo DF919C and map DF91C0 contain fifteen unwind states. Thunks CC0590 through
CC0600 clean Lua temporaries; states1 and6 additionally clean the corresponding
temporary NativeString before the enclosing Lua object. No unwind state owns
the allocated sampler or either pooled header. The reconstruction preserves this
boundary rather than adding an allocation rollback. Original exception dispatch
has been inspected, not executed by the fixture.

## Outer reader and ownership

B41830..B419A8 uses ECX descriptor, one stack Shader-object pointer and RET4.
It checks `Samplers`, releases that reference, then fetches it again. The table,
key and value are real tracked stack objects. It tests VALUE for end of iteration
and accepts any key passing B66A60, including zero, negative and sparse integer
keys. It neither clears nor sorts existing C4 entries.

For every accepted key it reloads global0108FE90, calls B57B50, captures the
returned pointer and appends it. When count equals capacity, growth is
max(capacity+4,6). B34620..B3467E clamps signed requests to six, allocates request*4
bytes, copies current live count/data, frees the old allocation, then publishes
pointer and capacity. Its false CALL_RETURN after `_free` at B3466C was repaired
under the Ghidra write lock; nine bytes B34671..B34679 are now part of the listing.
The allocator thunk BF55BE jumps to the already established BF681B allocation
contract. A reserve failure after sampler construction does not delete that sampler.

FuncInfo DF796C and map DF794C have four unwind states: the initial gate, then
table, key and value. Normal and unwind cleanup order is value, key, table.

The new C++ outer interface explicitly requires a
`NativeShaderSamplerClassBinding`. It installs one shared host callable table in
each parsed sampler, allowing existing descriptor C4 virtual0/flags1 cleanup to
destroy all of them. The binding stores the shared pool/string dependencies and
owns no objects, reference count or per-object registry. It must outlive every
bound sampler. The single-entry reader retains numeric D621F4; virtual-table
adaptation is an explicit host boundary of the outer reader. These interfaces
are not binary replacements for the original calling conventions.

## Actual Lua helpers

| Routine | Original ABI and established behavior |
| --- | --- |
| B66A60 | ECX object, AL Boolean, RET. Kind2/NUMBER required; float32 spill, CVTTSS2SI and ordered equality. NaN/infinity fail; float32-rounded integral values can qualify |
| B66000 | ECX object, AL Boolean, RET. Kind2 and exact Lua Boolean |
| B661B0 | ECX object, AL Boolean, RET. Kind0 false; kind2 checks Lua table; every other nonzero kind true without interpreter access |
| B662B0 | ECX object, EAX borrowed C-string, RET. lua_tolstring(owner.state,index,nullptr), with no kind guard |
| B662F0 | ECX object, stack fallback byte, AL result, RET4. Exact Boolean yields0/1; otherwise preserves the supplied low byte |
| B66420 | Receiver unused, stack object pointer, AL(kind==0), RET4 |
| B67080 | ECX table, stack key/value, RET8. Release existing value then key; checkstack2, push nil, lua_next; publish/register key then value |
| B67190 | Same ABI. Release value; a top key is untracked without popping, otherwise copy its captured index then remove its current tracked index; lua_next and publish/register results |

Iteration leaves each object's word0C and padding unchanged. Registration stores
the actual object address in the owner's slot, updates its high-water mark and
uses kind2/index/top tracking. There is no Lua-registry handle substitution.
Reference release retains the established last-swap, shifted-index and stale
high-water behavior from `NATIVE_SHADER_STATE_READER.md`.

## Verification and limits

The strict MSVC Win32 build and both existing CTests pass; no permanent tests
were added. One ignored fixture executes all eleven complete original routines,
with the previous packet's actual Lua/state-reader code as captured dependencies.
Calls into stock Lua5.1.1 use ABI adapters; strings, definition ownership and
pooled allocation use the established concrete implementation.

The fixture compares complete normalized1224-byte owner and20-byte object
snapshots across repeated first, top/non-top next, empty/end and cleanup paths.
It also compares thirteen Lua values across five object kinds and five
predicates/getters, exact fallbackA5, both integer-conversion modes, fresh
metatable lookup traces, parsed sampler contents, and string allocation/release
sequences. A seeded C4 grows to twelve entries/capacity14, preserving the seed
and Lua order. Changing the published definition slot during the first sampler
proves the first reader retains its captured manager while later entries reload
the new manager. Descriptor virtual cleanup returns all state slots.

Current installed `debugshader.shfx`, `alphablend.shfx` and `dx9_lua.inc` were
hashed and evaluated read-only. The full sampler-table parser returns no sampler
for debugshader and `MyTexture`, type2, source0, index0, vertex0 for alphablend;
its sampler rows are(1,1),(2,1), with an allocated empty texture-stage header.
The fixture's required DoFile callback selects the installed loose include; this
does not reconstruct the game's VFS callback or full descriptor loader.

All24 captured spans match the installed PE, including both unwind maps and nine
key literals. All eleven stored bodies cover their complete captured extents.
Padding bytes in heap-allocated samplers are intentionally excluded from value
comparison; the prior initializer fixture established their preservation.
Original exception ABI, allocation-failure unwinding, shader creation/rendering
and gameplay remain unvalidated. Counts/extents, stable tracked-object addresses,
live shared domains and the native50-slot/five-reference bounds remain caller
contracts.

## Follow-up packets

Recover the actual field-table allocation B573F0/B419B0 and complete descriptor
reader B43B00, checking current leases before work. Keep native Lua owner bootstrap,
callback frame-offset tracking and VFS DoFile recovery separate. The sampler
reader and actual state-definition/state-list dependencies are now reconstructed;
the older semantic parser remains a separate interface.
