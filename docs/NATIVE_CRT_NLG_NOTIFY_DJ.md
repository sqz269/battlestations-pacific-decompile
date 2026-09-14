# Complete native NLG notify leaf DJ

`src/native_crt_nlg_notify.cpp` reconstructs the complete original
`00C16879..00C16897` `__NLG_Notify`: 31 bytes, 16 instructions, no missing
tail or additional binding argument. It requires the accepted DD canonical
owner from candidate `6c44a02505c4fff9faf5e8b2d01a70697cc2f1ba`. The native
entry addresses the actual E16830 descriptor directly. No descriptor copy,
accessor call, host callback or shadow global substitutes for that storage.

The matching full installed PE is 12,223,752 bytes, SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every live CLI query verifies project `bsp` (configured existing
`C:/Users/sqz269/bsp.gpr`), `/battlestationspacific.exe`, Win32 x86 and
image base 400000h. Fresh live and PE bytes agree for the entire leaf,
descriptor and four complete defined caller bodies. All leaf/caller live
instruction addresses agree with independent PE decoding. Ghidra database
bytes are saved-image evidence, not an observation of a running game.

## Original register and stack contract

Let entry ESP be S, EAX=A, EBP=H, EBX=B, ECX=C and [S+4]=K. A is the raw
destination value, H the inherited native frame and K the sole original
stacked code DWORD. The C++ declaration uses that one stdcall code word;
the caller must separately establish EAX/EBP and the native stack. An
ordinary C++ call does not establish this register contract.

| Address | Bytes | Instruction and effect |
|---|---|---|
| C16879 | 53 | PUSH EBX: [S-4]=B |
| C1687A | 51 | PUSH ECX: [S-8]=C |
| C1687B | BB3068E100 | MOV EBX,E16830: actual descriptor |
| C16880 | 8B4C240C | MOV ECX,[ESP+Ch]: read K from S+4 |
| C16884 | 894B08 | MOV [EBX+8],ECX: code=K |
| C16887 | 894304 | MOV [EBX+4],EAX: destination=A |
| C1688A | 896B0C | MOV [EBX+Ch],EBP: frame=H |
| C1688D | 55 | PUSH EBP: [S-Ch]=H |
| C1688E | 51 | PUSH ECX: [S-10h]=K |
| C1688F | 50 | PUSH EAX: [S-14h]=A |
| C16890 | 58 | POP EAX: A restored |
| C16891 | 59 | POP ECX: K reloaded |
| C16892 | 5D | POP EBP: H restored |
| C16893 | 59 | POP ECX: C restored |
| C16894 | 5B | POP EBX: B restored |
| C16895 | C20400 | RET 4: return through [S], final ESP=S+8 |

All general registers apart from the required ESP advancement survive an
ordinary return. MOV, these register PUSH/POP operations and RET do not
change flags; there is no compiler prologue/epilogue, stack adjustment ADD,
flag save/restore, exception wrapper or DF normalization. The exact three
temporary stack writes and subsequent POPs remain. EDX/ESI/EDI are untouched.

The descriptor fields are signature/+0, destination/+4, code/+8 and frame/+C.
The signature is untouched. Writes occur code, destination, frame; no lock,
memory barrier, validation, allocation or callback is present. Native faults
or nonlocal exits can expose partial stores and do not promise restoration.
The leaf adds no thread serialization or debugger registration behavior.

## Actual data provider and incoming calls

`GameNativeCanonicalDataOwner` is a required precondition, not called by this
leaf. At the accepted base it maps actual RW E16000 from full PE page bytes,
checks E16830 signature 19930520h and zero initial fields, and publishes a
process-lifetime owner together with the other accepted RO/RW pages. Its
`nlg_descriptor()` returns a reference to actual E16830. The leaf's layout
assertions use that owner's existing descriptor type. The owner header and
implementation are unchanged. This packet does not repeat DD's controlled
child runtime probe or promote the owner's evidence to gameplay proof.

| Defined caller | Call site and original inputs | Concrete dependency after call |
|---|---|---|
| C0DBC4 `__local_unwind4` | C0DC32: PUSH101h, EAX=[EBX+8], inherited EBP | Reload cleanup through preserved EBX; set ECX=1; original RET4 consumed code |
| C0DCCD `_EH4_TransferToHandler` | C0DCD5: EBP=EDX frame, ESI=ECX handler, EAX=ECX, PUSH1 | ESI/EBP survive while five other registers are zeroed, then JMP ESI |
| C03570 saved longjmp-internal match | C035FA: EBP from saved buffer, EAX=[EBX+14h], PUSH0 | Preserved EBX identifies buffer; EBP remains saved frame; buffer restores final registers/stack |
| C167C9 `__local_unwind2` | C1682B: PUSH101h, EAX=[EBX+ESI*4+8], inherited EBP | Reload through EBX/ESI, preserve enclosing-level ECX for actual cleanup call |

Fresh xrefs also report physical CALL BFB993 in uncontained code. Bounded
PE/live bytes and aligned BFB98A..BFB9AB instructions show LEA ECX,[ESI*3],
PUSH1, EAX=[EDI+ECX*4+8], CALL C16879, then another table read using preserved
ECX. Its enclosing frame/control-flow is not recovered here. The lookup's
nearest `_strncat` candidate is not treated as a valid containing function.
The earlier BFB97B raw decode starts mid-instruction and is rejected as an
instruction listing; its complete byte capture is retained, along with the
correctly aligned bounded decode. No fifth parent is claimed reconstructed.

## Validation and evidence boundaries

The strict MSVC 19.51 Win32 build passed with /W4 /WX /fp:strict; all eight
original seed comparisons and both existing CTests passed. No new runtime
test is added. A local static link artifact forces extraction of the leaf
and the actual canonical-owner archive member. It is never executed. The
object, extracted archive member and linked PE leaf are audited against all
31 original bytes, including the fixed E16830 immediate and RET4; linked
provider ownership is checked in the MAP. Compiler commands, COFF sections,
symbols, relocations, full archive membership and disassembly are retained.
The leaf occupies exactly the 31-byte COFF section 7 with zero relocations;
the audit link places those identical bytes at 10001020h. The canonical
owner, read-only mapper and real cookie initializer resolve from their
accepted bsp_core members in the same audit image. These are static link
facts; the audit does not initialize that image's data owner or call the leaf.

The function is available in `bsp_core.lib`. The ordinary rebuilt game has
no reconstructed caller for this new leaf, so its linker may omit the member;
the static force-link audit is explicitly separate from game linkage. No
original NLG, cleanup, failure, unwind or game entry is executed by DJ. The
required existing math tests retain their ordinary scope. Full local-unwind,
native handler/caller composition and debugger symbol/address integration
remain unimplemented or unvalidated. Exact emitted leaf bytes establish this
bounded instruction/register ABI only, not replacement at original code VA,
native exception-path closure, runtime notification or gameplay validation.

The existing correct Ghidra library name is preserved. The owned C16879
comment appends the evidence while retaining the old library comment; the
write-locked annotation tool records the prior values, saves the project,
and the comment and refreshed export are read back. E16830 data metadata
and all other addresses remain outside this packet's mutations.

`reports/native_crt_nlg_notify_dj.json` records exact source, original PE and
artifact pins. The packet-local recursive evidence and external artifact
set are frozen with SHA256 and SHA512 in two inventories. The inventory
files exclude only themselves from recursion and are separately pinned in
the worker delivery. All rejected attempts retained in the bundle are
listed and distinguished from accepted evidence.
