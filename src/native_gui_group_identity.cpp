#include "bsp/native_gui_group_identity.hpp"
#include "bsp/native_gui_group_factory.hpp"
#include "bsp/native_ref_counted.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word current_profile(void* identity) noexcept {
    return *static_cast<const volatile Word*>(identity);
}
void require_slot(Word profile, NativeGuiGroupIdentityContext& context,
    std::size_t index, Word expected) {
    if (profile != 0x00d5cb80u || !context.group_profile_00d5cb80 ||
        context.group_profile_00d5cb80[index] != expected)
        throw std::logic_error("Group identity has no binding for the current native slot");
}
std::atomic<std::int32_t>& binding_count(void* identity, NativeGuiGroupIdentityContext& context) {
    if (!identity || reinterpret_cast<std::uintptr_t>(identity) % 4 ||
        context.registry.find(identity))
        throw std::invalid_argument("Group identity requires aligned unregistered actual storage");
    require_slot(current_profile(identity), context, 0, 0x00bd30e0u);
    require_slot(current_profile(identity), context, 1, 0x00ac73e0u);
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("Group identity requires a completed live actual count");
    return count;
}
class GroupDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    GroupDeleteCalls(void* identity, NativeGuiGroupReference& reference,
        NativeGuiGroupIdentityContext& context)
        : identity_(identity), reference_(reference), context_(context) {}
    void delete_vslot04(void* identity, Word captured_profile, Word flags) override {
        if (identity != identity_ || flags != 1)
            throw std::logic_error("Group terminal requires its same actual owner and flags1");
        // BD30E0 has freshly captured this profile after its caller's slot0.
        require_slot(captured_profile, context_, 1, 0x00ac73e0u);
        const volatile Word actual_flags = flags;
        reference_.delete_scalar_00ac73e0(actual_flags);
    }
private:
    void* const identity_;
    NativeGuiGroupReference& reference_;
    NativeGuiGroupIdentityContext& context_;
};
} // namespace

bool native_gui_group_is_kind_of_00ac6f70(Word descriptor,
    const volatile Word (&lineage)[3]) noexcept {
    for (const volatile Word* current = lineage; current != lineage + 3; ++current)
        if (*current == descriptor) return true;
    return false;
}

NativeGuiGroupReference::NativeGuiGroupReference(void* identity,
    NativeGuiGroupIdentityContext& context)
    : RenderCommandReference(binding_count(identity, context)), identity_(identity), context_(context) {
    context_.registry.bind(identity_, *this);
}
NativeGuiGroupReference::~NativeGuiGroupReference() {
    if (phase_ != Phase::retired) std::terminate();
    // The borrowed base reference destructor never reads its atomic.
}
bool NativeGuiGroupReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeGuiGroupReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.registry.unbind(identity_, *this); // exact host-map erase; no raw reads
}
void* NativeGuiGroupReference::delete_scalar_00ac73e0(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("Group scalar deletion requires its live canonical companion");
    phase_ = Phase::destroying;
    void* result;
    try {
        result = delete_native_gui_group_00ac73e0(identity_, flags, context_.lifetime);
    } catch (...) {
        retire();
        throw;
    }
    retire();
    return result; // no native payload/count access after possible pool return
}
void NativeGuiGroupReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(current_profile(identity_), context_, 0, 0x00bd30e0u);
        GroupDeleteCalls calls(identity_, *this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
