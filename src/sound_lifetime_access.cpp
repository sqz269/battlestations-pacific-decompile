#include "bsp/sound_lifetime_access.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstddef>
#include <cstring>

namespace bsp {
SoundLifetimeManagerView SoundLifetimeAccess::get_manager_00415350() const {
    if (actual_) return {nullptr, get_native_singleton_manager_00415350(*actual_)};
    return {semantic_->get_manager_00415350(), nullptr};
}
void* SoundLifetimeManagerView::native_system_section_10() const noexcept {
    if (semantic_) {
        auto* const projection = semantic_->system_owner().section_10;
        return projection ? projection->native_section : nullptr;
    }
    void* section;
    std::memcpy(&section, static_cast<const std::byte*>(actual_) + 0x10, sizeof section);
    return section;
}
void SoundLifetimeManagerView::register_object(void* object) {
    if (semantic_) semantic_->register_object(object);
    else register_native_singleton_object_00bd0c30(actual_, nullptr, object);
}
void SoundLifetimeManagerView::unregister_object(void* object) {
    if (semantic_) semantic_->unregister_object(object);
    else unregister_native_singleton_object_00bcfca0(actual_, nullptr, object);
}
void SoundLifetimeManagerView::move_object_after_00bd0d70(void* object, void* anchor) {
    if (semantic_) semantic_->move_object_after_00bd0d70(object, anchor);
    else move_native_singleton_object_after_00bd0d70(actual_, nullptr, object, anchor);
}
CapturedSoundLifetimeSection::CapturedSoundLifetimeSection(SoundLifetimeAccess access) {
    const auto manager = access.get_manager_00415350();
    if (manager.semantic_) {
        semantic_ = manager.semantic_->system_owner().section_10;
        if (semantic_) {
            singleton_enter_critical_section(*semantic_);
            ++semantic_->recursion_18;
        }
    } else {
        std::memcpy(&actual_, static_cast<const std::byte*>(manager.actual_) + 0x10, sizeof actual_);
        if (actual_) {
            EnterCriticalSection(static_cast<CRITICAL_SECTION*>(actual_));
            auto& depth = *reinterpret_cast<volatile std::uint32_t*>(static_cast<std::byte*>(actual_) + 0x18);
            depth = depth + 1u;
        }
    }
}
CapturedSoundLifetimeSection::~CapturedSoundLifetimeSection() {
    if (semantic_) {
        --semantic_->recursion_18;
        singleton_leave_critical_section(*semantic_);
    } else if (actual_) {
        auto& depth = *reinterpret_cast<volatile std::uint32_t*>(static_cast<std::byte*>(actual_) + 0x18);
        depth = depth - 1u;
        LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(actual_));
    }
}
} // namespace bsp
