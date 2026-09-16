# Native particle texture owners: ORCH4 application prerequisite audit

Date: 2026-09-16
Baseline: `main` at `af9ce1480`

## Result

The baseline had no reusable application-wide `NativeRenderActualOwners`
implementation. `PointEffectReleaseRuntime` is an existing implementation, but
its typed bindings contain only `NativePointEffectReference` objects and carry
point-effect frame/deferred-deletion behavior. It cannot be the canonical mesh,
material, stream, or texture companion domain.

`NativeRenderActualOwnerRegistry` now supplies the missing general metadata
binding. It admits one companion per actual identity only when the companion
borrows that identity's exact `raw+04` atomic. Binding and lookup do not change
the native count or bytes. Exact retirement removes the same identity/companion
pair; duplicate identity, mismatched count, missing resolve, and live registry
destruction are hard failures. Its three callback adapters match
`GuiNativeGeometryRegistration`'s bind/unbind/find shape. The registry serializes
its map only; the application must still provide the native resource/lifetime
synchronization that keeps a returned companion alive.

This closes one real owner-binding prerequisite. It does not claim that the
texture graph is installed in `bsp_game`.

## Concrete texture graph audit

| Required domain | Reconstructed source on baseline | Application installation on baseline |
| --- | --- | --- |
| Canonical actual companion registry | Abstract interface plus specialized point-effect and camera-probe implementations | Closed by `NativeRenderActualOwnerRegistry`; no resource family is auto-created |
| `GuiNativeGeometryOwners` shared registration | Complete bind/find/unbind consumer and mesh/section/material companion families | No production construction found |
| `NativeTexture2DOwnerContext` | Complete D61948 owner construction/destruction through B3F930/B3F590 | No production aggregate found |
| `NativeTextureLoadOwners` | Registers 2D/cube/volume creators into the geometry registry | No production construction found |
| `NativeTextureLoadingContext` / `NativeTextureCacheContext` | B2C2D0, B30B40, and B319B0 reconstructed | No production construction found |
| String/VFS identity | `GameNativeVfsApplication` owns the actual raw string/VFS graph | Its public raw-services view exposes publication, bindings, name resolution, and dates; the internal open route and stored-stream conversion are not exposed on this baseline |
| Texture name resolution | `bind_native_texture_vfs_name_resolution` binds retained BDF4C0 operations | Ready once the same VFS/string contexts are supplied |
| Renderer retry | `recreate_native_renderer_device_00b29670` is reconstructed | Requires one fully populated `NativeRendererDeviceRecreationContext`; no production owner of that aggregate was found |
| D3DX texture imports | Real named cube/volume import owner exists; 2D image-info/create signatures are reconstructed | No single application import owner supplies all B2C2D0 imports on this baseline |
| Post-load callback | B2C2D0 correctly requires a concrete callback when its incoming word is nonzero | The particle path at B013E3 calls B319B0 with word zero, so it does not reach this callback; broader texture consumers still need their real callback target |

An actual D3D9 device alone does not satisfy any of the companion, pool,
string/VFS, synchronization, cache, or retry ownership contracts above.

## Address and identity checks

- Live Ghidra bytes at `00D5F10C` (`00D5F0A8 + 64h`) are `B0 19 B3 00`, the
  little-endian `00B319B0` texture loader.
- `reports/frame_bounds_integration_native_identity.json` records primary-PE
  matches for B1D570 (30 bytes), B1EDC0 (407 bytes), and D5E5C4 (8 bytes).
  Live Ghidra D5E5C4 bytes are `E0 30 BD 00 70 D5 B1 00`, matching the expected
  BD30E0/B1D570 zero/deleting slots used by the focused owner test.
- No Ghidra mutation or new native address claim was made. The new registry is
  host ownership metadata over already reconstructed actual companions.

## Validation

- Strict MSVC Win32 `/W4 /WX /fp:strict` build passed.
- Both tracked CTests passed: `reconstructed_math` and `tool_tests`.
- An ignored Win32 focused probe used allocated actual 18h
  `NativeRenderContextStorage`, the genuine `NativeRenderContextReference`
  scalar-delete path, and exact registry retirement. All three checks passed:
  `exact_release_unbind=1`, `mismatched_counter=1`, and
  `duplicate_identity=1`.

The checks prove the host registry contract and build integration. They do not
prove application startup installation, D3D9 execution, or gameplay rendering.
