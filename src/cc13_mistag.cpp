// Library-tagged game rows, packet cc13-mistag (marker cc13_mistag). docs/CC13_MISTAG.md.
#include "bsp/cc13_mistag.hpp"

namespace bsp::cc13_mistag {
namespace {

template <class T>
T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}

void* singleton_scalar_deleting(MistagHost& host, void* object, std::uint32_t flags,
    std::uint32_t global) {
    host.clear_global(global);
    at<std::uint32_t>(object, 0) = kSingletonBaseVtable;
    if ((flags & 1) != 0) host.free_00bf65ac(object);
    return object;
}

}  // namespace

// Each row read from the disk bytes: B8 imm32 C3. The slot is the only .rdata dword equal to
// the routine's address (two for 0085EA30).
const TrivialGetter kTrivialGetters[kTrivialGetterCount] = {
    {0x008561D0, 0x00D0C410, 0x00CE544C, "Torpedo"},
    {0x0085EA30, 0x00D0D158, 0x00CEA74C, "WaterMine"},          // also slot 00D7CE88
    {0x009A28A0, 0x00D1F4FC, 0x00CFB630, "closetoship"},
    {0x009ADD20, 0x00D1FCCC, 0x00F8A424, nullptr},
    {0x009B33C0, 0x00D1FFB4, 0x00D20008, "LAND"},
    {0x009B33E0, 0x00D1FFDC, 0x00E08FA0, nullptr},
    {0x009B7B60, 0x00D2026C, 0x00F8A474, nullptr},
    {0x009BA820, 0x00D2039C, 0x00D203F0, "conditioned"},
    {0x009BAA90, 0x00D20494, 0x00D204E8, "gainaltitude"},
    {0x009C3B80, 0x00D20C1C, 0x00E08F78, nullptr},
    {0x009C79A0, 0x00D20E54, 0x00E08F20, nullptr},
    {0x009CC3F0, 0x00D210F4, 0x00CFB4D0, "strafe"},
    {0x009CFA00, 0x00D2123C, 0x00D21290, "takeoff"},
    {0x009DAC20, 0x00D215BC, 0x00E08F70, nullptr},
    {0x00749000, 0x00CFF79C, 0x00CFF7BC, "MLandFort"},
    {0x00951AE0, 0x00D19C00, 0x00D19C20, "MReconPlane"},
    {0x00951B30, 0x00D19C3C, 0x00D19C5C, "MPlaneFighter"},
    {0x00951C10, 0x00D19C7C, 0x00D19F34, "MPlaneDiveBomber"},
    {0x00951CF0, 0x00D19F58, 0x00D1A20C, "MPlaneTorpedoBomber"},
    {0x00951DD0, 0x00D1A230, 0x00D1A4E4, "MPlaneKamikaze"},
    {0x00951E20, 0x00D1A504, 0x00D1A524, "MPlaneBomber"},
    {0x00951E70, 0x00D1A544, 0x00D1A564, "MCommandBuilding"},
    {0x007AB9F0, 0x00D05088, 0x00CFA6C4, "Paratrooper"},
};

std::uint32_t trivial_getter_result(std::uint32_t address) noexcept {
    for (const TrivialGetter& g : kTrivialGetters)
        if (g.address == address) return g.value;
    return 0;
}

// 00735880..007358A8 (Ghidra CG_scalar_deleting_dtor_00735880; vtable slot 00CFEA04). The
// out-of-line base 00735770 is an undefined chunk.
void* singleton_scalar_deleting_destruct_00735880(MistagHost& host, void* object, std::uint32_t flags) {
    return singleton_scalar_deleting(host, object, flags, 0x010904F8);
}

// 00735900..00735928 (slot 00CFEA0C); out-of-line base 007357B0 undefined.
void* singleton_scalar_deleting_destruct_00735900(MistagHost& host, void* object, std::uint32_t flags) {
    return singleton_scalar_deleting(host, object, flags, 0x0109029C);
}

// 00866090..008660B8 (slot 00D0D3C8); out-of-line base 00866060 undefined.
void* singleton_scalar_deleting_destruct_00866090(MistagHost& host, void* object, std::uint32_t flags) {
    return singleton_scalar_deleting(host, object, flags, 0x00F87658);
}

// 00737110..00737147 (slot 00CFEA2C). The singleton base is the +4h subobject (the image takes
// 0 for a null this and would then store through 0; not modelled). Order as in the listing:
// global, +4h vtable, +0h vtable 00CFE9F4, then _free.
void* msar_provider_scalar_deleting_destruct_00737110(MistagHost& host, void* object, std::uint32_t flags) {
    host.clear_global(0x010904F8);
    at<std::uint32_t>(object, 4) = kSingletonBaseVtable;
    at<std::uint32_t>(object, 0) = 0x00CFE9F4;
    if ((flags & 1) != 0) host.free_00bf65ac(object);
    return object;
}

// 00996200..00996264. The two FSTP to [esp+4] round to float; the -0.0f at 00D7A208 is read
// from the image (bytes 00 00 00 80). FCOMIP with JBE: an unordered compare returns 0.
std::uint32_t angle_within_tolerance_00996200(MistagHost& host, void* condition) {
    const float current = host.object_vcall_50_angle(at<void*>(condition, 0x18));
    const float d = host.wrapped_angle_subtract_00438b10(at<float>(condition, 0x1C), current);
    const float magnitude = d > 0.0f ? d : -0.0f - d;     // 0099622A..00996240
    return at<float>(condition, 0x20) > magnitude ? 1u : 0u;
}

// 007B1A80..007B1AFF. The loop re-reads first/last each pass; index i is checked twice as the
// image does (first == null or i >= size -> 00BF6713) before the element is freed and nulled.
void point3_ptr_array_destruct_007b1a80(MistagHost& host, void* array) {
    at<std::uint32_t>(array, 0) = 0x00D053A4;
    for (std::uint32_t i = 0;; ++i) {
        auto* const first = at<void**>(array, 8);
        if (first == nullptr) break;
        const auto size = static_cast<std::uint32_t>(at<void**>(array, 0xC) - first);
        if (i >= size) break;
        void*& element = first[i];
        if (element != nullptr) {
            host.free_00bf65ac(element);
            element = nullptr;
        }
    }
    if (at<void*>(array, 8) != nullptr) host.free_00bf65ac(at<void*>(array, 8));
    at<std::uint32_t>(array, 0x08) = 0;
    at<std::uint32_t>(array, 0x0C) = 0;
    at<std::uint32_t>(array, 0x10) = 0;
}

}  // namespace bsp::cc13_mistag
