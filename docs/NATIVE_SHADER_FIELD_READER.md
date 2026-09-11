# Actual shader field records and table loading

Addresses: 00B573F0 00B419B0 00B34680 00B66380 00B685C0

The complete field readers now allocate actual1Ch records with native8h names
and append their pointers into actual0Ch array headers. Descriptor D0/DC cleanup
already destroys these names and frees the records. This implementation is
separate from the `ShaderField` projection used by semantic source-generation
adapters. Descriptive names are hypotheses, not recovered symbols.

Source: `src/native_shader_field_reader.cpp` and the two added helpers in
`src/native_lua_objects.cpp`. Evidence: `reports/native_shader_field_reader.json`.

| Routine | Coverage | Original ABI |
| --- | --- | --- |
| B573F0..B5762F | Complete576-byte field reader | Incoming ECX unused; stack actual Lua value; EAX new1Ch field; RET4 |
| B419B0..B41B09 | Complete346-byte field-table append | ECX unused; stack Shader/output-header/NativeString field-name; RET0C |
| B34680..B346DE | Complete95-byte pointer reserve | ECX0Ch header; stack signed request; RET4 |
| B66380..B663C6 | Complete71-byte integer/default helper | ECX actual Lua object; stack signed default; EAX signed result; RET4 |
| B685C0..B68622 | Complete99-byte fresh string/default helper | ECX actual Lua object, EDX unused; stack fresh8h output/fallback C-string; EAX output; RET8 |

## Record producer and missing values

B573F0 constructs tracked key/value objects, initializes an empty local name and
semantic index0, then iterates the supplied table. There is no preliminary table
type check. Only keys passing the float32 integral-number predicate are accepted.
Their actual numeric values do not select fields: accepted ordinal0 through4
select the five inputs, and later accepted values are still iterated but ignored.

| Offset | Produced value |
| --- | --- |
| 00/04 | NativeString length/data; ordinal0 requires actual Lua STRING, otherwise `undef` |
| 08 | Scalar type; ordinal1 uses coercing float32/native integer conversion |
| 0C | Component count; ordinal2 uses exact NUMBER conversion, otherwise0 |
| 10 | Component mask0 |
| 14 | Semantic; ordinal3 uses coercing float32/native integer conversion |
| 18 | Semantic index; ordinal4 uses exact NUMBER conversion, otherwise0; missing ordinal also0 |

If ordinal0 is absent the name remains empty. If ordinal1,2 or3 is absent, its
native stack local is never written. These three locations are entry-ESP minus
50h,4Ch,48h; body-ESP is entry-ESP minus64h, so the conversion stores at B57586,
B5759F and B575B6 write body offsets14h,18h,1Ch. They are not zero defaults.

The new C++ interface takes `NativeShaderFieldStackPreimage` explicitly, avoiding
uninitialized C++ reads while retaining these native inputs. The outer reader
requires `NativeShaderFieldStackInputs::for_entry` to supply them independently
for each inner table. This is a host input boundary, not a recovered game callback
or a claim that arbitrary native stack reuse has been reconstructed. Populated
fields overwrite all three inputs. The fixture seeds exactly these three native
stack cells before the original prologue and supplies identical values to C++.

B685C0 checks kind2 and exact STRING before calling lua_tolstring; otherwise it
constructs from the supplied fallback. It does not coerce a NUMBER to a string.
It constructs fresh output and does not release a previous output or its input
reference. B66380 checks kind2 and exact NUMBER; numeric strings take the full
signed fallback DWORD. Accepted numbers spill to float32 before the existing
live SSE2/x87 conversion path. Neither helper adds range/exceptional-number
rejection. C-string copying truncates at the first embedded NUL.

## Name copies and unwind

After ordinal0's temporary string is copied to the local name, the reader caches
the local length and data in EBP/EBX before releasing that temporary. These cached
values remain the source of the final field-name copy and normal local-name
cleanup. The destination length/data are reloaded after resize. Unwind cleanup
instead reads the current local string header, matching the native destructor.

FuncInfo DF90B4 and map DF90D8 have five states: key, value, local name, temporary
name and raw field allocation. State4's CC04E0 thunk reads the saved allocation
at EH-frame EBP+4 and calls the established `_free` service BF65AC; it does not
destroy the new field's name. Its predecessor is state2, which then destroys the
current local name, value and key. On success, the cached name is released before
value/key cleanup, and the completed field is returned.

The reconstruction frees raw field storage when output-name construction fails,
without inventing a field-name destructor in that unwind state. The rebuilt
fixture injects a failure into the third name allocation and verifies temporary
string release order and balanced tracked Lua objects. The original EH map and
thunks are verified statically; original exception dispatch was not executed.

## Outer traversal and descriptor ownership

B419B0 looks up the NativeString field name, checks TABLE, releases the gate,
then performs a fresh lookup. Both lookups use C-string semantics, independent
of the header's stored length. Missing or non-table input leaves output unchanged.
The inner table is traversed with real tracked Lua key/value objects.

Every outer value is passed to B573F0: outer numeric, string and Boolean keys are
not filtered. It passes global0108FE90 in ECX, but B573F0 never consumes that
receiver; the C++ interface therefore has no artificial definition-manager
dependency. Returned fields append in Lua iteration order and preserve prior
rows. Count==capacity triggers max(capacity+5,10), followed by pointer publication
at the current data/count and incrementing the current count. There is no rollback
of the newly created field if subsequent array growth fails.

B34680 clamps signed requests to10, allocates request*4 bytes through the
BF55BE/BF681B shared allocator, copies current live count/data, frees the previous
allocation, then publishes new data/capacity. A false CALL_RETURN at B346CC hid
nine publication bytes B346D1..B346D9. The locked repair preserves the callee's
no-return flag, saves the project and refreshes the export.

The outer four-state map DF7990/FuncInfo DF79B0 owns initial gate, table, key and
value; cleanup order after traversal is value, key, table. The records have no
vtable or reference count. The existing descriptor destructor's D0/DC paths
release each leading NativeString, free that same record, clear the entry and
finally free the pointer arrays.

## Verification and limits

Strict MSVC Win32 compilation and both existing CTests pass. No permanent tests
were added. One ignored fixture compares all five complete original routines
against the new implementation using stock Lua5.1.1 ABI adapters and actual
string, Lua-object and descriptor storage. It retains prior captured dependency
bodies without claiming every retained routine executes in this packet.

It covers thirteen Lua values across five object kinds, eleven field shapes,
both integer modes, exceptional/large numeric values, embedded NUL names,
missing ordinals with explicit stack preimages, sparse/mixed keys and ignored
extra ordinals. Parsed28-byte record snapshots normalize only their name pointer;
strings and allocation/release traces also match. A fresh-lookup fixture appends
twelve fields to a preexisting record, reaches count13/capacity15 and verifies
per-entry stack inputs and descriptor cleanup. The field-name header contains
trailing bytes after an embedded NUL to verify actual C-string lookup behavior.
Missing, Boolean and empty field tables preserve the existing array unchanged.

The current installed debugshader, alphablend and dx9_lua.inc files are hashed
and evaluated read-only. Debugshader yields Position/Color vertex fields and a
Color interpolator; alphablend yields Position/Normal/UV vertex fields and a UV
interpolator. All seven original/rebuilt records agree, including masks0 and
semantic indices0, and are cleaned through actual descriptor D0/DC ownership.
The fixture DoFile callback selects the installed loose include; native VFS
override resolution is not claimed.

All ten captured code/data spans match the installed PE. Five complete stored
Ghidra body ranges and nine unwind states are verified. Normal-path comparisons
do not prove original exception ABI, arbitrary callback-driven header mutation,
allocation-failure heap behavior, rendering or gameplay. Stable object addresses,
valid array extents, iterable inner values and native50-slot/five-reference Lua
bounds remain caller contracts. The explicit C++ interfaces are not drop-in ABI
replacements.

## Follow-up

Complete actual descriptor reader B43B00 using the sampler and field readers,
then connect descriptor publication to the actual material/effect lifetime and
shader compilation path. Check live leases first. Lua owner bootstrap, callback
frame-offset maintenance and the VFS DoFile callback remain separate work.
