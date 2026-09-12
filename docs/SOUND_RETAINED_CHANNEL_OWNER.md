# Retained channel construction and destruction

Names are descriptive hypotheses. The nine routines are complete behavioral
projections within the existing sound/string/lifetime host contracts, using
`SoundChannelInstance`, `SoundInstanceContext`, and the canonical actual 14h
`SoundRetainedSourceRecord`. The complete C++ channel objects have new layouts
and interfaces; they are not drop-in native ABI replacements.

| Native routine | Projection | Native table/size | Coverage |
| --- | --- | --- | --- |
| 00A7DA40 | construct_retained_sound_channel_00a7da40 | D5AD58 / 70h | complete |
| 00A7DB80 | construct_scaled_retained_sound_channel_00a7db80 | D5ADA0 / 74h | complete |
| 00A7DCD0 | construct_underwater_retained_sound_channel_00a7dcd0 | D5ADE8 / 70h | complete |
| 00A7DAD0 | destroy_retained_sound_channel_00a7dad0 | D5AD58 | complete |
| 00A7DC20 | destroy_scaled_retained_sound_channel_00a7dc20 | D5ADA0 | complete |
| 00A7DD60 | destroy_underwater_retained_sound_channel_00a7dd60 | D5ADE8 | complete |
| 00A7DB20 | scalar_delete_retained_sound_channel_00a7db20 | D5AD58 slot04 | complete |
| 00A7DC70 | scalar_delete_scaled_retained_sound_channel_00a7dc70 | D5ADA0 slot04 | complete |
| 00A7DDB0 | scalar_delete_underwater_retained_sound_channel_00a7ddb0 | D5ADE8 slot04 | complete |

## Constructor and producer evidence

All three constructors take ECX=self and two stack words: source-record pointer
and class-slot pointer. They return self in EAX and consume `RET 8` at A7DAC4,
A7DC11 and A7DD54 (each instruction is three bytes). A7F2F0 is their sole
recorded caller and allocates 70h/74h/70h. Its factory integration is a separate
packet.

A7DA70/A7DBB0/A7DD00 call the existing A7AE00 case-insensitive listener lookup
on the current manager+A4. The first two pass the literal at CE9B1C, whose bytes
are `41 69 72 00` (`Air`); the third passes CE5484 (`Underwater`). These are
literal strings, despite Ghidra's pointer/vtable rendering. The lookup's result
becomes base `type_48`; an unknown listener retains the existing lookup's zero
fallback. A7C480 receives exactly four stack arguments: sample from source+00,
class-slot pointer, lookup result, and flag1. Its `RET 10h` confirms the count;
the pseudocode's extra EDX-derived argument is not a native argument.

After base construction, all three store ended5A=0, channel54=null,
virtual59=wasVirtual58=1, and their own vtable. They call A7C5F0 with
ECX=self+5C and the original source pointer. That producer writes sample+00,
NativeString+04 (length/data at +04/+08), and x87-copied floats at +0C/+10.
Thus channel +5C is a second owned sample reference, +60/+64 the copied name,
and +68/+6C the float words. It is neither a borrowed sample alone nor a new
channel-specific duplicate record schema. The source-record packet documents
its independent producer evidence and partial-construction lifetime behavior.

All three clear paused1C again after the copy. A7DB80 alone loads D7A24C
(`00 00 80 3F`) using MOVSS and stores 1.0f to +70. Neither the basic Air nor
Underwater constructor writes that extra field.

## Destruction and native unwind evidence

Every ordinary and scalar destructor first calls 004C7FA0 with ECX=self+5C,
then A7BF40 with ECX=self. No derived-vtable reset precedes record destruction.
The record destroys its string before decrementing its sample, while A7BF40
sets D5ABF8, calls FMOD stop even for a null channel, and destroys the canonical
base sample/class/name. A constructed channel therefore holds two sample
references and releases both through their actual owners.

The scalar variants inline that same ordering and test flags bit0 only after
destruction returns. Set bit0 calls the returning CRT free service BF65AC;
the previously undisassembled bytes at A7DB69/A7DCB9/A7DDF9 are `83 C4 04`
(`ADD ESP,4`), not missing destructive behavior. EAX receives the original
pointer and `RET 4` ends each function at A7DB7D/A7DCCD/A7DE0D. The C++
projections use matching typed `delete` for the factory's typed `new` storage.
Their members have no implicit native-resource cleanup, so this does not
release the record or base twice.

| Owner | FuncInfo | State0 unwind action | Tail site -> callee |
| --- | --- | --- | --- |
| A7DA40 | DEB3CC | CB53D0 | CB53D3 -> A7BF40 |
| A7DAD0 | DEB3F8 | CB53F0 | CB53F3 -> A7BF40 |
| A7DB20 | DEB424 | CB5410 | CB5413 -> A7BF40 |
| A7DB80 | DEB450 | CB5430 | CB5433 -> A7BF40 |
| A7DC20 | DEB47C | CB5450 | CB5453 -> A7BF40 |
| A7DC70 | DEB4A8 | CB5470 | CB5473 -> A7BF40 |
| A7DCD0 | DEB4D4 | CB5490 | CB5493 -> A7BF40 |
| A7DD60 | DEB500 | CB54B0 | CB54B3 -> A7BF40 |
| A7DDB0 | DEB52C | CB54D0 | CB54D3 -> A7BF40 |

Every FuncInfo has maxState=1 and one entry `{next=-1, action}`. Each action
loads saved self from EBP-10h and tail-jumps to A7BF40. Constructors activate
state0 before A7C5F0, so failed record copy first runs its own sample-slot
cleanup and then the channel base. Destructor state0 remains active during
record destruction and becomes -1 before the explicit base call. The C++
implementation preserves this cleanup ordering. A base-constructor failure
belongs entirely to A7C480's own unwind contract; it does not run A7BF40.

## Service and verification boundaries

The report records all 24 direct call sites with their containing caller and
callee; EH tail transfers have separate tail-site/callee fields. A7AE00,
A7C480, A7C5F0, 004C7FA0, A7BF40 and BF65AC bodies were inspected before
using their existing contracts. No new opaque service or substitute record
was introduced. Nine full function spans, including RET immediate bytes and
the scalar stack-cleanup gaps, match the installed PE. The nine independent
FuncInfo/state-map/action chains also match live Ghidra and the installed PE.

Ghidra reads verify `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, against the configured target before each batch.
The report carries exact spans and SHA256 values. With the real record
dependency merged, `scripts/build.ps1` passed the MSVC Win32 Release build and
both existing tests after native seed verification. The focused local ownership
probe compiled with `/W4 /WX` and an embedded manifest. It passed both sample
retains/releases, Air/Underwater lookup, callback-visible vtable/order, scalar
flags and freeing, record-copy failure cleanup, and record-destroy failure
cleanup. These are host-fixture results, not native execution of these nine
routines. `tools/verify_report_calls.py` checked 24 direct call rows with zero
failures. No new tracked tests were added.
The C++ interfaces preserve the established string storage, class descriptor,
sample zero-reference callback, and FMOD host contracts. They do not recreate
native SEH frames or claim compatibility for nested exceptions from cleanup
hosts. Audible output, manager dispatch and gameplay are outside this packet's
validation; no game-validation claim follows from the construction evidence.
