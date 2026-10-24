#pragma once
// cc15_dyn_live: the referenced (live) Dyn/Mit bodies of segment 0, the first linked object's Mit/Dyn
// header run (docs/CC13_DYN_LIBRARY.md): the two global Mit::cPoolAllocator pools <36,128> and
// <20,128>, the 128-bucket edge hash map built on <20,128>, a fixed-block pool constructor, two
// vector-of-dword helpers, the profiler timer calibration and a 1024-byte vsprintf_s wrapper.
// Reconstructed, build-tested only; nothing is wired. Descriptive names are hypotheses, not
// recovered symbols. Evidence, ABI and uncertainty per row: docs/CC15_DYN_LIVE.md.
//
// The image addresses both pools by absolute address (<36,128> at 0109EA0C, <20,128> at 0109ED28);
// these functions take the instance. The CRT calls the image makes are injected through DynLiveCrt
// so the host decides which heap backs them: 00BF681B operator new, 00BF55BE operator new (thunk),
// 00BF6989 _free, 00BF9F1A _malloc. EnterCriticalSection/LeaveCriticalSection (IAT [00CE2218] /
// [00CE2210]), Sleep ([00CE2230]), RDTSC, __allmul and _vsprintf_s are bound to the host's.

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_dyn2.hpp"

namespace bsp::cc15_dyn_live {

struct DynLiveCrt {
    void* (*operator_new_00bf681b)(std::uint32_t bytes);
    void* (*operator_new_00bf55be)(std::uint32_t bytes);
    void (*free_00bf6989)(void* p);
    void* (*malloc_00bf9f1a)(std::uint32_t bytes);
};

// ---- Mit::cPoolAllocator<N,128> ----
// Both global pools have the same layout; the field offsets of <20,128> are proven by the absolute
// operands of 0040D860 (lock 0109ED34 = base+0Ch, depth 0109ED4C = +24h, pages 0109ED50 = +28h,
// count 0109ED54 = +2Ch, capacity 0109ED58 = +30h, first free page 0109ED5C = +34h).
using MitPoolAllocator = cc13_dyn2::MitPoolAllocator36x128;

// Lifecycle bodies at 00407590/00407690 and 0040F910/0040FA10. The original
// functions use absolute globals and take no arguments; constructors return the
// global in EAX, all four use plain RET, and ESI/EDI are preserved. These C++
// interfaces take actual pool and shared 00E188B4 head storage instead. The
// concrete/base vtable words remain image identities, not callable host vptrs.
// C++ allocation exceptions unwind table, lock, then registry links. Original
// FH3/SEH transport and binary ABI are not supplied. docs/OCT24_TAIL_POOL.md.
struct MitPoolLifecycleCrt {
    void* (*operator_new_00bf55be)(std::uint32_t bytes);
    void (*delete_00bf65ac)(void* page);
    void (*free_00bf6989)(void* table);
};

MitPoolAllocator* pool36_construct_00407590(MitPoolAllocator& pool,
    void*& allocator_registry_head_00e188b4, const MitPoolLifecycleCrt& crt);
void pool36_destroy_global_00407690(MitPoolAllocator& pool,
    void*& allocator_registry_head_00e188b4, const MitPoolLifecycleCrt& crt);
MitPoolAllocator* pool20_construct_0040f910(MitPoolAllocator& pool,
    void*& allocator_registry_head_00e188b4, const MitPoolLifecycleCrt& crt);
void pool20_destroy_global_0040fa10(MitPoolAllocator& pool,
    void*& allocator_registry_head_00e188b4, const MitPoolLifecycleCrt& crt);

// <36,128>: 28h-byte slots (page index at slot+24h), 1504h-byte pages, free stack at page+1400h,
// depth at page+1500h. <20,128>: 18h-byte slots (page index at slot+14h), D04h-byte pages, free
// stack at page+C00h, depth at page+D00h.

// 004084D0, register ABI: EAX = page, EDI = page index; returns EAX = page; plain RET; EBX/ESI
// preserved, ECX/EDX clobbered. Depth = 80h, stack[i] = 7Fh - i, every slot's +24h = page index.
std::uint8_t* pool36_page_construct_004084d0(std::uint8_t* page, std::uint32_t page_index) noexcept;
// 0040DA40, the same for <20,128>: depth +D00h, stack +C00h, slot +14h, stride 18h.
std::uint8_t* pool20_page_construct_0040da40(std::uint8_t* page, std::uint32_t page_index) noexcept;

// 004077F0, no register or stack inputs, plain RET; returns EAX = slot; ESI/EDI preserved.
// Also reached by the unreferenced JMP thunk 00C369A0.
void* pool36_allocate_004077f0(MitPoolAllocator& pool, const DynLiveCrt& crt);
// 0040D860, the same for <20,128>. Also reached by the unreferenced JMP thunk 0040CD90.
void* pool20_allocate_0040d860(MitPoolAllocator& pool, const DynLiveCrt& crt);
// 0040D9D0, register ABI: EAX = slot, plain RET; ESI/EDI preserved. Its only reference is the
// unreferenced JMP thunk 0040CDA0; the same body is inlined in 00408340.
void pool20_free_0040d9d0(MitPoolAllocator& pool, void* slot);

// ---- edge hash map (128 buckets over <20,128> nodes) ----
// Key: two uint16 vertex indices; value: up to two uint16 plus a signed count. The copy loops are
// not bounded by the array size; neither is this source.
struct EdgeEntry {
    std::uint16_t key_a_00;
    std::uint16_t key_b_02;
    std::uint16_t values_04[2];
    std::int32_t value_count_08;
};
static_assert(sizeof(EdgeEntry) == 0x0c);

struct EdgeNode {
    std::uint16_t key_a_00;
    std::uint16_t key_b_02;
    std::uint16_t values_04[2];
    std::int32_t value_count_08;
    EdgeNode* prev_0c;
    EdgeNode* next_10;
    std::uint32_t page_index_14;  // the <20,128> slot trailer
};
static_assert(sizeof(EdgeNode) == 0x18);
static_assert(offsetof(EdgeNode, next_10) == 0x10);

struct EdgeHashMap {
    EdgeNode* buckets_000[128];
    std::uint32_t count_200;
};
static_assert(offsetof(EdgeHashMap, count_200) == 0x200);

// Shared hash of 00407A50/00407AF0: k = (a << 16) + b; h = (k + E092C689h) ^ 4E67C6A7h, then four
// rounds h ^= (h << 5) + (h >> 2) + k; bucket = h & 7Fh.
std::uint32_t edge_hash_bucket(std::uint16_t key_a, std::uint16_t key_b) noexcept;

// 00407A50, register ABI: ECX = key (two uint16), EAX = out; stack (map), RET 4; ESI/EDI
// preserved. *out = the node whose (a << 16) + b equals the key's, else 0. EAX is returned unchanged.
EdgeNode** edge_map_find_00407a50(EdgeNode** out, const EdgeEntry* key, const EdgeHashMap* map) noexcept;
// 004080D0, register ABI: EAX = dst node, ECX = src entry; plain RET; returns EAX = dst. Copies the
// two key shorts, then src.count shorts of the value array, then the count.
EdgeNode* edge_node_assign_004080d0(EdgeNode* dst, const EdgeEntry* src) noexcept;
// 00408470, the same with the key copied as one dword.
EdgeNode* edge_node_construct_00408470(EdgeNode* dst, const EdgeEntry* src) noexcept;
// 00407AF0, register ABI: EBX = entry, EDI = map; plain RET; returns EAX = node; EBP/ESI preserved.
// Hit: tail-JMP 004080D0 (assigns the entry into the node). Miss: count += 1, allocate from <20,128>
// (0040D860), 00408470, push at the bucket head (prev = 0, old head's prev = node).
EdgeNode* edge_map_find_or_insert_00407af0(const EdgeEntry* entry, EdgeHashMap* map,
                                           MitPoolAllocator& pool20, const DynLiveCrt& crt);
// 00408340, __stdcall(map), RET 4: every node of the 128 buckets goes back to <20,128> (the
// inlined 0040D9D0, lock taken per node), then memset(map, 0, 200h) and count = 0.
void edge_map_clear_00408340(EdgeHashMap* map, MitPoolAllocator& pool20);

// ---- fixed-block pool of 44h-byte blocks (0040AF00) ----
// __stdcall(pool), RET 4; returns EAX = pool. Called only by the unreachable
// Dyn::SAPBroadPhaseManager constructor 00C37030 (twice, at +5Ch and +F8h). Field roles beyond the
// stores are not established; offsets are named for what 0040AF00 writes.
struct DynPool68 {
    std::uint8_t** pages_00;        // pointer vector {data, size, capacity}
    std::uint32_t page_count_04;
    std::uint32_t page_capacity_08;
    std::uint8_t* free_head_0c;     // first block of the newest page
    std::uint8_t inline_10[0x3c];   // +90h points here
    std::uint32_t field_4c;         // 0
    std::uint32_t* ptr_50;          // = &inline_54
    std::uint8_t inline_54[0x3c];
    std::uint8_t* ptr_90;           // = inline_10
    std::uint32_t field_94;         // 0
    std::uint32_t field_98;         // 0
};
static_assert(offsetof(DynPool68, ptr_50) == 0x50);
static_assert(offsetof(DynPool68, ptr_90) == 0x90);
static_assert(sizeof(DynPool68) == 0x9c);
// Page: 109A0h bytes = 1000 blocks of 44h; each block's dword at +40h points to the next block,
// the last one's (page+1099Ch) is 0. The first 40h bytes of each block are left uninitialised.
DynPool68* pool68_construct_0040af00(DynPool68* pool, const DynLiveCrt& crt);

// ---- vector of 4-byte elements {data, int size, capacity} ----
struct DynVector4 {
    std::uint32_t* data_00;
    std::int32_t size_04;  // compared signed by 0040EE00 and unsigned by 0040D7B0
    std::uint32_t capacity_08;
};
static_assert(sizeof(DynVector4) == 0x0c);
// 0040D7B0, register ABI: ESI = dst, EBX = src; plain RET; returns EAX = dst; EDI preserved.
DynVector4* vector4_copy_construct_0040d7b0(DynVector4* dst, const DynVector4* src, const DynLiveCrt& crt);
// 0040EE00 and 0040EEF0 (byte-identical bodies), register ABI: ESI = vector, EAX = new capacity;
// plain RET, no meaningful return. capacity = EAX; new buffer of capacity*4 bytes (FFFFFFFFh on
// overflow); copies size elements (signed loop); frees the old buffer when non-null.
void vector4_set_capacity_0040ee00(DynVector4* v, std::uint32_t capacity, const DynLiveCrt& crt);
void vector4_set_capacity_0040eef0(DynVector4* v, std::uint32_t capacity, const DynLiveCrt& crt);

// ---- profiler timer ----
// 00407000, __thiscall(ECX = 0109EA44, unused); plain RET; returns EAX = 0109EA44 (the image
// constant; this source returns the instance passed). Reached only by the static initializer
// 00CC89E0 (MOV ECX,0109EA44 / JMP 00407000). rdtsc, Sleep(100), rdtsc; the 64-bit tick delta
// times 10 (__allmul) goes through FILD/FSTP into the double [0109E9E8] (ticks per second).
void* profile_timer_calibrate_00407000(void* timer, double& ticks_per_second_0109e9e8);

// ---- 0040CFC0 ----
// __cdecl(buffer, format, ...): returns _vsprintf_s(buffer, 400h, format, va).
int format_string_1024_0040cfc0(char* buffer, const char* format, ...);

}  // namespace bsp::cc15_dyn_live
