// cc13_left1: leftover game bodies with no host (packet cc13-left1, docs/CC13_LEFT1.md). Names
// are hypotheses; see the header for the hooks. Each body was transcribed from its full export
// listing.

#include "bsp/cc13_left1.hpp"
#include "bsp/cc13_camera_controllers.hpp"

#include <array>

namespace bsp::cc13_left1 {

// ---------------------------------------------------------------------------
// 005B8810 BSP_AmbientMessage_PlaySound (provisional), __thiscall(ECX = object; clip by value at
// ESP+4..+0Fh, entity at +10h), RET 10h, EAX = object; SEH handler 00C7215B, EH states 0..2.
// Evidence, listing 005B8810..005B89EE:
//  - 005B8830..005B8859: [+4h]..[+10h] = 0 and [+0h] = 0 (+14h is not written), the observer
//    sub-object at +18h: [+1Ch]/[+20h]/[+24h]/[+2Ch] = 0, byte [+28h] = 1, [+18h] = 00CE75CC.
//  - 005B8863..005B8884: the inlined set-observed with the old value a constant 0 (ECX = EBX):
//    nothing when entity == 0; 006952A0 is unreachable (Ghidra drops 005B8872); [+2Ch] = entity,
//    then 00694A60(ECX = entity, EDX = object+18h).
//  - 005B8889..005B88B6: 00702CC0(ECX = temporary slot, &clip, owned resource 0), 005B82C0(ECX =
//    object, EAX), 005B7FC0(temporary).
//  - 005B88BD: 007027B0(object) == 0: the "NOT FOUND" trace 00CF0E40 with ([record+4h] or
//    00E1952E, clip index), then return.
//  - 005B88F7..005B8977: camera = [[00E188A8]+19FCh]; 00B6DB70(camera) when byte camera+5Ch has
//    bit 2 clear; camera +120h..+128h copied; 00414DB0(entity) when byte entity+C8h is 0;
//    d = entity+FCh..+104h - camera position (FSUB, FSTP float each); 0042B2F0(&d).
//  - 005B897C..005B89AF, bytes DD 05 D8 0D CF 00 / D9 C0 / DE E2 (FSUBRP ST2,ST0) / DE F9
//    (FDIVP ST1,ST0): v = (2000.0 - length) / 2000.0, FSTP float; FCOMIP with 0.25f [00CE3868],
//    JBE keeps v (so NaN is kept), else v = +0.0; 00702130(object, v).
//  - 005B89B4..005B89D2: the "played" trace 00CF0E14 with the same two arguments.
// Uncertainty: modelled at 24-bit x87 precision (each operation rounds to float); at 53-bit the
// quotient would round once more on the float store. The only caller is not established (no
// rel32 or absolute reference); the object is the 30h element 005B83E0 walks at owner+64h.
// ---------------------------------------------------------------------------
void* ambient_message_voice_construct_005b8810(Left1Host& host, void* object, AmbientClip12 clip,
                                               void* entity) {
    at<std::uint32_t>(object, 0x04) = 0;
    at<std::uint32_t>(object, 0x08) = 0;
    at<std::uint32_t>(object, 0x0C) = 0;
    at<std::uint32_t>(object, 0x10) = 0;
    at<std::uint32_t>(object, 0x00) = 0;
    void* const observer = add(object, 0x18);
    at<std::uint32_t>(observer, 0x04) = 0;
    at<std::uint32_t>(observer, 0x08) = 0;
    at<std::uint32_t>(observer, 0x0C) = 0;
    at<std::uint32_t>(observer, 0x14) = 0;
    at<std::uint8_t>(observer, 0x10) = 1;
    at<std::uint32_t>(observer, 0x00) = kAmbientObserverVtable;

    void* const old_target = nullptr;  // 005B8863 MOV ECX,EBX
    if (old_target != entity) {
        if (old_target != nullptr) host.observer_unregister_pair_006952a0(old_target, observer);
        at<void*>(observer, 0x14) = entity;
        if (entity != nullptr) host.observer_register_pair_00694a60(entity, observer);
    }

    alignas(4) std::uint8_t temporary[0x18];
    void* const started = host.voice_slot_construct_and_start_00702cc0(temporary, &clip, 0);
    host.voice_slot_assign_005b82c0(object, started);
    host.voice_slot_destroy_005b7fc0(temporary);

    // 005B88C6 / 005B89B4: the text is read at each trace, after the calls before it.
    const auto trace = [&](const char* format) {
        const char* const text = at<const char*>(clip.record, 4);
        const std::uint32_t args[2] = {
            text != nullptr ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(text)) : kEmptyText,
            static_cast<std::uint32_t>(clip.index)};
        host.trace_printf_004254b0(format, args, 2);
    };
    if (host.voice_slot_poll_state_007027b0(object) == 0) {
        trace("Ambient message sound NOT FOUND for: %s (%d)");
        return object;
    }

    void* const camera = ptr(host.game_global(), 0x19FC);
    if ((at<std::uint8_t>(camera, 0x5C) & 2) == 0) host.transform_refresh_world_matrix_00b6db70(camera);
    const float camera_position[3] = {at<float>(camera, 0x120), at<float>(camera, 0x124),
                                      at<float>(camera, 0x128)};
    if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    const float d[3] = {at<float>(entity, 0xFC) - camera_position[0],
                        at<float>(entity, 0x100) - camera_position[1],
                        at<float>(entity, 0x104) - camera_position[2]};
    const float length = host.vector3_length_threshold_0042b2f0(d);
    const float fade = static_cast<float>(kAmbientFadeDistance);
    float volume = (fade - length) / fade;
    if (kAmbientMinimumVolume > volume) volume = 0.0f;
    host.voice_slot_set_volume_00702130(object, volume);
    trace("Ambient message sound played for: %s (%d)");
    return object;
}

// ---------------------------------------------------------------------------
// 00605630 BSP_Math_DirectionAngleDeltas (provisional), __fastcall(ECX = float* yaw, EDX =
// float* pitch; a.xyz at ESP+4, b.xyz at ESP+10h), RET 18h. Sole caller 006082D0
// BSP_PlaneHudScreen_UpdateBombFire.
// Evidence: listing 00605630..006057DA, compared instruction by instruction with 00794750 (the
// path camera's aim deltas): after normalising both mnemonic streams, the only differences are
// the prologue's MOV EDI,EDX and the output stores (00605752 FSTP [ESI], 00605759 MOVSS [ESI],
// 006057C4 FSTP [EDI], 006057D1 MOVSS [EDI] here; a local pair copied to [ESI]/[ESI+4] there).
// Bytes checked here: 00605652 D9 E8 DE F1 and 006056A4 D9 E8 DE F1 (FLD1; FDIVRP ST1,ST0 = 1 /
// length); 00605673 D9 C0 DE CA (FMULP ST2,ST0). So yaw = 00438B10(atan2(b.x, b.z), atan2(a.x,
// a.z)) over the normalised vectors and pitch = -00438B10(0042CF10(b.y), 0042CF10(a.y)), each 0
// when either angle fails 00BFA4EC; camera_direction_delta_00794750 computes exactly that.
// Uncertainty: the image writes *yaw before the two 0042CF10 calls, the reuse writes both after
// them (the callees touch no caller memory). Precision as documented for 00794750.
// ---------------------------------------------------------------------------
void direction_angle_deltas_00605630(float* yaw, float* pitch, const float a[3], const float b[3]) {
    const std::array<float, 2> delta =
        cc13cam::camera_direction_delta_00794750({a[0], a[1], a[2]}, {b[0], b[1], b[2]});
    *yaw = delta[1];
    *pitch = delta[0];
}

}  // namespace bsp::cc13_left1
