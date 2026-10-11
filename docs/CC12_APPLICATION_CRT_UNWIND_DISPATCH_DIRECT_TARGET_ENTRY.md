# CRT unwind-dispatch direct-target entry

Both admitted one-byte reads at `00C07623` return `55`.
SHA-256: `a25513c7e0f6eaa80a3337ee18081b9e2ed09e00af8531c8f7bb2542764027e7`.
Manual decoding (`50+rd`, register index5) and actual detailed Capstone5.0.7
agree with the context-zero saved instruction text:

```text
00C07623  55  PUSH EBP
```

For entry ESP=T and current entry EBP=B, successful completion gives
`ESP=T-4` modulo2^32 and writes B to `DWORD SS:[T-4]`. EBP and flags remain
unchanged. `00C07624` is only the arithmetic next address; no byte or metadata
there was queried. The meaning and eventual behavior of the remaining body
are outside this packet.

## Positive numeric call provenance

The immediate accepted Git report
`reports/cc12_application_crt_dispatch_body_listing_metadata.json`, selector
`instruction_operands.code_hex`, contains the228-byte `00C07991` capture
with SHA-256 `3b08ba0b3c685e6065f6ce8071ccaadcdf9cbbbe82ff0af45c97f073c4acae60`. Its accepted PRIMARY report is
also a finite positive input. Only the five retained bytes at body offset
`D4` were independently redecoded offline:

```text
00C07A65  E8 B9 FB FF FF  CALL 00C07623
```

The signed displacement is -1095; adding it to next EIP `00C07A6A`
computes `00C07623`. Manual arithmetic and actual detailed Capstone agree
with the report's exact instruction row. This establishes numeric direct-call
provenance without a fresh caller or destination-body query.

The accepted caller schedule preserves unknown returned state after its
first opaque external call: current ESP=Q0, EBP=F0, EAX=P and memory=M0.
It assumes neither original frame preservation nor stack balance. On the
default direct-call path, the eight argument pushes load memory operands at
their individual push times; the target entry has `T=Q0-24h`, `B=F0` and the
E8 return address `00C07A6A` at `SS:[Q0-24h]`. If this entry PUSH completes,
it yields `ESP=Q0-28h` and stores current F0 at `SS:[Q0-28h]`.
That is a conditional one-instruction composition. All eventual returned
registers, flags, memory and stack cleanup remain unknown; this packet adds
no caller-frame or target-ABI guarantee.

## Saved metadata and admission

Paired exact-entry typed captures and the closing capture agree on all four
aliases: one `InstructionDB` at `00C07623`, length1. The saved owner is
`FindHandler`, entry `00C07623`, non-thunk/no-return=false, with one complete
854-byte saved range `00C07623..00C07978`. Default and effective flow are
`FALL_THROUGH`, override `NONE`, with no fall-through override.
Complete owner-range metadata does not constitute physical body coverage:
only the entry byte was read and the other853 bytes remain unread.

Only the first four prototype metadata lines were used:

```text
Function: FindHandler at 00c07623
Signature: void __cdecl FindHandler(EHExceptionRecord * param_1, EHRegistrationNode * param_2, _CONTEXT * param_3, void * param_4, _s_FuncInfo * param_5, uchar param_6, int param_7, EHRegistrationNode * param_8)
Entry: 00c07623
Body: 00c07623 - 00c07978
```

The saved name, parameter types and calling-convention text are unauthenticated
metadata here. Automatic prototype text beyond this prefix and typed
target/companion objects remain opaque. Root admitted the exact1B read pair,
context0 entry text, this four-line metadata prefix and closing exact-entry
typed/count checks only.

## Evidence closure and limits

All three complete direct typed responses identify saved
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86/32-bit and
unchanged epoch98. All four count checkpoints are64730.
There were no worker Native or helper failures. Root separately disclosed
two offline gate-preparation/launch failures caused by a missing owned
private directory; the correction preceded gate execution and had no Native
effect. That disclosure and the physical Root gate producer are retained.

The proof retains eight immediate positive Git blobs, actual
frozen/staged/final commit/tree objects, exactly this document and its report,
all worker command/raw/stream records, physical helpers and the executed
sealer preimage. Original and distinct-copy pure ZIP replays validate the
actual Git and byte/metadata closure. Each guarded replay uses one archive
read, one receipt write and one retained code execution after bootstrap.
Source subtree IDs are revision identity only; Source contents and prior
archives were not read.

No Original/FH3/ABI, saved-name semantics, handler selection, original frame
binding, complete function, ownership, activation or runtime credit is
established. No Source, build, test, probe, game, OS audio or Ghidra mutation
occurred. Remaining-body inspection requires a separate Root admission.
