// Leftover game bodies with no host (packet cc13-left1, docs/CC13_LEFT1.md).
//
// 005B8810 builds the 30h-byte ambient-message voice object and starts its sound; 00605630 is
// the plane HUD's copy of the direction angle deltas (the same arithmetic as the path camera's
// 00794750, src/cc13_camera_controllers.cpp, with a different ABI).
//
// Model: the bodies work on the original objects as raw bytes (Win32 layout, offsets as in the
// image) and reach the engine through Left1Host, whose methods name the image routine they stand
// for. New C++ interfaces, not drop-in binary replacements; nothing here is bound into bsp_game.
// Names are hypotheses, not recovered symbols. Marker: cc13_left1.

#ifndef BSP_CC13_LEFT1_HPP
#define BSP_CC13_LEFT1_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_left1 {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
inline void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }
inline void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

inline constexpr std::uint32_t kGameGlobal = 0x00E188A8u;          // [00E188A8], the game
inline constexpr std::uint32_t kAmbientObserverVtable = 0x00CE75CCu;
inline constexpr std::uint32_t kEmptyText = 0x00E1952Eu;           // the image's "" (a dword arg)
inline constexpr double kAmbientFadeDistance = 2000.0;             // double 00CF0DD8
inline constexpr float kAmbientMinimumVolume = 0.25f;              // float 00CE3868

// The 0Ch clip element passed by value (VoiceClipRef in engine_seg49_64_78_cc13.hpp: the
// element's vtable word, the clip record, the line index). Only the record and the index are
// read here.
struct AmbientClip12 {
    std::uint32_t vtable_word;
    void* record;            // record+4: the clip's text pointer (null: the image prints "")
    std::int32_t index;
};

struct Left1Host {
    virtual ~Left1Host() = default;
    virtual void* game_global() = 0;  // [00E188A8]
    // 006952A0 BSP_Observer_UnregisterPair / 00694A60 BSP_Observer_RegisterPair,
    // __fastcall(ECX = observed entity, EDX = observer).
    virtual void observer_unregister_pair_006952a0(void* target, void* observer) = 0;
    virtual void observer_register_pair_00694a60(void* target, void* observer) = 0;
    // 00702CC0 BSP_VoiceSlot_ConstructAndStart, __thiscall(ECX = fresh 18h slot; clip*, owned
    // resource by value), RET 8; EAX = slot.
    virtual void* voice_slot_construct_and_start_00702cc0(void* slot, const AmbientClip12* clip,
                                                          std::uint32_t owned_resource) = 0;
    // 005B82C0 BSP_VoiceSlot_Assign, __thiscall(ECX = destination; source*), RET 4.
    virtual void voice_slot_assign_005b82c0(void* destination, void* source) = 0;
    // 005B7FC0 BSP_VoiceSlot_Destroy, __fastcall(ECX = slot).
    virtual void voice_slot_destroy_005b7fc0(void* slot) = 0;
    // 007027B0 BSP_VoiceSlot_PollState, __fastcall(ECX = slot); EAX.
    virtual std::int32_t voice_slot_poll_state_007027b0(void* slot) = 0;
    // 00702130 BSP_VoiceSlot_SetVolume, __thiscall(ECX = slot; float), RET 4.
    virtual void voice_slot_set_volume_00702130(void* slot, float volume) = 0;
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix, __fastcall(ECX = transform).
    virtual void transform_refresh_world_matrix_00b6db70(void* transform) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, __fastcall(ECX = entity).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 0042B2F0 BSP_Vector3_LengthFloatThreshold, __thiscall(ECX = float[3]); ST0.
    virtual float vector3_length_threshold_0042b2f0(const float* v) = 0;
    // 004254B0, __cdecl(format, ...): a trivial body (the release build's compiled-out trace).
    // The pushed argument dwords are handed over in stack order.
    virtual void trace_printf_004254b0(const char* format, const std::uint32_t* args,
                                       std::uint32_t count) = 0;
};

// 005B8810 BSP_AmbientMessage_PlaySound, __thiscall(ECX = object; AmbientClip12 by value,
// entity*), RET 10h; EAX = object. SEH frame, handler 00C7215B.
void* ambient_message_voice_construct_005b8810(Left1Host& host, void* object, AmbientClip12 clip,
                                               void* entity);

// 00605630 BSP_Math_DirectionAngleDeltas, __fastcall(ECX = float* yaw, EDX = float* pitch;
// a.xyz, b.xyz by value), RET 18h.
void direction_angle_deltas_00605630(float* yaw, float* pitch, const float a[3], const float b[3]);

}  // namespace bsp::cc13_left1

#endif  // BSP_CC13_LEFT1_HPP
