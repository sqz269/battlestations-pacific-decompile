# Streamed-dialog singleton ownership and channel arbitration

Addresses: 00A778D0, 00A77970, 00A79230, 00A790D0, 00A78FB0, 00A789C0,
00A77C10, 00A78150, 00A781C0, 00A780B0, 00A77DD0, 004C87D0, 004C87F0,
00A77D00, 00A77D10, 00A778C0, 00A85C00, 00A86F40, 00A77FF0, 00A791D0, 00A796F0.

The application allocates 234h bytes at 0073DB02 and calls A79230 at 0073DB2B.
That constructor publishes F8BBCC through A778D0 and installs D58F78. Its two
vtable entries are A790D0 (scalar deletion) and A789C0 (frame update). The owner
loads `sound/streamed_dialogs.def`, owns two logical channel profiles and one
physical channel pair, and arbitrates which logical stream occupies the first
physical slot. "Alternate" is retained only in the C++ integration names already
used by `SoundSystemUpdateHost` and `SoundAlternateShutdownHost`. It does not
mean an independent audio engine. All descriptive names are hypotheses.

The implementation is `include/bsp/sound_alternate_owner.hpp` and
`src/sound_alternate_owner.cpp`. Original storage sizes are retained as aligned
bytes, with the existing actual-header NativeString APIs and actual native+4
Interlocked reference counts. No duplicate string layout or synthetic reference
counter is introduced. Native vtable words are dispatch identifiers for the
reconstructed runtime, not executable pointers into this C++ program.

| Routine | Coverage | Original ABI and behavior |
| --- | --- | --- |
| A778D0 | complete normal body | ECX owner, EAX owner, RET; install D58E58, capture manager section, publish CURRENT F8BBCC and register it |
| A77970 | complete normal body | ECX owner, RET; D58E58, unregister CURRENT F8BBCC, clear global, unlock captured section, CE3818 |
| A79230 | complete normal body with required table loader | ECX owner, EAX owner, RET; initialize embedded ownership, load table, create pair and logical profiles |
| A78FB0 | complete normal body | ECX owner, RET; clear active channels, pair array, two logical owners, table, reverse reference array, reverse record array, base |
| A790D0 | complete normal body | ECX owner, stack flags byte, EAX original owner, RET4; body then free on bit0 |
| A789C0 | complete normal control flow with required playback services | ECX owner, stack float dt, RET4; smooth gain, tick both logical channels, arbitrate physical stream |
| A77C10 | complete normal body | ECX owner, RET; release logical streams, clear request bytes, release current config references, release physical stream, -1 at208 |
| A78150/A781C0 | complete | ECX actual5Ch logical owner, stack category, EAX owner, RET4; distinct vtable and byte58 |
| A780B0 | complete normal body | ECX logical owner, RET; D58E5C, stream, pending config, current config, name |
| A77DD0 | complete normal body | ECX actual1Ch config, RET; additional ref18, then18h record |
| 004C87D0/004C87F0 | complete normal bodies | ECX actual18h record, RET; ctor returns owner; two native strings, reference10, float14 |
| A77D00/A77D10 | complete normal bodies | ECX actual8h pair, RET; ctor returns owner; two reference slots |
| A778C0 | complete | ECX actual4h reference slot, EAX slot, RET; null initialization |
| A85C00 | complete | ECX actual54h stream, RET; state20 in1/2 sets byteA=1 and B=0 |
| A86F40 | complete normal body | ECX actual54h stream, RET; build/release native diagnostic string before setting byte09=1 |
| A77FF0 | complete normal body | ECX stream, stack destination string, EAX destination, RET4; clear and copy native name24 |
| A791D0 | complete normal body in valid vector domain | ECX actual20h table, RET; reverse string cleanup, free data, CEB130 base vtable |
| A796F0 | complete normal body | ECX table, stack flags byte, EAX original table, RET4; table body then free on bit0 |

## Producer-established storage

| Owner offset | Constructor and ownership |
| --- | --- |
| 00 | D58F78, after publishing through D58E58 base |
| 04 | actual8h physical pair in a Ch allocation; DWORD count1 immediately before the pointer |
| 08/0C | two5Ch owners constructed by A78150(0) and A781C0(0) |
| 10..1BF | 18 actual18h records: string00, string08, reference10, float14=1 |
| 1C0..207 | 18 null native reference slots |
| 208 | -1; clear-channel body restores -1 |
| 20C | not written by A79230 |
| 210 | zero DWORD |
| 214/215 | bytes1/0; byte215 skips arbitration after both channel ticks |
| 216..217 | not written |
| 218/21C | profile volume values1; D58E60/D58E64 getters A77820/A77830 read CURRENT global owner |
| 220/224/228 | current gain1, target gain1, rate0 |
| 22C | actual20h reference-counted definition table |
| 230 | null completion callback address |

The table's native prefix is vtable00 D58F80, reference count04=1, data08,
count0C, capacity10, volume14=1, totalChannels18=0 and loop byte1C=0. Padding
1D..1F remains untouched. Its actual14h records begin with a NativeString, then
channel count08, first-channel0C, and the format enum10 already described by
`DialogChannelFormat` in `audio_online_startup.hpp`. The portable
`DialogStreamTable` in that header is a different C++ projection and cannot be
passed to this owner. A791D0 shrinks its native vector to0, decrementing count
before each reverse string destruction. It frees current data without clearing
data/capacity and then installs CEB130.

Each logical5Ch owner has stream04, NativeString08, a1Ch config10, a countdown
float2C that its constructor does not initialize, a1Ch config30, bytes4C/4D/4E
initialized0, category50, gain54=1, and profile byte58. Padding4F and59..5B is
untouched. Each config contains the same18h record and an additional ref18.
A78150 installs D58E60 and byte58=1; A781C0 installs D58E64 and byte58=0.

## Required services and observable ordering

| Call site | Caller | Native callee | Required contract |
| --- | --- | --- | --- |
| A793AC | A79230 | A87060 | Load the supplied NativeString path into actual20h table; both callers pass ECX table and one stack string |
| A78A32 | A789C0 | A78820 | Tick actual5Ch logical channel with dt and current owner gain; its only call site |
| A78C91 | A789C0 | A783F0 | Start or resume the selected logical channel, potentially replacing its stream04; its only call site |
| A78AC2 | A789C0 | indirect owner+230 | Invoke current callback address with temporary native name in ECX; indirect target body remains external |
| several, listed in report | owning routine | current reference vtable00 | Dispatch at actual native+4 count0; clear captured owning slot after callback |

The table zero callback is concrete for current D58F80: its BD30E0 slot0 calls
A796F0 with flag1. Other current vtables require the supplied existing
`GameplayEffectComponentLifetime`. This includes actual stream objects, whose
producer A877D0 establishes state20 and NativeString24. Passing a raw FMOD
channel here is invalid. Parent table-loading work can provide the first
dependency; this packet supplies no default service for unimplemented playback.

A86F40 itself is concrete: it uses the already recovered actual18h builder in
`native_vfs_open_logging.hpp`, appends `Stream TOSTOP:` and current stream name24,
destroys the builder, then writes byte09=1. Its constructor/destructor has no
sink operation. Allocation and release side effects are preserved.

Both logical channels tick before byte215 is tested. Eligible category0 channels
with byte4E clear and either stream04 or request byte4C compete for one physical
slot; the last eligible channel wins. Completion callbacks run for state3 before
the logical stream release, and owner/logical fields are reloaded after those
callbacks. A nonzero physical stream with no selected logical channel receives a
stop request unless its state is0/3, in which case it is released and cleared.
A competing stream receives the fade request and blocks replacement until it is
state0/3. Starting the selected logical stream precedes physical pointer
publication; then the incoming stream is retained before the captured old one
is released.

Native A789DC..A78A0C multiplies rate by dt in x87, stores a float step, and calls
the existing recovered `unit_step_towards_0042ac60` on220 with target224. The
reconstruction preserves that multiply/store and uses the shared math routine.
004254B0 is one RET; name temporaries around its diagnostic calls are still
constructed and destroyed. The two name diagnostics have `ADD ESP,8` atA78B91
and A78C40; the selected-channel diagnostic has `ADD ESP,Ch` atA78C8C, proving
format string, logical index and physical index0.

## Evidence corrections and limits

`docs/APP_INIT_AUDIO_ONLINE.md` called A77D10 the physical element constructor.
The actual array construction operands at A793EC..A79404 are constructor
A77D00 and destructor A77D10. The earlier Ghidra name
`BSP_DialogStreamTable_Load` for A79230 describes only one step in a234h owner
constructor; the new proposed name is `BSP_StreamedDialogManager_Construct`.

Ghidra's false no-return annotations for CRT free wrappers produce premature
returns in A78FB0 and A791D0 pseudocode. Assembly resumes after A79000, A7901A
and A79034 and reaches A790BA RET. Disk bytes after A79205 show stack cleanup,
BD30F0 at A79217 and the final A7922C RET. Scalar wrappers return original ESI
after free (A790E8/A79708); the decompiler's `extraout_EAX` is not the return
contract. Missing small function starts and final instruction lengths are
recorded in the report for primary-agent Ghidra repair. Workers do not mutate
Ghidra.

This is normal-path C++ reconstruction, not native MSVC SEH or binary ABI
compatibility. The valid-storage domain requires live nonnull channel/pair
allocations at cleanup/update, nonnegative valid table count/capacity and
nonthrowing reference callbacks. No guards convert invalid native storage into
successful cleanup. The shared CRT allocator has the native retry/new-handler
throwing contract. Constructor exceptions after native state3 clean table,
embedded arrays and base; its later raw pair/logical allocations are not added
to that cleanup because the native constructor has no such ownership states
after successful publication. State5 does free an incomplete physical allocation
if its vector construction throws; the two-slot null constructor cannot throw
in this C++ implementation. The owning A79230 call is still state-1 while its
base constructor runs.

The base routines' C++ exception paths restore CE3818 after the captured manager
guard unwinds, while preserving publication and partial registration effects.
Their native evidence is constructor FuncInfoDEAB3C/mapDEAB2C with state1
CB4CC8->411EE0 and state0 CB4CC0->412430; destructor FuncInfoDEAB70/mapDEAB60
uses the analogous CB4CE8 guard and CB4CE0 root reset. The 412430 body writes
CE3818. Reference-callback exceptions and Windows structured exceptions remain
outside the reconstructed exception domain. Native SEH ABI itself is not
translated. No game-path timing claim is made.

Build and evidence verification results are recorded in
`reports/sound_alternate_owner.json`. There is no claim of stream playback,
application instantiation, or game validation from this standalone packet.
The Release Win32 build and both existing tests passed after the exception
correction. A single ignored actual-manager probe verified constructor-throw
unlock/root reset/publication and normal destruction of the current global;
destructor-throw cleanup remains supported by the native handler map.
