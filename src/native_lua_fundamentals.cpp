#include "bsp/native_lua_fundamentals.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uintptr_t profile = 0x00d62c18;
constexpr std::uintptr_t base_profile = 0x00ce3818;
}

NativeLuaFundamentalsOwner* construct_native_lua_fundamentals_00b68340(
    void* allocation, NativeLuaFundamentalsContext& context) {
    auto* const owner = ::new (allocation) NativeLuaFundamentalsOwner;
    owner->vtable_00 = profile;
    NativeString path;
    unsigned unwind_state = 0;
    __try {
        path.resize_0041dd40(context.strings, 0x18, true);
        // Fixed native length18h includes no terminator; memcpy copies25 bytes.
        static constexpr char native_path[25] = "Scripts\\fundamentals.lua";
        if (path.data()) std::memcpy(path.data(), native_path, path.length() + 1u);
        void* const manager = context.manager_0109ceec;
        const auto open_table = capture_native_lua_vfs_table(manager);
        unwind_state = 1;
        void* const stream = context.vfs.open(open_table, manager, path, 2);
        if (context.vfs.is_open(capture_native_lua_vfs_table(stream), stream)) {
            owner->size_08 = static_cast<std::uint32_t>(
                context.vfs.length(capture_native_lua_vfs_table(stream), stream));
            auto* const bytes = static_cast<char*>(singleton_lifetime_allocate({
                SingletonAllocationKind::object, owner->size_08, owner->size_08}));
            owner->bytes_04 = bytes;
            std::uint32_t ignored_read_count;
            context.vfs.read(capture_native_lua_vfs_table(stream),
                stream, bytes, owner->size_08, &ignored_read_count);
            if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(
                    static_cast<char*>(stream) + 4)) == 0)
                context.vfs.zero_reference(capture_native_lua_vfs_table(stream), stream);
        }
    } __finally {
        if (unwind_state == 1) destroy_native_string_header_0041dd20(&path, context.strings);
        if (AbnormalTermination())
            destroy_native_lua_fundamentals_base_00b667d0(*owner, context.publication_0108ff1c);
    }
    return owner;
}

namespace {
// Keep the native allocation-unwind state in this inner SEH frame. The caller
// owns the captured lifetime section and releases it before the final reload.
void construct_and_register_fundamentals(NativeLuaFundamentalsContext& context) {
    void* allocation = nullptr;
    unsigned unwind_state = 0;
    __try {
        if (!context.publication_0108ff1c) {
            allocation = singleton_lifetime_allocate({SingletonAllocationKind::object,
                0xc, sizeof(NativeLuaFundamentalsOwner)});
            unwind_state = 1;
            auto* const owner = allocation
                ? construct_native_lua_fundamentals_00b68340(allocation, context) : nullptr;
            unwind_state = 0;
            context.publication_0108ff1c = owner;
            auto registration_manager = context.lifetime.get_manager_00415350();
            registration_manager->register_object(context.publication_0108ff1c);
        }
    } __finally {
        if (unwind_state == 1) singleton_lifetime_free(allocation);
    }
}
} // namespace

NativeLuaFundamentalsOwner* get_native_lua_fundamentals_00884770(
    NativeLuaFundamentalsContext& context) {
    if (auto* const existing = context.publication_0108ff1c) return existing;
    {
        CapturedSoundLifetimeSection captured(context.lifetime);
        construct_and_register_fundamentals(context);
    }
    return context.publication_0108ff1c;
}

const NativeLuaFundamentalsView* native_lua_fundamentals_callback(void* context) {
    return get_native_lua_fundamentals_00884770(*static_cast<NativeLuaFundamentalsContext*>(context));
}

void destroy_native_lua_fundamentals_base_00b667d0(NativeLuaFundamentalsOwner& owner,
    NativeLuaFundamentalsOwner* volatile& publication) noexcept {
    publication = nullptr;
    owner.vtable_00 = base_profile;
}

NativeLuaFundamentalsOwner* delete_native_lua_fundamentals_00b66b80(
    NativeLuaFundamentalsOwner& owner, std::uint32_t flags,
    NativeLuaFundamentalsOwner* volatile& publication) noexcept {
    auto* const original = &owner;
    owner.vtable_00 = profile;
    if (owner.bytes_04) {
        singleton_lifetime_free(const_cast<char*>(owner.bytes_04));
        owner.bytes_04 = nullptr; // B66B99, absent from stored Ghidra listing.
    }
    destroy_native_lua_fundamentals_base_00b667d0(owner, publication);
    if (flags & 1u) {
        owner.~NativeLuaFundamentalsOwner();
        singleton_lifetime_free(original);
    }
    return original;
}

NativeLuaFundamentalsLifetimeBinding::NativeLuaFundamentalsLifetimeBinding(
    NativeLuaFundamentalsOwner* volatile& publication, SingletonLifetimeCallbacks next)
    : publication_(publication), next_(next) {
    if (!next_.destroy_registered || !next_.invalid_parameter)
        throw std::invalid_argument("Fundamentals lifetime binding requires other-owner callbacks");
}
SingletonLifetimeCallbacks NativeLuaFundamentalsLifetimeBinding::callbacks() noexcept {
    return {this, &destroy_registered, &invalid_parameter};
}
void NativeLuaFundamentalsLifetimeBinding::destroy_registered(void* context, void* owner,
    std::uint32_t flags) noexcept {
    auto& binding = *static_cast<NativeLuaFundamentalsLifetimeBinding*>(context);
    std::uintptr_t identity;
    std::memcpy(&identity, owner, sizeof identity);
    if (identity == profile)
        delete_native_lua_fundamentals_00b66b80(*static_cast<NativeLuaFundamentalsOwner*>(owner),
            flags, binding.publication_);
    else binding.next_.destroy_registered(binding.next_.context, owner, flags);
}
void NativeLuaFundamentalsLifetimeBinding::invalid_parameter(void* context) {
    auto& binding = *static_cast<NativeLuaFundamentalsLifetimeBinding*>(context);
    binding.next_.invalid_parameter(binding.next_.context);
}
} // namespace bsp
