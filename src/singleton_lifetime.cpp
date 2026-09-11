#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <new.h>

#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Singleton lifetime reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
constexpr std::uint32_t max_pointer_count = 0x3FFFFFFFU;
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18,
    "The recovered pointer arithmetic and Win32 section require x86.");

std::uintptr_t address(const void* pointer) noexcept {
    return reinterpret_cast<std::uintptr_t>(pointer);
}
std::uint32_t distance(void* const* first, void* const* last) noexcept {
    // Native SUB followed by SAR 2, including the malformed-storage bit pattern.
    return static_cast<std::uint32_t>(
        static_cast<std::int32_t>(address(last) - address(first)) >> 2);
}
void** offset(void** pointer, std::uint32_t count) noexcept {
    return reinterpret_cast<void**>(address(pointer) + count * 4U);
}
void copy_slots(void** destination, void** first, void** last) {
    const auto bytes = distance(first, last) * 4U;
    if (bytes != 0) {
        // 00BD0500 reaches memmove_s only for a nonempty range.
        (void)memmove_s(destination, bytes, first, bytes);
    }
}
void** allocate_slots(std::uint32_t count) {
    if (count > max_pointer_count) {
        throw std::bad_alloc(); // 00BCFEB0 multiplication-overflow branch
    }
    return static_cast<void**>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, count * 4U, count * sizeof(void*)}));
}
} // namespace

void* singleton_lifetime_allocate(const SingletonAllocationRequest& request) {
    // 00BF681B: malloc, call the CRT new handler with the allocation size on
    // failure, retry if it returns nonzero, otherwise throw bad_alloc. Host
    // layout requires host_bytes; native_bytes retains the explicit ABI boundary.
    for (;;) {
        if (void* result = std::malloc(request.host_bytes)) {
            return result;
        }
        if (_callnewh(request.host_bytes) == 0) {
            throw std::bad_alloc();
        }
    }
}

void singleton_lifetime_free(void* pointer) noexcept {
    std::free(pointer);
}

struct ConcreteSingletonLifetimeManager::OwnedCriticalSection {
    CRITICAL_SECTION native;
    std::uint32_t recursion_18;
    SystemSingletonCriticalSection projection{&native, recursion_18};
};

void singleton_enter_critical_section(SystemSingletonCriticalSection& section) {
    EnterCriticalSection(static_cast<CRITICAL_SECTION*>(section.native_section));
}
void singleton_leave_critical_section(SystemSingletonCriticalSection& section) noexcept {
    LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(section.native_section));
}

ConcreteSingletonLifetimeManager::ConcreteSingletonLifetimeManager(
    SingletonLifetimeCallbacks callbacks) : callbacks_(callbacks) {
    static_assert(offsetof(OwnedCriticalSection, recursion_18) == 0x18,
        "The projected counter must be the actual section's native +18h field.");
    // 00BD0960 leaves native +0 untouched, zeros +4/+8/+C, reserves 256, then
    // creates the tracked lock. This class's vptr/layout is a typed interface.
    slots_.begin = allocate_slots(0x100);
    slots_.end = slots_.begin;
    slots_.capacity_end = offset(slots_.begin, 0x100);
    try {
        void* storage = singleton_lifetime_allocate({
            SingletonAllocationKind::critical_section, 0x1C, sizeof(OwnedCriticalSection)});
        owned_section_ = ::new (storage) OwnedCriticalSection;
        InitializeCriticalSection(&owned_section_->native);
        owned_section_->recursion_18 = 0;
        section_10_ = &owned_section_->projection;
    } catch (...) {
        singleton_lifetime_free(owned_section_);
        owned_section_ = nullptr;
        singleton_lifetime_free(slots_.begin);
        slots_ = {};
        throw;
    }
}

ConcreteSingletonLifetimeManager::~ConcreteSingletonLifetimeManager() {
    destroy_00bd0400();
}

void ConcreteSingletonLifetimeManager::lock() {
    singleton_enter_critical_section(*section_10_);
    ++section_10_->recursion_18;
}
void ConcreteSingletonLifetimeManager::unlock() {
    --section_10_->recursion_18;
    singleton_leave_critical_section(*section_10_);
}
void ConcreteSingletonLifetimeManager::invalid_parameter() {
    callbacks_.invalid_parameter(callbacks_.context);
}
std::uint32_t ConcreteSingletonLifetimeManager::count_00bcf910() const noexcept {
    return slots_.begin ? distance(slots_.begin, slots_.end) : 0;
}
std::uint32_t ConcreteSingletonLifetimeManager::capacity() const noexcept {
    return slots_.begin ? distance(slots_.begin, slots_.capacity_end) : 0;
}

void ConcreteSingletonLifetimeManager::register_object(void* object) {
    if (address(slots_.end) < address(slots_.begin)) {
        invalid_parameter();
    }
    if (object != nullptr) {
        append_pointer_00bd0bc0(&object);
    }
}

void ConcreteSingletonLifetimeManager::append_pointer_00bd0bc0(void* const* value) {
    if (slots_.begin && count_00bcf910() < capacity()) {
        *slots_.end = *value;
        slots_.end = offset(slots_.end, 1);
        return;
    }
    void** const position = slots_.end;
    if (address(position) < address(slots_.begin)) invalid_parameter();
    insert_pointer_at_checked_00bd08d0(position, value);
}

void ConcreteSingletonLifetimeManager::insert_pointer_at_checked_00bd08d0(
    void** position, void* const* value) {
    // 00BD08D0 captures the iterator's offset before insertion. Its owner is
    // this same container, so its distinct foreign-owner branch is unreachable.
    std::uint32_t index = 0;
    void** const index_begin = slots_.begin; // EDI retained across callback
    void** const index_end = slots_.end;
    if (index_begin && distance(index_begin, index_end) != 0) {
        if (address(index_end) < address(index_begin)) {
            invalid_parameter();
        }
        index = distance(index_begin, position);
    }
    // 00BD0700 captures the pointed-to object before moving/freeing any slots.
    void* const captured = *value;
    const auto old_count = count_00bcf910();
    if (max_pointer_count - old_count < 1U) {
        throw std::length_error("vector<T> too long"); // 00BD0590
    }
    const auto old_capacity = capacity();
    if (old_capacity < old_count + 1U) {
        auto grown = old_capacity > max_pointer_count - (old_capacity >> 1U)
            ? 0U : old_capacity + (old_capacity >> 1U);
        if (grown < old_count + 1U) {
            grown = old_count + 1U;
        }
        auto** replacement = allocate_slots(grown);
        copy_slots(replacement, slots_.begin, position);
        const auto prefix = distance(slots_.begin, position);
        *offset(replacement, prefix) = captured;
        copy_slots(offset(replacement, prefix + 1U), position, slots_.end);
        const auto retained_count = count_00bcf910();
        singleton_lifetime_free(slots_.begin);
        slots_.begin = replacement;
        slots_.capacity_end = offset(replacement, grown);
        slots_.end = offset(replacement, retained_count + 1U);
    } else {
        // Insertion within retained storage; memmove_s supports overlap.
        copy_slots(offset(position, 1), position, slots_.end);
        slots_.end = offset(slots_.end, 1);
        *position = captured;
    }
    void** const returned_begin = slots_.begin; // 00BD0925, before callback
    if (address(slots_.end) < address(returned_begin)) {
        invalid_parameter();
    }
    void** const returned_iterator = offset(returned_begin, index);
    if (address(returned_iterator) > address(slots_.end) ||
        address(returned_iterator) < address(slots_.begin)) {
        invalid_parameter();
    }
}

void ConcreteSingletonLifetimeManager::move_object_after_00bd0d70(
    void* object, void* after) {
    // The typed manager cannot be null or own a foreign checked iterator. The
    // remaining native bounds checks continue if invalid_parameter returns.
    auto find_first = [&](void* value) {
        void** cursor = slots_.begin;
        if (address(cursor) > address(slots_.end)) invalid_parameter();
        for (;;) {
            void** const end = slots_.end;
            if (address(slots_.begin) > address(end)) invalid_parameter();
            if (cursor == end) break;
            if (address(cursor) >= address(slots_.end)) invalid_parameter();
            if (*cursor == value) break;
            if (address(cursor) >= address(slots_.end)) invalid_parameter();
            cursor = offset(cursor, 1);
        }
        return cursor;
    };
    void** const position = find_first(object);
    if (address(position) >= address(slots_.end)) invalid_parameter();
    void* const captured = *position;
    void** const following = offset(position, 1);
    const auto remaining = static_cast<std::int32_t>(distance(following, slots_.end));
    if (remaining > 0) copy_slots(position, following, slots_.end);
    slots_.end = offset(slots_.end, 0xffffffffU); // native ADD end,-4
    if (address(slots_.end) < address(slots_.begin)) invalid_parameter();

    void** const anchor = find_first(after);
    void** const captured_end = slots_.end; // 00BD0E75 stores end before bounds checks
    if (address(captured_end) < address(slots_.begin)) invalid_parameter();
    void** const insertion = offset(anchor, 1);
    if (address(insertion) > address(slots_.end) ||
        address(insertion) < address(slots_.begin)) invalid_parameter();
    if (insertion == captured_end) {
        append_pointer_00bd0bc0(&captured);
        return;
    }
    if (address(insertion) > address(slots_.end) ||
        address(insertion) < address(slots_.begin)) invalid_parameter();
    insert_pointer_at_checked_00bd08d0(insertion, &captured);
}

void ConcreteSingletonLifetimeManager::unregister_object(void* object) {
    if (!object) {
        return;
    }
    std::uint32_t index = 0;
    if (count_00bcf910() == 0) {
        return;
    }
    for (;;) {
        if (!slots_.begin || count_00bcf910() <= index) {
            invalid_parameter();
        }
        if (*offset(slots_.begin, index) == object) {
            if (!slots_.begin || count_00bcf910() <= index) {
                invalid_parameter();
            }
            *offset(slots_.begin, index) = nullptr;
            return;
        }
        ++index;
        if (count_00bcf910() <= index) {
            return;
        }
    }
}

void ConcreteSingletonLifetimeManager::destroy_00bd0400() {
    while (count_00bcf910() != 0) {
        void** const captured_end = slots_.end;
        if (address(captured_end) < address(slots_.begin)) {
            invalid_parameter();
        }
        void** const last = offset(captured_end, 0xFFFFFFFFU);
        if (address(last) > address(slots_.end) || address(last) < address(slots_.begin)) {
            invalid_parameter();
        }
        if (address(slots_.end) <= address(last)) {
            invalid_parameter();
        }
        void* const object = *last;
        if (slots_.begin && count_00bcf910() != 0) {
            slots_.end = offset(slots_.end, 0xFFFFFFFFU);
        }
        if (object) {
            callbacks_.destroy_registered(callbacks_.context, object, 1);
        }
        // Reload count: destructors may append or null existing slots.
    }
    destroy_owned_section_0041cc80();
    singleton_lifetime_free(slots_.begin);
    slots_ = {};
}

void ConcreteSingletonLifetimeManager::destroy_owned_section_0041cc80() noexcept {
    if (!owned_section_) {
        return;
    }
    while (owned_section_->recursion_18 != 0 && owned_section_->recursion_18 < 0x80000000U) {
        --owned_section_->recursion_18;
        LeaveCriticalSection(&owned_section_->native);
    }
    DeleteCriticalSection(&owned_section_->native);
    owned_section_->~OwnedCriticalSection();
    singleton_lifetime_free(owned_section_);
    owned_section_ = nullptr;
    section_10_ = nullptr; // native clear occurs after free, at 0041CCB6
}

SingletonLifetimeDomain::SingletonLifetimeDomain(SingletonLifetimeCallbacks callbacks)
    : callbacks_(callbacks) {
    if (!callbacks_.destroy_registered || !callbacks_.invalid_parameter) {
        throw std::invalid_argument("Singleton lifetime requires owner destruction and validation callbacks");
    }
}
SingletonLifetimeDomain::~SingletonLifetimeDomain() {
    shutdown();
}
ConcreteSingletonLifetimeManager* SingletonLifetimeDomain::get_manager_00415350() {
    auto* manager = published_;
    if (!manager) {
        void* storage = singleton_lifetime_allocate({SingletonAllocationKind::manager,
            0x14, sizeof(ConcreteSingletonLifetimeManager)});
        try {
            manager = storage ? ::new (storage) ConcreteSingletonLifetimeManager(callbacks_) : nullptr;
            published_ = manager;
        } catch (...) {
            singleton_lifetime_free(storage);
            throw;
        }
    }
    return manager;
}
void SingletonLifetimeDomain::shutdown() {
    if (auto* manager = published_) {
        manager->~ConcreteSingletonLifetimeManager();
        singleton_lifetime_free(manager);
        published_ = nullptr;
    }
}

} // namespace bsp
