#include "bsp/native_gui_grid_identity.hpp"
#include "bsp/native_ref_counted.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word profile(void* raw) noexcept { return *static_cast<const volatile Word*>(raw); }
void require_slot(Word captured, NativeGuiGridIdentityContext& c, std::size_t index, Word expected) {
    if (captured != 0x00d5ce60 || !c.profile_00d5ce60 || c.profile_00d5ce60[index] != expected)
        throw std::logic_error("Grid identity requires its genuine current native slot binding");
}
std::atomic<std::int32_t>& count(void* raw, NativeGuiGridIdentityContext& c) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % 4 || c.grid.actual_owners.find(raw))
        throw std::invalid_argument("Grid requires aligned live unregistered actual storage");
    require_slot(profile(raw), c, 0, 0x00bd30e0);
    require_slot(profile(raw), c, 1, 0x00acbc90);
    auto& references = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(raw) + 4));
    if (references.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("Grid identity requires its constructed actual positive count");
    return references;
}
class DeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    DeleteCalls(NativeGuiGridReference& owner, NativeGuiGridIdentityContext& context)
        : owner_(owner), context_(context) {}
    void delete_vslot04(void* actual, Word captured, Word flags) override {
        if (actual != owner_.actual_identity() || flags != 1)
            throw std::logic_error("Grid zero terminal requires same actual identity and flags1");
        require_slot(captured, context_, 1, 0x00acbc90);
        const volatile Word actual_flags = flags;
        owner_.delete_scalar_00acbc90(actual_flags);
    }
private:
    NativeGuiGridReference& owner_;
    NativeGuiGridIdentityContext& context_;
};
} // namespace
NativeGuiGridReference::NativeGuiGridReference(void* raw, NativeGuiGridIdentityContext& c)
    : RenderCommandReference(count(raw, c)), identity_(raw), context_(c) {
    context_.grid.actual_owners.bind(identity_, *this);
}
NativeGuiGridReference::~NativeGuiGridReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeGuiGridReference::retired() const noexcept { return phase_ == Phase::retired; }
bool NativeGuiGridReference::failed() const noexcept { return phase_ == Phase::failed; }
void* NativeGuiGridReference::delete_scalar_00acbc90(const volatile Word& flags) {
    if (phase_ != Phase::bound) throw std::logic_error("Grid terminal cannot replay entered/failed work");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_gui_grid_00acbc90(identity_, flags, context_.grid, terminal_); }
    catch (...) { phase_ = Phase::failed; throw; } // keep canonical key/graph bound
    phase_ = Phase::retired;
    context_.grid.actual_owners.unbind(identity_, *this); // opaque key only, no post-pool raw read
    return result;
}
void NativeGuiGridReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(profile(identity_), context_, 0, 0x00bd30e0);
        DeleteCalls calls(*this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
