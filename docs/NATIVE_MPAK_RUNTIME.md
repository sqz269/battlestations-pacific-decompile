# Actual MPAK runtime composition

Addresses: `00BEF580`, `00BF4F40`, `00BEF750`. Other named entrypoints below are
existing reconstructed dependencies, not additional body counts for this packet.
Descriptive names remain hypotheses, not recovered symbols.

`NativeMpakRuntime` connects the existing AY factory/provider/parser and the AZ
open, entry, enumeration, device-selection and raw-inflater modules. It contains
source contexts and adapters only. Actual provider44h, file24h, directory14h,
inflater34h, stream14h and memory-backing10h owners remain in their established
storage. The existing canonical pool, publication cells, memory counters and
lifetime domain are borrowed; no alternate manager, cache or archive is created.

## Bound surfaces

| Original captured target | Source route |
| --- | --- |
| BDB078 -> BB83A0 factory | `NativeVfsRuntimeBindings::factory_create` -> actual MPAK create context |
| BB8240 / BB82C8 -> manager+4 BDF310 | Runtime provider dispatch -> existing actual VFS open route |
| Provider D641F8+08 BB5BB0 | VFS provider open -> actual MPAK open context |
| BB5BB0 -> BDD850 | Runtime open dispatch -> actual device visitor/traversal |
| BB5BB0 -> BB5080 | Runtime open dispatch -> actual entry materialization |
| BB5080 source+20 | Captured position target -> actual memory/physical/adopted/raw position |
| BDBC70 provider+24 BF0FB0 | Actual generic logical-name resolver through current contains route |
| BDBC70 provider+24 BB68F0 | Actual MPAK member-directory selector with explicit search library |
| D641F8+14 BB5F40 | Explicit runtime enumeration entry -> actual original-prefix enumeration |
| D641F8+0C BB79E0, +20 BB7A00, +28 BB79F0 | Explicit runtime entries -> genuine false,14h clear and no-op leaves |
| D641F8+10 BB4B20, +18 BB40C0, +1C BB5540 | Existing lookup module uses the actual contains/default/name-copy bodies |
| D641F8+0 BD30E0 -> current+4 BB7B80 | VFS zero-reference route -> actual provider destruction/deallocation |

All captured-entry methods consume the entry supplied by the original call site.
They do not recapture a provider or stream table before dispatch. BD30E0 is the
intentional exception: its established native body explicitly reloads current
slot4 after receiving slot0. The runtime preserves that reload.

The source context holds `NativeMpakCreateContext`, `NativeMpakProviderContext`
and `NativeMpakOpenContext` references. The runtime graph supplies the directory,
provider, cache, provider-operations, create, entry, open and device contexts in
dependency order. It installs the MPAK binding on VFS, the device context on
lookup, and the VFS stream dispatcher on the supplied conversion context.
Disposal restores the preceding pointers only if the current pointers still
identify those installed by this graph. Nested bindings use normal stack order;
the enclosing graph, VFS and borrowed inputs must outlive their borrowers. No
native publication, cache, stream or reference count is changed by binding or
unbinding alone.

Arbitrary out-of-order destruction is unsupported. Two live graphs borrowing
the same VFS install the same `conversion.adopted_substreams` pointer, so that
pointer equality cannot identify which graph installed it last. The predecessor
graph must outlive its successor, which must be disposed first. The conditional
pointer checks preserve distinct replacements; they are not an ownership-token
scheme for graph disposal in arbitrary order.

## Numeric raw inflater and memory conversion

The live original D64400 profile was read through the verified BSP CLI:

| Slot | Native target | Bound behavior |
| --- | --- | --- |
| +00 | BD30E0 | Reload current+4 and delete with flag1 |
| +04 | BBC3E0 | Actual BBC320 destruction, then optional owner free |
| +0C | BB8B80 | Two current file-type IDs, shared with adopted substreams |
| +18 | BBBDC0 | Return raw byte at owner+9 |
| +1C | BBC060 | Actual raw seek, retaining original incidental EAX |
| +20 | BBBE50 | Actual low position word with zero high word |
| +24 | BBC140 | Actual inflate/read and optional count output |
| +28 | BBC1C0 | Genuine RET0Ch, including untouched optional count |
| +2C | BE41A0 | Existing low-size wrapper over current length dispatch |
| +30 | BBBDD0 | Actual decoded length, zero high word |

`convert_native_stored_stream_00bef750` now recognizes D64400 at each current
type/seek/length/read dispatch. Type query uses existing BB8B80 and the borrowed
`actual_file_type_ids_0109db58`. Seek/read call actual raw methods with the
existing `adopted_substreams` dispatcher, which now also supports raw sources.
The native memory-type test is preserved; there is no hardcoded false answer.
The current slot is checked against the known numeric target at every stage.
Missing dispatch/ID bindings or changed targets are explicit source errors.
D64400 never falls through to calling an original numeric code address.

The body still queries type first; non-memory sources seek to zero, obtain only
the low length DWORD, allocate an actual backing, read once with null count
output, wrap it and release the temporary backing. There is no short-read trim,
source release or cursor restoration. BB5080 releases its temporary inflater
after conversion; numeric BD30E0/BBC3E0 now reaches its actual destructor and
that destructor's captured-source reference release through the same dispatcher.

## Position leaves

| Address | Inclusive range | Original ABI | Coverage |
| --- | --- | --- | --- |
| BEF580 | BEF580..BEF58A,11 bytes | ECX actual memory stream; EDX:EAX; RET | complete actual storage; supplements existing typed projection |
| BF4F40 | BF4F40..BF4F46,7 bytes | ECX actual physical stream; EDX:EAX; RET | complete actual storage |
| BEF750 | BEF750..BEF83F,240 bytes | ECX actual source; EAX memory wrapper; RET | existing complete body, numeric profile domain extended |

BEF580 loads backing+8, then current cursor+10, subtracts backing data+8 with
DWORD wrap, and sign-extends through CDQ. It adds no null/empty guard. BF4F40
loads current position low+10 then high+14 unchanged. The primary integrator
defined the previously missing BF4F40 body at this exact seven-byte range under
its write lock; this worker made no Ghidra mutations. The new C++ signatures
preserve the 64-bit result bits and are not original binary entrypoints.

## Verification and remaining boundaries

Strict Win32 Release build and both existing CTests passed. The generated
fixture decoded32 expected bytes, matched both position leaves against their
original instructions, preserved raw-write count output and restored nested
bindings. Final memory object count and requested-byte count were both zero.
Factory/open checks exercise the native decline branches; full parser/factory
construction is not claimed by this composition fixture.

The report records the exact live profile/body bytes and call-site mappings,
the strict Win32 build/CTest results and the focused generated fixture artifacts.
The fixture uses actual numeric profiles, a generated raw-DEFLATE payload,
actual raw constructor/read/destructor, BEF750, retained memory owners and VFS
dispatch. It checks decoded bytes, original-byte position leaves, raw no-op
write output preservation, nested source-context restoration and memory counters
returning to zero. Native/source reconstruction fixtures from enumeration,
entry, raw-inflater and device packets remain separate; prior enumeration
inputs and proofs were not changed.

Original container specializations BB4140, BB4F40/5EFBA0, and the AY parser's
file/directory/offset insertion/destruction operations remain explicit external
library contracts. No successful fallback implementation is supplied. Physical
and memory write leaves remain outside the VFS numeric write dispatcher; the
adopted stream's existing write forwarding may therefore reach that boundary.
Other unknown profiles/entries retain deliberate source errors, while BEF750's
pre-existing externally callable-table domain is unchanged. The actual manager
failure field+90 remains a callable original-ABI service contract.

The graph does not populate manager mounts, register a startup factory list,
load an installed archive or establish gameplay reachability. A successful
build/fixture does not prove native FH3/SEH, concurrent mutation, arbitrary
aliases into native stack spills or simultaneous cleanup failures. No game
installation changes, re-import, direct original numeric-code calls or Ghidra
mutations were performed by this worker.
