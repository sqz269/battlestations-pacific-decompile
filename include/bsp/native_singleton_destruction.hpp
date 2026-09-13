#pragma once

#include <cstdint>

namespace bsp {
struct NativeResourceRegistryDeleteBindings;
class XLiveOwnerAllocation;
struct NativeInputBackendOwnerContext;
struct NativeInputActionOwnerContext;
struct NativeInputSettingsLifetimeContext;
struct NativeLuaFundamentalsView;
struct NativeDebugFeatureOwnerContext;
struct NativeGameResourceFactoryContext;
struct NativeStringPoolStorage;
struct NativePhysicalFactoryContext;
struct NativeVfsManagerLifetimeContext;
struct NativeFileStoreFactoryContext;
struct NativeMpakFactoryContext;
struct NativePakRegistryContext;
class NativeObserverLifetime;
struct NativeObserverDispatchOwner;
namespace game { class GameSoundRuntime; }

// Stable borrowed source bindings. Every nonnull object admitted to the raw
// manager must carry one of these recovered slot-zero profiles: CE3818,
// D0DA64, D5E594, D5E59C, D5B44C, D5B460, D5B478, D58F78, D24138,
// D2413C, D5B5F4, D5B5F8, D5B72C, D5B630, D68200, D68CF8, D68D04 or
// D688B0, CFEA1C, D6418C, CF7E70, CF7E74, CE7548, D190C4, CF81CC, D68B94,
// CFD84C or D62C18. D0DA64
// requires its actual publication cell; registry and sound profiles require
// their concrete borrowed bindings. Sound and XLive owners retain C++ projected
// storage; XLive additionally requires the exact allocation identity. Input
// contexts borrow the same raw manager/publications as construction and must
// outlive drain. Input dispatch does not require current-publication identity:
// native destructors themselves implement its reload/unregister/clear rules.
// D190C4 uses the actual process-static F899E8 publication directly. Owners
// reconstructed against a different publication cell are outside that profile
// binding; no new private pending-lock context or automatic teardown is added.
// CE7548 likewise uses the actual process F878FC mission-entity lock cell;
// its only slot is 004C4890. The adjacent CE754C is a different owner profile.
// Profile identity is read when popped, not cached when registered. An unknown
// profile or missing binding throws a source contract error before dispatch.
// This finite map is not the original process's arbitrary virtual dispatch.
struct NativeSingletonDeletionBindings {
    void* volatile* actual_effect_publication_00f87664{};
    const NativeResourceRegistryDeleteBindings* resource_registry{};
    game::GameSoundRuntime* sound_runtime{};
    XLiveOwnerAllocation* xlive_owner{};
    NativeInputBackendOwnerContext* input_backend{};
    NativeInputActionOwnerContext* input_actions{};
    // Stable actual01090AA8/01090AA4 cells; both required for D68200.
    NativeStringPoolStorage* volatile* actual_string_pool_publication_01090aa8{};
    volatile std::uint32_t* actual_string_returns_disabled_01090aa4{};
    // D68CF8 is the registered factory+4 subobject. Dispatch adjusts -4.
    NativePhysicalFactoryContext* physical_factory{};
    // D68D04 deletes the actual VFS owner through BEDAC0 -> BE1F60.
    // Borrow the construction context and its SAME raw lifetime publication;
    // string release can recreate/register the pool during this drain.
    NativeVfsManagerLifetimeContext* vfs_manager{};
    // D688B0 is the registered FileStore factory+4 secondary; BE5340 adjusts -4.
    NativeFileStoreFactoryContext* filestore_factory{};
    // CFEA1C is the registered MPAK factory+4;735D30 adjusts -4.
    NativeMpakFactoryContext* mpak_factory{};
    // D6418C is the registered PAK registry+8;BB4FF0 adjusts -8.
    NativePakRegistryContext* pak_registry{};
    // CF7E70 deletes the observer lock through00694EA0. This lifetime borrows
    // the same application manager and actualE198E0 publication as construction
    // and must remain alive through drain; the popped owner is passed directly.
    NativeObserverLifetime* observer_lifetime{};
    // CF7E74 deletes the dispatch owner through00695F40. Borrow the same
    // actualE198DC cell used by construction; pass the popped owner even if
    // publication changed. The separateE198E4 alias is deliberately untouched.
    NativeObserverDispatchOwner* volatile* actual_observer_dispatch_owner_00e198dc{};
    // CF81CC deletes actual540h settings through006AB800. Borrow the same
    // raw manager/publication and populated-container services as construction.
    // Pass the popped owner even when its current publication differs.
    NativeInputSettingsLifetimeContext* input_settings{};
    // D68B94 deletes the unadjusted34h debug owner through00BE9600.
    // Borrow its construction context and the SAME actual raw manager/pool;
    // member-string releases may create/register the pool during this drain.
    NativeDebugFeatureOwnerContext* debug_features{};
    // CFD84C is the registered game-resource factory+4 lifetime subobject.
    // 00716520 adjusts -4 before00716560 frees the primary8-byte allocation.
    // Its context owns E19B90; the separate WinMain F8D31C alias is untouched.
    NativeGameResourceFactoryContext* game_resource_factory{};
    // D62C18 deletes the actual0Ch fundamentals cache through00B66B80.
    // Pass the popped owner and clear this same0108FF1C cell unconditionally,
    // including when publication changed after registration.
    NativeLuaFundamentalsView* volatile* actual_lua_fundamentals_publication_0108ff1c{};
};
static_assert(sizeof(NativeSingletonDeletionBindings) == 76);

// Full BD0400[197] normal schedule over raw14h manager storage. Native ECX
// owner, RET; new EDX reference to stable bindings above. Pop before deleting
// with flags1, then reread live count. After drain release actual +10 section,
// free current +4 vector and zero +4/+8/+C. On a C++ exception, native state0
// CC5450's BD0220 storage cleanup is performed and the exception rethrown.
// Caller retains ownership of the manager allocation/publication. Original
// FH3 stack-slot aliases, SEH/hardware-fault and caller ABI are not preserved.
void __fastcall destroy_native_singleton_manager_00bd0400(
    void* owner, const NativeSingletonDeletionBindings& bindings);

// Full00412440[31]: test low flags bit0 before rewriting CE3818; optionally
// free captured owner; EAX captured address bits, RET4. Native ECX owner,
// incoming EDX unused. This is the base profile's actual scalar deleter.
void* __fastcall delete_native_singleton_base_00412440(
    void* owner, void* unused_edx, std::uint32_t flags);

// Full0086B0B0[68], retained STL name: walk raw effect owner's18h-node tree
// with actual checked iterator increment869A20. Label stack slot is unused;
// ECX raw10h owner, incoming EDX unused, RET4. No payload ownership/probe data.
void __fastcall probe_native_gameplay_effect_registry_0086b0b0(
    void* owner, void* unused_edx, const char* label);
} // namespace bsp
