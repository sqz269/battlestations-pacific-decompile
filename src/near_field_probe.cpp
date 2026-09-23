// 007F0280, packet cc9_near_field_probe. See include/bsp/near_field_probe.hpp and
// docs/NEAR_FIELD_PROBE.md. Reconstructed and build-tested.
#include "bsp/near_field_probe.hpp"

namespace bsp {

namespace {
float magnitude(float v) noexcept { return v < 0.0f ? -v : v; }  // AND 7FFFFFFFh
}  // namespace

NearFieldProbeResult near_field_probe_007f0280(const float extents[3], const float weights[3],
                                               int self_index_9d0,
                                               const NearFieldCandidate* candidates,
                                               std::size_t count) noexcept {
    NearFieldProbeResult r;
    float pos_min[3] = {extents[0], extents[1], extents[2]};  // [ESP+50h..58h]
    float neg_min[3] = {extents[0], extents[1], extents[2]};  // [ESP+44h..4Ch]
    for (std::size_t k = 0; k < count; ++k) {
        const NearFieldCandidate& c = candidates[k];
        // 007F05F7-007F06A9: FCOMI extent vs |p|, JBE rejects.
        bool inside = true;
        for (int i = 0; i < 3; ++i) {
            if (!(extents[i] > magnitude(c.local[i]))) inside = false;
        }
        if (!inside) continue;
        r.hit = true;  // 007F06C0
        float p[3] = {c.local[0], c.local[1], c.local[2]};
        for (int i = 0; i < 3; ++i) {
            float f = 0.0f;  // 007F0580-007F058C
            if (weights[i] > 0.0f) {
                f = weights[i] * magnitude(c.local[i]) / extents[i] + 1.0f;
            }
            if (f > magnitude(p[i])) {  // 007F078C / 007F07DD / 007F0829
                bool positive;
                if (i == 1) {
                    positive = c.formation_index_9d0 > self_index_9d0;   // JLE at 007F07EF
                } else {
                    positive = c.formation_index_9d0 < self_index_9d0;   // JGE at 007F079E / 007F083B
                }
                p[i] = positive ? f : (-0.0f - f);
            }
            if (!(p[i] < 0.0f)) {                // COMISS / JC
                if (!(p[i] > pos_min[i])) pos_min[i] = p[i];
            } else {
                const float m = -p[i];
                if (!(m > neg_min[i])) neg_min[i] = m;
            }
        }
    }
    if (!r.hit) return r;  // 007F0936
    for (int i = 0; i < 3; ++i) {
        float pos = 0.0f;
        float neg = 0.0f;
        if (extents[i] > pos_min[i]) pos = 1.0f - pos_min[i] / extents[i];
        if (extents[i] > neg_min[i]) neg = neg_min[i] / extents[i] - 1.0f;
        r.out_a[i] = pos + neg;
        r.out_b[i] = (pos > -neg) ? pos : -neg;
    }
    return r;
}

float near_field_attackrun_sampler_009c42bd(const NearFieldProbeResult& r) noexcept {
    return r.out_a[0] * r.out_b[1] * r.out_b[2];
}

void near_field_flyover_extents_009c6aa3(bool flyabove_18, float out[3]) noexcept {
    out[0] = 60.0f;   // 00CEB4B0
    out[1] = 70.0f;   // 00CE77B4
    out[2] = 90.0f;   // 00D1A918
    if (!flyabove_18) {  // 009C6AFA JNE skips the override when the byte is set
        out[0] = 80.0f;   // 00CE5444
        out[2] = 140.0f;  // 00D04A24
    }
}

float near_field_flyover_slot_009c6b75(const NearFieldProbeResult& r, bool flyabove_18) noexcept {
    if (!(magnitude(r.out_a[0]) > 0.05)) return 0.0f;  // 009C6B61-009C6B75
    float v = static_cast<float>(-r.out_a[2] * r.out_b[0] * r.out_b[1] * 1.2000000476837158);
    if (flyabove_18) v = static_cast<float>(v - 0.2);  // 009C6BBE, 00CE3D10
    return v;
}

}  // namespace bsp
