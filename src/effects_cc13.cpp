// Effect-instance virtuals and the sound instance constructor (cc13-effects1).
// Status: reconstructed, build-tested. No fixture, no game run, not ABI-compatible.
// Evidence per function below; full notes in docs/CC13_EFFECTS.md.

#include "bsp/effects_cc13.hpp"

namespace bsp::effects_cc13 {

// 00873AE0..00873C7F. ECX instance, stack definition then point, EAX instance, RET8.
// Image order: base prefix (00CEB130, ref 1, 00D0C8C0, active 1, point +10h,
// definition +14h), derived tables 00D0DD78/00D0DD74, sound 0, type 2.
// When the definition has samples (00873B5B JE skips both draws otherwise):
//   00873B61 XOR ECX,ECX; 00873B63 CALL 00BD2FC0 -> DIV by sample_count_24, the
//   remainder picks samples_20[r]; 00A7E490 creates the tracked sound from
//   (sample, +30h, +34h, byte +2Ch); 0054D4C0 assigns it to +1Ch (retain) and the
//   temporary reference is released (00873BA3..00873BC2).
//   With a sound: 00873BD8..00873C19 set_3d(world position 006E0CD0, point +5Ch).
//   00873C1B FLD [def+38h]; FADD ST0,ST0; FSTP float; 00873C26 XOR ECX,ECX;
//   00873C39 CALL 00BD2F10 U(0, float(2s)) -> ST0. 00873C43 sound+14h = 1.
//   00873C3E..00873C5D: float(2s) * 0.5 (qword 00D7A280), FSUBP from u, + 1.0
//   (qword 00D7A210), FSTP float to sound+20h.
// The draws go to `random` under their own site keys, never stream 0.
EffectSoundInstanceStorage& construct_effect_sound_instance_00873ae0(
    EffectSoundInstanceStorage& instance, const EffectSoundDefinitionFields& definition,
    PointEffectInstanceStorage& point, EffectInstanceServices& services,
    EffectPresentationRandom& random) {
    auto& prefix = instance.prefix;
    prefix.table_00 = 0x00ceb130;
    prefix.references_04.store(1, std::memory_order_relaxed);
    prefix.secondary_table_08 = 0x00d0c8c0;
    prefix.active_0c = 1;
    prefix.borrowed_10 = &point;
    prefix.borrowed_14 = const_cast<EffectSoundDefinitionFields*>(&definition);
    prefix.table_00 = 0x00d0dd78;
    prefix.secondary_table_08 = 0x00d0dd74;
    instance.sound_1c = nullptr;
    prefix.type_18 = 2;

    const std::uint32_t count = definition.sample_count_24;
    if (count == 0) return instance;
    const std::uint32_t word = random.next_u32(EffectRandomSite::sound_sample_00873b63);
    void* const sample = definition.samples_20[word % count];
    EffectTrackedSound* const created = services.create_tracked_sound_00a7e490(sample,
        definition.class_index_30, definition.type_index_34, definition.persistent_2c);
    if (created) created->retain(); // 0054D4C0 assign
    instance.sound_1c = created;
    if (created) created->release(); // temporary reference
    EffectTrackedSound* const sound = instance.sound_1c;
    if (!sound) return instance;

    const Vec3 velocity{point.fields_5c[0], point.fields_5c[1], point.fields_5c[2]};
    sound->set_3d_attributes_34(services.point_world_position_006e0cd0(point), velocity);

    const float spread = definition.pitch_rnd_38 + definition.pitch_rnd_38; // FSTP float
    const float u = random.uniform(EffectRandomSite::sound_pitch_00873c39, 0.0f, spread);
    auto& fields = sound->fields();
    fields.dirty_14 = 1;
    fields.pitch_20 = static_cast<float>(
        static_cast<double>(u) - static_cast<double>(spread) * 0.5 + 1.0);
    return instance;
}

// 00872630, primary +30h. ECX instance, RET. Sound vtable+8 with 0; the field stays set.
void stop_effect_sound_instance_00872630(EffectSoundInstanceStorage& instance) {
    if (instance.sound_1c) instance.sound_1c->stop_08(0);
}

// 00872650, primary +08h. No sound: MOV AL,1. Otherwise tail-jumps to sound vtable+0Ch.
bool effect_sound_instance_is_complete_00872650(EffectSoundInstanceStorage& instance) {
    return instance.sound_1c ? instance.sound_1c->is_finished_0c() : true;
}

// 00872670, primary +0Ch, RET4. FLD/FSTP copies the float argument to 00A79880.
void scale_effect_sound_instance_volume_00872670(EffectSoundInstanceStorage& instance,
    float factor, EffectInstanceServices& services) {
    if (instance.sound_1c) services.scale_sound_volume_00a79880(*instance.sound_1c, factor);
}

// 00872690, primary +10h, RET4. MOVSS to sound+20h, then byte sound+14h = 1.
void set_effect_sound_instance_pitch_00872690(EffectSoundInstanceStorage& instance, float pitch) {
    if (!instance.sound_1c) return;
    auto& fields = instance.sound_1c->fields();
    fields.pitch_20 = pitch;
    fields.dirty_14 = 1;
}

// 008726C0, primary +1Ch, stack (index, t), RET8. Sound vtable+28h fills {id, lo, hi}
// for `index`; x87 at 008726E7..00872708: lo + (hi - lo) * t, rounded to float at
// 00872708, then sound vtable+2Ch(id, value).
void set_effect_sound_instance_parameter_fraction_008726c0(EffectSoundInstanceStorage& instance,
    std::int32_t index, float t) {
    EffectTrackedSound* const sound = instance.sound_1c;
    if (!sound) return;
    const EffectSoundParameterRange range = sound->parameter_range_28(index);
    const float value = static_cast<float>(static_cast<long double>(range.lo) +
        (static_cast<long double>(range.hi) - range.lo) * static_cast<long double>(t));
    sound->set_parameter_2c(range.id, value);
}

// 00872720, primary +24h. No sound: XOR AL,AL. Otherwise byte +15h of 00A81860(sound+4Ch).
bool effect_sound_instance_sample_flag15_00872720(EffectSoundInstanceStorage& instance,
    EffectInstanceServices& services) {
    if (!instance.sound_1c) return false;
    return services.sample_flag15_00a81860(instance.sound_1c->fields().sample_4c);
}

// 00872F40, primary +28h, stack (dt, reference node), RET8. Neither argument is read.
// Copies point +5Ch..+64h, refreshes the point node world matrix (00B6DB70 when
// node+5Ch bit 1 is clear) and passes node +120h..+128h; the same as 006E0CD0 inline.
void update_effect_sound_instance_00872f40(EffectSoundInstanceStorage& instance, float,
    void*, const PointEffectInstanceStorage& point, EffectInstanceServices& services) {
    EffectTrackedSound* const sound = instance.sound_1c;
    if (!sound) return;
    const Vec3 velocity{point.fields_5c[0], point.fields_5c[1], point.fields_5c[2]};
    sound->set_3d_attributes_34(services.point_world_position_006e0cd0(point), velocity);
}

// 00872FB0, primary +20h, RET4. SSE clamp: 0 when 0 > level (COMISS/JA), 1.0
// ([00D7A24C]) when level > 1.0, else level. Then FUCOMPI against sound+40h; on
// inequality (or unordered) store and set +14h.
// NaN: both COMISS are unordered, neither JA is taken, MOVAPS copies the NaN;
// FUCOMPI is unordered (PF=1), JNP is not taken, and the NaN is stored.
void set_effect_sound_instance_level_00872fb0(EffectSoundInstanceStorage& instance, float level) {
    EffectTrackedSound* const sound = instance.sound_1c;
    if (!sound) return;
    float clamped;
    if (0.0f > level) clamped = 0.0f;
    else if (level > 1.0f) clamped = 1.0f;
    else clamped = level;
    auto& fields = sound->fields();
    if (fields.level_40 != clamped) { // FUCOMPI: unordered also counts as different
        fields.level_40 = clamped;
        fields.dirty_14 = 1;
    }
}

// 00873DE0 between the table writes (00D0DD74 first, 00D0C888 last, then
// BSP_RefCounted_DestroyBase): sound vtable+8 with 1, release +1Ch, clear it.
void destruct_effect_sound_instance_00873de0(EffectSoundInstanceStorage& instance) {
    if (instance.sound_1c) instance.sound_1c->stop_08(1);
    if (EffectTrackedSound* const sound = instance.sound_1c) {
        sound->release();
        instance.sound_1c = nullptr;
    }
}

// 008721C0, primary +08h. MOVSS ring+20h, UCOMISS against 0.0 ([00D7A218]); equal
// (ZF=1, PF=0) returns 1, unordered or different returns 0. No ring returns 1.
bool effect_splash_instance_is_complete_008721c0(const EffectSplashInstanceStorage& instance) {
    if (!instance.ring_1c) return true;
    return instance.ring_1c->timer_20 == 0.0f;
}

// 00872DF0, primary +30h. ring+20h = 0, 00B9E4A0 on the ocean ([00E188A8]+19F0h)+A8h
// with the ring, +1Ch = 0. Unlike the destructor 00872D60, no check of +19F0h.
void stop_effect_splash_instance_00872df0(EffectSplashInstanceStorage& instance,
    EffectInstanceServices& services) {
    if (!instance.ring_1c) return;
    instance.ring_1c->timer_20 = 0.0f;
    services.remove_water_ring_00b9e4a0(instance.ring_1c);
    instance.ring_1c = nullptr;
}

// 00872E30, primary +28h, RET8. FLD ring+20h; FSUB dt; FSTP float; FCOMPI with 0:
// JBE (below, equal or unordered) stores 0, otherwise the rounded difference.
void update_effect_splash_instance_00872e30(EffectSplashInstanceStorage& instance, float dt) {
    if (!instance.ring_1c) return;
    const float left = static_cast<float>(
        static_cast<long double>(instance.ring_1c->timer_20) - static_cast<long double>(dt));
    instance.ring_1c->timer_20 = left > 0.0f ? left : 0.0f;
}

// 008720F0, primary +08h. No flare: MOV AL,1; otherwise tail-jumps to 00B8B670.
bool effect_flare_instance_is_complete_008720f0(EffectNodeInstanceStorage& instance,
    EffectInstanceServices& services) {
    return instance.payload_1c ? services.flare_pair_test_00b8b670(instance.payload_1c) : true;
}

// 008720A0, primary +28h, RET8: 00B8D150(flare; [[00E188A8]+19FCh], dt).
void update_effect_flare_instance_008720a0(EffectNodeInstanceStorage& instance, float dt,
    EffectInstanceServices& services) {
    if (instance.payload_1c) services.flare_update_00b8d150(instance.payload_1c, dt);
}

// 008720D0, primary +30h: 00B6DFA0 then +1Ch = 0, only when set.
void stop_effect_flare_instance_008720d0(EffectNodeInstanceStorage& instance,
    EffectInstanceServices& services) {
    if (!instance.payload_1c) return;
    services.unlink_and_release_node_00b6dfa0(instance.payload_1c);
    instance.payload_1c = nullptr;
}

// 00872120, primary +08h. Disabled waterdrops (byte 00F889FC == 0) report complete;
// otherwise byte +20h set returns 0, a payload tail-jumps to 00BC1150, none returns 1.
bool effect_waterdrops_instance_is_complete_00872120(EffectNodeInstanceStorage& instance,
    EffectInstanceServices& services) {
    if (!services.waterdrops_enabled_00f889fc()) return true;
    if (instance.suppressed_20) return false;
    if (!instance.payload_1c) return true;
    return services.waterdrops_finished_00bc1150(instance.payload_1c);
}

// 00872BF0, primary +30h: 00B6DFA0 when set; +1Ch = 0 on both paths (00872BFF, 00872C06).
void stop_effect_waterdrops_instance_00872bf0(EffectNodeInstanceStorage& instance,
    EffectInstanceServices& services) {
    if (instance.payload_1c) services.unlink_and_release_node_00b6dfa0(instance.payload_1c);
    instance.payload_1c = nullptr;
}

// 00872D30, primary +30h: 00B7C160, reload +1Ch, 00B6DFA0 and +1Ch = 0.
void stop_effect_light_instance_00872d30(EffectNodeInstanceStorage& instance,
    EffectInstanceServices& services) {
    if (!instance.payload_1c) return;
    services.unlink_light_models_00b7c160(instance.payload_1c);
    if (!instance.payload_1c) return;
    services.unlink_and_release_node_00b6dfa0(instance.payload_1c);
    instance.payload_1c = nullptr;
}

// 00865FE0. CMP [+1Ch],0 / JG false (+1Ch is the pending array count, 008679D0 appends
// retired children there); byte +0Ah set -> false; then every entries_0c slot must be
// null (NEG/SBB/TEST safe-bool idiom per slot). Returns 1 only when all hold.
bool point_effect_is_finished_00865fe0(const PointEffectInstanceStorage& point) {
    if (point.auxiliary_18.count > 0) return false;
    if (point.field_0a != 0) return false;
    RenderCommandReference* const* slot = point.entries_0c.begin;
    RenderCommandReference* const* const end = slot + point.entries_0c.count;
    for (; slot != end; ++slot)
        if (*slot) return false;
    return true;
}

} // namespace bsp::effects_cc13
