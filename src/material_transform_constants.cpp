#include "bsp/material_transform_constants.hpp"
#include "bsp/compiled_material.hpp"
#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
bool write_rows(std::vector<float>& words, std::uint8_t index, std::uint8_t rows,
    const CameraMatrix& matrix, std::string& error) {
    if (rows < 2 || rows > 4) return true;
    const auto offset = static_cast<std::size_t>(index) * 4;
    const auto count = static_cast<std::size_t>(rows) * 4;
    if (offset > words.size() || count > words.size() - offset) {
        error = "Material transform constants exceed the shared output capacity";
        return false;
    }
    float* destination = words.data() + offset;
    if (rows == 2) {
        // Native MOVSS-to-stack then REP MOVSD: retain raw NaN/zero bits.
        std::uint32_t temporary[8];
        for (unsigned row = 0; row < 2; ++row) {
            for (unsigned column = 0; column < 4; ++column) {
                std::memcpy(temporary + row * 4 + column,
                    matrix.data() + column * 4 + row, sizeof(float));
            }
        }
        std::memcpy(destination, temporary, sizeof(temporary));
    } else {
        // Native FLD/FSTP per destination word; no matrix-wide temporary.
        // Inherit caller x87 control/exception state, including sNaN quieting.
        for (unsigned row = 0; row < rows; ++row) {
            for (unsigned column = 0; column < 4; ++column) {
                const float* source = matrix.data() + column * 4 + row;
                float* output = destination + row * 4 + column;
                __asm {
                    mov eax, source
                    mov ecx, output
                    fld dword ptr [eax]
                    fstp dword ptr [ecx]
                }
            }
        }
    }
    return true;
}
}

const CameraMatrix& get_transform_inverse_world_00b6e0d0(CameraTransform& transform) {
    return get_camera_view_00b6fcb0(transform);
}

bool pack_material_world_constants_00b42a7c(const CompiledMaterialPass& pass,
    CameraTransform& transform, std::vector<float>& vertex_words,
    std::vector<float>& pixel_words, std::string& error) {
    if (pass.vb.registers[0] != 0xff) {
        const auto rows = pass.vb.counts[0]; // Captured before refresh.
        if ((transform.valid_flags & 2u) == 0)
            refresh_camera_world_00b6db70(transform);
        // Native reloads the selected register after refresh.
        if (!write_rows(vertex_words, pass.vb.registers[0], rows, transform.world, error))
            return false;
    }
    if (pass.pb.registers[0] != 0xff) {
        const auto rows = pass.vb.counts[0]; // Native PS deliberately reads VS metadata.
        if ((transform.valid_flags & 2u) == 0)
            refresh_camera_world_00b6db70(transform);
        if (!write_rows(pixel_words, pass.pb.registers[0], rows, transform.world, error))
            return false;
    }
    return true;
}

bool pack_material_inverse_world_constants_00b42ef9(const CompiledMaterialPass& pass,
    CameraTransform& transform, std::vector<float>& vertex_words, std::string& error) {
    if (pass.vb.registers[1] == 0xff) return true;
    const auto rows = pass.vb.counts[1];
    const auto index = pass.vb.registers[1]; // Both captured before cache helper.
    const auto& inverse = get_transform_inverse_world_00b6e0d0(transform);
    return write_rows(vertex_words, index, rows, inverse, error);
}
}
