#pragma once
#include "bsp/native_material_pass_states.hpp"
#include <array>

namespace bsp {
struct NativeMaterialPassRootStorage {
    std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    std::uint32_t word_08,word_0c,word_10;
    void* borrowed_effect_14;
};
struct NativeMaterialPassBaseStorage {
    NativeMaterialPassRootStorage root;
    NativeMaterialStateOwnerStorage* render_18;
    NativeMaterialStateOwnerStorage* third_1c;
    NativeMaterialStateOwnerStorage* sampler_20;
    NativeMaterialStateArray pairs_24,render_rows_30,third_rows_3c,sampler_rows_48;
    void* retained_54;
    void* retained_58;
};
static_assert(sizeof(NativeMaterialPassRootStorage)==0x18);
static_assert(sizeof(NativeMaterialPassBaseStorage)==0x5c);
static_assert(offsetof(NativeMaterialPassRootStorage,references_04)==4);
static_assert(offsetof(NativeMaterialPassRootStorage,borrowed_effect_14)==0x14);
static_assert(offsetof(NativeMaterialPassBaseStorage,render_18)==0x18);
static_assert(offsetof(NativeMaterialPassBaseStorage,third_1c)==0x1c);
static_assert(offsetof(NativeMaterialPassBaseStorage,sampler_20)==0x20);
static_assert(offsetof(NativeMaterialPassBaseStorage,pairs_24)==0x24);
static_assert(offsetof(NativeMaterialPassBaseStorage,render_rows_30)==0x30);
static_assert(offsetof(NativeMaterialPassBaseStorage,third_rows_3c)==0x3c);
static_assert(offsetof(NativeMaterialPassBaseStorage,sampler_rows_48)==0x48);
static_assert(offsetof(NativeMaterialPassBaseStorage,retained_54)==0x54);
static_assert(offsetof(NativeMaterialPassBaseStorage,retained_58)==0x58);
static_assert(std::is_standard_layout_v<NativeMaterialPassBaseStorage>);
static_assert(std::is_trivially_destructible_v<NativeMaterialPassBaseStorage>);

// Original ECX fresh18h/5Ch storage, EAX same storage, RET. Numeric tables are
// original identities, not callable host vtables. Root publishes CEB130/count1/
// D5E508 and clears08..14. Base publishes D62A80, clears four actual headers and
//54/58, allocates state owners in18/20/1C order, then installs20 render defaults
// and96 sampler defaults through the actual setters. Same shared heap domain.
// Caller binds these three actual children in its canonical owner domain before
// their first terminal release. Constructor failure cleans embedded arrays/base
// only, as native state4 specifies; already-published state pointers are not
// released by its unwind map. Whole derived88h pass and its pool remain separate.
NativeMaterialPassRootStorage* initialize_native_material_pass_root_00b17940(void*);
NativeMaterialPassBaseStorage* initialize_native_material_pass_base_00b5f720(void*);

// Actual12h header, stack signed request, RET4. B40C80 has the same8-byte
// reserve semantics as B40AC0. Resize grows if needed, zeroes newly exposed
// pairs, decrements excess count and stores the requested count. Nonnegative
// readable extents and fitting allocations required; corrupt extents rejected.
void reserve_native_material_pass_pairs_00b40c80(NativeMaterialStateArray&,std::int32_t);
void resize_native_material_pass_pairs_00b40e00(NativeMaterialStateArray&,std::int32_t);
// ECX actual header, RET. Pair cleanup resize(0)/free; other headers perform
// negative-capacity reserve(0), reduce count0/free. Data/capacity remain stale.
void destroy_native_material_pass_pairs_00b5f3f0(NativeMaterialStateArray&);
void destroy_native_material_pass_render_rows_00b41bd0(NativeMaterialStateArray&);
void destroy_native_material_pass_third_rows_00b41ca0(NativeMaterialStateArray&);
void destroy_native_material_pass_sampler_rows_00b41d90(NativeMaterialStateArray&);

// ECX actual5Ch base, RET. Publish D62A80; release18/20/1C/54/58, clearing each
// AFTER callbacks; then clean embedded48/3C/30/24 and publishCEB130. Array/base
// cleanup also follows native unwind states; no native SEH encoding emitted.
void destroy_native_material_pass_base_00b5f510(NativeMaterialPassBaseStorage&,NativeRenderActualOwners&);
// ECX base, stack flags, EAX original pointer, RET4. Shared heap free iff bit0.
NativeMaterialPassBaseStorage* delete_native_material_pass_base_00b5f990(
    NativeMaterialPassBaseStorage*,NativeRenderActualOwners&,std::uint32_t flags);

class NativeMaterialPassBaseReference;
struct NativeMaterialPassBaseCompanionDisposal {
    void* context;
    void (*retire)(void*,NativeMaterialPassBaseReference&) noexcept;
};
// Same actual+04; current D62A80/BD30E0/B5F990 profile. Final zero destroys and
// frees actual storage before explicit companion retirement. Caller supplies
// the existing shared actual-owner domain, including all three state owners.
class NativeMaterialPassBaseReference final:public RenderCommandReference {
public:
    NativeMaterialPassBaseReference(NativeMaterialPassBaseStorage&,NativeRenderActualOwners&,
        const volatile std::uint32_t*,NativeMaterialPassBaseCompanionDisposal);
    ~NativeMaterialPassBaseReference() override;
    NativeMaterialPassBaseStorage& storage() noexcept {return storage_;}
    void release_zero_references() noexcept override;
private:
    enum class Phase {bound,destroying,retired};
    NativeMaterialPassBaseStorage& storage_;
    NativeRenderActualOwners& owners_;
    const volatile std::uint32_t* profile_;
    NativeMaterialPassBaseCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_current_profile() const noexcept;
};
} // namespace bsp
