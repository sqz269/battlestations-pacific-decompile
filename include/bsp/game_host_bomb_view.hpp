#pragma once

#include "bsp/frontend_states.hpp"
#include "bsp/observer_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp::game {

// Screen 24h, allocated as 58h bytes by 0068CC70 at 0068D1A3 and retained
// at interface+8Ch. That site's inline constructor is also 0068A2D0, represented
// by cc13_fe5::small_hud_screen_construct. This is the actual leaf block, not
// a second selection/camera state alongside it. Vtable 00CF7938, listener
// 00CF7920 at +8h; the older 00CF7928 description names the wrong table start.
struct GameBombViewScreenImage {
    std::uint32_t vtable_00;
    FrontEndScreen flags_04;
    std::uint8_t padding_06[2];
    NativeObserverOwnerStorage callback_owner_08;
    std::uint8_t callback_flag_18;
    std::uint8_t padding_19[3];
    float saved_transition_1c[3];       // 0067BAA0, restored by 0067B450
    float previous_position_28[3];      // 0067B4D0, advanced by 0067B7E0
    void* controlled_unit_34;           // 0068B199, from 00644230
    float scratch_38;                   // 0067BAA0 clears these; meaning provisional
    float scratch_3c;
    std::uint32_t selection_40;         // 0067B3D0 -> 0067B7E0
    std::uint32_t selection_44;
    std::uint8_t selection_pending_48;
    std::uint8_t padding_49[3];
    void* bomb_camera_4c;               // 0067B4D0, released by 0067B450
    void* torpedo_camera_50;
    float entered_at_54;                // 0067BAA0 -> 0067B7E0
};

static_assert(sizeof(GameBombViewScreenImage) == 0x58);
static_assert(offsetof(GameBombViewScreenImage, flags_04) == 0x04);
static_assert(offsetof(GameBombViewScreenImage, callback_owner_08) == 0x08);
static_assert(offsetof(GameBombViewScreenImage, callback_flag_18) == 0x18);
static_assert(offsetof(GameBombViewScreenImage, saved_transition_1c) == 0x1C);
static_assert(offsetof(GameBombViewScreenImage, previous_position_28) == 0x28);
static_assert(offsetof(GameBombViewScreenImage, controlled_unit_34) == 0x34);
static_assert(offsetof(GameBombViewScreenImage, scratch_38) == 0x38);
static_assert(offsetof(GameBombViewScreenImage, scratch_3c) == 0x3C);
static_assert(offsetof(GameBombViewScreenImage, selection_40) == 0x40);
static_assert(offsetof(GameBombViewScreenImage, selection_44) == 0x44);
static_assert(offsetof(GameBombViewScreenImage, selection_pending_48) == 0x48);
static_assert(offsetof(GameBombViewScreenImage, bomb_camera_4c) == 0x4C);
static_assert(offsetof(GameBombViewScreenImage, torpedo_camera_50) == 0x50);
static_assert(offsetof(GameBombViewScreenImage, entered_at_54) == 0x54);

} // namespace bsp::game
