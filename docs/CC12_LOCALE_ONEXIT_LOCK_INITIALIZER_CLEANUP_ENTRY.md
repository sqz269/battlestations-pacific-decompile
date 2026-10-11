# Locale initializer numeric target entry at C11C18

The accepted caller at `00C11C0A` contains `E8 09 00 00 00`, targeting
`00C11C18` with physical return address `00C11C0F`. This packet establishes
only the target's first instruction: `6A 0A`, two bytes encoding PUSH 10.
The packet label and saved function name do not establish cleanup or unlock behavior.

Two initial typed captures and one closing exact-entry capture agree at
modification 98 in `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
All four listing aliases identify the same two-byte InstructionDB unit at
`00C11C18`. Default and effective flow are FALL_THROUGH to `00C11C1A`, with
no override or direct target. Four count checks returned 64,730 functions.
All raw HTTP JSON, base64 payloads, digests and command streams are retained.

Root admitted paired exact two-byte reads only after independently checking
the entry metadata. Both reads return `6A 0A`. Independent physical decoding
uses opcode `6A ib`: sign-extend immediate byte `0A` to the saved default
32-bit operand width. There is no ModRM or relative target. Actual detailed
Capstone 5.0.7 agrees on the instruction, operand, width and encoding offsets.
The context-zero saved instruction is `PUSH 0xa`.

On normal completion under that decode and a 32-bit stack model, if current
ESP is S, the instruction sets ESP to S-4 modulo 2^32 and writes `0000000A`
through current SS at that address. The PUSH leaves flags unchanged. It does
not read a caller argument. The literal has no established lock-number or
argument meaning. No return, cleanup, net stack balance or external returned
register, ESP, EBP, flag, segment or memory state follows from this entry alone.

Saved metadata names `LIBCRT_LockInitializer_UnlockLock10_00c11c18`, marks it
non-thunk with no-return false, and gives a complete nine-address-unit range
`00C11C18..00C11C20`. Those are saved labels and ownership, not behavior or ABI.
The remaining seven bytes `00C11C1A..00C11C20` were not read. No interior,
adjacent instruction, callee, target, data or provider was queried.

Only four prototype prefix lines were interpreted. Although requested with
`--lines 4`, the CLI delivered a 374-byte stream with a 189-byte suffix;
that complete suffix and automatic target objects remain opaque. The saved
`undefined ... (void)` signature does not establish arguments or actual ABI.

The finite Git proof contains six actual frozen commit/tree objects and eight
positive input files: the immediate body and primary doc/report pairs plus
target configuration and the three CLI producers. Physical helper preimages
and revision producers are preserved. Both original and physically distinct
copy core ZIPs pass a restricted replay: one archive read, one receipt write,
one pinned pure-helper execution and zero unexpected audit denials. No earlier
archive was traversed. All eleven Native command captures succeeded.

The final delivery checks the frozen/final actual Git objects, eight unchanged
inputs and exactly the two owned doc/report outputs, including working, index
and committed bytes. Source subtree IDs identify the worker revisions only;
there is no current Root equality assertion or Source implementation credit.
No build, new test, probe, original program, OS/audio, startup or gameplay
validation is claimed. Ghidra was read only.

Private evidence: `local/cc12_locale_onexit_lock_initializer_cleanup_entry/`.
Core SHA-256: `0accc63b0233556802e9bf33cd4a9c4947176e74a90f3f5337643627d7ee1ba4`.
The report pins captures, producers, preimages, the Root gate and both replay receipts.
The delivery archive and its receipts are produced after this commit and handed
off separately; no self-referential digest is claimed.
