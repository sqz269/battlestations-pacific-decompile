#pragma once
#include "bsp/native_gui_widget_copy_storage.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include <cstdint>

namespace bsp {
// AC6FA0: native ECX destination, stack source, EAX same destination, RET4.
// Complete 25-byte adapter to raw AA9520, then stamp D5CB80. Inherits that
// provider's supported actual Model26/3E, parent0 domain and failure contract.
// No allocation, source-type check, registration or derived field is added.
void* construct_native_gui_group_copy_00ac6fa0(void* actual_destination,
    const void* actual_source, NativeGuiWidgetCopyContext&,
    NativeGuiWidgetCopyAcquired&);

struct NativeGuiGroupDefaultInputs {
    const volatile std::uint32_t& actual_one_00d7a24c;
};
struct NativeGuiGroupCopyInputs {
    // Borrow the first byte at the SAME actual raw source address. Binding
    // this reference reads no byte and adds no class/type/provenance check.
    const unsigned char& actual_source_start;
    NativeGuiWidgetCopyContext& context;
    NativeGuiWidgetCopyAcquired& acquired;
};

// Closed borrowed alternatives: default has no source/copy requirements;
// copy has no one-word requirement. A real source reference cannot encode
// the null/default branch. Private immutable metadata keeps the source and
// its matching references together across allocation/constructor callbacks.
// Referents retain the existing raw providers' domain/lifetime contracts.
class NativeGuiGroupFactoryInputs final {
public:
    NativeGuiGroupFactoryInputs(NativeGuiGroupDefaultInputs inputs) noexcept
        : captured_source_(nullptr), default_(inputs) {}
    NativeGuiGroupFactoryInputs(NativeGuiGroupCopyInputs inputs) noexcept
        : captured_source_(&inputs.actual_source_start), copy_(inputs) {}
private:
    const void* const captured_source_;
    union {
        const NativeGuiGroupDefaultInputs default_;
        const NativeGuiGroupCopyInputs copy_;
    };
    friend void* create_native_gui_group_00aa12f0(NativeGuiGroupFactoryInputs);
};

// AA12F0: native ECX optional source, EAX actual slot or null, RET. Requires
// completed explicit Group-process startup. Uses its SAME F8BFA0 EC/F0 pool;
// a null source selects genuine AC6F50, otherwise genuine AC6FA0. A constructor
// exception returns only the failed slot (CB6DE0/CB6DE8 -> AC73D0), after the
// constructor's own cleanup. No successful-payload destructor or admission.
//
// The caller retains acquired diagnostics, including completed model clones
// even when a later AA9520 failure makes this wrapper return the widget slot.
// Never inspect the failed widget slot using that metadata. Default/null
// allocation leaves copy diagnostics untouched. A successful raw result has
// its native creator reference; the caller owns publication and final deletion.
// Host C++ exception projection only, not native FH3/SEH or binary ABI parity.
// Only binding metadata is captured before allocation; borrowed one/source/
// context/frame contents are consumed at their existing constructor epochs.
void* create_native_gui_group_00aa12f0(NativeGuiGroupFactoryInputs);

// Compatible entry: delegates its actual references to the matching closed
// alternative and the SAME complete allocation/constructor/unwind body.
void* create_native_gui_group_00aa12f0(const void* optional_actual_source,
    const volatile std::uint32_t& actual_one_00d7a24c,
    NativeGuiWidgetCopyContext&, NativeGuiWidgetCopyAcquired&);

// AC73E0: native ECX owner, stack flags DWORD, EAX saved owner, RET4. The
// supplied actual caller flags slot must remain live and may alias raw state
// or be changed by destructor callbacks. Read ONLY its low byte, AFTER raw
// AA9730 returns; bit0 returns the same actual slot via F8BFA0/AC7260.
// Does not decrement/test +04 or manage a host companion. Explicit deletion
// may occur at a nonzero count. No payload access follows the pool return.
void* delete_native_gui_group_00ac73e0(void* actual_group,
    const volatile std::uint32_t& actual_flags_slot,
    NativeGuiWidgetLifetimeContext&);
} // namespace bsp
