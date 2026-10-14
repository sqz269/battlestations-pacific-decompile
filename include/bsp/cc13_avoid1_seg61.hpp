#pragma once

// cc13_avoid1, packet B (segment 61): the spawn request completion 00947A30.
// docs/CC13_AVOID1.md. Names are hypotheses, not recovered symbols; a new C++
// interface over the native record, not a drop-in replacement. Not wired in.

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_left55.hpp"

namespace bsp::cc13_avoid1 {

using cc13_left55::LuaVariant;
using cc13_left55::NativeStr;

// A VS2005 vector of dwords as it sits in a record: proxy word, first, last, end.
struct Cc13NativeDwordVector {
    std::uint32_t proxy;                   // +00h
    std::uint32_t* first;                  // +04h
    std::uint32_t* last;                   // +08h
    std::uint32_t* end;                    // +0Ch
};
static_assert(sizeof(Cc13NativeDwordVector) == 0x10);

// The words of the spawn request record (BSP_SpawnRequest_CreateMembers 009483D0's
// ECX) that 00947A30 reads: the callback name at +84h and the created members'
// entity pointers at +CCh.
struct Cc13SpawnRequestCompletionNative {
    std::uint8_t unread_00[0x84];
    NativeStr callback;                    // +84h size, +88h data
    std::uint8_t unread_8c[0x40];
    Cc13NativeDwordVector members;         // +CCh, elements at +D0h..+D4h
};
static_assert(offsetof(Cc13SpawnRequestCompletionNative, callback) == 0x84);
static_assert(offsetof(Cc13SpawnRequestCompletionNative, members) == 0xcc);

// The 0Ch-byte LuaVariant vector 00947A30 allocates (first, last, end).
struct Cc13LuaVariantVectorNative {
    LuaVariant* first;
    LuaVariant* last;
    LuaVariant* end;
};
static_assert(sizeof(Cc13LuaVariantVectorNative) == 0x0c);

class Cc13SpawnCompletionHost {
public:
    virtual ~Cc13SpawnCompletionHost() = default;
    // 00947810, __thiscall(ECX = destination; const vector* source), RET 4:
    // the copy constructor of the member vector, into the by-value argument slot.
    virtual void member_vector_copy_00947810(Cc13NativeDwordVector& destination,
                                             const Cc13NativeDwordVector& source) = 0;
    // [[00E198C4]+BCh], the ECX of 0065D070.
    virtual void* support_orders_owner_00e198c4_bc() = 0;
    // 0065D070, __thiscall(owner; vector by value): the support-manager member
    // orders. The callee owns (and destroys) the by-value copy.
    virtual void support_member_orders_0065d070(void* owner,
                                                Cc13NativeDwordVector& by_value) = 0;
    // 00A16EF0, __fastcall(ECX = &name, EDX = &members): the script AI group.
    virtual void ai_group_from_script_00a16ef0(const NativeStr& name,
                                               const Cc13NativeDwordVector& members) = 0;
    // 00BF681B operator new(0Ch).
    virtual void* allocate_00bf681b(std::uint32_t bytes) = 0;
    // 00BF6713, the checked-index report; must not return.
    virtual void invalid_parameter_00bf6713() = 0;
    // 00884A90, __thiscall(ECX variant; entity), RET 4: the entity variant.
    virtual void variant_construct_entity_00884a90(LuaVariant& variant, std::uint32_t entity) = 0;
    // 006EDF00, __thiscall(ECX vector; const variant*), RET 4: push_back.
    virtual void variant_vector_push_back_006edf00(Cc13LuaVariantVectorNative* vector,
                                                   const LuaVariant& variant) = 0;
    // 00884AF0, __fastcall(ECX variant): the variant destructor.
    virtual void variant_destruct_00884af0(LuaVariant& variant) = 0;
    // [[00E188A8]+1A08h], the mission Lua runner.
    virtual void* lua_runner_00e188a8_1a08() = 0;
    // 00887E50, __thiscall(runner; 0, const NativeStr* name, vector* args, 0, -1).
    virtual void call_named_00887e50(void* runner, std::uint32_t zero0, const NativeStr& name,
                                     Cc13LuaVariantVectorNative* args, std::uint32_t zero1,
                                     std::int32_t minus_one) = 0;
    // 006B88E0, __thiscall(ECX vector; 0), RET 4: resize to zero.
    virtual void variant_vector_resize_006b88e0(Cc13LuaVariantVectorNative* vector,
                                                std::uint32_t size) = 0;
    // 00BF6989 / 00BF65AC, the two frees.
    virtual void free_array_00bf6989(void* block) noexcept = 0;
    virtual void free_record_00bf65ac(void* block) noexcept = 0;
};

// 00947A30, __fastcall(ECX request), RET; called at 009487A1 from 009483D0
// after every member is made and before +C0h = 1. Empty callback: nothing.
// "[supportmanager]" (00CF6B78, case-insensitive 00425850): 0065D070 with a
// copy of the members. Any other '['-name: 00A16EF0(name, members). Otherwise
// the named Lua global is called through 00887E50 with one entity variant per
// member, in member order, and the argument vector is released.
void cc13_spawn_request_complete_00947a30(Cc13SpawnRequestCompletionNative& request,
                                          Cc13SpawnCompletionHost& host);

}  // namespace bsp::cc13_avoid1
