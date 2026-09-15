# Tracked registration of pending native resource and mesh sources (CI)

CI registers these already reconstructed sources in `cmake/startup.cmake`:

| Source | Existing evidence |
| --- | --- |
| `src/native_camera_group_resource.cpp` | CC camera/GroupParams readers and lifetime |
| `src/native_resource_container_lifetime.cpp` | CE resource-container destruction |
| `src/native_mesh_scalar_fields.cpp` | CF bounds, LOD and weight-name fields |
| `src/native_mesh_buffer_fields.cpp` | CG index and vertex-buffer fields |
| `src/native_mesh_remaining_fields.cpp` | CH compressed metadata and raw lighting |

Exactly five deferred `target_sources` lines were inserted before the deferred
native-data-placement include. All other bytes of the CMake file are unchanged.
The shared file became unleased during the CH turn. A separate edit lease was
claimed, verified and released in about0.33seconds, before configuration,
building or running fixtures. No lease is retained for this file.

The old `CMAKE_PROJECT_INCLUDE_BEFORE` cache entry was removed. CMake was then
regenerated from the tracked configuration, and each of the five files appears
exactly once in the generated `bsp_core.vcxproj`. `scripts/build.ps1` passed both
existing CTests. This is an incremental build in the existing Win32 build tree,
not a claim of a fresh build from an empty directory.

The frozen CH controlled-child fixture was relinked against that core and
passed again: eight copied-original CH bodies and the original indirect getter,
metadata callback destination changes, lighting special values/overlap/empty
fields, source read-failure effects, actual D3D9 HAL uploads/readbacks and final
mesh/cache/stream retirement. The fixture source differs only in its log path.
Its original-reader/CRT, raw-pool/cache preconstruction, platform callback,
device-loss and native exception boundaries remain as documented in CH and CG.
No new behavioral tests were added for this registration change.

The 2679 tracked source/build inputs match the CH source manifest except for
the five-line CMake edit. CI freezes a new source manifest, generated project,
cache, core/libraries, game build artifact, fixture executable and logs under
`local/native_mesh_fields_registry_ci/registered/`; hashes are published in
`reports/native_mesh_fields_registry_ci.json`. Older CC/CE/CF/CG/CH proof is
preserved unchanged. Their statements that tracked source registration was
pending describe those earlier builds; CI supplies the subsequent registration
and validation evidence.

This registration is published on `agent/orch4-20260910`. Main was not merged:
the separate CD plane-control findings and later incoming deltas still require
review. CI makes this branch build through the ordinary tracked configuration;
it does not establish complete mesh/subset admission, game startup, device
recreation, binary ABI compatibility or gameplay validation. The next source
work remains texture/material-factory admission, section finalization and the
subset/aggregate parser.
