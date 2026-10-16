# Raw size-argument API for the existing __callnewh Source

Original address: `00C055B1`, `__callnewh`, complete 34 bytes / 15 instructions.
This packet adds **one Source API and zero Original functions**. The existing
whole-body implementation was already published by `c15c994d4eddc29081c1b3c4b74cbeb4bfc1ead8`
in `native_crt_new_handler.hpp/.cpp`; its API, body and historical evidence stay
unchanged. The prior report is `reports/oct16_native_crt_new_handler.json`.

The added `call_native_crt_new_handler_00c055b1` receives an actual four-byte
size-argument backing pointer, the existing `NativeCrtPointerDecodeContext`, and
the same borrowed current volatile encoded-handler word corresponding to
`0109DE44`. It captures that word and invokes the genuine existing
`native_crt_decode_pointer_00c04fde` before accessing any size-backing byte.
Decoded null returns zero without a size load. Otherwise one unchecked x86
DWORD read obtains the current size, including unaligned storage. The actual
decoded `int32 __cdecl(uint32)` handler receives the captured size; every
nonzero full result becomes one. There is no new generic callback binding.

The old `invoke_native_crt_new_handler_00c055b1` reads its own new Source value
parameter late. That published interface remains available. The additive API
makes the caller-supplied live backing explicit, including the no-backing-read
null-result path. Neither interface is the Original binary entry.

## Native order and ABI

The native sequence is current encoded-word PUSH, `C04FDE` CALL, TEST EAX,
POP ECX, null branch, current `[ESP+4]` size PUSH, actual CALL EAX, full TEST,
POP ECX, then exact zero/one returns. The decoder's first outgoing argument
word is writable under its existing contract; the parent consumes the returned
pointer and discards that temporary word. The Source call uses that existing
decoder API, including its extra context and physical argument-slot limits.

The Original has one cdecl size word, caller-pop4/plain RET, EAX zero or one.
The new Source interface has a backing pointer and two reference arguments,
caller-pop12. It does not establish native parameter/decoder-slot layout,
incidental ECX/EDX/EFLAGS, fault locations, stack aliasing, SEH/FH3 or drop-in
binary compatibility. The native body matches the installed PE and live Ghidra
bytes; its SHA-256 is
`ee46ba523d066e7aa16ef3d636e1869ecfeab38fec0c26e9b02266b6e44f42e1`.

## Admission and runtime limits

Added binding/bookkeeping, including compiler-private argument and temporary
backing, remains valid and stable and disjoint from reached actual size/native
cells. Real native contents retain admitted aliasing and reentrancy, and remain
mutable through real providers. Only a nonnull decoded handler requires four readable
size bytes and compatible callable native code. Inline assembly supplies the
raw x86 DWORD load, with no null/alignment/range guard or input normalization.
Decoded null permits absent backing and skips its read entirely.

All existing decoder and module-gate admissions remain: the actual current TLS
indices, actual IAT reference, actual PTD/provider storage and callable ABI, and
the established owning-errno/invalid-handler support context. This packet adds
no TLS, heap, encoded-handler, errno or startup owner and does not close their
Original SCC. It uses no SDK `_callnewh`, invented callback, lock, catch, retry,
LastError repair, cleanup or nonnull seeded state. No general runtime, Original
entry ABI, SEH or game validation is claimed.

## Preservation and verification

Baseline CPP SHA-256:
`d7f29cd0d22f12b44d729ea32546a394fe6f0a18ac7c3fd0e9f7b92b3ae1254a`.
Baseline HPP SHA-256:
`30d7e608299e659ea1b8c2cdea310c003f2d865c34235beb33b6b0ca9de725a3`.
The entire old method's exact raw Source bytes, excluding its following newline,
remain present unchanged; SHA-256:
`4a035e0c3e90cdcfcb045def3f32eb83e4b77aa247877ed5b87bca3512bd00bc`.
The additions preserve the entire existing method and public declaration.

The new report records before/after file hashes, the complete native body,
current direct/indirect transfer evidence, and seven true incoming CALL sites.
The three BFDE-range sites belong to current formal owner `00BFDCEC`, rather
than an inferred enclosing function at `00BFDE30`.

Primary full native/Source review passed. Its requested header-comment
clarification was applied without changing the CPP or API. Correct library
identity `__callnewh` and all captured prior Source53 comment text were retained
through the write-locked annotation tool, which recorded prior values and saved
the project; affected exports were refreshed. The ledger extends the same
Original address and record with one additional Source interface.

The primary's default `scripts/build.ps1` Release Win32 gate passed at
`d9e1fce24cc7b7475668db7d14245958ef0d5237`; all three existing checks passed
in 8.90 seconds. The tested CPP/HPP hashes equal the current Source hashes.
These regression checks do not execute the new helper's handler branches.

Independent whole-object inspection passed. The old public method's exact
45 CODE bytes / 22 instructions and sole decoder REL32 relocation at offset
`0D` equal the immutable published actual COFF object, SHA-256
`0266e6b2d3cfe7fc43822facfa18b1784b835731c1d964fff31908f52981259c`,
whose verified archive is in the movie-factory worktree's
`local/accepted_source53_crt_objects/` directory. Historical evidence was not
changed.

The retained current object has SHA-256
`5adba1574dbf31a4f42e63f01df55d7a160f86e7a0265d751c1af939d21d9fd3`.
Its complete CODE is exactly the old method plus the new 52-byte / 24-instruction
API: 97 bytes / 46 instructions total, two decoder REL32 relocations, no other
CODE helpers, EH/BSS/runtime data, padding or unclassified file gaps. The
linker directives, compiler debug metadata and checksum section are classified.

The new body captures decoded EAX into ECX before decoder-argument cleanup.
Its null return precedes both the size-pointer load and the raw DWORD read.
The nonnull path reads the current backing at `+1F`, stores the captured size
in compiler-private `[EBP+10]` at `+21`, pushes it and invokes the captured ECX
handler at `+27`. That temporary reuses the dead Source encoded-reference
argument word; it does not store through the actual global reference. This
illustrates the explicit private-bookkeeping disjointness admission. After
caller cleanup, NEG/SBB/NEG normalizes the complete EAX result to zero or one.

The report pins the read-only independent COFF parser and full-object receipt.
No CMake append, worker build or new test was performed; the translation unit
was already registered. Qualified ordinary Source readiness is established;
the runtime, Original entry/SEH/ABI and owner-closure limits above remain open.
