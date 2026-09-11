# Panel sequence advancement

This packet reconstructs the panel owner used by voice update005BBF10. Names are
descriptive hypotheses, not recovered symbols. `panel_sequence_types.hpp`
contains the queue and entry types; `panel_sequence.hpp` exposes the owner view,
required services and concrete routines. All storage references identify the
same owners used by the existing voice, scheduled-record and GUI implementations.

## State and command behavior

The owner at current game+21E4 has a case-insensitive NativeString map at+1C,
map count+24, current NativeString+28 and state+34. Bind count/state to the
canonical `VoicePanelState` gates; do not maintain a second queue or mirror.

004527F0 returns false from state0 with count0, otherwise selects and steps an
entry and writes state1 after callbacks. State2 first resets every voice row,
then selects/steps and writes1. Other states return false. State1 looks up the
current entry, erases exhausted entries and clears current, then snapshots the
current name. An entry with remaining commands and byte+38 instead erases and
returns true immediately, leaving state1 and selection until the next call.
After selection, an empty name resets rows and writes state0 before temporary
cleanup/false. A previously empty name resets rows and dispatches either the
command loop (signed cursor>0) or a full step. A changed nonempty name writes2;
an equal name steps. Temporary string cleanup happens after these callbacks.

00452360 initially checks and captures commands[cursor], even if the vector is
empty. It consumes command kinds1/2/4. Kind1 grows native14h slots, activates a
slot, copies text and palette strings and resets that voice row. Kind2 clears
the active byte when its unsigned index is in range. Kind4 writes global
00E17BFA=1, calls the existing00887E50 mission Lua implementation with zero
arguments, then writes0. The cursor increments from its current value after
callbacks. All repeated kind virtual calls remain repeated. A different kind
tail-calls the required00451C90 row publisher. No publisher call occurs merely
because the vector becomes exhausted.

00452740 retains its original entry across00452360, then inspects that entry's
current cursor. Kind0 stores selected slot+3C and resolves the named timed voice
record through00705E00 before005B94D0. Entry elapsed+C is captured before lookup
and survives as the third stack float. Kind3 stores command float+8 minus entry
elapsed+C in current voice manager delay+88. Cursor advancement follows calls.

005B94D0 selects the row, initializes its fade/color, calls actual005B93D0
admission, stores delay and uses the final timed key's end as hold+8C. Without
keys it performs the existing panel-text GUI sequence and005B9420 poll. Busy
sets hold0; idle computes signed UTF-16 text length * per-character+7C + base+80
with x87 and a final float spill. Borrowed `VoiceClipRecord` identity is preserved.

005B9490 rechecks the captured manager's current row count on every iteration
and calls005B8A00. That routine bounds-checks the index, constructs a local
zero/zero NativeString and calls resize(0,true). The recovered resize returns
without allocation, so the subsequent nonempty display arm is unreachable for
this fresh local. Its reachable arm hides group/background/decoration in order
and clears both text widgets with flag1. Row scheduling fields are untouched.

## Selection and ownership

0044C390 returns immediately when current has nonzero length. Otherwise it
walks map order, preferring larger priority+4, then smaller tie+8. Initial best
priority is exactly -10000000000.0f (00CE4ADC bits D01502F9). Its secondary
stackfloat+18 is genuinely uninitialized until a first winner. The context
therefore requires the actual incoming bits by reference; the input is read
only on equality with the sentinel before a first winner. There is no invented
zero/default or synthetic exception branch. Quiet NaNs follow ordered
comparison/parity behavior. Winning values are reread after key-copy allocation.

00443D00 and00449AF0 retain their established names and call host `_stricmp`
after the recovered stored-length-zero gates. This is the CRT comparison,
including NUL termination and active locale, not a replacement comparator.

00451920 performs lower-bound/insert of the actual zero/default native40h
entry. It retains both pair and node key deep copies and temporary cleanup.
0044BA00 returns an owner+iterator pair.00451020 advances that iterator by
value, unlinks the node, destroys its payload and frees it, then rereads and
decrements the current count only if positive. It is not a length-error throw.
The C++ uses `std::map` node extraction for the existing tree library work.
0044FFF0 installs its vtable, deletes nonnull commands via vslot0(1), and clears
the captured pointer cell after each callback. It then destroys slots, frees
the slot vector, and frees the command vector.0044B530 releases palette before
text;00450110 destroys the entry before releasing its key. NativeString has no
implicit ownership cleanup; queue owners must call the explicit erase path.

## ABI and evidence

Every listed range includes its final instruction. Source interfaces are typed
projections, not binary-compatible replacements. Assembly exports are under
ignored `exports/bsp/functions/<address>/assembly.txt`; the raw disk tail is
also necessary for00451020. The machine-readable report records per-function
scope, original ABI, evidence, final instruction and uncertainty.

| Address | Native ABI | Final instruction (length) |
|---|---|---|
|004527F0|ECX owner, AL boolean, RET|004529F6 (1)|
|00452740|ECX owner, RET|004527EB (1)|
|00452360|ECX owner, RET or tail00451C90|0045262E (1)|
|0044C390|ECX owner, RET|0044C599 (1)|
|0044BA00|ECX map, hidden iterator*/key*, RET8|0044BA63 (3)|
|00451020|ECX map, hidden iterator*/by-value owner+node, RET0C|004512D0 (3)|
|00451920|ECX map, key*, entry* EAX, RET4|00451A42 (3)|
|005B9490|ECX voice manager, RET|005B94CC (1)|
|00449AF0|ECX left string, right string*, AL, RET4|00449B3A (3)|
|00443D00|left/right string* on stack, AL, RET8|00443D4E (3)|
|004483F0|ECX owner unused, entry*/slot/string*, RET0C|0044842F (3)|
|005B8A00|ECX voice manager, row index, RET4|005B8C24 (3)|
|0044FFF0|ECX entry, RET|004500C6 (1)|
|00450110|ECX pair key then entry+8, RET|00450170 (1)|
|0044B530|ECX slot14h, RET|0044B5A6 (1)|
|005B94D0|ECX voice manager, row/record*/float delay, RET0C|005B9754 (3)|

00452740's `stl_probable` tag and00451020's `stl_throw_site` name/tag are
misleading for these complete bodies. Parent-authorized locked flow repair
cleared the erroneous free-call overrides at00451298,0045008A,004500A4, verified
disk/live bytes, disassembled gaps and saved.0044FFF0 now has no flow gaps.
00451020's decoded tail0045129D..004512D2 remains outside the stored function
body: parent must extend it through RET004512D0 length3 (inclusive end004512D2).
No other new function starts or main-routine flow gaps were found. See
`reports/panel_sequence_flow_repair.json`; no renames, prototypes, tags or
function definitions were changed in Ghidra by this worker.

## Remaining boundaries and validation

00705E00 record resolution/construction and00451C90 row publication remain
required real services, not defaults. The latter calls0044F220 string palette
lookup and005B6910 row configuration; these are not claimed reconstructed.
Virtual command destruction/kind, GUI services and matched Lua are real host
contracts. Host getters must be side-effect-free and expose the current actual
owners; a returned ScheduledVoiceContext must bind the supplied manager.

Container allocator/debug-iterator ABI, malformed containers, allocator/copy
reentry during standard-container insertion or growth, reentry that invalidates
retained nodes or buffers, native exception/SEH
unwinding, signaling NaNs/unmasked FP traps/status, and embedded CRT locale
state are outside the projection. Standard vector growth uses move ownership
instead of duplicating MSVC's internal copy/free sequence. No full-engine,
binary-ABI or live-game validation is claimed. Parent shared VoicePanelState
embedding and removal of the old VoiceSequenceHost callback are integration
work; this worker does not edit those files.

Validation is recorded in `reports/panel_sequence.json`. One ignored focused
fixture checks the concrete erase/reentry risk: unlink, command deletion,
palette/text/key release, then decrementing a count changed by the callback.
Recipe: `./local/run-panel-erase-probe.ps1 -SourceRoot <checkout>`; optionally
pass `-CoreLibrary <bsp_core.lib>`. It uses current headers/library rather than
copied reconstructed source. This fixture is not a state-machine/gameplay test.

Final standalone MSVC Win32 Release build passed. This worktree's configured
existing test passed (reconstructed_math, 1/1); no native differential test was
enabled here. The focused fixture passed against bsp_core.lib SHA256
C2A10FAEB8A12138690F4A50FC230EA7BB98B9841BA193A1536FB53CB7BEFC62.
