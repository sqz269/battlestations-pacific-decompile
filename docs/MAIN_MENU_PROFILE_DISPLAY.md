# Main-menu mission medal and localized date display

Addresses:00594B60,009052D0,0043BC30,00851E90,00851EC0,00851F50,00851FA0.

These concrete normal bodies use the existing mission-score map, screen2F8
widget/actual Icon companion, language catalog and NativeString storage. The
menu-service integrator must bind their current owners; this packet creates no
profile, selection state, replacement widget tree or copied completion map.
Names are reconstruction hypotheses. Evidence was read through the wrappers
which verify `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` before each
live batch. Ghidra was not mutated or saved by this worker.

| Routine | Original ABI/body | Coverage |
| --- | --- | --- |
|594B60|ECX screen, RET at594BE4;594B60..594BE4|Complete normal caller over existing semantic score storage and actual Icon|
|9052D0|ECX record,EAX0/1,RET;9052D0..9052D7|Complete predicate on represented record+00|
|43BC30|ECX fresh outputNativeString,EDX date60,stack date64/date68,EAX output,RET8 at43BDE3;43BC30..43BDE5|Complete normal builder sequence and all language arms|
|851E90|ECX16h builder,capacity stack,EAX builder,RET4 at851EBB;851E90..851EBD|Complete normal actual16h storage constructor|
|851EC0|ECX builder,RET at851ED0|Complete normal actual16h destructor; Ghidra currently truncates its body at851ECE|
|851F50|ECX builder,C-string stack,EAX builder,RET4 at851F9A;851F50..851F9C|Complete fixed-capacity string append|
|851FA0|ECX builder,signed DWORD stack,EAX builder,RET4 at852002;851FA0..852004|Complete integer append using native32-byte decimal temporary|

## Medal storage and sequencing

594B68 captures selected mission5806A0 before594B6D/73 reloads the current
game+6B4 mission-progress pointer. This is a distinct required association;
594B60 does not read the ProfileResetState at game+650. The established
`mission_record_00594a70` mutates that same case-insensitive score map. It must
perform operator[] insertion for an absent key. Native594A70's lower-bound,
less-than gate, default construction91D620, insert594860 and return node+14
confirm this; a read-only completion lookup would omit an observable mutation.
91D620 invokes91CE90 and clears+284;91CE90 clears the record's+00/+04/+08.
The canonical MissionScoreRecord and MissionProgress already carry these fields
and owned containers. MissionRecordData's existing name/id field supplies the
same native record+0 key. No new record layout is introduced.

9052D0 is exactly XOR EAX,EAX;CMP [ECX],EAX;SETNZ AL;RET. Any nonzero signed
completion value is true.594B9E dispatches actual medal current34 with that
captured Boolean. When true,594BA4 rereads the same record+4 AFTER the callback.
SUB1,TEST/JG implement signed positive selection after DWORD wrapping, then
594BCA consumes the low16 bits. Thus INT_MIN ranking wraps to INT_MAX before
the low-word truncation. The widget at screen2F8 is resolved again at594BC2;
the actual Icon runtime receives current88/AB1710(state,0,FLD1). Missing widget
or an unsupported companion fails explicitly. The native Boolean byte's
unspecified upper argument bytes are not part of this semantic interface.

## Date storage and bytes

Mission reader5C6A70 consumes its `date` triple, converts the floats at
5C6C3D/43/49 and stores+68/+60/+64 at5C6C4F/55/5A.43BC30 treats them as
year/month/day; the date-field names remain
in the public interface. Both known callers5966F0 at596E4A and623710 at6237A3
pass EDX plus two stack words. It does not normalize the month index or localize
the month immediately. The borrowed E08100 table has a null slot0, followed
by `globals.date_jan` through `globals.date_dec`, proved by live table and
CE4190..CE427F string bytes. Valid native indices/pointers remain required.

| Language branch | Exact output for date1945,1,2 |
| --- | --- |
|French or Italian|`2\| \|globals.date_jan\|. 1945`|
|Spanish|`2\| \|.de \|globals.date_jan\| \|.de \|1945`|
|German|`2. \|globals.date_jan\| 1945`|
|Default, including empty|`globals.date_jan\| 2, 1945`|

These are localization markup bytes from CE42F4..CE4333, including punctuation
and spaces. French first uses the captured language data pointer and CRT
_stricmp; remaining branches reuse actual-header00425850 and its CRT locale.
The canonical008D4870 catalog helper is called AFTER the builder allocation;
the current language index and table are borrowed, not cached or copied.
Its existing out-of-range fallback differs from native unchecked indexing,
so valid language indices remain the documented native domain.

The fixed builder is distinct from the existing18h pooled log builder. Its
producer851E90 writes vtableD0BEAC before array allocation(capacity+1), then
capacity+8,buffer+4,used+C=0.43BC30 requests capacity128, so the allocation is129
bytes. The initial buffer is uninitialized. Each851F50/FA0 append computes a
wrapping DWORD used+length, compares unsigned against capacity, and skips an
oversized item whole. Successful appends, even empty, write the NUL terminator.
FA0's signed decimal_itoa uses32 stack bytes and no pooled allocation.

43BC30 constructs the language NativeString after builder allocation, constructs
the caller's fresh outputNativeString after all appends, returns the language
block with length+1, then frees the independent builder buffer. The dedicated
allocator boundary retains this ordering and distinct allocation domain; the
provided implementation uses source CRT malloc/free and throws bad_alloc on
failure. The original allocator/new-handler/heap ABI is not claimed.

851EC0's saved Ghidra extent ends at851ECE after calling CRT_free/BF6989.
Installed executable disassembly at851ECF..851ED0 is POP ECX;RET, followed by
INT3 padding. BF6989's body returns normally. This is a false no-return/body
extent artifact; the destructor is reconstructed through the real RET without
changing saved Ghidra. Parent annotation must preserve existing comments and
correct library names.

## Validation and remaining boundaries

`local/run_profile_display_probe.ps1` strict-compiles the new provider and one
ignored fixture with MSVC Win32,/W4,/WX,/fp:strict. The embedded-manifest probe
exercises actual8h NativeString allocation/release, six locale cases, a language
index changed by builder allocation, signed date words, exact capacity/skip
behavior and the existing score-map insertion/case-insensitive identity plus
nonzero completion predicate. The complete medal widget path is NOT exercised
by that fixture. No permanent test was added. Full repository build and report
verification results are recorded in `reports/main_menu_profile_display.json`.

The existing std::map/std::string MissionProgress and language catalog are
semantic projections, not native tree/record/string-vector ABI. Borrowed
owners, records, slots and resources must survive callbacks. Complete menu
service composition, executable menu reachability, instruction differential,
native SEH/allocation failure equivalence, original ABI, rendering and installed
game validation are not established here.
