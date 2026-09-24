#include "bsp/native_ambient_lifetime.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/system_lighting_owners.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw ambient lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(SystemAmbientBacklinks) == 12);
static_assert(offsetof(SystemAmbientBacklinks, count_04) == 4);
static_assert(offsetof(SystemAmbientBacklinks, capacity_08) == 8);
Word profile(void* owner) noexcept { return *static_cast<const volatile Word*>(owner); }
class BaseCleanup final {
public:
    explicit BaseCleanup(void* owner) noexcept : owner_(owner) {}
    ~BaseCleanup() { if (armed) destroy_native_gui_ref_base_00aa6e10(owner_); }
    bool armed{true};
private:
    void* owner_;
};
void require_slot(Word current, NativeAmbientIdentityContext& context,
    std::size_t index, Word target) {
    if (current != 0x00d62f3cu || !context.ambient_profile_00d62f3c ||
        context.ambient_profile_00d62f3c[index] != target)
        throw std::logic_error("ambient identity has no binding for the current native slot");
}
std::atomic<std::int32_t>& bind_count(void* identity, NativeAmbientIdentityContext& context) {
    if (!identity || reinterpret_cast<std::uintptr_t>(identity) % 4 || context.registry.find(identity))
        throw std::invalid_argument("ambient identity requires aligned unregistered actual storage");
    require_slot(profile(identity), context, 0, 0x00bd30e0u);
    require_slot(profile(identity), context, 1, 0x00b7c7e0u);
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("ambient identity requires a completed live actual count");
    return count;
}
class AmbientDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    AmbientDeleteCalls(void* identity, NativeAmbientReference& reference,
        NativeAmbientIdentityContext& context)
        : identity_(identity), reference_(reference), context_(context) {}
    void delete_vslot04(void* identity, Word captured_profile, Word flags) override {
        if (identity != identity_ || flags != 1)
            throw std::logic_error("ambient terminal requires the same owner and flags1");
        require_slot(captured_profile, context_, 1, 0x00b7c7e0u);
        const volatile Word actual_flags = flags;
        reference_.delete_scalar_00b7c7e0(actual_flags);
    }
private:
    void* const identity_;
    NativeAmbientReference& reference_;
    NativeAmbientIdentityContext& context_;
};
}

void destroy_native_ambient_00b7c450(void* owner) {
    auto* const array = reinterpret_cast<SystemAmbientBacklinks*>(static_cast<std::byte*>(owner) + 8);
    BaseCleanup cleanup(owner);
    // This exact12B view's count0 path never accesses a SceneResource object.
    resize_system_ambient_backlinks_00b7bc70(*array, 0); // B7C47D
    void* const current_begin = *reinterpret_cast<void* const volatile*>(array);
    singleton_lifetime_free(current_begin); // B7C485, BF6989 returns
    cleanup.armed = false;
    *static_cast<volatile Word*>(owner) = 0x00d5c104u; // B7C497, post-free
    destroy_native_ref_counted_base_00bd30f0(owner); // B7C49D
}

void* delete_native_ambient_00b7c7e0(void* owner, const volatile Word& flags) {
    destroy_native_ambient_00b7c450(owner); // B7C7E3
    if ((*reinterpret_cast<const volatile std::uint8_t*>(&flags) & 1u) != 0)
        singleton_lifetime_free(owner); // B7C7F0
    return owner;
}

NativeAmbientReference::NativeAmbientReference(void* identity, NativeAmbientIdentityContext& context)
    : RenderCommandReference(bind_count(identity, context)), identity_(identity), context_(context) {
    context_.registry.bind(identity_, *this);
}
NativeAmbientReference::~NativeAmbientReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeAmbientReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeAmbientReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.registry.unbind(identity_, *this); // metadata only, no dead storage read
}
void* NativeAmbientReference::delete_scalar_00b7c7e0(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("ambient scalar deletion requires its live canonical companion");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_ambient_00b7c7e0(identity_, flags); }
    catch (...) { retire(); throw; }
    retire();
    return result;
}
void NativeAmbientReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(profile(identity_), context_, 0, 0x00bd30e0u);
        AmbientDeleteCalls calls(identity_, *this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}

} // namespace bsp
