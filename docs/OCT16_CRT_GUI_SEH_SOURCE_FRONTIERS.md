# CRT GUI and SEH4 Source frontiers, 2026-10-16

This is a read-only readiness checkpoint. It adds no reconstructed function,
Core registration, Ghidra mutation or runtime claim. The companion report pins
the worker evidence and independent integrator reviews. Local receipts are
immutable snapshots; their paths identify retained artifacts, not fresh live
queries. Descriptive Source names and proposed interfaces remain hypotheses.

## GUI path

`__encoded_null` at `00C04FD5` is a complete 9-byte, four-instruction leaf:
push zero, call the actual `00C04F67` encoder, pop the current outgoing pointer
word into ECX, and return. The proposed Source API borrows the existing
`NativeCrtPointerEncodeContext`. Its one extra caller-cleaned argument changes
the Source ABI. The proposed 17-byte, six-instruction naked body is a design,
not compiler-output evidence. Preserve the correct library name and stored
Ghidra `undefined __encoded_null(void)` prototype during eventual annotation.

The message-box parent at `00C2699A` has 445 physical code bytes and 164 starts.
Its current Ghidra body owns 439 bytes and 162 starts in three ranges. The
returning-Watson continuations at `00C26A3B` and `00C26ADA` each contain an
unowned three-byte `ADD ESP,14h`; no listing or body repair was performed.
Future physical-body Source coverage must state this distinction explicitly.

The parent is eligible for provided-domain Source after the encoded-null leaf
is implemented and independently accepted. Its other ordinary providers
already exist: actual encoder/decoder, OS-platform and major-version getters,
and canonical Watson. Both codec contexts must bind the coherent actual
TLS/PTD/CRT domain. The getters reuse `decoder.module_gate`; there is no new
error, Watson, resolver or support callback.

The accepted implementation proposal has twelve flat Source arguments: the
original text, caption and writable flags word; two existing codec references;
five references to the actual encoded cache cells `0109E994..0109E9A4`; and
references to the actual `LoadLibraryA`/`GetProcAddress` IAT cells `00CE2164`
and `00CE20F4`. The new caller cleans 48 bytes, versus the original 12. Stable
borrowed bindings do not turn the referenced native words into snapshots.

The reviewed stack schedule retains each current outgoing pointer word. The
third initialization encoder word remains underneath the platform getter.
Cleanup must pop the current output pointer, discard the extra support
reference with LEA, pop the prior encoder word, then discard the retained
encoder reference with LEA. The two station/information decodes similarly
retain both original argument words until their two pops. Added cleanup must
preserve live CMP/TEST flags. Actual compiler layout for `module_gate` still
needs verification during implementation.

Preserve the raw mandatory-cache zero gate, captured resolver, cache publication
order, saved encoded-null sentinel, fresh cache reloads, both decoder calls
before null testing, exactly three zeroed DWORD locals, uninitialized information
buffer/count, byte-only successful-information test, both returning-Watson
paths, mutation of the flags parameter itself, and final late argument reads.
The actual Watson header has `const wchar_t*` for its first three parameters;
one worker receipt abbreviates those qualifiers, and the pinned header governs.

Caller-supplied real owners, coherent codec bindings, actual API targets,
storage/stack validity and lifetimes remain required. Extra Source arguments
and stack placement exclude Original private-frame aliases, arbitrary writes
to added binding/control words and unproved fault/volatile-state equivalence.
Ordinary aliases such as text equal to caption remain allowed. No Source was
authorized by these readiness receipts.

## SEH4 path

The complete `00C07C00` prolog and `00C07C90` handler occupy 475 bytes and 146
starts. The handler allocates `14h` bytes, meaning twenty decimal bytes; after
four register saves its working ESP is entry ESP minus `24h`. A mandatory
immutable addendum corrects earlier wording without rewriting the receipt.

Five of the handler's six direct code targets have published Source. The
missing direct target is `00C16DD0`, the nonwritable-image validator. The fixed
read-only slot `00D6A618` additionally names actual internal destructor
`00C06AC8`; it is not a caller-selected callback. The handler checks the slot's
image properties before the indirect destructor call. Supplied initialized
canonical cookie/frame owners are valid preconditions, but do not close these
executable-code and relocation gaps.

The validator readiness audit covers 294 code bytes and 32 data bytes: its
187-byte physical body, two existing PE-query bodies totaling 107 bytes, the
28-byte scope table and a four-byte static cookie preimage. All 109 physical
instructions and sixteen control-flow sites were reviewed. The validator's
current ordinary body is 164 bytes/57 starts. Its excluded 20-byte/eight-start
AV filter and three-byte/one-start stack-restoring landing are physical code,
not assumed padding. Current Ghidra ownership was preserved.

`00C16D50` and `00C16D80` already have complete published implementations in
`native_crt_pe_image_queries.hpp/.cpp`. This corrects only the earlier assertion
that those children were missing. The validator still needs reconstruction.
It validates fixed image `00400000` before reading its argument, forms a
wrapping RVA without dereferencing the target, finds the actual section, and
returns the inverse writable characteristic bit. Its filter returns one only
for `C0000005`; its landing restores ESP from `[EBP-18h]` and follows the actual
zero/disarm/FS-restore exit. The cookie image value is analysis preimage,
not runtime truth.

The remaining exact-transport gates include the real SEH4 handler/destructor
code, Source scope and funclet relocations, the internal read-only slot,
current-image validation, SafeSEH membership and rebuilt caller integration.
A future MSVC compiler-SEH implementation is an unselected alternative whose
different runtime/cookie/frame policy would require explicit review; it cannot
silently substitute for the missing reconstructed handler. No new owner,
generic callback, Original-code bypass or default fatal policy is admitted.

These are static readiness findings. They establish neither compiled Source
emission nor fixture, Original binary-ABI, live SEH/UI or game behavior.
