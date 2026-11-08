// Focused PC24 comparison against isolated original PE bytes, without a game
// process. Also compare the wanderer's actual accumulation fragment, whose
// final FADD reverses the shared helper's operands.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "bsp/band_0067_cc13.hpp"
#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

using Vec3 = bsp::band_0067_cc13::Vec3;

static void* original_bytes(const std::vector<char>& image, unsigned address,
    unsigned size, bool append_ret = false) {
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(image.data());
    const auto* pe = reinterpret_cast<const IMAGE_NT_HEADERS32*>(image.data() + dos->e_lfanew);
    const auto* section = IMAGE_FIRST_SECTION(pe);
    for (unsigned i = 0; i < pe->FileHeader.NumberOfSections; ++i, ++section) {
        const unsigned rva = address - pe->OptionalHeader.ImageBase;
        if (rva < section->VirtualAddress ||
            rva + size > section->VirtualAddress + section->SizeOfRawData) continue;
        const auto offset = section->PointerToRawData + rva - section->VirtualAddress;
        if (offset + size > image.size()) throw std::runtime_error("short PE range");
        auto* memory = static_cast<unsigned char*>(VirtualAlloc(nullptr, size + 1,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        if (!memory) throw std::runtime_error("VirtualAlloc");
        std::memcpy(memory, image.data() + offset, size);
        if (append_ret) memory[size] = 0xc3;
        DWORD previous;
        if (!VirtualProtect(memory, size + 1, PAGE_EXECUTE_READ, &previous))
            throw std::runtime_error("VirtualProtect");
        FlushInstructionCache(GetCurrentProcess(), memory, size + 1);
        return memory;
    }
    throw std::runtime_error("missing original PE range");
}

// Set up only the input registers/stack slots of 007BE23C..007BE279. The
// executable fragment above is copied unmodified from the installed binary.
__declspec(naked) static void __cdecl accumulate_fragment(void*, Vec3*, const Vec3*, float) {
    __asm {
        push edi
        sub esp, 30h
        mov edi, dword ptr [esp + 3ch]
        mov edx, dword ptr [esp + 40h]
        mov eax, dword ptr [esp + 44h]
        mov dword ptr [esp + 10h], eax
        mov eax, dword ptr [edx]
        mov dword ptr [esp + 14h], eax
        mov eax, dword ptr [edx + 4]
        mov dword ptr [esp + 18h], eax
        mov eax, dword ptr [edx + 8]
        mov dword ptr [esp + 1ch], eax
        call dword ptr [esp + 38h]
        add esp, 30h
        pop edi
        ret
    }
}

static unsigned exceptions() {
    unsigned short status;
    __asm { fnstsw status }
    return status & 0x3fu;
}
int main(int argc, char** argv) {
    if (argc != 2) return 2;
    std::ifstream stream(argv[1], std::ios::binary);
    const std::vector<char> image((std::istreambuf_iterator<char>(stream)), {});
    if (image.size() < 0x1000) return 3;
    auto sub = reinterpret_cast<void (__fastcall*)(Vec3*, const Vec3*, const Vec3*)>(
        original_bytes(image, 0x00709ac0u, 0x3bu));
    auto accumulate = reinterpret_cast<void (__fastcall*)(Vec3*, const Vec3*, float)>(
        original_bytes(image, 0x00709b60u, 0x43u));
    void* fragment = original_bytes(image, 0x007be23cu, 0x40u, true);

    unsigned short previous, pc24;
    __asm { fnstcw previous }
    pc24 = static_cast<unsigned short>((previous & ~0x0f00u) | 0x003fu);
    __asm { fldcw pc24 }
    constexpr std::array<unsigned, 16> edges{
        0, 0x80000000u, 1, 0x80000001u, 0x007fffffu, 0x00800000u,
        0x3f800000u, 0xbf800000u, 0x7f7fffffu, 0xff7fffffu,
        0x7f800000u, 0xff800000u, 0x7fc01234u, 0xffc01234u,
        0x7f801234u, 0xff805678u};
    unsigned checks = 0, failures = 0, candidate_checks = 0, candidate_differences = 0;
    for (unsigned i = 0; i < edges.size(); ++i) {
        for (unsigned j = 0; j < edges.size(); ++j) {
            const unsigned a_bits[3]{edges[i], edges[j], edges[(i + j) % 16]};
            const unsigned b_bits[3]{edges[j], edges[i], edges[(i * 3 + j) % 16]};
            Vec3 a, b;
            std::memcpy(&a, a_bits, sizeof a);
            std::memcpy(&b, b_bits, sizeof b);
            Vec3 expected;
            __asm { fnclex }
            sub(&expected, &a, &b);
            const unsigned expected_status = exceptions();
            __asm { fnclex }
            const Vec3 actual = bsp::band_0067_cc13::vec3_sub_00709ac0(a, b);
            const unsigned actual_status = exceptions();
            ++checks;
            if (std::memcmp(&expected, &actual, sizeof actual) || expected_status != actual_status) {
                ++failures;
            }
            for (unsigned scale : edges) {
                float s;
                std::memcpy(&s, &scale, sizeof s);
                Vec3 leaf = a, caller = a;
                __asm { fnclex }
                accumulate(&leaf, &b, s);
                const unsigned leaf_status = exceptions();
                __asm { fnclex }
                accumulate_fragment(fragment, &caller, &b, s);
                const unsigned caller_status = exceptions();
                ++candidate_checks;
                if (std::memcmp(&leaf, &caller, sizeof leaf) || leaf_status != caller_status)
                    ++candidate_differences;
            }
        }
    }
    __asm { fnclex }
    __asm { fldcw previous }
    std::printf("PC24 subtraction: %u checks, %u failures (bits and exception flags)\n", checks, failures);
    std::printf("Skipped scaled-accumulate: %u/%u native helper/caller differences\n",
        candidate_differences, candidate_checks);
    VirtualFree(reinterpret_cast<void*>(sub), 0, MEM_RELEASE);
    VirtualFree(reinterpret_cast<void*>(accumulate), 0, MEM_RELEASE);
    VirtualFree(fragment, 0, MEM_RELEASE);
    return failures || !candidate_differences ? 1 : 0;
}
