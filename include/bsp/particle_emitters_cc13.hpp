#pragma once

// Particle geometry pass and per-type quad emitters, segment 82 (packet
// cc13_water2 particles, docs/CC13_PARTICLE_EMITTERS.md).
//
// Semantic MSVC Win32 C++ over the actual native storage (raw pointers and
// byte offsets), not binary replacements. Every routine was checked against
// the listing with x87 register arithmetic decoded from the opcode bytes.
// Values are rounded at the native float stores; extended intermediates are
// approximated with double, so results are close but not bit-exact.
//
// RNG RULE. 00B057C0 and 00B0C8B0 draw from 00BD2FC0 with ECX = 0, i.e.
// RandomStream::primary of the calling thread's RandomThreads state. On the
// main thread that is the same Mersenne state the gameplay draws use
// (00BD2F10 forwards the caller's stream to the same 00BD2ED0 lookup). These
// ports never draw themselves: the caller passes `ParticleRandom`, which must
// be a separate presentation stream (or stay behind an OFF switch) whenever it
// is wired into the host.

#include <cstdint>

namespace bsp {
struct CameraAxesCrtAccess;
}

namespace bsp::particle_emitters {

// A draw from 00BD2FC0 (ECX = stream). See the RNG RULE above.
class ParticleRandom {
public:
    virtual ~ParticleRandom() = default;
    virtual std::uint32_t next_u32_stream0() = 0;
};

// Parameter / curve objects: +04h constant float (kind 0) or row pointer,
// +0Ah u16 kind (0 constant, 1 linear, otherwise cubic/Hermite).
//   integrate: kind 0 inline as value * t; 1 -> 00AFFD20, else 00AFFCB0
//   evaluate:  kind 0 inline as value;     1 -> 00AFFA70, else 00AFFAE0
// The existing C++ bodies need their loading bindings, so the caller adapts
// them here.
class ParticleCurves {
public:
    virtual ~ParticleCurves() = default;
    virtual float integrate_linear_00affd20(const void* parameter, float t) = 0;
    virtual float integrate_hermite_00affcb0(const void* parameter, float t) = 0;
    virtual float evaluate_linear_00affa70(const void* curve, float t) = 0;
    virtual float evaluate_cubic_00affae0(const void* curve, float t) = 0;
};

// 00B057C0 `int __fastcall(int lo /*ECX*/, int hi /*EDX*/)`: one stream-0 draw,
// lo + draw % (unsigned)(hi - lo + 1) (DIV, unsigned). No direct caller in the
// image (rel32 census empty); kept for completeness.
std::int32_t random_int_inclusive_00b057c0(std::int32_t lo, std::int32_t hi, ParticleRandom& random);

// 00B06A90 `int __thiscall(definition, int frame)`, RET 4: frame index into
// [def+50h, def+54h]. def+62h set: lo + (frame - lo) % (hi - lo + 1) (IDIV,
// signed) when the span is positive, else lo. Clear: clamp to [lo, hi].
std::int32_t resolve_frame_index_00b06a90(const void* definition, std::int32_t frame);

// 00B0C8B0 `float __fastcall(state)`: texture-frame advance of a 6Ch state
// (state+64h = definition). Returns false and touches nothing when def+4Ch is
// clear; the native then returns with ST0 unchanged (no value pushed).
//   p = min(100, age+40h * 100 / life+44h) (double 100 [00D7A220], clamp
//   value 100.0f [00CE3D08]); scale = life / 100.
//   v = integral of def+5Ch over p; frame = float(v * scale * state+3Ch + state+34h).
//   def+61h set (random frames): state+30h = frame, d = frame - old frame;
//     when fmod(frame, 1) + d > 1: state+30h = state+38h and
//     state+38h = fmod(old +38h, 1) + (def+50h + draw % (def+54h - def+50h + 1))
//     (one stream-0 draw at 00B0C9AA); returns d.
//   clear: state+30h = frame, state+38h = frame + 1.0; returns frame.
// The only callers (00B06B10, 00B08210, 00B08F60) discard the result.
bool advance_texture_frame_00b0c8b0(void* state, ParticleCurves& curves, ParticleRandom& random,
                                    float& result);

// 00B05420 `void __fastcall(int lo /*ECX*/, int hi /*EDX*/, row* rows)`, RET 4:
// Hoare quicksort of 8-byte rows {u16 state index; float key at +4} into
// descending key order (far to near). The pivot is re-read from rows[mid]
// on every partition step (mid = (lo + hi) >> 1, arithmetic shift), and the
// right part is handled by the loop, the left part by recursion.
void sort_rows_descending_00b05420(std::int32_t lo, std::int32_t hi, std::uint8_t* rows);

// What 00B05550 reaches outside the container.
class ParticleGeometryHost {
public:
    virtual ~ParticleGeometryHost() = default;
    // 00B6DB60 on the model (container+4): its local matrix.
    virtual const float* model_local_matrix_00b6db60(void* model) = 0;
    // 00413920: dst = left * right.
    virtual void multiply_00413920(float dst[16], const float left[16], const float right[16]) = 0;
    // [[state+64h]]+28h: the type's quad emitter, (state, life percent, 0,
    // matrix, float) -> quads written. 00B06B10 for Axial, 00B08210 for
    // Floating, 00B09510 for Sprite (not yet a Ghidra function).
    virtual std::int32_t emit_quads_slot28(void* definition, void* state, float life_percent,
                                           std::int32_t zero, const float matrix[16], float argument) = 0;
};

// 00B05550 `void __thiscall(container, const float view[16], float argument)`,
// RET 8: the per-frame geometry pass of a 30h particle container.
//  * container+28h = 0; nothing when container+18h (state storage) is null or
//    the owner's [container+8]+0Ch +2Ch byte is clear.
//  * Owner +24h set: each row key = ((y*v[6] + x*v[2]) + z*v[10]) + v[14] of
//    its state position, then 00B05420(0, count-1) when container+10h > 1 and
//    count >= 2.
//  * Per row: matrix = model local * model+298h when model+1B0h and +1B1h are
//    both clear, else identity; life percent as in 00B0C8B0; quads +=
//    slot28(state, percent, 0, matrix, argument). container+28h = total.
void build_container_geometry_00b05550(void* container, const float view[16], float argument,
                                       ParticleGeometryHost& host);

// 00B06B10 `int __thiscall(axial definition, state, float t, int, const float
// matrix[16], float)`, RET 14h, vtable D5DCC0 slot 10 (+28h). Writes one
// four-vertex quad (4 x 38h bytes) and returns 1, or 0 when faded out.
// `wind_e13028` is the global vector [00E13028..00E13030].
std::int32_t emit_axial_quad_00b06b10(const void* definition, void* state, float t,
                                      const float matrix[16], const float wind_e13028[3],
                                      ParticleCurves& curves, ParticleRandom& random,
                                      const CameraAxesCrtAccess& crt);

}  // namespace bsp::particle_emitters
