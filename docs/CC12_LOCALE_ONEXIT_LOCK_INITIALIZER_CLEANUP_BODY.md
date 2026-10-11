# Locale initializer numeric target body at C11C18

The complete saved nine-byte body is `6A 0A E8 12 FF FF FF 59 C3`.
It encodes PUSH 10, CALL `C11B31`, POP ECX and RET. The external call's
behavior remains unknown; the saved name establishes no unlock effect.

| Address | Bytes | Physical instruction |
|---|---|---|
| 00C11C18 | 6A 0A | PUSH 10 |
| 00C11C1A | E8 12 FF FF FF | CALL 00C11B31 |
| 00C11C1F | 59 | POP ECX |
| 00C11C20 | C3 | RET |

Root checked both initial per-byte metadata sweeps before admitting the body
bytes. Four whole instructions partition all nine units as 2+5+1+1; exact and
containing instruction/code-unit aliases agree at every address, including
interiors. The closing sweep agrees with both initial sweeps. All 27 complete
raw responses retain HTTP JSON, base64 and digests. Project/program identity is
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, modification 98, with
64,730 functions at each of four count checks. No collection attempt failed.

Both exact byte reads agree. Independent opcode decoding covers every byte:
`6A ib` sign-extends 0A, `E8 rel32` adds signed -238 to physical next address
`C11C1F`, `59` selects ECX in the POP-register class, and `C3` is near RET.
There is no ModRM or IAT dereference. Actual detailed Capstone 5.0.7 independently
agrees on all four instructions, operands and encoding fields. The context-zero
saved text agrees at all four starts. Saved default/effective flow has no override.

These effects are conditional on normal completion under saved x86 default-32
decoding and a 32-bit stack model. Let S be entry ESP. PUSH writes `0000000A`
through current SS:[S-4]. CALL then pushes `00C11C1F` through current SS:[S-8]
and transfers control to numeric address `C11B31`. The literal 10 has no
established argument or lock-number meaning. No target body or data was read.

If control returns at `C11C1F`, every returned general register, ESP, EBP,
flag, segment and memory value is unknown. Let Q be that returned ESP.
POP ECX reads through the SS and memory current at that instruction at Q,
then advances ESP to Q+4 modulo 2^32. RET reads its destination through the
SS and memory current at that instruction at Q+4, then advances ESP to Q+8.
The destination and relation between Q and S remain unknown. Later reads may
alias earlier writes and observe their then-current contents. ECX is not proved
to receive 10, POP is not proved to clean an argument, and RET is not proved to
reach a particular caller. Local instructions leave arithmetic flags unchanged;
flags after the call remain the unknown returned flags.

The saved owner is `LIBCRT_LockInitializer_UnlockLock10_00c11c18`, non-thunk,
no-return false, with complete range `C11C18..C11C20`. Those fields establish
saved ownership only. Cleanup, unlock, CRT/provider identity, ABI, return,
net stack balance and status are not authenticated. Automatic target objects
remain opaque. No prototype, adjacent code, callee, provider or data query occurred.

The finite proof retains six actual frozen Git commit/tree objects and eight
positive inputs: the immediate entry and primary doc/report pairs, target
configuration and the three CLI producers. Physical helper preimages and revision
producers are retained. Both original and physically distinct copy core ZIPs
pass restricted replay with one archive read, one pinned pure-helper execution,
one receipt write and zero unexpected audit denials. All ten Native command
captures and streams are retained; no earlier archive was traversed.

Final delivery checks eight unchanged inputs and exactly the two owned outputs,
including actual Git objects and working/index/committed bytes. Frozen Source
subtree IDs identify the worker revisions only. No current Root Source equality,
Source implementation, build, new test, probe, original program, OS/audio,
startup or gameplay result is claimed. Ghidra was read only.

Private evidence: `local/cc12_locale_onexit_lock_initializer_cleanup_body/`.
Core SHA-256: `1e958734366ccc067ea1256192a2d4806790679eecc0763fe91e400ec63b8d16`.
The report pins raw captures, whole-unit gate, independent decoder, actual
Capstone capture, producer preimages and both restricted replay receipts.
Delivery archive and receipts are produced after this commit and handed off
separately; no self-referential digest is claimed.
