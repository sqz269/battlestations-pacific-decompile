#include "bsp/native_online_lifetime.hpp"
#include "bsp/sound_lifetime_access.hpp"
#include "bsp/xlive_ipc.hpp"
#include <cstddef>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
volatile Word& word(void* owner, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(owner) + offset);
}
void* pointer(void* owner, std::size_t offset) noexcept {
    return reinterpret_cast<void*>(word(owner, offset));
}
struct DerivedCleanup final {
    NativeOnlineManagerStorage* owner;
    NativeOnlineLifetimeContext& context;
    bool armed = true;
    ~DerivedCleanup() noexcept {
        if (!armed) return;
        destroy_native_online_achievement_ids_00a3f840(
            reinterpret_cast<std::byte*>(owner) + 0x360);
        try { destroy_native_online_base_00a3f5d0(owner, context); }
        catch (...) { std::terminate(); }
    }
};
} // namespace

NativeOnlineManagerStorage* __fastcall construct_native_online_base_00a3f530(
    NativeOnlineManagerStorage* owner, NativeOnlineLifetimeContext& context) {
    word(owner, 0) = 0x00d24138;
    try {
        const SoundLifetimeAccess lifetime(context.actual_manager_01090aa0);
        CapturedSoundLifetimeSection guard(lifetime);
        context.actual_online_00f8abe8 = owner;
        auto manager = lifetime.get_manager_00415350();
        manager.register_object(context.actual_online_00f8abe8);
    } catch (...) {
        word(owner, 0) = 0x00ce3818;
        throw;
    }
    return owner;
}

void __fastcall destroy_native_online_base_00a3f5d0(
    NativeOnlineManagerStorage* owner, NativeOnlineLifetimeContext& context) {
    word(owner, 0) = 0x00d24138;
    try {
        const SoundLifetimeAccess lifetime(context.actual_manager_01090aa0);
        CapturedSoundLifetimeSection guard(lifetime);
        auto manager = lifetime.get_manager_00415350();
        manager.unregister_object(context.actual_online_00f8abe8);
        context.actual_online_00f8abe8 = nullptr;
    } catch (...) {
        word(owner, 0) = 0x00ce3818;
        throw;
    }
    word(owner, 0) = 0x00ce3818;
}

void __fastcall destroy_native_online_achievement_ids_00a3f840(void* header) {
    void* const allocation = pointer(header, 4);
    if (allocation) singleton_lifetime_free(allocation);
    word(header, 4) = 0;
    word(header, 8) = 0;
    word(header, 12) = 0;
}

void __fastcall close_native_online_ipc_00a4c280(XLiveIpc* ipc) {
    if (ipc && reinterpret_cast<Word>(ipc) != 0xffffffffu)
        destroy_xlive_ipc_00a4bde0(ipc);
}

void __fastcall destroy_native_online_00a3f9d0(
    NativeOnlineManagerStorage* owner, NativeOnlineLifetimeContext& context) {
    word(owner, 0) = 0x00d2413c;
    DerivedCleanup cleanup{owner, context}; // DE9DF8: vector, then base.
    close_native_online_ipc_00a4c280(static_cast<XLiveIpc*>(pointer(owner, 0x3ac)));
    if (word(owner, 0x14c) != 0) {
        void* const allocation = pointer(owner, 0x14c); // Native second load.
        if (allocation) {
            singleton_lifetime_free(allocation);
            word(owner, 0x14c) = 0;
        }
    }
    destroy_native_online_achievement_ids_00a3f840(
        reinterpret_cast<std::byte*>(owner) + 0x360);
    cleanup.armed = false; // Native state -1 before the explicit base call.
    destroy_native_online_base_00a3f5d0(owner, context);
}

NativeOnlineManagerStorage* __fastcall delete_native_online_base_00a3f670(
    NativeOnlineManagerStorage* owner, NativeOnlineLifetimeContext& context, Word flags) {
    destroy_native_online_base_00a3f5d0(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

NativeOnlineManagerStorage* __fastcall delete_native_online_00a3fdc0(
    NativeOnlineManagerStorage* owner, NativeOnlineLifetimeContext& context, Word flags) {
    destroy_native_online_00a3f9d0(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
