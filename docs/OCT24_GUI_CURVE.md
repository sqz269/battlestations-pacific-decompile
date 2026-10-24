# Raw Curve storage and lifetime prerequisite

This packet supplies actual Curve allocation, construction, point methods,
loaded ordering and retirement over one genuine 140h object. It creates no
page, layout, `GuiWidgetOwner`, second collection, Model, process manager or
render application. Its source AA1800 arms are not retained-factory admission.

Evidence is the saved exports and narrowly inspected live EH bytes in
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` (77629 functions).
Names are descriptive hypotheses. The new C++ interfaces are not native
register/stack/FH3/SEH/fault adapters; there is no game/runtime validation.
Locked annotation preserved prior names/comments and saved the project;
the prior-value record is `local/ghidra-annotations-20261024T123409Z.json`.
Affected exports were refreshed. Existing whole-function records and library
names remain; the eight added records qualify this new raw-source interface.

| Original entry | Provided actual-storage contract |
| --- | --- |
| AA1800 / ABC790 | ECX optional captured source; request140h through the existing F8BE5C allocator. Dispatch ABE030 or ABE100, EAX actual/null, RET. Failed construction returns the captured slot after constructor cleanup. |
| ABE030 | ECX actual, EAX same, RET. Genuine AA9390(type13), D5C7F8 and exact defaults. |
| ABE100 | ECX destination, stack source, EAX same, RET4. Genuine AA9520 then **exact default derived fields**. No points, widths, texture, UV or material copy. |
| ABC920 | ECX actual, RET. If points capacity114 is negative, reserve0; decrement positive110 one at a time, then zero110; resize widthsF8 to0. Retain backing/capacity. |
| ABC970 | ECX actual, stack pair pointer/float width, RET8. Equality-triggered signed doubling/clamp1 for counted points then widths. Late pair x87 FLD/FSTP, point-count increment, late width-bit store, width-count increment. |
| ABC040 | ECX actual, RET. Required genuine AA7170 completes first, then direct AA6A30(1). No geometry or material creation is implied. |
| ABDE90 | ECX actual, RET. D5C7F8; captured material release; authored-vector tidy; counted points, pooled texture string, widths, control points; genuine AA9730 last. |
| ABE010 | ECX actual, stack flags, EAX saved identity, RET4. Complete destructor first; read current flags low byte afterward; ABC410 return iff bit0. No count decrement/test. |

`GuiCurvePoolTraits` already supplies the genuine F8BE5C pool, D5C7CC pool
profile, 144h slots/140h payload, 64 slots per slab, ABC790/ABC650 allocation
and ABC410 return. Existing process initialization remains explicit. DWORD140
is the pool's slab index, not extra Curve storage.

## Same raw arrays and constructor writes

| Object offset | Actual contents |
| --- | --- |
| EC/F0/F4 | Counted control-point array: data pointer, signed count, signed capacity; two-float elements. |
| F8/FC/100 | Counted float-width array, same three-word header. |
| 104/108 | Actual eight-byte pooled texture string, length/data. |
| 10C/110/114 | Counted point array, same header and two-float elements. |
| 118 | Constructor-zeroed DWORD; its meaning is not asserted here. |
| 11C..128 | Four raw UV float fields, default0,0,current D7A24C,current captured D7A24C. |
| 12C | Actual material identity with the same canonical atomic+04. |
| 130/134/138/13C | STL vector proxy/first/last/end for authored pairs. **130 is unwritten by both constructors.** |

ABE058..ABE09A and ABE12B..ABE16D zero EC through114;118 is also zeroed.
The single D7A24C read at ABE0B7/ABE18A is published to both124 and128.
Both constructors zero12C/134/138/13C and preserve130, base unwritten bytes,
and the pool trailer. ABE100's default derived state is native behavior, not a
copy repair. AA9520's genuine Model clone and its acquisition/failure frame are
reused; no derived acquisition can throw after that base returns normally.

The projected `gui_render.cpp` Curve mesh and host property/geometry routines
do not own these raw arrays. The Line's `{proxy,first,last,end}` vector transport
cannot operate on Curve's counted arrays. No projected vector, extra points
owner, raw/`GuiWidgetOwner` cast or copied base state is introduced.

ABC970 assembly at ABC9AC..ABC9B3 uses x87 loads/stores for each point float,
including current floating exceptions and signaling-NaN conversion. It reads
the source only after point reserve; it does not snapshot aliases before old
backing may be freed. ABC9EE uses MOVSS for the width after width reserve.
The explicit width-slot interface preserves those late unchanged DWORD bits.
Doubling uses 32-bit wrapping bits followed by the native signed comparison.
Callers provide valid live source/ranges and native lifetime synchronization;
the implementation adds no alias/range repair or count normalization.

Existing recovered `float_pair_array_reserve_0086a350` is reused through a
private Front6 adapter whose only reached operations are canonical
`singleton_lifetime_allocate/free`. Unrelated wide-host entries reject if
reached. This preserves its native pair-copy order and current old-backing
reread. Existing `reserve_native_float_array_008153e0` and
`resize_native_float_array_00818030` supply widths. The CRT allocation/free and
0041F6B0 vector tidy are qualified stock bindings/substitution, with no new
vendor-library or game-body reconstruction credit.

## Canonical material, cleanup and terminal custody

ABDEB6 captures material12C once. ABDECE reads the caller-supplied current
CE2220 import and decrements that actual+04 once. On zero, ABDEDE's current
virtual0 is delivered through the same canonical actual-owner registry;
there is no second decrement, count, owner or material destructor surrogate.
Only after the terminal returns is raw12C cleared. Callback changes to other
Curve fields remain visible to subsequent current reads.

Normal cleanup frees authored first134 then clears134/138/13C (preserves130).
Counted arrays clear their count then free the current data without clearing
the pointer/capacity. The pooled string destructor leaves its header as native
0041DD20 does. The C++ source does not sanitize dangling terminal fields.

EHHandler CB835E loads FuncInfo DEF1D0. Its six-entry unwind map at DEF1F4 has
state5->4/CB8350 (authored130 ->0041F6B0), state4->3/CB8342 (points10C
->0086AED0), state3->2/CB8334 (string104 ->0041DD20), state2->1/CB8326
(widthsF8 ->0081B610), state1->0/CB8318 (controlEC ->0086AED0), and
state0->-1/CB8310 (base ->AA9730). Normal destructor states become3 before
points,2 before string,1 before widths,0 before control and-1 before base.
Thus failure in a reached member destructor never retries that same member.
The C++ cleanup projects this order and terminates on a second unwind throw.
The constructor handlers CB83B0/CB8400 have matching member maps, but their
normal derived initialization makes no throwing calls after AA9390/AA9520.
Native binary FH3/SEH and fault transport remain unclaimed.

Every point/load/terminal operation requires a fresh external diagnostic
record. Entered failures retain reached effects, original failure site/state,
raw storage/providers and that record; no rollback, retry or slot return is
asserted. Destroying entered/failed diagnostics terminates. A completed raw
Curve can have exactly one `NativeGuiCurveReference` in the SAME actual-owner
registry, borrowing its existing positive atomic+04. Its current virtual0
BD30E0 reaches current04 ABE010(flags1). Direct scalar entry does not decrement
or test the count. Only successful terminal return unbinds the opaque raw key;
failed terminal custody remains bound and forbids slot reuse/dependency drain.

## Remaining delivery boundary

ABC040's AA7170 provider must perform the genuine current+60(0) and raw bounds
refresh on this same widget. Existing raw load-hook code requires a qualified
real table route; no numeric-address call, empty successful callback or
projected owner is supplied. ABE1D0 property loading, actual74 ABCC70 geometry,
7C ABCA10 rebuild/ABCED0 tessellation, ABDD80 material/texture binding and their
same-renderer HAL/resources remain required future services. The typed
`std::vector` mesh helpers alone do not establish these contracts. No page,
manager, retained factory, rendering, input or all18 delivery is claimed.

Validation: `scripts/build.ps1` compiled both new translation units in Win32
Release; existing `reconstructed_math` and `tool_tests` passed2/2 (13.77s).
Private registration was removed with byte-exact `cmake/startup.cmake`
restoration, SHA256 `8B50225902E1D70B21BE625D0596B00A17483FA681D8020F1F7A3B889F0C1098`.
No permanent CMake change, new test or game run belongs to this packet.
