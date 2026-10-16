# Actual default scalar-tree destruction

`destroy_native_input_default_scalar_tree_006a1df0` reconstructs the whole
`006A1DF0..006A1E23` ordinary entry: 52 bytes and 23 instruction starts.
`STL_Tree_Destroy_006a1df0` remains the established library identity. The new
descriptor is provisional: the settings producer initializes this 12-byte tree
at `InputSettings+54h`, with 18h-byte nodes and the nil byte at node+15h.
Existing AV, typed, full-prefix, and whole-range interfaces remain separate.

Native inputs are ECX=tree and unconsumed EDX, with no stacked arguments, plain
RET, and EAX=0. The Source API is `uint32_t __fastcall(void*, uint32_t)` and
retains those input and return slots. Its private frame is a new C++ frame.

The complete native evidence is `006A1E06 CALL 0069FE70` and
`006A1E0F CALL 00BF65AC`. Read-only caller evidence is `00C7F9B5 JMP 006A1DF0`
and `00C7FA95 JMP 006A1DF0`. Both unwind callers load saved
settings and add 54h. They are support evidence, outside this packet's ownership.

The implementation captures tree+4, then that captured head's left link. It
passes a private eight-byte output, the same tree as both iterator owners, and
those two captured endpoints to the actual whole `0069FE70` service. Its EAX
result is ignored. The child EDX slot is explicitly zero; native ambient EDX
bits are unconsumed and are not reproduced.

After the child returns, the implementation reloads CURRENT tree+4 and frees
that pointer through `singleton_lifetime_free`, including a zero pointer. It
then clears tree+4 before tree+8 and returns zero. It never clears opaque+0,
guards the tree/head, frees a cached head, substitutes a container or callback,
or adds cleanup, rollback, allocation, or revalidation. All accesses are raw
volatile DWORD reads/stores with unsigned 32-bit address wrap.

The child is the published actual Source from `3e60c88b8`, directly composed
with accepted whole `005572F0` and its `0086EE50` implementation. It owns the
established diagnostic, iterator-mutation, erasure, and output-alias progression.
A child exception propagates before this entry's free or final stores. This
entry has no local native EH state, handler, or catch and introduces none.
The private output is not read, and this packet does not construct a host node.

The default MSVC Win32 CMake build passed all three existing suites in 14.20
seconds of CTest time. No new tests, ad-hoc executable, native fixture, or game
execution was added. The existing unrelated LNK4006 duplicate spawn helper
warning remained nonfatal.

The final report verifier passed four JSON transfer rows and four Markdown
claims, with no failed or unchecked claims. Address-only ledger additions,
index refresh, frozen-provider hashes, and staged diff checks accompany the pin.

The entire actual CMake object was inspected: five functions, 112 code bytes,
52 instructions. Its public function is 62 bytes/26 instructions. The only
public relocations call the actual range service and `singleton_lifetime_free`.
The initial head load at 0009h precedes head.left at 000Eh; the range call is
0018h, CURRENT head reload 001Dh, and free call 0021h. Ordered clears are 0029h
and 0030h, followed by EAX=0 and plain RET. Four emitted private integer helpers
total 50 bytes/26 instructions and contain no calls. Nine focused checks pass.

Source is not instruction-identical to the native entry. The compiler pushes
some private metadata before the left-link read and clears EAX after the zero
stores. The private eight-byte frame and child argument frames are not the
original frames. Valid reached tree/head/node storage, compatible current-CRT
allocation ownership, and a tree writable through the final clears are required.
Unused registers/flags, original recursive origins, original heap/CRT/FH3/SEH/
RTTI/fault behavior, binary replacement, and game behavior remain unverified.
No floating-point/x87 operation or new NaN handling occurs in this entry.

The JSON report retains PE/live bytes, every current instruction start,
callee and producer fingerprints, the complete emitted functions/relocations,
and build evidence. The branch was synced only to published Main `7081f8240`
before authoring; registration is one local `bsp_core` append. Primary review
of the frozen pin remains pending; preliminary full Source/native review found
no discrepancy. Parent `006AA460`, preset/controller members, and their partial
range implementations are outside this Source packet.
