# Native CRT local-unwind adapters (EA)

This packet reconstructs complete C0DC9A–C0DCB5 (28 bytes, 10 instructions)
and C0DD00–C0DD16 (23 bytes, 9 instructions). Both naked MSVC Win32 entries
call the actual complete `unwind_native_crt_local_scopes_00c0dbc4` provider
inherited from sealed DP `6bff416237087ea4961731b6e8933adfb2f5d53b`.
The source has no imports, mutable state, frame substitutes or indirect calls.
There are two real direct-call relocation operands and no other native-byte
changes. The report records final compiler, archive and static linked evidence.

DP source development and EA source inheritance were authorized while parent
and DW/DX acceptance of DP remained pending. EA acceptance and parent
integration require that dependency approval; neither the inherited commit nor
EA's own build supplies it. During final packaging the parent explicitly
accepted DP's source/static provenance after primary and independent DW/DX
reviews. That decision is retained in `dependency_acceptance.json`. Its
historical stale export-metadata name is disclosed there; the frozen DP
packet is unchanged. Actual runtime and parent integration remain separate.

## Native evidence and names

Fresh supported BSP queries verified the existing `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, x86 little-endian 32-bit language and
image base 00400000 before each batch. All 51 physical bytes agree between
the live database and the pinned installed PE (SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`).
The complete listing has 19 instructions, no omitted instructions, tables,
padding or indirect sites. The two call sites are C0DCAA and C0DD0B, both
targeting C0DBC4; the live report-call checker verifies each exact site.

C0DC9A was `LIBCRT_unmatched_00c0dc9a` with an inventory comment. Its
evidence-backed descriptive name is now `BSP_CrtLocalUnwindFromContext`,
explicitly provisional. No original symbol or context type is recovered.
C0DD00 retains the confirmed Visual Studio 2005 library name
`_EH4_LocalUnwind` and its original library-identification comment. Both
comments were appended under the Ghidra write lock, saved and read back.
The snapshot was explicitly refreshed before export so export metadata uses
the saved names rather than an older `functions.json` name. No prototype,
calling-convention, flow override or function-body repair was needed.

The fresh current function boundary at C0DD17 is `strtoxq`; an older 26-byte
physical capture of C0DD00 therefore extends three bytes into the following
function. Those bytes are not part of this 23-byte entry and are not claimed
as padding. C0DCB6 begins the separate `_EH4_CallFilterFunc` entry immediately
after the complete 28-byte context adapter. Current incoming xrefs show none
for C0DC9A and two calls to C0DD00 at C07DC7/C07E1C in `__except_handler4`.
That caller is not reconstructed or declared closed by this packet.

## Exact stack and register contracts

Let entry ESP be S and incoming EBP be B. Both adapters save B at S−4,
then push three real provider arguments and call the loop at ESP L=S−14h.
The loop receives P=[L+4], F=[L+8], T=[L+Ch], together with inherited EBP H.
P is the actual scope-decoding cookie pointer; the loop's registration cookie
separately uses the real canonical word at E15590. There is no extra binding
argument or context shim between either adapter and that loop.

| Entry | H | P | F | T | Original return cleanup |
|---|---|---|---|---|---|
| C0DC9A | DWORD at context+0 | current DWORD at context+28h | current DWORD at context+18h | current DWORD at context+1Ch | RET 4; final ESP=S+8 |
| C0DD00 | original stack word [S+4] | original stack word [S+8] | incoming ECX | incoming EDX | RET 8; final ESP=S+Ch |

C0DC9A loads ECX from [S+4] only after saving B, then loads H from [ECX].
It reads and pushes +1Ch, +18h, +28h in precisely that order, retaining
alias-visible current loads and stack writes. The public `const void*`
declaration is an opaque actual context pointer, not a copied portable view.
The installed MSVC 14.51 x86 `setjmp.h` branch lays out `_JUMP_BUFFER.Ebp`
at 0, `Registration` at 18h, `TryLevel` at 1Ch and `UnwindData[0]` at 28h.
That is a qualified current-header comparison, not proof of the original
type/name. The unrelated x64 layout found by an initial generic struct
search was rejected before the explicit `_M_IX86` branch inspection.

C0DD00 uses a four-parameter fastcall declaration. ECX and EDX are the first
two words; only frame and cookie pointer occupy caller stack words. After
PUSH EBP / PUSH EDX / PUSH ECX, `PUSH [ESP+14h]` reads the original [S+8].
No selector, validation, register scratch or synthetic argument is inserted.

On normal return, each adapter adds 0Ch to remove its three loop arguments,
pops B and performs its original immediate RET. This preserves the loop's
unusual internal epilog, which skips its own saved EBP rather than popping
it: the adapter performs the actual outer EBP restoration. EBX/ESI/EDI
follow the real loop/provider contracts. EAX/ECX/EDX retain raw provider
effects; there is no invented return value. Final arithmetic flags come from
the actual ADD ESP,0Ch. The remaining POP/RET instructions leave flags
unchanged. DF is neither changed nor normalized. All reached providers
retain their own DF and native-frame requirements.

## Owning dependencies and static image boundary

The exact dependency header is `bsp/native_crt_seh4_nested_handler.hpp`.
Its actual loop and nested handler use the accepted canonical no-extra-word
DK cookie checker/reporter/hook, DJ NLG notifier, CU raw cleanup call and
DD process-lifetime canonical data owner with the retained RO pointer pair.
The old CT borrowed-context checker is not used. Normal cleanup must be an
actual native funclet with a valid inherited frame and register/stack domain.
Nothing here creates native OS registration records except the real DP loop.

The inherited DP CMake/MASM artifact explicitly supplies metadata to
`bsp_game` and anchors the real handler archive symbol. EA does not alter
that recipe or mitigation policy. Its current object, generated project,
MASM source/read/write evidence and actual SafeSEH table are checked. A
never-executed forced static image additionally roots both real adapters,
their actual DP/provider chain and the canonical owner. Every adapter byte
must equal the native byte outside its single four-byte REL32 operand, and
each linked operand must target the current real DP loop. Ordinary
`bsp_game` has no EA caller, so it may omit these two uncalled adapter
sections; its presence of the real nested handler is checked separately.

Neither image is executed. Static SafeSEH membership is a module admission
artifact; it does not validate runtime FS chains, OS exception dispatch,
cookie initialization timing, native caller frames or actual funclet behavior.
Faults/nonlocal exits may retain the real provider's partially published
scope/FS/global effects. No catch, rollback, termination replacement,
no-return pruning or new failure continuation is introduced. Original code
addresses are retained as evidence, not installed entry locations.

## Validation and retention

The final report binds all original/native/source instructions, both current
COFF definitions, current source/header/command/read/all matching write
groups and exact /Fo object, unique current archive members, provider symbols,
linked bytes, MAP addresses and real SafeSEH table. Current generated ML
options/read/write/object evidence is distinguished from DP's earlier actual
diagnostic ML command; EA does not claim to have captured a new diagnostic
ML invocation. Strict Win32 build, eight original seed checks and the two
existing CTests are required. No new repository tests, handler/cleanup/unwind,
NLG/cookie failure, original code or game execution is performed.

All failed or rejected checker attempts remain evidence. Frozen local and
reported external path sets are hashed completely twice using SHA256 and
SHA512. The final report, inventory manifests, clean commit and handoff have
separate pins. Current source/native/link agreement is not gameplay proof.
