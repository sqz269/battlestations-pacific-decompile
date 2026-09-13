#include "bsp/game_native_readonly_data.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <wincrypt.h>
#include <array>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

#pragma comment(lib, "advapi32.lib")

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Original numeric data mapping requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
// Exact supported PE: x86, preferred base400000, .rdata RVA8E2000,
// virtual size125B24, raw offset8E2000, raw size126000, flags40000040.
// Verify the complete file before relying on any of these fixed offsets.
constexpr DWORD image_bytes = 12223752;
constexpr std::size_t data_file_offset = 0x008e2000;
constexpr std::array<BYTE,32> image_sha256{
    0xb6,0x82,0xa8,0x2c,0x52,0xf8,0x1f,0x95,0x7b,0x2c,0x70,0x22,0x20,0x77,0x30,0x5a,
    0x93,0x3f,0x72,0x48,0x16,0x86,0xc8,0x88,0x43,0x07,0x7f,0x71,0x4b,0x95,0x6d,0xd6};
bool matches_image(const BYTE* bytes) {
    HCRYPTPROV provider = 0;
    if (!CryptAcquireContextW(&provider,nullptr,nullptr,PROV_RSA_AES,CRYPT_VERIFYCONTEXT))
        throw std::runtime_error("Cannot acquire native image SHA-256 provider");
    HCRYPTHASH hash = 0;
    std::array<BYTE,32> digest{};
    DWORD size = static_cast<DWORD>(digest.size());
    const bool ok = CryptCreateHash(provider,CALG_SHA_256,0,0,&hash) &&
        CryptHashData(hash,bytes,image_bytes,0) &&
        CryptGetHashParam(hash,HP_HASHVAL,digest.data(),&size,0);
    if (hash) CryptDestroyHash(hash);
    CryptReleaseContext(provider,0);
    if (!ok) throw std::runtime_error("Cannot hash native image data");
    return size == digest.size() && digest == image_sha256;
}
} // namespace

GameNativeReadOnlyData::GameNativeReadOnlyData(const std::filesystem::path& path) {
    // Reserve before allocating the file buffer: a large ordinary heap request
    // could otherwise occupy the very address range this service must retain.
    SYSTEM_INFO system{};
    GetSystemInfo(&system);
    const auto granularity = static_cast<std::uintptr_t>(system.dwAllocationGranularity);
    const auto page_size = static_cast<std::uintptr_t>(system.dwPageSize);
    const auto reservation_begin = begin_address / granularity * granularity;
    const auto commit_end = (begin_address + byte_count + page_size - 1) / page_size * page_size;
    void* const requested = reinterpret_cast<void*>(reservation_begin);
    reservation_ = VirtualAlloc(requested,commit_end-reservation_begin,MEM_RESERVE,PAGE_NOACCESS);
    if (reservation_ != requested) {
        if (reservation_) VirtualFree(reservation_,0,MEM_RELEASE);
        reservation_ = nullptr;
        throw std::runtime_error("Original read-only data addresses are unavailable in this process");
    }
    try {
        std::ifstream file(path,std::ios::binary | std::ios::ate);
        if (!file || file.tellg() != static_cast<std::streamoff>(image_bytes))
            throw std::invalid_argument("Native data requires the supported original executable size");
        std::vector<BYTE> bytes(image_bytes);
        file.seekg(0);
        if (!file.read(reinterpret_cast<char*>(bytes.data()),image_bytes) || !matches_image(bytes.data()))
            throw std::invalid_argument("Native data executable SHA-256 does not match the supported image");

        void* const data = reinterpret_cast<void*>(begin_address);
        if (VirtualAlloc(data,commit_end-begin_address,MEM_COMMIT,PAGE_READWRITE) != data)
            throw std::runtime_error("Cannot commit original read-only data pages");
        std::memcpy(data,bytes.data()+data_file_offset,byte_count);
        DWORD previous = 0;
        if (!VirtualProtect(data,commit_end-begin_address,PAGE_READONLY,&previous))
            throw std::runtime_error("Cannot protect original read-only data pages");
    } catch (...) {
        VirtualFree(reservation_,0,MEM_RELEASE);
        reservation_ = nullptr;
        throw;
    }
}

GameNativeReadOnlyData::~GameNativeReadOnlyData() noexcept {
    if (reservation_) VirtualFree(reservation_,0,MEM_RELEASE);
}

const void* GameNativeReadOnlyData::data_at(std::uintptr_t address,std::size_t bytes) const {
    if (address < begin_address || address - begin_address > byte_count ||
        bytes > byte_count - (address - begin_address))
        throw std::out_of_range("Requested native data span is outside the verified read-only section");
    return reinterpret_cast<const void*>(address);
}
} // namespace bsp::game
