// cc13 pool16 (segments 2 and 5). Descriptive names are hypotheses (cc13-poolw3, 2026-10-24).

#include "bsp/pool_pool16_cc13.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4, "the node pool storage targets MSVC Win32");
static_assert(sizeof(CRITICAL_SECTION) == 24);
static_assert(offsetof(NodePool1CStorage, critical_section_0c) == 0x0c);
static_assert(offsetof(NodePool1CStorage, recursion_24) == 0x24);
static_assert(offsetof(NodePool1CStorage, slabs_28) == 0x28);
static_assert(offsetof(NodePool1CStorage, slab_count_2c) == 0x2c);
static_assert(offsetof(NodePool1CStorage, table_capacity_30) == 0x30);
static_assert(offsetof(NodePool1CStorage, first_free_slab_34) == 0x34);
static_assert(sizeof(MovieNativeString) == 8);

constexpr std::uint32_t kSlotBytes = 0x1c;
constexpr std::uint32_t kSlotsPerSlab = 0x400;
constexpr std::uint32_t kSlotSlabIndex = 0x18;
constexpr std::uint32_t kFreeCount = 0x7800;
constexpr std::uint32_t kNoFreeSlab = 0xffffffffu;

CRITICAL_SECTION* section(NodePool1CStorage& pool) noexcept {
    return reinterpret_cast<CRITICAL_SECTION*>(pool.critical_section_0c);
}
std::uint16_t free_count(const std::byte* slab) noexcept {
    std::uint16_t value;
    std::memcpy(&value, slab + kFreeCount, sizeof(value));
    return value;
}
// 00402F70's body, inlined by 00435A50: drain the recursion count, then delete.
void drain_and_delete_section(NodePool1CStorage& pool) noexcept {
    if (pool.recursion_24 > 0) {
        do {
            --pool.recursion_24;
            LeaveCriticalSection(section(pool));
        } while (pool.recursion_24 > 0);
    }
    DeleteCriticalSection(section(pool));
}

}  // namespace

// 00436230..00436302 (68 insns), __thiscall(ECX = pool), plain RET, EAX = pool.
// Evidence: listing; unwind map 00D85688 (handler 00C5EEBE): state 0 -> 00403970 (unlink),
// state 1 -> 00402F70 (+0Ch section), state 2 -> 00435560 (+28h table freed through 00BF6989
// when non-null).
// Order: base vtable 00D7A0C0, prev = 0, next = head [00E188B4], head->prev = pool, head = pool
// (00436248..0043626E, the inlined allocator_list_elem constructor); EH state 0; vtable 00CE4120;
// InitializeCriticalSection(+0Ch) (import 00CE220C); +24h, +28h, +2Ch, +30h = 0, +34h = -1; EH
// state 2 (no state-1 store is emitted: nothing between can throw). With capacity < 20h (always,
// as it was just zeroed) capacity = 20h, operator new(80h) (00BF55BE), copy the current entries
// (count 0 here) into the non-null destination slots, free the old table through 00BF6989 when
// non-null, store the new table.
// EH: only operator new can throw; the image then frees the table (still null), drains/deletes
// the section and unlinks, then the exception continues. Modelled with catch/rethrow.
// Uncertainty: none in behaviour; allocator failure is the host's exception type.
NodePool1CStorage& node_pool_1c_construct_00436230(NodePool1CStorage& pool, AllocatorListDomain& list,
                                                   NodePool1CHost& host) {
    list.prepend_base_element(pool.allocator_00);
    pool.allocator_00.native_vtable_00 = kNodePool1CVtable;
    ::new (pool.critical_section_0c) CRITICAL_SECTION;
    InitializeCriticalSection(section(pool));
    pool.recursion_24 = 0;
    pool.slabs_28 = nullptr;
    pool.slab_count_2c = 0;
    pool.table_capacity_30 = 0;
    pool.first_free_slab_34 = kNoFreeSlab;
    if (pool.table_capacity_30 < 0x20) {
        pool.table_capacity_30 = 0x20;
        std::byte** table = nullptr;
        try {
            table = static_cast<std::byte**>(host.operator_new_00bf55be(0x80));
        } catch (...) {
            if (pool.slabs_28) host.free_00bf6989(pool.slabs_28);  // 00435560
            drain_and_delete_section(pool);                       // 00402F70
            list.unlink_base_element_00403970(pool.allocator_00);  // 00403970
            throw;
        }
        for (std::uint32_t i = 0; i < pool.slab_count_2c; ++i) {
            if (table + i != nullptr) table[i] = pool.slabs_28[i];
        }
        if (pool.slabs_28) host.free_00bf6989(pool.slabs_28);
        pool.slabs_28 = table;
    }
    return pool;
}

// 004360E0..0043617F (54 insns), slot 0 of vtable 00CE4120, __thiscall(ECX = pool), plain RET.
// Evidence: listing. For each slab index (reloading the count): a slab whose free count +7800h
// is 400h (every slot free) is freed through 00BF65AC, the last table entry is copied over it
// (even when it is the last), the count drops, and when the index is still below the count every
// slot of the moved slab gets its slab index (+18h, stride 1Ch) rewritten; the same index is then
// examined again (SUB EDI,1 / ADD EDI,1). Afterwards +34h = -1 and the first slab with a non-zero
// free count, if any, becomes +34h. No lock is taken. No EH frame.
// Ghidra lists a callee 004359E0; the body makes no such call (its only CALL is 00436102).
void node_pool_1c_trim_empty_slabs_004360e0(NodePool1CStorage& pool, NodePool1CHost& host) {
    for (std::uint32_t index = 0; index < pool.slab_count_2c; ++index) {
        std::byte* slab = pool.slabs_28[index];
        if (free_count(slab) != kSlotsPerSlab) continue;
        host.free_00bf65ac(slab);
        pool.slabs_28[index] = pool.slabs_28[pool.slab_count_2c - 1u];
        --pool.slab_count_2c;
        if (index < pool.slab_count_2c) {
            std::byte* moved = pool.slabs_28[index];
            for (std::uint32_t slot = 0; slot < kSlotsPerSlab; ++slot)
                std::memcpy(moved + slot * kSlotBytes + kSlotSlabIndex, &index, sizeof(index));
        }
        --index;  // the loop increment re-examines the same index (wraps from 0 as the image does)
    }
    pool.first_free_slab_34 = kNoFreeSlab;
    for (std::uint32_t i = 0; i < pool.slab_count_2c; ++i) {
        if (free_count(pool.slabs_28[i]) != 0) {
            pool.first_free_slab_34 = i;
            break;
        }
    }
}

// 00435A50..00435AD9 (51 insns), __fastcall(ECX = pool), plain RET (the CRT atexit thunks call it).
// Evidence: listing. Vtable 00CE4120; every slab freed through 00BF65AC (count reloaded each
// pass); the table freed through 00BF6989 when non-null; the 00402F70 body inlined (while +24h
// > 0: decrement, LeaveCriticalSection; then DeleteCriticalSection, imports 00CE2210 / 00CE2214);
// then the 00403970 body inlined (prev read, base vtable 00D7A0C0, prev->next or head [00E188B4] =
// next, next->prev = prev). The table/count fields are not cleared. No EH frame.
void node_pool_1c_destroy_00435a50(NodePool1CStorage& pool, AllocatorListDomain& list,
                                   NodePool1CHost& host) {
    pool.allocator_00.native_vtable_00 = kNodePool1CVtable;
    for (std::uint32_t i = 0; i < pool.slab_count_2c; ++i) host.free_00bf65ac(pool.slabs_28[i]);
    if (pool.slabs_28) host.free_00bf6989(pool.slabs_28);
    drain_and_delete_section(pool);
    list.unlink_base_element_00403970(pool.allocator_00);
}

// ---------------------------------------------------------------------------------------------
// 00460B80
// ---------------------------------------------------------------------------------------------
// Read from the listing by local/p16_parse.py (cc13-poolw3): for each CALL 0045D480, the pushes
// before it, right to left (flag pushes PUSH ESI = 0 / PUSH EBX = 1; each string built in its own
// 8-byte outgoing slot, SUB ESP,8). ESI and EBX are not written between 00460C17 and the epilogue.
// The text equals bsp::kMovieActionTypes (src/engine_movie_actions.cpp), except that the image
// passes the label (e.g. "Type:") as the sixth string and the '|' options as the seventh; 0045D480
// stores the sixth at record +38h ([ESP+5Ch] -> +38h at 0045D5ED) and the seventh at +40h.
const MovieActionTypeCall kMovieActionTypeCalls_00460b80[32] = {
    // 0: CALL 0045D480 at 00460E24; literals CE536C CE5378 CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"sSetTarget", "Target:", "", "", "", "", "", ""}, {1, 0, 0, 0, 0, 0}, 0x00u},
    // 1: CALL 0045D480 at 0046102C; literals CE5344 CE5350 CE5358 CE3A0C CE3A0C CE3A0C CE3A0C CE5364
    {{"cSetParent", "Parent:", "ConvTime:", "", "", "", "", "Offset:"}, {1, 1, 0, 0, 0, 1}, 0x00u},
    // 2: CALL 0045D480 at 00461233; literals CE5318 CE3A0C CE3A0C CE3A0C CE3A0C CE5328 CE5330 CE3A0C
    {{"pEngineFailure", "", "", "", "", "Type:", "Destroy|Blown|Kill", ""}, {0, 0, 0, 0, 1, 0}, 0x00u},
    // 3: CALL 0045D480 at 00461438; literals CE5308 CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"pForceRelease", "", "", "", "", "", "", ""}, {0, 0, 0, 0, 0, 0}, 0x00u},
    // 4: CALL 0045D480 at 0046163C; literals CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"", "", "", "", "", "", "", ""}, {0, 0, 0, 0, 0, 0}, 0x00u},
    // 5: CALL 0045D480 at 00461842; literals CE52FC CE3A0C CE5358 CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"cUnParent", "", "ConvTime:", "", "", "", "", ""}, {0, 1, 0, 0, 0, 0}, 0x00u},
    // 6: CALL 0045D480 at 00461A48; literals CE52DC CE3A0C CE52F0 CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"sForceDepthCharge", "", "PlatfrmID:", "", "", "", "", ""}, {0, 1, 0, 0, 0, 0}, 0x00u},
    // 7: CALL 0045D480 at 00461C4F; literals CE52B4 CE3A0C CE3A0C CE3A0C CE3A0C CE52C0 CE52C8 CE3A0C
    {{"pSetGunFire", "", "", "", "", "On/Off:", "OpenFire|HoldFire", ""}, {0, 0, 0, 0, 1, 0}, 0x00u},
    // 8: CALL 0045D480 at 00461E56; literals CE5290 CE3A0C CE3A0C CE3A0C CE3A0C CE52A4 CE52AC CE3A0C
    {{"uTogglePeriscope", "", "", "", "", "In/out:", "Out|In", ""}, {0, 0, 0, 0, 1, 0}, 0x00u},
    // 9: CALL 0045D480 at 0046205D; literals CE5260 CE3A0C CE5278 CE5284 CE3A0C CE3A0C CE3A0C CE3A0C
    {{"sForceArtilleryAngle", "", "V.angle:", "H.angle:", "", "", "", ""}, {0, 1, 1, 0, 0, 0}, 0x00u},
    // 10: CALL 0045D480 at 00462263; literals CE5248 CE5378 CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"sForceArtilleryTarget", "Target:", "", "", "", "", "", ""}, {1, 0, 0, 0, 0, 0}, 0x00u},
    // 11: CALL 0045D480 at 00462469; literals CE5238 CE3A0C CE52F0 CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"uForceTorpedo", "", "PlatfrmID:", "", "", "", "", ""}, {0, 1, 0, 0, 0, 0}, 0x00u},
    // 12: CALL 0045D480 at 00462670; literals CE51F8 CE3A0C CE3A0C CE3A0C CE3A0C CE520C CE5214 CE3A0C
    {{"aSetGunFireEnabled", "", "", "", "", "Weapon:", "All|AA_Flak|Artillery|Torpedo|DC", ""}, {0, 0, 0, 0, 1, 0}, 0x00u},
    // 13: CALL 0045D480 at 00462877; literals CE51E4 CE3A0C CE3A0C CE3A0C CE3A0C CE520C CE5214 CE3A0C
    {{"aSetGunFireDisabled", "", "", "", "", "Weapon:", "All|AA_Flak|Artillery|Torpedo|DC", ""}, {0, 0, 0, 0, 1, 0}, 0x00u},
    // 14: CALL 0045D480 at 00462A7E; literals CE51B8 CE3A0C CE3A0C CE3A0C CE3A0C CE52C0 CE51D0 CE3A0C
    {{"aSetGunFireTargetOnly", "", "", "", "", "On/Off:", "TargetOnly|AllEnemy", ""}, {0, 0, 0, 0, 1, 0}, 0x00u},
    // 15: CALL 0045D480 at 00462C86; literals CE5194 CE3A0C CE51A0 CE51A8 CE51B0 CE3A0C CE3A0C CE3A0C
    {{"aTurnGun", "", "GunID:", "H.Angle", "V.Angle", "", "", ""}, {0, 1, 1, 1, 0, 0}, 0x00u},
    // 16: CALL 0045D480 at 00462E8D; literals CE5158 CE3A0C CE3A0C CE3A0C CE3A0C CE5164 CE516C CE3A0C
    {{"uSetDepth", "", "", "", "", "Depth:", "Surface|Periscope|Underwater|Maxdepth", ""}, {0, 0, 0, 0, 1, 0}, 0x00u},
    // 17: CALL 0045D480 at 00463093; literals CE5148 CE3A0C CE52F0 CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"sForceAAFlak", "", "PlatfrmID:", "", "", "", "", ""}, {0, 1, 0, 0, 0, 0}, 0x00u},
    // 18: CALL 0045D480 at 00463299; literals CE5138 CE3A0C CE5140 CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"cFOV", "", "FOV:", "", "", "", "", ""}, {0, 1, 0, 0, 0, 0}, 0x00u},
    // 19: CALL 0045D480 at 004634A0; literals CE511C CE3A0C CE5128 CE5130 CE3A0C CE3A0C CE3A0C CE3A0C
    {{"cBlackOut", "", "Alpha:", "Time:", "", "", "", ""}, {0, 1, 1, 0, 0, 0}, 0x00u},
    // 20: CALL 0045D480 at 004636A9; literals CE50D0 CE3A0C CE50E0 CE3A0C CE3A0C CE50EC CE50F4 CE5364
    {{"aParticleEffect", "", "StartTime:", "", "", "Name:", "JetEngineBig|RealGiantSmoke|GiantSmoke", "Offset:"}, {0, 1, 0, 0, 1, 1}, 0x00u},
    // 21: CALL 0045D480 at 004637F4; literals CE50AC CE3A0C CE3A0C CE3A0C CE3A0C CE50B8 CE50C4 CE3A0C
    {{"aDont_Move", "", "", "", "", "Stay/Go:", "Stay|Move", ""}, {0, 0, 0, 0, 1, 0}, 0x3Fu},
    // 22: CALL 0045D480 at 00463900; literals CE50A0 CE3A0C CE5140 CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"cFOV_Now", "", "FOV:", "", "", "", "", ""}, {0, 1, 0, 0, 0, 0}, 0xFFu},
    // 23: CALL 0045D480 at 00463A0C; literals CE507C CE3A0C CE3A0C CE3A0C CE3A0C CE5088 CE5090 CE3A0C
    {{"pSetGear", "", "", "", "", "Gear:", "Auto|Down|Up", ""}, {0, 0, 0, 0, 1, 0}, 0xFFu},
    // 24: CALL 0045D480 at 00463B18; literals CE5028 CE3A0C CE5038 CE3A0C CE3A0C CE5044 CE5050 CE5074
    {{"cCameraShake", "", "Frequency:", "", "", "ShakeIt:", "CameraPos|LookAtPos|CameraAndLookAt", "Max:"}, {0, 1, 0, 0, 1, 1}, 0xFFu},
    // 25: CALL 0045D480 at 00463C24; literals CE4FFC CE3A0C CE3A0C CE3A0C CE3A0C CE5008 CE5010 CE3A0C
    {{"cBinocular", "", "", "", "", "Action:", "Binocular|Periscope|OFF", ""}, {0, 0, 0, 0, 1, 0}, 0xFFu},
    // 26: CALL 0045D480 at 00463D30; literals CE4FE4 CE3A0C CE4FF4 CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"pTakeOffHack", "", "Angle:", "", "", "", "", ""}, {0, 1, 0, 0, 0, 0}, 0xFFu},
    // 27: CALL 0045D480 at 00463E3C; literals CE4FC4 CE3A0C CE3A0C CE3A0C CE3A0C CE4FD4 CE4FDC CE3A0C
    {{"pWingTipEffect", "", "", "", "", "Effect:", "On|Off", ""}, {0, 0, 0, 0, 1, 0}, 0xFFu},
    // 28: CALL 0045D480 at 00463F48; literals CE4F9C CE3A0C CE3A0C CE3A0C CE3A0C CE4FAC CE4FB4 CE3A0C
    {{"sMoveToPoint", "", "", "", "", "MoveTP:", "Enable|Disable", ""}, {0, 0, 0, 0, 1, 0}, 0xFFu},
    // 29: CALL 0045D480 at 00464054; literals CE4F84 CE3A0C CE3A0C CE3A0C CE3A0C CE4F94 CE4FB4 CE3A0C
    {{"cWideBlackBars", "", "", "", "", "Bar:", "Enable|Disable", ""}, {0, 0, 0, 0, 1, 0}, 0xFFu},
    // 30: CALL 0045D480 at 00464160; literals CE4F6C CE3A0C CE4F7C CE3A0C CE3A0C CE3A0C CE3A0C CE3A0C
    {{"sUseCatapult", "", "Dir:", "", "", "", "", ""}, {0, 1, 0, 0, 0, 0}, 0xFFu},
    // 31: CALL 0045D480 at 0046426F; literals CE4F4C CE3A0C CE3A0C CE3A0C CE3A0C CE4F58 CE4F60 CE3A0C
    {{"aHideEntity", "", "", "", "", "H/S:", "Hide|Show", ""}, {0, 0, 0, 0, 1, 0}, 0xFFu},
};

// 00460B80..004642B4 (4464 insns), __thiscall(ECX = 74h block), plain RET, EAX = block. Only
// caller 004CCE50 (BSP_Game_BeginEngineMovie), which stores the result at game+38h.
// Evidence: disk-byte listing; unwind map 00D88680 (handler 00C618EF, 258 states).
// Block stores in image order: +58h, +5Ch = 0 (a NativeString; EH state 0 -> 0041DD20 on +58h);
// +68h, +6Ch, +70h = 0 (a vector at +64h whose allocator byte is not written; state 1 ->
// 0045CDD0 on +64h); +4h = 0.0f (XORPS/MOVSS); +8h..+54h = 0; +0h = [00CE5380] (MOVSS, the bits
// 41700000h); +60h = 0; EH state 1.
// Then, when the vector's first pointer [00E18554] is null or its size
// (last - first) / 58h (IMUL 2E8BA2E9h / SAR 4 plus the sign bit: signed, truncating) is 0, the
// 32 records are built: for each call the eight strings are built from the last argument to the
// first, either inlined (zero both words, 0041DD40(strlen, 1), and when the data pointer is
// non-null memcpy(data, literal, string length + 1) using the length field after the resize) or
// by the 0041E870 constructor. The string being built is owned by the frame only once the next
// state store runs: states 2..8 (per record) cover the strings already built, so an exception in
// a constructor unwinds the completed strings (most recent first), then +64h and +58h. Before
// 0045D480 the state returns to 1 (the callee owns the strings); the returned record is pushed
// (00460AD0 on 00E18550) under the record's own state (0045CFC0 on the temporary), then state 1
// and 0045CFC0 releases the temporary.
// x87: none (MOVSS only). Literals are .rdata addresses in the image; the C++ passes equal text.
// Uncertainty: the block's other fields are not named; the unwind order is modelled with
// catch/rethrow rather than the FH3 table.
std::byte* engine_movie_construct_00460b80(std::byte* block, std::byte* vector_00e18550,
                                           std::uint32_t clock_bits_00ce5380,
                                           EngineMovieConstructHost& host) {
    auto put = [block](std::uint32_t offset, std::uint32_t value) {
        std::memcpy(block + offset, &value, sizeof(value));
    };
    put(0x58, 0);
    put(0x5c, 0);
    put(0x68, 0);
    put(0x6c, 0);
    put(0x70, 0);
    const float zero = 0.0f;
    std::memcpy(block + 0x4, &zero, sizeof(zero));
    for (std::uint32_t offset = 0x8; offset <= 0x54; offset += 4) put(offset, 0);
    put(0x0, clock_bits_00ce5380);
    put(0x60, 0);

    std::uint32_t first = 0, last = 0;
    std::memcpy(&first, vector_00e18550 + 4, sizeof(first));
    std::memcpy(&last, vector_00e18550 + 8, sizeof(last));
    if (first != 0 && static_cast<std::int32_t>(last - first) / 0x58 != 0) return block;

    try {
        for (const MovieActionTypeCall& call : kMovieActionTypeCalls_00460b80) {
            MovieNativeString strings[8]{};
            int built = 0;  // strings 7..(8 - built) are owned by the frame
            try {
                for (int k = 7; k >= 0; --k) {
                    MovieNativeString& s = strings[k];
                    if (call.constructor_mask & (1u << k)) {
                        host.construct_native_string_0041e870(s, call.strings[k]);
                    } else {
                        s.length = 0;
                        s.data = nullptr;
                        host.native_string_resize_0041dd40(
                            s, static_cast<std::uint32_t>(std::strlen(call.strings[k])), 1);
                        if (s.data != nullptr) host.memcpy_00bf7680(s.data, call.strings[k], s.length + 1);
                    }
                    ++built;
                }
            } catch (...) {
                // The string under construction is not yet covered by a state.
                for (int k = 8 - built; k <= 7; ++k) host.native_string_destroy_0041dd20(strings[k]);
                throw;
            }
            alignas(4) std::byte record[0x58];
            void* constructed = host.movie_action_type_construct_0045d480(record, strings, call.flags);
            try {
                host.movie_action_types_push_back_00460ad0(vector_00e18550, constructed);
            } catch (...) {
                host.movie_action_type_destroy_0045cfc0(record);
                throw;
            }
            host.movie_action_type_destroy_0045cfc0(record);
        }
    } catch (...) {
        host.vector_tidy_0045cdd0(block + 0x64);
        host.native_string_destroy_0041dd20(*reinterpret_cast<MovieNativeString*>(block + 0x58));
        throw;
    }
    return block;
}

}  // namespace bsp
