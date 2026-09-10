# System scene lighting and shadow constants

This packet reconstructs the interior `00B46ED1..00B475B3` of
`00B46A70` through `write_system_lighting_shadow_prefix_00b46ed1`.
The parent native ABI is ECX = optional scene, EDX = camera, no stack arguments,
plain RET. The C++ function is a new bounded interface, not an ABI replacement.
It patches the caller's initialized 77-register float prefix. It neither clears
that buffer nor uploads it.

Evidence is the existing `C:/Users/sqz269/bsp.gpr` project, program
`/battlestationspacific.exe`, its exported assembly, and the installed PE at
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
Every live read/export used `tools/bsp.py ghidra`, which verifies the configured
project, program, language and image base before the request. The audit records
matching native/installed bytes and SHA-256 values for the complete fragment,
the copied shadow test fragment, the five helpers and two constants. No Ghidra
names, comments, source ledgers or shared metadata were changed by this worker.

## Retained owner contract

The scene, list, environment, light and shadow objects are borrowed projections
of actual retained objects. Field arrays are references, and pointer members are
references to live pointer slots. These classes do not create native objects,
retain or release them, or claim native object layout. The integrating caller
must bind the actual fields and keep every projected owner alive through its
last use; independent snapshots would violate that contract.

| Native access | C++ projection and behavior |
| --- | --- |
| `00B72110`, `[scene+1C]` | `SystemLightingScene::lighting_1c`; null scene or null returned lighting owner skips lighting and shadow |
| `[A+1C]`, `[sentinel+00]` | `SystemSceneLighting::sentinel_1c`, `SystemLightListNode::next_00`; first node identity is compared with the same sentinel |
| camera `+198`, `[A+10]`, `[first+08]` | Compare live mode to 3, capture environment, then capture first light; environment and first light remain the same through their uses |
| environment `+18`, `+28`, `+38+10h*i` | Normal ambient, mode-3 ambient, and six raw ambient-cube float4 records |
| light `+184`, `+1B4`, `+194`, `+1E0` | Normal diffuse, mode-3 diffuse, specular, and direction xyz |
| `00B7AAB0`, `[light+174]` | Same captured light supplies the shadow owner after the ambient-cube loop; only a null shadow owner skips the shadow portion |
| shadow `+144/+184/+1C4/+204`, `+38`, `+390` | Four contiguous raw matrices, direction xyz, and four limit words |
| shadow virtual `+08` | Required callback returning the actual borrowed texture owner; executed separately before width and height |
| returned texture virtual `+3C` / `+40` | Required `SystemShadowTexture` callbacks returning unsigned DWORD dimensions |

`SystemShadowTexture` deliberately requires concrete dimension callbacks.
`LogicalTexture` nominal dimensions have not been established as the return
values of these virtual calls. The second shadow callback can return a different
texture; the first result is not reused. `SystemShadowMapOwner` can be bound to
the same real owner as the parent-owned `MaterialShadowMapOwner`, but that
adapter and native lifecycle remain integration dependencies.

An empty first-light list is the distinct `native_empty_light_list_failure`
boundary at `00BF6713`, before any lighting write. It is not successful absence.
The inspected CRT wrapper calls `00BF66EF` with five zero arguments and has a
return path; the native caller continues at `00B46EF8` if it returns. This
packet does not invent a no-return property or emulate an unknown CRT handler.
The companion returns the explicit boundary status; native handler behavior and
possible sentinel payload continuation remain unreconstructed.

Missing required sentinel, first node, environment, light or callback-returned
texture produces `invalid_projection`. Callback inability produces
`callback_failed`. These are new host errors; native code does not guard those
pointers or expose these statuses. Earlier successful writes remain visible.
All three absence statuses and `complete` are successful bounded paths; callers
must distinguish those from the error and native-failure statuses.

## Exact write schedule

| Order | Destination | Source and copy behavior |
| --- | --- | --- |
| 1 | c43 | Environment `+28` in camera mode 3, otherwise `+18`; sequential raw DWORD copies |
| 2 | c44 | First light `+1B4` in mode 3, otherwise `+184`; sequential raw DWORD copies |
| 3 | c45 | First light `+194`; sequential raw DWORD copies |
| 4 | c52.xyz | First light `+1E0/+1E4/+1E8`; sequential raw MOVSS words |
| 5 | c42.x | Actual `00CFAD80` word reference; installed bits `43000000` (128.0f) |
| 6 | c46..51 | Captured environment's six ambient-cube records, in index order |
| 7 | c53..68 | Captured shadow's four matrices, transposed one raw source/store pair at a time in destination order |
| 8 | c69.xyz | Capture all three shadow direction words, then write all three |
| 9 | c70 | Capture all four shadow limit words, then write all four |
| 10 | c76.x | Shadow virtual `+08`, returned texture virtual `+3C`, unsigned conversion and float spill |
| 11 | c76.y | Same shadow virtual `+08` again, new result virtual `+40`, unsigned conversion and float spill |
| 12 | c76.z/w | Native x87 reciprocal schedule using the already stored width and height |

All other words, including c42.yzw, c52.w and c69.w, remain unchanged. The
four shadow transposes use bit copies, unlike camera helper `00B404A0`, whose
x87 FLD/FSTP can quiet signaling NaNs. Do not substitute that camera helper.

For each dimension, native FILD interprets the DWORD as signed 32-bit, then
conditionally adds the exactly representable float `2^32` from `00CE3978`.
FSTP spills to float before the next callback. The copied reciprocal sequence
is FLD width, FLD1, FLD ST0, FDIVRP ST2/ST0, FXCH, FSTP c76.z, FDIV height,
FSTP c76.w. The implementation uses that x87 schedule and inherits the caller's
precision, rounding and exception controls. Zero dimensions have no guard.
No reciprocal is written if the second texture or height callback is unavailable.

The caller supplies at least 308 float words. Output validation precedes the
native gates and is a new interface check. There is no clearing, resizing,
fallback data, texture cache, inferred clock state, or renderer global.

## Helper evidence and ABI

| Address | Native body | C++ interface |
| --- | --- | --- |
| `00B72110` | MOV EAX,[ECX+1C]; RET | `get_system_scene_lighting_00b72110` |
| `00B7AA20` | LEA EAX,[ECX+18]; RET | `get_system_ambient_00b7aa20` |
| `00B7AA30` | LEA EAX,[ECX+28]; RET | `get_system_mode3_ambient_00b7aa30` |
| `00B7AA40` | EAX = ECX+38+10h*stack index, arithmetic modulo 32-bit; RET4 | `get_system_ambient_cube_00b7aa40`; bounded to the caller's indices 0..5 |
| `00B7AAB0` | MOV EAX,[ECX+174]; RET | `get_system_shadow_owner_00b7aab0` |

Descriptive names are hypotheses. Proposed names/comments and complete existing
name/comment preimages are in the audit, for primary review and annotation.
Existing `BSP_Light_GetShadowMapOwner` naming and prior evidence comments are
preserved. The function inventory's recovered `void` signature for `00B46A70`
does not capture the ECX/EDX register inputs established by its prologue.

## Validation and remaining scope

MSVC Win32 isolated compilation passed with `/std:c++17 /EHsc /W4 /WX
/fp:strict /O2`. `scripts/build.ps1` also passed with both existing tests:
`reconstructed_math` and `native_math_differential`. This baseline build does not
compile the new source until the primary adds its CMake registration; the
separate isolated compile and native comparison do compile it.

One ignored local harness compared the 1,320 installed-byte shadow fragment
`00B4708C..00B475B3` against the companion. It relocated the two absolute `2^32`
constant references and appended RET, using synthetic ABI owners and callbacks
in the test process. All 308 prefix words matched. The case exercises a raw
signaling NaN transpose, two different returned textures, unsigned dimension
`FFFFFFFF`, height zero, write-before-next-callback observations and all callback
identities/order. Focused host checks cover mode-3 color selection, preserved
padding, failure on the second texture callback with earlier writes retained,
and the explicit empty-list boundary. No new tracked test suite was added.

The proof is local byte/fixture behavior under the tested x87 control state.
It is not native scene construction, a concrete game texture adapter, original
ABI compatibility, whole-system-builder completion, or gameplay validation.
The parent must integrate owner adapters, CMake registration, the other prefix
stages and final uploads. Native CRT failure continuation also remains open.
