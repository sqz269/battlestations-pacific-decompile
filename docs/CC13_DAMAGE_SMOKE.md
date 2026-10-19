# cc13-smokepc24: the damage-smoke slot count at PC24 (00822770)

Packet cc13-smokepc24. Switch `kDamageSmokeSlotCountPc24Cc13Bound` in
`include/bsp/cc13_damage_smoke.hpp`, ON by cc9's order (2026-10-19) with no pair: it is
faithful to the image, and the census below shows ON = OFF for every class. It is a sub-switch of `kDamageSmokeDrawsBound`
(ON); a `static_assert` in `damage_smoke_tick_008227e0` (`src/game_hosts_gunnery.cpp`)
enforces that. Names are hypotheses, not recovered symbols.

## The image (disk bytes 00822770-008227D3)

`__thiscall(ECX = the controller at unit+10A0h)`, one float stack arg (`RET 4`): the damage
1.0 - health that 00822813 passes.

| Address | Instruction | Meaning |
|---|---|---|
| 00822774 | `mov eax,[esi+2Ch]; add eax,1; mov [esp+4],eax` | +2Ch + 1, the class MaxNumber + 1 |
| 0082277E | `fild dword [esp+4]` | int32 (+2Ch + 1), exact |
| 00822783 | `fmul dword [esp+10h]` | m32 float, the damage; rounded to float (main thread PC24) |
| 00822787 | `call 00BF7420` | `_ftol`, truncation toward zero |
| 0082278C | `cmp eax,ecx; jl` | n = min(that, +2Ch), signed |
| 008227AD | unsigned compare with the slot count | grow only, via 00822560 |

ON: the host truncates `float(float(max + 1) * damage)`. OFF: it truncates the exact double
product, as before. The difference is one slot when the exact product lies within half a
float ulp below an integer k <= +2Ch (k = +2Ch + 1 is clamped to +2Ch on both sides).

The host keeps its `wanted > 0` guard, labelled: the image's `_ftol(NaN)` gives 80000000h,
which survives the signed min and becomes a huge resize at 008227AD.

## Census: which missions hit the half-ulp case

None. The per-tick damage is not in the logs, so the census is analytic
(`local/smokepc24_census.py`, run over this installation's
`scripts/datatables/autoload/vehicleclasses.lua`). No mission script sets DamageSmoke.

| DamageSmoke.MaxNumber | Classes | +2Ch + 1 | Flipping damage values |
|---|---|---|---|
| 0 | 479 | 1 | none (no slot ever; the host returns early) |
| 1 | 13 | 2 | none: a power of two, so the float product is exact |
| 2 | 3 | 3 | none: enumerated (below) |
| 3 | 138 | 4 | none: a power of two, so the float product is exact |

Total 633 classes.

- **Why 2 and 4 are exact.** Multiplying a float by a power of two only changes the exponent,
  so the product needs no rounding, and the float and double truncations agree.
- **The 3 enumeration.** For k = 1 and 2, every float damage from just above k/3 downward was
  walked until the float product fell below k. No damage below k/3 gives a float product
  that rounds up to k. For example, 3 x 0.33333331f = 1 - 2^-24 and 3 x 0.66666663f = 2 - 2^-23
  are both representable.
- **The first flippable case** is MaxNumber 5: k = 5 at damage 0.8333333f. Larger maxes flip
  more often (9: k = 7 at 0.7f and k = 9 at 0.9f; 49: 30 values).

So ON equals OFF for every unit in every mission of this installation.
