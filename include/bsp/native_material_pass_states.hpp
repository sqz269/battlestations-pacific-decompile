#pragma once
#include "bsp/native_render_context.hpp"
#include <cstddef>
#include <type_traits>

namespace bsp {
struct NativeMaterialRenderState {std::uint32_t state,value;};
struct NativeMaterialSamplerState {std::uint32_t slot,state,value;};
// Actual twelve-byte vector header at owner+08. The third profile has twelve-
// byte rows too; their interpretation is not established by this packet.
struct NativeMaterialStateArray {
    void* data_00;
    std::int32_t count_04,capacity_08;
};
struct NativeMaterialStateOwnerStorage {
    std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    NativeMaterialStateArray rows_08;
};
static_assert(sizeof(NativeMaterialRenderState)==8);
static_assert(sizeof(NativeMaterialSamplerState)==12);
static_assert(sizeof(NativeMaterialStateArray)==12);
static_assert(sizeof(NativeMaterialStateOwnerStorage)==20);
static_assert(offsetof(NativeMaterialStateOwnerStorage,references_04)==4);
static_assert(offsetof(NativeMaterialStateOwnerStorage,rows_08)==8);
static_assert(std::is_standard_layout_v<NativeMaterialStateOwnerStorage>);
static_assert(std::is_trivially_destructible_v<NativeMaterialStateOwnerStorage>);

enum class NativeMaterialStateKind {render,sampler,third};
// Only the three inlined initialization fragments B5F793..7AB,
// B5F7C0..7D8 and B5F7ED..805 (end exclusive), not the whole pass constructor.
// EAX allocated14h storage, EBX=0, EDI=1. Caller allocates in the same actual
// heap domain. Publish CEB130, count1, concrete table, then zero the header.
NativeMaterialStateOwnerStorage* initialize_native_material_state_owner_00b5f720_fragment(
    void* actual_storage,NativeMaterialStateKind);

// Original ECX actual12h header; stack signed requested capacity; RET4.
// Clamp request to at least1; grow only on signed old_capacity<request.
// Allocate/copy current rows/free old buffer/publish data then capacity.
// Live extents must fit the allocation; corrupt extents and DWORD allocation
// overflow are rejected, not reproduced as native out-of-bounds writes.
void reserve_native_material_render_states_00b40ac0(NativeMaterialStateArray&,std::int32_t);
void reserve_native_material_third_states_00b40b40(NativeMaterialStateArray&,std::int32_t);
void reserve_native_material_sampler_states_00b40be0(NativeMaterialStateArray&,std::int32_t);

// Actual pass pointer has state-owner slots +18(render),+1C(third),+20(sampler).
// This does not declare or construct the rest of the pass. Owners and rows
// are used in place: no semantic vector, copied reference count, or COW.
// Set: first key match, replace differing value; else append with doubled
// capacity (signed DWORD result <=1 becomes1). ECX pass, stack2/3 args RET8/Ch.
void set_native_material_render_state_00b5ec40(void* actual_pass,std::uint32_t state,std::uint32_t value);
void set_native_material_sampler_state_00b5ed60(void* actual_pass,std::uint32_t slot,
    std::uint32_t state,std::uint32_t value);
// ECX pass; stack state; RET4. First match gets last row, then current+18
// owner is reloaded and its count reduced. No-match is a no-op.
void remove_native_material_render_state_00b5ee00(void* actual_pass,std::uint32_t state);

// ECX owner, RET. A negative capacity triggers matching reserve(0), including
// allocate/copy/free. Then decrement positive count to0, free current buffer,
// publish CEB130. Data and capacity remain untouched after that final free.
// Base cleanup also executes on reserve failure; native SEH encoding is not
// emitted. Scalar wrappers take stack flags, return original pointer, RET4.
void destroy_native_material_render_states_00b420c0(NativeMaterialStateOwnerStorage&);
void destroy_native_material_sampler_states_00b42140(NativeMaterialStateOwnerStorage&);
void destroy_native_material_third_states_00b421c0(NativeMaterialStateOwnerStorage&);
NativeMaterialStateOwnerStorage* delete_native_material_render_states_00b422f0(NativeMaterialStateOwnerStorage*,std::uint32_t flags);
NativeMaterialStateOwnerStorage* delete_native_material_sampler_states_00b42310(NativeMaterialStateOwnerStorage*,std::uint32_t flags);
NativeMaterialStateOwnerStorage* delete_native_material_third_states_00b42330(NativeMaterialStateOwnerStorage*,std::uint32_t flags);

class NativeMaterialStateReference;
struct NativeMaterialStateCompanionDisposal {
    void* context;
    void (*retire)(void*,NativeMaterialStateReference&) noexcept;
};
// Borrows same actual+04. Bound current profiles D61A2C/34/3C have BD30E0
// plus B422F0/B42310/B42330 respectively. Final-zero destroys/frees actual
// storage before explicit companion retirement; no private owner registry.
class NativeMaterialStateReference final:public RenderCommandReference {
public:
    NativeMaterialStateReference(NativeMaterialStateOwnerStorage&,
        const volatile std::uint32_t* actual_profile,NativeMaterialStateCompanionDisposal);
    ~NativeMaterialStateReference() override;
    NativeMaterialStateOwnerStorage& storage() noexcept {return storage_;}
    void release_zero_references() noexcept override;
private:
    enum class Phase {bound,destroying,retired};
    NativeMaterialStateOwnerStorage& storage_;
    const volatile std::uint32_t* profile_;
    NativeMaterialStateCompanionDisposal disposal_;
    std::uint32_t bound_table_;
    Phase phase_{Phase::bound};
    void require_current_profile() const noexcept;
};
} // namespace bsp
