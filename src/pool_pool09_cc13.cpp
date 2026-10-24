#include "bsp/pool_pool09_cc13.hpp"

// cc13 pool09 (segment 34). Evidence is the disk listing (bsp.py disasm-raw)
// and Ghidra's pseudocode, checked against each other; string keys and
// constants were read from the image bytes (2026-10-24). Descriptive names are
// hypotheses. Main-thread x87 runs at 24-bit precision
// (docs/CC13_FPU_PRECISION.md); every product here is stored to a float
// before use, so float arithmetic reproduces it.

namespace bsp::cc13_pool09 {
namespace {

template <typename T>
T& at(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

void* ptr(void* base, std::size_t offset) { return at<void*>(base, offset); }

void* add(void* base, std::size_t offset) { return static_cast<std::uint8_t*>(base) + offset; }

// The NativeString {uint length +0, char* data +4} assignment the two bullet
// readers inline (007000E1..00700118, 0070111A..00701151): skipped when the
// two strings are the same object; resize with preserve 1; copy only when
// the source length is non-zero, and copy the destination's new length.
void assign_native_string(Pool09Host& host, void* dst, void* src) {
    if (dst == src) return;
    host.native_string_resize_0041dd40(dst, at<std::uint32_t>(src, 0), 1);
    if (at<std::uint32_t>(src, 0) != 0) {
        host.memcpy_00bf7680(ptr(dst, 4), ptr(src, 4), at<std::uint32_t>(dst, 0));
    }
}

}  // namespace

// 006FEF10 / 006FFDF0 / 00700BE0: the create (vtable slot +20h) of
// MDummyKamikazePlane (00CFA63C), MDummySubmarine (00CFA678) and MDummyTarget
// (00CFA5CC); docs/PROJECTILE_KINDS.md lists the three rows.
//
// Evidence: a normalised Capstone diff of the three bodies (1E3h bytes,
// 126 instructions each) differs only in the SEH handler, the allocation size
// (two PUSH imm) and the constructor call, and the shared remainder is the bomb
// create 006E2C00 (modelled in src/cc13_seg33a.cpp) instruction for
// instruction. Sizes 4A4h / 4BCh / 4A0h, constructors 006FE950 / 006FF800 /
// 00700A20, handlers 00C83B48 / 00C83C48 / 00C83D08.
//
// Original ABI: __thiscall(ECX = cls, a1, a2, position*, direction*, a5, a6,
// a7), RET 1Ch, SEH frame, result (record + 310h) in EAX. a5..a7 are never
// read. Register use: EDI holds cls across the allocation; ESI the record.
//
// EH states: 0 from just after the allocation until the constructor returns
// (the handler frees the memory if the constructor unwinds), -1 from 006FF000
// on. Nothing after that point can unwind through this frame's state.
//
// Body: operator new (00BF55BE), then 00BF79F0 clears the block BEFORE the null
// test (so a null allocation faults in memset first, as in the image). The
// constructor runs only on a non-null block. +54h = a2, +314h = cls. Then
// record->vtable[98h](0, [[00E188A8]+19CCh], identity 4x4) where the identity
// is built from 00D7A24C (1.0f) and XORPS zero. speed = [[+314h]+50h] (FSTP to
// a float slot); velocity +318h..+320h = direction[i] * speed, each stored as
// float. Local matrix row 2 (+94h..+9Ch) = direction, 0085DC80(record + 74h).
// Position goes to +A4h..+ACh, shot +60h..+68h and shot +6Ch..+74h (shot =
// record + 310h). Bytes +C8h and +10Ch = 0. 0042ED50 runs on each node of the
// chain starting at [record+48h] and linked through +44h.
//
// Uncertainty: what +54h, a1 and a2 mean (a1 goes to the constructor). The
// slot reads [record+314h] back after storing cls, which the model mirrors.
void* dummy_projectile_class_create(Pool09Host& host, DummyProjectileKind kind, void* cls, std::uint32_t a1,
                                    void* a2, const float* position, const float* direction) {
    std::size_t size = 0x4A4;
    if (kind == DummyProjectileKind::kSubmarine) size = 0x4BC;
    if (kind == DummyProjectileKind::kTarget) size = 0x4A0;

    void* const memory = host.operator_new_00bf55be(size);
    host.memset_00bf79f0(memory, 0, size);
    void* record = nullptr;
    if (memory != nullptr) {
        switch (kind) {
            case DummyProjectileKind::kKamikazePlane:
                record = host.dummy_kamikaze_plane_construct_006fe950(memory, a1);
                break;
            case DummyProjectileKind::kSubmarine:
                record = host.dummy_submarine_construct_006ff800(memory, a1);
                break;
            case DummyProjectileKind::kTarget:
                record = host.dummy_target_construct_00700a20(memory, a1);
                break;
        }
    }
    at<void*>(record, 0x54) = a2;
    at<void*>(record, 0x314) = cls;
    float identity[16];
    for (int i = 0; i < 16; ++i) identity[i] = (i % 5 == 0) ? kOne_00d7a24c : 0.0f;
    host.entity_attach_scene_vt98(record, 0, host.scene_root_00e188a8_19cc(), identity);

    const float speed = at<float>(ptr(record, 0x314), 0x50);
    const float vx = direction[0] * speed;
    const float vy = direction[1] * speed;
    const float vz = direction[2] * speed;
    at<float>(record, 0x318) = vx;
    at<float>(record, 0x31C) = vy;
    at<float>(record, 0x320) = vz;
    at<float>(record, 0x74 + 0x20) = direction[0];
    at<float>(record, 0x74 + 0x24) = direction[1];
    at<float>(record, 0x74 + 0x28) = direction[2];
    host.matrix_orthonormalize_basis_rows_0085dc80(static_cast<float*>(add(record, 0x74)));

    at<float>(record, 0xA4) = position[0];
    at<float>(record, 0xA8) = position[1];
    at<float>(record, 0xAC) = position[2];
    void* const shot = add(record, 0x310);
    at<float>(shot, 0x60) = position[0];
    at<float>(shot, 0x64) = position[1];
    at<float>(shot, 0x68) = position[2];
    at<float>(shot, 0x6C) = position[0];
    at<float>(shot, 0x70) = position[1];
    at<float>(shot, 0x74) = position[2];
    at<std::uint8_t>(record, 0xC8) = 0;
    at<std::uint8_t>(record, 0x10C) = 0;
    for (void* node = ptr(record, 0x48); node != nullptr; node = ptr(node, 0x44)) {
        host.scene_node_invalidate_subtree_pose_0042ed50(node);
    }
    return shot;
}

void* dummy_kamikaze_plane_class_create_projectile_006fef10(Pool09Host& host, void* cls, std::uint32_t a1, void* a2,
                                                            const float* position, const float* direction,
                                                            std::int32_t, std::int32_t, std::int32_t) {
    return dummy_projectile_class_create(host, DummyProjectileKind::kKamikazePlane, cls, a1, a2, position, direction);
}

void* dummy_submarine_class_create_projectile_006ffdf0(Pool09Host& host, void* cls, std::uint32_t a1, void* a2,
                                                       const float* position, const float* direction,
                                                       std::int32_t, std::int32_t, std::int32_t) {
    return dummy_projectile_class_create(host, DummyProjectileKind::kSubmarine, cls, a1, a2, position, direction);
}

void* dummy_target_bullet_class_create_projectile_00700be0(Pool09Host& host, void* cls, std::uint32_t a1, void* a2,
                                                           const float* position, const float* direction,
                                                           std::int32_t, std::int32_t, std::int32_t) {
    return dummy_projectile_class_create(host, DummyProjectileKind::kTarget, cls, a1, a2, position, direction);
}

// 00700050: MDummySubmarine's Lua reader, vtable 00CFA678 slot +18h.
// Evidence: listing 00700050..0070012A. The dword 00700050 also appears at
// 00D55584, 00D5592C, 00D55BD4 and seven more data dwords besides the vtable slot
// 00CFA690 (bsp.py callers);
// those tables are not interpreted here.
// Original ABI: __thiscall(ECX = cls, const LuaObject* row), RET 4, SEH frame
// (handler 00C83C88). EH state 0 covers the temporary SubmarineClass object
// from 007000B4 to 007000D4; the handler destroys it.
// Body: 006E1BE0(cls, row); "OpenAfterTime" (00CFC878) into +D8h through
// 006FE780 with a copy of row and default FLD1 (1.0f); [row."SubmarineClass"
// (00CFCD08)] as an integer (00B66290) gives 00964790(id, DL = 1) into +DCh,
// and only then is the temporary destroyed; finally the NativeString at +A0h
// takes the class's string at [+DCh]+38h.
// Uncertainty: the meaning of the descriptor's +38h string (the class name is
// the likely reading). The copy is built in the outgoing argument area and
// destroyed by the callee.
void dummy_submarine_class_read_lua_fields_00700050(Pool09Host& host, void* cls, const void* row) {
    host.bomb_class_read_lua_fields_006e1be0(cls, row);
    LuaObjectStorage copy{};
    host.lua_object_copy_construct_00b66fa0(copy.bytes, row);
    host.lua_read_number_or_default_006fe780(kKeyOpenAfterTime_00cfc878, &at<float>(cls, 0xD8), copy.bytes,
                                             kOne_00d7a24c);
    LuaObjectStorage temp{};
    void* const object = host.lua_object_get_by_name_00b67800(row, temp.bytes, kKeySubmarineClass_00cfcd08);
    const std::int32_t id = host.lua_object_get_integer_00b66290(object);
    at<void*>(cls, 0xDC) = host.vehicle_class_get_or_create_00964790(id, true);
    host.lua_object_destruct_00b67700(temp.bytes);
    assign_native_string(host, add(cls, 0xA0), add(ptr(cls, 0xDC), 0x38));
}

// 00701060: MDummyTarget's Lua reader, vtable 00CFA5CC slot +18h.
// Evidence: listing 00701060..00701163.
// Original ABI: __thiscall(ECX = cls, const LuaObject* row), RET 4, SEH frame
// (handler 00C83D68). EH state 0 covers the temporary DummyTargetVehicleClass
// object from 007010F1 to 00701104.
// Body: 006E1BE0(cls, row); "OpenAtFallingSpeed" (00CFD1F4) into +D8h through
// 00700290 with default FLDZ (0.0f); "OpenAfterTime" (00CFC878) into +DCh
// through 006FE780 with default [00CE3958] (2.0f); the integer
// row."DummyTargetVehicleClass" (00CFD1DC); the temporary is destroyed BEFORE
// 00964790(id, DL = 1) fills +E0h (the opposite order from 00700050); then the
// NativeString at +A0h takes [+E0h]+38h.
// Uncertainty: as for 00700050.
void dummy_target_bullet_class_read_lua_fields_00701060(Pool09Host& host, void* cls, const void* row) {
    host.bomb_class_read_lua_fields_006e1be0(cls, row);
    LuaObjectStorage copy{};
    host.lua_object_copy_construct_00b66fa0(copy.bytes, row);
    host.lua_read_number_or_default_00700290(kKeyOpenAtFallingSpeed_00cfd1f4, &at<float>(cls, 0xD8), copy.bytes,
                                             0.0f);
    LuaObjectStorage copy2{};
    host.lua_object_copy_construct_00b66fa0(copy2.bytes, row);
    host.lua_read_number_or_default_006fe780(kKeyOpenAfterTime_00cfc878, &at<float>(cls, 0xDC), copy2.bytes,
                                             kTwo_00ce3958);
    LuaObjectStorage temp{};
    void* const object = host.lua_object_get_by_name_00b67800(row, temp.bytes, kKeyDummyTargetVehicleClass_00cfd1dc);
    const std::int32_t id = host.lua_object_get_integer_00b66290(object);
    host.lua_object_destruct_00b67700(temp.bytes);
    at<void*>(cls, 0xE0) = host.vehicle_class_get_or_create_00964790(id, true);
    assign_native_string(host, add(cls, 0xA0), add(ptr(cls, 0xE0), 0x38));
}

// 00700E40 BSP_DummyTargetVehicleClass_ReadLuaFields (formerly
// BSP_DoorClass_ReadLuaFields): the dummy target vehicle class descriptor's
// Lua reader, slot +8h of vtable 00D1AA58 ([00D1AA60] = 00700E40).
// Evidence: listing 00700E40..00700F65. The only code store of 00D1AA58 is
// MOV [ESI],00D1AA58 at 00960058 (the dword at 0096005A, found by byte scan)
// in 00960050 BSP_VehicleClass_ConstructDummyTarget. That constructor also
// stores 00D1AA54 at +6Ch (0096005E) and clears +140h, the handle this reader
// fills. The keys agree: OpenTime / SlowingFactor appear only in the
// "Parachute target - tutorial" row (include/bsp/gameplay_seg34_cc13.hpp),
// and 00700660 reads them from the dummy target vehicle's descriptor.
// Original ABI: __thiscall(ECX = descriptor, const LuaObject* row), RET 4, SEH
// frame (handler 00C83D50). The incoming row argument's stack slot is reused
// as the acquired handle's out slot after the row pointer is held in EDI.
// The image passes that slot to 00870CD0 still holding the row pointer; the
// C++ passes a zeroed local instead. This assumes 00870CD0 stores its out slot
// without reading the old value.
// EH states: 0 while the ExplosionEfx temporary lives (00700E7F), 1 while the
// acquired handle also lives (00700E9D), back to 0 once the handle is
// released (00700ED9), -1 before the temporary is destroyed (00700F00).
// Body: 00960230(descriptor, row); the integer row."ExplosionEfx" (00CE44AC)
// goes to 00870CD0(&slot, id, 1); the handle it returns replaces +140h with
// counted references (InterlockedIncrement on the new +4h when non-null, then
// InterlockedDecrement on the old +4h and its vtable[0]() at zero), skipped
// entirely when old == new; the slot's own reference is then dropped the same
// way; the temporary is destroyed. "OpenTime" (00CFD1D0) into +138h through
// 00700330 and "SlowingFactor" (00CFD1C0) into +13Ch through 007003B0, each
// with a copy of row and no default.
// Uncertainty: what 00870CD0's third argument (1) selects.
void dummy_target_vehicle_class_read_lua_fields_00700e40(Pool09Host& host, void* descriptor, const void* row) {
    host.vehicle_class_read_lua_fields_00960230(descriptor, row);
    LuaObjectStorage temp{};
    void* const object = host.lua_object_get_by_name_00b67800(row, temp.bytes, kKeyExplosionEfx_00ce44ac);
    const std::int32_t effect_id = host.lua_object_get_integer_00b66290(object);
    void* slot = nullptr;
    void* const acquired = *host.effect_handle_acquire_00870cd0(&slot, effect_id, 1);
    void* const old = ptr(descriptor, 0x140);
    if (old != acquired) {
        at<void*>(descriptor, 0x140) = acquired;
        if (acquired != nullptr) host.interlocked_increment_00ce221c(&at<std::int32_t>(acquired, 4));
        if (old != nullptr && host.interlocked_decrement_00ce2220(&at<std::int32_t>(old, 4)) == 0) {
            host.refcounted_destroy_vt0(old);
        }
    }
    if (slot != nullptr) {
        if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(slot, 4)) == 0) {
            host.refcounted_destroy_vt0(slot);
        }
        slot = nullptr;
    }
    host.lua_object_destruct_00b67700(temp.bytes);

    LuaObjectStorage copy{};
    host.lua_object_copy_construct_00b66fa0(copy.bytes, row);
    host.lua_read_number_00700330(kKeyOpenTime_00cfd1d0, &at<float>(descriptor, 0x138), copy.bytes);
    LuaObjectStorage copy2{};
    host.lua_object_copy_construct_00b66fa0(copy2.bytes, row);
    host.lua_read_number_007003b0(kKeySlowingFactor_00cfd1c0, &at<float>(descriptor, 0x13C), copy2.bytes);
}

// 007001E0: slot +B8h (index 46) of the class-35h vtable 00CFCD60, reached by
// the minimap update 005C0F20 through `CALL [EDX+B8h]` at 005C1675.
// Evidence: body 007001E0..007001E3 is XOR AL,AL / RET; Ghidra defines no
// function there. include/bsp/hud_minimap.hpp already models the answer as
// HudMinimapShows::Never; this is the per-address body.
// Original ABI: __thiscall(ECX = this, unused) -> bool in AL, plain RET.
// Uncertainty: none in the body; "shows on minimap" is the caller's reading.
bool class35_shows_on_minimap_007001e0() noexcept { return false; }

}  // namespace bsp::cc13_pool09
