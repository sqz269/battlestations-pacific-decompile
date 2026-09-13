# Native keyboard binding application

Addresses: 006AA640, 006AA090, 00699BF0, 0069E860, 0069DD80

`native_input_keyboard_apply.cpp` reconstructs the complete normal schedule of
006AA640 over the actual 540h settings allocation. `NativeInputKeyboardApplication`
connects it to the existing raw default-binding preservation routine 006AB820.
It uses the actual input owner, binding storage, slot installer and whole-action
rebinding. The required `NativeInputKeyboardLibrary` operates on native tree,
vector and iterator addresses. It has no default provider. The settings
constructor/loader and complete production library bindings remain dependencies.

These interfaces are new C++ adapters, not drop-in original ABI replacements.
Names describe behavior and remain hypotheses, not recovered symbols. Existing
typed reconstructions in `keyboard_restore.cpp` and `keyboard_axis_pairs.cpp`
remain separately recorded; this packet adds raw-storage reconstruction records.

## Evidence and ABI

The original image and saved `C:/Users/sqz269/bsp.gpr` program
`/battlestationspacific.exe` agree on all captured bytes. The report records
52 native spans and 68 audited CALL rows for the engine, predicates and existing
A93750/A92260 dependencies. Assembly supplies the register/stack contracts and
floating-point instructions; pseudocode alone loses these details.

| Entry | Captured bytes | Original contract |
| --- | ---: | --- |
| 006AA640 | 1540 | ECX settings, no stack arguments, RET; through 006AAC43 |
| 006AA090 | 304 | ECX settings; stack device/input pooled-string pointers; AL, RET8 |
| 00699BF0 | 33 | ECX descriptor14h; canonical EAX boolean, RET |
| 0069E860 | 150 | ECX/EDX checked DWORD-vector headers; canonical EAX boolean, RET |
| 0069DD80 | 94 | Cdecl: first begin/end, second owner/begin; two unused trailing words; AL |

0069DD80 retains its existing library name. Its consumed range contract is
represented by a source adapter, with no new claim to a complete STL implementation.
The 006AA640 body ownership repair was already published in the AM packet; this
packet needs no new body definition. Two returning-free library continuations
are captured from bounded, live-verified disk spans for the fixture only. They
are listed separately in `library_disk_extensions`; saved library metadata and
CRT no-return flags are unchanged.

## Raw records and schedule

The byte50 DEVINPUTS gate, byte5 loaded gate and registered action128h gate precede
all tree work. Each device node carries its pooled key at +C and actual 84h
record at +14. Native tree headers contain opaque/head/count DWORDs. Device
fields consumed here are input descriptions +0, runtime bindings +C, sensitivity
descriptions +18, runtime multipliers +24, base scales +50, AxisPairs +5C,
reverse bits +6C and Min1SensHacks +78. These are consumer contracts; the fixture
constructs valid raw records explicitly and does not establish a complete
settings producer.

Per device, 0055B400 copies the nested signed-code/class scale map. Each sensitivity
description supplies a checked code vector and category. Categories 0, 1/3 and
2/4 select classes 0, 1 and 2. Other values retain the previous class, including
across devices; an explicit context input represents the initial stack preimage.
Inner signed-key/float subscripting reuses the existing deadline-map storage
adapter: 006A1560 has the same schedule as 004D6900 except its hint callee.

Ordered zero becomes the live one constant. COMISS performs the multiplier
threshold tests; NaN takes the hack lookup path. A code in Min1SensHacks protects
its scale from this small/unordered multiplier. Actual multiplication uses x87
FLD/FMUL/FSTP, including the original float spill. Sensitivity descriptions use
the native iterator with nil byte29.

Each input supplies exactly two binding descriptors and its ordered action-code
vector. The axis predicate compares an AxisPairs row's second name by length and
case-insensitive text, then looks up SECOND codes before FIRST codes. Matching
ordered vectors select slots2/3. The checked range comparator uses integer DWORD
equality. A dead native CMP EBP,EBP validation branch remains documented rather
than becoming an extra source condition.

For each action code, the current descriptor class selects its scale; an ordered
zero is replaced only in the local scale. Reverse-bit lookup uses the actual
owner/word/bit iterator and live vector bounds. A set bit multiplies by the live
double sign constant through x87. Ordinary slots spill through x87. Alternate
slots use SSE SUBSS of live negative zero minus the scale. Class1/code>=8 or
class2/code>=60 suppresses an alternate slot; these comparisons are unsigned,
so negative code bit patterns qualify. Suppression installs {-1,0,0,-1,flags}
and positive zero. The high24 flag bits come from an explicit native stack
preimage, while its low byte is cleared. All installations invoke the existing
actual slot installer and rebind the whole action against current backend lists.

## Cleanup and limits

The native FH3 descriptor DACEF8 has one state, whose unwind action C7F9F0 calls
0055B490 on the temporary map. Source ownership arms after copy returns. Normal
cleanup disarms before full-range 0055B230, frees the reloaded head, then clears
head/count and advances the device iterator. A supported C++ exception while
armed calls the required map destructor; a failure during normal erase is not
retried. Original FH3 execution, hardware faults, arbitrary mutation of native
private stack aliases and binary ABI compatibility are outside this source API.

## Validation

One ignored, manifested Win32 fixture executes copied original instructions in
an isolated process and compares the source against them. Both use valid raw
settings records, original library bodies, actual source owner/storage/rebind
boundaries and current CRT allocation. Missing/error library paths terminate the
probe; they are not successful stubs. Library production is therefore still open.

The fixture compares 372 words across finite, quiet-NaN and retained-category
scenarios, including the effective temporary scale trees before erase and all
resulting binding slots. It exercises all three early gates, two devices, reverse
bits, inner class insertion, alternate suppression and 30 native resizes/rebinds.
Only the low descriptor flag byte and defined binding bytes are compared where
native private stack/allocation preimages are unspecified. Original allocation
and free counters balance; source allocations cross separate CRT boundaries and
are not claimed covered by that counter. Backend pointer tokens are copied but
never polled or dereferenced as devices. The game was not launched or controlled.

Strict MSVC Win32 compilation and both existing CTests pass. The accompanying
report pins the final combined source/archive, fixture hashes, CALL audit and
eight disk/live seed checks. This is component evidence, not gameplay validation
or a complete input startup/runtime path.
