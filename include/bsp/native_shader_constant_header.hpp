#pragma once
#include "bsp/native_material_program_compiler.hpp"
#include "bsp/native_system_constant_registry.hpp"
#include <cstdarg>

namespace bsp {
// These references borrow the application's actual cells/buffer. No registry,
// string pool, scratch buffer, or register-limit snapshot is created here.
// Scratch and strings must have enough accessible storage for native sprintf;
// concurrent/reentrant formatting has the original shared-buffer restriction.
struct NativeShaderConstantHeaderContext {
    NativeStringStorage& strings;
    void* const volatile& actual_registry_0108fe94;
    const volatile std::uint32_t& actual_register_limit_00e13078;
    char* actual_format_scratch_0108d6f8;
    const char* actual_empty_0108d6f2;
};

// Stable host continuation state, never added to the B0h builder/20h records.
// A failure retains actual temporary headers and captured buffer preimages.
// The caller must keep frame, context, builder, registry, rows and strings alive
// and prohibit their retirement until completion or explicit diagnostic cleanup.
// This is NOT original FH3 unwinding; failed work cannot be replayed.
struct NativeShaderConstantHeaderOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    enum class Step { none, registry, prefix, prefix_copy, number_format,
        number_resize, number_copy, number_release, suffix_append,
        temporary_release, declaration, line_format, line_resize, line_append,
        line_release, newline_resize, newline_append, newline_release,
        suffix_release, cursor };
    Phase phase{Phase::fresh};
    Step step{Step::none};
    std::uint32_t function{}, native_site{}, completed_rows{}, cursor{};
    NativeShaderConstantHeaderContext* context{};
    NativeMaterialProgramBuilderStorage* builder{};
    NativeCompiledShaderConstants* list{};
    const NativeCompiledShaderConstantStorage* record{};
    NativeString* destination{};
    NativeString suffix, temporary, decimal_temporary, line_temporary;
    char decimal_text[52]{};
    char* suffix_data{};
    char* decimal_data{};
    char* line_data{};
    std::uint32_t decimal_length{}, line_length{}, old_length{}, append_length{};
    bool suffix_live{}, temporary_live{}, decimal_live{}, line_live{};
    bool decimal_captured{}, line_captured{};
    NativeShaderConstantHeaderOperation() = default;
    ~NativeShaderConstantHeaderOperation();
    NativeShaderConstantHeaderOperation(const NativeShaderConstantHeaderOperation&) = delete;
    NativeShaderConstantHeaderOperation& operator=(const NativeShaderConstantHeaderOperation&) = delete;
    // Only after all retained native acquisitions were explicitly cleaned up.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// ECX actual20h, RET; same native header/words, with current volatile reads.
std::uint32_t native_system_constant_second_dimension_00b5b840(const NativeCompiledShaderConstantStorage&) noexcept;
std::uint32_t native_system_constant_first_dimension_00b5b850(const NativeCompiledShaderConstantStorage&) noexcept;
std::uint32_t native_system_constant_array_count_00b5b860(const NativeCompiledShaderConstantStorage&) noexcept;
// ECX actual10h registry, EAX owner+04, RET. No global reload in this getter.
NativeCompiledShaderConstants& native_system_constant_list_00b5b890(void* actual_registry) noexcept;

// 5F1840 ECX fresh8h, stacked signed DWORD, EAX this, RET4. Native decimal
// buffer and BOTH pooled string headers survive a borrowed storage failure.
NativeString& construct_native_shader_decimal_005f1840(NativeString&, std::int32_t,
    NativeShaderConstantHeaderContext&, NativeShaderConstantHeaderOperation&);
// B35110 is cdecl(destination, format, ...), caller cleanup, RET. Formats into
// SAME0108D6F8, appends a pooled copy, then a separate pooled newline. The CRT
// remains the explicit host runtime boundary; this interface is not native ABI.
void append_native_shader_line_00b35110(NativeString&, NativeShaderConstantHeaderContext&,
    NativeShaderConstantHeaderOperation&, const char* format, ...);

// B38C60 ECX actualB0h builder, stack actual20h record/register DWORD, RET8.
// Complete normal body; unsigned type selection, signed %i and register test.
void append_native_system_constant_00b38c60(NativeMaterialProgramBuilderStorage&,
    const NativeCompiledShaderConstantStorage&, std::int32_t register_index,
    NativeShaderConstantHeaderContext&, NativeShaderConstantHeaderOperation&);
// B38FF0 ECX builder, stacked flag LOW BYTE, RET4. Captures initial registry's
// actual+04 array once; current count/data and E13078 reload each iteration.
// Cursor wraps DWORD and checks its start only. No clear, vector projection,
// descriptor filtering, compiler invocation, or registry ownership is added.
void append_native_system_constant_header_00b38ff0(NativeMaterialProgramBuilderStorage&,
    std::uint8_t explicit_registers, NativeShaderConstantHeaderContext&,
    NativeShaderConstantHeaderOperation&);
} // namespace bsp
