#include "bsp/native_gui_icon_property_state_invocations.hpp"
#include "bsp/native_gui_icon_identity.hpp"
#include "bsp/native_gui_icon_state_construction_storage.hpp"
#include "bsp/native_gui_texture_resolver_storage.hpp"
#include "bsp/native_gui_icon_placeholder_storage.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/game_native_gui_text_state.hpp"
#include <stdexcept>

namespace bsp {
class NativeGuiIconPropertyStateInvocations::Invocation final {
    friend class NativeGuiIconPropertyStateInvocations;
    Invocation(NativeGuiIconPropertyStateInvocations& owner,
        const NativeGuiIconPropertyStateServices& services)
        : owner_(owner),
          growth_{services.icon, services.feature_word_0109eea4},
          resolver_{services.atlas, services.cache, resolver_acquired_,
              services.dimensions, services.constants,
              services.widget_state.widget_empty_00f8bc60()},
          placeholder_{services.icon, services.cache, placeholder_acquired_,
              old_release_, services.feature_word_0109eea4,
              services.transparent_literal_00d5c058},
          context_{services.gui, resolver_, placeholder_, growth_,
              services.actual_one_00d7a24c} {}
    ~Invocation() = default;
    Invocation(const Invocation&) = delete;
    Invocation& operator=(const Invocation&) = delete;
    Invocation(Invocation&&) = delete;
    Invocation& operator=(Invocation&&) = delete;

    NativeGuiIconPropertyStateInvocations& owner_;
    NativeTextureCacheAcquired resolver_acquired_;
    NativeTextureCacheAcquired placeholder_acquired_;
    NativeRenderActualOwnerReleaseObservation old_release_;
    // Deliberately default-initialized by the user-provided constructor, with
    // no ()/{} or member initializer. Native home preimages are not seeded.
    NativeGuiIconStateConstructionStorageFrame frame_;
    NativeGuiIconStateGrowthStorageContext growth_;
    NativeGuiTextureResolverStorageContext resolver_;
    NativeGuiIconPlaceholderStorageContext placeholder_;
    NativeGuiIconStateConstructionStorageContext context_;
    Invocation* next_{};
    Phase phase_{Phase::prepared};
    std::exception_ptr failure_;
    std::uint32_t returned_bits_{}; // Source history, not a Native HOME.
};

NativeGuiIconPropertyStateInvocations::NativeGuiIconPropertyStateInvocations(
    NativeGuiIconReference& companion, const NativeGuiIconPropertyStateServices& services)
    : companion_(companion), services_(services), actual_icon_(companion.actual_identity()) {}

NativeGuiIconPropertyStateInvocations::~NativeGuiIconPropertyStateInvocations() {
    // Metadata destruction only under the explicit full-lifetime precondition.
    // No phase scan, logical retirement, Native destructor or debt discharge.
    while (first_) {
        Invocation* const old = first_;
        first_ = old->next_;
        delete old;
    }
}

NativeGuiIconPropertyStateInvocations::Invocation&
NativeGuiIconPropertyStateInvocations::prepare() {
    preparation_started_ = true;
    Invocation* fresh;
    try {
        fresh = new Invocation(*this, services_);
    } catch (...) {
        preparation_failed_ = true;
        if (!first_preparation_failure_) first_preparation_failure_ = std::current_exception();
        throw;
    }
    // Read CURRENT head AFTER allocation/construction: a real Source new-handler
    // may have synchronously prepared/invoked a different fresh nested record.
    // Publication is nonallocating; it never erases that nested/current chain.
    fresh->next_ = first_;
    first_ = fresh;
    return *fresh;
}

void NativeGuiIconPropertyStateInvocations::require_record(const Invocation& record) const {
    if (&record.owner_ != this)
        throw std::logic_error("Icon state invocation belongs to another retained Source owner");
}

NativeGuiIconStateConstructionStorageFrame&
NativeGuiIconPropertyStateInvocations::preparation_frame(Invocation& record) {
    require_record(record);
    if (record.phase_ != Phase::prepared)
        throw std::logic_error("Icon state Frame preparation cannot replay an entered record");
    return record.frame_;
}

std::uint32_t NativeGuiIconPropertyStateInvocations::invoke_00ab66c0(
    Invocation& record, volatile std::uint32_t (&actual_argument_homes)[4]) {
    require_record(record);
    if (record.phase_ != Phase::prepared)
        throw std::logic_error("Icon state invocation cannot replay");
    record.phase_ = Phase::entered;
    try {
        record.returned_bits_ = add_native_gui_icon_state_storage_00ab66c0(
            actual_icon_, actual_argument_homes, record.frame_, record.context_);
        record.phase_ = Phase::normal_returned;
        return record.returned_bits_;
    } catch (...) {
        record.phase_ = Phase::failed;
        record.failure_ = std::current_exception();
        throw;
    }
}

const NativeGuiIconPropertyStateInvocations::Invocation*
NativeGuiIconPropertyStateInvocations::first() const { return first_; }
const NativeGuiIconPropertyStateInvocations::Invocation*
NativeGuiIconPropertyStateInvocations::next(const Invocation& record) const {
    require_record(record); return record.next_;
}
NativeGuiIconPropertyStateInvocations::Phase
NativeGuiIconPropertyStateInvocations::phase(const Invocation& record) const {
    require_record(record); return record.phase_;
}
const std::exception_ptr&
NativeGuiIconPropertyStateInvocations::failure(const Invocation& record) const {
    require_record(record); return record.failure_;
}
std::uint32_t NativeGuiIconPropertyStateInvocations::returned_bits(const Invocation& record) const {
    require_record(record); return record.returned_bits_;
}
bool NativeGuiIconPropertyStateInvocations::preparation_started() const { return preparation_started_; }
bool NativeGuiIconPropertyStateInvocations::preparation_failed() const { return preparation_failed_; }
const std::exception_ptr&
NativeGuiIconPropertyStateInvocations::first_preparation_failure() const {
    return first_preparation_failure_;
}
} // namespace bsp
