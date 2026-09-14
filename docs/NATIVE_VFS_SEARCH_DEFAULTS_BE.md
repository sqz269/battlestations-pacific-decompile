# Native VFS startup search defaults (BE)
The complete saved `00738360..0073BAD4` body is 14,197 bytes. The installed PE and live Ghidra bytes match SHA-256 `69fee27dbfafde84f1de4c8debe61839d9b25b82d6cc3b6771828d039174e0da`. All 4,310 decoded instructions are present in the saved function listing. Its 648 direct CALL sites are recorded and verified in the report.
The source accepts a borrowed, already constructed actual A0h manager, actual pooled-string storage, and lifetime callbacks. It invokes the existing raw registration services for all 78 calls in native order (7 extensions, 58 directories, 13 prefix entries). Every entry constructs the value string before the group string, registers, then destroys the group before the value. The first 51 entries use explicit native resize plus `length+1` byte copying; the final 27 use native C-string construction. Eight-byte headers and actual pool ownership are retained.
## Ordered calls
| # | Site | Service | Group/extension | Value/directory |
|---:|---|---|---|---|
| 1 | `00738401` | `00be25e0` | `textures` | `dds` |
| 2 | `007384d2` | `00be25e0` | `textures` | `tga` |
| 3 | `007385a1` | `00be2600` | `textures` | `textures\` |
| 4 | `00738670` | `00be2600` | `textures` | `models\textures\` |
| 5 | `0073873f` | `00be2600` | `textures` | `models\textures\noseart\` |
| 6 | `0073880e` | `00be2600` | `textures` | `models\gui\map\units\` |
| 7 | `007388dd` | `00be2600` | `textures` | `models\gui\map\icons\` |
| 8 | `007389ac` | `00be2600` | `textures` | `interface\textures\mainmenu\` |
| 9 | `00738a7b` | `00be2600` | `textures` | `interface\textures\` |
| 10 | `00738b4a` | `00be2600` | `textures` | `Particles\Textures\` |
| 11 | `00738c19` | `00be2600` | `textures` | `Particles\Textures\anims\fragments\` |
| 12 | `00738ce8` | `00be2600` | `textures` | `Effects\Flares\Textures\` |
| 13 | `00738db7` | `00be2600` | `textures` | `Fonts\` |
| 14 | `00738e86` | `00be2600` | `textures` | `Weather\` |
| 15 | `00738f55` | `00be2600` | `textures` | `Terrain\` |
| 16 | `00739024` | `00be2600` | `textures` | `Effects\` |
| 17 | `007390f3` | `00be2600` | `textures` | `Effects\Coast\` |
| 18 | `007391c2` | `00be2600` | `textures` | `Effects\WaterTracer\` |
| 19 | `00739291` | `00be2600` | `textures` | `Effects\Foam\` |
| 20 | `00739360` | `00be2600` | `textures` | `Effects\Caustics\` |
| 21 | `0073942f` | `00be2600` | `textures` | `Effects\Lightning\` |
| 22 | `007394fe` | `00be2600` | `textures` | `Effects\Traceline\` |
| 23 | `007395cd` | `00be2600` | `textures` | `Effects\Foliage\` |
| 24 | `0073969c` | `00be2600` | `textures` | `Effects\postprocess\` |
| 25 | `0073976b` | `00be2600` | `textures` | `interface\textures\terkep\` |
| 26 | `0073983a` | `00be1480` | `lua` | `interface\` |
| 27 | `00739909` | `00be1480` | `mmod` | `models\` |
| 28 | `007399d8` | `00be1480` | `dat` | `fonts\` |
| 29 | `00739aa7` | `00be1480` | `dat` | `textures\` |
| 30 | `00739b76` | `00be1480` | `shbin` | `shaderfx\bin\` |
| 31 | `00739c45` | `00be25e0` | `shaderfx` | `shfx` |
| 32 | `00739d14` | `00be2600` | `shaderfx` | `shaderfx\` |
| 33 | `00739de3` | `00be2600` | `shaderfx` | `shaderfx\plane\` |
| 34 | `00739eb2` | `00be2600` | `shaderfx` | `shaderfx\ship\` |
| 35 | `00739f81` | `00be2600` | `shaderfx` | `shaderfx\common\` |
| 36 | `0073a050` | `00be2600` | `shaderfx` | `shaderfx\particles\` |
| 37 | `0073a11f` | `00be2600` | `shaderfx` | `shaderfx\terrain\` |
| 38 | `0073a1ee` | `00be2600` | `shaderfx` | `shaderfx\ocean\` |
| 39 | `0073a2bd` | `00be2600` | `shaderfx` | `shaderfx\lights\` |
| 40 | `0073a38c` | `00be2600` | `shaderfx` | `shaderfx\postprocess\` |
| 41 | `0073a45b` | `00be2600` | `shaderfx` | `shaderfx\gui\` |
| 42 | `0073a52a` | `00be1480` | `pso` | `shaders\` |
| 43 | `0073a5f9` | `00be1480` | `vso` | `shaders\` |
| 44 | `0073a6c8` | `00be25e0` | `fshaders` | `mshd` |
| 45 | `0073a797` | `00be2600` | `fshaders` | `fshaders\` |
| 46 | `0073a866` | `00be2600` | `fshaders` | `fshaders\plane\` |
| 47 | `0073a935` | `00be2600` | `fshaders` | `fshaders\ship\` |
| 48 | `0073aa04` | `00be2600` | `fshaders` | `fshaders\common\` |
| 49 | `0073aad3` | `00be2600` | `fshaders` | `fshaders\particles\` |
| 50 | `0073aba2` | `00be2600` | `fshaders` | `fshaders\terrain\` |
| 51 | `0073ac71` | `00be2600` | `fshaders` | `fshaders\ocean\` |
| 52 | `0073acf6` | `00be1480` | `pel` | `effects\postprocess\` |
| 53 | `0073ad7b` | `00be1480` | `pfx` | `effects\postprocess\` |
| 54 | `0073ae00` | `00be1480` | `pfx` | `effects\postprocess\` |
| 55 | `0073ae85` | `00be1480` | `raw` | `Weather\` |
| 56 | `0073af0a` | `00be1480` | `pes` | `particles\` |
| 57 | `0073af8f` | `00be25e0` | `sound` | `fsb` |
| 58 | `0073b014` | `00be2600` | `sound` | `sound\events\` |
| 59 | `0073b099` | `00be2600` | `sound` | `sound\engines\` |
| 60 | `0073b11e` | `00be2600` | `sound` | `sound\explosions\` |
| 61 | `0073b1a3` | `00be2600` | `sound` | `sound\weapons\` |
| 62 | `0073b228` | `00be2600` | `sound` | `sound\messages\` |
| 63 | `0073b2ad` | `00be2600` | `sound` | `sound\messages\warning` |
| 64 | `0073b332` | `00be2600` | `sound` | `sound\environment\` |
| 65 | `0073b3b7` | `00be2600` | `sound` | `sound\music\` |
| 66 | `0073b43c` | `00be2600` | `sound` | `sound\muzzle\` |
| 67 | `0073b4c1` | `00be2600` | `sound` | `sound\impact\` |
| 68 | `0073b546` | `00be2600` | `sound` | `sound\splash\` |
| 69 | `0073b5cb` | `00be25e0` | `events` | `fev` |
| 70 | `0073b650` | `00be2600` | `events` | `sound\events\` |
| 71 | `0073b6d5` | `00be1480` | `bik` | `movies` |
| 72 | `0073b75a` | `00be25e0` | `mpaks` | `mpak` |
| 73 | `0073b7df` | `00be2600` | `mpaks` | `mpak\classes\` |
| 74 | `0073b864` | `00be2600` | `mpaks` | `mpak\scenes\` |
| 75 | `0073b8e9` | `00be2600` | `mpaks` | `mpak\global\` |
| 76 | `0073b96e` | `00be2600` | `mpaks` | `mpak_pc\classes\` |
| 77 | `0073b9f3` | `00be2600` | `mpaks` | `mpak_pc\scenes\` |
| 78 | `0073ba78` | `00be2600` | `mpaks` | `mpak_pc\global\` |

The duplicate `pfx` prefix is present twice. `sound\messages\warning` and `movies` have no trailing slash in the original literal. The source preserves these exact bytes and order.
## Boundary
This is source-level registration for the normal initialized-manager path. It does not construct the manager, bind global publications, reproduce the original stack/EH ABI, or prove game startup. The returned source interface explicitly borrows the manager and services. The caller must keep their raw deletion bindings and contexts alive through manager drain. The source is not yet registered in `cmake/startup.cmake`; primary integration owns that append and must rebuild the combined archive.
Proposed name: `register_native_vfs_search_defaults_00738360`. Original symbol remains `BSP_Application_RegisterResourceSearchPaths`; register inputs are from globals in the original, so the source signature is an explicit composition API rather than original ABI. No Ghidra mutation was made by this worker.
