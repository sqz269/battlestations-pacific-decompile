# Actual camera constructor and renderer parameter boundary

Addresses: B71A80; compiler boundaries CC1AD0, CC1AD8, CC1AE3, CC1AF1; shared viewport provider B1F850 and actual renderer getter B1FF60.

| Entry | Range | Bytes | Coverage |
| --- | --- | ---: | --- |
| B71A80 camera constructor | [B71A80,B71CDC) | 604 | Complete normal body; source C++ cleanup projection |
| B1F850 viewport constructor | [B1F850,B1F8EC) | 156 | Existing single body, additional actual-renderer binding |
| B1FF60 renderer parameter getter | [B1FF60,B1FF67) | 7 | Complete actual receiver +1A14 |
| CC1AD0 base cleanup | [CC1AD0,CC1AD8) | 8 | Compiler boundary evidence; raw B6F440 reused |
| CC1AD8 failed viewport allocation cleanup | [CC1AD8,CC1AE3) | 11 | Compiler boundary evidence; current incoming cell freed |
| CC1AE3 retained438 cleanup | [CC1AE3,CC1AF1) | 14 | Compiler boundary evidence; raw605FD0 reused |
| CC1AF1 FH3 handler | [CC1AF1,CC1AFB) | 10 | Transport evidence only |

All 810 bytes, including the 43 compiler bytes, match live Ghidra and the original PE. The four compiler entries already have complete functions; no worker Ghidra mutation or repair is needed. Exact last instructions, data bytes and all 17 direct/indirect/tail call rows are in the report and ignored evidence directory.

## Actual storage and providers

`construct_native_camera_storage_00b71a80` receives actual aligned45Ch storage, the mutable original incoming argument word, pointer views over initialized caller scratch, borrowed current bindings and fresh persistent diagnostics. It invokes the genuine raw B6F5A0 prefix constructor and establishes no enclosing camera/tail aggregate. That prefix creates the actual atomic+4. The new body never adopts `NativeCameraOwner`, `CameraState` or `SceneAttachmentRuntime`.

The plane set occupies exactly +2F4..+437. CameraPlaneSet and CameraPlaneRecord are asserted aggregate/trivially-copyable types. A byte-preserving memcpy starts the implicit object lifetime without invoking member initializers; genuine B659D0 then performs its original initialization. This source lifetime operation is outside the native store schedule and preserves every preimage byte. There is no overlapping live aggregate. The shared B659D0 private matrix/plane temporaries remain outside raw alias and fault-time claims.

The existing viewport body is factored once for its prior environment and the new `NativeViewportRawEnvironment`. The latter borrows the actual F8D394 publication, a required pure profile resolver and genuine exact-target slot30 invoker. It reloads publication/profile/slot independently for each B1F850 call and reads actual result+0C then result+10. The raw B1FF60 provider performs only actual receiver+1A14; it creates no renderer, parameters, COM object or dimensions. The provider binding must establish the concrete renderer extent/profile and genuine reached behavior. No D3D9StateCache reinterpretation or unknown-profile fallback is used by the raw path.

The camera's two later calls use one captured actual renderer. It invokes the first current slot30, captures the then-current table **before** reading height from the first result, then loads slot30 from that captured table and invokes the second call. Width is read afterward. The current viewport is captured between the width and height scratch stores. The required resolver is pure and x87-neutral; all tables, actual results and bindings remain alive throughout reached reads.

B71A80 allocates the real34h viewport through the existing CRT new-handler allocation boundary and invokes B1F850. The raw viewport origin/dimensions/depth setters, B659D0, full raw B700E0 pose closure and raw node cleanup are reused. Fog+184 is zeroed; this constructor does not invoke B84E50 or B71940 and makes no claim about their separately audited raw APIs. The pool remains the existing actual45Ch physical camera pool with its +458 slot index.

## Scratch, current reads and cleanup

For native entry ESP=E, the six live scratch words occupy E-24..E-10. Origin and dimensions reuse the first two; pose later overwrites all six with target{0,hundred,hundred}, eye{0,hundred,0}, using one current CE3D08 capture. Two mutable pushed argument cells occupy E-40 and E-3C. The latter is reused for base name, allocation size, viewport setters and pose target; the former is pose eye. Pose entry is E-44 and its nested views retain the previous pose packet's physical overlays. Metadata stays disjoint from the one live DWORD backing. Native saved-register, return, EH and provider-private gaps are excluded.

The incoming name pointer is captured before B6F5A0. The **same original incoming word** becomes the viewport allocation pointer after BF681B, then later color scratch: byte2, byte1, byte0 are cleared before current CE3C88 is read; byte3 is cleared only after the intervening camera stores; the current whole word is then loaded for +190. It is not replaced with a synthesized zero or argument snapshot.

After pose, current CE7D20, D0C5F8 and D7A24C are captured in native order, followed by interleaved stores/current D5BD98 and CE77FC reads. Depth arguments retain FLDZ/FLD1 and FSTP into their actual pushed cell. All raw stores preserve IEEE word bits, and later one reads remain distinct from those in node, viewport, plane and pose providers.

DFAA38 declares maxstate3, map DFAA20 entries {-1,CC1AD0}, {0,CC1AD8}, {0,CC1AE3}, no try blocks, flags1. State0 begins before camera stamp/allocation. State1 frees the **current original argument word** on viewport construction failure, then consumes base cleanup. After successful publication, state0 does not own viewport180. State2 consumes current438 cleanup through605FD0 with a fresh decrement import, then base cleanup. Both state0 and state2 failures leave the published viewport allocation unretired, matching the native prefix. Diagnostics retain its allocation/result and nested cleanup progress for explicit disposition; no rollback or extra decrement is invented. Cleanup consumes its state before each call and terminates if a second source C++ cleanup throws.

A null allocation is not silently accepted as success: native continues to unconditional viewport calls. The genuine allocation provider returns allocated storage or throws; invalid/unbound renderer and zero-viewport source checks are explicit boundaries. Native FH3/SEH, unmasked fault transport and private CRT exception identity are not reproduced by source C++ exceptions.

## Verification status

Strict Win32 build and all three existing CTests pass. Compiled B1FF60 has exactly the original seven bytes. Compiled constructor inspection retains FLDZ/FSTP/FLD1/FSTP, second profile resolution before first-result height, height before second slot load, and the byte2/1/0/current-scalar/byte3/current-whole-word schedule.

One ignored application fixture links all70 current production application objects other than its forwarding game_main, plus three current libraries; SHA receipts match the current CMake object set. Its main differs from the preserved shadow-provider scaffold in exactly two lines: the forwarding-header include and wrapper type. No production object is replaced. The strict fixture uses /MD, /fp:strict, /W4, /WX and /MANIFEST:EMBED, and is launched only with tools/run_game.ps1 and isolated settings.

The existing application bootstrap/diagnostic fixture runs the complete genuine B32410 normal renderer constructor over actual1D94h storage and creates a real D3D9 HAL device. The log reports windowed=0; this is **not a hidden-window run**. Its launcher still reports120 concrete and46 unimplemented host methods. Later B107F0/B13010 resource stages are unactivated. Thus this is reached-provider evidence, not complete native startup or gameplay.

Two original/source pairs execute the copied604B camera body,156B viewport body and7B getter, reusing genuine raw node/plane/pose/setter/allocator providers. Nine direct rel32 operands and six absolute data operands are relocated; all copied indirect instructions remain unchanged. The actual renderer's profile word temporarily names one of two fixture-owned copies of its table, with slot30 pointing to a bridge that executes genuine B1FF60. Source lookup maps those two declared table identities to the same original numeric target. The actual renderer profile and parameter width/height are restored before application execution resumes; no original mapped table or occupied mapping is overwritten.

The second pair instruments **after genuine getter calls**, changing the actual renderer profile, parameter values, one binding and original incoming allocation/color word. CE3C88 is explicitly bound to that same word: the final scalar observes AB000000 after bytes2/1/0 are cleared, before byte3 clears, while final color is zero. Height419 is captured before the fourth getter changes it to421; width733 is read afterward. Both alternate renderer tables retain the same genuine slot30 target, so this comparison does not independently establish different-target selection. Native/source and compiled read-order inspection provide that separate ordering evidence. The mutations are harness instrumentation, not native getter behavior.

The pairs compare the whole initialized camera payload (normalized actual identities), initialized viewport words, six still-live outer scratch words, original argument output, callback trace and masked x87 status. Each viewport's own unwritten21..33 bytes are captured during its first callback and checked after return; they are not compared across unrelated CRT allocations. Native saved/EH/private-stack bytes and dead nested scratch are excluded. Original pose execution delegates to the already-verified raw pose provider on a separate live fixture stack, preserving the outer physical pointer views.

Every successful camera is admitted through the existing canonical NativeCameraStorageReference over the **same actual atomic+4 and application's real45Ch pool slot**. A genuine Windows creator decrement reaches zero; the raw camera/viewport terminal and pool return complete, then the canonical identity is unbound. A single additional source-only injected failure after the third genuine getter verifies state2/current438/base cleanup and the surviving viewport credit. Its later viewport disposal and failed-slot return are explicit fixture actions, not rollback attributed to the constructor. State1 failure, native FH3/SEH, unmasked faults and arbitrary polymorphic callbacks remain unexercised.

The final run exits0, performs the ordinary reached application drain, joins the worker and ends with COM device/API counts0/0. No extra camera/node retain, fog construction, logical-camera conversion, broad test suite or production activation was added.
