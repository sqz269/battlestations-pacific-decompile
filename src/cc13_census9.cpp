// Census of nine segments, packet cc13-census9 (marker cc13_census9). docs/CC13_CENSUS9.md.
#include "bsp/cc13_census9.hpp"

namespace bsp::cc13_census9 {
namespace {

template <class T>
T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

// The image's GunTurnSync record constructor and its trivial destructor (0075FE90 / 0075A5B0).
constexpr std::uint32_t kRecordCtor = 0x0075FE90;
constexpr std::uint32_t kRecordDtor = 0x0075A5B0;
// The controller slot constructor and destructor (009318D0 / 0092FD50).
constexpr std::uint32_t kSlotCtor = 0x009318D0;
constexpr std::uint32_t kSlotDtor = 0x0092FD50;
// The five RefPtrs of the sound request queue (004C3810 releases one).
constexpr std::uint32_t kRefPtrDtor = 0x004C3810;

// 00932865-009328A0 and three copies: a std::vector of non-trivial elements (allocator at +0,
// first +4, last +8, end +0Ch). The range destroy runs only when first is non-null; the three
// pointers are zeroed either way.
void vector_tidy(Census9Host& host, void* vector, void* owner,
    void (Census9Host::*destroy)(void*, void*, void*, void*)) {
    void* const first = at<void*>(vector, 4);
    if (first != nullptr) {
        (host.*destroy)(vector, owner, first, at<void*>(vector, 8));
        host.free_00bf65ac(at<void*>(vector, 4));
    }
    at<void*>(vector, 4) = nullptr;
    at<void*>(vector, 8) = nullptr;
    at<void*>(vector, 0xC) = nullptr;
}

// 0093293B-0093296A (and +34h, +8h): a std::vector of trivial elements, pointers at +0/+4/+8.
void vector_tidy_trivial(Census9Host& host, void* first_field) {
    if (at<void*>(first_field, 0) != nullptr) host.free_00bf65ac(at<void*>(first_field, 0));
    at<void*>(first_field, 0) = nullptr;
    at<void*>(first_field, 4) = nullptr;
    at<void*>(first_field, 8) = nullptr;
}

}  // namespace

// 0080FCF0..0080FD85 (Ghidra CG_array_ctor_helper_0080fcf0; one caller, 00816C12 in
// BSP_UnitInstance_SendShipSyncMessage). Base constructor with type 8Dh, then the header id
// word +18h and relay byte +1Ah, delivery +4h = 0, +1Ch = unit, +20h = (phase > 1), vtable,
// +22h = 0 (the gun count the writer 00814000 fills), +21h = (phase > 0 || force), and the
// 64h GunTurnSync records at +24h.
void* ship_guns_sync_construct_0080fcf0(Census9Host& host, void* message, void* unit,
    std::int32_t phase, bool force) {
    host.session_message_construct_base_0075b430(message, kShipGunsSyncType);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x04) = 0;
    at<void*>(message, 0x1C) = unit;
    at<std::uint8_t>(message, 0x20) = phase > 1 ? 1 : 0;      // CMP ECX,1 / SETG DL
    at<std::uint32_t>(message, 0x00) = kShipGunsSyncVtable;
    at<std::uint8_t>(message, 0x22) = 0;
    at<std::uint8_t>(message, 0x21) = (phase > 0 || force) ? 1 : 0;   // 0080FD31-0080FD4F
    host.eh_vector_construct_00bf7cd1(add(message, kShipGunsSyncRecords), kShipGunsSyncRecordSize,
        kShipGunsSyncRecordCount, kRecordCtor, kRecordDtor);
    return message;
}

// 00760C10..00760C7C (Ghidra CG_array_ctor_helper_00760c10; the 8Dh row of the session
// message factory). The base constructor is not called: +8h, +0Ch, +14h, the type byte +10h,
// the id word +18h, relay byte +1Ah and delivery +4h are zeroed, then the vtable, the records
// and +1Ch (unit) = 0. +20h..+22h are left as they were.
void* ship_guns_sync_construct_for_read_00760c10(Census9Host& host, void* message) {
    at<std::uint32_t>(message, 0x08) = 0;
    at<std::uint32_t>(message, 0x0C) = 0;
    at<std::uint32_t>(message, 0x14) = 0;
    at<std::uint8_t>(message, 0x10) = 0;
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x04) = 0;
    at<std::uint32_t>(message, 0x00) = kShipGunsSyncVtable;
    host.eh_vector_construct_00bf7cd1(add(message, kShipGunsSyncRecords), kShipGunsSyncRecordSize,
        kShipGunsSyncRecordCount, kRecordCtor, kRecordDtor);
    at<std::uint32_t>(message, 0x1C) = 0;
    return message;
}

// 00760CB0..00760CFC (Ghidra CG_vector_deleting_dtor_00760cb0; callers the scalar deleting
// destructor 00762520 and the SendShipSyncMessage unwind). The derived vtable is not
// re-stored: the records are destroyed, then the base vtable 00CE4974 goes in.
void ship_guns_sync_destruct_00760cb0(Census9Host& host, void* message) {
    host.eh_vector_destruct_00bf7c6e(add(message, kShipGunsSyncRecords), kShipGunsSyncRecordSize,
        kShipGunsSyncRecordCount, kRecordDtor);
    at<std::uint32_t>(message, 0x00) = kShipGunsSyncBaseVtable;
}

// 00939CB0..00939E43 (ledger BSP_UnitController_Construct; Ghidra tag cg_array_ctor_helper).
// Stores in listing order; the eight SSE zero floats are XORPS stores, so +0.0f.
void* unit_controller_construct_00939cb0(Census9Host& host, void* controller, void* unit) {
    void* const c = controller;
    at<std::uint32_t>(c, 0x00) = kControllerVtable;
    at<std::uint32_t>(c, 0x08) = 0;
    at<std::uint32_t>(c, 0x0C) = 0;
    at<std::uint32_t>(c, 0x10) = 0;
    at<std::uint32_t>(c, 0x20) = kControllerListenerVtable;
    at<void*>(c, 0x28) = c;
    at<std::uint32_t>(c, 0x24) = kControllerListenerMask;
    at<std::uint32_t>(c, 0x34) = 0;
    at<std::uint32_t>(c, 0x38) = 0;
    at<std::uint32_t>(c, 0x3C) = 0;
    for (std::size_t off : {0x5Cu, 0x58u, 0x4Cu, 0x48u, 0x44u, 0x40u, 0x54u, 0x50u})
        at<float>(c, off) = 0.0f;
    at<std::uint8_t>(c, 0x60) = 0;
    host.eh_vector_construct_00bf7cd1(add(c, kControllerSlots), kControllerSlotSize,
        kControllerSlotCount, kSlotCtor, kSlotDtor);
    // Five std::vector pointer triples: +310h (trivial), +320h, +330h, +340h, +364h.
    for (std::size_t off : {0x310u, 0x320u, 0x330u, 0x340u, 0x364u}) {
        at<std::uint32_t>(c, off) = 0;
        at<std::uint32_t>(c, off + 4) = 0;
        at<std::uint32_t>(c, off + 8) = 0;
    }
    at<void*>(c, 0x1C) = unit;
    at<std::uint8_t>(c, 0x14) = 0;
    at<std::uint8_t>(c, 0x64) = 1;
    at<float>(c, 0x80) = 0.0f;
    at<std::uint32_t>(c, 0x2C) = 0;                       // physics body
    for (int i = 0; i < 3; ++i) at<float>(c, 0x378 + 4 * i) = host.controller_default_vec3_00f87574(i);
    for (int i = 0; i < 3; ++i) at<float>(c, 0x384 + 4 * i) = host.controller_default_vec3_00f87574(i);
    at<std::uint32_t>(c, 0x18) = 0;
    host.controller_build_sub_objects_00937c90(c);
    return c;
}

// 00932840..009329B8 (Ghidra CG_vector_deleting_dtor_00932840; it is the complete destructor:
// callers 0081F499 in BSP_UnitVehicleBase_Destruct, the scalar deleting destructor 0080DF20 and
// the 00939E50 unwind). The listener subobject at +20h gets no destructor call and the vtable is
// not reset to a base.
void unit_controller_destruct_00932840(Census9Host& host, void* controller) {
    void* const c = controller;
    at<std::uint32_t>(c, 0x00) = kControllerVtable;
    vector_tidy(host, add(c, 0x360), c, &Census9Host::vector_destroy_range_009316f0);
    vector_tidy(host, add(c, 0x33C), c, &Census9Host::vector_destroy_range_00931760);
    vector_tidy(host, add(c, 0x32C), c, &Census9Host::vector_destroy_range_00931760);
    vector_tidy(host, add(c, 0x31C), c, &Census9Host::vector_destroy_range_009316f0);
    vector_tidy_trivial(host, add(c, 0x310));
    host.eh_vector_destruct_00bf7c6e(add(c, kControllerSlots), kControllerSlotSize,
        kControllerSlotCount, kSlotDtor);
    vector_tidy_trivial(host, add(c, 0x34));
    vector_tidy_trivial(host, add(c, 0x08));
}

// 00940E40..00940E9C (ledger BSP_GuiSoundSet_DestroyBody; the matching constructor is 00940DD0,
// the singleton row 004C1B90 / 00F89B34 in src/engine_seg4_6_9_cc13.cpp).
void sound_request_queue_destruct_00940e40(Census9Host& host, void* queue) {
    at<std::uint32_t>(queue, 0x00) = kSoundQueueVtable;
    host.eh_vector_destruct_00bf7c6e(add(queue, 4), 4, 5, kRefPtrDtor);
    host.clear_sound_queue_publication_00f89b34();
    at<std::uint32_t>(queue, 0x00) = kSingletonBaseVtable;
}

// 006C23D0..006C23F9 (Ghidra CG_scalar_deleting_dtor_006c23d0): the holder destructor is
// inlined; it unlinks the holder from the global list 00E19948.
void* air_ops_holder_scalar_deleting_destruct_006c23d0(Census9Host& host, void* holder,
    std::uint32_t flags) {
    at<std::uint32_t>(holder, 0x00) = kAirOpsHolderVtable;
    host.air_ops_holder_list_unlink_006bf880(holder);
    if ((flags & 1) != 0) host.free_00bf65ac(holder);
    return holder;
}

}  // namespace bsp::cc13_census9
