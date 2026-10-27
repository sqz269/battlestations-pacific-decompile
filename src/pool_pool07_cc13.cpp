// Packet cc13-pool07, segment 34 (see include/bsp/pool_pool07_cc13.hpp). Build-only.
#include "bsp/pool_pool07_cc13.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error cc13-pool07 reconstruction reads Win32 image storage.
#endif

namespace bsp::pool_pool07_cc13 {

namespace {

template <class T>
T& at(void* object, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(object) + off);
}

// The inlined new[] size of an element array: count * size, all ones when the
// 32-bit multiply overflows (MUL; SETO; NEG; OR), then + `header` with the
// same saturation on the carry (ADD; SETC; NEG; OR).
std::uint32_t saturated_array_bytes(std::uint32_t count, std::uint32_t size, std::uint32_t header) noexcept {
    const std::uint64_t product = static_cast<std::uint64_t>(count) * size;
    std::uint32_t bytes = product > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<std::uint32_t>(product);
    if (header != 0) bytes = bytes > 0xFFFFFFFFu - header ? 0xFFFFFFFFu : bytes + header;
    return bytes;
}

// Three chained float reads into `out[0..2]`, each on the node the previous one
// returned (the three addresses are pushed before the first call).
void read_float3(void* node, float* out, Pool07Host& host) {
    void* next = host.node_read_float_00be9a60(node, &out[0]);
    next = host.node_read_float_00be9a60(next, &out[1]);
    host.node_read_float_00be9a60(next, &out[2]);
}

// The shared body of 006FA7F0 and 006FA910: read the count, new[] a counted
// array of 20h-byte elements (the dword before the first element holds the
// count, the EH constructor iterator builds them), store the array pointer
// (null when new fails) in *out, then per element read the float3 at +4h and
// the u16 arrays named by `arrays`; the first array's count's low word goes to
// +1Ch. A failed new leaves *out null and the loop still runs (the image has
// no guard).
void read_counted_array20(void* node, void** out, std::uint32_t* count, std::uint32_t ctor, std::uint32_t dtor,
                          const std::size_t* arrays, std::size_t array_count, Pool07Host& host) {
    host.node_read_u32_00be9a80(node, count);
    const std::uint32_t n = *count;
    auto* const block = static_cast<std::uint32_t*>(host.operator_new_00bf55be(saturated_array_bytes(n, 0x20, 4)));
    std::uint32_t* first = nullptr;
    // EH state 0
    if (block != nullptr) {
        first = block + 1;
        *block = n;
        host.eh_vector_constructor_iterator_00bf7cd1(first, 0x20, n, ctor, dtor);
    }
    *out = first;
    // EH state -1
    for (std::int32_t i = 0; i < static_cast<std::int32_t>(*count); ++i) {
        auto* const element = static_cast<std::uint8_t*>(*out) + i * 0x20;
        read_float3(node, &at<float>(element, 0x04), host);
        std::uint32_t array_size = 0;  // the image reuses the new[] result's stack slot
        for (std::size_t k = 0; k != array_count; ++k) {
            host.convex_read_u16_array_006f9ac0(node, &at<std::uint16_t*>(element, arrays[k]), &array_size);
            if (k == 0) at<std::uint16_t>(element, 0x1C) = static_cast<std::uint16_t>(array_size);
        }
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 006FA7F0 (98 insns), ledger name Unknown_ConvexPayloadArrays_Read_006fa7f0
// (was CG_array_ctor_helper_006fa7f0). __fastcall(ECX = node, EDX = out array
// cell; [ESP+4] u32* count), RET 4. Only caller 006FAD70
// BSP_ConvexObject_ReadPayload. Evidence: assembly 006FA7F0..006FA904.
// 00BE9A80 reads *count; new[] (count * 20h saturated, + 4 saturated) through
// 00BF55BE; on success the count is stored at block[0] and 00BF7CD1 builds
// `count` elements at block+4 (ctor 006F9C20 BSP_ConvexHullArrays_Construct,
// dtor 006F9C40); *out = block+4 or 0; then for each element (stride 20h):
// float3 at +4h (00BE9A60 x3, chained), 006F9AC0 into +10h, the low word of
// that count to +1Ch, 006F9AC0 into +14h and +18h. The loop bound re-reads
// *count every iteration; the element base re-reads *out.
// EH states: 0 around the allocation and constructor iterator, -1 after.
// Uncertain: the meaning of the three index arrays (the live host's
// read_convex_points in src/mmod_hull_convex_box.cpp parses the same records
// for the hull box and skips the rest).
void convex_read_hull_arrays_006fa7f0(void* node, void** out, std::uint32_t* count, Pool07Host& host) {
    static constexpr std::size_t kArrays[] = {0x10, 0x14, 0x18};
    read_counted_array20(node, out, count, 0x006F9C20u, 0x006F9C40u, kArrays, 3, host);
}

// ---------------------------------------------------------------------------
// 006FA910 (93 insns), ledger name Unknown_ConvexPayloadArrays_Read_006fa910
// (was CG_array_ctor_helper_006fa910). Same ABI and shape as 006FA7F0, with
// element ctor 006F9C80 BSP_ConvexHullArrays2_Construct (vtable 00CFB6A0,
// zeroes +14h/+18h) and dtor 006F9CA0, and only two u16 arrays: +14h (its
// count's low word to +1Ch) and +18h; +10h is not touched. Evidence: assembly
// 006FA910..006FAA15. EH states: 0, -1. Only caller 006FAD70.
void convex_read_hull_arrays2_006fa910(void* node, void** out, std::uint32_t* count, Pool07Host& host) {
    static constexpr std::size_t kArrays[] = {0x14, 0x18};
    read_counted_array20(node, out, count, 0x006F9C80u, 0x006F9CA0u, kArrays, 2, host);
}

// ---------------------------------------------------------------------------
// 006FAA20 (49 insns), BSP_ConvexObject_ReadVec3Array. __fastcall(ECX = node,
// EDX = out array cell; [ESP+4] u32* count), RET 4, no EH frame. Evidence:
// assembly 006FAA20..006FAA94: 00BE9A80 reads *count; operator new(count * 0Ch,
// saturated on overflow, no header) into *out; then count float3 records at
// stride 0Ch, each three chained 00BE9A60 reads. No null test on the new
// result. Only caller 006FAD70.
void convex_read_vec3_array_006faa20(void* node, void** out, std::uint32_t* count, Pool07Host& host) {
    host.node_read_u32_00be9a80(node, count);                          // 006FAA2F
    *out = host.operator_new_00bf55be(saturated_array_bytes(*count, 0x0C, 0));  // 006FAA47
    for (std::int32_t i = 0; i < static_cast<std::int32_t>(*count); ++i) {
        read_float3(node, &at<float>(*out, i * 0x0C), host);        // 006FAA72..006FAA80
    }
}

// ---------------------------------------------------------------------------
// 006FACE0 (40 insns), BSP_ConvexObject_ReadRecord14Array. __fastcall(ECX =
// node, EDX = out array cell; [ESP+4] u32* count), RET 4, no EH frame.
// Evidence: assembly 006FACE0..006FAD3D: 00BE9A80 reads *count; operator
// new(count * 14h, saturated) into *out; then count records at stride 14h,
// each filled by 006F9B20(ECX = node, EDX = record) (its returned node is not
// used). Only caller 006FAD70.
void convex_read_record14_array_006face0(void* node, void** out, std::uint32_t* count, Pool07Host& host) {
    host.node_read_u32_00be9a80(node, count);                          // 006FACEF
    *out = host.operator_new_00bf55be(saturated_array_bytes(*count, 0x14, 0));  // 006FAD07
    for (std::int32_t i = 0; i < static_cast<std::int32_t>(*count); ++i) {
        host.convex_read_record14_006f9b20(node, static_cast<std::uint8_t*>(*out) + i * 0x14);  // 006FAD29
    }
}

// ---------------------------------------------------------------------------
// 006FB300 (17 insns), BSP_Cruiser_Construct. __thiscall(ECX = unit; arg),
// RET 4; EAX = unit. No EH frame. Evidence: assembly 006FB300..006FB360.
// 0081ED40(unit, arg), then the eight vtables of the cruiser instance and the
// class id +C4h = 0Ah (ENTITY_CLASS_IDS.md: 0Ah MCruiser). Same shape as the
// battleship's 006DFC90 (src/cc13_seg33a.cpp).
void* cruiser_construct_006fb300(void* unit, void* arg, Pool07Host& host) {
    host.unit_vehicle_base_construct_0081ed40(unit, arg);             // 006FB308
    at<std::uint32_t>(unit, 0x000) = 0x00CFB738u;
    at<std::uint32_t>(unit, 0x010) = 0x00CFB71Cu;
    at<std::uint32_t>(unit, 0x024) = 0x00CFB714u;
    at<std::uint32_t>(unit, 0x170) = 0x00CFB710u;
    at<std::uint32_t>(unit, 0x1E4) = 0x00CFB708u;
    at<std::uint32_t>(unit, 0x310) = 0x00CFB6F0u;
    at<std::uint32_t>(unit, 0x38C) = 0x00CFB6ECu;
    at<std::uint32_t>(unit, 0x72C) = 0x00CFB6E8u;
    at<std::uint32_t>(unit, 0x0C4) = 0x0A;                            // 006FB353
    return unit;
}

// ---------------------------------------------------------------------------
// 006FB430 (40 insns), BSP_Cruiser_Create. __thiscall(ECX = vehicle class;
// arg), RET 4; EAX = unit. Slot +28h of descriptor 00D1AD38. Evidence:
// assembly 006FB430..006FB4B0: operator new(1188h); memset(block, 0, 1188h)
// before the null test (as in the image); EH state 0 around 006FB300(block,
// arg), skipped (unit = 0) when new failed; state -1; 009553D0(ECX = unit,
// class); unit+354h = class. A failed allocation faults in 009553D0 in the
// image; no guard is added. Same shape as the battleship's 006DFEF0.
void* cruiser_create_006fb430(void* vehicle_class, void* arg, Pool07Host& host) {
    void* const block = host.operator_new_00bf55be(0x1188);           // 006FB44F
    host.memset_00bf79f0(block, 0, 0x1188);                           // 006FB45E
    // EH state 0
    void* const unit = block != nullptr ? cruiser_construct_006fb300(block, arg, host) : nullptr;
    // EH state -1
    host.unit_set_vehicle_class_009553d0(unit, vehicle_class);        // 006FB493
    at<void*>(unit, 0x354) = vehicle_class;                           // 006FB49C
    return unit;
}

// ---------------------------------------------------------------------------
// 006FB510 (20 insns), BSP_Cruiser_DetachFromWorldLists, slot +134h of vtable
// 00CFB738. __fastcall(ECX = unit), RET. Evidence: assembly 006FB510..006FB545:
// 006DFFC0(unit), then the class list at [unit+30h]+90h: walk the nodes from
// [list+4] by +4h, and erase (004837D0, ECX = list) the first whose payload
// (+8h) is the unit; stop there. The world pointer +30h is read after
// 006DFFC0 returns.
void cruiser_detach_from_world_lists_006fb510(void* unit, Pool07Host& host) {
    host.unit_detach_mid_world_lists_006dffc0(unit);                  // 006FB513
    void* const list = static_cast<std::uint8_t*>(at<void*>(unit, 0x30)) + 0x90;  // 006FB518
    for (void* node = at<void*>(list, 4); node != nullptr; node = at<void*>(node, 4)) {
        if (at<void*>(node, 8) == unit) {                             // 006FB530
            host.unit_list_erase_node_004837d0(list, node);           // 006FB53F
            return;
        }
    }
}

}  // namespace bsp::pool_pool07_cc13
