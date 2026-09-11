# Actual-storage VFS open consumer

Addresses: `00bdf310`, `00bda690`, `00bdca80`, `00bd9040`, `00bdd0a0`.

The consumer now opens through the current callable provider table, retains the
original flags, and returns the provider's stream unchanged. It uses the existing
actual pooled-string implementation and initialized mount/alias storage. Provider,
stream and manager construction are still external. No file is opened by a new
host filesystem adapter and no replacement stream owner is created.

| Routine | Original ABI | Coverage |
|---|---|---|
| `00bdf310` | ECX captured manager; name/flags stacked; EAX stream; RET8 | Complete consumer control flow with required original-ABI services |
| `00bda690` | ECX visitor; mount payload/name stacked; RET8; return unspecified | Complete provider-call consumer |
| `00bdca80` | ECX manager; mutable name stacked; RET4 | Complete actual alias storage consumer |
| `00bd9040` | ECX visitor; RET; EAX0/1 | Complete nine-byte raw leaf; Ghidra definition pending integration |
| `00bdd0a0` | ECX captured manager; name/visitor stacked; RET8 | Complete traversal control flow qualified for current D6838C visitor identity and BDA690/BD9040 slots |

Names describe reconstructed behavior; they are not recovered symbols. The C++
interfaces add context arguments and do not preserve the original binary ABI,
FH3/SEH implementation or every incidental caller register. Existing date and
lookup traversal implementations and the semantic mounted-stream API are retained.

## Storage and ordering

The native wrapper writes visitor identity D6838C at `00bdf38b`, stream null at
`00bdf393`, and full flags at `00bdf397`. Its ownership byte at visitor+0C is left
uninitialized until a successful provider call. The table bytes at D6838C are
`00bd9e40 / 00bda690 / 00bd9040`. The getter is `XOR EAX,EAX; CMP [ECX+4],EAX;
SETNZ AL; RET`: a pointer whose low byte is zero still stops traversal.

Manager construction `00be1dc0` initializes diagnostic byte+20, counters+24/+28/+2C,
mount head+40, and zeroes failure field+90 and alias header+94/+98/+9C.
`00bdec70` grows that array in 16-byte steps, initializing four zero words per
record; `00bdb850` releases its two eight-byte string headers in reverse order.
The first half is compared and the second half is copied by `00bdca80`. This
packet consumes these existing bytes and does not publish a competing manager or
alias-record structure. Mount storage and its producer remain the established
actual traversal contract in `NATIVE_VFS_LOOKUP_ROUTES.md`.

Open copies and normalizes its input, then applies exactly the first equal-length,
case-insensitive alias. It reloads the current alias base before copying the
replacement, stops after that copy, and does not normalize the replacement again.
The caller's name remains unchanged.

The callback first reads current published manager+20, payload byte+0C and visitor
flags bit0 for its diagnostic gate. The target `004254b0` is a verified single
`RET`; the source preserves the conditional name-pointer read and introduces no
host logging. It then reads payload+8, current flags and current provider table+8,
calls with the full flags word, publishes the returned stream at visitor+4, and
copies the current payload byte+0C only for a nonnull stream. A failed visit leaves
the previous ownership byte intact. Traversal rereads visitor identity and +8
after the provider call. Unsupported identities or changed slots produce an
explicit source `invalid_argument` boundary; they are not native failure behavior.

The outer wrapper preserves these distinct cases:

- Null result with flags bit0 set: release the copied name and return null.
- Null result with bit0 clear: diagnostic uses the original caller name; reload
  publication and call the **field** at manager+90, then return the captured null.
- Nonnull result with bit0 set: increment captured manager+24 only.
- Nonnull result with bit0 clear: call BDE9C0 using the newly published manager,
  copied name, stream and mount byte; increment captured manager+28; reread the
  returned stream's current table+2C and call it with stacked zero; add returned
  EAX to current captured manager+2C with DWORD wrap.

The stream is neither retained nor released by this consumer. BDE9C0's inspected
body is filename-filtered logging, not stream registration. Its stream argument
is unused. The native call pushes all four bytes of visitor+0C, although only its
low byte is established and consumed; the source passes that low byte zero-extended
and makes no assertion about the three native unspecified bytes.

## Required external contracts

| Call site | Actual dependency | Boundary |
|---|---|---|
| `00bda6db` | Current payload+8 provider, current table+08, name/full flags, ECX owner, RET8, EAX stream | Caller supplies callable original-ABI tables; no arbitrary provider resolver or default provider |
| `00bdf432` | Current publication's **field**+90, no stacked args, ECX current manager | Actual application callback required when reached; null retains native invalid-call behavior |
| `00bdf44b` | `00bde9c0`, ECX current manager, name/stream/mount byte, RET0C | Explicit required `NativeVfsOpenedResourceLog`; absent binding throws at the reached source boundary |
| `00bdf468` | Returned stream's current table+2C, stacked DWORD0, RET4, EAX consumed | Callable original-ABI stream required; owner and operation implementation remain external |

The fastcall bridge reserves EDX so stack arguments keep their original positions.
For indirect calls it supplies the selected target in EDX, matching the visible
call-site setup. This is not a guarantee for unspecified scratch registers.
Provider and stream tables must contain executable addresses, unlike the numeric
identity table used for the reconstructed visitor. There is no default service.

Next independent packets are concrete:

1. Physical provider `00bf4ba0`: actual pool/acquire `00bf42a0`, stream construction
   `00bf3770`, open thunk `00bf5590 -> 00bf52a0`, virtual+18 validity and intrusive
   failure release. Existing actual path builder `00bf3970` can be reused.
2. FileStore provider `00be5fa0`: actual find `00be5e90`, then actual stored-stream
   conversion `00bef750` and returned owner. The existing semantic FileStore stream
   is not an actual owner substitute.
3. MPKG provider `00bb8e70 -> 00bb8d60 -> 00bb8be0`: actual archive state and stream
   allocation/lifetime for all entry branches. Existing archive algorithms do not
   by themselves establish the native storage/lifetime chain.
4. MSAR D643C4 table+08 target `00bba7c0`: body/stream contract still unread by this
   packet; preserve the address label until its callee is inspected.
5. Open logging `00bde9c0`: actual logger/builder `00426500`, `00bd1a60`, `00bd1a20`,
   `00425f80` and current global0109CEE8. The filename gate is already inspected,
   but no logger owner is supplied here.

The first three provider bodies were read before documenting their contracts.
No lease was taken for these consumed dependencies. Numeric table slots were
rechecked for physical D69168, FileStore D689E8, MPKG D64390 and MSAR D643C4.

## Verification

`./scripts/build.ps1` passes the MSVC Win32 Release build and the checkout's one
configured CTest (`reconstructed_math`). `tools/verify_report_calls.py` checks the
report's direct call addresses against live stored function bodies; unresolved
virtual/field calls are explicitly marked indirect.

Eight current Ghidra spans, 1,563 bytes, match the installed PE byte-for-byte.
An ignored local differential fixture executes those original instructions with
bridges for existing actual string/pool/normalization helpers. It compares the
linked library implementation across eight open compositions, including empty
tree, missing/opened providers, flags2/3/32h/F3h, first alias without recursion or
renormalization, failed first mount followed by success, nonmatching prefix,
publication changes, logging order, late size-slot replacement and counter wrap.
The return, complete 2,048-byte arena, actual pool prefix and service events agree.
Separate original/source getter checks cover null and pointer00000100h.

Provider/log/size/failure fixture functions are instrumentation, not evidence of
implemented provider owners. The test does not execute native provider I/O, the
original logger, native FH3 unwind, or the original game's runtime. The new route
is not yet connected to `bsp_game`, so no frame or gameplay claim is made.
Fixture source, executable, build command, raw byte captures and hashes are under
the worktree's ignored `local/`; the durable report records their evidence.
