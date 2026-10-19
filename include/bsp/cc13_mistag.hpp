// Library-tagged rows that are game code, packet cc13-mistag (marker cc13_mistag).
//
// The filter (docs/CC13_MISTAG.md) looked for library-tagged open rows in the 16 approved
// segments that call game routines, touch game globals or name game strings. This file holds
// the small ones: 23 one-instruction getters (MOV EAX, imm32; RET) and four singleton deleting
// destructors whose out-of-line base copy is not a recorded function.
//
// Model: raw bytes (Win32 layout, image offsets); image addresses are kept as 32-bit image
// values; callees go through MistagHost. A new C++ interface, not a drop-in binary
// replacement; nothing here is wired into bsp_game. Names are hypotheses.

#ifndef BSP_CC13_MISTAG_HPP
#define BSP_CC13_MISTAG_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_mistag {

// One getter: `address` is the routine, `slot` the .rdata vtable slot that holds it, `value`
// the image address returned in EAX, `text` the C string at `value` when it is one (else the
// returned object is a .data global).
struct TrivialGetter {
    std::uint32_t address;
    std::uint32_t slot;
    std::uint32_t value;
    const char* text;
};
inline constexpr std::size_t kTrivialGetterCount = 23;
extern const TrivialGetter kTrivialGetters[kTrivialGetterCount];

// The EAX of the getter at `address` (__thiscall or __cdecl, no arguments read, plain RET);
// 0 when `address` is not one of the table's routines.
std::uint32_t trivial_getter_result(std::uint32_t address) noexcept;

inline constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818;

class MistagHost {
public:
    virtual ~MistagHost() = default;
    // MOV dword [global], 0: the singleton publication slot.
    virtual void clear_global(std::uint32_t global) = 0;
    // 00BF65AC _free.
    virtual void free_00bf65ac(void* block) = 0;
};

// The singleton scalar deleting destructor shape: [global] = 0, vtable 00CE3818, _free(this)
// when flags & 1; __thiscall(ECX = object, flags), RET 4, EAX = object.
// 00735880: global 010904F8; 00735900: global 0109029C; 00866090: global 00F87658.
void* singleton_scalar_deleting_destruct_00735880(MistagHost& host, void* object, std::uint32_t flags);
void* singleton_scalar_deleting_destruct_00735900(MistagHost& host, void* object, std::uint32_t flags);
void* singleton_scalar_deleting_destruct_00866090(MistagHost& host, void* object, std::uint32_t flags);
// 00737110: the same with the singleton base at +4h and vtable 00CFE9F4 at +0h (global 010904F8).
void* msar_provider_scalar_deleting_destruct_00737110(MistagHost& host, void* object, std::uint32_t flags);

}  // namespace bsp::cc13_mistag

#endif
