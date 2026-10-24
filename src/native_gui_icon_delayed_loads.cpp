#include "bsp/native_gui_icon_delayed_loads.hpp"
#include "bsp/sound_lifetime_access.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Icon delayed loads require MSVC Win32.
#endif

namespace bsp {
namespace {
using Operation = NativeGuiIconDelayedLoadsOperation;
using Phase = Operation::Phase;
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
void require_array(const NativeGuiIconDelayedLoadsStorage& storage) {
    const auto count = storage.count_08;
    const auto capacity = storage.capacity_0c;
    require(count >= 0 && capacity >= count &&
        capacity <= std::numeric_limits<std::int32_t>::max() / 0x0c &&
        (!capacity || storage.entries_04), "Raw Icon delayed array is outside the valid native domain");
}
} // namespace

struct NativeGuiIconDelayedLoads::Impl {
    NativeStringRawPoolContext& strings;
    NativeGuiIconDelayedLoadsStorage* volatile& publication;
    // Metadata borrow only. Actual ownership is the one registration in the
    // existing process manager. There is no second registration or native count.
    NativeGuiIconDelayedLoadsStorage* registered_identity{};
    std::unique_ptr<Operation> operation;
    bool active{}, failed{};
    std::exception_ptr failure;

    Impl(NativeStringRawPoolContext& strings_in,
        NativeGuiIconDelayedLoadsStorage* volatile& publication_in)
        : strings(strings_in), publication(publication_in) {
        require(publication == nullptr, "Raw Icon delayed producer requires its fresh canonical null publication");
    }
    template<class F> auto invoke(Operation::Kind kind, std::uint32_t entry, F&& body) -> decltype(body()) {
        require(!active && !failed, "Raw Icon delayed service cannot reenter or replay a failed call");
        auto fresh = std::make_unique<Operation>();
        fresh->kind = kind;
        fresh->native_site = entry;
        operation = std::move(fresh); // Prior completed frame owns no native effect.
        active = true;
        try {
            if constexpr (std::is_void_v<decltype(body())>) {
                body();
                operation->phase = Phase::complete;
                active = false;
            } else {
                auto result = body();
                operation->phase = Phase::complete;
                active = false;
                return result;
            }
        } catch (...) {
            operation->failure = failure = std::current_exception();
            operation->phase = Phase::failed;
            failed = true;
            active = false;
            throw;
        }
    }
    void require_owned(NativeGuiIconDelayedLoadsStorage* raw) const {
        require(raw && raw == registered_identity && raw->native_vtable_00 == 0x00d5c4b8,
            "Raw Icon delayed call requires its same actual registered D5C4B8 owner");
        require_array(*raw);
    }
    NativeGuiIconDelayedLoadsStorage* get() {
        auto* const initial = publication; // AB5715; fast return uses this capture.
        if (initial) {
            require_owned(initial);
            operation->actual = initial;
            return initial;
        }
        require(!registered_identity, "Raw Icon delayed publication was cleared before its registered owner retired");
        SoundLifetimeAccess lifetime(strings.actual_manager_publication_01090aa0);
        operation->phase = Phase::manager_guard;
        operation->native_site = 0x00ab5725;
        try {
            CapturedSoundLifetimeSection section(lifetime); // Same first manager/section.
            operation->guard_entered = true;
            if (!publication) { // AB574C, recheck only after entering captured section.
                operation->phase = Phase::allocation;
                operation->native_site = 0x00ab5756;
                void* raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x10, 0x10});
                operation->allocated = raw;
                auto* created = raw ? ::new (raw) NativeGuiIconDelayedLoadsStorage{
                    0x00d5c4b8, nullptr, 0, 0} : nullptr;
                operation->actual = created;
                operation->phase = Phase::publication;
                operation->native_site = 0x00ab5775;
                publication = created;
                registered_identity = created; // Borrow, including a later registration failure.
                operation->phase = Phase::registration;
                operation->native_site = 0x00ab577a;
                auto manager = lifetime.get_manager_00415350();
                operation->native_site = 0x00ab5788;
                manager.register_object(publication); // Reload current publication after getter.
                operation->registration_returned = true;
            }
        } catch (...) {
            // The existing captured-section helper completed its native guard
            // cleanup while unwinding. No publication/allocation rollback.
            operation->guard_cleanup_completed = operation->guard_entered;
            throw;
        }
        operation->guard_cleanup_completed = true;
        operation->native_site = 0x00ab579c;
        auto* result = publication; // Slow return reloads AFTER captured-section leave.
        if (result) require_owned(result);
        return result;
    }
    void cancel(NativeGuiIconDelayedLoadsStorage& storage, const void* owner) {
        require_owned(&storage);
        operation->actual = &storage;
        operation->cancel_owner = owner;
        operation->phase = Phase::cancel;
        std::int32_t index = 0;
        // AB50E6's real count gate is reached even for a fresh empty singleton.
        while (index < storage.count_08) {
            auto* entry = storage.entries_04 + index;
            if (entry->owner_08 != owner) {
                ++index; // AB5192; removed entries do not advance this index.
                continue;
            }
            if (index != storage.count_08 - 1) {
                auto* source = storage.entries_04 + storage.count_08 - 1;
                auto* destination = storage.entries_04 + index;
                if (source != destination) {
                    operation->native_site = 0x00ab513c;
                    resize_native_string_header_0041dd40(destination, strings, source->length_00, true);
                    // Native tests CURRENT source length, then copies CURRENT
                    // destination length from fresh source/destination buffers.
                    if (source->length_00) {
                        const auto bytes = destination->length_00;
                        if (bytes) std::memcpy(destination->characters_04, source->characters_04, bytes);
                    }
                }
                destination->owner_08 = source->owner_08; // AB5159, after string callbacks.
            }
            auto* tail = storage.entries_04 + storage.count_08 - 1; // Native reload.
            operation->native_site = 0x00ab517c;
            destroy_native_string_header_0041dd20(tail, strings);
            storage.count_08 = storage.count_08 - 1; // AFTER release, AB5188.
        }
    }
    void* scalar_delete(void* raw, std::uint32_t flags) {
        auto* storage = static_cast<NativeGuiIconDelayedLoadsStorage*>(raw);
        require_owned(storage);
        operation->actual = storage;
        operation->flags = flags;
        operation->phase = Phase::shrink;
        operation->native_site = 0x00ab558d;
        // Reached AB4850(0) shrink arm: decrement BEFORE each tail's string
        // release, reload current count/backing after callbacks, then store0.
        while (storage->count_08 > 0) {
            storage->count_08 = storage->count_08 - 1;
            auto* tail = storage->entries_04 + storage->count_08;
            destroy_native_string_header_0041dd20(tail, strings);
        }
        storage->count_08 = 0;
        operation->phase = Phase::backing_free;
        operation->native_site = 0x00ab5595;
        operation->backing_free_started = true;
        singleton_lifetime_free(storage->entries_04); // Do not clear consumed raw pointer/capacity.
        operation->phase = Phase::publication_clear;
        operation->native_site = 0x00ab55a1;
        publication = nullptr; // Current cell cleared even when it differs from popped owner.
        operation->phase = Phase::base_profile;
        operation->native_site = 0x00ab55ab;
        storage->native_vtable_00 = 0x00ce3818;
        if (flags & 1u) {
            operation->phase = Phase::object_free;
            operation->native_site = 0x00ab57d0;
            operation->object_free_started = true;
            singleton_lifetime_free(storage);
        }
        registered_identity = nullptr; // No access to consumed storage follows.
        return raw;
    }
};

NativeGuiIconDelayedLoads::NativeGuiIconDelayedLoads(NativeStringRawPoolContext& strings,
    NativeGuiIconDelayedLoadsStorage* volatile& publication)
    : impl_(std::make_unique<Impl>(strings, publication)) {}
NativeGuiIconDelayedLoads::~NativeGuiIconDelayedLoads() noexcept {
    if (requires_process_retention()) std::terminate();
}
NativeGuiIconDelayedLoadsStorage* NativeGuiIconDelayedLoads::get_00ab5700() {
    return impl_->invoke(Operation::Kind::getter, 0x00ab5700, [&] { return impl_->get(); });
}
void NativeGuiIconDelayedLoads::cancel_owner_00ab50e0(NativeGuiIconDelayedLoadsStorage& storage,
    const void* owner) {
    impl_->invoke(Operation::Kind::cancel, 0x00ab50e0, [&] { impl_->cancel(storage, owner); });
}
void* NativeGuiIconDelayedLoads::delete_registered_00ab57c0(void* raw, std::uint32_t flags) {
    return impl_->invoke(Operation::Kind::scalar_delete, 0x00ab57c0,
        [&] { return impl_->scalar_delete(raw, flags); });
}
bool NativeGuiIconDelayedLoads::requires_process_retention() const noexcept {
    return impl_->active || impl_->failed || impl_->registered_identity != nullptr;
}
const NativeGuiIconDelayedLoadsOperation* NativeGuiIconDelayedLoads::last_operation() const noexcept {
    return impl_->operation.get();
}
const std::exception_ptr& NativeGuiIconDelayedLoads::failure() const noexcept { return impl_->failure; }
} // namespace bsp
