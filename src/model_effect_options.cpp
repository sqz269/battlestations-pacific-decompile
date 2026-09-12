#include "bsp/model_effect_options.hpp"
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Model effect options require MSVC Win32.
#endif

namespace bsp {
namespace {
class CapturedSection final {
public:
    explicit CapturedSection(SystemSingletonCriticalSection* section) : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~CapturedSection() {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
        }
    }
    CapturedSection(const CapturedSection&) = delete;
    CapturedSection& operator=(const CapturedSection&) = delete;
private:
    SystemSingletonCriticalSection* section_;
};
}

NativeModelEffectOptionsStorage* model_effect_options_singleton_0051f6b0(
    NativeModelEffectOptionsStorage* volatile& global, SingletonLifetimeDomain& domain) {
    if (auto* existing = global) return existing;
    {
        CapturedSection section(domain.get_manager_00415350()->system_owner().section_10);
        if (!global) {
            void* const raw = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, 8, sizeof(NativeModelEffectOptionsStorage)});
            NativeModelEffectOptionsStorage* created = nullptr;
            if (raw) {
                created = ::new(raw) NativeModelEffectOptionsStorage;
                created->native_table_00 = 0x00ceca14u;
                created->option_04 = 0;
                created->option_05 = 0;
            }
            global = created;
            auto* const manager = domain.get_manager_00415350();
            manager->register_object(global);
        }
    }
    return global;
}

NativeModelEffectOptionsStorage* delete_model_effect_options_0051f770(
    NativeModelEffectOptionsStorage* owner, std::uint32_t flags,
    NativeModelEffectOptionsStorage* volatile& global) noexcept {
    auto* const original = owner;
    global = nullptr;
    owner->native_table_00 = 0x00ce3818u;
    if (flags & 1u) {
        owner->~NativeModelEffectOptionsStorage();
        singleton_lifetime_free(owner);
    }
    return original;
}
} // namespace bsp
