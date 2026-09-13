#include "bsp/native_shader_sampler_declarations.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T read(const void* owner, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const char*>(owner) + offset);
}
std::int32_t signed_slot(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof result);
    return result;
}
void append(NativeMaterialProgramBuilderStorage& builder,
    NativeShaderConstantHeaderContext& context,
    NativeShaderSamplerDeclarationsOperation& a, bool vertex) {
    using Phase = NativeShaderSamplerDeclarationsOperation::Phase;
    if (a.phase != Phase::fresh)
        throw std::logic_error("sampler declaration operation is one-shot");
    a.phase = Phase::running;
    a.function = vertex ? 0x00b38080u : 0x00b37ef0u;
    a.builder = &builder;
    a.context = &context;
    try {
        for (const auto offset : {0x70u, 0x74u}) {
            a.descriptor_offset = offset;
            a.index = 0;
            a.descriptor = read<NativeShaderDescriptorStorage*>(&builder, offset);
            if (read<std::uint32_t>(a.descriptor, 0xc8) == 0) continue;
            do {
                a.sampler_data = read<void*>(a.descriptor, 0xc4);
                a.sampler = read<NativeShaderSamplerStorage*>(a.sampler_data, a.index * 4u);
                a.sampled_vertex_byte = read<std::uint8_t>(a.sampler, 0xc);
                if ((a.sampled_vertex_byte != 0) == vertex) {
                    a.sampled_type = read<std::uint32_t>(a.sampler, 0x10);
                    const char* format = nullptr;
                    switch (a.sampled_type) {
                    case 1: format = "sampler1D\t%s\t\t: register(s%i);"; break;
                    case 2: format = "sampler2D\t%s\t\t: register(s%i);"; break;
                    case 4: format = "sampler3D\t%s\t\t: register(s%i);"; break;
                    case 3: format = "samplerCUBE\t%s\t\t: register(s%i);"; break;
                    default: break;
                    }
                    if (format) {
                        a.name = read<const char*>(a.sampler, 8);
                        if (!a.name) a.name = context.actual_empty_0108d6f2;
                        a.native_site = vertex ? (offset == 0x70 ? 0x00b38128u : 0x00b381e8u)
                                               : (offset == 0x70 ? 0x00b37f98u : 0x00b38058u);
                        // Only a completed child is replaced. A thrown child
                        // remains in the persistent parent for explicit cleanup.
                        a.line.emplace();
                        append_native_shader_line_00b35110(builder.source_4c, context,
                            *a.line, format, a.name, signed_slot(a.slot));
                        ++a.emitted_lines;
                    }
                    ++a.slot;
                }
                a.descriptor = read<NativeShaderDescriptorStorage*>(&builder, offset);
                ++a.index;
                ++a.completed_rows;
            } while (a.index < read<std::uint32_t>(a.descriptor, 0xc8));
        }
        a.phase = Phase::complete;
    } catch (...) { a.phase = Phase::failed; throw; }
}
} // namespace

NativeShaderSamplerDeclarationsOperation::~NativeShaderSamplerDeclarationsOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
void NativeShaderSamplerDeclarationsOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::failed) {
        if (line) line->acknowledge_diagnostic_cleanup();
        phase = Phase::diagnostic_retired;
    }
}
void append_native_pixel_sampler_declarations_00b37ef0(
    NativeMaterialProgramBuilderStorage& builder, NativeShaderConstantHeaderContext& context,
    NativeShaderSamplerDeclarationsOperation& operation) {
    append(builder, context, operation, false);
}
void append_native_vertex_sampler_declarations_00b38080(
    NativeMaterialProgramBuilderStorage& builder, NativeShaderConstantHeaderContext& context,
    NativeShaderSamplerDeclarationsOperation& operation) {
    append(builder, context, operation, true);
}
} // namespace bsp
