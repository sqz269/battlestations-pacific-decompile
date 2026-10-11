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
// The only callers (00B06B10, 00B08210, 00B09510 at 00B0987F) discard the result.
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
    // Floating, 00B09510 for Sprite.
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

// 00B08210 `int __thiscall(floating definition, state, float t, int, const
// float matrix[16], float)`, RET 14h, vtable D5DCEC slot 10 (+28h). The matrix
// and the last argument are unused. A surface-floating, rotating sprite:
//  * fade gate on emitter+24h < 0.004 as in 00B06B10; cursor = [emitter+8]+1C4h.
//  * angle = +50h + +54h * (integral(RotationSpeed +84h) * pi/180 [00D5DAF8]) * scale.
//  * state+0..8 = integral(speed +2Ch) * scale * dir+18h + origin+0Ch; no
//    gravity or wind; then 00B0C8B0 when def+4Ch is set.
//  * alpha gate and premultiplied colour as in 00B06B10; then state+4 (y) = 0.
//  * size = state+58h * curve(+88h); frames A/B and blend as in 00B06B10.
//  * four 24h-byte vertices {xyz, corner angle, ARGB, A.u, A.v, B.u, B.v (u16),
//    blend, size}; the corner angles are angle+pi, angle-pi/2, angle+pi/2 and
//    angle with frame columns (14h,16h) (10h,16h) (14h,12h) (10h,12h). Skipped
//    when the cursor is null; the cursor advances by 90h; the room count +1BCh
//    drops by one either way; returns 1. No def+65h second buffer, no bounds check.
std::int32_t emit_floating_quad_00b08210(const void* definition, void* state, float t, ParticleCurves& curves,
                                         ParticleRandom& random);

// What 00B09510 reaches outside the definition and state.
class SpriteParticleHost {
public:
    virtual ~SpriteParticleHost() = default;
    // 00B6FCB0 with ECX = [[00F8C274]+1Ch]: the current camera's view matrix,
    // which the native copies (REP MOVSD, 16 dwords) before using it.
    virtual const float* camera_view_00b6fcb0() = 0;
    // 00B6DB70 on the scene node [emitter+8], when node+5Ch bit 1 is clear.
    virtual void refresh_world_00b6db70(void* node) = 0;
    // 004B62E0 BSP_Light_SetBaseDiffuse(light, const float rgba[4]).
    virtual void light_set_base_diffuse_004b62e0(void* light, const float rgba[4]) = 0;
};

// 00B09510 `int __thiscall(sprite definition, state, float t, int, const float
// matrix[16], float view_size_scale)`, RET 14h, vtable D5DD18 slot 10 (+28h).
// Defined as a Ghidra function by cc13_particles1 (00B09510..00B09FF7, INT3
// padding on both sides). Unlike Axial/Floating it uses the last argument.
//  * fade gate, angle (as 00B08210), position (speed, gravity, wind as in
//    00B06B10, plus an optional parameter def+48h along state+24h..2Ch), stored
//    to state+0..8; 00B0C8B0 when def+4Ch is set.
//  * alpha = +5Ch * curve(+38h) * emitter alpha, with NO 0.004 gate; size =
//    +58h * curve(+88h).
//  * near-camera fade: r = clamp(size * view_size_scale / z_view, 0, 1) with
//    z_view from 004142E0(state position, view); fade = (1 - r)^2 (inline pow);
//    unless def+65h is set, returns 0 when 0.007 > fade * alpha. The fade does
//    not reach the colour.
//  * dynamic light state+60h (when non-null): position = state position, plus
//    node+120h..128h when node+1B0h is set (after 00B6DB70 if node+5Ch bit 1 is
//    clear); diffuse = raw colour curves with 1.0; light+1F8h = max(0.01, size).
//  * colour premultiplied by alpha*255 and packed as in 00B06B10; additive
//    (def+7Ch == 1) sets alpha byte 1 and flag half 3C00h.
//  * four 38h-byte vertices {xyz, corner angle, ARGB, 1.0 x3, blend, A.u, A.v,
//    B.u, B.v (u16), size, angle, u16 0, u16 flag}; corner angle = angle +
//    {-pi, -pi/2, +pi/2, 0} + pi/4 with frame columns (10h,16h) (14h,16h)
//    (10h,12h) (14h,12h). Buffer selection, cursor +E0h and room -1 as in
//    00B06B10; no bounds check; returns 1.
std::int32_t emit_sprite_quad_00b09510(const void* definition, void* state, float t, const float matrix[16],
                                       float view_size_scale, const float wind_e13028[3], ParticleCurves& curves,
                                       ParticleRandom& random, SpriteParticleHost& host);

// Vtable slot 9 (+24h): `float* __thiscall(definition, float sphere[4], state,
// float t)`, RET 0Ch, EAX = sphere. A bounding sphere {centre xyz, radius} of
// one particle. Defined as Ghidra functions by cc13_particles1. Reached only
// through the vtables; the dispatching consumer is not yet identified (no
// CALL [reg+24h] in 00AE0000..00BC0000; MOV reg,[reg+24h]/CALL reg not scanned).
//  * 00B05DD0 Axial (D5DCC0): L = +58h * curve(+8Ch), W = +58h * curve(+90h);
//    axis = def+94h..9Ch, or with FollowDirection the 00419510-normalised curve
//    velocity, whose gravity term is (0, curve(+30h) * scale * +48h, 0): no
//    matrix, unlike 00B06B10. Centre = position + L*axis (Alignment 1),
//    position - L*axis (2), else the position; radius = sqrt(L*L + W*W)
//    (00BF7030).
//  * 00B07BF0 Floating (D5DCEC) and 00B08950 Sprite (D5DD18): centre = the
//    position, radius = +58h * curve(Size +88h) * 0.5 [00D7A280].
float* axial_bounding_sphere_00b05dd0(const void* definition, float sphere[4], const void* state, float t,
                                      const float wind_e13028[3], ParticleCurves& curves,
                                      const CameraAxesCrtAccess& crt);
float* floating_bounding_sphere_00b07bf0(const void* definition, float sphere[4], const void* state, float t,
                                         ParticleCurves& curves);
float* sprite_bounding_sphere_00b08950(const void* definition, float sphere[4], const void* state, float t,
                                       ParticleCurves& curves);

// Vtable slot 7 (+1Ch) ReleaseState, `void __thiscall(definition, state, record)`
// RET 8 with both stack arguments unread: decrement a live-particle counter.
//  * 00B00750 Axial: [00F8D380] -= 1. The only literal reference to 00F8D380 in
//    the image is this SUB (byte scan); the Axial initializer 00B059C0 does
//    not increment it, so natively it only goes down unless a non-literal
//    writer exists.
//  * 00B007A0 Floating: [00F8D384] -= 1, incremented by 00B077D0 at 00B07BBA
//    and 00B07BD6. (Sprite's slot is the existing 00B007F0, counter 00F8D388.)
void release_axial_state_00b00750(volatile std::int32_t& axial_count_00f8d380);
void release_floating_state_00b007a0(volatile std::int32_t& floating_count_00f8d384);

}  // namespace bsp::particle_emitters
