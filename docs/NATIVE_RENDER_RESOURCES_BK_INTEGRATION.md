# Render-resource construction and child lifetime integration (BK/BL)

Six complete native bodies are reconstructed in explicit C++ provider domains: the 6ACh render-resource constructor, the 36-byte post-effect owner destructor/deleting destructor, and the CCh texture helper auxiliary release/destructor/deleting destructor. This batch adds 2,100 native body bytes. It does not close startup or gameplay validation.

| Native entry | Body bytes | Reconstructed behavior |
| --- | ---: | --- |
| `00b14a10` | 1356 | Render resources construction, ordered field initialization, concrete children and nine cleanup states |
| `00b4e2f0` | 217 | Post-effect owner member release and base cleanup |
| `00b4e450` | 30 | Post-effect deleting destructor |
| `00b52270` | 143 | Four auxiliary owners released through captured decrement function |
| `00b52400` | 324 | CCh texture helper destruction, texture releases and reverse vector cleanup |
| `00b52840` | 30 | CCh texture helper deleting destructor |

The post-effect and texture helper modules share the Win32 LONG decrement-cell type. Their canonical references borrow the actual live counter, validate current concrete profiles and retain captured identity through final release. Non-null fields clear after the release returns; null fields skip the clear. Registry binding can allocate metadata. Host companion retirement and quiescence remain explicit obligations.

The existing `00b52550` constructor now begins the counter's `std::atomic<int32_t>` lifetime at the original count-one store. This adds no native-body credit. Independent COFF comparison verified identical 1,141-byte function sections and all 35 symbolic relocations before/after in the worker; the combined-build comparison only normalizes the MSVC anonymous-namespace path hash. There is one count-one store and no count-zero store. Whole objects differ because additional standard-library COMDATs are emitted. Earlier constructor fixture evidence remains historical.

## Validation

`./scripts/build.ps1` passed on `b4f36b1b405b659e5b17da00cc11c7424a902e68` with 2540 unchanged tracked build inputs and both existing CTests (`reconstructed_math`, `native_math_differential`). All four scoped call-report audits passed. The constructor and child lifetime source were independently reviewed against the original assembly; the separately authored post-effect canonical terminal composition was also reviewed. The validation report records exact scope and source hashes.

Seven names/comments were saved in the existing BSP Ghidra project, preserving prior comments and the existing `00b52550` signature. Six original `__thiscall` signatures were recovered and read back; all seven exports were force-refreshed. Saved listing repairs restore omitted caller cleanup instructions and the complete texture-helper destructor tail. Three raw EH handler definitions add no reconstruction credit.

There is no new native differential fixture for these six routines, no drop-in binary ABI or native FH3/SEH claim, and no gameplay validation. The `00b4e840` post-effect constructor and its source producers remain open. The parent release path also needs explicit failed-initialization admission, including the preimage of member `+70` and ownership of the `+67C` texture's retained consumers. Do not infer construction-to-binding closure from these lifetime providers.

Evidence: [combined validation](../reports/native_render_resources_bk_validation.json), [constructor](NATIVE_RENDER_RESOURCES_CONSTRUCTION_BK.md), [post-effect owner](NATIVE_POST_EFFECT_OWNER_BL.md), [texture helper lifetime](NATIVE_RENDER_SERVICE_TEXTURE_LIFETIME_BL.md), and [release listing analysis](NATIVE_RENDER_RESOURCES_RELEASE_BL.md).
