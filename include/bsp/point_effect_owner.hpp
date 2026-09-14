#pragma once

#include "bsp/camera_affine.hpp"
#include "bsp/camera_transform.hpp"
#include "bsp/effect_admission.hpp"
#include "bsp/render_command_queue.hpp"
#include "bsp/system_time_constants.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// Borrow the actual manager+08 slot. Reload occurs AFTER allocation; neither
// this view nor the wrapper creates a manager or retains its referenced owner.
struct PointAnnouncementManagerView {
    void* const& owner_08;
};

class PointAnnouncementConstruction {
public:
    virtual ~PointAnnouncementConstruction() = default;
    virtual void* allocate_00bf681b(std::size_t native_bytes) = 0;
    virtual void free_00bf65ac(void* raw) noexcept = 0;
    // REQUIRED unreconstructed constructor. Construct the actual 30h owner in
    // raw storage, including 00496920 and the descriptor/random/vector body.
    // Successful native EAX is that same storage. On exception unwind its
    // constructed subobjects, but leave raw deallocation to the caller.
    // All generated records copy captured_xyz into their own +08/+0C/+10.
    // Neither XYZ input may be retained. secondary_xyz feeds record+14..1C.
    virtual void construct_0049c000(void* raw, void* owner_08,
        const std::array<float, 3>& captured_xyz,
        const std::array<float, 3>& secondary_xyz, std::int32_t descriptor) = 0;
};

// Complete 0049C940 orchestration, with the constructor explicitly required.
// Native ECX manager; stack capturedXYZ, secondaryXYZ, signed descriptor;
// EAX allocated owner or null; RET0C. No added retain/registration policy.
void* create_point_announcement_0049c940(PointAnnouncementManagerView,
    const std::array<float, 3>& captured_xyz,
    const std::array<float, 3>& secondary_xyz, std::int32_t descriptor,
    PointAnnouncementConstruction&);

// Same complete wrapper over the actual native manager address. Its +08
// field is read only AFTER successful allocation; negative descriptor and
// null allocation may return with a null manager without dereferencing it.
// This overload adds no manager owner, registry, validation or fallback.
void* create_point_announcement_0049c940(void* actual_manager,
    const std::array<float, 3>& captured_xyz,
    const std::array<float, 3>& secondary_xyz, std::int32_t descriptor,
    PointAnnouncementConstruction&);

// ONLY 0049C1DB..0049C1F3 inside 0049C000, not the full constructor.
// actual_record_xyz_08 is the existing selected record's writable XYZ storage,
// after its native vector validation/reload. Ordered x87 FLD/FSTP copies retain
// caller FP behavior and overlapping-copy order; no allocation or retention.
void copy_point_record_xyz_0049c1db(float* actual_record_xyz_08,
    const std::array<float, 3>& captured_xyz);

struct PointEffectManagerView {
    // Actual 00866440 manager+04, NOT the lifetime manager's +10 field.
    // The canonical section projection has the same OS section and +18 count.
    SystemSingletonCriticalSection* const& section_04;
};

class PointEffectConstruction {
public:
    virtual ~PointEffectConstruction() = default;
    virtual PointEffectManagerView manager_00866440() = 0;
    // Pure load of CURRENT [E188A8]+19FC after optional parent refresh. Do not
    // cache the root owner or fabricate a camera; binding must stay alive.
    virtual CameraTransform& reference_transform_e188a8_19fc() = 0;
    // REQUIRED actual 0086A650, including template-entry visibility writes.
    // A success-assuming predicate is not an implementation of this boundary.
    virtual bool eligible_0086a650(RenderCommandReference& actual_template,
        EffectPointView test_xyz, CameraTransform& reference) = 0;
    virtual void* allocate_00bf681b(std::size_t native_bytes) = 0;
    virtual void free_00bf65ac(void* raw) noexcept = 0;
    // REQUIRED actual 008680B0. The caller transfers ONE extra template ref,
    // consumed on success AND exception (C94FD0 -> 0041DE40). The constructor
    // separately retains the template in actual owner+84. Success transfers
    // the raw allocation and returns its canonical companion borrowing the
    // actual +04 count (native initial value1); never a new diagnostic count.
    // Native stack: template, parent, third word, matrix, transform byte,
    // option byte, tail word. Matrix is the original object supplied by the
    // entrypoint (caller matrix or mutable F87610), never a snapshot.
    // On throw unwind constructed subobjects/argument; caller frees raw.
    virtual RenderCommandReference& construct_008680b0(void* raw,
        RenderCommandReference* consumed_template, CameraTransform* parent,
        std::uint32_t third_word, const CameraMatrix& actual_matrix,
        std::uint8_t transform_byte, std::uint8_t option_byte,
        std::uint32_t tail_word) = 0;
};

// Complete 008689C0 orchestration for the canonical nonthrowing terminal-ref
// domain. Native ECX output-slot, EDX parent; stack CONSUMED template, XYZ,
// transform byte, option byte, tail word; EAX output-slot; RET14. The decompile
// loses the last two stack arguments. Null parent is valid only if transform=0.
// output is fresh storage: native overwrites it without releasing a prior word.
// On a thrown getter/eligibility/allocation/construction failure it is untouched.
// Actual globals F87640/44/48 are matrix[12/13/14], copied even for null template.
// A nonzero transform byte affects only eligibility XYZ, not those global stores.
// Input ownership is consumed on every path; release occurs AFTER unlocking.
// Bind references to their actual atomic counts and virtual+0 terminal actions.
// Terminal actions and deallocation must not throw in this C++ host domain.
RenderCommandReference*& create_point_effect_008689c0(
    RenderCommandReference*& output, CameraTransform* parent,
    RenderCommandReference* consumed_template,
    const std::array<float, 3>& captured_xyz, std::uint8_t transform_byte,
    std::uint8_t option_byte, std::uint32_t tail_word,
    CameraMatrix& actual_matrix_00f87610, PointEffectConstruction&);

// Additional complete native creation entrypoints. All require a nonnull
// CONSUMED template reference; unlike8689C0 they call admission unconditionally.
// The fresh output word is untouched on a throwing getter/admission/allocation/
// constructor failure; input release occurs after the captured lock is left.
// 868420: ECX out, EDX third_word; stack template,matrix,option,tail; RET10.
// Admission borrows original matrix+30; constructor parent=null, transform=0.
RenderCommandReference*& create_point_effect_matrix_00868420(
    RenderCommandReference*& output, std::uint32_t third_word,
    RenderCommandReference& consumed_template, const CameraMatrix& original_matrix,
    std::uint8_t option_byte, std::uint32_t tail_word, PointEffectConstruction&);
// 8685E0: ECX out, EDX third_word; stack template,XYZ,option,tail; RET10.
// Copy XYZ sequentially to F87640/44/48 under lock BEFORE admission. Admission
// receives the original live XYZ address; construction reads CURRENT F87610.
RenderCommandReference*& create_point_effect_position_008685e0(
    RenderCommandReference*& output, std::uint32_t third_word,
    RenderCommandReference& consumed_template, EffectPointView original_xyz,
    std::uint8_t option_byte, std::uint32_t tail_word,
    CameraMatrix& actual_matrix_00f87610, PointEffectConstruction&);
// 8687C0: ECX out, EDX parent; stack template,matrix,transform,option,tail; RET14.
// If transform!=0, require parent and compute full4x4 matrix*parent.world solely
// for admission. Construction still receives the ORIGINAL live matrix, parent,
// third_word=0 and original low transform byte. With transform0, admission
// borrows original matrix+30 directly and parent may be null.
RenderCommandReference*& create_point_effect_parent_matrix_008687c0(
    RenderCommandReference*& output, CameraTransform* parent,
    RenderCommandReference& consumed_template, const CameraMatrix& original_matrix,
    std::uint8_t transform_byte, std::uint8_t option_byte, std::uint32_t tail_word,
    PointEffectConstruction&);

} // namespace bsp
