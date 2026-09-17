# Native runtime 2D texture creation

Addresses: 00b2a070, 00b3f7b0, 00b33fc0

## Result and scope

R73 implements the renderer's runtime 2D texture factory and its complete unnamed
base and concrete owner constructors in `native_runtime_texture_creation`.
The source uses the existing actual COM interfaces, canonical texture pool,
string storage, resource-support singleton, owner lifetime and renderer domains.
This closes a dependency of the shadow texture holder and render-resource
initializer. It does not bind those callers into the application.

| Original routine | Inclusive body | Original ABI | Coverage |
| --- | --- | --- | --- |
| `00b2a070` | `00b2a070..00b2a360`, 753 bytes | ECX renderer; width, height, levels, format, flags; EAX owner; `RET 14h` | Complete source control flow and C++ cleanup projection; native ABI/FH3/SEH unproved |
| `00b3f7b0` | `00b3f7b0..00b3f92e`, 383 bytes | ECX raw 50h owner; COM, saved width, saved height, flags; EAX owner; `RET 10h` | Complete source control flow and C++ cleanup projection; native ABI/FH3/SEH unproved |
| `00b33fc0` | `00b33fc0..00b34006`, 71 bytes | ECX owner; COM, flags; EAX owner; `RET 8` | Complete source stores; new C++ interface |

Names are descriptive hypotheses. Original bodies, switch table, constants,
literal, unwind helpers and maps are recorded against live Ghidra and the
original PE in `reports/native_runtime_texture_creation_r73.json`.

## Factory contract

The factory preserves the optional renderer guard and reads current device
`renderer+1a10`. The low flag nibble maps 0..3 directly to D3DPOOL values;
other values leave the original pool stack slot unchanged. The source acquired
frame therefore requires a readable preimage when that domain is exercised.
It does not substitute a default pool.

Usage comes from bit 10h and exact masked values: 100h -> 2, 200h -> 4000h,
300h -> 40h, 400h -> 100h, 500h -> 80h; the F000h field equal to 1000h adds
200h, and the FF000000h field equal to 01000000h adds 400h. Actual
`IDirect3DDevice9::CreateTexture` is dispatched through current slot 5Ch.

Only a null output with nonzero HRESULT other than 8876017Ch or 8007000Eh
calls the existing full `00b29670` recreation body. The factory reloads the
device and retries once. A recreation context is required when that branch is
reached. No null-result recovery, success substitute or retry loop is added.

The captured output receives the original diagnostic AddRef/Release pair.
`00b3f2b0` supplies the canonical raw slot, then `00b3f7b0` constructs it with
current flags. Original flags bit 10h controls allocation accounting. The x87
sequence preserves wrapped width*height, unsigned conversions through the
original CE3978/D57DA0 constants, single-precision bytes-per-pixel storage,
FNSTCW before FADDP, temporary truncate mode, FISTP64, low-DWORD counter store
before restoring the control word. A fixture starts the counter at FFFFFFF0h
to exercise its unsigned path and wrapped low result.

The owner is appended as a borrowed pointer to `renderer+1b00`, using actual
`00735ff0` capacity growth and reloaded storage. The second diagnostic COM pair
precedes an unconditional final release of the reloaded creator output. Current
guard mode is checked at exit. No extra owner retain or cleanup is introduced.

## Owner and base contracts

The base writes CEB130, D5F1F4, then D5F228 profiles; starts actual count+4 at
one; clears +8/+C/+14; borrows COM at +10; records flags at +1C and the current
shared serial at +20 before incrementing 0108D6E8. It leaves +18 and bytes
beyond the 24h prefix untouched, including pool metadata.

The concrete constructor installs D61948 and initializes its complete 50h
owner body. It retains current COM, obtains current mip count, calls the input
COM's GetLevelDesc and consumes its output while ignoring HRESULT as native
does. Descriptor storage is a readable caller-owned preimage, not a synthetic
zero descriptor. Actual and saved dimensions remain distinct.

Two actual native strings carry the original D619E4 `handmade texture` literal
through the diagnostic record and resource-support getter. The normal path
returns both strings through actual native storage. The tracking increment
uses a fresh flags read. The owner retains one COM reference at factory return.

## Failure and lifetime boundaries

Factory handler CBD370 uses FuncInfo DF5B5C/map DF5B4C: state 1 returns the raw
slot through B3DCD0, then state 0 destroys the optional guard through B21110.
Constructor handler CBEF43 uses FuncInfo DF7800/map DF77E0: states 3, 2, 1, 0
destroy diagnostic B3F4C0, first string 41DD20, cache B3EC40 and base B34010.
Source RAII preserves the initial C++ exception search; cleanup terminates on
a second C++ exception. Acquired frames remain available after failures.

No COM release or complete owner rollback is invented on construction failure.
The source cleanup projection is not a native FH3/SEH or stack-layout proof.
Fresh-frame guards and required-domain errors are source interface contracts.

## Verification

- Strict MSVC Win32 build with `/MD /W4 /WX /fp:strict`; all three existing CTests pass.
- Full original factory, constructor and base bodies (1207 code bytes) execute
  in an isolated child with relocated direct calls and data operands.
- Four real Direct3D9 cases pass: original/source default-pool render target
  (64x32, one mip, flags 10h), and original/source managed texture (64x32,
  levels zero yielding seven mips, flags 1).
- Entire 50h original/source owner bytes agree after normalizing only COM
  identity and shared serial. Descriptor, mip count, flags, dimensions, serial,
  tracking and borrowed registration checks pass. Native pool metadata survives.
- Accounting produces 00001FF0h for the render target and leaves FFFFFFF0h
  unchanged for the managed case. x87 control word remains 027Fh.
- Actual owner deletion unregisters, restores tracking and returns the pool slot.
  Raw singleton shutdown drains two genuine registrations. Final real device
  and API Release results are both zero.

The probe uses an explicitly zeroed 1D94h renderer fixture with a real device
and empty resource containers. It does not run the renderer constructor or
application startup. Canonical raw strings, texture/surface pools, scalar
domains and resource-support manager are genuine existing providers. Original
COM calls reach real current Direct3D9 interfaces. Direct dependency calls use
the corresponding full source providers through x86 adapters; the three packet
bodies themselves execute their original bytes. Original EH and recreation
targets deliberately fail the fixture if reached. Those paths were unreached.

The first probe setup omitted the raw string-pool deletion binding: all four
creation cases passed, then shutdown rejected the missing binding. A temporary
diagnostic guessed an internal manager layout and faulted; it was removed and
replaced with the recovered slot-count routine. The final probe supplies the
actual raw deletion bindings and passes complete shutdown. These were probe
setup failures; they required no production-source change. Historical logs are
retained in the local evidence archive.

Retry, creation failures/null outputs, nonzero optional guard mode, invalid low
pool nibbles, cleanup exceptions, original FH3/SEH, application wiring and
gameplay are untested. The application was not rerun for these unbound routines.
No new repository tests were added.

## Follow-up dependency work

The shadow texture holder B4E020 now has a concrete runtime texture factory.
Its lazy level-surface getter B3FD80 and B3D640 operation still need full body
and ABI recovery; the latter lacks a separately defined Ghidra function.
The holder's lifetime and the larger B107F0 resource initializer remain open.

The bounded B107F0 producer investigation retained under
`local/resource_service_r73/` locates service+70 at B11563, following B4E470
at B11555. That existing 20h post-effect producer is a useful next lifetime
contract, not evidence of initialized storage before the producer executes.
Other producer-map rows remain locator hints requiring call-site review.
No whole resource-service completion or safe pre-initialization deletion is claimed.
