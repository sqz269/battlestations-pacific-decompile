#pragma once
#include "bsp/native_shader_constant_header.hpp"
#include <memory>

namespace bsp {
// Stable host continuation metadata, never inserted into the actual 8h strings,
// 1Ch fields, 0Ch pointer list or B0h builder. The caller keeps every borrowed
// owner/context alive and excludes retirement while a call is running/failed.
// A failed frame cannot be replayed; there is no recovered native FH3 unwind.
// Existing 41E870/41E350/4261A0/711370 helper cleanup remains its own boundary:
// an entered-but-not-returned header does NOT prove a buffer is still owned.
struct NativeShaderStructDeclarationOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    enum class Step { none, text_construct, text_append, text_release,
        punctuation_resize, punctuation_append, punctuation_release,
        type_assign, width_number, width_join, field_name, semantic_append,
        semantic_number, formatted_line, field_format, field_release, vpos };
    Phase phase{Phase::fresh};
    Step step{Step::none};
    std::uint32_t function{}, native_site{}, row{}, completed_rows{};
    NativeShaderConstantHeaderContext* context{};
    NativeMaterialProgramBuilderStorage* builder{};
    const NativeShaderDescriptorArray* fields{};
    const NativeShaderFieldStorage* field{};
    NativeString* destination{};
    const NativeString* source{};
    const char* cstring{};
    NativeString temporary, number, joined, punctuation, field_output;
    bool temporary_entered{}, temporary_returned{}, temporary_live{};
    bool number_entered{}, number_returned{}, number_live{};
    bool joined_entered{}, joined_returned{}, joined_live{};
    bool punctuation_live{}, punctuation_captured{}, field_output_live{};
    char* captured_data{};
    std::uint32_t captured_length{}, append_length{}, old_length{};
    std::uint8_t semantics{}, allow_vpos{};
    // The exact existing B35110 child survives failure. Completed children may
    // be replaced for the next line; their native acquisitions are then gone.
    std::unique_ptr<NativeShaderConstantHeaderOperation> formatted_line;
    NativeShaderStructDeclarationOperation() = default;
    ~NativeShaderStructDeclarationOperation();
    NativeShaderStructDeclarationOperation(const NativeShaderStructDeclarationOperation&) = delete;
    NativeShaderStructDeclarationOperation& operator=(const NativeShaderStructDeclarationOperation&) = delete;
    // Diagnostic use only AFTER manually resolving all retained acquisitions,
    // including the existing formatted-line child. Does not complete native work.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// B34F20 ECX actual8h destination, stack C string, RET4. Constructs a pooled
// copy, captures its length/data across destination resize, releases that copy,
// then constructs/appends/releases a separate one-character pooled newline.
void append_native_shader_cstring_line_00b34f20(NativeString&, const char*,
    NativeShaderConstantHeaderContext&, NativeShaderStructDeclarationOperation&);
// B35030 ECX actual8h destination, stack actual8h source, RET4. Captures length
// before resize but reloads source data afterward (including self-append).
void append_native_shader_string_line_00b35030(NativeString&, const NativeString&,
    NativeShaderConstantHeaderContext&, NativeShaderStructDeclarationOperation&);
// B385B0 ECX actual1Ch field, stack fresh output8h/semantics low byte; EAX
// output; RET8. Clears output without releasing prior storage. Unknown scalar
// omits the type; unknown semantic omits its token but still emits unsigned
// index. Width/index use the existing UNSIGNED 711370 constructor.
NativeString& format_native_shader_field_00b385b0(const NativeShaderFieldStorage&,
    NativeString&, std::uint8_t semantics, NativeShaderConstantHeaderContext&,
    NativeShaderStructDeclarationOperation&);
// B38B50 ECX actualB0h builder; stack unsigned start, actual8h name, actual0Ch
// pointer list, semantics low byte, allow-vpos low byte; RET14h. Append only.
// Reloads list count/data each row. After rows, reads CURRENT builder70/74+30
// low bytes, short-circuiting as native. All traversed pointers must be valid;
// no descriptor snapshot, field projection, filtering or null fallback added.
void append_native_shader_struct_00b38b50(NativeMaterialProgramBuilderStorage&,
    std::uint32_t start, const NativeString& name, const NativeShaderDescriptorArray&,
    std::uint8_t semantics, std::uint8_t allow_vpos,
    NativeShaderConstantHeaderContext&, NativeShaderStructDeclarationOperation&);
} // namespace bsp
