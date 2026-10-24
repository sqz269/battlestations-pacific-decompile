#include "bsp/native_gui_button_identity.hpp"
#include "bsp/native_ref_counted.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word profile(void* raw) noexcept { return *static_cast<const volatile Word*>(raw); }
void require_slot(Word captured, NativeGuiButtonIdentityContext& context,
    std::size_t index, Word expected) {
    if (captured != 0x00d5bed0 || !context.button_profile_00d5bed0 ||
        context.button_profile_00d5bed0[index] != expected)
        throw std::logic_error("Button identity has no genuine current native slot binding");
}
std::atomic<std::int32_t>& count(void* raw, NativeGuiButtonIdentityContext& context) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % 4 || context.registry.find(raw))
        throw std::invalid_argument("Button identity requires aligned live unregistered actual storage");
    require_slot(profile(raw), context, 0, 0x00bd30e0);
    require_slot(profile(raw), context, 1, 0x00aa3d10);
    auto& references = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(raw) + 4));
    if (references.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("Button identity requires its actual constructed positive count");
    return references;
}
class DeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    DeleteCalls(NativeGuiButtonReference& owner, NativeGuiButtonIdentityContext& context)
        : owner_(owner), context_(context) {}
    void delete_vslot04(void* actual, Word captured, Word flags) override {
        if (actual != owner_.actual_identity() || flags != 1)
            throw std::logic_error("Button zero terminal requires its same actual owner and flags1");
        require_slot(captured, context_, 1, 0x00aa3d10);
        const volatile Word caller_flags = flags;
        owner_.delete_scalar_00aa3d10(caller_flags);
    }
private:
    NativeGuiButtonReference& owner_;
    NativeGuiButtonIdentityContext& context_;
};
} // namespace
NativeGuiButtonReference::NativeGuiButtonReference(void* raw, NativeGuiButtonIdentityContext& context)
    : RenderCommandReference(count(raw, context)), identity_(raw), context_(context) {
    context_.registry.bind(identity_, *this);
}
NativeGuiButtonReference::~NativeGuiButtonReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeGuiButtonReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeGuiButtonReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.registry.unbind(identity_, *this); // opaque key only, no post-pool payload read
}
void* NativeGuiButtonReference::delete_scalar_00aa3d10(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("Button scalar deletion cannot replay entered or retired work");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_gui_button_00aa3d10(identity_, flags, context_.lifetime); }
    catch (...) { retire(); throw; }
    retire();
    return result;
}
void NativeGuiButtonReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(profile(identity_), context_, 0, 0x00bd30e0);
        DeleteCalls calls(*this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
