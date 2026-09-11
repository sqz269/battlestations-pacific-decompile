#include "bsp/native_material_parameters.hpp"
#include <cstring>
#include <new>
#include <stdexcept>
#include <type_traits>

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeString) == 8);
static_assert(sizeof(NativeMaterialParameterStorage) == 0x84);
static_assert(offsetof(NativeMaterialParameterStorage, source_08) == 8);
static_assert(offsetof(NativeMaterialParameterStorage, word_count_0c) == 0x0c);
static_assert(offsetof(NativeMaterialParameterStorage, matrix_10) == 0x10);
static_assert(offsetof(NativeMaterialParameterStorage, vertex_registers_14) == 0x14);
static_assert(offsetof(NativeMaterialParameterStorage, pixel_registers_4c) == 0x4c);
static_assert(std::is_trivially_destructible_v<NativeMaterialParameterStorage>);
namespace {
template<class T> T read(const void* pointer, std::size_t offset = 0) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(pointer) + offset, sizeof(value));
    return value;
}
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
bool same_name(const void* entry, const void* name, NativeMaterialParameterAccess& access) {
    require(entry && name, "native material binding requires live string headers");
    const auto requested_length = read<std::uint32_t>(name);
    const auto entry_length = read<std::uint32_t>(entry);
    if (entry_length != requested_length) return false;
    if (entry_length == 0) return true;
    require(access.compare_names_00bf7fbf != nullptr, "native material binding requires the actual CRT name comparison");
    return access.compare_names_00bf7fbf(read<const char*>(entry, 4), read<const char*>(name, 4)) == 0;
}
std::int32_t find_register(void* stage, const void* name, NativeMaterialParameterAccess& access) {
    require(stage != nullptr, "nonnull native effect pass requires both compiled stage owners");
    auto cursor = reinterpret_cast<std::uintptr_t>(read<const void*>(stage, 0x78));
    auto end = cursor + read<std::uint32_t>(stage, 0x7c) * 0x20u;
    while (cursor != end) {
        const auto* constant = reinterpret_cast<const void*>(cursor);
        if (same_name(native_shader_constant_name_00b5b820(constant), name, access))
            return native_shader_constant_register_00b5b870(constant);
        // Preserve the native cursor and reload the CURRENT end before its
        // increment. A metadata copy or index into a replaced base differs.
        end = reinterpret_cast<std::uintptr_t>(read<const void*>(stage, 0x78))
            + read<std::uint32_t>(stage, 0x7c) * 0x20u;
        cursor += 0x20u;
    }
    return -1;
}
std::int32_t parameter_count(const NativeMaterialStorage& material) {
    const auto count = material.parameter_count_100;
    require(count >= 0 && count <= 32, "native material parameter count is outside the actual32 slots");
    return count;
}
NativeMaterialParameterStorage* initialize_parameter(void* slot) {
    if (!slot) return nullptr;
    auto* parameter = ::new (slot) NativeMaterialParameterStorage;
    // B44ECF..B44EFA: NativeString construction writes only the two name words.
    // Interleaved register stores preserve original order; +0C/+11..13 untouched.
    for (std::size_t selector = 0; selector != 14; ++selector) {
        parameter->vertex_registers_14[selector] = -1;
        parameter->pixel_registers_4c[selector] = -1;
    }
    parameter->source_08 = nullptr;
    parameter->matrix_10 = 0;
    return parameter;
}
} // namespace

const void* native_shader_constant_name_00b5b820(const void* constant) noexcept {
    return static_cast<const std::byte*>(constant) + 0x14;
}
std::int32_t native_shader_constant_register_00b5b870(const void* constant) noexcept {
    return read<std::int32_t>(constant);
}
void* allocate_native_material_parameter_slot_00b18790(NativeMaterialParameterPool& pool) {
    return pool.allocate_slot_00b185a0();
}

NativeMaterialParameterStorage* bind_native_material_parameter_00b44d60(
    void* effect, NativeMaterialStorage& material, NativeMaterialParameterStorage* parameter,
    const void* name, const void* source, std::uint32_t count, std::uint8_t matrix,
    NativeMaterialParameterAccess& access) {
    require(effect != nullptr, "native parameter binding requires an actual effect");
    bool updated = false;
    for (std::size_t selector = 0; selector != 14; ++selector) {
        const auto offset = 0xc8 + selector * 4;
        void* pass = read<void*>(effect, offset);
        if (!pass) continue;
        const auto vertex = find_register(read<void*>(pass, 0x70), name, access);
        // B44E29 reloads the pass before acquiring its pixel stage.
        pass = read<void*>(effect, offset);
        require(pass != nullptr, "native pass lifetime changed during name comparison");
        const auto pixel = find_register(read<void*>(pass, 0x74), name, access);
        if (vertex < 0 && pixel < 0) continue;
        if (!updated) {
            if (!parameter) {
                parameter = initialize_parameter(allocate_native_material_parameter_slot_00b18790(access.parameter_slots));
                // Native publishes the pointer/count before string resize.
                const auto index = parameter_count(material);
                require(index < 32, "material parameter count changed during allocation");
                material.parameters_80[static_cast<std::size_t>(index)] = parameter;
                ++material.parameter_count_100;
            }
            require(parameter != nullptr, "native parameter allocation returned null after publication");
            if (parameter != name) {
                require(name != nullptr, "native parameter update requires a string header");
                resize_native_string_header_0041dd40(parameter, access.parameter_names,
                    read<std::uint32_t>(name), true);
                // Native reloads both headers AFTER the real storage callback.
                if (read<std::uint32_t>(name) != 0) {
                    const auto bytes = read<std::uint32_t>(parameter);
                    const auto* source_data = read<const void*>(name, 4);
                    auto* destination_data = read<void*>(parameter, 4);
                    // As in the existing actual-header string adapter, omit
                    // the zero-byte CRT operation with possibly null buffers.
                    if (bytes != 0) std::memcpy(destination_data, source_data, bytes);
                }
            }
            parameter->source_08 = source;
            parameter->word_count_0c = count;
            parameter->matrix_10 = matrix;
            updated = true;
        }
        parameter->vertex_registers_14[selector] = vertex;
        parameter->pixel_registers_4c[selector] = pixel;
    }
    return parameter;
}

NativeMaterialParameterStorage* register_native_material_parameter_00b17e10(
    NativeMaterialStorage& material, const void* name, const void* source,
    std::uint32_t count, std::uint8_t matrix, NativeMaterialParameterAccess& access) {
    if (!material.effect_7c) return nullptr;
    NativeMaterialParameterStorage* parameter = nullptr;
    for (std::int32_t index = 0; index != parameter_count(material); ++index) {
        require(index < 32, "material parameter end moved behind the search cursor");
        auto* candidate = static_cast<NativeMaterialParameterStorage*>(material.parameters_80[static_cast<std::size_t>(index)]);
        require(candidate != nullptr, "counted native parameter slot must be nonnull");
        if (same_name(candidate, name, access)) {
            parameter = static_cast<NativeMaterialParameterStorage*>(material.parameters_80[static_cast<std::size_t>(index)]);
            break;
        }
    }
    void* effect = material.effect_7c; // native reload before current+10
    require(effect && read<std::uint32_t>(effect) == 0x00d61a00 &&
        access.effect_table_00d61a00 && access.effect_table_00d61a00[4] == 0x00b44d60,
        "native parameter dispatch requires the current D61A00/B44D60 effect profile");
    return bind_native_material_parameter_00b44d60(effect, material, parameter, name, source, count, matrix, access);
}
} // namespace bsp
