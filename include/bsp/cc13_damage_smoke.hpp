#pragma once

// cc13_damage_smoke: 00822770, the damage-smoke slot count of 008227E0 (packet cc13-smokepc24).
// Read from the disk bytes 00822770-008227D3. Names are hypotheses, not recovered symbols.
//
// The image, __thiscall(ECX = the controller at unit+10A0h), one float stack arg (RET 4),
// the damage 1.0 - health that 00822813 passes:
//   00822774 mov eax,[esi+2Ch]; add eax,1; mov [esp+4],eax
//   0082277E fild dword [esp+4]      int32 (+2Ch + 1), exact
//   00822783 fmul dword [esp+10h]    m32 float, the damage; rounded to float at PC24
//   00822787 call 00BF7420           _ftol: truncation toward zero
//   0082278C n = min(that, +2Ch) by a signed jl; 008227AD grows the slots (00822560) only when
//            n is above the slot count by an unsigned compare.
// The main thread's x87 runs at precision control 24, so the product is rounded to float
// before the truncation. The host truncated the exact (double) product, which comes out ONE
// lower when the exact product lies within half a float ulp below an integer k <= +2Ch.
//
// Census (this installation, scripts/datatables/autoload/vehicleclasses.lua, 633 classes):
//   MaxNumber  classes  +2Ch+1  flipping damages
//       0        479       1    none (no slot; the host returns early)
//       1         13       2    none: a power of two only shifts the exponent, so exact
//       2          3       3    none: enumerated (3 x 0.33333331f = 1 - 2^-24 and
//                               3 x 0.66666663f = 2 - 2^-23 are representable)
//       3        138       4    none: a power of two, exact
// No mission script sets DamageSmoke (docs/CC13_DAMAGE_SMOKE.md). So the flip set
// is EMPTY for every class here: ON and OFF give the same slot counts in every mission. It
// becomes non-empty from MaxNumber 5 (k = 5 at damage 0.8333333f). The per-tick damage is not
// in the logs, so the census is analytic (local/smokepc24_census.py).

namespace bsp::cc13_damage_smoke {

// The behaviour switch, a sub-switch of kDamageSmokeDrawsBound (game_hosts_gunnery.cpp).
// True: the slot count truncates the float product, as 00822770 does at PC24. False: the
// host truncates the exact double product, as before.
// ON by cc9's order (2026-10-19), without a pair: faithful to the image, and the census
// above shows ON = OFF for every class in this installation.
constexpr bool kDamageSmokeSlotCountPc24Cc13Bound = true;

// 0082277E..00822787: _ftol(float(float(max_number + 1) * damage)), before the clamp to
// max_number. A volatile store forces the float rounding on any x87 or SSE build.
inline int damage_smoke_slot_product_pc24_00822770(int max_number, float damage) {
    volatile float product = static_cast<float>(max_number + 1) * damage;
    return static_cast<int>(product);
}

}  // namespace bsp::cc13_damage_smoke
