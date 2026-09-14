#pragma once
#include "bsp/native_render_context.hpp"
#include "bsp/native_string.hpp"
#include <array>
#include <cstddef>
#include <type_traits>

namespace bsp {
struct NativeMaterialEffectDescriptorContext;

// Exact C4h base and178h derived storage. No semantic effect/cache copy.
// Unwritten bytes and fields intentionally have no default initializers.
struct NativeMaterialEffectBaseStorage {
    std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    std::uint8_t byte_08;
    std::array<std::byte,3> preserved_09;
    std::array<void*,11> textures_0c;
    std::int16_t texture_count_38;
    std::array<std::byte,2> preserved_3a;
    std::array<NativeString,11> names_3c;
    std::int16_t name_count_94;
    std::array<std::byte,2> preserved_96;
    void* fallback_98;
    std::array<void*,3> retained_9c;
    std::int32_t retained_count_a8;
    std::uint32_t word_ac,word_b0;
    std::uint8_t dirty_b4;
    std::array<std::byte,3> preserved_b5;
    NativeString name_b8;
    std::uint32_t serial_c0;
};
struct NativeMaterialEffectStorage {
    NativeMaterialEffectBaseStorage base;
    void* descriptor_c4;
    std::array<void*,14> passes_c8;
    std::array<void*,14> secondary_100;
    void* retained_138;
    std::uint8_t byte_13c;
    std::array<std::byte,3> preserved_13d;
    std::array<std::uint32_t,14> words_140;
};
static_assert(sizeof(NativeMaterialEffectBaseStorage)==0xc4);
static_assert(sizeof(NativeMaterialEffectStorage)==0x178);
static_assert(std::is_standard_layout_v<NativeMaterialEffectBaseStorage>);
static_assert(std::is_standard_layout_v<NativeMaterialEffectStorage>);
static_assert(std::is_trivially_destructible_v<NativeMaterialEffectStorage>);
static_assert(offsetof(NativeMaterialEffectBaseStorage,references_04)==4);
static_assert(offsetof(NativeMaterialEffectBaseStorage,textures_0c)==0xc);
static_assert(offsetof(NativeMaterialEffectBaseStorage,texture_count_38)==0x38);
static_assert(offsetof(NativeMaterialEffectBaseStorage,names_3c)==0x3c);
static_assert(offsetof(NativeMaterialEffectBaseStorage,name_count_94)==0x94);
static_assert(offsetof(NativeMaterialEffectBaseStorage,fallback_98)==0x98);
static_assert(offsetof(NativeMaterialEffectBaseStorage,retained_9c)==0x9c);
static_assert(offsetof(NativeMaterialEffectBaseStorage,retained_count_a8)==0xa8);
static_assert(offsetof(NativeMaterialEffectBaseStorage,dirty_b4)==0xb4);
static_assert(offsetof(NativeMaterialEffectBaseStorage,name_b8)==0xb8);
static_assert(offsetof(NativeMaterialEffectBaseStorage,serial_c0)==0xc0);
static_assert(offsetof(NativeMaterialEffectStorage,descriptor_c4)==0xc4);
static_assert(offsetof(NativeMaterialEffectStorage,passes_c8)==0xc8);
static_assert(offsetof(NativeMaterialEffectStorage,secondary_100)==0x100);
static_assert(offsetof(NativeMaterialEffectStorage,retained_138)==0x138);
static_assert(offsetof(NativeMaterialEffectStorage,words_140)==0x140);

struct NativeMaterialEffectConstructionAccess {
    NativeStringStorage& strings; // same actual00419CC0/BD1510 pool
    void* const volatile& current_renderer_00f8d394;
    volatile std::uint32_t& next_serial_00f8d3a8;
};
struct NativeMaterialEffectDestructionAccess {
    NativeStringStorage& strings;
    NativeRenderActualOwners& retained_owners;
    // Optional raw numeric-profile path. Its strings must be this SAME service.
    // Null retains the existing explicitly callable descriptor-table API.
    NativeMaterialEffectDescriptorContext* actual_descriptor{};
};

// Original ECX fresh storage, EAX same storage, RET. Aligned C4h/178h storage
// required. Base builds the actual temporary "error.tga" header, then invokes
// CURRENT callable renderer+64 (ECX renderer; stack header/0; EAX owned fallback;
// RET8). Original numeric vtables alone are not callable host bindings. No extra
// fallback retain or null substitution. Release temporary before sampling the
// shared DWORD serial; write full+C0, bytes+08/+B4, then increment modulo2^32.
// Derived leaves+C8..137 UNWRITTEN. Caller/loader must establish every pass
// slot before destruction. Base leaves+9C..A7, +AC/+B0 and padding untouched.
NativeMaterialEffectBaseStorage* initialize_native_material_effect_base_00b18d60(
    void* actual_storage, NativeMaterialEffectConstructionAccess&);
NativeMaterialEffectStorage* initialize_native_material_effect_00b407a0(
    void* actual_storage, NativeMaterialEffectConstructionAccess&);

// Full B18D50 helper: ECX first of eleven actual8h headers, RET. Destroy in
// reverse order, leaving headers as the underlying0041DD20 release leaves them.
void destroy_native_material_effect_names_00b18d50(NativeString* actual_names,
    NativeStringStorage&) noexcept;
// ECX base, RET. Current signed +38 end is reloaded after each texture release;
// then clear count, repeat current+A8 for three owners, and clear that count.
// Slots clear AFTER callbacks. Valid counts0..11/0..3 and forward live cursors
// are required; invalid/reentrant extents are host errors, not memory corruption.
void release_native_material_effect_base_owners_00b187a0(
    NativeMaterialEffectBaseStorage&, NativeRenderActualOwners&);
// ECX derived, RET. Current descriptor+ C4 virtual0(flags1) is DIRECT deletion,
// independent of any reference count. It requires the actual callable deleting
// table; no fallback descriptor service/registry is supplied. Then release all
// fourteen+C8, fourteen+100 and+138 identities, clearing after each callback.
void release_native_material_effect_owners_00b41b10(
    NativeMaterialEffectStorage&, NativeRenderActualOwners&);
void release_native_material_effect_owners_00b41b10(
    NativeMaterialEffectStorage&, NativeRenderActualOwners&, NativeMaterialEffectDescriptorContext&);

// Complete normal/member-unwind resource ordering, not native SEH encoding.
// Base releases fallback, then arrays, then+B8 name, reverse eleven names,
// then CEB130 base cleanup. Derived releases its owners before base cleanup,
// including on a throwing direct descriptor callback. Terminal retained-owner
// dispatch and string release must be nonthrowing. Native scalar deletion
// frees through the shared actual allocation domain iff flags&1.
void destroy_native_material_effect_base_00b18eb0(NativeMaterialEffectBaseStorage&,
    NativeMaterialEffectDestructionAccess&);
void destroy_native_material_effect_00b41f80(NativeMaterialEffectStorage&,
    NativeMaterialEffectDestructionAccess&);
NativeMaterialEffectBaseStorage* delete_native_material_effect_base_00b192d0(
    NativeMaterialEffectBaseStorage*, NativeMaterialEffectDestructionAccess&, std::uint32_t flags);
NativeMaterialEffectStorage* delete_native_material_effect_00b422d0(
    NativeMaterialEffectStorage*, NativeMaterialEffectDestructionAccess&, std::uint32_t flags);

class NativeMaterialEffectReference;
struct NativeMaterialEffectCompanionDisposal {
    void* context;
    void (*retire)(void*,NativeMaterialEffectReference&) noexcept;
};
// Canonical companion borrows SAME actual+04; no retained child or second
// counter. Register in the same domain as materials/textures/pass owners.
// Bound profile is either D5E534/BD30E0/B192D0 for base storage or
// D61A00/BD30E0/B422D0 for complete derived storage, and must remain current.
class NativeMaterialEffectReference final:public RenderCommandReference {
public:
    NativeMaterialEffectReference(NativeMaterialEffectBaseStorage&,
        NativeMaterialEffectDestructionAccess&, const volatile std::uint32_t* actual_profile,
        NativeMaterialEffectCompanionDisposal);
    ~NativeMaterialEffectReference() override;
    NativeMaterialEffectBaseStorage& storage() noexcept {return storage_;}
    void release_zero_references() noexcept override;
private:
    enum class Phase {bound,destroying,retired};
    NativeMaterialEffectBaseStorage& storage_;
    NativeMaterialEffectDestructionAccess& access_;
    const volatile std::uint32_t* profile_;
    NativeMaterialEffectCompanionDisposal disposal_;
    std::uint32_t bound_table_;
    Phase phase_{Phase::bound};
    void require_current_profile() const noexcept;
};
} // namespace bsp
