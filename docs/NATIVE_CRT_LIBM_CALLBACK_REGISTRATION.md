# Actual CRT pointer encoding and libm callback publication

This packet implements the complete110-byte encoder atC04F67 and the separate
41-byte registration body atC0F0BB. It borrows actual current TLS/FLS/PTD/import
storage and the existing full module-gate source. Registration publishes an
encoded callback address; it does not establish or own a live registration.
The decoder atC04FDE and the libm dispatcher remain separate packets.

## Original bodies and qualified source interface

Original encoder is cdecl with a pointer word at entryESP+4, EAX result and
plain RET. Original registration is cdecl with one raw callback word and RET;
it has no semantic result. Both source entries retain that actual first caller
argument slot, add a stable context reference at entryESP+8, and require the
caller to clean both arguments. Encoder still saves onlyESI. Context loads and
the fixed gate bridge are explicit additions; the source is not native caller
ABI-compatible or a151-byte literal replacement.

| Encoder context byte offset | Borrowed binding |
|---|---|
| 0 | Current DWORD E15B00, real TLS index selecting the raw getter |
| 4 | Current DWORD E15AFC, actual FLS/TLS PTD index |
| 8 | Current CE20BC TlsGetValue IAT function word |
| C | Existing NativeCrtPointerDecodeSupportContext |

The two indices are current volatile DWORD references. The IAT binding is a
const-volatile reference to the actual `void* (__stdcall*)(uint32_t)` import
word, not an arbitrary getter callback or a copied function pointer. These
binding addresses, context objects and added argument words stay valid/stable
through every reached call; their actual native values can change. Encoder
and independently implemented decoder use disjoint headers/contexts with the
same reviewed four-reference shape; no shared header is modified.

Registration context contains encoder context reference at0, actual mutable
callback-present109E1B8 reference at4, and actual mutable encoded109ED80
reference at8. Null registration reads only its flag binding/value; it does
not read the encoded value or encoder binding. The added contexts are not
placed in a native owner or used to synthesize its storage/lifecycle.

## Complete encoder schedule

| Original range | Source schedule |
|---|---|
| C04F67..C04F76 | SaveESI; push currentE15B00 BEFORE loading currentCE20BC intoESI; call that captured real import. |
| C04F76..C04F84 | Only a nonnull first result reads currentE15AFC; FFFFFFFF selects fallback. |
| C04F84..C04F8F | Push captured PTD index; rereadE15B00; call the same captured imported getter; call its newly returned raw getter unchecked with the captured index. |
| C04F8F..C04F9B | Null PTD selects fallback; nonnull loads current PTD+1F8 and jumps directly to selected-provider test, including a null field. |
| C04F9B..C04FAC | Actual GetModuleHandleA with exact KERNEL32.DLL bytes; capture module inESI; null selects identity. |
| C04FAC..C04FB5 | Fixed bridge calls complete source C04EFB with borrowed existing context; test its result. Captured module survives the call. |
| C04FB5..C04FC1 | Actual GetProcAddress(captured module,exact EncodePointer bytes). |
| C04FC1..C04FCF | If selected encoder nonnull, push current first argument slot, call actual stdcall encoder, write returnedEAX to that SAME caller slot. |
| C04FCF..C04FD5 | Reload current first argument slot intoEAX, restoreESI, RET. Identity paths did not write the slot. |

With entryESP=E, saved-ESI ESP isE-4: actual first argument is[ESP+8], context
is[ESP+C]. After an extra pending index is pushed, context is[ESP+10]. The
second query has both its current TLS index and the captured PTD index below
it; each actual stdcall consumes its own one word. The gate bridge adds then
removes its context word. No slot-pointer replacement or C++ by-value adapter
hides the encoder's original argument writeback.

The actual PTD needs a readable1FCh-byte prefix; +1F8 is a current stdcall
`void*(void*)` encoder. A null pointer inside an existing PTD gives identity,
not fallback. The second getter can differ from the first result, which is
used only as a condition. Its returned function is called unchecked; no new
null-success path or TLS retry is inserted. E15AFC/E15B00 are actual initialized
OS selectors, not fixed constants. Discovery verified214h-byte startup PTD
allocation, but this implementation neither allocates it nor claims that full
startup/locale/lock/SEH closure is reconstructed.

Readonly copies pin the exact thirteen KERNEL32.DLL bytes atD69FD0 and fourteen
EncodePointer bytes atD69FC0. Only concrete named Win32 imports consume them.
There is no LoadLibrary/search fallback, generic module resolver, decoder cache
update, extra LastError restoration, pointer validation or changed error policy.
Whole static COFF/link proof must establish the actual named import operands.

## Complete registration schedule

`CMP DWORD[ESP+4],0` precedes context use. Null loads the actual flag-cell
binding, performs `AND DWORD[cell],0` as a real read-modify-write, and returns.
The encoded word stays stale. Nonnull loads the stable encoder context, then
rereads the actual first caller slot after pushing that context. The full
encoder receives a real callee argument slot, plus its added context word.
After it returns, source removes both arguments, reads the fixed encoded-cell
binding and writesEAX, then reads the fixed flag-cell binding and writes1.

No pre-clear or publication occurs before the encoder returns. Provider side
effects and failures retain their actual behavior; no catch, rollback or fake
return is added. Original null EAX is incidental; nonnull EAX happens to retain
the encoded value but neither is exposed as a semantic return. Extra context
loads change incidental volatile registers. In particular, the source setter
uses `ADD ESP,8` where the original uses `POP ECX`; that added cleanup changes
incidental outgoing EFLAGS. No semantic return or native volatile-register /
EFLAGS identity is claimed.

The true callback ABI for the later libm service is
`int __cdecl(CameraAxesCrtException*)`, with a mutable32-byte record, balanced
stack, preserved nonvolatile registers and valid x87 stack convention. This
setter only accepts and encodes its raw address. It neither calls nor retains
an owner for the callback. Code and dependent data must outlive all captured
calls. Clearing the flag does not quiesce previously captured callbacks; no
registration lock, atomic pair, retirement policy or current lifetime proof
is added. On the nonnull branch, a changed second argument-slot read is not
revalidated: the actual second read is encoded and that result is published.

## Provider and evidence boundaries

The fixed bridge directly composes
`native_crt_pointer_decode_module_gate_00c04efb`, complete source108 bytes;
that uses full native winmajor60 and raw strcmp136 source. The gate's borrowed
owning errno and returning invalid-parameter services remain the established
source boundaries. No broader CRT startup reconstruction or generic external
handler equivalence is claimed. Actual OS/PTD/import code and all reached
storage must stay valid; no private TLS/FLS allocation replaces that contract.

Fresh guarded BSP queries pin both full owned bodies (151bytes/46instructions),
the full304-byte prerequisite closure, return boundaries, exact names and data.
Thirteen spans comprise508 disk-backed bytes and eight separate saved virtual
bytes. The virtual zeros at109E1B8/109ED80 describe saved PE initialization,
not current runtime state or an encoded-null representation. Original game PE
SHA256 isb682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6.
The discovery seal remains unchanged. Ghidra still has no function atC0F0BB;
definition/annotation is primary-owned after whole-body review.

The reviewed source passed the strict MSVC Win32 build, both existing CTests,
and eight fresh differential seed checks. No additional runtime test was added.
The address-only link artifact and its seven member-address accessors were
compiled and inspected, never executed. Encoder, setter, callbacks and original
bodies were not run.

The first build rejected reference-member `offsetof` assertions. Its source,
input manifest and full log are preserved. Only those invalid assertions and
the unused `<cstddef>` include were removed; the reviewed naked sequences and
header stayed unchanged. Final `sizeof` checks prove context sizes 16 and 12.
Seven compiled C++ address accessors independently prove stored reference-word
offsets 0/4/8/C and 0/4/8. Each has exactly the prologue, argument load, single
binding-word load at its expected offset, epilogue and RET. No accessor runs.
The unchanged prerequisite object was compiled during the first attempt and
retained by the final successful incremental build; its actual compile command,
source hashes and exact archive membership are checked.

The actual strict archive is frozen at SHA256
`c5e75faa961ffb7ea23cbe55d36539651e044b2a51ba4411a64072dc73b43e83`.
Both complete archived objects and their 24 source/header files are frozen.
All 24 mapped COFF sections match the linked image after applying all 30
relocations. Whole owned object evidence includes seven sections/seven
relocations; the provider object includes thirteen sections/five relocations,
including unlinked sections. The encoder compiles to 128 bytes/46 instructions,
registration to 55 bytes/17 instructions, and the fixed gate bridge to 19 bytes.
The verifier maps all 46 original instructions in order to every compiled
encoder/setter instruction, explicitly accounting for context loads, extra
arguments/cleanup, branches and provider bindings. This is complete mapping,
not literal 151-byte equality.

The bridge's full instructions prove its current context+0C binding load and
direct call to the actual gate. Concrete GetModuleHandleA/GetProcAddress IAT
operands resolve to KERNEL32 imports. Both original encoder names and both
reached gate `.mixcrt` operands resolve to exact immutable bytes. The full
136-byte strcmp provider is literal original identity. Existing translated
module-gate/winmajor source boundaries remain; their full archived code,
relocations and source are pinned without claiming native instruction identity.

Replay inputs and results are under ignored
`local/libm_callback_registration/`. `REPLAY.md` gives frozen-only link and
verification order; `sealed.json` pins the final evidence. No runtime IAT/PTD
values, installed callback lifetime, native caller/SEH/fault-site equivalence,
CRT startup closure, game behavior or arbitrary provider ABI is established.


Primary integrated these entries into the main Win32 library and replayed the unchanged complete static verifiers against its frozen actual objects. Full151/46 mapped to128+55bytes/63instructions;2actualobjects24mappedsections/30relocs,fullbridge19,7compiler layoutaccessors and exact136strcmp. The same46 prebuild source/header/build inputs remained unchanged; both existing CTests and8 seed comparisons passed. The main artifact is frozen under `local/lifecycle_pointer_build_frozen/`; fresh native captures and complete static replay are under `local/lifecycle_pointer_replay/`. Reviewed names/comments are saved and all affected exports refreshed. The formerly missing C0F0BB function now spans the complete41 bytes through C0F0E3; surrounding metadata was retained. These checks do not execute the new routines or establish native SEH, original caller ABI or game behavior.
