#include "bsp/native_land_follow_parameters.hpp"
#include "bsp/game_tuning_singleton.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
// Complete original instruction sequence, 009BE150..009BE28B exclusive.
// EDX is deliberately unused on entry; native byte copies clobber it.
__declspec(naked) void* __fastcall copy_native_land_follow_parameters_009be150(
    void*, const void*, const void*) noexcept {
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [esp + 4]
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 04h]
        fstp dword ptr [eax + 04h]
        fld dword ptr [ecx + 08h]
        fstp dword ptr [eax + 08h]
        fld dword ptr [ecx + 0ch]
        fstp dword ptr [eax + 0ch]
        fld dword ptr [ecx + 10h]
        fstp dword ptr [eax + 10h]
        fld dword ptr [ecx + 14h]
        fstp dword ptr [eax + 14h]
        fld dword ptr [ecx + 18h]
        fstp dword ptr [eax + 18h]
        fld dword ptr [ecx + 1ch]
        fstp dword ptr [eax + 1ch]
        fld dword ptr [ecx + 20h]
        fstp dword ptr [eax + 20h]
        fld dword ptr [ecx + 24h]
        fstp dword ptr [eax + 24h]
        fld dword ptr [ecx + 28h]
        fstp dword ptr [eax + 28h]
        fld dword ptr [ecx + 2ch]
        fstp dword ptr [eax + 2ch]
        fld dword ptr [ecx + 30h]
        fstp dword ptr [eax + 30h]
        fld dword ptr [ecx + 34h]
        fstp dword ptr [eax + 34h]
        fld dword ptr [ecx + 38h]
        fstp dword ptr [eax + 38h]
        fld dword ptr [ecx + 3ch]
        fstp dword ptr [eax + 3ch]
        fld dword ptr [ecx + 40h]
        fstp dword ptr [eax + 40h]
        fld dword ptr [ecx + 44h]
        fstp dword ptr [eax + 44h]
        fld dword ptr [ecx + 48h]
        fstp dword ptr [eax + 48h]
        fld dword ptr [ecx + 4ch]
        fstp dword ptr [eax + 4ch]
        fld dword ptr [ecx + 50h]
        fstp dword ptr [eax + 50h]
        fld dword ptr [ecx + 54h]
        fstp dword ptr [eax + 54h]
        fld dword ptr [ecx + 58h]
        fstp dword ptr [eax + 58h]
        fld dword ptr [ecx + 5ch]
        fstp dword ptr [eax + 5ch]
        fld dword ptr [ecx + 60h]
        fstp dword ptr [eax + 60h]
        fld dword ptr [ecx + 64h]
        fstp dword ptr [eax + 64h]
        movzx edx, byte ptr [ecx + 68h]
        mov byte ptr [eax + 68h], dl
        movzx edx, byte ptr [ecx + 69h]
        mov byte ptr [eax + 69h], dl
        fld dword ptr [ecx + 6ch]
        fstp dword ptr [eax + 6ch]
        fld dword ptr [ecx + 70h]
        fstp dword ptr [eax + 70h]
        fld dword ptr [ecx + 74h]
        fstp dword ptr [eax + 74h]
        fld dword ptr [ecx + 78h]
        fstp dword ptr [eax + 78h]
        fld dword ptr [ecx + 7ch]
        fstp dword ptr [eax + 7ch]
        fld dword ptr [ecx + 80h]
        fstp dword ptr [eax + 80h]
        fld dword ptr [ecx + 84h]
        fstp dword ptr [eax + 84h]
        fld dword ptr [ecx + 88h]
        fstp dword ptr [eax + 88h]
        fld dword ptr [ecx + 8ch]
        fstp dword ptr [eax + 8ch]
        fld dword ptr [ecx + 90h]
        fstp dword ptr [eax + 90h]
        fld dword ptr [ecx + 94h]
        fstp dword ptr [eax + 94h]
        fld dword ptr [ecx + 98h]
        fstp dword ptr [eax + 98h]
        fld dword ptr [ecx + 9ch]
        fstp dword ptr [eax + 9ch]
        fld dword ptr [ecx + 0a0h]
        fstp dword ptr [eax + 0a0h]
        ret 4
    }
}

bool FollowParameterSnapshot::observe_follow_think(
    const GameTuningBlock& tuning, std::uint64_t think) noexcept {
    const bool entered = !initialized_ ||
        (think != last_think_ && think != last_think_ + 1);
    if (entered) {
        static_assert(offsetof(GameTuningBlock, pilot_follow_followed_point_dist) == 0x380);
        static_assert(offsetof(GameTuningBlock, pilot_follow_safe_alt) == 0x388);
        static_assert(offsetof(GameTuningBlock, pilot_follow_pwr_spd_meter_per_sec) == 0x420);
        // bsp.py show 009BED80 --asm: 009BED89 loads the EXISTING state+6Ch
        // payload; 009BED8C/92 copy the actual tuning singleton+380h into it.
        // Keep all A4h bytes alive in this owner, not a temporary copy just to
        // invoke the kernel. Padding 6Ah/6Bh is intentionally never consumed.
        copy_native_land_follow_parameters_009be150(parameters_.data(), nullptr,
            reinterpret_cast<const std::byte*>(&tuning) + 0x380);
        initialized_ = true;
    }
    last_think_ = think;
    return entered;
}

float FollowParameterSnapshot::safe_alt() const {
    if (!initialized_) throw std::logic_error("Follow parameter payload has not been entered");
    float value;
    // 009BEDB0/BB latches state+88h from the copied payload+8h. The kernel
    // already performed FLD/FSTP on this lane; retain that value across thinks.
    std::memcpy(&value, parameters_.data() + 8, sizeof value);
    return value;
}

} // namespace bsp
