#pragma once

#include "bsp/d3d9_texture.hpp"
#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_texture_2d_owner.hpp"
#include "bsp/native_render_resource_record.hpp"
#include <memory>

namespace bsp {
class ActualNativeStringPoolStorage;
class NativeVfsRuntimeBindings;
struct NativeVfsOpenRouteContext;
struct NativeVfsDateRouteContext;
struct NativeStoredStreamConversionContext;
struct ResourceLoadEventHost;
struct NativeTextureCacheContext;

// The integrator's concrete holder owns ONE actual BDF4C0 acquired operation.
// It must not borrow a shared scratch acquired slot. The factory only creates
// host metadata; it must not execute native work. Publication precedes capture
// of the current manager and the first invocation of this retained child.
class NativeTextureNameResolutionOperation {
public:
    virtual ~NativeTextureNameResolutionOperation() = default;
    virtual bool resolve(void* captured_manager, void* actual_mutable_header) = 0;
};
using MakeNativeTextureNameResolutionOperation =
    std::unique_ptr<NativeTextureNameResolutionOperation> (*)(void*);
struct NativeTextureNameResolutionCall final {
    std::unique_ptr<NativeTextureNameResolutionOperation> operation;
    void* manager{};
    void* name{};
    bool factory_started{}, started{}, returned{}, failed{}, result{};
    bool invoke(void* actual_mutable_header, void* const volatile& current_manager,
        void* factory_context, MakeNativeTextureNameResolutionOperation);
};

// Address-stable actual8h header plus host diagnostic state. Native cleanup
// releases its current (or separately captured) buffer and LEAVES the header
// bytes intact. After released=true those bytes are consumed preimages, not
// readable string ownership and never a replayable input.
struct NativeTextureCacheName final {
    NativeString value;
    bool initialized{}, cleanup_armed{}, released{};
    char* released_data{};
    std::uint32_t released_size{};
    void initialize();
    void destroy_current(ActualNativeStringPoolStorage&) noexcept;
    void destroy_captured(char*, ActualNativeStringPoolStorage&) noexcept;
};
struct NativeTextureCacheNames final {
    NativeTextureCacheName wrapper, normalized, resolved, resolver_output;
};

// A retained invocation, never an extra resource owner/count. Native locals
// and acquired identities survive host-provider failure; no destructor rolls
// back native work. Resolve a failed frame externally before destroying it.
struct NativeTextureLoadAcquired final {
    enum class Phase { not_started, running, complete, failed };
    ~NativeTextureLoadAcquired();
    NativeTextureLoadAcquired() = default;
    NativeTextureLoadAcquired(const NativeTextureLoadAcquired&) = delete;
    NativeTextureLoadAcquired& operator=(const NativeTextureLoadAcquired&) = delete;
    Phase phase{Phase::not_started};
    std::uint32_t native_site{};
    NativeString name;
    bool name_retained{};
    void* source{};
    void* memory{};
    bool source_release_started{};
    bool memory_release_started{};
    IDirect3DDevice9* captured_device{};
    IDirect3DTexture9* texture{};
    HRESULT last_hresult{};
    std::uint32_t resource_type{};
    void* raw_slot{};
    void* creator{};
    bool constructor_complete{};
    bool source_assigned{};
    bool callback_started{};
    RenderCommandReference* companion{};
    void* owner_record{};
    bool registered{};
};

// Stable companions use the application's SAME canonical registration and
// actual +04 counter. A creator is published before companion registration;
// bind failure does not release it. Destruction runs actual B3F590, then
// unbinds and retires that single companion. No second cache or owner map.
class NativeTextureLoadOwners final {
public:
    NativeTextureLoadOwners(GuiNativeGeometryOwners&, NativeTexture2DOwnerContext&,
        const volatile std::uint32_t* profile_00d61948);
    ~NativeTextureLoadOwners();
    NativeTextureLoadOwners(const NativeTextureLoadOwners&) = delete;
    NativeTextureLoadOwners& operator=(const NativeTextureLoadOwners&) = delete;
    void register_completed_creator(NativeTextureLoadAcquired&);
    NativeTexture2DOwnerContext& texture_context() noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

struct NativeTextureLoadingContext {
    ActualNativeStringPoolStorage& strings;
    NativeTextureLoadOwners& owners;
    NativeVfsRuntimeBindings& streams;
    NativeVfsOpenRouteContext& opens;
    NativeStoredStreamConversionContext& conversion;
    void* const volatile& current_vfs_0109ceec;
    const void* volatile& current_renderer_00f8d394;
    NativeRendererSynchronizationGlobals& synchronization_0108d6dc;
    ReadImageInfoFromMemory image_info_00c2dfec;
    CreateTextureFromMemory create_texture_00c2dfe6;
    void* resolution_context;
    MakeNativeTextureNameResolutionOperation make_resolution_operation;
    const char* empty_string_0108d5a4;
    // Required only on the observed retry/nonzero callback paths. These are
    // concrete body bindings, never numeric addresses called as host code.
    void* callback_context;
    void (*retry_device_00b29670)(void*, const void* current_renderer);
    void (*post_load_callback)(void*, std::uint32_t captured_target, void* texture);
    NativeTextureCacheContext* cache{};
};

struct NativeTextureCacheContext {
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& validation;
    NativeTextureLoadingContext& textures;
    NativeVfsDateRouteContext& dates;
    ResourceLoadEventHost* const volatile& platform_0109cf04;
    NativeRendererSynchronizationGlobals& synchronization_0108d6dc;
    const volatile std::uint32_t* registry_vtable_00d5f088;
    const NativeRenderResourceAccountingTables& accounting_tables;
};

struct NativeTextureCacheAcquired final {
    enum class Phase { not_started, running, complete, failed };
    NativeTextureCacheAcquired();
    ~NativeTextureCacheAcquired();
    NativeTextureCacheAcquired(const NativeTextureCacheAcquired&) = delete;
    NativeTextureCacheAcquired& operator=(const NativeTextureCacheAcquired&) = delete;
    Phase phase{Phase::not_started};
    std::uint32_t native_site{};
    // These precede child members so their storage also survives child teardown.
    NativeTextureCacheNames names;
    NativeRenderResourceRecord pending_record; // Native +08/+28 start unwritten.
    bool record_initialized{}, record_cleanup_armed{}, record_destroyed{};
    bool wrapper_started{}, resolver_started{};
    void* fallback_name{}; // Borrowed actual registry+14, not a copied header.
    NativeTextureNameResolutionCall resolution;
    NativeTextureNameResolutionCall fallback_resolution;
    NativeTextureLoadAcquired loader;
    std::unique_ptr<NativeTextureCacheAcquired> fallback;
    bool fallback_initialized{};
    void* loaded{};
    void* result{};
    bool cache_published{};
    bool caller_acquired{};
};

void append_native_texture_record_00b30130(void* actual_array_header,
    const NativeRenderResourceRecord&, NativeTextureCacheContext&);
void* acquire_native_cached_texture_00b31d80(void*) noexcept;
void initialize_native_texture_fallback_00b31bd0(void* registry,
    NativeTextureCacheContext&, NativeTextureCacheAcquired&);
void* resolve_native_texture_cache_name_00b31c20(void* registry, void* output,
    const void* name, NativeTextureCacheContext&, NativeTextureCacheAcquired*);
// B31C20 requires acquired, and output == &acquired->names.resolver_output.value.
// Direct helpers are single-use: reject replay before any header/native write.
// Complete raw D5F088 cache consumer: ECX registry, four DWORD arguments,
// RET10. All-alias hit then resolver/first-alias search; null rows and dates
// are preserved. Fresh result is NOT retained unless acquire_new is nonzero;
// record copies themselves never retain. Every nonnull hot hit retains.
// Without acquired, only the first-search nonnull hit is admitted. Any path
// reaching B31C20 (even allow_load=0) requires persistent names/child storage.
void* load_native_cached_texture_00b30b40(void* registry, const void* name,
    std::uint32_t post_load_word, std::uint8_t acquire_new, std::uint8_t allow_load,
    NativeTextureCacheContext&, NativeTextureCacheAcquired* acquired = nullptr);
// Complete B319B0 wrapper, RET8. Separate receiver/guard captures, lowercase
// actual temporary, registry at receiver+1A74, forwarded word and flags0/1.
void* load_native_renderer_texture_00b319b0(void* renderer, const void* name,
    std::uint32_t post_load_word, NativeTextureCacheContext&,
    NativeTextureCacheAcquired* acquired = nullptr);
// B2C2D0 common and 2D arms, unused ECX, name/nullable callback stack, RET8.
// Actual VFS open/BEF750, real D3DX imports, actual B3F930 and retained source.
// Cube/volume named constructors remain explicit retained source boundaries.
// D3DX image-info failure with unwritten output is outside this source domain.
void* load_native_texture_2d_00b2c2d0(const void* name, std::uint32_t post_load_word,
    NativeTextureLoadingContext&, NativeTextureLoadAcquired&);
} // namespace bsp
