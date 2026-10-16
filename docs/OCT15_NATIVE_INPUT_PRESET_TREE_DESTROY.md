# Actual preset-tree destruction family

This packet reconstructs whole ordinary entries `006A2060..006A2093`,
`006A24B0..006A24E3` and `006A2B00..006A2B33`: each 52 bytes/23 instructions,
156 bytes/69 starts total. Their established `STL_Tree_Destroy_...` library
identities remain. The three descriptive Source APIs and preset interpretation
are provisional; existing AV overloads retain their separate contracts.

Native inputs are ECX=tree and unconsumed EDX, with no stacked arguments,
plain RET and EAX=0. Each Source API is `uint32_t __fastcall(void*, uint32_t,
NativeStringRawPoolContext&)`; the added actual borrowed context gives RET4.
The real accepted `NativeInputPresetTreeIterator` carrier is eight bytes.
Native private output/argument aliases and the original C++ declaration are
not validated by this new physical Source interface.

Each entry captures initial head from tree+4, then that head's left link,
before the actual published whole preset range service. It passes the same
tree as both iterator owners, the captured left/head endpoints, a private
eight-byte output which is ignored, and the same RawCtx reference. The child
EDX is explicitly zero; original unconsumed EDX residue is not reproduced.
The complete actual range retains its diagnostics, partial path, iterator
mutation, pool ownership, output aliases and error transport.

Only after child success does each entry reload CURRENT tree+4 and pass it to
the actual current-CRT free, including a zero pointer. It then clears tree+4
BEFORE tree+8 and returns DWORD zero. Opaque+0 remains untouched. There is no
cached-head free, ownership/null/count guard, count shortcut, extra key/pair
cleanup, rollback or local EH. A range exception retains child progress and
prevents this entry's free and final stores. Tree storage must remain writable
through those post-free clears. Raw volatile DWORD accesses use unsigned
32-bit address wrap; there is no node overlay or host ownership model.

The six owned native transfers are:

- `006A2076 CALL 006A1AA0` — actual whole preset range.
- `006A207F CALL 00BF65AC` — CURRENT head free.
- `006A24C6 CALL 006A1AA0` — actual whole preset range.
- `006A24CF CALL 00BF65AC` — CURRENT head free.
- `006A2B16 CALL 006A1AA0` — actual whole preset range.
- `006A2B1F CALL 00BF65AC` — CURRENT head free.

Read-only origins `00C7F994 JMP 006A2B00` and `00C7FA74 JMP 006A2B00`
belong to the native unwind scopes at C7F98E/C7FA6E. They load saved objects
from runtime EBP-18h/EBP-10h, add 24h, and tail to the ordinary entry. Their
22 bytes/six instructions are support only. No action, public EBP reader,
synthetic frame, member or parent owner is reconstructed here.

Fresh PE/live/current-listing proof covers 156 owned bytes/69 starts, plus
the read-only published default reference's 52 bytes/23 starts and the two
origins: 230 bytes/98 starts, ten external targets. Every body is complete.
All 52 bytes of each candidate normalize to actual `006A1DF0` after only its
two separately resolved E8 operand fields: eight mask bytes and 44 untouched
bytes. No branch, opcode, field, padding or broad address mask is used. The
Source bindings remain the actual preset range and CRT free; the default
reference is not delegated to.

Source work began only after a fresh remote ancestry check proved accepted
range pin `7c6578fe637f8af50bd79bdc91600c24fd8064d4` on published Main
`d15ba1ba6d9faec2b71e73fa0d4c5dd30c56bda2`. The earlier false ancestry check
kept preparation read-only. The branch then fast-forwarded to that exact Main
before claiming the three addresses and four output files.

The default MSVC Win32 build passed all three existing suites in 10.71 seconds
of CTest time. No tests, ad-hoc executable, native fixture or game run were
added. The existing duplicate spawn-helper LNK4006 warning remained nonfatal.
The verifier checks six owned CALLs and two explicitly read-only tail JMPs
in both JSON and Markdown.

The entire actual CMake object was inspected: seven functions, 251 code bytes
and 107 instructions. Three public bodies are each 67 bytes/27 instructions;
four raw integer helpers total 50 bytes/26 instructions. All three public
bodies have identical bytes, but every actual relocation is independently
checked: six REL32/addend-zero bindings to preset range/free. The complete
default compiled reference matches after the exact added-context push and
RET4-versus-RET edits; all four private helpers match without masks. There is
no code padding, hidden call, local EH, FS access, cookie or cleanup state.
All section data and directives are retained; the compiler's unused four-byte
weak BSS fallback has no code references and is recorded as uninitialized
storage rather than file payload. Eleven focused COFF checks pass.

Emitted head/head.left reads at 0Ch/11h precede the range at 1Bh. CURRENT head
reload at 20h precedes free at 24h. Ordered clears are 2Ch/33h, then EAX=0 at
3Ah and RET4 at 40h. Native computes EAX=0 BEFORE the clears. Added context
and metadata pushes also interleave the Source reads differently. Memory
capture/store progression is preserved; original frame, fault, flag and
incidental register timing are qualified.

All direct bindings use the published actual range and current-CRT free.
The report fingerprints their complete accepted transitive closure and keeps
comparison/AV files separate from actual call targets. Original heap/CRT/
FH3/SEH/RTTI/fault behavior, private frame/argument aliases, recursive origins,
CurrentCpp/noexcept/termination, binary replacement and game equivalence remain
unproved. No x87 operation or new NaN policy occurs. Registration is one local
bsp_core append, without a shared registry lease. Root preliminary full Source,
Native and actual COFF review passed 14 checks with no findings; primary review
of the immutable pin remains pending. No caller, action or `006AA460` owner
is promoted.
