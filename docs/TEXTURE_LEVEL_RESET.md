# 2D texture and cached-level reset

`D3D9ResetTexture2D` represents the concrete 2D callbacks at `00b3dd30` and
`00b3dd90`, reached through vtable `00d61948` slots `+20h` / `+24h`.
It owns one `IDirect3DTexture9` reference and borrows existing cached-level
surface bindings. It is not the complete native texture constructor, intrusive
owner, texture loader, renderer registration list, or device reset scheduler.

## Original fields and ABI

| Native field | Typed projection |
|---|---|
| `+10h` | Owned texture COM pointer |
| `+14h` | Mip count |
| `+18h` | Format |
| `+1ch` | Native creation flags |
| `+28h`, `+2ch` | Width, height |
| `+40h`, `+44h` | Array/count of eight-byte cached-level records |
| Record `+0h`, `+4h` | Level number and surface-wrapper pointer |

Release receives wrapper ECX and returns with ordinary `RET`. Restore receives
wrapper ECX, a device stack argument, and returns with `RET4`. Neither contains
an optional guard. New C++ functions expose HRESULTs and require caller-owned
synchronization plus stable level membership and wrapper/device lifetimes.
The native loops use signed indices and reload record base/count after each
callback; dynamic mutation is not supported by this typed projection.

## Release ordering

`release_texture_levels_00b3dd30` first visits cached level bindings in stored
order and invokes existing `surface_release_for_reset_00b3d510`. Only after
all nested surfaces have released their COM references does it perform the
native balanced AddRef/Release access pair on the texture, reload its member,
release the owned texture reference, and clear the pointer. Metadata and cached
binding identities remain unchanged. No registration or owner destruction is
invented. Null level entries or counts outside the native signed range are
rejected before mutation instead of dereferencing malformed owners.

The typed destructor releases only its owned texture reference. It neither
owns nor destroys borrowed cached bindings and is not a reconstruction of
`00b3f2e0`. Callers must manage those bindings' COM ownership and lifetime,
as well as any renderer-list membership, separately.

## Creation flags and restore ordering

`restore_texture_levels_00b3dd90` decodes the native masks directly:

| Flags | Device usage/pool mapping |
|---|---|
| Low nibble `0..3` | D3DPOOL values `0..3` |
| Bit `10h` | Usage `1` (RENDERTARGET) |
| Group `100h` / `200h` / `300h` / `400h` / `500h` | Usage `2` / `4000h` / `40h` / `100h` / `80h` |
| `(flags & f000h)==1000h` | Usage `200h` (DYNAMIC) |
| `(flags & ff000000h)==01000000h` | Usage `400h` (AUTOGENMIPMAP) |

Unmatched usage groups add nothing, as in the inspected branches. This texture
routine does not insert the buffer-specific DEFAULT-pool WRITEONLY mapping.
The pool jump table at `00b3defc` selects cases `00b3ddab`, `00b3ddaf`,
`00b3ddb6`, `00b3ddbd`. A low nibble greater than three falls through to native
code using the device argument as a pool value; the typed interface rejects
that unsupported input explicitly.

The function calls CreateTexture with width, height, mip count, decoded usage,
format, decoded pool, and null shared handle. On successful creation it stores
the new pointer, retains it before releasing a distinct old pointer, then drops
the temporary creation reference. The identity-equal case still drops that
temporary reference. These operations follow `00b3de6c..00b3deaf`.

Next, each cached record receives `GetSurfaceLevel(level)`. Its existing binding
is reinitialized via `surface_initialize_00b3cc80`, then the getter reference
is released. No standalone surface is created and no cached wrapper is replaced.
This preserves the essential relationship between a mip surface and the new
texture. Existing cached bindings must have empty COM owners after release;
otherwise typed restore rejects before texture creation.

## Failure domain and validation

Native restore ignores HRESULTs and does not initialize its CreateTexture output
temporary before the COM call. Therefore its failing-device path is not modeled
as a predictable null result. The typed function initializes temporaries,
reports failed HRESULTs (or E_POINTER for an unexpected successful-null result),
and cleans up any returned temporary reference. CreateTexture failure leaves
the previous texture owner unchanged. After successful installation, a later
GetSurfaceLevel/initializer failure leaves the new texture and any already
restored level bindings installed. There is no rollback. Later levels are not
attempted after such a failure; this is a documented safe interface boundary,
not a claimed native error branch.

The existing device reset probe now includes a 64x64 DEFAULT-pool 2D texture
with two cached levels. It releases both level bindings before the texture,
resets the device, recreates the texture and checks the same bindings against
GetSurfaceLevel(0/1), including 64x64 and 32x32 descriptions. This runs between
the default-surface and standalone-surface phases. The Win32 build, both existing
CTests and full installed-asset D3D9 probe passed; see
`reports/font_texture_reset_probe.txt`. No new test target or framework was added.
This verifies surface identity/metadata and real reset, not content restoration,
full scheduler integration, native ABI compatibility or gameplay.

Every analysis batch verified project `bsp`, program
`/battlestationspacific.exe`, x86 image base `00400000`. Saved Ghidra bytes
matched the installed PE for both full bodies and the pool table:

| Range | Bytes | SHA-256 |
|---|---:|---|
| `00b3dd30..00b3dd85` | 86 | `79b651f7ffa1321bc2e2a1652f0d397b58b9a77a9a9d4f35add970d4521815e2` |
| `00b3dd90..00b3def9` | 362 | `725508b5f5d5ffcae47674b0daca2805c95bb1ff3d60603448483708ddedb81b` |
| `00b3defc..00b3df0b` | 16 | `bf6e27ad79143889b5709e8c6b17e04821abde4c73b5c53fb8468ecb6c9da448` |

Restore was recovered from raw assembly because it was absent from the function
snapshot during the audit. Parent integration verified the body/table, checked
the 362-byte dry-run result and created the function. Both routines now have
descriptive Ghidra names/evidence comments; previous values were preserved,
the project saved, and the inventory/exports refreshed.
