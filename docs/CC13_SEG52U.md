# CC13 seg52U: segment 52 leftovers (008288D0-00851CB0)

Packet cc13-seg52U, under the cc9 gameplay terms approved on 2026-10-16 (local\cc13_gameplay_terms.txt).
The input was 131 functions in segment 52 that had no tag and no reconstruction record
(gameunit, stock, torpedo avoidance). Main had no tag or record for any of them when the packet
began (8cdd2db82).

New code: `src/cc13_seg52U.cpp` and `include/bsp/cc13_seg52U.hpp` (namespace
`bsp::cc13_seg52U`). These are new C++ interfaces over the Win32 layouts. Nothing in them is wired into bsp_game.
Names are hypotheses, not recovered symbols.

## Triage

| class | count | action |
| --- | --- | --- |
| struck cc9 ranges | 14 | left alone (no lease, tag or record) |
| STL template bodies | 101 | tagged `stl_shape`, evidence `cc13seg52u_stl`, bind-only |
| compiler-generated / STL-only members | 11 | tagged `stl_shape`, evidence `cc13seg52u_cg`, bind-only |
| hosted by cc9 | 1 | verified against the listing, ledger only |
| reconstructed | 4 | `src/cc13_seg52U.cpp` |

**Struck.** These sit inside 00836000-00837FFF and 0084E000-0084EFFF, where cc9 packets are live:

- 008360C0, 008362A0, 008363E0, 008364B0, 00836510, 00836680, 00836790, 008367F0, 00836EB0 and
  00837DE0;
- 0084EB60, 0084EC10, 0084EFB0 and 0084EFE0.

**Compiler-generated or STL-only members.** None of these contains any game logic:

- the 1Ch-byte ship Idle record (two `std::map` members):
  - 0082E1F0 construct;
  - 0082E5A0 copy-construct;
  - 0082DF00 assign;
  - 0082E280 destruct;
- the Idle and Traffic `std::vector` resize wrappers, 0082F8E0 and 0082FD20;
- 0084D480, a `std::list` copy-construct;
- 0084FA30, an 8-byte-pair vector erase (the shift loop, then `last -= 8`);
- 0084FC50, a NativeString tree node construct;
- 00847FE0, a swap-based backward move over 1Ch-byte strings;
- 008482F0, which calls 00847FE0 and returns `dest - n * 1Ch`.

## Hosted by cc9, verified

**0082D040 BSP_ShipClass_BuildBuoyancyElements.**

- **Host.** cc9 hosts it as `ship_buoyancy_build_list_0082d4ca` and its helpers
  (src/ship_buoyancy_elements.cpp). Its only host caller is `game_hosts_units.cpp` ~3351
  (`ship_buoyancy_build_from_model_0082fea9`). The list is bound by `kShipBuoyancyElementsBound`
  (~7170, true).
- **Checked against the listing, 0082D4CA..0082D690:**
  - **Arguments.** The argument order is (bottom, deck, ratio). 0082D1EA sorts `[ESP+C0h]` into
    the vector at `ESP+38h`. 0082D36A sorts `[ESP+C4h]` into `ESP+48h`.
  - **Samples.** The first sample (0082D585) reads `ESP+4Ch..50h`, which is the deck line. It
    goes into +08h and +1Ch. The second sample (0082D5D8) is the bottom line and goes into +0Ch.
    The host passes them the same way.
  - **Station z.** It is `float(i) * Length / (segments - 1) + (-Length * 0.5)`, with
    `[ESP+10h]` reused as `segments - 1` and reloaded from EBP at the loop end. A segment count of
    1 or less gives 0.
  - **Element fields:**
    - +10h = d - b;
    - waterline +04h = d*(1-r) + b*r, with (1-r) spilled as a double;
    - +14h = waterline - b, where the waterline is reloaded from its float store;
    - coefficient +00h = (B0h * 10.0 * 0.5) / draught / segments.
  - **The two elements per station.** The second element differs only in +18h:
    `-(A4h) * 0.5` against `A4h * 0.5`.
  - **Constants.** The double constants are 10.0 (00CE3DC0) and 0.5 (00D7A280), both exact, and
    the host uses the same float values.
- **Verdict: matches.** At the main thread's 24-bit precision, every operation rounds once, as
  the host's float arithmetic does.
- **Not re-read here:** the parts AABB block 0082D067..0082D192 (cc9 records it as computed
  and never read) and the vector-lifetime code.

## Reconstructed

| address | function | ABI | notes |
| --- | --- | --- | --- |
| 0084DDA0 | `squadron_block_begin_command_0084dda0` | `__thiscall(ECX = block; byte flag)`, RET 4 -> AL | Slot +78h of 00D0BD98 (dword 00D0BE10). Whole body 0084DDA0-0084E00C read. |
| 008390C0 | `kind_value_lookup_008390c0` | `__thiscall(ECX = owner; class)`, RET 4 -> ST0 | `{kind, float}` vector at owner+1DCh, class vtable +18h. |
| 00839160 | `kind_value_lookup_00839160` | same | Twin, class vtable +5Ch. |
| 00846A80 | `shipyard_destruct_00846a80` | `__fastcall(ECX = shipyard)`, RET | Eight vtable words, three vector tidies, unit base destruct. |

### 0084DDA0, the squadron command block's begin

The function runs in this order:

1. **Order and base begin.** The byte argument selects the order word: +54h when it is set,
   else +188h. The order is read before the base begin 0071F600(flag) runs. When the base
   begin answers false, the function returns 0.
2. **moveto (00E08F68) aimed at the block's own unit +224h.** The target is resolved by 00521EA0
   from the record at +58h.
   - When the leader (unit+3D0h) has byte +184h set, nothing is rewritten.
   - **On-water stop.** The order becomes `stop` 00E08F88 when all of these hold:
     - the unit has at most one plane (+3CCh <= 1);
     - the leader's state is +900h == 6, which is on the water;
     - the leader's speed (vtable +38h, spilled as a double) is below
       `007C47F0(unit+35Ch) * 0.5`. FCOMIP/JBE makes this test false on NaN.

     The target record is then reset to the .bss position 00F87574 with zeroed header words.
   - **Re-aimed moveto.** Otherwise the moveto is re-aimed:
     - the target is the leader's world position (00427EB0, not null-tested) clamped to the
       map (007EF8F0);
     - y = `U(0, 100)` from 00BD2F10 stream 1, plus tuning+53Ch;
     - the result is written through 00467C80(point, 0.0f).
   - Both arms then set +48h = 1.
3. **land (00E08FA0) or retreat (00E08F90), outside session mode 2.**
   - **Message.** When byte +228h is clear, or the leader is null, or the leader's +184h is
     clear, it sends MT_RESUME_AI_CONTROL: 0077C470(leader, 2, 0), then message 57h
     (vtable 00CECCB4, +4h = 1) routed by 0077C2A0(leader, &msg, 0, 0).
   - **Always.** vtable +40h(0) runs and +228h is cleared.
4. Returns 1.

This is a seaplane-on-the-water rule and an AI hand-back for land and retreat. It does **not**
touch level-bomb, goaway, torpedo or submarine logic. It reads the leader's +900h (== 6) but
does not write it.

### 008390C0 / 00839160

- **Callers.** Neither has a caller. A byte scan of the PE found no absolute dword holding
  either address, and a rel32 E8/E9 scan of .text found no call or jump to either. The scan tool
  was checked against a known slot dword: 00836790 is found at 00D09FA0. Both bodies are dead
  in this image.
- **Body.** Each keeps every `_SECURE_SCL` test the image makes:
  - first <= last, twice;
  - the null-container test before the compare;
  - `cur < last` (last re-read) before each dereference and each advance.

### 00846A80

- **Vtable words.** It stores eight vtable words, not the four the ledger name mentions:
  - +0h 00D0B770;
  - +10h 00D0B754;
  - +24h 00D0B74C;
  - +170h 00D0B748;
  - +1E4h 00D0B740;
  - +310h 00D0B728;
  - +38Ch 00D0B724;
  - +72Ch 00D0B718.
- **Tidies and base.** It then calls the tidies 008468F0 (+790h), 008468A0 (+780h) and
  00846850 (+770h), and finally 00959940.
- **Caller.** The only caller is the scalar deleting destructor 00847610 (rel32 at 00847613).

## Adoption notes (checked at the host callers)

1. **0084DDA0 is not hosted. Grade: MINOR, with no observable difference at the current host
   call site.**
   - **Host caller.** The host's only mention of it is in `GameUnitsHost::run_squadron_free_attack_009f8470`
     (game_hosts_units.cpp ~16634), labelled "0071F290 arm 5's begin (0084DDA0 -> 0071F600) is
     not run".
   - **Why it changes nothing there.** That path issues attack-class orders. The two arms of
     0084DDA0 that do anything fire only for moveto aimed at the squadron itself, and for land or
     retreat. So skipping it there changes nothing.
   - **Base begin.** The host's director stand-in `begin_command_base` (game_hosts_commands.cpp
     ~2713) records 0071F600 and returns false. It does not dispatch the squadron block's
     override.
   - **Live in a normal mission?** Only if the host later routes moveto, land or retreat to a
     squadron command block through vtable +78h. Then two things are missing:
     - the seaplane-on-water stop;
     - the MT_RESUME_AI_CONTROL hand-back.
   - **Shared RNG.** It is used: 00BD2F10 stream 1 in the re-aim arm.
2. **0082D040 matches** (see above).
3. **008390C0 / 00839160 and 00846A80 are not hosted by cc9.** I found no host function or switch
   by address, name or behaviour (git grep on src and include). These are new code only.

## Ledger corrections

- `BSP_Shipyard_Destruct` (00846A80): the name evidence lists four vtables. The body stores
  eight, listed above.
