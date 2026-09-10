#include "bsp/system_fog_world_factory.hpp"
#include <array>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error World fog factory requires MSVC Win32 x87 operation ordering.
#endif

namespace bsp {
namespace {
template<class T>
T* load_pointer_slot(T* const& slot) noexcept {
    const auto* address = &slot;
    T* value;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}

std::uint32_t spill_x87_zero() noexcept {
    std::uint32_t bits;
    __asm {
        fldz
        fstp dword ptr bits
    }
    return bits;
}

// 004DF720..004DF770. Keep the x87 alpha live while building both the raw
// float4 words and the parent's packed-color live-out. No C++ float conversion.
void make_initial_color(std::array<std::uint32_t, 4>& rgba,
    std::uint32_t& actual_packed_color_temporary) noexcept {
    static const std::uint64_t divisor_00ce4b48 = 0x406fe00000000000ULL; // double255
    auto* words = rgba.data();
    auto* packed = &actual_packed_color_temporary;
    __asm {
        mov eax, words
        mov edx, packed
        fldz
        fdiv qword ptr divisor_00ce4b48
        mov dword ptr [eax], 03f25a5a6h // raw00CE7D60
        mov dword ptr [eax+4], 03f23a3a4h // raw00CE7D5C
        mov ecx, 03f2babach // raw00CE7D58, loaded before packed byte stores
        mov byte ptr [edx+2], 0a5h
        mov byte ptr [edx+1], 0a3h
        mov byte ptr [edx], 0abh
        mov byte ptr [edx+3], 0
        mov dword ptr [eax+8], ecx
        fstp dword ptr [eax+12]
    }
}

// Internal continuation after the actual allocator/initializer. The creator
// owns exactly one reference on entry. No test service or alternate allocator.
void publish_and_initialize_world_fog(SystemFogOwner& creator,
    const WorldFogFactoryGameFields& actual_game,
    std::uint32_t& actual_packed_color_temporary_18) noexcept {
    auto* receiver = load_pointer_slot(actual_game.receiver_19e8); //004DF6D0
    if (receiver)
        set_system_fog_world_owner_00bbdf20(receiver->fog_10, &creator);

    auto* camera = load_pointer_slot(actual_game.camera_19fc); //004DF6E7
    set_system_fog_camera_owner_00b71940(camera->fog_184, &creator);
    release_system_fog_owner(creator); //004DF6F7: before every following field write

    auto bits = spill_x87_zero(); //004DF706
    set_system_fog_scalar_68_00b84d00(creator, bits);
    bits = spill_x87_zero(); //004DF713
    set_system_fog_scalar_78_00b84d40(creator, bits);
    std::array<std::uint32_t, 4> color;
    make_initial_color(color, actual_packed_color_temporary_18);
    set_system_fog_color_00b84c40(creator, color.data()); //004DF771

    if (load_pointer_slot(actual_game.scene_record_05fc)) { //004DF776 guard only
        for (std::uint32_t index = 0; index != 4; ++index) {
            const auto* record = static_cast<const unsigned char*>(
                load_pointer_slot(actual_game.scene_record_05fc)); //004DF786 each turn
            set_system_fog_directional_color_00b84fa0(creator,
                record + 0xa20 + index * 0x10, index);
        }
    }
}
} // namespace

void initialize_world_fog_004df6a3(const WorldFogFactoryGameFields& actual_game,
    std::uint32_t& actual_packed_color_temporary_18) {
    // Native allocation state24 covers raw construction only and is restored
    // before publication. The existing initializer is noexcept. The allocator's
    // actual host new-handler/bad_alloc behavior propagates before any slot load.
    auto* creator = allocate_system_fog_owner();
    publish_and_initialize_world_fog(*creator, actual_game, actual_packed_color_temporary_18);
}

} // namespace bsp
