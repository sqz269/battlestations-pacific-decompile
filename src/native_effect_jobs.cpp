#include "bsp/native_effect_jobs.hpp"
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native effect jobs require MSVC Win32.
#endif

namespace bsp {
namespace {
class CapturedSection final {
public:
    explicit CapturedSection(SystemSingletonCriticalSection* value) : section_(value) {
        if (section_) { singleton_enter_critical_section(*section_); ++section_->recursion_18; }
    }
    ~CapturedSection() {
        if (section_) { --section_->recursion_18; singleton_leave_critical_section(*section_); }
    }
private:
    SystemSingletonCriticalSection* section_;
};
// Read the original global delta with FLD, then the reference global, then
// round/store delta with FSTP. No C++ callback can alter either input between
// these reads. Current event-table resolution below is a pure host projection.
__declspec(naked) void* __fastcall capture_arguments(float*, const volatile float*,
    void* volatile*) {
    __asm {
        fld dword ptr [edx]
        mov eax,dword ptr [esp+4]
        mov eax,dword ptr [eax]
        fstp dword ptr [ecx]
        ret 4
    }
}
}

void execute_native_effect_job_008663b0(void* event, volatile float& delta,
    void* volatile& reference, NativeEffectJobEvents& events, PointEffectChildEvents& children) {
    float argument;
    void* const captured_reference = capture_arguments(&argument, &delta, &reference);
    auto& existing = events.existing_reference(event);
    children.update_virtual_28(existing, argument, captured_reference);
}
NativeEffectJobDispatch::NativeEffectJobDispatch(volatile float& delta,
    void* volatile& reference, const volatile std::uint32_t* table,
    NativeEffectJobEvents& events, PointEffectChildEvents& children, NativeFrameJobDispatch& remaining)
    : delta_(delta), reference_(reference), table_(table), events_(events),
      children_(children), remaining_(remaining) {
    if (!table || table[0] != 0x008663b0u)
        throw std::invalid_argument("Effect jobs require the actual D0D3E8 primary table");
}
void NativeEffectJobDispatch::execute_current_00(void* owner, std::uint32_t argument) {
    const auto current_table = *static_cast<const volatile std::uint32_t*>(owner);
    if (current_table != 0x00d0d3e8u) {
        remaining_.execute_current_00(owner, argument);
        return;
    }
    if (table_[0] != 0x008663b0u)
        throw std::invalid_argument("Missing current native effect job callback binding");
    execute_native_effect_job_008663b0(reinterpret_cast<void*>(argument), delta_, reference_,
        events_, children_);
}
NativeEffectJobOwnerStorage* native_effect_job_singleton_008667f0(
    NativeEffectJobOwnerStorage* volatile& global, SingletonLifetimeDomain& domain) {
    if (auto* existing = global) return existing;
    {
        CapturedSection section(domain.get_manager_00415350()->system_owner().section_10);
        if (!global) {
            void* const raw = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, 8, sizeof(NativeEffectJobOwnerStorage)});
            NativeEffectJobOwnerStorage* created = nullptr;
            if (raw) {
                created = ::new(raw) NativeEffectJobOwnerStorage;
                created->secondary_table_04 = 0x00d0d3c8u;
                created->primary_table_00 = 0x00d0d3e8u;
                created->secondary_table_04 = 0x00d0d3e4u;
            }
            global = created;
            auto* const published = global;
            void* const secondary = published
                ? const_cast<std::uint32_t*>(&published->secondary_table_04) : nullptr;
            auto* const manager = domain.get_manager_00415350();
            manager->register_object(secondary);
        }
    }
    return global;
}
NativeEffectJobOwnerStorage* delete_native_effect_job_00866400(
    NativeEffectJobOwnerStorage* owner, std::uint32_t flags,
    NativeEffectJobOwnerStorage* volatile& global) noexcept {
    auto* const original = owner;
    global = nullptr;
    owner->secondary_table_04 = 0x00ce3818u;
    if (flags & 1u) { owner->~NativeEffectJobOwnerStorage(); singleton_lifetime_free(owner); }
    return original;
}
NativeEffectJobOwnerStorage* delete_native_effect_job_secondary_008663d0(
    volatile std::uint32_t* secondary, std::uint32_t flags,
    NativeEffectJobOwnerStorage* volatile& global) noexcept {
    auto* const primary = reinterpret_cast<NativeEffectJobOwnerStorage*>(
        reinterpret_cast<std::uintptr_t>(secondary) - 4u);
    return delete_native_effect_job_00866400(primary, flags, global);
}
void dispatch_native_effect_entries_00866c60(NativeLiveEffectManagerStorage& manager,
    float delta, void* reference, volatile float& global_delta, void* volatile& global_reference,
    NativeEffectJobOwnerStorage* volatile& job_global,
    NativeFrameJobOwnerStorage* volatile& frame_global, SingletonLifetimeDomain& domain,
    NativeFrameJobLifetimeBindings& bindings) {
    const auto count = static_cast<std::uint32_t>(manager.references_1c.count_04);
    global_reference = reference;
    const auto end_base = reinterpret_cast<std::uintptr_t>(manager.references_1c.data_00);
    auto cursor = reinterpret_cast<std::uintptr_t>(manager.references_1c.data_00);
    const auto end = end_base + count * 4u;
    global_delta = delta;
    while (cursor != end) {
        auto* const frame = native_frame_job_singleton_004c1130(frame_global, domain, bindings);
        void* const event = *reinterpret_cast<void* const*>(cursor);
        auto& pool = frame->pool_04;
        auto* const job = native_effect_job_singleton_008667f0(job_global, domain);
        bindings.execution.require_enqueue_virtual_04(pool);
        enqueue_native_frame_job_00be3020(pool, job, reinterpret_cast<std::uint32_t>(event));
        cursor += 4u;
    }
    auto& pool = native_frame_job_singleton_004c1130(frame_global, domain, bindings)->pool_04;
    bindings.execution.require_dispatch_virtual_08(pool);
    dispatch_native_frame_jobs_00be3150(pool, 1, bindings.scope, bindings.execution);
}
} // namespace bsp
