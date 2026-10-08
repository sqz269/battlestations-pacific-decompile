#include "bsp/native_parent_list_header_storage.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && native_parent_list_header_storage_bytes == 12,
    "The original raw header and physical ABI are Win32.");

// Complete native13, SHA256
// 1e09d34a9de7a48bd061703ef73f341df8596595468b282c5f19c36b6bf0b98d.
// Literal whole encodings retain 33 C9 XOR, EDX/DF preservation and plain RET.
// No synthetic globals, heap bindings, dispatch, or owning parent is supplied.
__declspec(naked) void* __fastcall initialize_native_parent_list_header_storage_004b7ec0(
    void*, std::uint32_t) noexcept {
    __asm {
        _emit 0x8b
        _emit 0xc1
        _emit 0x33
        _emit 0xc9
        _emit 0x89
        _emit 0x08
        _emit 0x89
        _emit 0x48
        _emit 0x04
        _emit 0x89
        _emit 0x48
        _emit 0x08
        _emit 0xc3
    }
}

} // namespace bsp
