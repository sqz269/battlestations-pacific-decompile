#include "bsp/avoid_zone_layer_sampling.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#include <cstddef>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {

const double kLayerUpperInset = 2.0; // 00D7A308: 00 00 00 00 00 00 00 40

static_assert(sizeof(void*) == 4);
static_assert(offsetof(Cc13TerrainGridLayerNative, half_extent) == TerrainGridLayerOffsets::kHalfExtent);
static_assert(offsetof(Cc13TerrainGridLayerNative, dimension) == TerrainGridLayerOffsets::kDimension);
static_assert(offsetof(Cc13TerrainGridLayerNative, cell_size) == TerrainGridLayerOffsets::kCellSize);
static_assert(offsetof(Cc13TerrainGridLayerNative, file_scalar) == TerrainGridLayerOffsets::kFileScalar);
static_assert(offsetof(Cc13TerrainGridLayerNative, grid_first) == TerrainGridLayerOffsets::kGridVector + 4);
static_assert(offsetof(Cc13TerrainGridLayerNative, grid_last) == TerrainGridLayerOffsets::kGridVector + 8);

} // namespace

__declspec(naked) float* __fastcall layer_world_to_cell_kernel_00417fa0(
    const Cc13TerrainGridLayerNative*, float*, float, float) {
    // Adapt only incoming/outgoing Source ABI locations. The dead two float
    // arguments become the same u/upper scratch slots used by the native body.
    __asm {
        fld dword ptr [esp + 4]
        fadd dword ptr [ecx + 4]
        xorps xmm1, xmm1
        fdiv dword ptr [ecx + 0ch]
        fstp dword ptr [edx]
        fld dword ptr [esp + 8]
        fadd dword ptr [ecx + 4]
        movss xmm0, dword ptr [edx]
        comiss xmm1, xmm0
        movss dword ptr [esp + 4], xmm0
        fdiv dword ptr [ecx + 0ch]
        fstp dword ptr [edx + 4]
        fild dword ptr [ecx + 8]
        fld kLayerUpperInset
        fsub st(1), st(0)
        fxch
        fstp dword ptr [esp + 8]
        jbe upper_x
        movaps xmm0, xmm1
        jmp store_x
    upper_x:
        fld dword ptr [esp + 8]
        fld dword ptr [esp + 4]
        fcomip st(0), st(1)
        fstp st(0)
        jbe store_x
        movss xmm0, dword ptr [esp + 8]
    store_x:
        movss dword ptr [edx], xmm0
        fisubr dword ptr [ecx + 8]
        movss xmm0, dword ptr [edx + 4]
        comiss xmm1, xmm0
        movss dword ptr [esp + 4], xmm0
        fstp dword ptr [esp + 8]
        jbe upper_z
        movaps xmm0, xmm1
        movss dword ptr [edx + 4], xmm0
        mov eax, edx
        ret 8
    upper_z:
        fld dword ptr [esp + 8]
        fld dword ptr [esp + 4]
        fcomip st(0), st(1)
        fstp st(0)
        jbe store_z
        movss xmm0, dword ptr [esp + 8]
    store_z:
        movss dword ptr [edx + 4], xmm0
        mov eax, edx
        ret 8
    }
}

__declspec(naked) float __fastcall layer_cell_height_kernel_0041bae0(
    Cc13TerrainGridLayerNative*, std::int32_t, std::int32_t) {
    __asm {
        test edx, edx
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 8]
        push edi
        lea ecx, [eax - 1]
        jge clamp_x_upper
        xor edx, edx
        jmp clamp_z
    clamp_x_upper:
        cmp edx, ecx
        jle clamp_z
        mov edx, ecx
    clamp_z:
        mov edi, dword ptr [esp + 0ch]
        test edi, edi
        jge clamp_z_upper
        xor ecx, ecx
        jmp make_index
    clamp_z_upper:
        cmp edi, ecx
        jg make_index
        mov ecx, edi
    make_index:
        imul eax, ecx
        add eax, edx
        mov edi, eax
        mov eax, dword ptr [esi + 1ch]
        test eax, eax
        jz invalid_span
        mov ecx, dword ptr [esi + 20h]
        sub ecx, eax
        cmp edi, ecx
        jb read_cell
    invalid_span:
        call invoke_native_invalid_parameter_00bf6713 // 0041BB25; qualified CRT
    read_cell:
        mov eax, dword ptr [esi + 1ch]
        movzx ecx, byte ptr [eax + edi]
        mov dword ptr [esp + 0ch], ecx
        pop edi
        fild dword ptr [esp + 8]
        fmul dword ptr [esi + 10h]
        pop esi
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        ret 4
    }
}

AvoidZoneLayerSamplingHost::AvoidZoneLayerSamplingHost(
    const TerrainGridLayerRecord& layer) noexcept : record_(&layer) {
    // Field-by-field bit copies preserve NaNs/signed zeros without pretending
    // the semantic record or its std::vector has the native object layout.
    std::memcpy(&backing_.half_extent, &layer.half_extent, sizeof(float));
    backing_.dimension = layer.dimension;
    std::memcpy(&backing_.cell_size, &layer.cell_size, sizeof(float));
    std::memcpy(&backing_.file_scalar, &layer.file_scalar, sizeof(float));
    std::memcpy(&backing_.slope_limit, &layer.slope_limit, sizeof(float));
    backing_.grid_first = const_cast<std::uint8_t*>(layer.grid.data());
    backing_.grid_last = backing_.grid_first + layer.grid.size();
    backing_.grid_end = backing_.grid_last; // private used-span view; not ownership
}

std::optional<AvoidZoneLayerSamplingHost> AvoidZoneLayerSamplingHost::admit(
    const TerrainGridLayerRecord& layer) noexcept {
    if (layer.dimension < 2) return std::nullopt;
    const auto dimension = static_cast<std::uint64_t>(layer.dimension);
    const auto expected = dimension * dimension;
    if (expected > std::numeric_limits<std::uint32_t>::max() ||
        expected != layer.grid.size() || layer.grid.data() == nullptr) {
        return std::nullopt;
    }
    return AvoidZoneLayerSamplingHost(layer);
}

void AvoidZoneLayerSamplingHost::world_to_cell_00417fa0(
    const TerrainGridLayerRecord& layer, std::array<float, 2>& out,
    float world_x, float world_z) {
    if (&layer != record_) throw std::invalid_argument("layer sampling record identity");
    layer_world_to_cell_kernel_00417fa0(&backing_, out.data(), world_x, world_z);
}

float AvoidZoneLayerSamplingHost::cell_height_0041bae0(
    const TerrainGridLayerRecord& layer, std::int32_t x, std::int32_t z) {
    if (&layer != record_) throw std::invalid_argument("layer sampling record identity");
    return layer_cell_height_kernel_0041bae0(&backing_, x, z);
}

} // namespace bsp
