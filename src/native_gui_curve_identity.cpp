#include "bsp/native_gui_curve_identity.hpp"
#include "bsp/native_ref_counted.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word profile(void* raw) noexcept { return *static_cast<const volatile Word*>(raw); }
void require_slot(Word captured, NativeGuiCurveIdentityContext& c, std::size_t index, Word expected) {
    if (captured != 0x00d5c7f8 || !c.profile_00d5c7f8 || c.profile_00d5c7f8[index] != expected)
        throw std::logic_error("Curve identity requires its genuine current native slot binding");
}
std::atomic<std::int32_t>& count(void* raw, NativeGuiCurveIdentityContext& c) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % 4 || c.curve.actual_owners.find(raw))
        throw std::invalid_argument("Curve requires aligned live unregistered actual storage");
    require_slot(profile(raw), c, 0, 0x00bd30e0);
    require_slot(profile(raw), c, 1, 0x00abe010);
    auto& references = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(raw) + 4));
    if (references.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("Curve identity requires its constructed actual positive count");
    return references;
}
class DeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    DeleteCalls(NativeGuiCurveReference& owner, NativeGuiCurveIdentityContext& context)
        : owner_(owner), context_(context) {}
    void delete_vslot04(void* actual, Word captured, Word flags) override {
        if (actual != owner_.actual_identity() || flags != 1)
            throw std::logic_error("Curve zero terminal requires same actual identity and flags1");
        require_slot(captured, context_, 1, 0x00abe010);
        const volatile Word actual_flags = flags;
        owner_.delete_scalar_00abe010(actual_flags);
    }
private:
    NativeGuiCurveReference& owner_;
    NativeGuiCurveIdentityContext& context_;
};
} // namespace
NativeGuiCurveReference::NativeGuiCurveReference(void* raw, NativeGuiCurveIdentityContext& c)
    : RenderCommandReference(count(raw, c)), identity_(raw), context_(c) {
    context_.curve.actual_owners.bind(identity_, *this);
}
NativeGuiCurveReference::~NativeGuiCurveReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeGuiCurveReference::retired() const noexcept { return phase_ == Phase::retired; }
bool NativeGuiCurveReference::failed() const noexcept { return phase_ == Phase::failed; }
void* NativeGuiCurveReference::delete_scalar_00abe010(const volatile Word& flags) {
    if (phase_ != Phase::bound) throw std::logic_error("Curve terminal cannot replay entered or failed work");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_gui_curve_00abe010(identity_, flags, context_.curve, terminal_); }
    catch (...) { phase_ = Phase::failed; throw; }
    phase_ = Phase::retired;
    context_.curve.actual_owners.unbind(identity_, *this); // opaque key, no post-pool raw read
    return result;
}
void NativeGuiCurveReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(profile(identity_), context_, 0, 0x00bd30e0);
        DeleteCalls calls(*this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
