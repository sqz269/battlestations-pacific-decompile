# CC13 left8c: two hard segment 8 x87 functions

Worker cc13-left8c, 2026-10-14. Code: `src/cc13_left8c_seg8.cpp`, `include/bsp/cc13_left8c_seg8.hpp`
(`Left8cHost` extends `Left8bHost`). Both functions are reconstructed and build-tested only;
nothing is bound to a host, fixture-tested or game-validated. Ghidra was read only.

## Functions

| address | C++ | ABI | summary |
| --- | --- | --- | --- |
| 00496BD0 | `scene_traffic_update_members_00496bd0` | fastcall(ECX runtime), RET; EBP frame aligned to 8 | The scene-traffic member update that the path-runtime slot 1 0049A360 ends in. Reconciles the member count (target = `_ftol2(+84h / (+38h / +44h)) * [+34h]`, 0 on a cyclic path): dead indices below `[+88h]` leave list +78h, then each step either erases the matching dead index (004887A0) or detaches and kills the front member (vtable +114h(0), 00926D90(member, 2), unlink and free). Rows run from a start to an end on the path (non-cyclic: the knot distances 007B05E0 of `+28h`/`+24h` and the CRT fmod phase; cyclic: `+148h` and `+84h * dir * +44h`), stepping by `+38h * dir`; each row hashes `{index, 42}` (00441F30, 00484660) into a jittered path position (fmod-wrapped or clamped), samples it (007B04C0), tests the view box (004B48E0), and when visible builds the row frame (007B03C0, direction negated when reversed, global up, 0085DAD0). Per column: dead indices are skipped; visible members get the row frame shifted by the hashed column offset and the four-sine sway (amplitude `+48h * +44h * +130h`), with landscape height and (vehicles) normal (0087FA20, 0087FB90, 0085DAD0); missing members are created as in 00497BB0 except that the animation pick hashes `{index, 54h}`; the pose is written and the member refreshed (vtable +11Ch/+D8h/+118h, 004134F0, 0042ED50). |
| 0049B1F0 | `plane_debris_tick_0049b1f0` | thiscall(ECX spawner, float dt), RET 4 | Slot 1 of vtable 00CE6830 (slot 0 0049D610, slot 2 00480B70; installed by 00498E60 and 0049A580; no direct caller). The plane-debris spawner tick: latches `+49h` and counts `[00E186E8]` when the plane is gone; counts the spawn timer `+4Ch` down and, while the plane lives, adds one record per expiry with 18 RNG draws plus a random direction (the 0049A760 record setup with a clamp-only class pick from weights 00E1877C, the spin and decay draws stored in reverse order, and the draw order `+34h, +30h, +2Ch` for the signed turn rates). Each alive record: life countdown (dead records detach and kill their entity), gravity `class+58h * -9.81 * dt`, drag, integration, water (0078CF20) and ground (00903860) contact with an optional splash effect (008685E0, gated by `class+5Ch`, water >= ground and 0043B9C0), turn-rate decay toward zero, angle integration, entity creation on demand (class vtable +8, +98h, +114h, +A0h, trail effect 008685E0 + 004845D0 into `+188h`), the pose `00413920(0047C2A0(angles), translate(position))` copied into `entity+74h`, the fade `+18Ch = 00419010(0, 0, 2, 1, life)` and vtable +D8h. Byte `+48h` is cleared; when it was clear on entry, `+49h` is set and no record was alive, the spawner deletes itself (vtable slot 0(1)). |

## Evidence

- 00496BD0: the full listing (915 lines) read with an ESP-relative slot map. x87 operand order
  checked against the bytes with `tools/x87_dc_form_scan.py --range 00496BD0-00497B80
  --forms d8,dc,de --include-st0`: DE E9 at 00496E34 (end = start - dir * (P - 1e-4)), DE EB at
  00497129 (box min = p - extent), DC C9 at 004971BE and D8 C9 at 004971D0 (all three direction
  components negated), DC E9 at 0049742A and DE E1 at 00497440 (column offset
  `j - ([+34h] - 1) * 0.5`), D8 C9 at 0049745D, DE E9 at 00497B46 (cursor - spacing * dir).
  The x87 values carried across branches: the row-loop test loads end and cursor and swaps
  them with FXCH only when `+144h` is set, so FCOMIP compares cursor < end (forward) or
  end < cursor (reversed); unordered exits.
- Constants read from the PE: 00D7A268 = 1e-4f widened, 00D7A250 = -1.0, 00D7A218 = 0.0f,
  00CE67E8/E0/D8/D0 = 0.12f/0.11f/0.13f/0.14f widened, 00CE6428 = 369097.0.
- 0049B1F0: vtable slot found by scanning the image for the address (one hit, 00CE6834 in
  .rdata; its vtable base 00CE6830 is written by 00498E60 and 0049A580). The full listing
  (1000 lines) read; dt is carried in ST0 across the record loop (popped before every call
  and reloaded from the argument). x87 bytes: DE EA at 0049B253 (timer - dt), D8 E1 at
  0049B9AA (life - dt), DC C1 at 0049BA1E (velocity.x + 0.0), D8 EA / DE F2 at
  0049BA6A/0049BA6C (`(len - drag*dt) / len`), D8 CA + DE E9 at 0049BC69/0049BC92 (rate -
  decay*dt). Constants: 00CE65E0 = -9.81f widened, 00CE65D8 = -100.0f, 00D7A3A0 = 0.1f
  widened, 00CE3958 = 2.0f, 00CE684C/00D7A264 = -/+pi, 00CE6848/00CE38B8 = -/+10.
- The record field names (position, velocity, turn rates, decay, life, drag) follow from
  how this tick integrates them; they are hypotheses. The decay fields `+38h..+40h` are
  what docs/CC13_LEFT8B.md calls scales for 0049A760.

## RNG

- 00496BD0 makes no draws: every jitter and pick hashes the member index (00484660, 00480A90).
- 0049B1F0 draws from the shared generator 00BD2F10 on stream 0: once for the timer and 18
  times per added record, plus 00439950 (a random unit direction, itself RNG-backed). The
  per-record tick makes no draws. A host that binds it must apply the cc9 RNG rule.

## Simplifications and uncertainty

- x87 expressions are evaluated in double, not 80-bit extended, and rounded at each image
  float store; sin/cos come from the CRT in double where the image runs FSIN/FCOS on the
  float input.
- Iterator and vector self-checks that cannot fail on a consistent container are not
  modelled (they are listed in each function's comment). Their EH unwinds are not modelled.
- 00496BD0 passes 0/1 as the 007B04C0/007B03C0 flag; the image's dword carries leftover EDX
  bits above the SETZ byte and the sampler reads only the low byte.
- 0049B1F0's 00495D40 fill record is uninitialised stack in the image and is not modelled.
- `path_runner_tick_0049a360` (cc13_left8) still reaches 00496BD0 through the
  `Left8Host::path_runner_update_members_00496bd0` hook; a host can forward it to
  `scene_traffic_update_members_00496bd0`.

## Skipped

None.
