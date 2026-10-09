#include "bsp/native_unit_gunnery_category_header.hpp"

#include <cstddef>
#include <type_traits>

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);
static_assert(std::is_standard_layout_v<NativeUnitGunneryCategoryHeaderStorage>);
static_assert(sizeof(NativeUnitGunneryCategoryHeaderStorage) == 0x0c);
static_assert(alignof(NativeUnitGunneryCategoryHeaderStorage) == 4);
static_assert(offsetof(NativeUnitGunneryCategoryHeaderStorage, word_00) == 0);
static_assert(offsetof(NativeUnitGunneryCategoryHeaderStorage, word_04) == 4);
static_assert(offsetof(NativeUnitGunneryCategoryHeaderStorage, word_08) == 8);

__declspec(naked) NativeUnitGunneryCategoryHeaderStorage* __fastcall
construct_native_unit_gunnery_category_header_00952640(
    NativeUnitGunneryCategoryHeaderStorage*) {
    __asm {
        mov eax, ecx                     // 00952640
        xor ecx, ecx                     // 00952642
        mov dword ptr [eax], ecx         // 00952644
        mov dword ptr [eax + 4], ecx     // 00952646
        mov dword ptr [eax + 8], ecx     // 00952649
        ret                              // 0095264C
    }
}

} // namespace bsp
