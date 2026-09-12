# Raw mouse and joystick table tails

Addresses: 00A99F10, 00A99F20, 00A99F30, 00A99F40, 00A99F50, 00A99F60,
00A99710. Descriptive names are source hypotheses, not recovered symbols.

This supplemental packet implements the six mouse DWORD accessors and the raw
joystick label getter discovered while reviewing the finite input runtime.
All seven normal bodies and the joystick's documented C++ exception cleanup are
covered over valid actual storage. The implementations do not introduce a
typed device companion, shadow metadata array, index guard, runtime owner,
singleton domain, allocator or new native string implementation.

## Correct table extents and signatures

Fresh verified Ghidra byte reads show keyboard D5B904 ends at slot34 (38h bytes),
common D5B670 at38 (3Ch bytes), XInput D5BB48 and joystick D5B7F0 at3C (40h bytes),
and mouse D5B8B0 at50 (54h bytes). There are 82 slots total. Mouse slot38 remains
A9A380's ST0 float getter; slots3C..50 are DWORD accessors. XInput slot3C is the
existing raw A9AA40 control-name getter and is outside this packet.

| Entry / mouse slot | Body | Inclusive end | Coverage |
|---|---|---|---|
| A99F10 /3C | EAX = DWORD[ECX+228]; RET | A99F16 | Complete aligned bytes; missing Ghidra start |
| A99F20 /40 | EAX = DWORD[ECX+22C]; RET | A99F26 | Complete aligned bytes; missing Ghidra start |
| A99F30 /44 | EAX = DWORD[ECX+230]; RET | A99F36 | Complete aligned bytes; missing Ghidra start |
| A99F40 /48 | DWORD[ECX+228] = stack DWORD; RET4 | A99F4C | Complete aligned bytes; missing Ghidra start |
| A99F50 /4C | DWORD[ECX+22C] = stack DWORD; RET4 | A99F5C | Complete aligned bytes; missing Ghidra start |
| A99F60 /50 | DWORD[ECX+230] = stack DWORD; RET4 | A99F6C | Complete aligned bytes; missing Ghidra start |
| A99710 /joystick3C | ECX actualB48h; output/code stack; EAX output; RET8 | A998F1 | Complete source body and recovered unwind actions |

The getter returns all DWORD bits; the setter consumes all DWORD bits without a
Boolean/float conversion. A9A290 initializes +228/+22C/+230 and A9A180 adds its
sample x/y/z words into them, including retained sample fields after SDK failure.
Setters have no semantic result; the native EAX still contains their argument.
EDX is not an input to any of these seven entries. New C++ APIs do not claim
native register upper bits, stack/FH3/SEH compatibility or hardware-fault behavior.

The six mouse starts have no current Ghidra functions. Their final RETs are at
A99F16/26/36 (one byte) and A99F4A/5A/6A (three bytes). The next bytes at
A99F17/27/37/4D/5D/6D are CC padding, not omitted tails. Their only observed data
xrefs are slots D5B8EC/F0/F4/F8/FC/D5B900. The next table starts at D5B904.
No containing-function membership is claimed for those aligned disk decodes.

## Joystick layout, reloads and strings

A99940 produces the actualB48h receiver. Its +290 points to 1Ch metadata records
whose leading eight bytes are the canonical native string header. Bindings at
+29C are 0Ch records containing kind, metadata index and a relative byte. The
same actual records are read directly; code/index arithmetic is not clamped.

A99710 constructs the output empty using CE3A0C before reading the binding.
Kind0 returns the constructed empty result. Otherwise it selects metadata by
the current index and copies its name with the native self-copy guard. The
existing actual-header copy primitive preserves source/destination reloads after
resize callbacks. An output alias of the selected metadata name is allowed:
the initial empty constructor overwrites that same header before the self-copy
guard. It deliberately does not free an earlier live output buffer.

The kind is reloaded from the original binding after copying. Kinds3/5 append
`/Left`, 4/6 `/Right`, 7 `/Up`, 8 `/Down`; other kinds keep the name. The literals
match D5B830/D5B838/D5B848/D5B840. Left captures temporary length and data before
resizing the result, uses that captured data for memcpy and the normal release.
The other suffix arms call existing 00425E10, which reloads both current data
pointers after resizing. This distinction is retained without copying headers.

All dependencies are existing source: actual-header constructor0041E870 in
`native_physical_file_date`, resize0041DD40/copy/destructor0041DD20 in
`native_string`, append00425E10 in `native_string_append`, and real CRT memcpy.
`NativeStringStorage` supplies the already established 00419CC0/BD1510 storage
boundary, including the explicit sized release; this function creates no pool
or fallback allocator. Native allocator/return services are caller-composed.

## Exception evidence

Handler CB6AA9 selects FuncInfo DECEEC (magic19930522, maxState5) and unwind map
DECF10. State0 -> -1 executes CB6A70: test/clear constructed-result bit and tail
0041DD20 on the caller's output. States1..4 ->0 execute CB6AA1/CB6A99/CB6A89/
CB6A91 on the actual suffix header, then run the result cleanup. The output
cleanup is armed only after successful empty construction. A failed suffix
constructor has not entered its suffix state. The new guards preserve those
actions: suffix before output, freeing storage without clearing the output
header. A failed result must not be destroyed again. No native compiler frame
layout or arbitrary saved-stack alias mutation is claimed.

## Validation and review boundary

Read-only batches verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. A99710 was refreshed and every direct CALL and the
five unwind tail JMPs are recorded with exact sites/containing functions in the
report. Recognized CRT and existing string services retain their names/contracts.
The six mouse leaves have no calls. Existing typed joystick labels are preserved
by adding a raw reconstruction fragment, not replacing their record.

Win32 Release build passed, all eight native seeds matched, and both existing
CTests passed. One ignored manifested probe used actualB48h field offsets,
actual1Ch metadata and existing string helpers. A fixture allocation callback
changed binding kind3 to7 during name copy; the result was `Axis/Up`, proving
the reload. A later forced result-resize exception released the suffix6 block
then output5 block, retained the failed output header and ended with zero live
fixture blocks. Three mouse raw-bit round trips passed. The probe supplies
controlled storage callbacks; it does not establish hardware or application
dispatch, constructor execution, original ABI or gameplay behavior.

The prior read-only runtime review is saved in ignored
`local/native_input_runtime_review_ae.md`: its 74 existing mappings and captured
profile routes matched; these eight extra table slots were the material omission.
Primary runtime files and all Ghidra metadata were left untouched by this worker.
