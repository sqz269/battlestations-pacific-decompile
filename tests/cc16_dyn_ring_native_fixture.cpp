// One fixed concave-five normal-path differential. Native instructions are
// read from the hash-pinned installed PE, never duplicated as a C++ oracle.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#include <xmmintrin.h>
#include "bsp/cc16_dyn_ring_bindings.hpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#pragma comment(lib, "bcrypt.lib")

namespace {
using U32 = std::uint32_t;
using Owner = bsp::cc16_dyn_ring_owner::DynRingOwner;
using Input = bsp::cc16_dyn_ring_init::DynRingOwnerCount;
static_assert(sizeof(void*) == 4);
constexpr char kCandidate[] = "2f76638f6d2d0dfaa3caac8bb85ad7fa7b776cb3";
constexpr char kBinarySha[] = "b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6";
constexpr char kPayloadSha[] = "9e549deab27f23f5b442846e9c2aea69ad8e4aadbb79d682ee7b2f885bdd1f61";
constexpr U32 kPayloadBytes = 2876u;
struct Span { U32 address, length, offset; };
constexpr Span kSpans[] = {
    {0x00c33510u,310u,0u},
    {0x00c4d5a0u,978u,320u},
    {0x00c4d370u,549u,1312u},
    {0x00c410e0u,168u,1872u},
    {0x00c41190u,155u,2048u},
    {0x00c41230u,158u,2208u},
    {0x00c377f0u,104u,2368u},
    {0x00c37860u,293u,2480u},
    {0x0040d7b0u,92u,2784u},
};
struct Patch { U32 offset, target, kind; };
constexpr Patch kPatches[] = {
    {32u,0u,1u},
    {82u,0u,2u},
    {149u,0u,1u},
    {199u,0u,2u},
    {323u,0u,3u},
    {422u,0u,0u},
    {451u,1312u,0u},
    {476u,2208u,0u},
    {539u,0u,1u},
    {640u,0u,2u},
    {728u,2048u,0u},
    {772u,0u,1u},
    {820u,0u,2u},
    {896u,0u,1u},
    {953u,0u,2u},
    {1049u,0u,1u},
    {1111u,2784u,0u},
    {1143u,0u,2u},
    {1174u,0u,2u},
    {1218u,2784u,0u},
    {1490u,1872u,0u},
    {1560u,0u,1u},
    {1615u,0u,2u},
    {1667u,0u,1u},
    {1716u,0u,2u},
    {1768u,0u,1u},
    {1823u,0u,2u},
    {1957u,2480u,0u},
    {2098u,2368u,0u},
    {2156u,2368u,0u},
    {2188u,1872u,0u},
    {2260u,2368u,0u},
    {2318u,2368u,0u},
    {2351u,1872u,0u},
    {2803u,0u,1u},
};

[[noreturn]] void fail(const char* message) { throw std::runtime_error(message); }
thread_local bool executing_kernel = false;
LONG CALLBACK reject_exception(EXCEPTION_POINTERS*) {
    if (executing_kernel) ExitProcess(90);
    return EXCEPTION_CONTINUE_SEARCH;
}
int __cdecl reject_seh(void*, void*, void*, void*) { ExitProcess(91); }

struct Event { U32 op, id, bytes; };
struct Block { void* pointer{}; U32 bytes{}; bool live{}; };
struct Trace {
    std::array<Block, 32> blocks{};
    std::array<Event, 64> events{};
    U32 count{}, event_count{};
    U32 id(const void* pointer) const {
        if (!pointer) return 0;
        for (U32 i = 0; i < count; ++i)
            if (blocks[i].live && blocks[i].pointer == pointer) return i + 1;
        fail("output pointer is not a live synthetic-heap allocation");
    }
    void clear() noexcept {
        for (U32 i = 0; i < count; ++i) if (blocks[i].live) {
            std::free(blocks[i].pointer); blocks[i].live = false;
        }
    }
    ~Trace() { clear(); }
};
// Test-only callback recording context, not a fabricated native global/service.
thread_local Trace* active_trace = nullptr;
void* __cdecl allocate_block(U32 bytes) {
    Trace* t = active_trace;
    if (!t || t->count == t->blocks.size() || t->event_count == t->events.size()) ExitProcess(92);
    void* p = std::malloc(bytes);
    if (!p) ExitProcess(93);
    const U32 id = ++t->count;
    t->blocks[id - 1] = {p, bytes, true};
    t->events[t->event_count++] = {0, id, bytes};
    return p;
}
void __cdecl free_block(void* pointer) {
    Trace* t = active_trace;
    if (!t || t->event_count == t->events.size()) ExitProcess(94);
    const U32 id = t->id(pointer);
    if (!id) ExitProcess(95);
    t->events[t->event_count++] = {1, id, t->blocks[id - 1].bytes};
    std::free(pointer); t->blocks[id - 1].live = false;
}

std::string sha256(const std::vector<unsigned char>& bytes) {
    BCRYPT_ALG_HANDLE algorithm{};
    unsigned char digest[32];
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
        fail("BCryptOpenAlgorithmProvider failed");
    const auto status = BCryptHash(algorithm, nullptr, 0,
        const_cast<unsigned char*>(bytes.data()), static_cast<ULONG>(bytes.size()), digest, sizeof(digest));
    BCryptCloseAlgorithmProvider(algorithm, 0);
    if (status < 0) fail("BCryptHash failed");
    constexpr char hex[] = "0123456789abcdef";
    std::string result;
    for (unsigned char b : digest) { result += hex[b >> 4]; result += hex[b & 15]; }
    return result;
}

struct NativeCode {
    unsigned char* memory{};
    explicit NativeCode(const char* path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) fail("cannot read installed PE");
        const std::vector<unsigned char> installed((std::istreambuf_iterator<char>(file)),
                                                   std::istreambuf_iterator<char>());
        if (sha256(installed) != kBinarySha) fail("installed-image hash changed");
        IMAGE_DOS_HEADER dos; std::memcpy(&dos, installed.data(), sizeof(dos));
        IMAGE_NT_HEADERS32 nt; std::memcpy(&nt, installed.data() + dos.e_lfanew, sizeof(nt));
        if (nt.OptionalHeader.ImageBase != 0x00400000u || nt.FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
            fail("unexpected PE target");
        const auto section_offset = dos.e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER)
            + nt.FileHeader.SizeOfOptionalHeader;
        std::vector<unsigned char> payload(kPayloadBytes, 0xcc);
        for (const auto& span : kSpans) {
            const U32 rva = span.address - nt.OptionalHeader.ImageBase;
            bool copied = false;
            for (WORD i = 0; i < nt.FileHeader.NumberOfSections; ++i) {
                IMAGE_SECTION_HEADER section;
                std::memcpy(&section, installed.data() + section_offset + i * sizeof(section), sizeof(section));
                if (rva >= section.VirtualAddress && rva + span.length <= section.VirtualAddress + section.SizeOfRawData) {
                    std::memcpy(payload.data() + span.offset,
                        installed.data() + section.PointerToRawData + rva - section.VirtualAddress, span.length);
                    copied = true; break;
                }
            }
            if (!copied) fail("span is not disk backed");
        }
        if (sha256(payload) != kPayloadSha) fail("packed original-byte hash mismatch");
        memory = static_cast<unsigned char*>(VirtualAlloc(nullptr, payload.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        if (!memory) fail("VirtualAlloc failed");
        std::memcpy(memory, payload.data(), payload.size());
        const U32 origin = reinterpret_cast<U32>(memory);
        for (const auto& patch : kPatches) {
            U32 destination = origin + patch.target;
            if (patch.kind == 1) destination = reinterpret_cast<U32>(&allocate_block);
            if (patch.kind == 2) destination = reinterpret_cast<U32>(&free_block);
            if (patch.kind == 3) destination = reinterpret_cast<U32>(&reject_seh);
            const U32 value = patch.kind == 3 ? destination : destination - (origin + patch.offset + 4u);
            std::memcpy(memory + patch.offset, &value, 4);
        }
        DWORD previous;
        if (!VirtualProtect(memory, payload.size(), PAGE_EXECUTE_READ, &previous)) fail("VirtualProtect failed");
        FlushInstructionCache(GetCurrentProcess(), memory, payload.size());
    }
    void* constructor() const {
        for (const auto& span : kSpans) if (span.address == 0x00c4d5a0u) return memory + span.offset;
        fail("constructor span absent");
    }
    ~NativeCode() { if (memory) VirtualFree(memory, 0, MEM_RELEASE); }
};

struct NativeRegisters { U32 before_esp, after_esp, ebx, esi, edi, ebp, eax; };
__declspec(naked) void __cdecl invoke_native(void*, Owner*, const Input*, NativeRegisters*) {
    __asm {
        push ebp
        push ebx
        push esi
        push edi
        mov edx, dword ptr [esp + 32]
        mov dword ptr [edx], esp
        mov ebx, 13572468h
        mov esi, 24681357h
        mov edi, 31415926h
        mov ebp, 27182818h
        push dword ptr [esp + 28]
        push dword ptr [esp + 28]
        call dword ptr [esp + 28]
        mov edx, dword ptr [esp + 32]
        mov dword ptr [edx + 4], esp
        mov dword ptr [edx + 8], ebx
        mov dword ptr [edx + 12], esi
        mov dword ptr [edx + 16], edi
        mov dword ptr [edx + 20], ebp
        mov dword ptr [edx + 24], eax
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret
    }
}
std::uint16_t control_word() { std::uint16_t value; __asm { fnstcw value } return value; }
std::uint16_t top() { std::uint16_t value; __asm { fnstsw value } return static_cast<std::uint16_t>(value & 0x3800u); }
U32 seh_head() {
    U32 value;
    __asm {
        mov eax, dword ptr fs:[0]
        mov value, eax
    }
    return value;
}
struct FpScope {
    std::uint16_t saved = control_word(); U32 csr = _mm_getcsr();
    explicit FpScope(U32 precision) {
        const std::uint16_t desired = static_cast<std::uint16_t>((saved & ~0x0f00u) | precision | 0x003fu);
        __asm {
            fnclex
            fldcw desired
        }
        _mm_setcsr(0x1f80u);
    }
    ~FpScope() {
        const std::uint16_t restore_word = saved;
        __asm {
            fnclex
            fldcw restore_word
        }
        _mm_setcsr(csr);
    }
};

void append_vector(std::vector<U32>& out, const Trace& trace, const U32* data, U32 size, U32 capacity) {
    if (size > capacity || capacity > 32) fail("unexpected fixture vector bounds");
    const U32 id = trace.id(data);
    if (capacity && (!id || trace.blocks[id - 1].bytes < capacity * 4u)) fail("fixture vector custody mismatch");
    out.insert(out.end(), {id, size, capacity});
    for (U32 i = 0; i < size; ++i) out.push_back(data[i]);
}
std::vector<U32> snapshot(const Owner& owner, const Input& input, const Trace& trace) {
    if (owner.ring_00.owner_08 != &input) fail("input owner identity changed");
    std::vector<U32> out{owner.ring_00.count_00, owner.ring_00.opaque_04,
                         owner.next_cursor_30, owner.previous_cursor_34};
    append_vector(out, trace, owner.ring_00.next_0c, owner.ring_00.next_size_10, owner.ring_00.next_capacity_14);
    append_vector(out, trace, owner.ring_00.previous_18, owner.ring_00.previous_size_1c, owner.ring_00.previous_capacity_20);
    append_vector(out, trace, owner.scratch_24.data_00, static_cast<U32>(owner.scratch_24.size_04), owner.scratch_24.capacity_08);
    if (owner.chains_38.size_04 > owner.chains_38.capacity_08 || owner.chains_38.capacity_08 > 8) fail("unexpected chain bounds");
    const U32 chains_id = trace.id(owner.chains_38.data_00);
    if (owner.chains_38.capacity_08 && (!chains_id || trace.blocks[chains_id - 1].bytes < owner.chains_38.capacity_08 * 12u))
        fail("fixture chain custody mismatch");
    out.insert(out.end(), {chains_id, owner.chains_38.size_04, owner.chains_38.capacity_08});
    for (U32 i = 0; i < owner.chains_38.size_04; ++i) {
        const auto& chain = owner.chains_38.data_00[i];
        append_vector(out, trace, chain.data_00, static_cast<U32>(chain.size_04), chain.capacity_08);
    }
    if (owner.pairs_44.size_04 > owner.pairs_44.capacity_08 || owner.pairs_44.capacity_08 > 8) fail("unexpected pair bounds");
    const U32 pairs_id = trace.id(owner.pairs_44.data_00);
    if (owner.pairs_44.capacity_08 && (!pairs_id || trace.blocks[pairs_id - 1].bytes < owner.pairs_44.capacity_08 * 8u))
        fail("fixture pair custody mismatch");
    out.insert(out.end(), {pairs_id, owner.pairs_44.size_04, owner.pairs_44.capacity_08});
    for (U32 i = 0; i < owner.pairs_44.size_04; ++i)
        out.insert(out.end(), {owner.pairs_44.data_00[i].previous_00, owner.pairs_44.data_00[i].next_04});
    return out;
}
std::vector<U32> events(const Trace& trace) {
    std::vector<U32> out;
    for (U32 i = 0; i < trace.event_count; ++i)
        out.insert(out.end(), {trace.events[i].op, trace.events[i].id, trace.events[i].bytes});
    return out;
}
void print_words(const std::vector<U32>& words) {
    std::cout << '[';
    for (std::size_t i = 0; i < words.size(); ++i) { if (i) std::cout << ','; std::cout << words[i]; }
    std::cout << ']';
}

void compare_mode(NativeCode& code, const Input& input, U32 precision, const char* name) {
    FpScope fp(precision);
    const auto expected_cw = control_word(); const U32 expected_csr = _mm_getcsr() & ~0x3fu;
    Owner original{}, reconstructed{}; Trace native_trace, cpp_trace;
    NativeRegisters registers{};
    const U32 fs = seh_head(); const auto original_top = top();
    active_trace = &native_trace;
    executing_kernel = true;
    invoke_native(code.constructor(), &original, &input, &registers);
    executing_kernel = false;
    active_trace = nullptr;
    const bool registers_ok = registers.before_esp == registers.after_esp
        && registers.ebx == 0x13572468u && registers.esi == 0x24681357u
        && registers.edi == 0x31415926u && registers.ebp == 0x27182818u
        && registers.eax == reinterpret_cast<U32>(&original);
    const bool native_controls = control_word() == expected_cw && (_mm_getcsr() & ~0x3fu) == expected_csr;
    const bool native_frames = seh_head() == fs && top() == original_top;
    const auto native_words = snapshot(original, input, native_trace);
    const auto native_events = events(native_trace);
    const bsp::cc15_dyn_live::DynLiveCrt crt{nullptr, &allocate_block, &free_block, nullptr};
    __asm { fnclex }
    _mm_setcsr(expected_csr);
    active_trace = &cpp_trace;
    executing_kernel = true;
    Owner* returned = bsp::cc16_dyn_ring_bindings::construct_dyn_ring_owner_bound_00c4d5a0(reconstructed, input, crt);
    executing_kernel = false;
    active_trace = nullptr;
    const bool cpp_controls = control_word() == expected_cw && (_mm_getcsr() & ~0x3fu) == expected_csr;
    const bool cpp_frames = seh_head() == fs && top() == original_top;
    const auto cpp_words = snapshot(reconstructed, input, cpp_trace);
    const auto cpp_events = events(cpp_trace);
    const bool same = native_words == cpp_words && native_events == cpp_events;
    if (!registers_ok || !native_controls || !cpp_controls || !native_frames || !cpp_frames || returned != &reconstructed || !same)
        fail("normal-path differential mismatch");
    std::cout << "{\"mode\":\"" << name << "\",\"same\":true,\"native_registers_esp\":true,\"fp_fs_restored\":true,\"words\":";
    print_words(native_words); std::cout << ",\"heap_events\":"; print_words(native_events); std::cout << "}";
}
}  // namespace

int main(int argc, char** argv) {
    const auto saved_cw = control_word(); const U32 saved_csr = _mm_getcsr();
    const DWORD saved_error_mode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
    try {
        if (argc != 2) fail("pass the installed PE path");
        NativeCode code(argv[1]);
        const float points[5][2]{{0,0},{2,0},{2,2},{0.8f,0.9f},{0,2}};
        const Input input{reinterpret_cast<U32>(points), 5};
        void* exception_guard = AddVectoredExceptionHandler(1, &reject_exception);
        if (!exception_guard) fail("exception isolation unavailable");
        std::cout << "{\"candidate\":\"" << kCandidate << "\",\"cases\":[";
        compare_mode(code, input, 0x0000u, "PC24_RN_masked"); std::cout << ',';
        compare_mode(code, input, 0x0200u, "PC53_RN_masked");
        RemoveVectoredExceptionHandler(exception_guard);
        if (control_word() != saved_cw || _mm_getcsr() != saved_csr) fail("fixture failed to restore controls");
        std::cout << "],\"controls_restored\":true}\n";
        SetErrorMode(saved_error_mode);
        return 0;
    } catch (const std::exception& error) {
        active_trace = nullptr;
        std::cerr << error.what() << '\n'; SetErrorMode(saved_error_mode); return 2;
    }
}
