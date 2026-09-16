# Native particle Layer lifetime (orch4 j10)

This packet reconstructs all three raw Layer lifetime bodies in
`src/native_particle_layer_lifetime.cpp`. It accepts the actual 40h storage and
`NativeStringRawPoolContext`; there is no projected Layer owner, host string,
callback facade, unresolved body, or parser implementation.

## Bodies and original ABI

| Original span | Bytes | Source function | Native ABI |
| --- | ---: | --- | --- |
| AFAB90..AFAC46 | 183 | construct_native_particle_layer_00afab90 | ECX Layer; stack borrowed name-header pointer, DWORD value; EAX Layer; RET8 |
| AFAC50..AFACD8 | 137 | destroy_native_particle_layer_00afac50 | ECX Layer; RET; no defined EAX result |
| AFACE0..AFACFD | 30 | scalar_delete_native_particle_layer_00aface0 | ECX Layer; stack flags; EAX Layer; RET4 |

The paired JSON report records all 350 body bytes and their full live/installed-PE
SHA-256 hashes, nine direct call sites, five cleanup tail jumps, original names
and comments, ABI, and complete supporting EH/table/constant spans. Queries used
the existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
The parent repaired the scalar call site's false CALL_RETURN override at AFACF0:
AFACF5..AFACF7 is `ADD ESP,4`. The full return path ends at AFACFD. Prior metadata
and the saved repair are retained in
`reports/native_particle_layer_flow_orch4_j10.json`; this worker made no Ghidra
changes. The constructor and destructor had no listing gaps.

## Raw storage and schedules

Construction writes CEB130 to +0, reference count 1 to +4, then D5DC38 to +0.
It zeros name length/data at +8/+C and material length/data at +14/+18. Identity
of the supplied name header and Layer+8 is tested after these stores; identity
therefore leaves an empty header and abandons any old data. Byte +2C becomes 1
before the name resize/copy. A distinct name is borrowed, never consumed.

The requested name length is captured before 41DD40(preserve=true). After resize,
the source length is reread; if nonzero, the code loads current destination
length, source data, and destination data in that order. BF7680's overlapping
copy behavior is represented by memmove. A zero-byte copy is omitted consistently
with the existing raw-string source providers.

After copying, the caller's DWORD is written to +10. Fields +30, +34, +3C become
positive zero and +38 becomes the exact binary32 word 459C4000 (5000.0), read by
the original MOVSS from D1AF84. There is no x87 conversion. Bytes +1C..+2B and
+2D..+2F remain untouched; do not value-initialize the entire 40h object.

Destruction writes D5DC38, releases material+14, then name+8, and calls the
genuine BD30F0 base destructor. Each normal string release captures data before
the state transition, then reads current length+1 before resolving the raw pool.
Null data skips that length read and both pool calls. The headers are never
cleared. The second header is loaded after material release, permitting changes
made through native providers to be observed. Scalar deletion calls the full
destructor, invokes the established BF65AC heap provider only for flags bit0,
and returns the original object address.

D5DC38 contains [BD30E0, AFACE0], with writes at AFABC3 and AFAC6D. Slot0 is the
existing ref-counted dispatch to the current table's deleting-destructor slot;
the table address in these source bodies remains original ABI evidence, not a
promise that an unbound original address is executable in the rebuilt process.

## Exception cleanup

Constructor handler CBB0AE names FuncInfo DF2FE4, map DF2FCC. The map contains
2->1/current material+14 (CBB0A3), 1->0/current name+8 (CBB098), and 0->-1/base
(CBB090). Execution sets state0 before header initialization, then state2 before
the byte+2C store and name copy. All three actions use the current object.

Destructor handler CBB0D3 names FuncInfo DF3018, map DF3008. Material release runs
in state1: failure destroys the current name via CBB0C8 and then the base via
CBB0C0, without retrying material. Name release runs in state0: failure destroys
only the base. State-1 precedes normal BD30F0. The source uses a noexcept unwind
guard; if cleanup throws while another C++ exception is active, termination
prevents replacing the original exception or continuing lower cleanup states.

Dependencies are the existing raw 41DD40/41DD20 string providers, concrete
419CC0/BD1510 pool composition, BD30F0 base, and canonical heap free. No new
allocator or injectable cleanup interface was introduced.

## Validation and limits

The strict MSVC Win32 build and both existing CTests passed. All 14 call/tail
rows verified with zero failures; the current 60/42/11-instruction bodies have
zero listing gaps. The focused original/source comparison passed all 350 body
bytes. Artifact hashes and results are recorded in the paired JSON report.
The focused ignored
`local/layer_lifetime_probe.cpp` executes copied full original bodies, patches
their external calls to the existing source dependencies, and relocates the
constant address. It compares normalized complete 40h images, embedded-NUL name
copy, empty and self-header construction, unchanged unknown bytes, actual pool
material-before-name returns, null material, and scalar flags2/3 and return values.
No permanent test suite was added.

These owning C++ interfaces add a raw pool context. The normal original/source
probe does not execute original FH3/SEH, allocation-failure paths, asynchronous
faults, or a second unwind exception. Those schedules are assembly/map reviewed;
neither binary replacement compatibility nor gameplay validation is claimed.
AFAD00 and changes to the resource caller AF4280 are outside this packet.
