#include "bsp/native_gui_texture_normalization.hpp"

#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_texture_saved_dimensions.hpp"

#include <cstddef>
#include <limits>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI texture normalization requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
using Phase = NativeGuiTextureNormalizationPhase;
struct Region { const void* address; std::size_t bytes; };

std::uintptr_t begin(const Region& region) {
    return reinterpret_cast<std::uintptr_t>(region.address);
}
void check_regions(const Region* regions, std::size_t count,
    std::size_t allowed_a, std::size_t allowed_b) {
    for (std::size_t i=0; i<count; ++i) {
        const auto first=begin(regions[i]);
        if (!first || !regions[i].bytes ||
            regions[i].bytes>std::numeric_limits<std::uintptr_t>::max()-first)
            throw std::invalid_argument("texture normalization region is invalid");
        for (std::size_t j=0; j<i; ++j) {
            if (j==allowed_a && i==allowed_b) continue;
            if (first<begin(regions[j])+regions[j].bytes &&
                begin(regions[j])<first+regions[i].bytes)
                throw std::invalid_argument("texture normalization regions alias");
        }
    }
}
U current_word(const void* address, std::uintptr_t offset=0) {
    return *reinterpret_cast<const volatile U*>(
        reinterpret_cast<std::uintptr_t>(address)+offset);
}

// Original's live ST0 width must span the CURRENT height selector capture,
// but never a throwing C++ admission. Scratch+4 is the signed-FILD integer
// cell; scratch+0 receives width only AFTER the actual current +4C capture.
__declspec(noinline) void width_and_height_selector(U value,
    const void* actual_texture, const float* uint_bias,
    const double* width_divisor, volatile U* scratch,
    volatile U* captured_profile, volatile U* captured_target) noexcept {
    __asm {
        mov eax, value
        mov ecx, scratch
        test eax, eax
        mov dword ptr [ecx+4], eax
        fild dword ptr [ecx+4]
        jns width_nonnegative
        mov edx, uint_bias
        fadd dword ptr [edx]
    width_nonnegative:
        mov edx, width_divisor
        fdiv qword ptr [edx]
        mov eax, actual_texture
        mov eax, dword ptr [eax]
        mov edx, captured_profile
        mov dword ptr [edx], eax
        mov eax, dword ptr [eax+4ch]
        mov edx, captured_target
        mov dword ptr [edx], eax
        fstp dword ptr [ecx]
    }
}

// scratch+4 is reused for height integer then rounded height, matching the
// Original eight-byte scratch. The only output accesses are the final two
// sequential x87 stores; there are no later actual-texture accesses here.
__declspec(noinline) void height_and_output(U value, const float* uint_bias,
    const double* height_divisor, volatile U* scratch, void* output) noexcept {
    __asm {
        mov eax, value
        mov ecx, scratch
        test eax, eax
        mov dword ptr [ecx+4], eax
        fild dword ptr [ecx+4]
        jns height_nonnegative
        mov edx, uint_bias
        fadd dword ptr [edx]
    height_nonnegative:
        mov edx, height_divisor
        fdiv qword ptr [edx]
        mov eax, output
        fstp dword ptr [ecx+4]
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx+4]
        fstp dword ptr [eax+4]
    }
}
} // namespace

struct NativeGuiTextureNormalizationAccess final {
    static void preflight(void* output, const void* texture,
        const NativeGuiTextureNormalizationProfiles& profiles,
        const NativeGuiTextureNormalizationFrame& frame) {
        const Region regions[]={{&frame,sizeof(frame)},
            {&profiles,sizeof(profiles)},{profiles.data_,sizeof(*profiles.data_)},
            {const_cast<const U*>(profiles.texture_2d_),0x50},
            {profiles.uint_bias_,4},{profiles.width_divisor_,8},
            {profiles.height_divisor_,8},{texture,0x50},{output,8}};
        check_regions(regions,sizeof(regions)/sizeof(regions[0]),7,8);
        if ((reinterpret_cast<std::uintptr_t>(texture)&3u) ||
            (reinterpret_cast<std::uintptr_t>(output)&3u))
            throw std::invalid_argument("texture normalization storage is unaligned");
        if (frame.diagnostics_.phase!=Phase::fresh)
            throw std::logic_error("texture normalization frame was already entered");
    }
    static void admit(U profile, U target,
        const NativeGuiTextureNormalizationProfiles& profiles, U expected) {
        if (profile!=reinterpret_cast<std::uintptr_t>(profiles.texture_2d_) ||
            target!=expected)
            throw std::logic_error("current texture dimension route is unsupported");
    }
    static void* run(void* output, const void* texture,
        const NativeGuiTextureNormalizationProfiles& profiles,
        NativeGuiTextureNormalizationFrame& frame) {
        preflight(output,texture,profiles,frame);
        auto& d=frame.diagnostics_;
        d.output=output;
        d.texture=texture;
        d.phase=Phase::entered;
        try {
            const U width_profile=current_word(texture);
            const U width_target=current_word(
                reinterpret_cast<const void*>(width_profile),0x48);
            d.width_profile=width_profile;
            d.width_target=width_target;
            d.phase=Phase::width_selected;
            admit(width_profile,width_target,profiles,0x00b3ce70);
            const U width=get_native_texture_saved_width_00b3ce70(texture);
            d.width_value=width;
            d.phase=Phase::width_queried;
            width_and_height_selector(width,texture,profiles.uint_bias_,
                profiles.width_divisor_,frame.scratch_,
                &frame.captured_height_profile_,&frame.captured_height_target_);
            const U height_profile=frame.captured_height_profile_;
            const U height_target=frame.captured_height_target_;
            d.height_profile=height_profile;
            d.height_target=height_target;
            d.phase=Phase::height_selected;
            admit(height_profile,height_target,profiles,0x00b3ce80);
            const U height=get_native_texture_saved_height_00b3ce80(texture);
            d.height_value=height;
            d.phase=Phase::height_queried;
            height_and_output(height,profiles.uint_bias_,profiles.height_divisor_,
                frame.scratch_,output);
            d.phase=Phase::completed;
            return output;
        } catch (...) {
            d.failed_from=d.phase;
            d.phase=Phase::failed;
            throw;
        }
    }
};

NativeGuiTextureNormalizationProfiles::NativeGuiTextureNormalizationProfiles(
    const game::GameNativeReadOnlyData& data) {
    const auto profile=static_cast<const U*>(data.data_at(0x00d61948,0x50));
    const auto bias=static_cast<const float*>(data.data_at(0x00ce3978,4));
    const auto width=static_cast<const double*>(data.data_at(0x00cec380,8));
    const auto height=static_cast<const double*>(data.data_at(0x00cef1b8,8));
    const Region regions[]={{this,sizeof(*this)},{&data,sizeof(data)},
        {profile,0x50},{bias,4},{width,8},{height,8}};
    check_regions(regions,sizeof(regions)/sizeof(regions[0]),6,6);
    data_=&data;
    texture_2d_=profile;
    uint_bias_=bias;
    width_divisor_=width;
    height_divisor_=height;
}

void* normalize_gui_texture_size_00aceb70(void* output, const void* actual_texture,
    const NativeGuiTextureNormalizationProfiles& profiles,
    NativeGuiTextureNormalizationFrame& frame) {
    return NativeGuiTextureNormalizationAccess::run(
        output,actual_texture,profiles,frame);
}
} // namespace bsp
