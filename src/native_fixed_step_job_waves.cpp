#include "bsp/native_fixed_step_job_waves.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace bsp {
namespace {

template<class T> T& raw_at(void* owner, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(owner) + offset);
}
void* current_pointer(void* owner, std::size_t offset) noexcept {
    return raw_at<void* volatile>(owner, offset);
}
class CapturedSection final {
public:
    explicit CapturedSection(SystemSingletonCriticalSection* section) : section_(section) {
        if (section_) { singleton_enter_critical_section(*section_); ++section_->recursion_18; }
    }
    ~CapturedSection() {
        if (section_) { --section_->recursion_18; singleton_leave_critical_section(*section_); }
    }
private:
    SystemSingletonCriticalSection* section_;
};
struct DescriptorProfile {
    std::uint32_t primary;
    std::uint32_t holder;
    std::uint32_t secondary;
    std::size_t bytes;
};
constexpr DescriptorProfile kWave1{0x00d0deb4u, 0x00d0de94u, 0x00d0deb0u, 8};
constexpr DescriptorProfile kWave2{0x00d0dec4u, 0x00d0de9cu, 0x00d0dec0u, 8};
constexpr DescriptorProfile kWave3{0x00d0deacu, 0x00d0de90u, 0x00d0dea8u, 8};
constexpr DescriptorProfile kInterpolation{0x00d0debcu, 0x00d0de98u, 0x00d0deb8u, 12};

void* descriptor_singleton(void* volatile& publication, SingletonLifetimeDomain& domain,
    const DescriptorProfile& profile) {
    if (void* const existing = publication) return existing;
    {
        CapturedSection section(domain.get_manager_00415350()->system_owner().section_10);
        if (!publication) {
            void* const raw = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, profile.bytes, profile.bytes});
            if (raw) {
                raw_at<volatile std::uint32_t>(raw, 4) = profile.holder;
                raw_at<volatile std::uint32_t>(raw, 0) = profile.primary;
                raw_at<volatile std::uint32_t>(raw, 4) = profile.secondary;
            }
            publication = raw;
            void* const published = publication;
            void* const secondary = published
                ? static_cast<std::uint8_t*>(published) + 4 : nullptr;
            // Native reloads the manager AFTER capturing the published +04.
            auto* const manager = domain.get_manager_00415350();
            manager->register_object(secondary);
        }
    }
    return publication;
}

// Preserve the original FLD/FSTP float argument rounding/exception behavior.
// Table resolution in the borrowed tick binding is a pure Source lookup.
__declspec(naked) void __fastcall capture_argument(float*, const volatile float*) {
    __asm {
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        ret
    }
}
void require_body(const volatile std::uint32_t* table, std::uint32_t expected) {
    if (!table || table[0] != expected)
        throw std::invalid_argument("Missing current native fixed-step descriptor body binding");
}
bool admitted(JobWavePhase phase, void* element) noexcept {
    // Native captures +28 once before either byte test (EAX at 875CC0/D30/DA0).
    void* const payload = current_pointer(element, 0x28);
    if (phase == JobWavePhase::kWave2) {
        if (raw_at<volatile std::uint8_t>(payload, 0xbd) == 0)
            return false;
        return raw_at<volatile std::uint8_t>(payload, 0x5e) == 0;
    }
    if (raw_at<volatile std::uint8_t>(payload, 0x5e) != 0)
        return false;
    return phase == JobWavePhase::kInterpolation
        || raw_at<volatile std::uint8_t>(payload, 0xbd) != 0;
}
} // namespace

void* native_step_job_wave1_descriptor_008755a0(NativeFixedStepJobDescriptors& bindings) {
    return descriptor_singleton(bindings.wave1_00f878d4, bindings.lifetime_01090aa0, kWave1);
}
void* native_step_job_wave2_descriptor_00875750(NativeFixedStepJobDescriptors& bindings) {
    return descriptor_singleton(bindings.wave2_00f878dc, bindings.lifetime_01090aa0, kWave2);
}
void* native_step_job_wave3_descriptor_008754d0(NativeFixedStepJobDescriptors& bindings) {
    return descriptor_singleton(bindings.wave3_00f878d0, bindings.lifetime_01090aa0, kWave3);
}
void* native_step_job_interpolation_descriptor_00875670(NativeFixedStepJobDescriptors& bindings) {
    return descriptor_singleton(bindings.interpolation_00f878d8,
        bindings.lifetime_01090aa0, kInterpolation);
}
bool delete_native_fixed_step_descriptor_current_00(void* secondary, std::uint32_t flags,
    NativeFixedStepJobDescriptors& bindings) noexcept {
    const auto current = raw_at<volatile std::uint32_t>(secondary, 0);
    void* volatile* publication = nullptr;
    switch (current) {
    case 0x00d0deb0u: publication = &bindings.wave1_00f878d4; break;
    case 0x00d0dec0u: publication = &bindings.wave2_00f878dc; break;
    case 0x00d0dea8u: publication = &bindings.wave3_00f878d0; break;
    case 0x00d0deb8u: publication = &bindings.interpolation_00f878d8; break;
    default: return false;
    }
    void* const primary = static_cast<std::uint8_t*>(secondary) - 4;
    *publication = nullptr;
    raw_at<volatile std::uint32_t>(secondary, 0) = 0x00ce3818u;
    if (flags & 1u) singleton_lifetime_free(primary);
    return true;
}

NativeFixedStepJobDispatch::NativeFixedStepJobDispatch(const volatile float& fixed_step,
    NativeFixedStepJobTables tables, NativeFixedStepTickBindings& ticks,
    NativeFrameJobDispatch& remaining)
    : fixed_step_(fixed_step), tables_(tables), ticks_(ticks), remaining_(remaining) {
    require_body(tables_.wave1_00d0deb4, 0x008750a0u);
    require_body(tables_.wave2_00d0dec4, 0x00875b90u);
    require_body(tables_.wave3_00d0deac, 0x00874fe0u);
    require_body(tables_.interpolation_00d0debc, 0x00875160u);
}
void NativeFixedStepJobDispatch::execute_current_00(void* owner, std::uint32_t argument) {
    const auto table = raw_at<volatile std::uint32_t>(owner, 0);
    void* const element = reinterpret_cast<void*>(static_cast<std::uintptr_t>(argument));
    switch (table) {
    case 0x00d0deb4u: {
        require_body(tables_.wave1_00d0deb4, 0x008750a0u);
        float captured;
        capture_argument(&captured, &fixed_step_);
        ticks_.step_current_04(element, captured);
        ticks_.post_current_0c(element);
        ticks_.refresh_world_00414db0(current_pointer(element, 0x28));
        // Reload after the FIRST refresh. The second refresh uses this capture.
        void* const payload = current_pointer(element, 0x28);
        if (raw_at<volatile std::uint8_t>(payload, 0x10c) == 0) {
            ticks_.refresh_world_00414db0(payload);
            raw_at<volatile std::uint8_t>(payload, 0x10c) = 1;
            ticks_.build_inverse_00b63d50(static_cast<std::uint8_t*>(payload) + 0x110,
                static_cast<const std::uint8_t*>(payload) + 0xcc);
        }
        return;
    }
    case 0x00d0dec4u: {
        require_body(tables_.wave2_00d0dec4, 0x00875b90u);
        float captured;
        capture_argument(&captured, &fixed_step_);
        ticks_.run_subnodes_008759b0(element, captured);
        return;
    }
    case 0x00d0deacu: {
        require_body(tables_.wave3_00d0deac, 0x00874fe0u);
        float captured;
        capture_argument(&captured, &fixed_step_);
        ticks_.pre_current_08(element, captured);
        ticks_.refresh_world_00414db0(current_pointer(element, 0x28));
        return;
    }
    case 0x00d0debcu: {
        require_body(tables_.interpolation_00d0debc, 0x00875160u);
        float captured;
        capture_argument(&captured, &raw_at<volatile float>(owner, 8));
        ticks_.step_current_04(element, captured);
        return;
    }
    default:
        remaining_.execute_current_00(owner, argument);
        return;
    }
}

NativeFixedStepJobWaves::NativeFixedStepJobWaves(void* groups,
    void* pending_head, void* pending_tail, const volatile std::uint8_t& run_pass,
    NativeFrameJobOwnerStorage* volatile& frame, NativeFrameJobLifetimeBindings& frame_lifetime,
    NativeFixedStepJobDescriptors& descriptors)
    : groups_(groups), pending_head_(pending_head), pending_tail_(pending_tail), run_pass_(run_pass),
      frame_(frame), frame_lifetime_(frame_lifetime), descriptors_(descriptors) {
    if (!groups || !pending_head || !pending_tail)
        throw std::invalid_argument("Native fixed-step jobs require actual groups/pending sentinels");
}
void NativeFixedStepJobWaves::flush_pending_groups_00874c90() {
    void* element = current_pointer(pending_head_, 8);
    while (element != pending_tail_) {
        void* const next = current_pointer(element, 8);
        raw_at<void* volatile>(current_pointer(element, 4), 8) = next;
        void* const current_next = current_pointer(element, 8);
        void* const current_previous = current_pointer(element, 4);
        raw_at<void* volatile>(current_next, 4) = current_previous;
        const auto group_word = raw_at<volatile std::uint32_t>(element, 0x14);
        auto* const group = static_cast<std::uint8_t*>(groups_) + group_word * 0x68u;
        raw_at<void* volatile>(element, 4) = current_pointer(group, 0x38);
        raw_at<void* volatile>(element, 8) = group + 0x34;
        raw_at<void* volatile>(current_pointer(group, 0x38), 8) = element;
        raw_at<void* volatile>(group, 0x38) = element;
        element = next;
    }
    raw_at<volatile std::uint32_t>(pending_head_, 4) = 0;
    raw_at<void* volatile>(pending_head_, 8) = pending_tail_;
    raw_at<volatile std::uint32_t>(pending_tail_, 8) = 0;
    raw_at<void* volatile>(pending_tail_, 4) = pending_head_;
}
void NativeFixedStepJobWaves::queue_element(JobWavePhase phase, void* element) {
    // The original gets the current pool BEFORE the per-element descriptor.
    auto* const frame = native_frame_job_singleton_004c1130(frame_,
        descriptors_.lifetime_01090aa0, frame_lifetime_);
    void* descriptor;
    switch (phase) {
    case JobWavePhase::kWave1: descriptor = native_step_job_wave1_descriptor_008755a0(descriptors_); break;
    case JobWavePhase::kWave2: descriptor = native_step_job_wave2_descriptor_00875750(descriptors_); break;
    case JobWavePhase::kWave3: descriptor = native_step_job_wave3_descriptor_008754d0(descriptors_); break;
    case JobWavePhase::kInterpolation:
        descriptor = native_step_job_interpolation_descriptor_00875670(descriptors_); break;
    default: throw std::invalid_argument("Unknown native fixed-step wave");
    }
    frame_lifetime_.execution.require_enqueue_virtual_04(frame->pool_04);
    enqueue_native_frame_job_00be3020(frame->pool_04, descriptor,
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(element)));
}
void NativeFixedStepJobWaves::dispatch_group() {
    auto* const frame = native_frame_job_singleton_004c1130(frame_,
        descriptors_.lifetime_01090aa0, frame_lifetime_);
    frame_lifetime_.execution.require_dispatch_virtual_08(frame->pool_04);
    // Native reads the byte anew at each dispatch, after the CURRENT slot load.
    dispatch_native_frame_jobs_00be3150(frame->pool_04, run_pass_,
        frame_lifetime_.scope, frame_lifetime_.execution);
}
void NativeFixedStepJobWaves::run_wave(JobWavePhase phase, NativeFixedStepJobWaveCounts& counts) {
    auto* group = static_cast<std::uint8_t*>(groups_);
    for (std::size_t index = 0; index < kJobWaveGroupCount; ++index, group += 0x68) {
        void* const tail = group + 0x34;
        void* element = current_pointer(group, 8);
        std::uint32_t queued = 0;
        while (element != tail) {
            ++counts.elements_seen;
            if (admitted(phase, element)) {
                ++queued;
                queue_element(phase, element);
                ++counts.jobs_queued;
            }
            element = current_pointer(element, 8);
        }
        if (queued != 0) {
            dispatch_group();
            ++counts.group_dispatches;
        }
    }
}
void NativeFixedStepJobWaves::run_fixed_waves(NativeFixedStepJobWaveCounts& counts) {
    run_wave(JobWavePhase::kWave1, counts);
    run_wave(JobWavePhase::kWave2, counts);
    run_wave(JobWavePhase::kWave3, counts);
}
void NativeFixedStepJobWaves::run_interpolation(float leftover, NativeFixedStepJobWaveCounts& counts) {
    // 00875F43/00875F50: the factory and +08 store precede EVERY group walk,
    // even with no admitted elements. Per-element getters still reload it.
    void* const descriptor = native_step_job_interpolation_descriptor_00875670(descriptors_);
    raw_at<volatile float>(descriptor, 8) = leftover;
    run_wave(JobWavePhase::kInterpolation, counts);
}
} // namespace bsp
