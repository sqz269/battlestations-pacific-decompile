#include "bsp/oct18b_nc_low.hpp"

#include <cstring>

namespace bsp::support_seg61_cc13 {
namespace {
void* singleton_scalar_delete_00924150(Raw self, std::uint8_t flags,
                                      std::uint32_t global, Seg61Host& host) {
    host.global_u32(global) = 0;
    self.at<std::uint32_t>(0) = 0x00ce3818;
    void* result = self.p;
    if ((flags & 1) != 0) host.operator_delete_00bf65ac(self.p);
    return result;
}
}
void* pending_init_lock_scalar_delete_00924150(Raw self, std::uint8_t flags, Seg61Host& host) {
    return singleton_scalar_delete_00924150(self, flags, 0x00f899e4, host);
}
void* pending_entity_lock_scalar_delete_009241d0(Raw self, std::uint8_t flags, Seg61Host& host) {
    return singleton_scalar_delete_00924150(self, flags, 0x00f899e8, host);
}
void* deferred_event_lock_scalar_delete_00924250(Raw self, std::uint8_t flags, Seg61Host& host) {
    return singleton_scalar_delete_00924150(self, flags, 0x00f899ec, host);
}
void* sound_request_queue_scalar_delete_00940da0(Raw self, std::uint8_t flags, Seg61Host& host) {
    return singleton_scalar_delete_00924150(self, flags, 0x00f89b34, host);
}

// 0094F700..0094F762: strlen loop, fill current block, advance at equality,
// memcpy min(remaining, block_size-used), update +14h. The NUL is excluded.
Raw block_buffer_append_text_0094f700(Raw buffer, const char* text, Seg61Host& host) {
    auto remaining = static_cast<std::uint32_t>(std::strlen(text));
    while (remaining != 0) {
        if (buffer.at<std::uint32_t>(0x14) == buffer.at<std::uint32_t>(4))
            block_buffer_next_block_0094f680(buffer, host);
        std::uint32_t count = buffer.at<std::uint32_t>(4) - buffer.at<std::uint32_t>(0x14);
        if (remaining <= count) count = remaining;
        auto** blocks = static_cast<std::uint8_t**>(buffer.ptr(8));
        std::memcpy(blocks[buffer.at<std::uint32_t>(0x0c)] + buffer.at<std::uint32_t>(0x14), text, count);
        buffer.at<std::uint32_t>(0x14) += count;
        text += count;
        remaining -= count;
    }
    return buffer;
}
}

namespace bsp::cc13_seg49 {
namespace {
// 0092BAA7..0092BAC9 / 0092BADE..0092BAFA. Keep x87 accumulation and
// the explicit float store before adding the half-height. MSVC Win32 only.
float transformed_y_0092b920(const void* transform, const float* vector) {
    float result;
    __asm {
        mov eax, transform
        mov edx, vector
        fld dword ptr [eax + 4]
        fmul dword ptr [edx]
        fld dword ptr [eax + 10h]
        fmul dword ptr [edx + 4]
        faddp st(1), st(0)
        fld dword ptr [eax + 1ch]
        fmul dword ptr [edx + 8]
        faddp st(1), st(0)
        fadd dword ptr [eax + 28h]
        fstp result
    }
    return result;
}
}

// 0092B920..0092BC5F, vtable 00D19500 slot 55. Effects are culled below
// -0.5, spray runs for 40 seconds, debris starts below -5 once, shapes lose
// bit 8 after +388h goes negative, and both bounds below settings+3F4h kill.
void sinking_wreck_update_0092b920(Raw entity, float delta, SinkingWreckHost& host) {
    if (delta == 0.0f || entity.ptr(0x354) == nullptr) return;
    for (std::uint32_t index = 0; index < entity.at<std::uint32_t>(0x334); ++index) {
        auto** effects = static_cast<void**>(entity.ptr(0x330));
        if (effects[index] == nullptr) continue;
        Raw node{static_cast<std::uint8_t*>(entity.ptr(0x350))};
        if ((node.at<std::uint8_t>(0x5c) & 2) == 0) host.transform_refresh_world_matrix_00b6db70(node.p);
        const auto* local = static_cast<const float*>(entity.ptr(0x324));
        if (!(static_cast<double>(local[index * 3 + 1]) + node.at<float>(0x124) < -0.5)) continue;
        void* effect = static_cast<void**>(entity.ptr(0x330))[index];
        host.point_effect_stop_children_00867b10(effect);
        Raw{static_cast<std::uint8_t*>(effect)}.at<std::uint8_t>(9) = 1;
        void*& slot = static_cast<void**>(entity.ptr(0x330))[index];
        Raw counted{static_cast<std::uint8_t*>(slot)};
        if (slot != nullptr) {
            if (host.interlocked_decrement(reinterpret_cast<volatile long*>(counted.p + 4)) == 0)
                host.refcounted_virtual_0(slot);
            slot = nullptr;
        }
        slot = nullptr;
    }
    const float elapsed = entity.at<float>(0x38c) + delta;
    entity.at<float>(0x38c) = elapsed;
    if (elapsed < 40.0f) {
        hull_surface_spray_0092a6d0(host, entity);
        hull_side_spray_0092a8c0(host, entity);
    }
    entity.at<float>(0x388) -= delta;
    float first[3], second[3];
    host.physics_body_bounds_00c31f90(entity.ptr(0x354), first, second);
    const float half = static_cast<float>(static_cast<double>(first[1]) * 0.5);
    first[0] = first[1] = second[0] = second[1] = 0.0f;
    const void* transform = host.physics_body_transform_00c32000(entity.ptr(0x354));
    const float second_y = transformed_y_0092b920(transform, second) + half;
    transform = host.physics_body_transform_00c32000(entity.ptr(0x354));
    const float first_y = transformed_y_0092b920(transform, first) + half;
    if (host.game_settings_00424c40().at<std::uint8_t>(0x680) != 0 &&
        entity.at<std::uint8_t>(0x394) == 0 && second_y < -5.0f && first_y < -5.0f) {
        entity.at<std::uint8_t>(0x394) = 1;
        transform = host.physics_body_transform_00c32000(entity.ptr(0x354));
        float matrix[16];
        host.physics_transform_expand_00c33650(transform, matrix);
        host.physics_body_bounds_00c31f90(entity.ptr(0x354), first, second);
        float span[3]{second[0] - first[0], second[1] - first[1], second[2] - first[2]};
        const float mass = host.physics_body_mass_00c31fc0(entity.ptr(0x354));
        host.spawn_sinking_wreck_debris_00935540(entity.ptr(0x390), matrix, mass, span);
    }
    bool submerged = false;
    if (second_y < host.game_settings_00424c40().at<float>(0x3f4))
        submerged = first_y < host.game_settings_00424c40().at<float>(0x3f4);
    if (entity.at<float>(0x388) < 0.0f) {
        for (void* shape = host.physics_first_shape_00c31dc0(entity.ptr(0x354)); shape != nullptr;
             shape = Raw{static_cast<std::uint8_t*>(shape)}.ptr(0x208)) {
            if ((Raw{static_cast<std::uint8_t*>(shape)}.at<std::uint8_t>(0x30) & 8) != 0)
                host.physics_shape_clear_flags_00c47f60(shape, 8);
        }
    }
    if (submerged) entity_detach_and_kill_00929d70(host, entity);
}
}

namespace bsp::cc13_left61 {
void single_effect_entity_update_0094f9e0(Raw entity, float, SingleEffectUpdateHost& host) {
    if (entity.ptr(0x1e4) != nullptr && entity.ptr(0x1e8) == nullptr) {
        if (entity.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_00414db0(entity.p);
        Raw game{reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(host.global_u32(0x00e188a8)))};
        const auto kind = game.at<std::uint32_t>(0x19ec);
        void* definition = entity.ptr(0x1e4);
        if (definition != nullptr)
            host.interlocked_increment_ce221c(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(definition) + 4));
        void* temporary = nullptr;
        void** created = host.point_effect_create_00868420(&temporary, kind, definition,
            reinterpret_cast<const float*>(entity.field(0xcc)), 0, 0);
        host.handle_assign_004845d0(entity.field(0x1e8), created);
        if (temporary != nullptr) {
            if (host.interlocked_decrement_ce2220(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(temporary) + 4)) == 0)
                host.object_release_slot0(temporary);
            temporary = nullptr;
        }
    }
    void* effect = entity.ptr(0x1e8);
    if (effect == nullptr) {
        host.mission_entity_kill_00926d90(entity.p, 2);
        return;
    }
    if (entity.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_00414db0(entity.p);
    host.point_effect_set_matrix_0053d9c0(entity.ptr(0x1e8), reinterpret_cast<const float*>(entity.field(0xcc)));
    if (host.point_effect_finished_00865fe0(entity.ptr(0x1e8))) {
        Raw{static_cast<std::uint8_t*>(entity.ptr(0x1e8))}.at<std::uint8_t>(9) = 1;
        host.effect_reference_adopt_00484620(&entity.ptr(0x1e8), nullptr);
    }
}
}

namespace bsp::cc13_seg49_64_78 {
std::uint32_t event_param_kind_00968090() noexcept { return 0; }
std::uint32_t event_param_kind_009680f0() noexcept { return 1; }
std::uint32_t event_param_kind_00968130() noexcept { return 2; }
std::uint32_t event_param_kind_00968170() noexcept { return 4; }

void* event_param_scalar_delete_009682b0(void* parameter, std::uint8_t flags,
                                       support_seg61_cc13::Seg61Host& host) {
    *static_cast<std::uint32_t*>(parameter) = 0x00d1af18;
    if ((flags & 1) != 0) host.operator_delete_00bf65ac(parameter);
    return parameter;
}
void* event_param_scalar_delete_009682d0(void* parameter, std::uint8_t flags,
                                       support_seg61_cc13::Seg61Host& host) {
    return event_param_scalar_delete_009682b0(parameter, flags, host);
}
void* event_param_scalar_delete_009682f0(void* parameter, std::uint8_t flags,
                                       support_seg61_cc13::Seg61Host& host) {
    return event_param_scalar_delete_009682b0(parameter, flags, host);
}

// Same ordered, inclusive comparisons as the existing RangeFilter::matches;
// NaN fails either FCOMI/JC comparison.
bool range_filter_matches_00968440(const RangeFilter& filter, const EventParam& parameter) noexcept {
    return parameter.number04 >= filter.min && filter.max >= parameter.number04;
}

bool kill_warning_is_duplicate_0096ac60(const WarningObject& self, const WarningObject& other,
                                       KillWarningHost& host) {
    if (other.type != 3) return false;
    // Type 3's 70h-byte warning stores the entity directly at +6Ch; this
    // is not the observer/entity (+6Ch/+80h) layout of other warning types.
    void* entity = reinterpret_cast<void*>(static_cast<std::uintptr_t>(other.word_6c));
    if (!host.is_kind_of_5c(entity, 0x18) && !host.is_kind_of_5c(entity, 0x0f) &&
        !(host.is_kind_of_5c(entity, 6) && host.torpedo_boat_or_small_landing_ship_00827f70(
            *reinterpret_cast<void**>(static_cast<std::uint8_t*>(entity) + 0x538))))
        return false;
    return warning_is_duplicate_0096aa40(self, other, host);
}
float warning_default_lifetime_0096f230() noexcept { return 5.0f; }
bool confirm_warning_is_duplicate_00973ed0(const WarningObject&, const WarningObject&) noexcept { return false; }
bool string_list_warning_is_duplicate_00976290(const WarningObject&, const WarningObject&) noexcept { return false; }
}

namespace bsp {
const char* cruiser_class_name_00963be0() noexcept { return "MCruiser"; }
bool submarine_class_is_kind_of_00963e10(std::uint32_t kind) noexcept {
    return kind == 8 || kind == 6 || kind == 5 || kind == 4;
}
}
