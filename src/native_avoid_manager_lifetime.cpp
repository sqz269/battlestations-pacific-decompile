#include "bsp/native_avoid_manager_lifetime.hpp"

#include "bsp/native_avoid_manager_clear.hpp"
#include "bsp/native_hull_list_head.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstring>
#include <intrin.h>

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(Cc13AvoidZoneManagerNative) == 0x78);
static_assert(offsetof(Cc13AvoidZoneManagerNative, critical_section) == 0x04);
static_assert(offsetof(Cc13AvoidZoneManagerNative, group_slots) == 0x08);
static_assert(offsetof(Cc13AvoidZoneManagerNative, group_count) == 0x58);
static_assert(offsetof(Cc13AvoidZoneManagerNative, vector_first) == 0x60);
static_assert(offsetof(Cc13AvoidZoneManagerNative, vector_last) == 0x64);
static_assert(offsetof(Cc13AvoidZoneManagerNative, vector_end) == 0x68);
static_assert(offsetof(Cc13AvoidZoneManagerNative, list_head) == 0x70);
static_assert(offsetof(Cc13AvoidZoneManagerNative, list_size) == 0x74);

namespace {

// These compiler barriers retain the observed DWORD access order and reloads
// over actual raw storage; memcpy avoids an extra C++ aliasing assumption.
std::uint32_t load_word(const void* storage, std::size_t offset) noexcept {
    _ReadWriteBarrier();
    std::uint32_t result;
    std::memcpy(&result, static_cast<const unsigned char*>(storage) + offset, sizeof(result));
    _ReadWriteBarrier();
    return result;
}
void store_word(void* storage, std::size_t offset, std::uint32_t value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(static_cast<unsigned char*>(storage) + offset, &value, sizeof(value));
    _ReadWriteBarrier();
}
void* list_header(Cc13AvoidZoneManagerNative* manager) noexcept {
    return reinterpret_cast<unsigned char*>(manager) + 0x6c;
}

void* allocate_current_hull(void*, std::size_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
void release_current_hull(void*, void* block) {
    singleton_lifetime_free(block);
}
__declspec(noinline) void __fastcall destroy_current_hull(AvoidZoneDynHullHandle* handle) {
    const AvoidZoneDynHullMemory memory{nullptr, allocate_current_hull, release_current_hull};
    avoid_zone_dyn_hull_destroy_00c37450(*handle, memory);
}

// Fixed concrete calls, not injectable providers. In particular, a local
// volatile clear entry stops MSVC from inferring that the visible naked body
// cannot throw and erasing a required current C++ guard phase.
void invoke_current_list_clear(void* header) {
    using Entry = void (__fastcall*)(void*);
    Entry volatile entry = clear_native_avoid_hull_list_0041e200;
    entry(header);
}

void free_current_list_head(Cc13AvoidZoneManagerNative* manager) noexcept {
    singleton_lifetime_free(reinterpret_cast<void*>(load_word(manager, 0x70)));
    store_word(manager, 0x70, 0);
}
void tidy_current_hull_list(Cc13AvoidZoneManagerNative* manager) {
    invoke_current_list_clear(list_header(manager));
    free_current_list_head(manager);
}
void tidy_current_body_vector(Cc13AvoidZoneManagerNative* manager) noexcept {
    void* const first = reinterpret_cast<void*>(load_word(manager, 0x60));
    if (first != nullptr) singleton_lifetime_free(first);
    store_word(manager, 0x60, 0);
    store_word(manager, 0x64, 0);
    store_word(manager, 0x68, 0);
}
void reset_current_manager_base(Cc13AvoidZoneManagerNative* manager,
    const NativeAvoidManagerLifetimeContext* context) noexcept {
    store_word(context->actual_00e17624_publication_cell, 0, 0);
    // Actual recovered base profile identity; deliberately no fabricated
    // Source callable base vtable is substituted for this raw destructed state.
    store_word(manager, 0, 0x00ce3818);
}

struct CurrentManagerUnwindGuard {
    Cc13AvoidZoneManagerNative* manager;
    const NativeAvoidManagerLifetimeContext* context;
    volatile std::int32_t state = -1;

    ~CurrentManagerUnwindGuard() noexcept {
        if (state == 2) {
            state = 1;
            tidy_current_hull_list(manager);
        }
        if (state == 1) {
            state = 0;
            tidy_current_body_vector(manager);
        }
        if (state == 0) {
            state = -1;
            reset_current_manager_base(manager, context);
        }
    }
};

} // namespace

const NativeAvoidManagerLifetimeSourceVtable kNativeAvoidManagerLifetimeSourceVtable{
    scalar_delete_native_avoid_manager_00420680};

__declspec(naked) void __fastcall clear_native_avoid_hull_list_0041e200(void*) {
    __asm {
        push esi
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi + 4]
        mov esi, dword ptr [eax]
        mov dword ptr [eax], eax
        mov eax, dword ptr [edi + 4]
        mov dword ptr [eax + 4], eax
        cmp esi, dword ptr [edi + 4]
        mov dword ptr [edi + 8], 0
        je done
        push ebx
        mov edi, edi
    next_node:
        mov ebx, dword ptr [esi]
        lea ecx, [esi + 8]
        call destroy_current_hull
        push esi
        call singleton_lifetime_free
        add esp, 4
        cmp ebx, dword ptr [edi + 4]
        mov esi, ebx
        jne next_node
        pop ebx
    done:
        pop edi
        pop esi
        ret
    }
}

__declspec(noinline) Cc13AvoidZoneManagerNative* __fastcall
construct_native_avoid_manager_00421500(Cc13AvoidZoneManagerNative* manager,
    const NativeAvoidManagerLifetimeContext* context) {
    CurrentManagerUnwindGuard cleanup{manager, context};
    store_word(manager, 0, reinterpret_cast<std::uint32_t>(
        &kNativeAvoidManagerLifetimeSourceVtable));
    cleanup.state = 0;
    store_word(manager, 0x60, 0);
    store_word(manager, 0x64, 0);
    store_word(manager, 0x68, 0);
    cleanup.state = 1;
    using BuyHead = Cc13AvoidZoneHullListNode* (__cdecl*)();
    BuyHead volatile buy_head = buy_native_hull_list_head_0041bf50;
    auto* const head = buy_head();
    store_word(manager, 0x70, reinterpret_cast<std::uint32_t>(head));
    store_word(manager, 0x74, 0);
    store_word(manager, 0x58, 0);
    for (std::uint32_t index = 0; index < 19; ++index) {
        store_word(manager, 0x08 + index * 4, 0);
    }
    cleanup.state = 2;
    store_word(manager, 0x54, 0);
    using CreateSection = TrackedCriticalSection* (__cdecl*)();
    CreateSection volatile create_section = create_native_tracked_critical_section_00bd1860;
    auto* const section = create_section();
    store_word(manager, 0x04, reinterpret_cast<std::uint32_t>(section));
    cleanup.state = -1;
    return manager;
}

__declspec(noinline) void __fastcall destroy_native_avoid_manager_0041f980(
    Cc13AvoidZoneManagerNative* manager, const NativeAvoidManagerLifetimeContext* context) {
    CurrentManagerUnwindGuard cleanup{manager, context};
    store_word(manager, 0, reinterpret_cast<std::uint32_t>(
        &kNativeAvoidManagerLifetimeSourceVtable));
    cleanup.state = 2;
    using ClearManager = void (__fastcall*)(Cc13AvoidZoneManagerNative*, void* const*);
    ClearManager volatile clear_manager = clear_native_avoid_manager_0041f600;
    clear_manager(manager, context->actual_00e188a8_publication_cell);
    using ReleaseSection = void (__fastcall*)(TrackedCriticalSection**);
    ReleaseSection volatile release_section = release_native_tracked_critical_section_0041cc80;
    release_section(reinterpret_cast<TrackedCriticalSection**>(
        reinterpret_cast<unsigned char*>(manager) + 0x04));
    // Native dismisses list cleanup BEFORE the normal list clear/head free.
    // The vector/base cleanup remains active through the entire normal tail.
    cleanup.state = 1;
    invoke_current_list_clear(list_header(manager));
    free_current_list_head(manager);
    tidy_current_body_vector(manager);
    reset_current_manager_base(manager, context);
    cleanup.state = -1;
}

__declspec(naked) Cc13AvoidZoneManagerNative* __fastcall
scalar_delete_native_avoid_manager_00420680(
    Cc13AvoidZoneManagerNative*, const NativeAvoidManagerLifetimeContext*, std::uint32_t) {
    __asm {
        push esi
        mov esi, ecx
        call destroy_native_avoid_manager_0041f980
        test byte ptr [esp + 8], 1
        jz retained
        push esi
        call singleton_lifetime_free
        add esp, 4
    retained:
        mov eax, esi
        pop esi
        ret 4
    }
}

} // namespace bsp
