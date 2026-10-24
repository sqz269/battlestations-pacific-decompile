# Raw GUI Model widget storage prerequisite

This packet supplies actual class9 storage, default allocation, placement
copy, destruction/scalar and one canonical reference over its existing+04.
The widget is **11Ch**, with a120h F8C09C pool slot. It is distinct from the
renderer184h Model at widget4C. No renderer Model, wrapper, page, layout, tree,
transform or process owner is duplicated. Nothing is admitted to a retained
factory or activated in a page/application/manager.

Evidence is saved exports and narrow live EH-map queries in
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` (77629 functions).
Names are descriptive hypotheses. New source interfaces are not native
register/stack/FH3/SEH/fault adapters and have no game/runtime validation.
Locked annotation preserved prior names/comments and saved the project;
the prior-value record is `local/ghidra-annotations-20261024T124847Z.json`.
Six affected exports were refreshed. Five bounded raw-source records were
appended while preserving existing whole-function records and library names.

| Original entry | Provided actual-storage contract |
| --- | --- |
| AA16E0 default | ECX null source, EAX actual/null, RET. Request11Ch through existing ACC490/ACC350. Complete ACC740 default arm; ACC1C0 failed-constructor slot return. **Copy factory is not exposed.** |
| ACC490 | Existing allocator thunk: ECX F8C09C, tail ACC350. Same GuiModelPoolTraits, D5CEF0 pool profile, 120h slots/11Ch payload,8 slots per slab. |
| ACC740 | ECX actual, EAX same, RET. Genuine raw AA9390(type9), zeroEC/F0, stampD5CF38, zero10C/110/114/118. Preserve six float wordsF4..108 and pool trailer11C. |
| ACCF70 | ECX destination, stack source, EAX same, RET4. Genuine AA9520, stampD5CF38, zeroEC/F0, x87-copyF4..108, copy string10C then114, mandatory real ACCCF0 over the same destination and its own headers. |
| ACC7B0 | ECX actual, RET. StampD5CF38; capture wrapperF0, current-import actual+04 decrement/current zero, clearF0 after return; destroy114 then10C then genuine AA9730. EC is not cleared. |
| ACC880 | ECX actual, stack flags, EAX saved identity, RET4. Complete ACC7B0, read CURRENT flags low byte afterward, same ACC050 slot return iff bit0. No count decrement/test. |

## Actual tail and source ordering

| Offset | Actual storage |
| --- | --- |
| EC | Borrowed actual root, obtained from current generated wrapperF0+0C. |
| F0 | Actual generated resource-wrapper identity/ref-counted creator. |
| F4/F8/FC | Three ScaleVector float fields. |
| 100/104/108 | Three RotationEuler float fields. |
| 10C/110 | ModelName native eight-byte length/data header. |
| 114/118 | ModelTextureOverride native eight-byte length/data header. |

ACC740 never initializes the six float fields. Stale raw pool bytes are not
replaced with identity/default scale or rotation. AA9390's exact raw base and
unwritten bytes are retained. The trailing11C DWORD is pool metadata, not an
enlarged widget field. The same initialized GuiModelPoolTraits process must
precede default allocation; no private pool or implicit process startup exists.

ACCF70 first completes the existing real AA9520 raw base/Model clone. It
clearsEC/F0 rather than retaining/copying the source wrapper. ACCFAE..ACD002
copies the six floats via x87 FLD/FSTP in ascending order. ACCFD4 loadsFC,
ACCFDA arms native state0, ACCFDE storesFC; the source helper preserves that
floating/diagnostic-state ordering. Native floating exceptions and signaling
NaN conversion are not replaced by SSE or packed state copies. Native SEH/FH3
transport remains outside the C++ interface.

The two inline string constructors have the existing00426060 raw-header
algorithm: zero both destination words, compare identity, resize to captured
source length with preserve1, guard on CURRENT source length, then copy the
CURRENT destination count and source/destination data. The existing genuine
raw-pool variant is reused, including its qualified overlap/zero-byte-copy
policy, without a duplicate string owner or new vendor-body credit. Name
completion arms state1 before override initialization; override completion
arms state2 before ACCCF0. Both arguments passed to the required loader are
the widget's exact10C and114 headers, not converted strings.

## Genuine loader and factory failure boundary

`NativeGuiModelLoadCalls` is an explicit mandatory service, with no default
implementation. Its ACCCF0 must perform the actual raw ACCB90 default-factory
load, source transforms, material/texture/clip operations and release schedule
over this widget and the SAME renderer/resource domains. Returning without
those effects is not a valid provider. Unavailable providers remain rejected;
the projected `GuiModelLoadHost` body is not a raw activation binding.

ACCB90 reaches the actual004C1400 resource manager/B80D70 default factory and
current resource+08(6,1.0). It publishes the generated wrapper atF0, releases
the resource creator, then reads CURRENT F0+0C toEC. Nonnull roots are parented
to the widget's SAME actual4C Model and receive ACC8A0 material parameters.
ACCCF0 additionally uses ACBEC0/current root+38 transforms, current generated
geometry/sections, the actual F8D394 texture-manager+64 result, B189F0 and
AA9F10 raw parameters and actual texture-reference retirement. No synthetic
root, substitute null-success loader, copied material parameters or second
wrapper/Model/transform/count is supplied. Missing/empty resource handling is
not repaired by this storage packet.

Copy handler CB8ED4 loads FuncInfo DF01E0, whose map DF01C8 is:
state2->1/CB8EC6 (string114 ->0041DD20), state1->0/CB8EB8 (string10C
->0041DD20), state0->-1/CB8EB0 (base ->AA9730). It contains **no wrapperF0
cleanup**. A failure in a string being constructed cleans only previously
completed members; a loader throw cleans both strings then base. A second
cleanup throw terminates. The implementation neither retries nor invents a
wrapper release or full recovery of partially acquired resources.

Original AA16E0 handler CB6ED0/FuncInfo DED4D8/mapDED4C8 has two state->-1
funclets CB6EC0/CB6EC8, both tail ACC1C0 to return the captured slot following
constructor failure. A partial ACCCF0 provider can already have published a
wrapper or material parameters borrowing this widget before interruption.
There is no genuine provider for that partial-load/factory failure custody
here. Consequently only the **default factory arm** is exposed; placement
copy retains its failed raw slot, real loader obligations, contexts and
diagnostics with the caller's sole existing owner. This is an explicit source
boundary, not a replacement exception policy for native AA16E0 or a claim of
complete copy-factory delivery.

## Same wrapper and widget terminal lifetime

ACC7D4 capturesF0 once; ACC7EA reads the caller's CURRENT CE2220 import and
decrements that actual wrapper+04 once. Zero ACC7FA dispatches the same
canonical actual-owner companion/current virtual0, without another decrement.
Only after it returns is the CURRENT rawF0 slot zeroed. EC is deliberately
left as the source leaves it. Header destructors also leave their native
length/data words untouched after pool return; no sanitizing writes occur.

Destructor handler CB8E74 loads FuncInfo DF0160/mapDF0148:
state2->1/CB8E66 (string114), state1->0/CB8E58 (string10C), state0->-1/CB8E50
(base). Normal state1 precedes override destruction,0 precedes name
destruction,-1 precedes AA9730. Thus the current member is not retried if its
destructor throws. C++ unwind follows the saved map; native binary FH3/SEH
and fault behavior are not claimed.

`NativeGuiModelWidgetReference` borrows the widget's constructed positive
atomic+04 and registers once in the SAME canonical actual-owner registry.
It is not `NativeModelReference`, and creates no native count, page token,
wrapper, renderer Model or tree. Current virtual0 BD30E0 reaches current04
ACC880(flags1). Explicit scalar entry performs no count check/decrement. Only
successful terminal return unbinds the opaque raw key, after pool effects;
there is no post-return raw read. Entered failures preserve the canonical
binding, raw storage/providers and diagnostic record and forbid retry, reuse
or dependency drain. Destroying entered/failed diagnostics or a live/failed
companion terminates. Serialized lifetime/slot synchronization remains the
caller's native obligation.

Remaining delivery needs a genuine raw loader and its partial-failure custody,
current20 ACC6A0 dispatch, property reader ACCE20/current callbacks, and the
actual same-renderer material/resource contracts above. No page, manager,
retained factory, GUI Model rendering, input or all18 delivery is claimed.

Validation: `scripts/build.ps1` compiled both new translation units in Win32
Release on the first pass. Existing `reconstructed_math` and `tool_tests`
passed2/2 (12.23s). Private `cmake/startup.cmake` registration was restored
byte-for-byte, SHA256 `8B50225902E1D70B21BE625D0596B00A17483FA681D8020F1F7A3B889F0C1098`.
No new tests, games or permanent CMake edits belong to this packet.
