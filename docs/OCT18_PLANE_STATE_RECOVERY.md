# Whole plane state receiver 007D1360

Owner agent/oct18-rec, coordinator follow-up. Complete source driver in
`src/oct18_plane_state.cpp`, contract `include/bsp/oct18_plane_state.hpp`.
No host wiring. The previous six-word `pilot_command_path.cpp` projection is
retained separately. Names are hypotheses, not recovered symbols.

## Scope, source audit and original ABI

Address greps preceded writing. No host executes the whole 007D1360 receiver:
the address appears in the partial message/control projection, declarations and
documentation, not a host call. A raw driver is therefore required, rather than
a ledger-only host receipt. Lease checks showed no root-address conflict.

The helpers **007D8180** and **007DB1F0** lie in the cc9 physics interval
007D8000-007E3000. They are required dependency callbacks, with receiver unit+AB0h;
their existing source, host bindings and ownership are untouched. All other
dependencies are also explicit, or reuse the existing exact 004134F0 matrix copy.
No library/STL helper body is reconstructed merely to link.

Project/program verified: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
Original [007D1360,007D1B4C): 2,028 bytes, 466 instructions, SHA-256
`86d9ced1d61ee2e78bc3bd050dc2cf1ab76172e9cb232bbe3c16067e8c943362`.
Original ECX plane, one mutable message stack pointer, RET4; no semantic result.
The new C++ interface borrows actual publications, constants and dependencies;
it does not reconstruct a new game/global/physics owner.

## Register and frame recovery

BL captures whether [00E188A8] exists and game+1FE4h == 1. This is not a promised
input EBX value. The decompiler's later `unaff_EBX` tests are ordinary frame-byte
tests: 007D173B reads [ESP+12h], and 007D17E6 reads [ESP+13h], after PUSH EBP.
Source retains one uninitialized 8Ch-byte frame span, with these exact aliases:

| Source frame offset | Original ESP after PUSH EBP | Meaning / reuse |
|---|---|---|
| +02h | +12h | local ownership/restore gate |
| +03h | +13h | captured initial flight state in {7,6,4,5} |
| +04h | +14h | queued received-ID DWORD, later old factor, later clamp lower zero |
| +08h/+0Ch/+10h | +18h/+1Ch/+20h | saved linear vector, position delta, later new Euler angles |
| +14h | +24h | correction candidate and clamp result |
| +18h | +28h | modulo-32 message tick delta, later clamp upper bound |
| +1Ch/+20h/+24h | +2Ch/+30h/+34h | captured pre-replay world position |
| +28h/+2Ch/+30h | +38h/+3Ch/+40h | saved angular vector |
| +34h/+38h/+3Ch | +44h/+48h/+4Ch | captured original Euler angles |
| +40h | +50h | unit virtual +34h output scratch (return ignored) |
| +4Ch..+8Bh | +5Ch..+9Bh | identity matrix, then controller state record |

The queue argument at 007D140B is MOVZX(unit+174h) followed by a DWORD MOV;
it is **integer bits, not a float conversion**. The tick subtraction is also
DWORD arithmetic. Negative replay ticks use modulo-32 negation and bit-preserving
conversion, including INT_MIN, instead of undefined signed C++ negation.

The two control copies reload message+38h..4Ch independently, first into
unit+9E4h..9F8h, then +9FCh..A10h. There is no memcpy snapshot and no write to
unit+A14h. The early nonflying arm builds/copies the matrix BEFORE those copies;
the ordinary active arm copies controls BEFORE building the matrix. The scratch
matrix is reused by 007BAAF0 and passed to unit+AB0h, preserving byte +27h which
that existing control-state dependency does not copy.

## Full control flow and dependency contracts

Host packets reject nonflying planes and state-byte mismatches before any receive
side effects. Nonhost paths preserve local-role, +6E8h and +9E1h gates, including
the already-owned branch which continues through a writeback message. The receiver
then queues the integer ID (nonhost only), finds state through 0077C6B0, and calls
007C2AF0 with ECX message, stack(unit, found_state). Optional gravity goes through
007D8180 without replacing that helper. Child invalidation reloads child+44h
after every callback, then world pose/velocity/Euler acquisition occurs in order.

After saving/resetting vectors, 007C78A0 may transition a nonhost plane out of the
flying states; its early arm resets +9E1h and returns after the matrix/control
copies. Otherwise +C49h is set. The restore arm compares the old factor with the
current 00D7A218 float using COMISS, restores the saved vectors/factor only when
strictly above, calls unit virtual +1DCh with zero, and clears only +C49h.

The ordinary arm sets +9E0h, builds the matrix/controller state, applies it through
007DB1F0, invalidates and updates pose, then either replays negative tick delta
through 008746A0 or calls the EMBEDDED unit+310h component's virtual +04h with zero.
It tests the EMBEDDED unit+72Ch component's virtual +38h before correction. These
receivers are component addresses, not pointers loaded from those offsets.
After correction the second pose update occurs, optional +6E8h reset propagates
all zero-vector fields in the original order, and +9E0h/+C49h are cleared.

All callback signatures in the header identify their original address/receiver.
Virtual +34h is thiscall(unit; scratch) RET4, +04h and +1DCh are thiscall(receiver;
zero float) RET4, and +38h is thiscall(component) RET with AL result. 0042D2E0 is
ECX matrix, EDX X pointer, stack Y/Z pointers, RET8. 007BDF50 is ECX message,
stack float[16] output, RET4 with EAX output; returned matrix pointers are used
for the subsequent exact sequential x87 copies rather than assumed/snapshotted.

## x87 and alias contracts

Raw PE decoding verifies **DC 25** at 007D1873: FSUB qword [00D7A308], not float.
The current double is 2.0 in the checked image, while +908h is float. The candidate
is spilled to float, clamped by pointer, spilled again and FST-stored to +4DCh
before the original unordered-aware positive comparison. Required raw math entries
retain ECX/EDX/stack pointer arguments for 00415620, ECX vector for 00427E30, and
two stack floats / RET8 for 00438B10. Their ST0 results are consumed directly in
assembly; no C++ return temporary is interposed. These are required bindings to
existing operations, not fallback math implementations or altered stock bodies.

Position differences are individually float-spilled before sequential x87 stores
to +4A8h. Length-squared comparison uses JA after the original x87 compare, with
the signed message+90h > 5 fallback. Current zero-vector DWORDs are reloaded at
their original write sites. Each wrapped-angle result is FSTP-committed before
loading later arguments, preserving receiver/source/frame alias order. No artificial
flag clearing, replay bound, finite-input guard or message quantizer is added.

## Validation and limits

Seed verification passes before native differential work. The Win32 build and all
three enabled CTest groups pass. A focused manifested native fixture relocates the
original 2,028 bytes' call/global operands to the SAME deterministic dependencies
used by the source driver. All control branches and instructions remain original.
The three raw math bindings execute their original leaf bytes, with wrapped-angle
constants relocated to byte-identical captured values. No original installed game
or live host is launched or modified.

Fourteen ordinary-path cases compare the complete C60h unit image, mutable A0h
message and ordered callback events: host/client receive, early rejects, nonflying
transition, positive/NaN restore factor, zero and INT_MIN replay deltas, message
aliasing the unit/control storage, correction count/distance rejection, zero clamp,
and late +6E8h reset. Every case matches. Additional callback assertions validate
the queue's integer bits and relative frame addresses of velocity, Euler, identity
matrix and controller-state buffers; the unchanged +27h and +A14h bytes are checked.

Evidence: `local/rec_plane_raw.asm`, `local/rec_plane_reference.inc`,
`local/rec_plane_probe.cpp`, `local/rec_plane_probe.log`, and build/seed logs.
This is whole ordinary-source and focused fixture proof. It is not a new host
binding, original ABI replacement, exhaustive arbitrary alias/exception proof,
or gameplay validation; callback implementations must satisfy the stated contracts.
