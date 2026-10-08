# Actual squadron promotion and selected BE receipt (CC11)

`native_plane_squadron_promote_007ed610` reconstructs the complete ordinary
`007ED610` body on borrowed actual storage. It rotates the actual member cells,
publishes the selected first plane, then directly invokes the complete raw
`007ED260` provider on the same root. The bounded selected-BE receipt supplies a
connected Source consumer. This is compiled and fixture checked Source with a
new ABI; executable class dispatch, deferred delivery and game binding remain
external.

## Native evidence and Source order

| Evidence | End exclusive | Bytes | SHA256 |
| --- | --- | ---: | --- |
| Whole promotion `007ED610` | `007ED64F` | 63 | `5874214b311ea4a5142d909488157d90077df23048afc8304ba8d26b0dfd367a` |
| Selected BE arm `007F0077` | `007F008C` | 21 | `78ab1b274cd03250d19915b8af88e5233cdc9eeb470a3b6e02713515fc8c1522` |
| Existing raw reindex `007ED260` | `007ED375` | 277 | `34c211366b9eae72404ec5d797ff97716b371b4a96e986c57c5c2ea5d1f4887d` |

All three full reviewed spans match original PE and live Ghidra bytes, queried
through the project/program-verifying CLI for `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. Promotion has 20 instructions: native ECX is the
actual squadron, the stacked DWORD is a signed index, and `007ED64C` returns
with `RET4`. Its descriptive name is a hypothesis; the Source void interface
does not promise native EAX/flags or original register ABI.

`index <= 0` returns before any count observation (`007ED614/616`). The next
signed comparison rejects `count <= index` (`007ED618/61E`). Otherwise capture
the selected actual member once (`007ED621`), read each preceding member and
shift backward (`007ED630/633`), publish the captured member at first slot
(`007ED63F`), then call whole reindex (`007ED645`). There is no added capacity,
duplicate or null-member guard, allocation/free, rotation callback or index
array projection.

The sole live direct caller is `007F007B` in `007F0030`. Its already-selected BE
arm loads message `+1C`, invokes promotion and returns AL=1 even for rejected
indices. `D087C0+164` at `D08924` contains `007F0030`, **not** promotion;
promotion has no table xref. These profile words are DATA evidence and are not
installed as a callable Source class table. The new receipt borrows the live
`NativeSessionMessageBE` payload using its existing `+1C` static assertion. It
does not implement other dispatcher arms, type selection or delivery.

## Actual storage and caller frame

The pure checked view requires a stable nonnull actual squadron with at least
`3E4h` backing, alignment, the genuinely live signed count at `+3CC` and five
pointer cells at `+3D0`. It checks addresses and returns borrowed references;
it observes no represented field values and supplies no callbacks, native
calls, allocation, defaults or translated caches on valid placement. Invalid
Source placement throws `logic_error`; native invalid-memory behavior remains
outside admission. A forged view is not proof of coherent storage or lifetime.

The ordinary domain keeps count in `0..5`, occupied actual planes alive with
writable aligned `+9D0/+9D8` cells within `9DCh` backing, and their old formation
indices in `0..4`. Storage and structure remain coherent and stable through
the operation. `PlaneSquadronEntity` is a compact semantic Source projection;
its address cannot be passed to raw reindex. Existing semantic APIs stay
unchanged. No copy or synchronization between semantic and actual arrays is
introduced.

Raw reindex retains physical signed stack scans. At count five, candidate five
reads the caller return word; candidate six reads the following caller word,
and negative words can advance the scan further. The caller must supply the
existing raw readable-frame/nonnegative-stop domain. The count bound alone
does not establish it; no candidate guard or substitute frame is added.

The standalone Source operation compiles to a tail JMP after first-slot
publication: raw reindex sees the outer Source return/argument words. The
selected-BE receipt inlines rotation and makes a direct CALL with its own
saved Source registers. Neither frame is the original promotion frame with
saved incoming EDI, nor the original BE route that carries a message in EDI.
Equal general five-member results, original class ABI and actual runtime
frame/lifetime admission are not claimed.

## Focused validation

One ignored count-four fixture uses genuine live typed count/member and plane
`+9D0/+9D4/+9D8` cells, static offsets and full backing/guards. In each fresh
Source/original comparison setup the existing sorted producer appends the
fourth plane once. Index-two promotion then updates actual fields; the same
first cell and ordinal cells feed the existing first-plane and leader readers.
Rejected indices zero and equal-count are checked in this same fixture family.

The complete original 63-byte promotion executes with only its natural CALL
operand relocated directly to whole raw Source reindex: four designated bytes,
59 unchanged bytes, and no extra bridge frame. The fixture ABI wrapper is new;
no synthetic table is used. The live Source BE payload is not an invocation of
its constructor, executable profile or full dispatcher. These controlled
Source shapes/setup do not prove native object construction or game lifetimes.

Final strict MSVC Win32 compilation covers six fresh TUs: promotion, raw
reindex, sorted insertion, first-plane reader, leader provider and probe. It
uses `/W4 /WX /fp:strict`, a manifested PE32 probe and frozen current core/Lua/
zlib inputs. The sealed run passes 61 checks. The initial identical case also
passed 61, but its pre-pin helper failed Windows command quoting; the final
same-case rebuild follows corrected complete input/toolchain pins. Counts are
not aggregated and no old fixture family is replayed.

The manifest records 47 pre/post-stable Source/recipe/support/PE inputs, three
actual Hostx86 compiler tools, 34 actual Source compiler includes (248 observed
include paths), 16 artifacts and whole COFF bodies. Promotion and receipt are
68 bytes each; the 277-byte raw body equals the PE with zero relocations. COFF
shows the initial guard before count, capture/backward stores, first-slot store
before the direct raw route, and receipt AL=1. The standalone primitive is
checked through whole COFF; the runtime Source receipt inlines it under `/O2`.

Artifacts are `local/cc11_plane_squadron_promotion_manifest.json`,
`..._native.json`, `..._build.ps1`, `..._coff.txt`, `..._output.txt` and
`..._probe.exe`. The JSON report supplies complete paths, hashes and native-call
receipts. Primary owns full CMake registration, build and independent replay.

Whole `007F3970` removal remains outside this packet: it additionally needs
complete `007F2FD0` Lua/world behavior, actual departing-plane fields through
`+C41` and real pending-kill owners/access/lifetimes. No drain/death ordering,
retired task ownership, observer/cache lifetime, arena, private EH/fault,
concurrency, full construction/world/profile or gameplay binding is added.

## Primary integration

Whole 63-byte promotion: signed index guards, backward actual member stores, first-cell publication and direct raw277 Source on the same root. One 61-check count-four family connects actual sorted producer, selected BE receipt, ordinal/first-plane/leader consumers. Original63 executes with only its natural four-byte CALL operand relocated to raw Source; no extra bridge. Standalone Source tail-JMP and inlined BE Source CALL frames are distinct; original five-member caller-word/runtime/class/deferred-message/game contracts remain external.

Main `5096df208` passed the full Win32 build and all three existing CTests. The independent primary recipe freshly compiled 6 TUs; its sealed receipt is `local/cc11_promotion_current_primary/inputs_after.json`. Original class/world/game validation remains separate.
