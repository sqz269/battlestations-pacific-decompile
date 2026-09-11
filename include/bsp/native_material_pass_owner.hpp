#pragma once
#include "bsp/native_material_pass_base.hpp"
#include "bsp/native_material_pools.hpp"
#include "bsp/native_string.hpp"

namespace bsp {
// Actual 10h binding object. Word00 is not interpreted by this lifetime packet.
// B42250 releases +04 and the actual string at +08; its caller frees the object.
struct NativeMaterialPassBindingStorage {
    std::uint32_t word_00;
    void* retained_04;
    NativeString name_08;
};
struct NativeMaterialPassStorage {
    NativeMaterialPassBaseStorage base;
    std::array<NativeMaterialPassBindingStorage*,4> bindings_5c;
    std::uint32_t binding_count_6c;
    void* vertex_shader_70;
    void* pixel_shader_74;
    std::int32_t index_78, index_7c;
    std::uint8_t byte_80;
    std::array<std::byte,3> preserved_81;
    void* fallback_84;
};
static_assert(sizeof(NativeMaterialPassBindingStorage) == 0x10);
static_assert(offsetof(NativeMaterialPassBindingStorage, retained_04) == 4);
static_assert(offsetof(NativeMaterialPassBindingStorage, name_08) == 8);
static_assert(sizeof(NativeMaterialPassStorage) == NativeMaterialPassPool::object_bytes);
static_assert(offsetof(NativeMaterialPassStorage, bindings_5c) == 0x5c);
static_assert(offsetof(NativeMaterialPassStorage, binding_count_6c) == 0x6c);
static_assert(offsetof(NativeMaterialPassStorage, vertex_shader_70) == 0x70);
static_assert(offsetof(NativeMaterialPassStorage, pixel_shader_74) == 0x74);
static_assert(offsetof(NativeMaterialPassStorage, index_78) == 0x78);
static_assert(offsetof(NativeMaterialPassStorage, index_7c) == 0x7c);
static_assert(offsetof(NativeMaterialPassStorage, byte_80) == 0x80);
static_assert(offsetof(NativeMaterialPassStorage, fallback_84) == 0x84);
static_assert(std::is_standard_layout_v<NativeMaterialPassStorage>);
static_assert(std::is_trivially_destructible_v<NativeMaterialPassStorage>);

struct NativeMaterialPassDestructionAccess {
    NativeStringStorage& strings;
    NativeRenderActualOwners& retained_owners;
    NativeMaterialPassPool& pool; // canonical actual 0108FBF8 companion
};
struct NativeMaterialPassStateRegistration {
    void* context;
    // Host metadata only: register the three already-constructed actual state
    // owners in the SAME canonical owner domain, without retaining/copying them.
    // Must complete without throwing before string/renderer calls can unwind.
    void (*bind)(void*, NativeMaterialPassBaseStorage&) noexcept;
};
struct NativeMaterialPassConstructionAccess {
    NativeMaterialPassDestructionAccess& lifetime;
    void* const volatile& current_renderer_00f8d394;
    NativeMaterialPassStateRegistration state_registration;
};

// ECX fresh 88h pass, EAX same pass, RET. Construct actual 5Ch base, publish
// D61BE8, clear count/70/74/80/84 and set78/7C=-1. Four binding pointers and
// three padding bytes remain UNWRITTEN. Acquire owned "white.tga" through the
// CURRENT callable renderer+64 using its actual temporary string header/flags0.
// Store returned owner at84 without extra retain; release temporary afterward.
// On failure, native states clean the armed temporary then the actual base.
NativeMaterialPassStorage* initialize_native_material_pass_00b44b10(
    void* actual_storage, NativeMaterialPassConstructionAccess&);
// ECX actual10h binding, RET. Release +04, clear AFTER callback, then destroy
// actual +08 string on normal/member-unwind paths; leave its header stale.
void destroy_native_material_pass_binding_00b42250(NativeMaterialPassBindingStorage&,
    NativeMaterialPassDestructionAccess&);
// ECX actual88h pass, RET. Re-read unsigned binding count after every slot;
// destroy/free/clear nonnull entries, then release/clear84,70,74 and base.
// Valid readable extents0..4 required. Binding count stays stale. Base cleanup
// follows native member-unwind ordering; original SEH encoding is not emitted.
void destroy_native_material_pass_00b454e0(NativeMaterialPassStorage&,
    NativeMaterialPassDestructionAccess&);
// ECX pass, stack flags, EAX original pass, RET4. Return raw slot to the
// actual pool iff flags&1, AFTER full destruction. Never free the slot as heap.
NativeMaterialPassStorage* delete_native_material_pass_00b46910(NativeMaterialPassStorage*,
    NativeMaterialPassDestructionAccess&, std::uint32_t flags);

// Complete root +14 accessors. Setter ECX root, stack identity, EAX identity,
// RET4; getter ECX root, EAX identity, RET. No retain/release or owner transfer.
void* set_native_material_pass_effect_00b172b0(NativeMaterialPassRootStorage&, void*) noexcept;
void* get_native_material_pass_effect_00b172c0(const NativeMaterialPassRootStorage&) noexcept;

class NativeMaterialPassReference;
struct NativeMaterialPassCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeMaterialPassReference&) noexcept;
};
// Borrows the SAME actual+04 and current D61BE8/BD30E0/B46910 profile.
// Final zero destroys the actual payload and returns its slot before explicit
// companion retirement. Children resolve in the supplied shared owner domain.
class NativeMaterialPassReference final : public RenderCommandReference {
public:
    NativeMaterialPassReference(NativeMaterialPassStorage&, NativeMaterialPassDestructionAccess&,
        const volatile std::uint32_t* actual_profile, NativeMaterialPassCompanionDisposal);
    ~NativeMaterialPassReference() override;
    NativeMaterialPassStorage& storage() noexcept { return storage_; }
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    NativeMaterialPassStorage& storage_;
    NativeMaterialPassDestructionAccess& access_;
    const volatile std::uint32_t* profile_;
    NativeMaterialPassCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_current_profile() const noexcept;
};
} // namespace bsp
