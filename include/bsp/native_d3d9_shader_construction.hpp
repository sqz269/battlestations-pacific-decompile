#pragma once
#include "bsp/native_render_context.hpp"
#include "bsp/native_resource_support.hpp"
#include "bsp/native_string.hpp"
#include <memory>

namespace bsp {
// Actual 10h D62A60/D62A70 wrapper, not another engine/COM owner. The engine
// count is the SAME raw+04 atomic borrowed by its canonical companion.
struct NativeD3d9ShaderStorage {
    volatile std::uint32_t native_vtable_00;
    std::atomic<std::int32_t> references_04;
    void* shader_08;
    void* opaque_0c; // Constructor clears this word; semantic role is unproved.
};
static_assert(sizeof(NativeD3d9ShaderStorage) == 0x10);
static_assert(offsetof(NativeD3d9ShaderStorage, references_04) == 4);
static_assert(offsetof(NativeD3d9ShaderStorage, shader_08) == 8);
static_assert(offsetof(NativeD3d9ShaderStorage, opaque_0c) == 0x0c);

struct NativeD3d9ShaderRegistryOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t function{}, native_site{}, cursor{};
    void* array{};
    void* shader{};
    void* allocation{};
    std::int32_t requested_capacity{};
    std::unique_ptr<NativeD3d9ShaderRegistryOperation> reserve_child;
    NativeD3d9ShaderRegistryOperation() = default;
    ~NativeD3d9ShaderRegistryOperation();
    NativeD3d9ShaderRegistryOperation(const NativeD3d9ShaderRegistryOperation&) = delete;
    NativeD3d9ShaderRegistryOperation& operator=(const NativeD3d9ShaderRegistryOperation&) = delete;
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Existing actual domains; no private string pool, renderer or owner registry.
// Caller admits/binds the wrapper's canonical companion to raw+04 in owners
// and excludes its retirement while construction is running/failed. Its REAL
// terminal provider remains required externally: B5F410/B5F490/B5F6E0/B5F700
// are not implemented here. Constructors never decrement engine references,
// so they must not invoke the zero-count-only resolve_actual protocol.
struct NativeD3d9ShaderConstructionContext {
    NativeStringStorage& strings;
    NativeResourceSupportStorage* volatile& actual_support_0108fedc;
    SoundLifetimeAccess actual_lifetime;
    void* const volatile& actual_renderer_00f8d394;
    NativeRenderActualOwners& owners;
};
struct NativeD3d9ShaderConstructionOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t function{}, native_site{}, first_length{};
    NativeD3d9ShaderStorage* owner{};
    NativeD3d9ShaderConstructionContext* context{};
    void* argument_com{};
    void* support_com_snapshot{};
    bool base_published{}, com_addref_entered{}, com_addref_returned{};
    bool first_live{}, second_live{}, second_captured{};
    NativeString first_name, second_name;
    char* second_data{};
    std::unique_ptr<NativeD3d9ShaderRegistryOperation> registry;
    NativeD3d9ShaderConstructionOperation() = default;
    ~NativeD3d9ShaderConstructionOperation();
    NativeD3d9ShaderConstructionOperation(const NativeD3d9ShaderConstructionOperation&) = delete;
    NativeD3d9ShaderConstructionOperation& operator=(const NativeD3d9ShaderConstructionOperation&) = delete;
    // AFTER explicit acquisition/child resolution. Does not undo native output,
    // release COM, unregister a shader, or implement original private FH3 unwind.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// ECX actual0Ch renderer registry; stack pointer to raw shader pointer; RET4,
// AL found. Swap-remove first match using captured data/count; no retain/free.
bool remove_native_vertex_shader_registry_00b253e0(void* array, const void* pointer_cell) noexcept;
bool remove_native_pixel_shader_registry_00b25450(void* array, const void* pointer_cell) noexcept;
// ECX actual0Ch registry; stacked signed capacity, RET4. Signed minimum1,
// current count/data copy; free old storage BEFORE publishing new data/capacity.
void reserve_native_vertex_shader_registry_00b22dd0(void*, std::int32_t, NativeD3d9ShaderRegistryOperation&);
void reserve_native_pixel_shader_registry_00b22e30(void*, std::int32_t, NativeD3d9ShaderRegistryOperation&);
// ECX actual renderer; stacked raw wrapper, RET4. +1AC4 vertex/+1AD0 pixel;
// remove existing match, grow if full, append borrowed pointer. No engine retain.
void register_native_vertex_shader_00b289a0(void*, void*, NativeD3d9ShaderRegistryOperation&);
void register_native_pixel_shader_00b289f0(void*, void*, NativeD3d9ShaderRegistryOperation&);
// ECX fresh actual10h, stacked actual COM shader or null, EAX same, RET4.
// Publish base/count1, derived profile, clear0C/08, then store/AddRef actual COM.
// Pixel registers AFTER two pooled-name cleanups; vertex registers BEFORE them.
// B3E730 uses the same canonical singleton domain. One-shot persistent failures;
// explicit C++ interfaces, not original ABI/FH3 or terminal lifetime providers.
NativeD3d9ShaderStorage* construct_native_pixel_shader_00b5f9b0(void*, void*,
    NativeD3d9ShaderConstructionContext&, NativeD3d9ShaderConstructionOperation&);
NativeD3d9ShaderStorage* construct_native_vertex_shader_00b5faf0(void*, void*,
    NativeD3d9ShaderConstructionContext&, NativeD3d9ShaderConstructionOperation&);
} // namespace bsp
