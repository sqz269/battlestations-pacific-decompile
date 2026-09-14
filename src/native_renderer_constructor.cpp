#include "bsp/native_renderer_constructor.hpp"
#include "bsp/native_renderer_base_lifetime.hpp"
#include "bsp/native_render_cache_construction.hpp"
#include "bsp/native_renderer_frame_statistics.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_renderer_lua_owner.hpp"
#include "bsp/native_shader_state_definitions.hpp"
#include "bsp/native_system_constant_registry.hpp"
#include "bsp/native_renderer_gather_capabilities.hpp"
#include "bsp/native_renderer_control_worker.hpp"
#include "bsp/native_renderer_resolution_enumeration.hpp"
#include "bsp/native_renderer_parent_member_cleanup.hpp"
#include "bsp/native_renderer_pointer_array_destroy.hpp"
#include "bsp/native_renderer_record_array_cleanup.hpp"
#include "bsp/native_renderer_remaining_array_cleanup.hpp"
#include "bsp/native_renderer_record_destroy.hpp"
#include "bsp/native_vertex_declaration_registry_lifetime.hpp"
#include "bsp/native_renderer_cache_cleanup.hpp"
#include "bsp/native_effect_registry_destroy.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d9.h>
#include <cstring>
#include <exception>

namespace bsp {
namespace {
using U = std::uint32_t;
using I = std::int32_t;
static_assert(sizeof(void*) == 4 && sizeof(D3DADAPTER_IDENTIFIER9) == 0x44c);
void* at(void* owner, U offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(owner) + offset);
}
volatile U& word(void* owner, U offset) noexcept {
    return *static_cast<volatile U*>(at(owner, offset));
}
volatile std::uint8_t& byte(void* owner, U offset) noexcept {
    return *static_cast<volatile std::uint8_t*>(at(owner, offset));
}
void* allocate(U bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

// Native29-state parent cleanup. No factory, successful singleton-child or
// completed control-worker disposal is present in this map.
void unwind_parent(void* owner, I state, void* captured_child,
    NativeRendererConstructorContext& context, NativeRendererBaseContext& base,
    NativeRendererWorkerLifetimeContext& worker) noexcept {
    if (state >= 25 && state <= 28) {
        singleton_lifetime_free(captured_child); // CBDEBD/CBDECB/CBDED9/CBDEE7
        state = 24;
    }
    switch (state) {
    case 24: destroy_native_renderer_worker_00b5e2f0(at(owner, 0x1d2c), &worker); [[fallthrough]];
    case 23: destroy_native_renderer_records40_00b29c60(at(owner, 0x1d18)); [[fallthrough]];
    case 22: destroy_native_renderer_records24_00b29c20(at(owner, 0x1d0c)); [[fallthrough]];
    case 21: destroy_native_renderer_records20_00b29be0(at(owner, 0x1d00)); [[fallthrough]];
    case 20: destroy_native_renderer_records16_00b29ba0(at(owner, 0x1cf4)); [[fallthrough]];
    case 19: destroy_native_renderer_capabilities_thunk_00b2f700(at(owner, 0x1b18)); [[fallthrough]];
    case 18: destroy_native_renderer_array_00b280f0(at(owner, 0x1b0c)); [[fallthrough]];
    case 17: destroy_native_renderer_array_00737bf0(at(owner, 0x1b00)); [[fallthrough]];
    case 16: destroy_native_renderer_array_00b280d0(at(owner, 0x1af4)); [[fallthrough]];
    case 15: destroy_native_renderer_array_00b280b0(at(owner, 0x1ae8)); [[fallthrough]];
    case 14: destroy_native_renderer_third_state_cache_00b28090(at(owner, 0x1adc)); [[fallthrough]];
    case 13: destroy_native_renderer_pixel_shader_registry_00b29b80(at(owner, 0x1ad0)); [[fallthrough]];
    case 12: destroy_native_renderer_vertex_shader_registry_00b29b60(at(owner, 0x1ac4)); [[fallthrough]];
    case 11: destroy_native_renderer_index_pointers_00b29b40(at(owner, 0x1ab8)); [[fallthrough]];
    case 10: destroy_native_renderer_vertex_pointers_00b29b20(at(owner, 0x1aac)); [[fallthrough]];
    case 9: destroy_native_effect_registry_thunk_00b32200(at(owner, 0x1a98), context.effect_cleanup); [[fallthrough]];
    case 8: destroy_native_texture_resource_cache_00b32370(at(owner, 0x1a74), context.texture_cleanup); [[fallthrough]];
    // B321D0 is the exact five-byte tail JMP to this substantive provider.
    case 7: destroy_native_declaration_registry_00b32030(at(owner, 0x1a60), context.declaration_cleanup); [[fallthrough]];
    case 6: destroy_native_embedded_tracked_section_00402f70(at(owner, 0x19f4)); [[fallthrough]];
    case 5: destroy_native_renderer_index_pointers_00b29b40(at(owner, 0x19c4)); [[fallthrough]];
    case 4: destroy_native_renderer_vertex_pointers_00b29b20(at(owner, 0x19b0)); [[fallthrough]];
    case 3: destroy_native_renderer_query_pointers_00b27f50(at(owner, 0x19a0)); [[fallthrough]];
    case 2: destroy_native_renderer_dword_array_0086ae00(at(owner, 0x28)); [[fallthrough]];
    case 1: destroy_native_renderer_resolution_pairs_008d4e60(at(owner, 0x1c)); [[fallthrough]];
    case 0: destroy_native_renderer_base_00b284e0(owner, base); [[fallthrough]];
    case -1: break;
    default: std::terminate();
    }
}
} // namespace

void* __fastcall construct_native_renderer_00b32410(
    void* owner, NativeRendererConstructorContext* context) {
    NativeRendererBaseContext base{context->current_renderer_00f8d394,
        context->singleton_manager_01090aa0};
    NativeStringRawPoolContext raw_strings{context->string_pool_01090aa8,
        context->small_returns_disabled_01090aa4, context->singleton_manager_01090aa0};
    ActualNativeStringPoolStorage strings(context->string_pool_01090aa8,
        context->small_returns_disabled_01090aa4, context->singleton_manager_01090aa0);
    NativeRendererLuaOwnerContext lua{context->singleton_manager_01090aa0,
        context->lua_owner_00f8d434, strings, context->lua_bootstrap};
    NativeSystemConstantRegistryRawContext system{context->system_constants_0108fe94, raw_strings};
    NativeRendererWorkerLifetimeContext worker{&strings};
    NativeRendererGatherCapabilitiesActualContext capabilities{context->actual_capabilities_scratch,
        context->string_pool_01090aa8, context->small_returns_disabled_01090aa4,
        context->singleton_manager_01090aa0};
    volatile I state = -1;
    void* captured_child = nullptr;
    void* control = nullptr;
    void* api = nullptr;
    TrackedCriticalSection* final_section = nullptr;
    U captured_one = 0;
    bool nvidia = false;
    try {
        construct_native_renderer_base_00b283f0(owner, base); // 00b32434
        word(owner, 0x0) = 0xd5f0a8; // 00b3243b
        word(owner, 0xc) = 0xd5f0a4; // 00b32441
        state = 0; // 00b32448
        word(owner, 0x1c) = 0; // 00b3244f
        word(owner, 0x20) = 0; // 00b32452
        word(owner, 0x24) = 0; // 00b32455
        word(owner, 0x28) = 0; // 00b32458
        word(owner, 0x2c) = 0; // 00b3245b
        word(owner, 0x30) = 0; // 00b3245e
        state = 2; // 00b32464
        construct_native_render_cache_00b29430(at(owner, 0x34), context->live_one_00d7a24c); // 00b3246c
        word(owner, 0x1970) = 0; // 00b32473
        api = ::Direct3DCreate9(0x20); // 00b32479
        word(owner, 0x1990) = reinterpret_cast<U>(api); // 00b3247e
        word(owner, 0x1994) = 0; // 00b32484
        word(owner, 0x1998) = 0; // 00b3248a
        word(owner, 0x199c) = 0; // 00b32490
        word(owner, 0x19a0) = 0; // 00b32496
        word(owner, 0x19a4) = 0; // 00b3249c
        word(owner, 0x19a8) = 0; // 00b324a2
        byte(owner, 0x19ac) = 0; // 00b324a8
        word(owner, 0x19b0) = 0; // 00b324ae
        word(owner, 0x19b4) = 0; // 00b324b4
        word(owner, 0x19b8) = 0; // 00b324ba
        word(owner, 0x19c4) = 0; // 00b324c0
        word(owner, 0x19c8) = 0; // 00b324c6
        word(owner, 0x19cc) = 0; // 00b324cc
        word(owner, 0x19d8) = 0; // 00b324d9
        word(owner, 0x19dc) = 0; // 00b324df
        word(owner, 0x19e0) = 0; // 00b324e5
        word(owner, 0x19e4) = 0; // 00b324eb
        word(owner, 0x19e8) = 0; // 00b324f1
        word(owner, 0x19ec) = 0; // 00b324f7
        word(owner, 0x19f0) = 0; // 00b324fd
        ::InitializeCriticalSection(static_cast<CRITICAL_SECTION*>(at(owner, 0x19f4))); // 00b32503
        word(owner, 0x1a0c) = 0; // 00b32509
        word(owner, 0x1a10) = 0; // 00b3250c
        byte(owner, 0x1a14) = 0; // 00b32512
        word(owner, 0x1a18) = 3; // 00b32518
        byte(owner, 0x1a1c) = 0; // 00b32522
        word(owner, 0x1a20) = 0; // 00b32528
        word(owner, 0x1a24) = 0; // 00b3252e
        word(owner, 0x1a64) = 0; // 00b32534
        word(owner, 0x1a68) = 0; // 00b3253a
        word(owner, 0x1a6c) = 0; // 00b32540
        word(owner, 0x1a70) = 0; // 00b32546
        word(owner, 0x1a60) = 0xd5f060; // 00b3254c
        word(owner, 0x1a78) = 0; // 00b32556
        word(owner, 0x1a7c) = 0; // 00b3255c
        word(owner, 0x1a80) = 0; // 00b32562
        word(owner, 0x1a84) = 0; // 00b32568
        word(owner, 0x1a74) = 0xd5f088; // 00b3256e
        word(owner, 0x1a88) = 0; // 00b32578
        word(owner, 0x1a8c) = 0; // 00b3257e
        byte(owner, 0x1a90) = 0; // 00b32584
        word(owner, 0x1a94) = 0; // 00b3258a
        word(owner, 0x1a9c) = 0; // 00b32590
        word(owner, 0x1aa0) = 0; // 00b32596
        word(owner, 0x1aa4) = 0; // 00b3259c
        word(owner, 0x1aa8) = 0; // 00b325a2
        word(owner, 0x1a98) = 0xd5f074; // 00b325a8
        word(owner, 0x1aac) = 0; // 00b325b2
        word(owner, 0x1ab0) = 0; // 00b325b8
        word(owner, 0x1ab4) = 0; // 00b325be
        word(owner, 0x1ab8) = 0; // 00b325c4
        word(owner, 0x1abc) = 0; // 00b325ca
        word(owner, 0x1ac0) = 0; // 00b325d0
        word(owner, 0x1ac4) = 0; // 00b325d6
        word(owner, 0x1ac8) = 0; // 00b325dc
        word(owner, 0x1acc) = 0; // 00b325e2
        word(owner, 0x1ad0) = 0; // 00b325e8
        word(owner, 0x1ad4) = 0; // 00b325ee
        word(owner, 0x1ad8) = 0; // 00b325f4
        word(owner, 0x1adc) = 0; // 00b325fa
        word(owner, 0x1ae0) = 0; // 00b32600
        word(owner, 0x1ae4) = 0; // 00b32606
        word(owner, 0x1ae8) = 0; // 00b3260c
        word(owner, 0x1aec) = 0; // 00b32612
        word(owner, 0x1af0) = 0; // 00b32618
        word(owner, 0x1af4) = 0; // 00b3261e
        word(owner, 0x1af8) = 0; // 00b32624
        word(owner, 0x1afc) = 0; // 00b3262a
        word(owner, 0x1b00) = 0; // 00b32630
        word(owner, 0x1b04) = 0; // 00b32636
        word(owner, 0x1b08) = 0; // 00b3263c
        word(owner, 0x1b0c) = 0; // 00b32642
        word(owner, 0x1b10) = 0; // 00b32648
        word(owner, 0x1b14) = 0; // 00b3264e
        word(owner, 0x1b5c) = 0; // 00b32657
        word(owner, 0x1b60) = 0; // 00b3265d
        word(owner, 0x1b64) = 0; // 00b32663
        word(owner, 0x1b68) = 0; // 00b32669
        word(owner, 0x1b6c) = 0; // 00b3266f
        word(owner, 0x1b70) = 0; // 00b32675
        word(owner, 0x1b18) = 0; // 00b3267b
        word(owner, 0x1b1c) = 0; // 00b32681
        word(owner, 0x1b24) = 0; // 00b32687
        word(owner, 0x1b28) = 0; // 00b3268d
        word(owner, 0x1b2c) = 0; // 00b32693
        byte(owner, 0x1b30) = 0; // 00b32699
        word(owner, 0x1b20) = 0; // 00b3269f
        word(owner, 0x1b40) = 0; // 00b326a5
        word(owner, 0x1b44) = 0; // 00b326ab
        byte(owner, 0x1b50) = 0; // 00b326b1
        word(owner, 0x1b34) = 0; // 00b326b7
        word(owner, 0x1b38) = 0; // 00b326bf
        captured_one = context->live_one_00d7a24c; // 00b326c7
        word(owner, 0x1b3c) = captured_one; // 00b326cf
        word(owner, 0x1b4c) = 0; // 00b326d7
        byte(owner, 0x1b51) = 0; // 00b326dd
        byte(owner, 0x1b53) = 0; // 00b326e3
        byte(owner, 0x1b54) = 0; // 00b326e9
        byte(owner, 0x1b55) = 0; // 00b326ef
        byte(owner, 0x1b74) = 0; // 00b326f5
        state = 19; // 00b32701
        construct_native_renderer_frame_statistics_00b0cce0(at(owner, 0x1b78)); // 00b32709
        word(owner, 0x1cf4) = 0; // 00b3270e
        word(owner, 0x1cf8) = 0; // 00b32714
        word(owner, 0x1cfc) = 0; // 00b3271a
        word(owner, 0x1d00) = 0; // 00b32720
        word(owner, 0x1d04) = 0; // 00b32726
        word(owner, 0x1d08) = 0; // 00b3272c
        word(owner, 0x1d0c) = 0; // 00b32732
        word(owner, 0x1d10) = 0; // 00b32738
        word(owner, 0x1d14) = 0; // 00b3273e
        word(owner, 0x1d18) = 0; // 00b32744
        word(owner, 0x1d1c) = 0; // 00b3274a
        word(owner, 0x1d20) = 0; // 00b32750
        state = 23; // 00b3275c
        construct_native_renderer_worker_00b5e270(at(owner, 0x1d2c), &worker); // 00b32764
        word(owner, 0x1d84) = 0; // 00b32769
        word(owner, 0x1d80) = 1; // 00b32774
        byte(owner, 0x1d8a) = 1; // 00b3277a
        byte(owner, 0x1d8b) = 0; // 00b32780
        byte(owner, 0x1d8c) = 0; // 00b32786
        word(owner, 0x1d90) = 0; // 00b3278c
        word(owner, 0x197c) = 0; // 00b32792
        word(owner, 0x1980) = 0; // 00b32798
        word(owner, 0x1984) = 0; // 00b3279e
        word(owner, 0x1988) = 0; // 00b327a4
        state = 24; // 00b327af
        context->frame_flag_0108d4b8 = 0; // 00b327b7
        context->device_lost_0108d4b9 = 0; // 00b327bd
        word(owner, 0x198c) = 0; // 00b327c3
        word(owner, 0x1974) = 0; // 00b327c9
        word(owner, 0x1978) = 0; // 00b327cf
        word(owner, 0x1d24) = 0; // 00b327d5
        word(owner, 0x1d28) = 0; // 00b327db
        captured_child = allocate(0x4cc); // 00b327e1
        state = 25; // 00b327ef
        if (captured_child) construct_native_renderer_lua_owner_00b1bb90(captured_child, lua); // 00b327fb
        state = 24; // 00b32802
        captured_child = allocate(0x2c); // 00b3280a
        state = 26; // 00b32818
        if (captured_child) construct_native_shader_state_definitions_00b585a0(captured_child, context->state_definitions_0108fe90, SoundLifetimeAccess(context->singleton_manager_01090aa0), strings); // 00b32824
        state = 24; // 00b3282b
        captured_child = allocate(0x10); // 00b32833
        state = 27; // 00b32841
        if (captured_child) construct_native_system_constant_registry_00b5bf70(captured_child, system); // 00b3284d
        state = 24; // 00b32854
        enumerate_native_renderer_resolutions_00b27d80(owner, context->actual_resolution_mode_scratch); // 00b3285c
        static_cast<void>(reinterpret_cast<IDirect3D9*>(word(owner, 0x1990))->GetAdapterIdentifier(0, 0, static_cast<D3DADAPTER_IDENTIFIER9*>(context->actual_identifier_scratch))); // 00b32874
        nvidia = std::strstr(static_cast<const char*>(context->actual_identifier_scratch) + 0x200, "NVIDIA") != nullptr; // 00b32883
        byte(owner, 0x1d88) = nvidia ? 1u : 0u; // 00b32892
        word(owner, 0x14) = 0; // 00b32898
        gather_native_renderer_capabilities_00b2c8e0(owner, &capabilities); // 00b3289b
        captured_child = allocate(0x24); // 00b328a2
        state = 28; // 00b328b0
        if (captured_child) control = construct_native_renderer_control_worker_00b33da0(captured_child); // 00b328bc
        state = 24; // 00b328c5
        word(owner, 0x1970) = reinterpret_cast<U>(control); // 00b328cd
        final_section = create_native_tracked_critical_section_00bd1860(); // 00b328d3
        word(owner, 0x199c) = reinterpret_cast<U>(final_section); // 00b328df

    } catch (...) {
        unwind_parent(owner, state, captured_child, *context, base, worker);
        throw;
    }
    return owner;
}
} // namespace bsp
