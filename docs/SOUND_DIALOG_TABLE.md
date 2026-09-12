# Actual streamed-dialog table loading

Addresses: 00A87060, 00A77EF0, 00A78DC0, 00A77EA0.

The A79230 streamed-dialog manager constructs the actual20h refcounted table
at owner+22C. A87060 appends records from the supplied native filename through
the existing VFS scanner. This replaces a required table-loader dependency;
the older portable DialogStreamTable parser remains a separate projection.
Names are descriptive hypotheses, not recovered symbols.

The table contains vtable/refcount0/4, vector data/count/capacity8/C/10,
volume14, total physical channels18 and loop byte1C. Each actual14h record
contains a native string0/4, channel count8, first channelC, and format10.
The format array E12EF0 initially contains DWORDs0,1,2,6. It remains a supplied
live array and is reread after token acceptance and during record copies.

The original parser does not reset the table, volume, loop, or channel total.
Channels sections append until `end`; format keywords are `mono`, `stereo`,
and literally `51`. Volume uses BEF170 and Loop stores1. Unknown top-level
tokens, including a quoted empty token, do not advance. Input must terminate;
this reconstruction does not manufacture a malformed-input recovery rule.

A77EF0 clears destination name fields before checking self-copy, reads the
source format, stores the format-derived count, then deep-copies the name.
After allocation it rereads the source fields and overwrites count/first from
the current source. Self-copy consequently leaks the original name. A78DC0
clamps capacity to at least1, copies records forward, destroys old names
forward while rereading current base/count, frees the old allocation, and
publishes the new pointer/capacity. A77EA0 releases only a captured nonnull
name using its current length+1, leaving the header unchanged.

Native temporary record+C is unwritten stack storage. The host accepts that
opaque word explicitly; the parser copies it and immediately overwrites the
appended row's first-channel field before its next external call. No unknown
stack value is presented as a recovered constant.

## Assembly and exception evidence

All four routines use ECX self. A87060 and A78DC0 return void with RET4;
A77EF0 returns its destination in EAX with RET4; A77EA0 has RET. Native ends
are A87386/3, A78EB2/3, A77F7D/3 and A77EBC/1 respectively. The parser's
24-byte tail after free was verified separately; Ghidra still has a truncated
stored function body. Reserve's15-byte free fall-through gap was repaired.

FuncInfo DEABF4 references unwind map DEABEC: copy state0 cleans its name
through CB4D40 ->0041DD20. FuncInfo DEAD40/map DEAD38 only calls the empty
placement-delete00401130 via CB4E30 when growth copying fails; completed
prefix records and the new allocation survive. FuncInfo DEC1CC/map DEC1F0
has10 states. Scanner construction has an allocation guard; after successful
construction the scanner has no parser exception cleanup. Temporary record
state2 uses CB60A3 ->A77EA0. Append-copy state7 invokes the empty placement
delete, then temporary cleanup. These surviving allocations are preserved.

## Validation and boundaries

The Win32 source compiles using actual record/header storage. The scanner is
the existing NativeTextTokens/SoundSampleScanner value projection over VFS
bytes, with its established token-size and C++ exception-domain limits;
its C++ allocation is not the original828h scanner ABI. Host allocation and
pooled strings use existing explicit services. Invalid pointers, invalid
format indices, overflowing allocation sizes and native SEH are outside the
documented domain. No replacement vtable addresses are callable host code.

The integrated validation receipt in reports/sound_dialog_table.json records
the final build, existing tests, installed-input fixture, native byte hashes,
annotations and source hashes. An independent worker checked all four bodies
and exception maps; its quoted-empty-token correction is included.

## Follow-up packets

Bind A78820 logical-channel update and A783F0 logical start to the actual54h
stream owner/runtime before claiming streamed-dialog playback. Application
phase5 also needs the shared raw singleton registration/deletion integration
underway on main. Sound runtime composition and dialog construction alone
do not establish application startup, audible output, ABI compatibility, or
gameplay validation.
