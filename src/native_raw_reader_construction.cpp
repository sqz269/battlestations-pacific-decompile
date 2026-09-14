#include "bsp/native_raw_reader_construction.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4, "The native reader stores Win32 pointers.");

// Exact call-free base primitive, including its ECX clobber and EAX return.
// Assembly keeps native unaligned DWORD access and ordered individual stores.
__declspec(naked) void* __fastcall construct_native_raw_stream_reader_base_00bf09a0(void*) {
    __asm {
        mov eax, ecx
        xor ecx, ecx
        mov dword ptr [eax], ecx
        mov dword ptr [eax + 4], ecx
        mov dword ptr [eax + 8], ecx
        mov dword ptr [eax + 0Ch], ecx
        ret
    }
}

void* construct_native_raw_reader_path_storage_00bea150(void* actual_reader70h) {
    construct_native_raw_stream_reader_base_00bf09a0(actual_reader70h);
    auto* const path_headers = static_cast<unsigned char*>(actual_reader70h) + 0x10;
    for (unsigned i = 0; i != 10; ++i)
        initialize_native_string_header_00415270(path_headers + i * 8);

    // BEA195/BEA198/BEA19F/BEA1A2: publish these fields only after all ten
    // actual header constructors. These disjoint native stores neither
    // dereference a provider nor require aligned C++ DWORD subobjects.
    __asm {
        mov eax, actual_reader70h
        mov dword ptr [eax + 60h], 0
        mov dword ptr [eax + 64h], 0FFFFFFFFh
        mov dword ptr [eax + 68h], 0
        mov dword ptr [eax + 6Ch], 0
    }
    return actual_reader70h;
}

} // namespace bsp
