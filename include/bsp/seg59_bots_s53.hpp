#pragma once
// Segment 59, the TorpedoBot / DepthChargeBot slot bodies (packet s53_seg59_bots,
// docs/SHIP_AI_OPEN_ITEMS.md 264). Names are hypotheses, not recovered symbols.
// The bodies run over raw image memory through the host below; nothing here is
// wired: the gunnery host models the gameplay part of each (264 says where).
//
// Bot layout used: +50h the owner unit (its +1ACh is the role-0 party slot),
// +58h the cached gun object, +5Ch the think timer (float), +64h the owner when
// it answers IsKindOf(08h) (MSubmarine), +6Ch the TorpedoBot's fire-target byte.
#include <cstdint>

namespace bsp {

class Seg59BotHost {
public:
    virtual ~Seg59BotHost() = default;
    // 0072BD50 BSP_GunBot_ClearFireTarget, __thiscall(bot), RET.
    virtual void gun_bot_clear_fire_target_0072bd50(void* bot) = 0;
    // 00927F10 BSP_PartySlot_IsAiHeld, (int slot), RET 4, AL.
    virtual bool party_slot_is_ai_held_00927f10(std::int32_t slot) = 0;
    // gun vtable +1E8h, __thiscall(gun)(int), RET 4.
    virtual void gun_vcall_1e8(void* gun, std::int32_t value) = 0;
    // 008FBC80 BSP_GunBot_Attach, __thiscall(bot)(node), RET 4.
    virtual void gun_bot_attach_008fbc80(void* bot, void* node) = 0;
    // entity vtable +5Ch IsKindOf, __thiscall(entity)(int kind), RET 4, AL.
    virtual bool entity_vcall_5c_is_kind_of(void* entity, std::int32_t kind) = 0;
};

// 008FF2B0, TorpedoBot vtable 00D182B0 +30h, __thiscall(ECX = bot), RET.
void seg59_torpedo_bot_clear_fire_target_008ff2b0(void* bot, Seg59BotHost& host);
// 008FF500, DepthChargeBot vtable 00D18338 +30h, __thiscall(ECX = bot), RET.
void seg59_depth_charge_bot_clear_fire_target_008ff500(void* bot, Seg59BotHost& host);
// 008FF310, TorpedoBot vtable 00D182B0 +04h, __thiscall(bot)(node), RET 4.
void seg59_torpedo_bot_attach_008ff310(void* bot, void* node, Seg59BotHost& host);
// 008FC060, DepthChargeBot vtable 00D18338 +04h, __thiscall(bot)(node), RET 4.
void seg59_depth_charge_bot_attach_008fc060(void* bot, void* node, Seg59BotHost& host);

}  // namespace bsp
