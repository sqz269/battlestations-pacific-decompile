# Native VFS startup search defaults (BE)
The complete saved `00738360..0073BAD4` body is 14,197 bytes. The installed PE and live Ghidra bytes match SHA-256 `69fee27dbfafde84f1de4c8debe61839d9b25b82d6cc3b6771828d039174e0da`. All 4,310 decoded instructions are present in the saved function listing. Its 648 direct CALL sites are recorded and verified in the report.
The source borrows the actual A0h manager publication as `void* volatile& actual_manager_publication_0109ceec`, actual pooled-string storage, and lifetime callbacks. It invokes the existing raw registration services for all 78 calls in native order (7 extensions, 58 directories, 13 prefix entries). Every entry constructs the value string before the group string, registers, then destroys the group before the value. The first 51 entries use explicit native resize plus `length+1` byte copying; the final 27 use native C-string construction. Eight-byte headers and actual pool ownership are retained.
The original entry takes no arguments and reads global `0109CEEC` into ECX separately for each registration. All 78 loads occur after both header constructions (and after the optional group memcpy for the first 51 entries). No CALL or ECX write intervenes before the corresponding registration, and no branch targets the instructions after a load through that call. The source rereads the borrowed publication in each registration call expression, so changes during allocation, registration, or cleanup affect the next native load point. The publication must stay alive through the call; `volatile` preserves these reads without providing thread synchronization.

The ordered audit matches all 4,310 saved instruction addresses and all 648 direct calls against the installed PE, then checks all 156 literal operands and 78 source scopes. Each report registration records its load, constructor, optional copy, and literal addresses. The original no-argument ABI remains unchanged; these references are an explicit source composition interface.

## Ordered calls
| # | Site | ECX load | Service | Group/extension | Value/directory |
|---:|---|---|---|---|---|
| 1 | `00738401` | `007383f6` | `00be25e0` | `textures` | `dds` |
| 2 | `007384d2` | `007384bd` | `00be25e0` | `textures` | `tga` |
| 3 | `007385a1` | `00738596` | `00be2600` | `textures` | `textures\` |
| 4 | `00738670` | `0073865b` | `00be2600` | `textures` | `models\textures\` |
| 5 | `0073873f` | `00738734` | `00be2600` | `textures` | `models\textures\noseart\` |
| 6 | `0073880e` | `007387f9` | `00be2600` | `textures` | `models\gui\map\units\` |
| 7 | `007388dd` | `007388d2` | `00be2600` | `textures` | `models\gui\map\icons\` |
| 8 | `007389ac` | `00738997` | `00be2600` | `textures` | `interface\textures\mainmenu\` |
| 9 | `00738a7b` | `00738a70` | `00be2600` | `textures` | `interface\textures\` |
| 10 | `00738b4a` | `00738b35` | `00be2600` | `textures` | `Particles\Textures\` |
| 11 | `00738c19` | `00738c0e` | `00be2600` | `textures` | `Particles\Textures\anims\fragments\` |
| 12 | `00738ce8` | `00738cd3` | `00be2600` | `textures` | `Effects\Flares\Textures\` |
| 13 | `00738db7` | `00738dac` | `00be2600` | `textures` | `Fonts\` |
| 14 | `00738e86` | `00738e71` | `00be2600` | `textures` | `Weather\` |
| 15 | `00738f55` | `00738f4a` | `00be2600` | `textures` | `Terrain\` |
| 16 | `00739024` | `0073900f` | `00be2600` | `textures` | `Effects\` |
| 17 | `007390f3` | `007390e8` | `00be2600` | `textures` | `Effects\Coast\` |
| 18 | `007391c2` | `007391ad` | `00be2600` | `textures` | `Effects\WaterTracer\` |
| 19 | `00739291` | `00739286` | `00be2600` | `textures` | `Effects\Foam\` |
| 20 | `00739360` | `0073934b` | `00be2600` | `textures` | `Effects\Caustics\` |
| 21 | `0073942f` | `00739424` | `00be2600` | `textures` | `Effects\Lightning\` |
| 22 | `007394fe` | `007394e9` | `00be2600` | `textures` | `Effects\Traceline\` |
| 23 | `007395cd` | `007395c2` | `00be2600` | `textures` | `Effects\Foliage\` |
| 24 | `0073969c` | `00739687` | `00be2600` | `textures` | `Effects\postprocess\` |
| 25 | `0073976b` | `00739760` | `00be2600` | `textures` | `interface\textures\terkep\` |
| 26 | `0073983a` | `00739825` | `00be1480` | `lua` | `interface\` |
| 27 | `00739909` | `007398fe` | `00be1480` | `mmod` | `models\` |
| 28 | `007399d8` | `007399c3` | `00be1480` | `dat` | `fonts\` |
| 29 | `00739aa7` | `00739a9c` | `00be1480` | `dat` | `textures\` |
| 30 | `00739b76` | `00739b61` | `00be1480` | `shbin` | `shaderfx\bin\` |
| 31 | `00739c45` | `00739c3a` | `00be25e0` | `shaderfx` | `shfx` |
| 32 | `00739d14` | `00739cff` | `00be2600` | `shaderfx` | `shaderfx\` |
| 33 | `00739de3` | `00739dd8` | `00be2600` | `shaderfx` | `shaderfx\plane\` |
| 34 | `00739eb2` | `00739e9d` | `00be2600` | `shaderfx` | `shaderfx\ship\` |
| 35 | `00739f81` | `00739f76` | `00be2600` | `shaderfx` | `shaderfx\common\` |
| 36 | `0073a050` | `0073a03b` | `00be2600` | `shaderfx` | `shaderfx\particles\` |
| 37 | `0073a11f` | `0073a114` | `00be2600` | `shaderfx` | `shaderfx\terrain\` |
| 38 | `0073a1ee` | `0073a1d9` | `00be2600` | `shaderfx` | `shaderfx\ocean\` |
| 39 | `0073a2bd` | `0073a2b2` | `00be2600` | `shaderfx` | `shaderfx\lights\` |
| 40 | `0073a38c` | `0073a377` | `00be2600` | `shaderfx` | `shaderfx\postprocess\` |
| 41 | `0073a45b` | `0073a450` | `00be2600` | `shaderfx` | `shaderfx\gui\` |
| 42 | `0073a52a` | `0073a515` | `00be1480` | `pso` | `shaders\` |
| 43 | `0073a5f9` | `0073a5ee` | `00be1480` | `vso` | `shaders\` |
| 44 | `0073a6c8` | `0073a6b3` | `00be25e0` | `fshaders` | `mshd` |
| 45 | `0073a797` | `0073a78c` | `00be2600` | `fshaders` | `fshaders\` |
| 46 | `0073a866` | `0073a851` | `00be2600` | `fshaders` | `fshaders\plane\` |
| 47 | `0073a935` | `0073a92a` | `00be2600` | `fshaders` | `fshaders\ship\` |
| 48 | `0073aa04` | `0073a9ef` | `00be2600` | `fshaders` | `fshaders\common\` |
| 49 | `0073aad3` | `0073aac8` | `00be2600` | `fshaders` | `fshaders\particles\` |
| 50 | `0073aba2` | `0073ab8d` | `00be2600` | `fshaders` | `fshaders\terrain\` |
| 51 | `0073ac71` | `0073ac66` | `00be2600` | `fshaders` | `fshaders\ocean\` |
| 52 | `0073acf6` | `0073ace1` | `00be1480` | `pel` | `effects\postprocess\` |
| 53 | `0073ad7b` | `0073ad6b` | `00be1480` | `pfx` | `effects\postprocess\` |
| 54 | `0073ae00` | `0073adf5` | `00be1480` | `pfv` | `effects\postprocess\` |
| 55 | `0073ae85` | `0073ae70` | `00be1480` | `raw` | `Weather\` |
| 56 | `0073af0a` | `0073aefa` | `00be1480` | `pes` | `particles\` |
| 57 | `0073af8f` | `0073af84` | `00be25e0` | `sound` | `fsb` |
| 58 | `0073b014` | `0073afff` | `00be2600` | `sound` | `sound\events\` |
| 59 | `0073b099` | `0073b089` | `00be2600` | `sound` | `sound\engines\` |
| 60 | `0073b11e` | `0073b113` | `00be2600` | `sound` | `sound\explosions\` |
| 61 | `0073b1a3` | `0073b18e` | `00be2600` | `sound` | `sound\weapons\` |
| 62 | `0073b228` | `0073b218` | `00be2600` | `sound` | `sound\messages\` |
| 63 | `0073b2ad` | `0073b2a2` | `00be2600` | `sound` | `sound\messages\warning` |
| 64 | `0073b332` | `0073b31d` | `00be2600` | `sound` | `sound\environment\` |
| 65 | `0073b3b7` | `0073b3a7` | `00be2600` | `sound` | `sound\music\` |
| 66 | `0073b43c` | `0073b431` | `00be2600` | `sound` | `sound\muzzle\` |
| 67 | `0073b4c1` | `0073b4ac` | `00be2600` | `sound` | `sound\impact\` |
| 68 | `0073b546` | `0073b536` | `00be2600` | `sound` | `sound\splash\` |
| 69 | `0073b5cb` | `0073b5c0` | `00be25e0` | `events` | `fev` |
| 70 | `0073b650` | `0073b63b` | `00be2600` | `events` | `sound\events\` |
| 71 | `0073b6d5` | `0073b6c5` | `00be1480` | `bik` | `movies` |
| 72 | `0073b75a` | `0073b74f` | `00be25e0` | `mpaks` | `mpak` |
| 73 | `0073b7df` | `0073b7ca` | `00be2600` | `mpaks` | `mpak\classes\` |
| 74 | `0073b864` | `0073b854` | `00be2600` | `mpaks` | `mpak\scenes\` |
| 75 | `0073b8e9` | `0073b8de` | `00be2600` | `mpaks` | `mpak\global\` |
| 76 | `0073b96e` | `0073b959` | `00be2600` | `mpaks` | `mpak_pc\classes\` |
| 77 | `0073b9f3` | `0073b9e3` | `00be2600` | `mpaks` | `mpak_pc\scenes\` |
| 78 | `0073ba78` | `0073ba6d` | `00be2600` | `mpaks` | `mpak_pc\global\` |

Entry 54 is `pfv`, correcting the previous source/report duplication of `pfx`: PUSH `00CFED0C` at `0073ADD5` feeds constructor `0073ADE6`; both the installed PE and live Ghidra memory contain `70 66 76 00` (`pfv` plus NUL) there. Entry 53 uses the separate `pfx` literal at `00CFED10`. The other 155 literal operands match the prior report. `sound\messages\warning` and `movies` have no trailing slash in the original literal. The source preserves these exact bytes and order.
## Boundary
This is source-level registration for the normal initialized-manager path. It does not construct the manager, bind global publications, reproduce the original stack/EH ABI, or prove game startup. The source interface explicitly borrows the publication slot and services; it rereads the slot at every native load point rather than retaining the initial pointer. The caller must keep their raw deletion bindings and contexts alive through manager drain. The source is not yet registered in `cmake/startup.cmake`; primary integration owns that append and must rebuild the combined archive.
Proposed name: `register_native_vfs_search_defaults_00738360`. Original symbol remains `BSP_Application_RegisterResourceSearchPaths`; register inputs are from globals in the original, so the source signature is an explicit composition API rather than original ABI. No Ghidra mutation was made by this worker.

## Publication correction validation

The corrected translation unit compiles with MSVC 19.51 x86 using `/c /std:c++20 /EHsc /MD /W4 /WX /permissive-` with no diagnostics. The complete ordered audit passes for 78 manager loads, 78 registrations, 156 literals, 648 direct CALL sites, and 4,310 instruction addresses; native body bytes retain the recorded hash. The combined CMake build and existing tests remain primary integration work. No new tests, Ghidra annotations, binary adapter, or game startup validation are claimed.
