#pragma once

// 007F0280, the near-field avoidance box, packet cc9_near_field_probe.
// docs/NEAR_FIELD_PROBE.md (the accumulator) and docs/BOT_PROBE_007F0280.md
// (the arguments, the iteration and the eighteen callers). Reconstructed and
// build-tested; names are hypotheses, not recovered symbols.
//
// `void __thiscall(ECX = fallback list owner, unit, const float extents[3],
// float out_a[3], float out_b[3], const float weights[3], byte mode)`, RET 18h.
// Candidates (mode != 0: [unit+C50h]'s +30h list, kept when vtable[5Ch](0Fh);
// else ECX+3D0h for ECX+3CCh entries), self excluded (007F056F), each put in
// the unit's frame (004142E0 with unit+110h) and kept only inside the box
// |p.x| < e.x && |p.y| < e.y && |p.z| < e.z (007F05F7-007F06A9, strict).
//
// Per kept candidate (007F06AF-007F0916; loop x87 stack is [1.0], see the doc):
//   f_i = w_i > 0 ? 1 + w_i * |p_i| / e_i : 0            (+34h..+3Ch, reset per candidate)
//   p_i' = |p_i| < f_i ? (tie-break on unit+9D0h) +-f_i : p_i
//          x, z: candidate+9D0h <  self+9D0h -> +f, else -f  (JGE)
//          y:    candidate+9D0h >  self+9D0h -> +f, else -f  (JLE)
//   p_i' >= 0: P_i = min(P_i, p_i');  else N_i = min(N_i, -p_i')
// with P_i and N_i starting at e_i (007F02FE-007F031C, read at depth +4).
// Output, only when some candidate was kept (007F0936):
//   pos_i = e_i > P_i ? 1 - P_i / e_i : 0
//   neg_i = e_i > N_i ? N_i / e_i - 1 : 0
//   out_a_i = pos_i + neg_i                 (007F0A1D-007F0A71)
//   out_b_i = pos_i > -neg_i ? pos_i : -neg_i (007F0A74-007F0AED)
// Otherwise both out-triples keep the zeros written at entry.

#include <cstddef>

namespace bsp {

struct NearFieldCandidate {
    float local[3] = {0.0f, 0.0f, 0.0f};  // in the probing unit's frame
    int formation_index_9d0 = 0;          // candidate+9D0h
};

struct NearFieldProbeResult {
    bool hit = false;                     // [ESP+1Bh]
    float out_a[3] = {0.0f, 0.0f, 0.0f};  // arg2
    float out_b[3] = {0.0f, 0.0f, 0.0f};  // arg3
};

NearFieldProbeResult near_field_probe_007f0280(const float extents[3], const float weights[3],
                                               int self_index_9d0,
                                               const NearFieldCandidate* candidates,
                                               std::size_t count) noexcept;

// The dive-bomb attack run at 009C42B8: extents (80, 60, 120) (009C4258,
// 009C4268, 009C4287), weights (0, 0, 0), mode 1. 009C42BD-009C42C7 form
// -out_a.x * out_b.y * out_b.z; the host's attack-run rule negates
// `sampler_result`, so this returns out_a.x * out_b.y * out_b.z.
float near_field_attackrun_sampler_009c42bd(const NearFieldProbeResult& r) noexcept;

// The fly-over at 009C6B3A: extents (80, 70, 140) with flyabove+18h clear,
// (60, 70, 90) with it set (00CE5444, 00CE77B4, 00D04A24 / 00CEB4B0, 00D1A918),
// weights (30, 0, 0) (00CE38C8), mode 1. Slot entry-108 is written only when
// |out_a.x| > 0.05 (00D7A270, double): -out_a.z * out_b.x * out_b.y * 1.2
// (00CEC160, double), minus 0.2 (00CE3D10, double) when flyabove+18h is set;
// otherwise it keeps the 0.0 stored at 009C6AD9.
void near_field_flyover_extents_009c6aa3(bool flyabove_18, float out[3]) noexcept;
float near_field_flyover_slot_009c6b75(const NearFieldProbeResult& r, bool flyabove_18) noexcept;

}  // namespace bsp
