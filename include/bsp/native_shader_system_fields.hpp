#pragma once
#include "bsp/native_material_program_compiler.hpp"

namespace bsp {
// B34E20: ECX actual1Ch destination, six stack arguments (name, scalar,
// width, semantic, index, mask), EAX same, RET18. Clears the owning8h name,
// copies through the supplied SAME string domain, then writes08/0C/10/14/18.
// No refcount/vtable. Caller retains raw storage if name construction fails.
NativeShaderFieldStorage* construct_native_shader_field_00b34e20(void*,
    const NativeString&, std::int32_t scalar, std::int32_t width,
    std::int32_t semantic, std::int32_t index, std::uint32_t mask,
    NativeStringStorage&);

// One persistent host call frame, not another builder/field owner. The caller
// retains this frame, SAME builder and string domain after failure, and excludes
// builder teardown while an operation is pending. No implicit rollback/retry.
// Native FH3 has raw-slot/name cleanup states; this interface instead retains
// failed acquisitions for diagnosis and future native continuation integration.
struct NativeShaderSystemFieldsOperation final {
    enum class Phase { fresh, allocation, temporary_name, field_constructor,
        reserve, publication, name_cleanup, complete, failed, diagnostic_retired };
    NativeString temporary_name; // Address-stable actual8h header, reused after release.
    NativeMaterialProgramBuilderStorage* builder{};
    NativeStringStorage* strings{};
    const char* source_name{};
    NativeShaderFieldStorage* current_field{};
    std::uint32_t function{}, row{}, native_site{}, completed_rows{};
    Phase phase{Phase::fresh};
    bool temporary_initialized{}, temporary_live{};
    bool field_name_initialized{}, field_constructor_returned{}, field_published{};
    NativeShaderSystemFieldsOperation() = default;
    ~NativeShaderSystemFieldsOperation();
    NativeShaderSystemFieldsOperation(const NativeShaderSystemFieldsOperation&) = delete;
    NativeShaderSystemFieldsOperation& operator=(const NativeShaderSystemFieldsOperation&) = delete;
    // Diagnostic harness only, AFTER all native acquisitions have explicitly
    // been resolved and tracked ownership cleared. Never native completion.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Complete normal readable-domain B35BE0/B372D0: ECX actualB0h builder, RET.
// Append sixteen independent1Ch fields to CURRENT10/34 arrays without clearing
// or deduplicating. Allocate field, construct temporary name, copy field name,
// reserve only at count==capacity, publish slot, increment CURRENT count, then
// release temporary name. Existing rows/scalars/descriptor identities survive.
// Actual BF681B service throws on allocation failure; no successful substitute
// field/code/pass is returned. Neither entry reads descriptor/source-mode flags.
void append_native_shader_vertex_system_fields_00b35be0(
    NativeMaterialProgramBuilderStorage&, NativeStringStorage&, NativeShaderSystemFieldsOperation&);
void append_native_shader_pixel_system_fields_00b372d0(
    NativeMaterialProgramBuilderStorage&, NativeStringStorage&, NativeShaderSystemFieldsOperation&);
} // namespace bsp
