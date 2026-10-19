// Library-tagged game rows, packet cc13-mistag (marker cc13_mistag). docs/CC13_MISTAG.md.
#include "bsp/cc13_mistag.hpp"

namespace bsp::cc13_mistag {
namespace {

template <class T>
T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}

void* singleton_scalar_deleting(MistagHost& host, void* object, std::uint32_t flags,
    std::uint32_t global) {
    host.clear_global(global);
    at<std::uint32_t>(object, 0) = kSingletonBaseVtable;
    if ((flags & 1) != 0) host.free_00bf65ac(object);
    return object;
}

}  // namespace

// Each row read from the disk bytes: B8 imm32 C3. The slot is the only .rdata dword equal to
// the routine's address (two for 0085EA30).
const TrivialGetter kTrivialGetters[kTrivialGetterCount] = {
    {0x008561D0, 0x00D0C410, 0x00CE544C, "Torpedo"},
    {0x0085EA30, 0x00D0D158, 0x00CEA74C, "WaterMine"},          // also slot 00D7CE88
    {0x009A28A0, 0x00D1F4FC, 0x00CFB630, "closetoship"},
    {0x009ADD20, 0x00D1FCCC, 0x00F8A424, nullptr},
    {0x009B33C0, 0x00D1FFB4, 0x00D20008, "LAND"},
    {0x009B33E0, 0x00D1FFDC, 0x00E08FA0, nullptr},
    {0x009B7B60, 0x00D2026C, 0x00F8A474, nullptr},
    {0x009BA820, 0x00D2039C, 0x00D203F0, "conditioned"},
    {0x009BAA90, 0x00D20494, 0x00D204E8, "gainaltitude"},
    {0x009C3B80, 0x00D20C1C, 0x00E08F78, nullptr},
    {0x009C79A0, 0x00D20E54, 0x00E08F20, nullptr},
    {0x009CC3F0, 0x00D210F4, 0x00CFB4D0, "strafe"},
    {0x009CFA00, 0x00D2123C, 0x00D21290, "takeoff"},
    {0x009DAC20, 0x00D215BC, 0x00E08F70, nullptr},
    {0x00749000, 0x00CFF79C, 0x00CFF7BC, "MLandFort"},
    {0x00951AE0, 0x00D19C00, 0x00D19C20, "MReconPlane"},
    {0x00951B30, 0x00D19C3C, 0x00D19C5C, "MPlaneFighter"},
    {0x00951C10, 0x00D19C7C, 0x00D19F34, "MPlaneDiveBomber"},
    {0x00951CF0, 0x00D19F58, 0x00D1A20C, "MPlaneTorpedoBomber"},
    {0x00951DD0, 0x00D1A230, 0x00D1A4E4, "MPlaneKamikaze"},
    {0x00951E20, 0x00D1A504, 0x00D1A524, "MPlaneBomber"},
    {0x00951E70, 0x00D1A544, 0x00D1A564, "MCommandBuilding"},
    {0x007AB9F0, 0x00D05088, 0x00CFA6C4, "Paratrooper"},
};

std::uint32_t trivial_getter_result(std::uint32_t address) noexcept {
    for (const TrivialGetter& g : kTrivialGetters)
        if (g.address == address) return g.value;
    return 0;
}

// 00735880..007358A8 (Ghidra CG_scalar_deleting_dtor_00735880; vtable slot 00CFEA04). The
// out-of-line base 00735770 is an undefined chunk.
void* singleton_scalar_deleting_destruct_00735880(MistagHost& host, void* object, std::uint32_t flags) {
    return singleton_scalar_deleting(host, object, flags, 0x010904F8);
}

// 00735900..00735928 (slot 00CFEA0C); out-of-line base 007357B0 undefined.
void* singleton_scalar_deleting_destruct_00735900(MistagHost& host, void* object, std::uint32_t flags) {
    return singleton_scalar_deleting(host, object, flags, 0x0109029C);
}

// 00866090..008660B8 (slot 00D0D3C8); out-of-line base 00866060 undefined.
void* singleton_scalar_deleting_destruct_00866090(MistagHost& host, void* object, std::uint32_t flags) {
    return singleton_scalar_deleting(host, object, flags, 0x00F87658);
}

// 00737110..00737147 (slot 00CFEA2C). The singleton base is the +4h subobject (the image takes
// 0 for a null this and would then store through 0; not modelled). Order as in the listing:
// global, +4h vtable, +0h vtable 00CFE9F4, then _free.
void* msar_provider_scalar_deleting_destruct_00737110(MistagHost& host, void* object, std::uint32_t flags) {
    host.clear_global(0x010904F8);
    at<std::uint32_t>(object, 4) = kSingletonBaseVtable;
    at<std::uint32_t>(object, 0) = 0x00CFE9F4;
    if ((flags & 1) != 0) host.free_00bf65ac(object);
    return object;
}

// 00996200..00996264. The two FSTP to [esp+4] round to float; the -0.0f at 00D7A208 is read
// from the image (bytes 00 00 00 80). FCOMIP with JBE: an unordered compare returns 0.
std::uint32_t angle_within_tolerance_00996200(MistagHost& host, void* condition) {
    const float current = host.object_vcall_50_angle(at<void*>(condition, 0x18));
    const float d = host.wrapped_angle_subtract_00438b10(at<float>(condition, 0x1C), current);
    const float magnitude = d > 0.0f ? d : -0.0f - d;     // 0099622A..00996240
    return at<float>(condition, 0x20) > magnitude ? 1u : 0u;
}

// 007B1A80..007B1AFF. The loop re-reads first/last each pass; index i is checked twice as the
// image does (first == null or i >= size -> 00BF6713) before the element is freed and nulled.
void point3_ptr_array_destruct_007b1a80(MistagHost& host, void* array) {
    at<std::uint32_t>(array, 0) = 0x00D053A4;
    for (std::uint32_t i = 0;; ++i) {
        auto* const first = at<void**>(array, 8);
        if (first == nullptr) break;
        const auto size = static_cast<std::uint32_t>(at<void**>(array, 0xC) - first);
        if (i >= size) break;
        void*& element = first[i];
        if (element != nullptr) {
            host.free_00bf65ac(element);
            element = nullptr;
        }
    }
    if (at<void*>(array, 8) != nullptr) host.free_00bf65ac(at<void*>(array, 8));
    at<std::uint32_t>(array, 0x08) = 0;
    at<std::uint32_t>(array, 0x0C) = 0;
    at<std::uint32_t>(array, 0x10) = 0;
}

// 00728B80..00728B8A: GUN_CREATE message (constructor 0072A1D0, sender 0072B640).
void gun_create_message_destruct_00728b80(MistagDtorHost& host, void* message) {
    at<std::uint32_t>(message, 0) = 0x00CFDE1C;
    host.net_entity_create_message_destruct_007668e0(message);
}

// 00858150..00858161: the effect-instance base shared by the trace-line, waterdrops, light and
// splash instances (their destructors store the same pair); +8h is the second subobject.
void effect_instance_base_destruct_00858150(MistagDtorHost& host, void* instance) {
    at<std::uint32_t>(instance, 0) = 0x00D0C88C;
    at<std::uint32_t>(instance, 8) = 0x00D0C888;
    host.ref_counted_destroy_base_00bd30f0(instance);
}

// 009A1BF0..009A1BFD: BotApproachCloseToShip block (constructor 009A1D60); ADD ECX,2Ch then
// JMP 0064A610 on the observed-owner member.
void bot_approach_close_to_ship_destruct_009a1bf0(MistagDtorHost& host, void* block) {
    at<std::uint32_t>(block, 0) = 0x00D1F470;
    host.observed_owner_base_destroy_0064a610(static_cast<std::uint8_t*>(block) + 0x2C);
}

// 009BE2B0..009BE2BD: BotApproachMoveTo block (constructor 009C1C30); ADD ECX,30h then
// JMP 0042BCA0 on the target-observer member.
void bot_approach_move_to_destruct_009be2b0(MistagDtorHost& host, void* block) {
    at<std::uint32_t>(block, 0) = 0x00D209CC;
    host.target_observer_destruct_0042bca0(static_cast<std::uint8_t*>(block) + 0x30);
}

// 007B1BD0..007B1BDA and 007B1BE0..007B1BEA: two byte-identical copies, the path component
// (constructor 007B28C0) over its Point3PtrArray base 007B1A80.
void path_component_destruct_007b1bd0(MistagDtorHost& host, void* component) {
    at<std::uint32_t>(component, 0) = 0x00D053D8;
    host.point3_ptr_array_destruct_007b1a80(component);
}
void path_component_destruct_007b1be0(MistagDtorHost& host, void* component) {
    at<std::uint32_t>(component, 0) = 0x00D053D8;
    host.point3_ptr_array_destruct_007b1a80(component);
}

// 0083B120..0083B34F, in listing order. Vector fields are {allocator, first, last, end} with the
// first pointer named by its offset; RefPtr slots release through InterlockedDecrement(+4h).
void game_settings_destruct_0083b120(SettingsDtorHost& host, void* settings) {
    void* const s = settings;
    at<std::uint32_t>(s, 0x000) = 0x00CE3994;
    // 0083B152..0083B1B5: std::vector<RefPtr> first +664h / last +668h, walked by a checked
    // index; a non-null slot is released and nulled.
    for (std::uint32_t i = 0;; ++i) {
        auto* const first = at<void**>(s, 0x664);
        if (first == nullptr) break;
        if (i >= static_cast<std::uint32_t>(at<void**>(s, 0x668) - first)) break;
        void*& slot = first[i];
        if (slot != nullptr) {
            host.ref_release(slot);
            slot = nullptr;
        }
    }
    // 0083B1B7..0083B209: 30h-byte elements first +740h / last +744h.
    if (auto* const first = at<std::uint8_t*>(s, 0x740)) {
        for (std::uint8_t* p = first; p != at<std::uint8_t*>(s, 0x744); p += 0x30)
            host.element30_destruct_0041e7a0(p);
        host.free_00bf65ac(at<void*>(s, 0x740));
    }
    at<std::uint32_t>(s, 0x740) = 0;
    at<std::uint32_t>(s, 0x744) = 0;
    at<std::uint32_t>(s, 0x748) = 0;
    host.eh_vector_destruct_00bf7c6e(static_cast<std::uint8_t*>(s) + 0x72C, 4, 2, 0x0041DE40);
    host.container_6b8_resize_0041f370(static_cast<std::uint8_t*>(s) + 0x6B8, 0);
    host.free_00bf6989(at<void*>(s, 0x6B8));
    if (at<void*>(s, 0x674) != nullptr) host.free_00bf65ac(at<void*>(s, 0x674));
    at<std::uint32_t>(s, 0x674) = 0;
    at<std::uint32_t>(s, 0x678) = 0;
    at<std::uint32_t>(s, 0x67C) = 0;
    if (at<void*>(s, 0x664) != nullptr) host.free_00bf65ac(at<void*>(s, 0x664));
    at<std::uint32_t>(s, 0x664) = 0;
    at<std::uint32_t>(s, 0x668) = 0;
    at<std::uint32_t>(s, 0x66C) = 0;
    // 0083B26F..0083B2B3: the vector at +3E4h (first +3E8h, last +3ECh).
    void* const v = static_cast<std::uint8_t*>(s) + 0x3E4;
    if (at<void*>(v, 4) != nullptr) {
        host.destroy_range_00421470(v, s, at<void*>(v, 4), at<void*>(v, 8));
        host.free_00bf65ac(at<void*>(v, 4));
    }
    at<std::uint32_t>(v, 4) = 0;
    at<std::uint32_t>(v, 8) = 0;
    at<std::uint32_t>(v, 0xC) = 0;
    // RefPtrs +3A8h then +3A4h: nulled only when they were set.
    for (std::size_t off : {0x3A8u, 0x3A4u}) {
        if (void* const object = at<void*>(s, off)) {
            host.ref_release(object);
            at<void*>(s, off) = nullptr;
        }
    }
    if (at<void*>(s, 0x1E0) != nullptr) host.free_00bf65ac(at<void*>(s, 0x1E0));
    at<std::uint32_t>(s, 0x1E0) = 0;
    at<std::uint32_t>(s, 0x1E4) = 0;
    at<std::uint32_t>(s, 0x1E8) = 0;
    host.clear_game_settings_publication_00f8753c();
    at<std::uint32_t>(s, 0x000) = 0x00CE3818;
}

// 0099A880..0099A934. Float constants read from the image: 00E0E2EC = 0.24f (3E75C28Fh, a .data
// dword with no writer in the index), 00CF8B3C = 256.0f, 00D7A260 = -1.0f.
void* pilot_bot_construct_0099a880(MistagHost& host, void* bot) {
    host.gun_bot_construct_base_0072bbd0(bot, host.pilot_robots_table_00f8a30c());
    at<std::uint32_t>(bot, 0x00) = 0x00D1F348;
    at<std::uint32_t>(bot, 0x1C) = 0x00D1F32C;
    for (std::size_t off = 0x58; off <= 0x6C; off += 4) at<std::uint32_t>(bot, off) = 0;
    host.eh_vector_construct_00bf7cd1(static_cast<std::uint8_t*>(bot) + 0x84, 0x1C, 2,
        0x0099A630, 0x0099A010);
    at<float>(bot, 0x70) = 0.0f;
    at<float>(bot, 0x78) = 0.24f;
    at<float>(bot, 0x80) = 256.0f;
    at<std::uint8_t>(bot, 0x7C) = 0;
    at<float>(bot, 0x74) = -1.0f;
    return bot;
}

// 0099BE30..0099BED1. base is dereferenced without a null test (+538h, +9D4h). Constants:
// 00D7A24C = 1.0f, 00CE81A8 = 0.12f (3DF5C28Fh), 00E0E2EC = 0.24f.
void* pilot_plan_construct_0099be30(MistagHost& host, void* plan, void* base) {
    host.eh_vector_construct_00bf7cd1(plan, 0xC8, 3, 0x0099BDB0, 0x007B3BA0);
    at<float>(plan, 0x258) = 1.0f;
    at<std::uint32_t>(plan, 0x25C) = 0;
    at<void*>(plan, 0x2F0) = base;
    at<std::uint32_t>(plan, 0x2F4) = at<std::uint32_t>(base, 0x538);
    at<std::uint32_t>(plan, 0x268) = 0;
    at<std::uint32_t>(plan, 0x2F8) = at<std::uint32_t>(base, 0x9D4);
    at<std::uint32_t>(plan, 0x2E0) = 7;
    at<float>(plan, 0x2B8) = 1.0f;
    at<float>(plan, 0x260) = 0.12f;
    at<float>(plan, 0x2EC) = 0.24f;
    at<float>(plan, 0x264) = 0.0f;
    host.pilot_bot_seed_plan_slots_0099b450(plan);
    return plan;
}

// 00816B00..00816C76. The two messages live in the routine's 1B08h-byte stack frame: the
// MT_SHIP_SYNC buffer (4Ch bytes at frame-1B0Ch) and the MT_SHIP_GUNS_SYNC buffer (1AB4h).
// The sync message has no destructor call; the guns message's 64h records are destroyed.
void unit_send_ship_sync_message_00816b00(MistagHost& host, void* unit, void* holder, std::int32_t phase) {
    alignas(4) std::uint8_t sync_buffer[0x4C];
    alignas(4) std::uint8_t guns_buffer[0x1AB4];
    bool full = false;                                                    // [esp+14h] byte
    void* const state = host.ship_sync_state_get_or_create_0080e240(holder);
    at<std::int32_t>(state, 0x14) += 1;
    if (at<std::int32_t>(state, 0x14) >= phase * 3 + 1 || at<std::uint8_t>(state, 0x0F) != 0) {
        full = true;
        at<std::int32_t>(state, 0x14) = 0;
    }
    void* const sync = host.unit_state_message_build_00813ca0(sync_buffer, unit, state, phase, full);
    const std::uint32_t peer = at<std::uint32_t>(holder, 0);
    host.entity_send_message_to_peer_00779fc0(unit, peer, sync);
    host.net_entity_holder_stamp_sync_tick_00779f80(unit, holder);

    std::uint8_t damaged = 0;                                             // [esp+13h]
    bool send_guns;
    if (phase >= 5) {
        send_guns = false;                                                // 00816BE3 -> 00816C4F
    } else if (phase > 0) {
        send_guns = true;                                                 // 00816BE7 -> 00816C09
    } else {
        if (phase == 0 && at<std::uint8_t>(state, 0x18) == 0) {
            // 00816BAA..00816BDE: the first gun part (+48h list, +44h links) answering
            // IsKindOf(22h) that is damaged sets both flags.
            for (void* part = at<void*>(unit, 0x48); part != nullptr; part = at<void*>(part, 0x44)) {
                if (host.is_kind_of_5c(part, 0x22) && host.unit_is_damaged_00859c60(part)) {
                    damaged = 1;
                    at<std::uint8_t>(state, 0x19) = 1;
                    break;
                }
            }
        }
        // 00816BE9 (00816BA8 jumps straight to 00816BF5 when +18h is set).
        send_guns = at<std::uint8_t>(state, 0x18) != 0 || at<std::uint8_t>(state, 0x19) != 0;
    }
    if (send_guns) {
        const bool force = phase == 0 && at<std::uint8_t>(state, 0x18) != 0;   // 00816BF5..00816C09
        void* const guns = host.ship_guns_sync_construct_0080fcf0(guns_buffer, unit, phase, force);
        host.entity_send_message_to_peer_00779fc0(unit, at<std::uint32_t>(holder, 0), guns);
        host.eh_vector_destruct_00bf7c6e(guns_buffer + 0x24, 0x44, 0x64, 0x0075A5B0);
    }
    at<std::uint8_t>(state, 0x19) = damaged;                             // 00816C59
    at<std::uint8_t>(state, 0x18) = phase > 0 ? 1 : 0;                   // SETG at 00816C56
}

}  // namespace bsp::cc13_mistag
