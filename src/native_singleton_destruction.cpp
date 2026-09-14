#include "bsp/native_singleton_destruction.hpp"
#include "bsp/observer_lifetime.hpp"
#include "bsp/observer_dispatch_owner.hpp"
#include "bsp/native_pending_entity_lock.hpp"
#include "bsp/native_mission_entity_lock.hpp"
#include "bsp/game_sound_runtime.hpp"
#include "bsp/native_input_action_owner.hpp"
#include "bsp/native_input_settings_lifetime.hpp"
#include "bsp/native_lua_fundamentals.hpp"
#include "bsp/native_debug_feature_owner.hpp"
#include "bsp/native_game_resource_factory.hpp"
#include "bsp/native_input_backend_owner.hpp"
#include "bsp/native_physical_factory.hpp"
#include "bsp/native_physical_stream_open.hpp"
#include "bsp/native_render_batch_lifetime.hpp"
#include "bsp/light_type_bootstrap.hpp"
#include "bsp/native_filestore_factory.hpp"
#include "bsp/native_mpak_factory.hpp"
#include "bsp/native_mpkg_provider.hpp"
#include "bsp/native_pak_registry.hpp"
#include "bsp/native_vfs_derived_manager.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/xlive_owner_lifetime.hpp"

#include "bsp/native_gameplay_effect_destruction.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/native_resource_registry_scalar_delete.hpp"
#include "bsp/native_resource_manager_lifetime.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdlib>
#include <stdexcept>

namespace bsp {
namespace {
__declspec(naked) void __cdecl current_crt_invalid_parameter() {
    __asm { jmp _invalid_parameter_noinfo }
}

// Concrete recovered slot-zero targets. No original numeric address is
// invoked as a source vtable and no generic destructor callback is supplied.
__declspec(noinline) void __fastcall delete_current_profile(void* owner,
    const NativeSingletonDeletionBindings& bindings, std::uint32_t profile,
    std::uint32_t flags) {
    switch (profile) {
    case 0x00d63128:
    case 0x00d63084: case 0x00d63094: case 0x00d630a4:
    case 0x00d630b4: case 0x00d630c4: case 0x00d630d4:
        if (bindings.resource_manager != nullptr) {
            delete_native_resource_registered_owner(profile, owner, flags, *bindings.resource_manager);
            return;
        }
        break;
    case 0x00ce3818:
        delete_native_singleton_base_00412440(owner, nullptr, flags);
        return;
    case 0x00d0da64:
        if (bindings.actual_effect_publication_00f87664 != nullptr) {
            delete_native_gameplay_effect_manager_008703e0(
                owner, *bindings.actual_effect_publication_00f87664, flags);
            return;
        }
        break;
    case 0x00d5e594:
        if (bindings.resource_registry != nullptr) {
            delete_native_resource_registry_00b1b660(owner, bindings.resource_registry, flags);
            return;
        }
        break;
    case 0x00d5e59c:
        if (bindings.resource_registry != nullptr) {
            delete_native_resource_registry_00b1b710(owner, bindings.resource_registry, flags);
            return;
        }
        break;
    case 0x00d5b44c:
    case 0x00d5b460:
    case 0x00d5b478:
    case 0x00d58f78:
        if (bindings.sound_runtime != nullptr && bindings.sound_runtime->owns_registered(owner)) {
            bindings.sound_runtime->delete_registered(owner, flags);
            return;
        }
        break;
    case 0x00d24138:
        if (bindings.xlive_owner != nullptr && bindings.xlive_owner->owns_identity(owner)) {
            delete_xlive_manager_base_00a3f670(*bindings.xlive_owner,
                static_cast<std::uint8_t>(flags));
            return;
        }
        break;
    case 0x00d2413c:
        if (bindings.xlive_owner != nullptr && bindings.xlive_owner->owns_identity(owner)) {
            delete_xlive_manager_00a3fdc0(*bindings.xlive_owner,
                static_cast<std::uint8_t>(flags));
            return;
        }
        break;
    case 0x00d5b5f4:
        if (bindings.input_backend != nullptr) {
            scalar_delete_native_input_backend_base_00a909e0(owner,
                static_cast<std::uint8_t>(flags), *bindings.input_backend);
            return;
        }
        break;
    case 0x00d5b5f8:
        if (bindings.input_backend != nullptr) {
            scalar_delete_native_input_backend_groups_00a91150(owner,
                static_cast<std::uint8_t>(flags), *bindings.input_backend);
            return;
        }
        break;
    case 0x00d5b72c:
        if (bindings.input_backend != nullptr) {
            scalar_delete_native_input_backend_00a97c00(owner,
                static_cast<std::uint8_t>(flags), *bindings.input_backend);
            return;
        }
        break;
    case 0x00d5b630:
        if (bindings.input_actions != nullptr) {
            scalar_delete_native_input_action_owner_00a93e50(owner, flags,
                *bindings.input_actions);
            return;
        }
        break;
    case 0x00cf81cc:
        if (bindings.input_settings != nullptr) {
            scalar_delete_native_input_settings_006ab800(owner, flags,
                *bindings.input_settings);
            return;
        }
        break;
    case 0x00d62c18:
        if (bindings.actual_lua_fundamentals_publication_0108ff1c != nullptr) {
            delete_native_lua_fundamentals_00b66b80(
                *static_cast<NativeLuaFundamentalsOwner*>(owner), flags,
                *bindings.actual_lua_fundamentals_publication_0108ff1c);
            return;
        }
        break;
    case 0x00d68ec0:
        if (bindings.physical_stream_pool != nullptr) {
            delete_native_physical_stream_pool_00bf4370(owner, flags,
                *bindings.physical_stream_pool);
            return;
        }
        break;
    case 0x00d5e5dc:
        if (bindings.render_batch_lifetime != nullptr) {
            bindings.render_batch_lifetime->delete_pool_00b1e930(
                static_cast<NativeRenderBatchPoolStorage*>(owner), flags);
            return;
        }
        break;
    case 0x00d5e5d4:
        if (bindings.render_batch_lifetime != nullptr) {
            bindings.render_batch_lifetime->delete_lock_owner_00b1d530(
                static_cast<NativeRenderBatchLockOwner*>(owner), flags);
            return;
        }
        break;
    case 0x00cfea10:
        if (bindings.mpkg_factory != nullptr) {
            delete_native_mpkg_factory_secondary_00735d00(owner, flags,
                *bindings.mpkg_factory);
            return;
        }
        break;
    case 0x00cfb6c4:
        if (bindings.type_id_counter_lifetime != nullptr) {
            bindings.type_id_counter_lifetime->deleting_destructor_006fad40(
                static_cast<TypeIdCounterStorage*>(owner), flags);
            return;
        }
        break;
    case 0x00d68200:
        if (bindings.actual_string_pool_publication_01090aa8 != nullptr &&
            bindings.actual_string_returns_disabled_01090aa4 != nullptr) {
            delete_native_string_pool_00bd1730(*static_cast<NativeStringPoolStorage*>(owner),
                flags, *bindings.actual_string_pool_publication_01090aa8,
                *bindings.actual_string_returns_disabled_01090aa4);
            return;
        }
        break;
    case 0x00d68cf8:
        if (bindings.physical_factory != nullptr) {
            delete_native_physical_factory_secondary_00bed910(owner, flags,
                *bindings.physical_factory);
            return;
        }
        break;
    case 0x00d68d04:
        if (bindings.vfs_manager != nullptr) {
            delete_native_vfs_derived_manager_00bedac0(owner, flags,
                *bindings.vfs_manager);
            return;
        }
        break;
    case 0x00d688b0:
        if (bindings.filestore_factory != nullptr) {
            delete_native_filestore_factory_secondary_00be5340(owner, flags,
                *bindings.filestore_factory);
            return;
        }
        break;
    case 0x00cfea1c:
        if (bindings.mpak_factory != nullptr) {
            delete_native_mpak_factory_secondary_00735d30(owner, flags,
                *bindings.mpak_factory);
            return;
        }
        break;
    case 0x00cf7e70:
        if (bindings.observer_lifetime != nullptr) {
            bindings.observer_lifetime->delete_lock_owner_00694ea0(
                static_cast<NativeObserverLockOwner*>(owner), flags);
            return;
        }
        break;
    case 0x00ce7548:
        // CE7548 has only slot0=4C4890. CE754C begins another profile.
        // Delete the popped owner and clear the actual F878FC publication,
        // even if that publication has changed since registration.
        delete_native_mission_entity_lock_004c4890(
            static_cast<NativeMissionEntityLockOwner*>(owner), flags,
            process_native_mission_entity_lock_00f878fc());
        return;
    case 0x00d190c4:
        // This profile belongs to the actual process F899E8 owner. Pass the
        // popped allocation even when publication differs: native teardown
        // clears that process cell unconditionally. D190C8 is another profile.
        delete_native_pending_entity_lock_009256d0(
            static_cast<NativePendingEntityLockOwner*>(owner), flags,
            process_native_pending_entity_lock_00f899e8());
        return;
    case 0x00cf7e74:
        if (bindings.actual_observer_dispatch_owner_00e198dc != nullptr) {
            delete_observer_dispatch_owner_00695f40(
                static_cast<NativeObserverDispatchOwner*>(owner), flags,
                *bindings.actual_observer_dispatch_owner_00e198dc);
            return;
        }
        break;
    case 0x00d6418c:
        if (bindings.pak_registry != nullptr) {
            delete_native_pak_registry_secondary_00bb4ff0(owner, flags,
                *bindings.pak_registry);
            return;
        }
        break;
    case 0x00d68b94:
        if (bindings.debug_features != nullptr) {
            delete_native_debug_feature_owner_00be9600(owner, flags,
                *bindings.debug_features);
            return;
        }
        break;
    case 0x00cfd84c:
        if (bindings.game_resource_factory != nullptr) {
            delete_native_game_resource_factory_secondary_00716520(owner, flags,
                *bindings.game_resource_factory);
            return;
        }
        break;
    }
    throw std::logic_error("raw singleton deletion requires a recovered profile and its actual bindings");
}

__declspec(naked) void __fastcall destroy_manager_body(
    void*, const NativeSingletonDeletionBindings&) {
    __asm {
        // Replace native FH3 registration/scratch with24bytes of padding.
        // The source outer scope owns EH. Its stable binding replaces the
        // native unwind-only owner spill; normal iterator scratch stays +14h.
        sub esp, 18h
        push ebx
        push esi
        mov esi, ecx
        mov dword ptr [esp + 8], edx
        xor ebx, ebx
        call count_native_singleton_slots_00bcf910
        test eax, eax
        jz drained
        push edi
    next_owner:
        mov edi, dword ptr [esi + 8]
        cmp dword ptr [esi + 4], edi
        jbe end_valid
        call current_crt_invalid_parameter
    end_valid:
        lea eax, [edi - 4]
        cmp eax, dword ptr [esi + 8]
        mov dword ptr [esp + 14h], edi
        ja invalid_last
        cmp eax, dword ptr [esi + 4]
        jae last_valid
    invalid_last:
        call current_crt_invalid_parameter
    last_valid:
        add edi, -4
        cmp edi, dword ptr [esi + 8]
        jb slot_valid
        call current_crt_invalid_parameter
    slot_valid:
        mov eax, dword ptr [esi + 4]
        cmp eax, ebx
        mov ecx, dword ptr [edi]
        je popped
        mov edx, dword ptr [esi + 8]
        mov edi, edx
        sub edi, eax
        sar edi, 2
        je popped
        add edx, -4
        mov dword ptr [esi + 8], edx
    popped:
        cmp ecx, ebx
        je recount
        mov eax, dword ptr [ecx]
        push 1
        push eax
        // +8 binding spill +4 savedEDI +8 source arguments = +14h.
        mov edx, dword ptr [esp + 14h]
        call delete_current_profile
    recount:
        mov ecx, esi
        call count_native_singleton_slots_00bcf910
        test eax, eax
        jne next_owner
        pop edi
    drained:
        lea ecx, [esi + 10h]
        call release_native_tracked_critical_section_0041cc80
        mov eax, dword ptr [esi + 4]
        cmp eax, ebx
        je storage_released
        push eax
        call singleton_lifetime_free
        add esp, 4
    storage_released:
        mov dword ptr [esi + 4], ebx
        mov dword ptr [esi + 8], ebx
        mov dword ptr [esi + 0ch], ebx
        pop esi
        pop ebx
        add esp, 18h
        ret
    }
}
} // namespace

__declspec(noinline) void __fastcall destroy_native_singleton_manager_00bd0400(
    void* owner, const NativeSingletonDeletionBindings& bindings) {
    try {
        destroy_manager_body(owner, bindings);
    } catch (...) {
        // Native FuncInfoDFF490: state0 -> -1, CC5450 -> BD0220.
        clear_native_singleton_storage_00bd0220(owner, nullptr);
        throw;
    }
}

__declspec(naked) void* __fastcall delete_native_singleton_base_00412440(
    void*, void*, std::uint32_t) {
    __asm {
        test byte ptr [esp + 4], 1
        push esi
        mov esi, ecx
        mov dword ptr [esi], 00ce3818h
        je keep_owner
        push esi
        call singleton_lifetime_free
        add esp, 4
    keep_owner:
        mov eax, esi
        pop esi
        ret 4
    }
}

__declspec(naked) void __fastcall probe_native_gameplay_effect_registry_0086b0b0(
    void*, void*, const char*) {
    __asm {
        sub esp, 8
        mov eax, dword ptr [ecx + 8]
        push esi
        lea esi, [ecx + 4]
        mov ecx, dword ptr [eax]
        mov eax, esi
        push edi
        mov dword ptr [esp + 0ch], ecx
        mov dword ptr [esp + 8], eax
    next:
        test eax, eax
        mov edi, dword ptr [esi + 4]
        je invalid_iterator
        cmp eax, esi
        je compare_end
    invalid_iterator:
        call current_crt_invalid_parameter
    compare_end:
        cmp dword ptr [esp + 0ch], edi
        je done
        lea ecx, [esp + 8]
        call increment_native_int_pointer_tree18_00869a20
        mov eax, dword ptr [esp + 8]
        jmp next
    done:
        pop edi
        pop esi
        add esp, 8
        ret 4
    }
}
} // namespace bsp
