#pragma once
#include "bsp/native_shader_constant_header.hpp"
#include <memory>

namespace bsp {
// Borrow the existing actual scratch/string context unchanged. The instance
// getter419CA0 uses E17654, distinct from the field-name fallback108D6F2.
struct NativeShaderFieldInitializationContext {
    NativeShaderConstantHeaderContext& lines;
    const char* actual_instance_empty_00e17654;
};

// Host continuation metadata, not part of any actual8h/1Ch/B0h owner. Caller
// keeps every borrowed input/domain and this frame alive after failure, and
// excludes owner teardown. Original FH3 is not emitted. Work cannot be replayed.
struct NativeShaderFieldInitializationOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    enum class Step { none, field_name, instance_getter, descriptor_gate,
        prefix_limit, swizzle_construct, swizzle_assign, formatted_line, swizzle_release };
    Phase phase{Phase::fresh};
    Step step{Step::none};
    std::uint32_t function{}, native_site{}, row{}, completed_rows{}, limit{};
    NativeShaderFieldInitializationContext* context{};
    NativeMaterialProgramBuilderStorage* builder{};
    const NativeShaderDescriptorArray* fields{};
    const NativeString* instance{};
    const NativeShaderDescriptorStorage* captured_descriptor{};
    const NativeShaderFieldStorage* field_before_swizzle{};
    NativeString* swizzle_output{};
    NativeString swizzle;
    bool swizzle_live{}, swizzle_initialized{}, assignment_entered{}, assignment_returned{};
    const char* captured_field_name{};
    const char* instance_name{};
    const char* swizzle_text{};
    const char* right_field_name{};
    const char* left_field_name{};
    // Retains the actual existing B35110 child on a failed row. Successful
    // children have released native temporaries before the next child is made.
    std::unique_ptr<NativeShaderConstantHeaderOperation> line;
    NativeShaderFieldInitializationOperation() = default;
    ~NativeShaderFieldInitializationOperation();
    NativeShaderFieldInitializationOperation(const NativeShaderFieldInitializationOperation&) = delete;
    NativeShaderFieldInitializationOperation& operator=(const NativeShaderFieldInitializationOperation&) = delete;
    // Only AFTER manually resolving all native acquisitions, including the
    // line child. This explicit diagnostic retirement does not complete work.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// B34E90 ECX actual1Ch field; stack fresh output8h; EAX output; RET4. Clear
// output without freeing an old buffer. Current unsigned width1..4 selects
// x/xy/xyz/xyzw through unchanged41E350; all other values leave a null header.
// The existing string assignment failure boundary remains unchanged.
NativeString& construct_native_shader_field_swizzle_00b34e90(
    const NativeShaderFieldStorage&, NativeString&,
    NativeShaderFieldInitializationContext&, NativeShaderFieldInitializationOperation&);

// B357D0 ECX actualB0h builder; stack instance8h/list0Ch; RET8. Captures each
// current field name BEFORE calling existing419CA0 for the current instance.
// Appends two tabs, instance.field=0; and newline. Rechecks list count/data each
// row; ignores field type, width, mask and semantic. No clearing or owner copy.
void append_native_shader_zero_fields_00b357d0(NativeMaterialProgramBuilderStorage&,
    const NativeString& instance, const NativeShaderDescriptorArray&,
    NativeShaderFieldInitializationContext&, NativeShaderFieldInitializationOperation&);

// B35820 ECX actualB0h builder, RET. Initial descriptor70 byte1F gate and
// min(unsigned descriptor20, builder08) are captured before iteration. Reload
// current builder04 field pointers after swizzle construction, preserve both
// name reads and the captured loop bound, release swizzle after each B35110.
// Requires readable traversed pointers, including descriptor70 for gate reads.
void append_native_shader_vertex_decode_00b35820(NativeMaterialProgramBuilderStorage&,
    NativeShaderFieldInitializationContext&, NativeShaderFieldInitializationOperation&);
} // namespace bsp
