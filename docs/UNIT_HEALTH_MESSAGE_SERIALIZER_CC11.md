# Complete ordinary serializer on the health message path

`NativeUnitHealthMessageSerializedCalls` completes `00783C80` after the accepted
setter, D2 construction, flags4 routing, peer selection and actual locked wrapper.
It remains **abstract**: complete transport virtual+20 flushing is required.
The serializer calls the current executable message writer, so it also handles
other admitted translated message profiles. There is no D2-only substitute.

The packet also completes `00428EC0` and `00429540`, the serializer's two bit
helpers. Their normal source declarations are used by production and by the
focused original-PE differential probe. All prior implementation bytes are
preserved, apart from adding the needed `cstring` include.

## Native bodies and bindings

| Body | Complete native extent | Native ABI |
| --- | --- | --- |
| Serializer, 113 instructions | `00783C80..00783DC0` exclusive | ECX=transport; stack target/message; RET8 |
| Extract, 53 instructions | `00428EC0..00428F45` exclusive | ECX=cursor; stack destination/start/count; RET0C |
| Append, 52 instructions | `00429540..004295BF` exclusive | ECX=cursor; stack source/count; RET8 |

The listings are intact and integer-only. Assembly resolves the serializer
decompiler's `unaff_EBX` and `unaff_retaddr` artifacts: its saved ECX is the
transport and the original first stack argument is the target. No register
input is invented. Descriptive source names are hypotheses, not recovered
symbols. These are new C++ interfaces, not binary-compatible replacement ABIs.

The adapter borrows the actual low tick WORD at `00F876B0`. Required pure
accessors expose the actual `target+D40+4*delivery` cursor-pointer cell and the
current transport table. They perform no early value snapshots, callbacks,
allocation, ownership changes or FP changes. The selected cursor is captured
once from the first full message delivery DWORD and stays live throughout the
operation, even if a flush callback changes its target cell.

## Complete sequence

1. Read the captured cursor's current pointer, base and bit offset; calculate
   bit position with native 32-bit arithmetic. If empty, write the actual tick
   WORD as16 bits, then a fresh message delivery WORD as3 bits. Recompute the
   prefix end. A nonempty cursor does not read or write this prefix.
2. Load the actual current message profile+4 and invoke its executable writer
   on that same raw cursor. Recompute the resulting bit position.
3. Interpret wrapping `after_bits+20h` as signed32 and compare against2320h.
   On overflow, copy `after-before` bits from the absolute pre-write position,
   rewind with the existing `00428B80`, and flush the prefix through the fresh
   transport table+20 with a fresh full delivery DWORD.
4. After that flush, read the captured cursor's fresh base once; set current
   to that base, set bit offset0, and clear exactly its first byte. Read a fresh
   tick WORD and delivery WORD, write the new19-bit prefix, then restore the
   copied bits with the exact append helper.
5. Reload the full delivery DWORD. Value2 reaches the final flush without
   reading rounded byte count. Otherwise calculate the unsigned native count
   `current-base+((bit&7)!=0)` and flush at380h bytes or more. Use the fresh
   transport table+20 and the captured target/cursor, then perform the same
   reset from the cursor's fresh post-flush base.

The actual scratch is **200h bytes (512)**. Native `SUB ESP,204h` includes an
additional saved transport DWORD; that DWORD is not payload space. The source
scratch is uninitialized and has the same512-byte extent. Its compiler adds
its own spill/security-cookie frame, which is not a native frame-layout claim.

## Exact bit conventions

`00428EC0` starts at `base[start/8]`, visits source bits `7-(start&7)` down to0,
and packs destination bits7 down to0. It clears each new output byte before
reloading the cursor base and reading the source. A partial final byte has
unused low bits cleared; a zero count writes nothing. Cursor fields are not
updated.

`00429540` consumes source bits7 down to0, but ORs them at the destination
cursor's **LSB-indexed** bit offset. It reloads the actual low bit-offset byte
after the output OR, increments the actual DWORD, and preserves the native
carry-byte write when crossing a byte boundary, including a zero carry.

This unusual pair is preserved exactly. It is not replaced with the generic
cursor read/write functions or a presumed wire-format correction. The existing
signed-word writer and rewind remain unchanged and are reused directly.

Admitted backing includes every reached byte and the writable native carry
byte; delivery indices resolve valid actual cells. An overflow delta is at
most1000h bits, fitting the real512-byte scratch. These are valid-backing
preconditions, not added clamps, capacity checks or invented failure returns.

## Focused validation

One ignored actual-TU MSVC Win32 probe passed **53 checks, zero failures** with
`/O2 /Gy /W4 /WX /fp:strict` and an embedded `asInvoker` manifest. It covers:

- Four differential rows using exact original PE bytes for both pure helpers:
  zero bits,8 bits with a carry boundary, a29-bit partial tail, and4096 bits.
  Source output bytes, untouched tails/carry bytes and cursor state match.
  The original bodies are copied read-only from the installed PE into an
  isolated executable page; no game process or other native body is invoked.
- The complete connected health path into an empty buffer: actual tick16,
  delivery3 and executable D2 writer29 produce the expected48 bits and carry
  byte. The local target is excluded, the actual lock balances, and no flush
  or frame release occurs.
- A nonempty D2 append landing at exactly380h bytes: one flush, followed by
  reset from the provider's replacement base. Length and following bytes are
  retained, and the lock balances.
- Overflow with the existing real type15 (**decimal15, 0Fh**) constructor and
  executable writer. Count232 emits1880 bits into a cursor starting at7159
  bits. The first flush sees the cursor rewound to7159. Its controlled source
  provider changes the tick, message delivery to2, transport table, target's
  cursor cell and the captured cursor's backing. The restored bytes match an
  oracle using the original helpers plus an independent prefix writer. A
  second flush uses the new table and delivery2; it still receives the same
  captured cursor. Final reset uses another fresh base. The replacement cursor
  remains untouched.

The controlled provider mutations establish source behavior only; they do not
claim observed native serializer or network reentry. The type15 object is a
separately owned direct-case message. The fixture never retains the setter's
borrowed frame, and its flush bodies consume/copy bytes synchronously.

Generated `/FAs` listings for both helpers and the full serializer were reviewed,
including inlined helper loops, signed overflow comparison, WORD reads, fresh
tables, captured cursor, delivery2 bypass, unsigned byte threshold and resets.
MSVC reuses an inlined local name in that textual listing; raw object disassembly
confirms the immutable delta at stack+24h is distinct from the loop count at+30h
and is correctly reloaded for rewind. All source artifact hashes are recorded.

Reproduce from the worker checkout:

```text
cmd /c local\cc11_health_message_serializer_check.cmd
python tools/verify_report_calls.py reports/unit_health_message_serializer_cc11.json
```

Complete virtual+20 transport effects and ownership remain required. No default
flush success, transport, queue, clone, allocator or retained borrowed cursor
is supplied. Actual network delivery, original full ABI, private EH, faults,
invalid backing/concurrent mutation and gameplay remain unproved. Primary
integration owns the full build, metadata and Ghidra annotations.
