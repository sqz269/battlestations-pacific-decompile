# Post-effect BN / BO20 integration

Three native bodies totaling 284 bytes are reconstructed and build-tested at `afee0acdebab4e3b8152159b83e13de51fb8068c`.

| Native entry | Bytes | Behavior |
| --- | ---: | --- |
| `00b4e1f0` | 189 | 20h D61EC0 owner destruction: material, nodes, raw draw record and frame, then base cleanup |
| `00b4e430` | 30 | 20h deleting destructor: destroy, optional free, return captured allocation |
| `00b51bd0` | 65 | Initialize a caller-allocated 28h draw record through existing B51A20 |

The 20h owner is distinct from the existing 24h D61EC8 owner: its `+18` member is the raw 28h draw record. Material identity is captured before the decrement IAT; later member reads remain fresh. Non-null members clear after returning release calls. Its canonical reference borrows the existing atomic counter without initialization, reset or retain; companion storage persists through retirement notification. The primary authored and reviewed these two bodies against all original instructions and saved base-only cleanup evidence; no independent peer review is claimed for them.

The primary independently reviewed the worker's draw-record wrapper and all 24 original instructions. The current 71-byte generated source section matches the original after six argument-displacement adjustments, saving/restoring the borrowed EDX access pointer, reloading it before the existing call, and normalizing that call's relocation. Original x87 bytes, argument order, saved ESI, captured return pointer and RET18h remain. The wrapper performs no allocation, refcount operation or key-field writes. The extra EDX source argument is an explicit new interface.

## Validation

`./scripts/build.ps1` passed in MSVC Win32 Release with 2553 unchanged tracked build inputs. Both existing CTests (`reconstructed_math`, `native_math_differential`) passed. Fourteen numeric call references passed the scoped report audit; five indirect rows remain explicitly outside that numeric check. Source/provider pins and all three live/original-PE body spans matched. The worker's earlier CMake timestamp-access failure and successful unchanged-source retry are preserved separately from this successful combined build.

Three names and original `__thiscall` signatures were saved and read back in the existing BSP project; prior comments were preserved and exports force-refreshed. Two omitted caller cleanup spans were restored without changing callee no-return flags. Defining the CBFAF8 exception handler adds no body credit.

The [validation report](../reports/native_post_effect_bn_bo_validation.json) records exact hashes and the immutable archive. These are source/build/static-instruction results. There is no new native differential fixture, drop-in binary ABI, unrestricted FH3/SEH or gameplay claim. B4E470 producer construction and actual parent construction-to-binding execution remain open; the separately active B4E840 constructor is excluded from this batch. Main integration belongs to the existing main orchestrator.

Evidence: [20h owner](NATIVE_POST_EFFECT_OWNER_20H_BO.md), [draw record](NATIVE_POST_EFFECT_DRAW_RECORD_BN.md), and [member producer map](NATIVE_RENDER_RESOURCE_MEMBER_PRODUCERS_BM.md).
