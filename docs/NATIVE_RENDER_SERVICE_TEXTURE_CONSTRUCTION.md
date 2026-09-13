# Native render-service texture helper construction

Addresses: `00B52550..00B5283F` (752 bytes, 242 instructions). Descriptive names
are hypotheses. This packet adds one complete normal body and reuses existing
providers; it adds no caller, destructor, dimension getter, vector, ledger or
Ghidra mutation.

| Routine | Coverage | Original ABI | Source |
| --- | --- | --- | --- |
| B52550 | complete normal body | ECX actual CCh allocation; zero stack arguments; EAX same owner; RET | `construct_native_render_service_textures_00b52550` |

The new C++ API takes borrowed service context and a caller-retained invocation.
It is not a binary replacement. B14A10 is the sole caller: B14EC1 pushes CCh,
B14EC6 calls BF681B, B14ECB removes one DWORD. B14ED2/B14ED9 test the returned
allocation against EBX0; B14EDB moves it to ECX before B14EDD. B14EF0 publishes
EAX at service+34. The entire 302-instruction caller was read: EBX is zeroed at
B14A57 and has no later write before this site. No `sizeof` host owner substitutes
for CCh and this constructor does not allocate its owner.

The owner changes CEB130 -> refcount1 -> D62074, then zeroes pointer/count/capacity
triples at +24, +7C, +8C and +A4 in native order. It leaves every other preimage
byte intact until the reached operation explicitly writes it. The four names are
raw 8-byte string headers at offsets 0/C/14/1C of one retained 24h region. Offset8
holds the saved owner. B52655 and B526F1 overwrite the FIRST released name length
with saved height, after capturing the next width entry and before invoking it.

| Load site | Current literal | Resize length | Published texture | Subsequent result |
| --- | --- | --- | --- | --- |
| B52616 | D620A0 `kosz_01.tga` | 11 | +18 | unsigned width/height -> +1C; zero +30 |
| B526B2 | D62094 `szor_01.tga` | 11 | +3C | unsigned width/height -> +40; zero +68 |
| B5274E | D62088 `csikok.tga` | 10 | +70 | unsigned width>>1 -> +74; zero +88 |
| B527D4 | D6207C `splotch.tga` | 11 | +98 | unsigned width/height -> +9C; zero +B0 |

Resize passes preserve1 and RET8. Each copy captures the current data pointer,
then current length+1, and copies from the supplied current literal bytes.
BF7680 is named `_memcpy`, but its body handles backward overlap at
BF7694..BF769A -> BF7844; the source therefore uses `memmove`.

Every load freshly captures F8D394, that actual renderer's current profile, and
its current +64 entry. The supported D5F0A8/B319B0 route calls the existing full
wrapper and persistent B30B40 cache. Four separate acquired objects preserve
all wrapper names and any retained resolver/load identities. Context setup calls
the existing `bind_native_texture_vfs_name_resolution`: it installs the concrete
retained BDF4C0 operation and verifies shared string/VFS publication domains.
Raw string pool/return-gate/manager cells must be the SAME cells consumed by
`cache.strings`; this remains an integration precondition because that bridge's
private cell references are not introspectable here. No private pool, texture
identity, resolver result, global snapshot or successful callback is invented.

Texture publication precedes temporary release. Release captures the current
data pointer BEFORE state4, captures current length+1 BEFORE the fresh419CC0
getter, and passes its result to BD1510 with the current01090AA4 gate. Only then
is the current owner texture field reloaded. A paired dimension read keeps the
same receiver while rereading its profile/slot between calls. D61948/+48 is
B3CE70, reading +34; +4C is B3CE80, reading +38. B3F930's B3F9EA/B3F9F9 stores
produce these saved values, separately from COM dimensions at +28/+2C. Either
known dimension target is admitted when selected by the current requested slot.
Unsupported profiles/targets throw an explicit source boundary. There is no
null texture fallback or default dimension. B52661/B526FD/B5281D are unsigned
integer DIV with zero EDX; B52782 is SHR1. They are not x87 expressions. A narrow
assembly helper retains real unsigned DIV and its hardware fault behavior.

FH3 handler CBFFDA..CBFFE3 selects FuncInfo DF8AE0 (magic19930522, maxstate9),
with unwind map DF8B04..DF8B4B. State0 destroys BD30F0 base; 1/2/3/4 destroy
vectors at +24/+7C/+8C/+A4; 5..8 first destroy their corresponding raw name,
then continue from4. Normal code arms0 after derived profile,4 after all triples,
then5..8 immediately before each load. It disarms each name before normal pool
return and never releases a published texture during parent cleanup. Source
cleanup preserves this partial schedule for supported C++ failures. Failed
child frames must stay alive until explicitly resolved under their existing
contracts; discarding/resetting them would lose native acquired identities.

All 39 numeric transfer rows, argument/cleanup contracts, original containing
functions, source/provider hashes and corrections are in the companion report.
The 11 register calls are resolved by captured profile bytes and producer
evidence; the mechanical verifier labels them indirect rather than proving them.

Verification: strict MSVC Win32 /W4 /WX /O2 /EHsc /MD /fp:strict passed after the
overlap correction. Worktree baseline f5ce17eb passed its full build, both
existing CTests and all eight seed spans. That baseline excludes this unregistered
parent and predates vector CMake registration. The initial external fixture
therefore compiles only this parent plus its probe against root's exact tested
b7683394 three libraries. Later report/source-only root commits are not the
library build provenance. No new repository tests were added.

One focused original/source pair passed full204-byte owner equality including
patterned preimages, four actual native alias-cache hits/+4 references per path,
native local spacing, saved width8000000F/height7 against real COM8x4 dimensions,
and real canonical texture/COM/string-pool/raw-manager teardown. It uses actual
pooled B3F930 construction, record/alias providers, and private Microsoft XLive
pretranslation with the actual null-online BECB20 guard. Original752-byte code
is relocated by16 external CALL operands and4 publication-address operands;
three copied profile entries temporarily select ABI adapters for native register
calls and are restored before source numeric dispatch. Rebuilt callee bodies
are shared; their original machine code is not independently replayed.

Capture: `C:/Users/sqz269/bsp-bc-texture-construction/worker_capture_v2.zip` contains
the exact source/header, b7683394 source/header input archive, three libraries,
probe/includes/recipes, native bytes/listings, runtime DLLs, hashes and logs.
Default `run.ps1 -Repo <integrated root>` compiles external probe.cpp only against
that root's three current libraries. `-WorkerSource` is explicit initial-stage
support, not evidence of an integrated parent. Root owns CMake/ledger integration
and the subsequent current-library-only replay.

Limits: this fixture covers hot-cache returns, not cold VFS/D3DX loading,
overlapping literal buffers, failure/unwind execution or arbitrary profiles.
Original FH3/SEH identity, second-exception search ordering, hardware-fault
unwinding, private context/frame aliases, allocation failure, unrestricted ABI,
concurrency and gameplay remain unproved. Current raw storage must be valid
at each native access. No executable path or frame behavior is claimed here.

## BC integration checkpoint

The integrator reviewed the complete native body and actual producer/provider
evidence, saved its original analysis signature and full stored range in the
existing BSP project, and registered the source where needed. Exact combined
validation follows separately from worker checks. No original binary entry,
unrestricted FH3/SEH, whole owner lifetime or gameplay claim follows.
