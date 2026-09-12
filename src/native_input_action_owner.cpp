#include "bsp/native_input_action_owner.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native input action ownership requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T read(const void* owner, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(owner) + offset, sizeof value);
    return value;
}
template<class T> void write(void* owner, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(owner) + offset, &value, sizeof value);
}
void* at(void* owner, std::size_t offset) noexcept {
    return static_cast<std::byte*>(owner) + offset;
}

// Native CB6820 -> A92180, also inlined at A93E33. Neither path unregisters,
// compares publication identity, clears vectors, or invokes another owner.
struct BaseCleanup final {
    void* owner;
    void* volatile& publication;
    ~BaseCleanup() noexcept {
        publication = nullptr;
        write<std::uint32_t>(owner, 0, 0x00ce3818u);
    }
};

// Native state1 -> CB6828 -> A93D80. State0 starts BEFORE calling A93C10
// normally, so a failure in that call must not run this cleanup a second time.
struct ActionTableUnwind final {
    void* header;
    NativeInputActionOwnerCalls& records;
    bool armed{true};
    ~ActionTableUnwind() noexcept {
        if (armed) {
            records.call_00a93c10(header, 0);
            singleton_lifetime_free(read<void*>(header, 0));
        }
    }
};
} // namespace

void* construct_native_input_action_owner_00a93da0(void* owner) noexcept {
    write<std::uint32_t>(owner, 0, 0x00d5b630u);
    write<void*>(owner, 4, nullptr);
    write<std::int32_t>(owner, 8, 0);
    write<std::int32_t>(owner, 0x0c, 0);
    write<void*>(owner, 0x10, nullptr);
    write<std::int32_t>(owner, 0x14, 0);
    write<std::int32_t>(owner, 0x18, 0);
    write<std::uint8_t>(owner, 0x1c, 1);
    write<std::uint32_t>(owner, 0x20, 1);
    return owner;
}

void* get_native_input_action_owner_004bec00(NativeInputActionOwnerContext& context) {
    void* const initial = context.publication_00f8bbf8;
    if (initial) return initial;
    {
        CapturedSoundLifetimeSection section(context.lifetime);
        if (!context.publication_00f8bbf8) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x24, 0x24});
            void* result;
            try {
                result = allocation
                    ? construct_native_input_action_owner_00a93da0(allocation)
                    : nullptr;
            } catch (...) {
                // Native state1 ends before the publication at4BEC81.
                // The current constructor has no throwing C++ operations;
                // native hardware faults are outside this source interface.
                singleton_lifetime_free(allocation);
                throw;
            }
            context.publication_00f8bbf8 = result;
            auto current_manager = context.lifetime.get_manager_00415350();
            // Separate statement: do not evaluate this argument before the
            // second getter, or substitute the originally allocated pointer.
            current_manager->register_object(context.publication_00f8bbf8);
        }
    }
    return context.publication_00f8bbf8;
}

void destroy_native_input_action_owner_00a93dd0(
    void* owner, NativeInputActionOwnerContext& context) {
    write<std::uint32_t>(owner, 0, 0x00d5b630u);
    BaseCleanup base{owner, context.publication_00f8bbf8};
    ActionTableUnwind actions{at(owner, 4), context.records};
    void* const dword_header = at(owner, 0x10);
    context.records.call_0086a430(dword_header, 0);
    // A93E08 captures the current base after resize, not before its callbacks.
    singleton_lifetime_free(read<void*>(dword_header, 0));
    actions.armed = false;
    context.records.call_00a93c10(actions.header, 0);
    singleton_lifetime_free(read<void*>(actions.header, 0));
    // base clears the current publication and stamps the original owner.
    // Freed base pointers and capacities remain in its bytes, as natively.
}

void* scalar_delete_native_input_action_owner_00a93e50(
    void* owner, std::uint32_t flags, NativeInputActionOwnerContext& context) {
    destroy_native_input_action_owner_00a93dd0(owner, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
