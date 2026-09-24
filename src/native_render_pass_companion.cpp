#include "bsp/native_render_pass_companion.hpp"
#include "bsp/native_bright_pass_lifetime.hpp"
#include "bsp/native_luminance_owner.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
constexpr std::array<Word, 7> profiles{
    0x00d5e164u, 0x00d5e178u, 0x00d5e18cu, 0x00d5e1a0u,
    0x00d5e1b4u, 0x00d61fe0u, 0x00d62150u};
constexpr std::array<Word, 7> terminals{
    0x00b10120u, 0x00b10140u, 0x00b10160u, 0x00b10180u,
    0x00b101a0u, 0x00b50fe0u, 0x00b54f70u};
std::size_t current_profile(void* owner, NativeRenderPassCompanionContext& c) {
    const Word current = *static_cast<const volatile Word*>(owner);
    for (std::size_t i = 0; i < profiles.size(); ++i) {
        if (current == profiles[i] && c.profiles[i]) return i;
    }
    throw std::invalid_argument("render pass has no admitted current profile");
}
std::size_t current_terminal(void* owner, NativeRenderPassCompanionContext& c) {
    const auto first = current_profile(owner, c);
    if (c.profiles[first][0] != 0x00bd30e0u)
        throw std::invalid_argument("render pass current virtual0 is not BD30E0");
    // BD30E0 reloads the owner's current profile for its deleting-slot call.
    const auto second = current_profile(owner, c);
    if (c.profiles[second][1] != terminals[second])
        throw std::invalid_argument("render pass deleting slot is not recovered");
    return second;
}
std::atomic<std::int32_t>& binding_count(void* owner, NativeRenderPassCompanionContext& c) {
    if (!owner || reinterpret_cast<std::uintptr_t>(owner) % 4 ||
        &c.effects.actual_post_effects != &c.registry)
        throw std::invalid_argument("render pass requires aligned storage and the same registry");
    if (c.registry.find(owner))
        throw std::invalid_argument("render pass already has a canonical companion");
    (void)current_terminal(owner, c);
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(owner) + 4));
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("render pass binding requires a live actual count");
    return count;
}
void delete_current(void* owner, std::size_t index, NativeRenderEffectLifetimeContext& c) {
    switch (index) {
    case 0: (void)delete_native_depth_downscale_pass_00b10120(owner, 1, c); break;
    case 1: (void)delete_native_particle_blend_pass_00b10140(owner, 1, c); break;
    case 2: (void)delete_native_downscale4x4_pass_00b10160(owner, 1, c); break;
    case 3: (void)delete_native_downscale2x2_pass_00b10180(owner, 1, c); break;
    case 4: (void)delete_native_bright_pass_00b101a0(owner, 1, c); break;
    case 5: (void)delete_native_luminance_owner_00b50fe0(owner, 1, c); break;
    case 6: (void)delete_native_bloom_owner_00b54f70(owner, 1, c); break;
    default: std::terminate();
    }
}
} // namespace

NativeRenderPassReference::NativeRenderPassReference(void* owner,
    NativeRenderPassCompanionContext& context)
    : RenderCommandReference(binding_count(owner, context)), identity_(owner), context_(context) {
    context_.registry.bind(identity_, *this);
}
NativeRenderPassReference::~NativeRenderPassReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeRenderPassReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeRenderPassReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        const auto terminal = current_terminal(identity_, context_);
        phase_ = Phase::destroying;
        delete_current(identity_, terminal, context_.effects);
        phase_ = Phase::retired;
        context_.registry.unbind(identity_, *this);
        // No access to the retired native allocation or its counter follows.
    } catch (...) { std::terminate(); }
}
} // namespace bsp
