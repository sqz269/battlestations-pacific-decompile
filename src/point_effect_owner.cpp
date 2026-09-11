#include "bsp/point_effect_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/camera_multiply.hpp"

namespace bsp {
namespace {
class ConsumedReference final {
public:
    explicit ConsumedReference(RenderCommandReference* reference) noexcept
        : reference_(reference) {}
    ~ConsumedReference() {
        if (reference_) release_render_command_reference(*reference_);
    }
    ConsumedReference(const ConsumedReference&) = delete;
    ConsumedReference& operator=(const ConsumedReference&) = delete;
private:
    RenderCommandReference* reference_;
};

class CapturedEffectSection final {
public:
    explicit CapturedEffectSection(SystemSingletonCriticalSection* section)
        : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18; // 00868A12, after successful OS entry
        }
    }
    ~CapturedEffectSection() {
        if (section_) {
            --section_->recursion_18; // 00868B50/BA6, before OS leave
            singleton_leave_critical_section(*section_);
        }
    }
    CapturedEffectSection(const CapturedEffectSection&) = delete;
    CapturedEffectSection& operator=(const CapturedEffectSection&) = delete;
private:
    SystemSingletonCriticalSection* section_;
};

// Native stores are x87, unlike the effect wrapper's MOVSS stores. Do not
// replace with memcpy: signaling NaNs/FP traps belong to the caller's x87 state.
__declspec(naked) void __fastcall copy_record_xyz_kernel(float*, const float*) {
    __asm {
        fld dword ptr [edx]       // 0049C1DB
        fstp dword ptr [ecx]      // 0049C1E1
        fld dword ptr [edx + 4]   // 0049C1E6
        fstp dword ptr [ecx + 4]  // 0049C1E9
        fld dword ptr [edx + 8]   // 0049C1EC
        fstp dword ptr [ecx + 8]  // 0049C1F1
        ret
    }
}

// Common successful-admission tail of868420/8685E0/8687C0. Caller still owns
// the captured lock and consumed input. Callee consumes the extra argument
// before raw free on failure; no source-matrix snapshot is taken here.
RenderCommandReference*& construct_admitted_point(RenderCommandReference*& output,
    RenderCommandReference& definition, CameraTransform* parent, std::uint32_t third_word,
    const CameraMatrix& matrix, std::uint8_t transform, std::uint8_t option,
    std::uint32_t tail, PointEffectConstruction& construction) {
    void* const raw = construction.allocate_00bf681b(0x114);
    if (!raw) { output = nullptr; return output; }
    retain_render_command_reference(definition);
    RenderCommandReference* created;
    try {
        created = &construction.construct_008680b0(raw, &definition, parent,
            third_word, matrix, transform, option, tail);
    } catch (...) {
        construction.free_00bf65ac(raw);
        throw;
    }
    output = nullptr;
    output = created;
    retain_render_command_reference(*created);
    release_render_command_reference(*created);
    return output;
}
} // namespace

void* create_point_announcement_0049c940(PointAnnouncementManagerView manager,
    const std::array<float, 3>& captured_xyz,
    const std::array<float, 3>& secondary_xyz, std::int32_t descriptor,
    PointAnnouncementConstruction& construction) {
    if (descriptor < 0) return nullptr;
    void* raw = construction.allocate_00bf681b(0x30);
    if (!raw) return nullptr;
    try {
        construction.construct_0049c000(raw, manager.owner_08,
            captured_xyz, secondary_xyz, descriptor);
    } catch (...) {
        construction.free_00bf65ac(raw); // C63330, state0 -> -1
        throw;
    }
    return raw;
}

void copy_point_record_xyz_0049c1db(float* actual_record_xyz_08,
    const std::array<float, 3>& captured_xyz) {
    copy_record_xyz_kernel(actual_record_xyz_08, captured_xyz.data());
}

RenderCommandReference*& create_point_effect_008689c0(
    RenderCommandReference*& output, CameraTransform* parent,
    RenderCommandReference* consumed_template,
    const std::array<float, 3>& captured_xyz, std::uint8_t transform_byte,
    std::uint8_t option_byte, std::uint32_t tail_word,
    CameraMatrix& actual_matrix_00f87610, PointEffectConstruction& construction) {
    // State1 exists BEFORE the manager getter, so even getter failure consumes
    // the incoming by-value reference. Declaration order ensures unlock first.
    ConsumedReference input(consumed_template);
    CapturedEffectSection lock(construction.manager_00866440().section_04);

    actual_matrix_00f87610[12] = captured_xyz[0]; // 00868A25
    actual_matrix_00f87610[13] = captured_xyz[1]; // 00868A39
    actual_matrix_00f87610[14] = captured_xyz[2]; // 00868A4B
    if (!consumed_template) {
        output = nullptr;
        return output;
    }

    std::array<float, 3> transformed;
    const std::array<float, 3>* test_xyz = &captured_xyz;
    if (transform_byte != 0) {
        if ((parent->valid_flags & 2u) == 0) refresh_camera_world_00b6db70(*parent);
        transform_point_004142e0(captured_xyz, parent->world, transformed);
        test_xyz = &transformed;
    }
    CameraTransform& reference = construction.reference_transform_e188a8_19fc();
    if (!construction.eligible_0086a650(*consumed_template, *test_xyz, reference)) {
        output = nullptr;
        return output;
    }

    void* raw = construction.allocate_00bf681b(0x114);
    if (!raw) {
        output = nullptr;
        return output;
    }
    retain_render_command_reference(*consumed_template); // 00868AF6
    RenderCommandReference* created;
    try {
        created = &construction.construct_008680b0(raw, consumed_template,
            parent, 0, actual_matrix_00f87610, transform_byte, option_byte, tail_word);
    } catch (...) {
        // The callee consumes its extra argument BEFORE this allocation free;
        // then state3->2->1 unwinds raw storage, lock, original input reference.
        construction.free_00bf65ac(raw); // C95149
        throw;
    }
    output = nullptr; // 00868B0F: fresh output, no old-pointer release
    output = created;
    retain_render_command_reference(*created); // 00868B1D, after publication
    release_render_command_reference(*created); // 00868B3E, still locked
    return output; // lock destructor, THEN input destructor
}

RenderCommandReference*& create_point_effect_matrix_00868420(
    RenderCommandReference*& output, std::uint32_t third_word,
    RenderCommandReference& definition, const CameraMatrix& matrix,
    std::uint8_t option, std::uint32_t tail, PointEffectConstruction& construction) {
    ConsumedReference input(&definition);
    CapturedEffectSection lock(construction.manager_00866440().section_04);
    auto& reference = construction.reference_transform_e188a8_19fc();
    if (!construction.eligible_0086a650(definition, EffectPointView(matrix.data() + 12), reference)) {
        output = nullptr;
        return output;
    }
    return construct_admitted_point(output, definition, nullptr, third_word,
        matrix, 0, option, tail, construction);
}

RenderCommandReference*& create_point_effect_position_008685e0(
    RenderCommandReference*& output, std::uint32_t third_word,
    RenderCommandReference& definition, EffectPointView point, std::uint8_t option,
    std::uint32_t tail, CameraMatrix& global_matrix, PointEffectConstruction& construction) {
    ConsumedReference input(&definition);
    CapturedEffectSection lock(construction.manager_00866440().section_04);
    global_matrix[12] = point[0];
    global_matrix[13] = point[1];
    global_matrix[14] = point[2];
    auto& reference = construction.reference_transform_e188a8_19fc();
    if (!construction.eligible_0086a650(definition, point, reference)) {
        output = nullptr;
        return output;
    }
    return construct_admitted_point(output, definition, nullptr, third_word,
        global_matrix, 0, option, tail, construction);
}

RenderCommandReference*& create_point_effect_parent_matrix_008687c0(
    RenderCommandReference*& output, CameraTransform* parent,
    RenderCommandReference& definition, const CameraMatrix& matrix,
    std::uint8_t transform, std::uint8_t option, std::uint32_t tail,
    PointEffectConstruction& construction) {
    ConsumedReference input(&definition);
    CapturedEffectSection lock(construction.manager_00866440().section_04);
    CameraMatrix composed;
    const CameraMatrix* test_matrix = &matrix;
    if (transform != 0) {
        if ((parent->valid_flags & 2u) == 0) refresh_camera_world_00b6db70(*parent);
        multiply_camera_matrices_00413920(composed, matrix, parent->world);
        test_matrix = &composed;
    }
    auto& reference = construction.reference_transform_e188a8_19fc();
    if (!construction.eligible_0086a650(definition, EffectPointView(test_matrix->data() + 12), reference)) {
        output = nullptr;
        return output;
    }
    return construct_admitted_point(output, definition, parent, 0,
        matrix, transform, option, tail, construction);
}

} // namespace bsp
