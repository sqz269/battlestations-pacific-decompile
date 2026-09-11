# Native renderer viewport and clear

This packet reconstructs complete B26770 (290 bytes), B21430 (171 bytes), and
four 4-byte viewport leaves B1F730/B1F740/B1F7B0/B1F7D0: **477 owned original
bytes**. Original renderer table D5F0A8 has Clear at slot +08 and BindViewport
at slot +A4. The names describe observed behavior, not recovered source names.

Fresh guarded queries against `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, match the installed PE for 22 bounded spans,
925 bytes including both complete parent bodies, leaves, EH records, completed
child-provider spans, globals and renderer-table slots. The installed PE SHA256
is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Exact spans and artifact pins accompany the tracked audit.

## Interfaces and storage

Both original parents receive the renderer in ECX. BindViewport has one stacked
viewport argument and `RET 4`. Clear has six stacked arguments in this order:
count, rectangle pointer, flags, pointer to a color DWORD, by-value float depth,
stencil; it returns with `RET 18h`. The new fastcall interfaces additionally
receive `NativeRendererViewportClearContext*` in EDX and expose no semantic
return value. The 8-byte context borrows immutable pointers to the actual
synchronization-global storage and the original readonly 1.0 DWORD D7A24C.
Both referenced objects must outlive the call. Clear does not use that constant.

The four raw leaves preserve their entire original instruction bytes. They
return origin address `viewport+08`, size address `viewport+10`, scissor rectangle
address `viewport+24`, or the raw byte at `viewport+20`. Address getters use
32-bit LEA, including native wrap behavior, without dereferencing their input.
The byte getter returns AL without modifying EAX's other bits. Its interface
uses `uint8_t`, not bool. No viewport constructor, retained type, owner lifetime
or compatibility with a semantic viewport class is claimed by these leaves.

The renderer must contain its real tracked-critical-section pointer at +04 and
current real D3D9 device at +1A10. The completed synchronization/state providers
retain their existing contracts. Renderer+1904 is a borrowed viewport pointer;
the original body performs no retain/release around its publication. The new
code does not cast whole semantic renderer or viewport overlays.

## Preserved ordering

BindViewport enters the optional guard before loading its actual callee viewport
argument word. It arms cleanup after that load and before the first getter.
It calls the origin getter twice and size getter twice, separately loading the
four raw DWORDs into outgoing D3DVIEWPORT9 storage. It captures the current
device before storing MinZ with MOVSS, reads D7A24C with MOVSS for MaxZ, then
reads the current device table and calls slot +BC (SetViewport). No C++ float
conversion, x87 load or coefficient snapshot replaces these operations.

Only after SetViewport returns does the body increment the current DWORD at
+1BD0 and publish its captured viewport pointer to +1904. It zero-extends the
raw +20 byte for state **0xAE**, calls the complete cached-state provider, then
reads the byte again. If it is now nonzero, it captures the current device's
table **before** calling the rectangle getter. It then loads slot +12C from
that captured table, reloads the current renderer device, and calls
SetScissorRect with the getter's original pointer. This precise table/getter/
current-device sequence is checked in the actual built body.

Clear captures flags before guard entry. Zero flags return without dereferencing
the context, renderer device, color or depth. With nonzero flags, it enters the
guard and only then performs `FLD` from the **actual by-value callee depth slot**.
It loads current device, stencil, table, color pointer and color DWORD in native
order. The `FSTP` into outgoing depth occurs after color loading and before the
later rectangle/count loads. Signaling-NaN quieting and x87 status therefore
occur at the original load/store boundary. Flags retain their pre-entry value;
the other arguments remain live until their prescribed loads.

A private local record holds these native outgoing argument values. The code
pushes all seven COM arguments, then sets its volatile cleanup flag immediately
before the indirect Clear call at table +AC. Cleanup is not armed during input
reads, x87 transfer or outgoing stack construction. Only a normal return
increments current renderer+1BD4. Both parents ignore HRESULT for continuation.

At normal exit, both read the current synchronization mode before disarming
cleanup, then call the complete leave provider if that captured mode is nonzero.
The private armed flag is volatile so this order remains present in MSVC object
code. Exception cleanup uses the complete current-gate guard destructor. A
second C++ exception during cleanup terminates, matching the existing provider
contract. Skipped entry leaves original guard fields uninitialized; a later
mode flip that requires those fields is not repaired or given a fabricated
renderer. Such invalid inherited guard domains remain unsupported.

Each public naked adapter passes the **address of its own original callee
argument block** into its full private body. It does not snapshot values or
dereference the context. The source `float depth` is already a by-value callee
argument; it is not a camera+18C field, external float reference, or promise to
delay the caller's prior expression evaluation. Fixture guard callbacks mutate
these actual callee slots before their loads. New private stack frames and the
outgoing-value record are explicit interface differences; arbitrary references
to old private scratch addresses are not preserved.

## EH and validation

The original viewport handler thunk CBD148 selects FuncInfo DF5848, whose sole
state-0 unwind entry DF5840 calls funclet CBD140. That funclet addresses the
guard at handler EBP-2C and tail-jumps to B21110. Clear uses CBCCF8, FuncInfo
DF5240, unwind entry DF5238, and funclet CBCCF0 with EBP-14. Both records have
magic 19930522, one unwind state, transition to -1, no catch maps, and flag 1.
Both original thunks tail-jump to BF6B43. These complete bytes and references
are freshly captured; no Ghidra definitions or shared metadata were changed.

Strict MSVC Win32 Release `/W4 /WX /fp:strict` build, both existing CTests and
all eight native seeds passed. The packet is included only by an ignored
extra-source CMake hook. Existing semantic APIs, shared CMake, ledgers, saved
Ghidra analysis and installed game files are untouched. No permanent tests were
added.

The ignored fixture executes the complete original six owned bodies with their
original two EH records registered against the complete actual built library.
The Clear exception case exercises its state-0 unwind path. Thirty-one
declared address-operand relocations adapt the original code/data to isolated
memory and complete guard/state ABI bridges. Original EH thunks dispatch to
the real host `__CxxFrameHandler3`; this is explicitly original-owned-body /
actual-library-provider composition, not original descendant-machine-code or
general original-SEH-ABI equivalence.

Five paired runs use two hidden real HAL D3D9 devices, actual COM methods and
real tracked Win32 critical sections. Their bounded observers forward real
calls before mutating the actual viewport/depth/color/stencil argument words,
current device, raw scissor byte or counters. They cover viewport/scissor,
post-state scissor disabling, zero-flags clear with invalid unused context and
pointers plus signaling depth, normal nonzero clear, and an exception after a
real nonzero Clear call. The final pair leaves the callback-written counter
unchanged and drains the real guard; its normal twin increments that same
current counter. The raw initial flags remain 1 even when entry changes the
callee flags slot to 0.

All five pairs match **381,936 bytes** of complete callback/pre/post renderer,
input, global, lock and device-query traces without tolerances. Fourteen real
COM calls return actual S_OK; 24 real Enter/Leave calls are observed. Normal
returns preserve stack cleanup and nonvolatile registers. The x87 stack remains
empty and control word stays 037F. The real viewport path sets MXCSR precision
status to 1FA0 on this HAL, identically on both sides. Clear's masked signaling
depth becomes 7FC00123 with x87 invalid status 0001; zero flags leave it unloaded
with status 0000. No HRESULT-failure or unmasked-FP-trap claim is made.

The final library SHA256 is
`f2353009a9500b802163586bddf80fac6590413a674ea3f82df309e2d856cb8a`.
Three exact archive members supply the two parents, four leaves, cached state
and synchronization. All 355 mapped local COFF sections and 1,457 relocations
match linked bytes; all 338 immutable sections match runtime images. The 199
mapped function entries include 22 library functions and 177 fixture functions,
including inline COMDATs after stripping every leading map `f`/`i` token.
Whole section proofs include compiler padding/data without calling it code.
Eleven postimage phases preserve all 27 captured code/data/table spans. Loader
IAT writes and the two bounded OS observer substitutions are explicit. External
module identities and function-entry bytes are verified; two writable CRT data
imports carry identity/live-value evidence only.

Final immutable evidence is in the worker's `local/viewport_clear/`, with the
actual final archive and exact objects under `frozen_final/`. The earlier
unsealed `frozen/` draft is not part of the final evidence. This packet is
reconstructed, build-tested and fixture-tested, with explicit new interfaces;
it is not a binary drop-in replacement, parent-frame integration or gameplay
validation. Malformed raw objects, invalid guard lifetimes, private stack
aliasing, asynchronous races and every exception domain are not asserted to be
covered by the focused runtime cases.


## Primary main-library integration

All entries are registered in main. Strict MSVC Win32 compilation, both existing
CTests and all eight native seeds passed.

The primary verified 578 worker artifacts and all four owned files.
Twenty-two fresh guarded spans matched 925 bytes, including 477 owned bytes.
The unchanged fixture linked the actual main archive and passed all five
original/library pairs: 381,936 state bytes, fourteen real S_OK COM calls and
twenty-four actual OS guard calls. Actual callee-slot mutations, raw0x80 state,
current-device/scissor reload, flags-zero skipped reads, normal counter update
and post-real-Clear exception suppression/cleanup all passed.

Three exact archive members and 355 complete COFF sections matched linked
bytes after all 1,457 relocations. All 199 local functions (22 library and
177 fixture, including f/i map entries), 338 immutable runtime sections and
eleven whole postimages of twenty-seven spans passed. The two native EH
descriptors are pinned; the Clear state0 cleanup executes through actual host
FH3 and the complete reconstructed guard bridge. This remains a scoped
original-C++-exception composition, without a general native SEH ABI claim.

The actual main library SHA256 is `afebfdb70e5013aa459ff7bdd15a27db70cc2e28126a6ce3229e43375a5f68ac`.
The read-only primary bundle is `local/viewport_clear_primary/`, seal
`aea730879ae06b55ec0024d472e45eb91a14e8c442bc2f441f3352293f38afbd`. Reviewed evidence was appended to preserved
Ghidra names/comments, saved, exported and registered in the sharded ledger.
Full original caller ABI and gameplay remain unvalidated.
