// Segment 59 TorpedoBot / DepthChargeBot slot bodies (packet s53_seg59_bots,
// docs/SHIP_AI_OPEN_ITEMS.md 264). Transcribed from the disk listing; no x87.

#include "bsp/seg59_bots_s53.hpp"

#include <cstddef>
#include <cstring>

namespace bsp {

namespace {

template <class T>
T load(const void* base, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(base) + offset, sizeof value);
    return value;
}

template <class T>
void store(void* base, std::size_t offset, T value) {
    std::memcpy(static_cast<unsigned char*>(base) + offset, &value, sizeof value);
}

// 008FF2B8..008FF2DD / 008FF508..008FF52D: the owner's role-0 slot +1ACh; slot 8
// or an AI-held slot sends gun->vtable[1E8h](0).
void release_gun_when_ai_held(void* bot, Seg59BotHost& host) {
    void* const owner = load<void*>(bot, 0x50);
    const std::int32_t slot = load<std::int32_t>(owner, 0x1ac);
    if (slot != 8 && !host.party_slot_is_ai_held_00927f10(slot)) return;
    host.gun_vcall_1e8(load<void*>(bot, 0x58), 0);
}

// 008FF31E..008FF329 / 008FC06D..008FC076: +58h = +50h, +5Ch = 0.0f (XORPS / MOVSS).
void cache_owner(void* bot) {
    store<void*>(bot, 0x58, load<void*>(bot, 0x50));
    store<float>(bot, 0x5c, 0.0f);
}

}  // namespace

void seg59_torpedo_bot_clear_fire_target_008ff2b0(void* bot, Seg59BotHost& host) {
    host.gun_bot_clear_fire_target_0072bd50(bot);   // 008FF2B3
    release_gun_when_ai_held(bot, host);
    store<std::uint8_t>(bot, 0x6c, 0);               // 008FF2DF, on both paths
}

void seg59_depth_charge_bot_clear_fire_target_008ff500(void* bot, Seg59BotHost& host) {
    host.gun_bot_clear_fire_target_0072bd50(bot);   // 008FF503
    release_gun_when_ai_held(bot, host);
}

void seg59_torpedo_bot_attach_008ff310(void* bot, void* node, Seg59BotHost& host) {
    host.gun_bot_attach_008fbc80(bot, node);        // 008FF319
    cache_owner(bot);
    void* const owner = load<void*>(bot, 0x50);
    // 008FF32E..008FF34A: +64h = owner when it answers IsKindOf(08h), else 0.
    if (owner != nullptr && host.entity_vcall_5c_is_kind_of(owner, 8)) {
        store<void*>(bot, 0x64, owner);
        return;
    }
    store<void*>(bot, 0x64, nullptr);
}

void seg59_depth_charge_bot_attach_008fc060(void* bot, void* node, Seg59BotHost& host) {
    host.gun_bot_attach_008fbc80(bot, node);        // 008FC068
    cache_owner(bot);
}

}  // namespace bsp
