# Native fonts in normal application startup, packet 36

Normal `GameFontHost` now uses `GameNativeFontApplication` on the application's existing renderer, VFS, strings and texture cache. It publishes into the same stable registry that the frontend reads. This replaces the normal host's semantic `FontResources`, private D3DX imports and independently loaded image owners. The existing sprite/CPU Text bridge borrows the native resources; native Text buffer/material drawing remains a separate integration.

The tested Source is **`a53cd233266d3494fb952acbae24b0ea87e736c9`**. The host change is `b596d6ba4`, sprite bridge `88f60d172`, and independent lifetime review `1655f581b`. See the [machine receipt](../reports/cc12_native_font_host_adoption_36.json) and [lifetime review](CC12_NATIVE_FONT_HOST_RETIREMENT_REVIEW_36.md).

## Ownership and failure behavior

The producer uses the recovered GFX → alpha → DAT order. Each stable registry row borrows its producer's `NativeFontResources`; adoption consumes the sole decoded `FontData` allocation and both returned actual image references. Startup checks the descriptor/row/operation identities, completed production, absent semantic owner and empty transferred input slots. It does not call `native_fonts()` or bind a native Text factory in this packet.

The sprite bridge reads that same live data allocation. It resolves the GFX image through the resource's canonical actual-owner registry, checks that its companion aliases the live raw `+04` count and that the profile is `D61948`, then calls the existing `B3CEA0` COM getter. It performs no `AddRef`, metric copy, replacement texture construction or raw-vtable call. The semantic row arm remains available for standalone compatibility callers.

Normal shutdown and exceptional destructor fallback both close the menu/frontend and their cached sprite runs, retire the fonts, then release the device view and drain the native renderer/singletons. Producer and descriptor metadata survive the shared drain. Member order destroys the retired producer before its registry. The explicit manager-drain entry also requires this closure.

Host state distinguishes a completed load awaiting retirement from an interrupted operation. It latches load failures, including callee preflight and post-production failures; a second load is rejected. Retirement marks its attempt before callee preflight, permits one successful transition and makes later host closure metadata-only. Interrupted native state takes the existing `_Exit` containment path instead of replay or guessed rollback. The [source review](CC12_NATIVE_FONT_HOST_RETIREMENT_REVIEW_36.md) identified and verified the fixed load-retry gap. Failure injection was not performed.

Future native Text integration must retire Text allocations, retain their **same bound factory** through font retirement, and only then destroy factory metadata. Deleting that future factory before fonts would violate the producer contract. The current frontend has no such factory.

## Evidence

`./scripts/build.ps1` passed on the combined Source: MSVC Win32 and all three existing suites, **8.77 s** for CTest. These math/native-math/tool suites do not test font drawing. The runtime executable, map, relevant objects, 17 source files and test log were frozen as 32 hash-checked inputs before launch.

Two fresh runs used the frozen executable, real installed assets/providers, private windowed settings and the serialized launcher. Both returned **0**, loaded six native fonts, kept the same descriptors/registry with zero semantic font owners, and retired fonts before native drain. GFX identities demonstrate two shared glyph images; all six rows share the same actual alpha image. All image profiles were `D61948`. The DAT glyph counts were 116 for the two Viper descriptors and 207 for the four Arial descriptors.

| Run | Observation | Limit |
| --- | --- | --- |
| `runtime_01` | 800×600, two loop ticks, one Present and one inhibited Present; two Viper19 runs, 55 glyph quads; readable title/login prompt in the frontend capture | 43 host methods still unimplemented |
| `runtime_02` | Eight ticks, seven Presents; synthetic press-start reaches `ScreenVisible`; title text remains readable; clean retirement | Menu body is blank; only Viper19 was drawn; 74 reached methods unimplemented |

Each run retained the GUI Font/Bilinear/cold-POST three qualified sampler observations and the warm POST cache hit without a fourth observation. The font-owner change reduces independent COM image duplication, so current observed device counts need not equal the older semantic-font run's counts. No fixed substitute count or sampler word was introduced.

The shared drains contained 41 registered slots in run 1 and 42 after the menu transition. Both left native renderer/manager publications null, joined the worker and ended with **device/API COM counts `0/0`**. Screenshots are diagnostic frontend captures before native EndFrame; they do not prove the full native drawing path. The original installed executable remained SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

`font_resource_opens=7` now means six completed DAT reads plus the separate fingerprint read. Renderer image-cache/provider opens are excluded. The fingerprint still reports 240 defined bytes.

The ignored evidence archive is `local/cc12_native_font_host_adoption_36/evidence_family.zip`, SHA-256 **`f3784755c3130ae1724aa35b5187d8f59560a7b69ca2e17614d8450c0921c1a1`**. It contains 52 payload files plus its manifest; every archived payload was hash-checked. Original installed assets and OS/D3DX binaries remain external dependencies. Later ledger command receipts are supplemental files outside this sealed archive.

## Original evidence and remaining scope

The associated Original entries are `0073BAE0` (font/GUI startup), `00AC3910` (Lua descriptors), `00AD4C30` (initial resource load) and the existing `00AD53A0` retirement fragment. See [producer evidence](GAME_NATIVE_FONT_APPLICATION.md), [resource ownership](NATIVE_FONT_RESOURCES.md) and [font ownership](FONT_RESOURCE_OWNERSHIP.md). This packet adopts existing recovered behavior and leaves Original names/bodies unchanged.

Original `AD4C30` takes ECX font and five stack strings, with `RET 14h`. The Source host/producer is a new C++ API. Whole-file DAT transport, typed string/tree allocation and exception timing do not reproduce the Original stream/pool/SEH ABI. Retirement uses the established typed payload/GFX/alpha fragment; the raw font string/tree/sentinel tail remains outside that projection. Destructive path reload, device recreation and interrupted cleanup are unproved.

All six fonts loaded, but only Viper19 drawing was observed. Native Text buffers, sections and font-effect submission are not activated; the normal menu remains incomplete, and gameplay is unvalidated. The [Text owner readiness report](CC12_NATIVE_TEXT_OWNER_READINESS_36.md) identifies the next concrete work: `CD71F0`/`CE0A80` Text-pool CRT ownership and `CD7210` live lineage with `AA8200` prerequisite, followed by canonical mutable GUI cells and the primary raw model/group route. The overall reconstruction is not complete.
