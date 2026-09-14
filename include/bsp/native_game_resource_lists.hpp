#pragma once
#include <cstdint>
namespace bsp {
struct SingletonLifetimeCallbacks;

// Actual16h vector view: unused/debug word0, begin4, end8, capacityEndC.
// Values are raw DWORD pointers. No item retain, release or type operation.
// All entries below are new C++ interfaces; source exception services use the
// current CRT, not native exception objects/FH3, private spills or binary ABI.
// Pointer differences use wrapping32-bit arithmetic followed by SAR2.
// Copy calls use the existing CRT memmove_s contract and ignore its status.
// Full insertion paths preserve native reads/publication order; no rollback is
// added for allocation or copy exceptions. Value is captured before count0.
// Invalid-iterator callbacks may return and mutate the current vector fields.
// Caller supplies valid raw storage; no corruption or bounds repair is added.
#define BSP_DECLARE_GAME_LIST_FAMILY(tag,alloc,fillrange,backward,copy,fill,insert,one,append) \
void* allocate_native_game_##tag##_pointers_##alloc(std::uint32_t count); \
void fill_native_game_##tag##_range_##fillrange(void* first,void* last,const void* value); \
void* copy_native_game_##tag##_backward_##backward(const void* first,const void* last,void* destination_end); \
void* copy_native_game_##tag##_range_##copy(const void* first,const void* last,void* destination); \
void* fill_native_game_##tag##_copies_##fill(void* destination,std::uint32_t count,const void* value); \
void insert_native_game_##tag##_copies_##insert(void* list,void* unused_owner,void* position,std::uint32_t count,const void* value); \
void* insert_native_game_##tag##_one_##one(void* list,void* output,void* iterator_owner,void* position,const void* value,const SingletonLifetimeCallbacks&); \
void append_native_game_##tag##_pointer_##append(void* list,const void* value,const SingletonLifetimeCallbacks&);
BSP_DECLARE_GAME_LIST_FAMILY(type44,00714120,00716a50,00716a70,00718350,007188c0,0071a5a0,0071b230,0071b8d0)
BSP_DECLARE_GAME_LIST_FAMILY(type54,00714180,00716aa0,00716ac0,00718380,007188f0,0071a760,0071b2c0,0071b940)
BSP_DECLARE_GAME_LIST_FAMILY(type64,007141e0,00716af0,00716b10,007183b0,00718920,0071a920,0071b350,0071b9b0)
#undef BSP_DECLARE_GAME_LIST_FAMILY

// Native allocation: ECX count, EDX unused, RET. Checked count*4 through BF681B;
// overflow raises bad_alloc via the original CRT throw machinery.
// Fill-range/backward-copy: three stack inputs, cdecl RET. Copy/fill-copies:
// three stack inputs, RET12; incoming ECX unused; EAX resulting pointer.
// Insert-copies: ECX list, stack owner/position/count/value, RET16, no result.
// Insert-one: ECX list, stack output/owner/position/value, EAX output, RET16.
// Append: ECX list, stack value-address, RET4. Debug word0 is untouched.
// Ordinary raw copies do not reproduce native fault delivery, errno/handler
// ownership or original throw-object layout. See NATIVE_GAME_RESOURCE_LISTS_BX.
} // namespace bsp
