#pragma once

// cc15_g24d: four segment 61 game bodies the cc15-stlA audit moved from stl_probable to game_code
// (docs/CC15_STL_PROBABLE_AUDIT.md, packet cc15-g24d, docs/CC15_G24D.md): the spawn manager's
// request-list erase by id (00943E40) and by entity (00943EC0), its per-player usage sum
// (00946970), and the quote-aware string split (0094EDC0). Names are hypotheses, not recovered
// symbols.
//
// Same conventions as cc13_left61 / cc13_large61: raw image objects touched only at the offsets
// the image uses (Raw), one named host hook per native call, at the same point with the same
// arguments, and the inlined MSVC9 checked-iterator tests (00BF6713, which returns) kept in their
// places with the image's re-reads. Exception unwinding (0094EDC0's EH frame around the temporary
// string) is not modelled: the hooks are assumed not to throw.
//
// Status: reconstructed, build-tested only; new C++ interfaces, not ABI-compatible, not
// fixture-tested, not game-validated. Nothing is wired into a game host. The signatures match the
// existing seams (cc13_large61 spawn_manager_remove_request_00943e40 /
// spawn_manager_player_usage_00946970, cc13_seg52b string_vector_split_0094edc0), so a host can
// implement those virtuals by calling these bodies.

#include <cstdint>

#include "bsp/cc13_left61.hpp"

namespace bsp::cc15_g24d {

using bsp::cc13_left61::Raw;

// The spawn manager's request list, an MSVC9 std::list<Request*> at the manager's +0:
// +4 head (sentinel) node, +8 size. Node: +0 next, +4 prev, +8 the request pointer.
constexpr std::size_t kListHead = 0x4;
constexpr std::size_t kListSize = 0x8;
constexpr std::size_t kNodeNext = 0x0;
constexpr std::size_t kNodePrev = 0x4;
constexpr std::size_t kNodeValue = 0x8;
// The request's id (compared with the message +58h id and the entity +28Ch id).
constexpr std::size_t kRequestId = 0x7c;
// The entity's spawn request id read by 00943EC0.
constexpr std::size_t kEntityRequestId = 0x28c;

// An MSVC9 std::string (0x1C bytes): +4 the inline buffer or the heap pointer, +14h size,
// +18h capacity (heap when >= 10h).
constexpr std::size_t kStringBuffer = 0x4;
constexpr std::size_t kStringSize = 0x14;
constexpr std::size_t kStringCapacity = 0x18;
constexpr std::size_t kStringBytes = 0x1c;
constexpr std::uint32_t kStringInlineLimit = 0x10;

struct Cc15G24dHost {
    virtual ~Cc15G24dHost() = default;

    // --- CRT ----------------------------------------------------------------
    // 00BF6713 _invalid_parameter_noinfo, the checked-iterator report. The image carries on
    // after it returns.
    virtual void checked_iterator_report_00bf6713() = 0;
    // 00BF65AC _free (cdecl).
    virtual void free_00bf65ac(void* block) = 0;
    // 00BF86F0 _strchr (cdecl): the delimiter-set test; returns null when absent. The
    // character is the MOVSX of the string byte.
    virtual const char* strchr_00bf86f0(const char* set, std::int32_t character) = 0;

    // --- the spawn request list ---------------------------------------------
    // 009439B0 STL_inst_009439b0 (the list erase): __thiscall(ECX = list; out iterator*,
    // iterator {container, node} by value), RET 0Ch. The image's 00943EC0 passes the list itself
    // as both ECX and the iterator's container.
    virtual void request_list_erase_009439b0(Raw list, void* out_iterator, void* iterator_container,
                                             void* iterator_node) = 0;
    // 00946870 (the members' resource usage for one player): __thiscall(ECX = request;
    // player), RET 4, ST0. Kept as float: the caller FADDs it to a float spilled with FSTP float.
    virtual float request_player_usage_00946870(void* request, std::uint32_t player) = 0;

    // --- std::string / std::vector<std::string> ------------------------------
    // 004CDBE0 STL_LegacySboString_ConstructSubstring_004cdbe0: __thiscall(ECX = source string;
    // out string*, offset, count), RET 0Ch, EAX = out.
    virtual void* string_construct_substring_004cdbe0(const std::uint8_t* source, void* out,
                                                      std::uint32_t offset, std::uint32_t count) = 0;
    // 00849170 STL_Vector_PushBack_00849170: __thiscall(ECX = vector<string>; const string*),
    // RET 4.
    virtual void string_vector_push_back_00849170(std::uint8_t* vector, const void* string) = 0;
};

// 00943E40 BSP_SpawnManager_RemoveRequest: __thiscall(ECX = manager; id), RET 4. Erases the
// first node whose request +7Ch equals id, if any.
void spawn_manager_remove_request_00943e40(Raw manager, std::uint32_t id, Cc15G24dHost& host);

// 00943EC0 BSP_SpawnManager_RemoveRequestForEntity: __thiscall(ECX = manager; entity), RET 4.
// Id = entity +28Ch; nothing when 0, else the first matching node goes through 009439B0.
void spawn_manager_remove_request_for_entity_00943ec0(Raw manager, Raw entity, Cc15G24dHost& host);

// 00946970 BSP_SpawnManager_SumRequestValue: __thiscall(ECX = manager; player), RET 4,
// ST0 = the float sum of 00946870(request, player) over the list.
float spawn_manager_player_usage_00946970(Raw manager, std::uint32_t player, Cc15G24dHost& host);

// 0094EDC0 BSP_String_SplitQuoted: __fastcall(ECX = vector<string>, EDX = const string*;
// const char* separators), RET 4. Pushes every maximal run of non-separator characters
// (separators inside "..." do not split; the quotes are kept) as a new string.
void string_vector_split_0094edc0(std::uint8_t* vector, std::uint8_t* text, const char* separators,
                                  Cc15G24dHost& host);

}  // namespace bsp::cc15_g24d
