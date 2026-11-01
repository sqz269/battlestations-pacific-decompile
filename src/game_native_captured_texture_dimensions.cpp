#include "bsp/game_native_captured_texture_dimensions.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_system_constant_gather.hpp"
#include "bsp/native_texture_saved_dimensions.hpp"
#include "bsp/render_command_queue.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Captured texture dimensions require MSVC Win32.
#endif

namespace bsp::game {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4 && sizeof(std::int32_t) == 4);
static_assert(sizeof(std::atomic<std::int32_t>) == 4 && alignof(std::atomic<std::int32_t>) == 4);
void require_dimension(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
std::atomic<std::int32_t>& actual_count(void* raw) {
    require_dimension(raw && !(reinterpret_cast<std::uintptr_t>(raw) & 3u),
        "Captured texture dimensions require aligned actual storage");
    return *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(raw) + 4));
}
}

GameNativeCapturedTextureDimensions::GameNativeCapturedTextureDimensions(
    NativeRenderActualOwnerRegistry& owners) : owners_(owners) {
    callbacks_.dimension_from_captured =
        [this](void* actual, std::uint32_t profile, std::uint32_t target) {
            return dimension(actual, profile, target);
        };
}

const GuiTextureCallbacks& GameNativeCapturedTextureDimensions::captured_callbacks() const noexcept {
    return callbacks_;
}

std::uint32_t GameNativeCapturedTextureDimensions::dimension(void* actual,
    std::uint32_t captured_profile, std::uint32_t captured_target) const {
    require_dimension(actual && captured_profile == 0x00d61948,
        "Captured texture dimensions require the supported D61948 Source family");
    auto& reference = owners_.resolve_actual(actual);
    require_dimension(&reference.reference_count == &actual_count(actual) &&
        reference.reference_count.load() > 0,
        "Captured texture dimensions require the SAME canonical actual count");
    if (captured_target == 0x00b3ce50)
        return get_raw_texture_width_00b3ce50(actual);
    if (captured_target == 0x00b3ce60)
        return get_raw_texture_height_00b3ce60(actual);
    if (captured_target == 0x00b3ce70)
        return get_native_texture_saved_width_00b3ce70(actual);
    if (captured_target == 0x00b3ce80)
        return get_native_texture_saved_height_00b3ce80(actual);
    throw std::logic_error("Unsupported captured Source texture dimension target");
}
} // namespace bsp::game
