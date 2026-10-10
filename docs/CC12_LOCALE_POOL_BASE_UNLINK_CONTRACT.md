# CC12 locale pool base unlink contract

`00403970` captures the element's predecessor before stamping base table
`00D7A0C0`, repairs either that predecessor's forward link or the fixed shared
head cell `00E188B4`, then rereads the successor and predecessor for the backward
repair. It makes no call, takes no lock, validates no membership and explicitly
clears no removed-element link. The existing `AllocatorListDomain` provider
already supplies this leaf operation through a caller-supplied head reference.
No duplicate leaf C++ or whole-owner GO follows from this packet.

The full machine record is [the unlink report](../reports/cc12_locale_pool_base_unlink_contract.json).
The accepted [action forwarding](CC12_LOCALE_UNWIND_PRIMARY.md),
[section cleanup](CC12_LOCALE_SECTION_CLEANUP_PRIMARY.md),
[table cleanup](CC12_LOCALE_TABLE_CLEANUP_PRIMARY.md), and
[normal lifecycle](CC12_LOCALE_LIFECYCLE_PRIMARY.md) remain separate contracts.

## Body and current-access order

Prototype and complete typed ownership admitted exactly `00403970..0040399A`:
one 43-byte body/hull, below the approved 128-byte limit. The worker extended its
lease to that range before assembly/byte reads. Fifteen instructions occupy
every byte, with no gap, padding or inferred neighboring boundary. All bytes
match the retained original PE, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The original receiver is ECX, pointing directly to three DWORD element words:
table at `+0`, predecessor at `+4`, successor at `+8`. The exact schedule is:

1. `403970` reads `[ECX+4]` into EAX. TEST captures its zero/nonzero condition.
   `403975` then writes `D7A0C0` to `[ECX]`; this MOV preserves the tested flags.
2. If the captured predecessor is nonnull, read CURRENT `[ECX+8]` into EDX and
   write it to captured predecessor `+8` at `403980`. The shared head is untouched.
3. Otherwise, read CURRENT `[ECX+8]` into EAX and write it directly to
   `E188B4` at `403988`. There is no head read, `head==element` check or traversal.
4. Both paths join at `40398D` and reread CURRENT `[ECX+8]` into EAX.
   If this reread successor is nonnull, read CURRENT `[ECX+4]` into ECX and
   write it to that captured successor's `+4` at `403997`. The final predecessor
   read is not replaced with the first captured predecessor.
5. Use plain RET. There is no local cleanup, state store or recovery path.

The first forward/head repair precedes the common successor reload and backward
repair. Current values and this order remain explicit even though a coherent,
unchanged list normally presents the same links at both reads. The routine does
not reset the removed element's own links, establish idempotence, traverse or
free neighbors, touch section/table storage, or use Enter/Leave/Delete. It supplies
no serialization. Aliases and invalid membership are not normalized into a safer
container operation.

The complete normal graph has 16 internal edges. All branch targets lie inside
the admitted hull. The existing name `BSP_AllocatorListElement_UnlinkBase` and
`undefined ...(void)` prototype are preserved; all ten captured instruction-site
records retain override `NONE`, with no calls. The stored void prototype omits
the ECX element input. No body/listing/flow/signature/no-return repair is made.

## Null branches, registers and failed prefixes

Null predecessor and null successor are legitimate branch inputs; a null
receiver is not guarded. Its first attempted access is DWORD address `4`.
An element with null predecessor replaces `E188B4` unconditionally, so the caller
must supply the actual linked element rather than any zeroed or layout-compatible
object. No live head contents or current list membership was inspected here.

On normal return, EAX contains the final successor read (possibly zero).
ECX remains the incoming receiver if that successor is null; otherwise it holds
the final predecessor read. EDX holds the first successor read on the nonnull
predecessor path and is untouched on the head path. EBX, ESI, EDI and EBP are
untouched, and ESP is unchanged until RET consumes the existing return address.
There are no explicit stack arguments. Final defined arithmetic flags reflect
the last successor TEST: CF/OF clear, ZF/SF/PF from the successor bits, AF undefined.
These are observed machine outputs, not a recovered meaningful API return value.

Ordering also bounds failure prefixes. A fault during the first predecessor
read precedes the table stamp. A fault in the first neighbor/head repair can
follow that stamp; a fault in the final backward repair can follow both earlier
stores. There is no local rollback or EH frame. Fault delivery, recovery and
double-failure behavior were not executed or established.

Retained constructor bytes prepend the same incoming pool/list base to the same
`E188B4`: predecessor zero, next from the shared head, former head predecessor
repair when nonnull, then shared-head publication. State 0 is armed afterward.
The prior `C5EEA0` action reloads its saved receiver and tail-jumps here without
an offset. Under valid invocation, frame setup, membership and serialization,
this is the base unlink for that constructed element, including an ordinary
failed-construction prefix armed at state 0. Actual FH3 invocation and dispatcher
`EBP=S=R+0Ch` remain unproved; the leaf does not establish them.

## Existing Source and actual-domain conditions

Five bounded excerpts from the frozen `src/allocator_list.cpp` and
`include/bsp/allocator_list.hpp` are retained with Git blob and line-range proof.
`AllocatorListDomain::unlink_base_element_00403970` captures predecessor before
stamping the base table, performs the predecessor/head repair, then uses current
successor/previous fields for the backward repair. Its ordinary coherent-list
schedule is reusable. These ordinary C++ pointer accesses are not a fresh proof
of compiled load counts, hardware fault timing, arbitrary aliases or concurrent
mutation behavior.

The element header explicitly describes native data words rather than a C++
vptr; Win32 layout checks establish offsets 4 and 8 and size 12. The domain's
explicit constructor accepts `AllocatorListElement*&` and stores that reference.
Both prepend and unlink use the same referenced head. The separate `virtual0_`
binding map is documented as not being the list head and is untouched by unlink.
The binding interface calls for a concrete recovered profile, owner context and
invoke service; unlink neither invents these nor unregisters them.

For locale composition, the supplied domain must refer to the actual head storage
representing the shared native `E188B4`, and the element must be the same actual
pool/list base published through that domain. A parameter name or pointer field
does not authenticate this identity. This packet verifies the existing provider,
not a live locale-domain instance, canonical owner construction, binding
retirement or quiescence. It creates no head copy, domain, context or default
service. The Source method's receiver is a host domain plus an explicit element
reference; it is not the original ECX-element entry ABI.

## Proof and next finite scope

Capture HEAD is `fbd0f5e1cd5529e3164ea2e22a03470fe371a4cd`. The 4,217 frozen
Source files exactly match actual Git blobs and the prior section-cleanup Source
epoch. Retained commit/tree objects prove all 4,210 `include/`/`src/` paths plus
seven explicit Source inputs. Twenty dependency and four tool/config blobs are
retained. Historical artifacts receive no new credit.

Six read-only query outputs verify the existing BSP project/program. Entry plus
nine site records match before/after at capture epoch 90, closed before later
Root annotation work. Fresh Native bytes total 43; the prior 1,522 reviewed body
and data bytes keep their older provenance. No new PE data, head-cell contents,
callee, OS import, free thunk, handler or EH data was expanded.

`local/cc12_locale_pool_base_unlink_contract/evidence.zip` has 4,273 payload
files / 103,092,763 uncompressed bytes, ZIP size 72,757,861 bytes, SHA-256
`4e7a26caa16d436e0dcd443026702cc45b8da6facdb32788eed37be6d32456da`.
Original and independent physical copies pass guarded ZIP-only replay of nested
proofs, manifests, actual Git objects/closed paths, PE bytes, full decoding/CFG,
typed metadata equality and frozen Source excerpts. External reads, directory
access, process/network calls and dynamic loading are denied after guard setup.
No C++, CMake, ledger, GPR, build, test, probe, runtime or OS change is added.

All three declared cleanup actions now have bounded target-effect contracts,
with the earlier normal-return qualifications. Existing reusable leaf Source is
present. The next proposed finite owner-provenance gate is metadata-only xrefs
and callers for canonical `E1768C` and constructor `436230`, identifying exact
initialization/publication sites before any new body admission. Root can then
connect those facts to the supplied Source domain, storage and services. Actual
FH3 frame setup remains a separate ABI question and should reuse shared runtime
evidence when available. No duplicate cleanup leaf or whole-owner activation is
presumed by this handoff.
