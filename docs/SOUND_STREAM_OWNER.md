# Actual stream audio ownership

Addresses: `00A877D0`, `00A87390`, `00A87B30`, `00A79150`, `00A86540`,
`00A868B0`, `00A86990`. Evidence: existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified by every live CLI batch. Ghidra was read
only. Names are descriptive hypotheses; `cStreamAudio::~cStreamAudio()` is the
literal diagnostic at D5B340. Source is `src/sound_stream_owner.cpp`.

| Routine | Original ABI | Coverage / end exclusive |
| --- | --- | --- |
| A877D0 constructor | ECX actual54h, stacked owned8h NativeString and retained4h table; EAX this; RET0C | Complete normal and EH ordering in stated host domain; A87B2F |
| A87390 destructor | ECX actual54h, RET | Complete normal and EH ordering; A874CE |
| A87B30 scalar delete | ECX actual54h, stacked flags low byte; EAX original pointer; RET4 | Complete; A87B4E |
| A79150 table constructor | ECX actual20h, stacked borrowed filename header; EAX this; RET4 | Complete normal and EH ordering; A791C7 |
| A86540 control reserve | ECX actual0Ch header, stacked signed capacity; RET4 | Complete; A865C2 |
| A868B0 control resize | ECX actual0Ch header, stacked signed count; RET4 | Complete; A86980 |
| A86990 control destruction | ECX actual0Ch header, RET | Complete; A869A7 |

## Storage and caller ownership

| Actual54h offset | Constructor stores / subsequent ownership |
| --- | --- |
| 0/4 | CEB130 then D5B360; intrusive count1 |
| 8..B | Four zero bytes |
| C / 10 / 14 | Float1 / **unwritten** / float1 |
| 18/1C/20 | Zero words; runtime positions and state |
| 24/2C/34 | Three actual8h pooled strings, initially empty |
| 3C | Initially null retained actual20h table |
| 40/44 | Zero handles; required runtime stop owns the FMOD operations |
| 48/4C/50 | Actual0Ch control vector: data/count/capacity, initially zero |

The table has base/refcount at0/4, a 14h-record vector at8/C/10, float1 at14,
total channels0 at18, and loop byte0 at1C. Padding1D..1F is preserved. A79150
calls the existing actual A87060 loader, which appends into those current fields.

A783F0 allocates54h at A7845F, retains logical+28 into the outgoing4h argument
at A7848B, then copy-constructs the outgoing8h string from logical+10 at A784A0.
The constructor call is A784AB. The caller releases the reference if that
string copy throws, and frees raw54h storage if the constructor throws. Once
A877D0 is entered, it owns both outgoing arguments. A877D0 copies the name into
+24 and +2C, calls mutable-name resolution on +2C, and eventually destroys the
incoming string and releases the incoming reference. The C++ API therefore
accepts `NativeString& owned_name, void* owned_table`; callers must not add an
extra copy, retain, or cleanup after entering it. The destroyed string header
retains its dangling bytes, as the native caller's discarded stack slot does.

The other constructor call sites A877D0 receives are 588528 in5884A0 and52EEB5
in52EE30: both build owned name arguments from current menu storage+40/+48 and
pass a null table. A79150 is also called at A7960E inA79480 with an allocated20h
owner and borrowed temporary filename. All callee stack cleanups were read.

## Constructor and vector behavior

The `.def` path is built even when a table was supplied. After resolution,
A87918 calls existing substring469840 with start0 and DWORD(length-4), then
A8792E calls existing concat4261A0 with `.def` (CF779C). The temporary result is
copied into the definition name; concatenation, substring, and extension are
released in that order. No extension validation or length clamp is added.
Normal extension cleanup uses the data captured immediately after resizing it,
with the current length; EH uses its current header instead.

For a supplied table, A879F0 publishes it to current+3C before incrementing its
actual+4 count, then releases the previously captured table. For a null argument,
A87A1E allocates20h and A87A3A constructs it. Only that raw allocation is deleted
if table construction throws. A completed table is published without another
retain. The constructor reloads table+0C to size its controls; this is the number
of table rows, not total channels+18. It then zeros the first two floats in each
control. Its next index uses `MOVZX EAX,CX` at A87AA8: counts above65535 can loop
without progress after wrap; the source retains this behavior.

A86540 clamps signed capacity to at least1, allocates `capacity*12` with DWORD
wrap, copies all three words forward, reloads current source/count, frees the
current old data, then publishes the new data/capacity. BF55BE tail-calls the
throwing BF681B allocator. Allocation failure leaves the vector unchanged.
A868B0 uses the native four-record unroll and signed/wrapped comparisons. Each
record reloads the current data pointer, conditionally writes floats1 at0/4,
and leaves word8 untouched. Shrinking decrements count without element cleanup.
A86990 resizes0 and frees current data, retaining dangling data/capacity bytes.

## Native EH and destruction

A877D0 handler CB61EA uses FuncInfo DEC2E0 and unwind map DEC304. Its states are:

| State | Cleanup | Next |
| --- | --- | --- |
| 0 | Incoming retained reference via52E020 | -1 |
| 1 | Incoming name via41DD20 | 0 |
| 2 | Root vtable viaBD30F0 | 1 |
| 3/4/5 | Current strings24/2C/34 via41DD20 | Previous |
| 6 | Current table reference3C via52E020 | 5 |
| 7 | Current vector48 viaA86990 | 6 |
| 8 | Definition-name temporary | 7 |
| 9 | Extension temporary | 8 |
| 10 | Substring temporary | 9 |
| 11 | Concatenation temporary | 10 |
| 12 | Raw partial20h allocation viaBF65AC | 8 |

No constructor cleanup invokes stream stop. A failed constructor does not own
a completed object's teardown contract. After successful member construction,
normal code drops to state0 before consuming the name and state-1 before
releasing the incoming table. If that final zero-reference callback throws,
native EH leaves the completed members live; the source preserves the state.

A79150 handler CB4EF3 uses FuncInfo DEADE4/map DEADD4: state1 destroys current
vector8 throughA790F0, then state0 resets the base throughBD30F0. Existing
A791D0 is exactly that sequence, so it is reused for this cleanup; its source
has no derived-vtable write. Caller A877D0 then frees the partial table block.
Existing A87060's documented post-open scanner leak and reserve's partially
copied prefix leak remain inherited native behavior, not host RAII additions.

A87390 first installsD5B360, constructs a18h diagnostic builder426500, appends
D5B340 throughBD1A60 and current+24 throughBD1A20, then destroys the builder.
There is no diagnostic sink or callback in these builder bodies. It calls
required stopA86BF0, explicitly resizes controls0, then destroys them (another
resize0 plus free), releases current table3C, releases strings34/2C/24, and
setsCEB130. Member field reads occur after prior release callbacks. Its handler
CB6127 uses FuncInfo DEC240/map DEC264: states0..5 are root, strings24/2C/34,
table3C, vector48; state6 adds the diagnostic builder. A throwing stop therefore
still unwinds current members. Native teardown steps are disarmed before calls.
Secondary exceptions during this C++ unwind terminate.

D5B360 slot0 isBD30E0, which invokes current slot4 with flag1; slot4 isA87B30.
Scalar deletion always destructs, frees only when flags&1, and returns the
original address. It does not free storage if destruction throws. Retained
tables use their current vtable after InterlockedDecrement; D58F80 routes to
the existing concrete A796F0. Other vtables require the bound zero-reference
service, with no fallback or invented delete function.

## Required services and evidence limits

`SoundStreamOwnerContext` borrows the existing dialog context and its sole
string allocator, the actual-reference dispatch service, and a required host.
Host BDF4C0 normalizes and resolves an actual mutable string using the live VFS;
its Boolean result is deliberately ignored. Host A86BF0 must supply the runtime
body: state1 polls open state, state2 captures position, stops current channel,
releases current sound, clears handles, and then resets native state/flags.
It is an explicit application binding to the independently recovered runtime,
not an FMOD stub. Diagnostic, string, table, allocator, and reference operations
reuse existing concrete bodies.

These are MSVC Win32 C++ interfaces over actual storage, not injected original
thiscall entry points or native SEH frames. The allocator is the existing
malloc/new-handler throwing domain; the unreachable native null-new branches
do not become successful null constructions. Valid storage and counts must
denote owned accessible memory. Integer/address wraps and malformed no-progress
behavior are retained, but arbitrary invalid memory access is not made safe.
The scanner remains the existing token/stream value projection and retains its
documented native-malformed-input and exception boundaries.

Ghidra's false no-return truncation omitted A87390 afterA8741A and A86990 after
A869A1. Disk bytes establish RET atA874CD(length1) andA869A6(length1). Interior
gaps A865B3..A865BC andA87B45..A87B47 were also read from disk: they restore the
stack and publish vector data/capacity. Report distinguishes these tail sites
and EH funclets from ordinary calls inside currently registered bodies. No
no-return flag, function body, prototype, or annotation was mutated here.

Validation results and the focused local probe are recorded in
`reports/sound_stream_owner.json`. Build/fixture evidence does not establish
installed FMOD playback, binary ABI compatibility, or game validation.

The MSVC Win32 Release build passed with `MSBUILDDISABLENODEREUSE=1`, after all
eight seed byte comparisons matched. Both existing math CTests passed. The
single ignored `local/stream_owner_probe.cpp`, compiled with `/W4 /WX /MD` and
`/MANIFEST:EMBED`, passed eight consecutive constructor string-allocation
failure points, a throwing stop's member unwind, a null-table scanner creation
failure, and untouched-field/forward-vector-copy checks. These are local host
fixture checks; the scanner and stop failure injection are deliberate probe
dependencies, not production bindings. Report-call verification checked44
direct rows with zero failures; indirect calls and raw tail/EH rows remain
explicitly identified for primary integration.
