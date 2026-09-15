#pragma once
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
class SoundLifetimeAccess;
class CapturedSoundLifetimeSection;

// A borrowed manager view. Resolve the manager before evaluating registration
// arguments, preserving the native second-getter/current-publication ordering.
// Neither this view nor SoundLifetimeAccess allocates a private owner/domain.
class SoundLifetimeManagerView final {
public:
    SoundLifetimeManagerView* operator->() noexcept { return this; }
    // Capture this resolved manager's native +10h critical-section pointer.
    // This neither re-fetches the manager nor enters the section. A semantic
    // domain supplies its existing native_section; actual storage reads +10h.
    void* native_system_section_10() const noexcept;
    void register_object(void*);
    void unregister_object(void*);
    void move_object_after_00bd0d70(void*, void* anchor);
private:
    friend class SoundLifetimeAccess;
    friend class CapturedSoundLifetimeSection;
    SoundLifetimeManagerView(ConcreteSingletonLifetimeManager* semantic, void* actual) noexcept
        : semantic_(semantic), actual_(actual) {}
    ConcreteSingletonLifetimeManager* semantic_;
    void* actual_;
};

// Source integration boundary: either the existing semantic fixture domain or
// the application's actual01090AA0 cell. Borrowed publication storage must
// outlive all sound registration/destruction. Only the caller drains it.
class SoundLifetimeAccess final {
public:
    SoundLifetimeAccess(SingletonLifetimeDomain& domain) noexcept : semantic_(&domain) {}
    SoundLifetimeAccess(void* volatile& actual_publication) noexcept : actual_(&actual_publication) {}
    SoundLifetimeManagerView get_manager_00415350() const;
    bool uses_actual_storage() const noexcept { return actual_ != nullptr; }
    // Compare borrowed storage identity without loading or creating a manager.
    // Copies of a view share a domain; raw and semantic views never do.
    bool borrows_same_domain(SoundLifetimeAccess other) const noexcept {
        return semantic_ == other.semantic_ && actual_ == other.actual_;
    }
private:
    SingletonLifetimeDomain* semantic_{};
    void* volatile* actual_{};
};

// Capture the first manager's section, enter/increment once, then leave that
// same section even if callbacks replace its publication. Raw sections are
// actual1Ch Win32 CRITICAL_SECTION plus recursion word18; fixture sections use
// their existing semantic provider. No synchronization is added elsewhere.
class CapturedSoundLifetimeSection final {
public:
    explicit CapturedSoundLifetimeSection(SoundLifetimeAccess);
    ~CapturedSoundLifetimeSection();
    CapturedSoundLifetimeSection(const CapturedSoundLifetimeSection&) = delete;
    CapturedSoundLifetimeSection& operator=(const CapturedSoundLifetimeSection&) = delete;
private:
    SystemSingletonCriticalSection* semantic_{};
    void* actual_{};
};
} // namespace bsp
