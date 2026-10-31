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

// C++ metadata storage only. User-provided constructor deliberately omits the
// trivial Native homes: new T() must not value-zero their preimages.
struct NativeGuiIconDelayedRawScalarCallStorage {
    NativeGuiIconDelayedLoadsDestroyStorageFrame native_frame;
    std::uint32_t flags_home;
    std::unique_ptr<Operation> prepared_operation;
    NativeGuiIconDelayedRawScalarCallStorage()
        : prepared_operation(std::make_unique<Operation>()) {}
};

struct NativeGuiIconDelayedLoadsRawScalarDispatchContext::Impl {
    NativeGuiIconDelayedLoads* const expected_service;
    std::unique_ptr<NativeGuiIconDelayedRawScalarCallStorage> pending;
    bool armed{}, unknown_debt{}; // Source DENIAL only, never Native permission.
    explicit Impl(NativeGuiIconDelayedLoads& service)
        : expected_service(&service),
          pending(std::make_unique<NativeGuiIconDelayedRawScalarCallStorage>()) {}
};

struct NativeGuiIconDelayedLoads::Impl {
    NativeStringRawPoolContext& strings;
    NativeGuiIconDelayedLoadsStorage* volatile& publication;
    // Metadata borrow only. Actual ownership is the one registration in the
    // existing process manager. There is no second registration or native count.
    NativeGuiIconDelayedLoadsStorage* registered_identity{};
    std::unique_ptr<Operation> operation;
    bool active{}, failed{};
    std::exception_ptr failure;
    // Preparation-only immutable borrow, appended after the complete old prefix.
    // Null means the explicit legacy route; it never supplies a fallback value.
    const volatile std::uint32_t* const actual_feature_word;
    // Own only adopted C++ call storage, never the Native receiver/backing.
    std::unique_ptr<NativeGuiIconDelayedRawScalarCallStorage> retained_raw_scalar_call;

    Impl(NativeStringRawPoolContext& strings_in,
        NativeGuiIconDelayedLoadsStorage* volatile& publication_in)
        : Impl(strings_in, publication_in, nullptr) {}
    Impl(NativeStringRawPoolContext& strings_in,
        NativeGuiIconDelayedLoadsStorage* volatile& publication_in,
        const volatile std::uint32_t& feature_in)
        : Impl(strings_in, publication_in, &feature_in) {}
    Impl(NativeStringRawPoolContext& strings_in,
        NativeGuiIconDelayedLoadsStorage* volatile& publication_in,
        const volatile std::uint32_t* feature_in)
        : strings(strings_in), publication(publication_in), actual_feature_word(feature_in) {
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

NativeGuiIconDelayedLoadsRawScalarDispatchContext::NativeGuiIconDelayedLoadsRawScalarDispatchContext(
    std::unique_ptr<Impl> prepared) : impl_(std::move(prepared)) {}
NativeGuiIconDelayedLoadsRawScalarDispatchContext::~NativeGuiIconDelayedLoadsRawScalarDispatchContext() = default;
bool NativeGuiIconDelayedLoadsRawScalarDispatchContext::unknown_escape_debt() const {
    return impl_->unknown_debt;
}
void NativeGuiIconDelayedLoadsRawScalarDispatchContext::arm_before_source_dispatch() {
    const bool prior_debt = impl_->armed || impl_->unknown_debt;
    impl_->armed = true; // BEFORE even a late admission error can escape.
    require(!prior_debt, "prepared raw Icon dispatch cannot reenter or replay Source debt");
}
bool NativeGuiIconDelayedLoadsRawScalarDispatchContext::matches_service(
    const NativeGuiIconDelayedLoads* service) const {
    return impl_->expected_service == service;
}
void NativeGuiIconDelayedLoadsRawScalarDispatchContext::stage_actual_pushed_flags(
    std::uint32_t flags) {
    require(impl_->pending != nullptr, "prepared raw Icon dispatch was already consumed");
    // Actual caller PUSH transport, before destroy. Never an early flags read.
    impl_->pending->flags_home = flags;
}
void NativeGuiIconDelayedLoadsRawScalarDispatchContext::disarm_after_normal_source_return() {
    impl_->armed = false; // No throwing action or subsequent target precedes it.
}
bool gui_icon_raw_scalar_denies_manager_cleanup(
    const NativeGuiIconDelayedLoadsRawScalarDispatchContext& context) {
    return context.impl_->armed;
}
void note_gui_icon_raw_scalar_unknown_escape(
    NativeGuiIconDelayedLoadsRawScalarDispatchContext& context) {
    context.impl_->unknown_debt = true; // Never classify the exception as Native.
}

std::unique_ptr<NativeGuiIconDelayedLoadsRawScalarDispatchContext>
NativeGuiIconDelayedLoads::prepare_raw_scalar_dispatch() {
    require(!impl_->active && !impl_->failed,
        "raw Icon dispatch preparation requires an idle nonfailed service");
    require(impl_->actual_feature_word != nullptr,
        "raw Icon dispatch preparation requires the genuine canonical feature borrow");
    // Only metadata/address admission. No home, owner, publication or feature
    // value is read. Complete all allocation before any caller Native effects.
    auto prepared = std::make_unique<NativeGuiIconDelayedLoadsRawScalarDispatchContext::Impl>(*this);
    return std::unique_ptr<NativeGuiIconDelayedLoadsRawScalarDispatchContext>(
        new NativeGuiIconDelayedLoadsRawScalarDispatchContext(std::move(prepared)));
}
void* NativeGuiIconDelayedLoads::delete_registered_raw_storage_00ab57c0(
    void* raw, NativeGuiIconDelayedLoadsRawScalarDispatchContext& context) {
    require(context.impl_->armed && context.impl_->expected_service == this,
        "raw Icon scalar requires its actual armed caller context");
    require(!impl_->active && !impl_->failed,
        "raw Icon scalar cannot reenter or replay a failed service");
    require(impl_->actual_feature_word != nullptr && context.impl_->pending &&
        context.impl_->pending->prepared_operation,
        "raw Icon scalar requires its cold canonical prepared operation");
    require(raw && raw == impl_->registered_identity,
        "raw Icon scalar requires its actual registered Source construction history");
    // No require_array/profile/count/capacity or flags-value input filter.
    // Adopt before raw entry; unique_ptr moves keep home/acquisition addresses.
    auto adopted = std::move(context.impl_->pending);
    impl_->operation = std::move(adopted->prepared_operation);
    impl_->retained_raw_scalar_call = std::move(adopted);
    auto& operation = *impl_->operation;
    auto& call = *impl_->retained_raw_scalar_call;
    operation.kind = Operation::Kind::scalar_delete;
    operation.native_site = 0x00ab57c0;
    operation.actual = impl_->registered_identity; // Genuine typed history, no cast.
    operation.source_route = Operation::SourceRoute::prepared_raw_scalar;
    impl_->active = true;
    try {
        operation.raw_call_entered = true; // Source entry status, NOT Origin.
        void* const result = scalar_delete_native_gui_icon_delayed_loads_storage_00ab57c0(
            raw, call.flags_home, call.native_frame, operation.raw_scalar_acquired,
            impl_->strings, impl_->publication, *impl_->actual_feature_word);
        operation.raw_call_returned = true;
        // As with legacy flags0 completion, physical custody stays explicit.
        impl_->registered_identity = nullptr;
        operation.phase = Phase::complete;
        impl_->active = false;
        return result; // Captured address only; no receiver/pointee access.
    } catch (...) {
        operation.failure = impl_->failure = std::current_exception();
        operation.phase = Phase::failed;
        impl_->failed = true;
        impl_->active = false;
        throw; // No Native unwind/cleanup/compensation; retain adopted storage.
    }
}

NativeGuiIconDelayedLoads::NativeGuiIconDelayedLoads(NativeStringRawPoolContext& strings,
    NativeGuiIconDelayedLoadsStorage* volatile& publication)
    : impl_(std::make_unique<Impl>(strings, publication)) {}
NativeGuiIconDelayedLoads::NativeGuiIconDelayedLoads(NativeStringRawPoolContext& strings,
    NativeGuiIconDelayedLoadsStorage* volatile& publication,
    const volatile std::uint32_t& actual_feature_word_0109eea4)
    : impl_(std::make_unique<Impl>(strings, publication, actual_feature_word_0109eea4)) {}
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
