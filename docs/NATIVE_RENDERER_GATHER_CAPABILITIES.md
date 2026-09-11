# Raw renderer capability gathering

This packet translates the complete `00B2C8E0..00B2D8DD` body (4,094 bytes,
885 decoded instructions). It operates on actual renderer storage and the full
raw array/string providers. The existing typed capability and settings APIs
remain separate. This source has an explicit host scratch/context ABI and two
fixed valid-buffer CRT adaptations; it is not a binary replacement.

## Raw storage and source interface

Original ECX is the renderer, RET has no stack parameters, and there is no
semantic return value. The source takes ECX renderer and EDX a fixed context.
The minimum reached renderer extent is `1D8A`; that is an access bound, not a
claim to recover its whole owner. Current `+1990` is the actual IDirect3D9
factory, with the concrete methods below. No device/window is created or
modified by this packet's verification.

The caller provides four-byte-aligned, initialized readable/writable raw
scratch of at least `770h` bytes. Offsets correspond to ESP after the original
four nonvolatile pushes. The source writes only the offsets the body writes;
it does not construct zero-filled typed caps/identifier outputs. If native
reads extend beyond the minimum arena, the caller supplies that additional
valid extent too. Context and its binding identities remain stable and do not
alias writable raw operands. This replaces the private native stack/EH frame;
it does not expose saved registers or permit aliases into private C++ frames.

| Scratch offset | Actual use |
|---|---|
| `10` | Current format index, reloaded after queries/allocation |
| `14,18` | Actual eight-byte local string header: length, data |
| `20..23` | Four individually zeroed flags; only first three later updated |
| `24..27` | Only byte24 updated; all four bytes copied as record payload |
| `28` | Saved inner-header pointer, reloaded across record reserve |
| `2C..1F3` | 57 eight-byte format/metadata rows, 285 ordered native stores |
| `1F4..323` | Raw D3DCAPS9 output, including bytes left by failed calls |
| `324..76F` | Raw D3DADAPTER_IDENTIFIER9 output; Description begins at524 |

Description scanning is an unchecked byte walk to NUL, not a 512-byte bounded
field operation. Both HRESULT output buffers remain whatever the real COM
provider wrote. Each format query uses adapter0, HAL1, adapter format16h;
exact HRESULT zero alone means supported. Current factory, table and reached
method are read again at every query. Raw aliases and reentry must continue to
satisfy all reached object, buffer and allocator ownership contracts.

## Complete original-to-source schedule

The source comments name original instruction locations. These ordered regions
cover the entire body; compiler register allocation, push encodings and private
C++ exception machinery are new. Source helpers factor repeated schedules,
not external callback interfaces or replacement container implementations.

| Original half-open range | Source operation and preserved observations |
|---|---|
| `B2C8E0..B2C8FF` | Private native FS/stack/nonvolatile prologue becomes the new fixed context and C++ cleanup state. No binary/SEH identity claim. |
| `B2C8FF..B2C91C` | Capture current1990/table, call +38 GetDeviceCaps(0,1,scratch1F4), ignore result. |
| `B2C91C..B2C99A` | Ordered Caps2/width/height/VS/PS/volume raw loads and publications. Capture factory before PS/volume stores; then current captured factory's table+28 ATOC query. |
| `B2C99A..B2C9BD` | Zero result sets1B55, failure leaves it; recapture1990/table, call+14 GetAdapterIdentifier(0,0,scratch324), ignore result. |
| `B2C9BD..B2C9E7` | Zero only local header14/18; byte-scan Description; full41DD40(length,true) with actual current pool storage. Cleanup not armed. |
| `B2C9E7..B2CA0A` | Capture data once, if nonzero capture current length+1 then source/destination; fixed host memmove, including overlap. Zero length still captures operands before service omission. Compare captured data for later search gate. |
| `B2CA0A..B2CA86` | Arm state0; search captured data for8800,8600,8200,ATI in order; retain found-pointer DWORD subtraction versusFFFFFFFF; publish1D89. |
| `B2CA86..B2CAE9` | Reload current PS1B40; publish1B48; PS<=104 writes1B2C then1B30; PS200/300 writes1B30 then1B2C; other values retain those fields. |
| `B2CAE9..B2CB55` | Ordered MaxVSConst, clip count, textures, anisotropy loads; publish1B4C before DevCaps read; then1B28/24/51; hardware/VS gate sets1B74 and may override1B44=101. |
| `B2CB55..B2CC53` | Five current-header declaration appends0..4. Full B236B0 only if count==capacity; signed doubled capacity clamp1; count before base; computed-null store skipped; current count incremented anyway. |
| `B2CC53..B2CE72` | Per-append current scratch BYTE DeclTypes tests bits0..7 then DWORD bit8; appends5,8,9,10,11,12,13,14,15. No earlier mask snapshot. |
| `B2CE72..B2CEBA` | Reload current PS; optional actual INST query, temporary1B50 publication, then read DevCaps2 byte and unconditionally clear1B50 before1B52. |
| `B2CEBA..B2D716` | `initialize_format_table`: every original ordered DWORD/BYTE MOV, all285 writes, full456-byte coverage exactly once. No broad memcpy or aggregate store. |
| `B2D716..B2D730` | Resource index3, initialize scratch counter0. Native <=3 resource loop executes this one resource type. Alignment NOPs have no source memory operation. |
| `B2D730..B2D756` | Load current counter and capture format BEFORE current factory query usage0. Nonzero result skips all support/record work. |
| `B2D756..B2D7E7` | Reload counter for metadata0 comparison BEFORE four flag zeroes; optional usage1; reload counter for metadata1/usage2, then metadata2/usage200. Captured format remains unchanged. |
| `B2D7E7..B2D821` | Always query usage80001 after accepted format; compare current outer count BEFORE byte24 result store; full B2AE20 request4 if needed. |
| `B2D821..B2D856` | Reload current outer base, inner capacity then count; publish saved inner pointer28 after comparison; full B22B30 on equality, reloading28 both before and after call. |
| `B2D856..B2D879` | Current inner count then base; computed-null skips source loads/stores; otherwise flags DWORD load BEFORE format store, then flag store, payload DWORD load/store; increment current inner count. |
| `B2D879..B2D899` | Reload current scratch counter, wrapping add1, unsigned comparison39, then publish increment and loop. Native resource increment/signed termination completes. |
| `B2D899..B2D8C9` | Test current string pointer before disarm; normal cleanup captures current length BEFORE current data, then current419CC0/BD1510 with length+1. Header remains stale/published. |
| `B2D8C9..B2D8DE` | Native FS/stack restore/RET replaced by host function epilogue; no original incidental-register claim. |

All nine static COM sites map to concrete IDirect3D9 methods: one caps, one
identifier and seven CheckDeviceFormat sites (ATOC, INST, five loop usages).
`check_format` reads the current object/table each time. COM arguments have
their original values; native physical outgoing stack slots/register spills
are not part of the new source ABI.

## Provider closure and CRT boundary

| Original dependency | Actual source binding |
|---|---|
| `B22B30[130]` | Full raw 12-byte-record reserve, singleton_lifetime allocation/free |
| `B236B0[95]` | Full raw DWORD reserve, same shared heap |
| `B2AE20[116]` | Full raw outer resize, complete nested resize/reserve/copy providers |
| `41DD40[164]` | Full actual eight-byte-header resize through concrete ActualNativeStringPoolStorage |
| `419CC0[192]` | Current actual pool singleton, actual01090AA8 and canonical lifetime |
| `BD1510[95]` | Actual current pool return with actual mutable01090AA4 gate |
| `BF7680[869]` | Fixed host memmove valid-buffer adaptation for the reached gather copy |
| `BF9440[134]` | Fixed host strstr valid-NUL-buffer adaptation, exact pinned readonly needles |

The shared lifetime must carry the application's actual pool owner binding;
no temporary pool/domain or alternate string allocation policy is introduced.
Every concrete storage allocation/release calls the current pool getter, even
large buffers and disabled small returns. Full41DD40 retains its own current
header reads, equal-length early return, nullable allocation/unguarded final
terminator and existing zero-copy/fresh-versus-live memcpy source limits.

BF7680 really handles overlap, reads mutable0109EEA4 for eligible large forward
copies and may tail-jump to C0C82B. The fixed host memmove deliberately does not
reconstruct that vector provider, dispatch word, flags/SIMD state, access order,
page/fault behavior or memory races. Its supported contract is the resulting
bytes of valid buffers. No fake dispatch argument or fixed global is supplied.
The zero-byte adaptation still preserves original length/source/destination
captures. BF9440's full body includes an internal BF86F6 strchr tail for a
one-character needle; these four pinned needles have lengths3/4. Host strstr
is case-sensitive/first-match within valid terminated buffers. Neither that
internal tail nor optimized host/native access or fault equivalence is claimed.

## Cleanup and exceptional boundaries

Fresh handler CBD56B loads FuncInfo DF5DF8; map DF5DF0 is {-1,CBD560}.
CBD560 addresses the original local header and jumps to complete41DD20.
State0 arms atB2CA0A after the entire string resize/copy. Failures before arm
gain no new string rollback. Later C++ failures run full41DD20, which captures
current data before current length, and rethrow. Earlier renderer/array writes
remain published. Nested reserve retains its existing actual no-op cleanup
specialization, including lack of rollback for fresh partial allocations.

Normal exit tests data before disarm, then captures length before data; it calls
current419CC0 directly and thenBD1510. A failure there propagates without a
second string cleanup. Exceptional cleanup uses existing noexcept string
release, requiring a returning pool getter; lazy recreation failure terminates
under that pre-existing source interface. Original native FS/FuncInfo/C++ EH
ABI, hardware faults and aliases into either implementation's private frames
are outside this source contract.

## Validation

The primary reviewed the complete CPP/HPP, assembly schedule and corrected
whole-table verifier before compilation. The strict MSVC Win32 build passed
both existing CTests and eight fresh native seeds. The source/header remained
unchanged after review. All build source/header inputs were hash-pinned before
compilation and rechecked before freezing the actual archive and used sources.

Nineteen fresh guarded spans total6,030 bytes and match the installed PE.
The complete owned body decodes885 instructions. The table verifier extracts
the entire source helper body and requires exact list equality with285 decoded
native stores: order, width, offset, value and absence of extra statements.
It verifies the dominating XOR EBX,EBX and no intervening body write, with the
explicit concrete x86 nonvolatile-callee ABI assumption. It covers57 rows,
456 bytes; it is source mapping, not compiled instruction identity.

The unexecuted address-only probe links the frozen actual library. Static
verification checks125 mapped COFF sections and324 relocations, all27 owned
sections and49 relocations, exact membership of seven archive objects and21
source/header files. Actual CL command records confirm /W4 /WX /fp:strict for
every linked source member. All341 original bytes of the three immediate raw
array providers match linked code except their seven explicitly bound CALL
operands. Whole original EH handler/map/action bytes are pinned and checked;
generic __CxxFrameHandler3 atBF6B43 remains a CRT boundary.

The linked PE records52 named static imports, including memmove and strstr.
It was never executed: no loaded-module/provider postimage, raw gather/COM,
device/display/gamma/game, fault or original-runtime result is claimed. The
existing tests validate their existing math scope, not gather behavior. Frozen
evidence and exact static replay instructions are under ignored
`local/renderer_gather_capabilities/`, with `sealed.json` the immutable manifest.


## Primary integration

All 1 complete entries are registered against the same strict main Win32 library `e59c0a7d198e8059d3eb65064cd1857b72e48f91e858906a448a9b38d4414ccf`; two existing CTests and eight original seed spans pass. Complete4094-byte/885-instruction source schedule with actual renderer extent1D8A, stable fixed EDX context and initialized aligned770h caller scratch. Full285 ordered table stores/57rows456B verified as exact helper body. Actual current COM factory/method reads and raw outputs/HRESULT schedule,12 actual array/string/pool/lifetime bindings,44-byte nativeEH map/FuncInfo chain reviewed. Fixed host memmove/strstr valid-buffer adaptations and new C++ EH/scratch ABI explicit. All seven actual archive members, every mapped COFF byte and relocation, whole owned COFF sections and exact341 native primitive/resize bytes checked. Incremental main verifier only drops unnecessary repeated nested-source compile-line demand; strict actual CL commands/full current member evidence retained. Address-only probe never executed; no COM/device/gather/nativeEH/originalcallerABI/game validation. Reviewed names and evidence comments are saved with prior comments retained; all affected exports are forcibly refreshed. Immutable evidence: `local/renderer_gather_capabilities_primary/`.
