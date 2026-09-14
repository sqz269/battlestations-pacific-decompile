#include "bsp/game_native_vfs_constants.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <wincrypt.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <stdexcept>
#include <vector>

#pragma comment(lib, "advapi32.lib")

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS image constants require MSVC Win32.
#endif

namespace bsp::game {
namespace {
// Same supported image identity as GameNativeReadOnlyData. This module does
// not change the fixed-address mapper or copy any unrelated writable globals.
constexpr DWORD image_bytes = 12223752;
constexpr std::size_t key_file_offset = 0x00a144f0;
constexpr std::size_t null_file_offset = 0x00a17bf0;
constexpr std::array<BYTE,32> image_sha256{
    0xb6,0x82,0xa8,0x2c,0x52,0xf8,0x1f,0x95,0x7b,0x2c,0x70,0x22,0x20,0x77,0x30,0x5a,
    0x93,0x3f,0x72,0x48,0x16,0x86,0xc8,0x88,0x43,0x07,0x7f,0x71,0x4b,0x95,0x6d,0xd6};
static_assert(key_file_offset + GameNativeVfsConstants::mpkg_xor_key_bytes <= 0x00a18000);
static_assert(null_file_offset + 1 <= 0x00a18000);

bool matches_image(const BYTE* bytes) {
    HCRYPTPROV provider=0;
    if (!CryptAcquireContextW(&provider,nullptr,nullptr,PROV_RSA_AES,CRYPT_VERIFYCONTEXT))
        throw std::runtime_error("Cannot acquire native image SHA-256 provider");
    HCRYPTHASH hash=0;
    std::array<BYTE,32> digest{};
    DWORD size=static_cast<DWORD>(digest.size());
    const bool ok=CryptCreateHash(provider,CALG_SHA_256,0,0,&hash) &&
        CryptHashData(hash,bytes,image_bytes,0) &&
        CryptGetHashParam(hash,HP_HASHVAL,digest.data(),&size,0);
    if (hash) CryptDestroyHash(hash);
    CryptReleaseContext(provider,0);
    if (!ok) throw std::runtime_error("Cannot hash native image data");
    return size==digest.size() && digest==image_sha256;
}
} // namespace

GameNativeVfsConstants::GameNativeVfsConstants(const std::filesystem::path& path) {
    std::ifstream file(path,std::ios::binary | std::ios::ate);
    if (!file || file.tellg()!=static_cast<std::streamoff>(image_bytes))
        throw std::invalid_argument("Native VFS constants require supported original executable size");
    std::vector<BYTE> bytes(image_bytes);
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(bytes.data()),image_bytes) || !matches_image(bytes.data()))
        throw std::invalid_argument("Native VFS constants executable SHA-256 mismatch");
    std::copy_n(bytes.data()+key_file_offset,mpkg_xor_key_bytes,mpkg_xor_key_.begin());
    null_pattern_[0]=static_cast<char>(bytes[null_file_offset]);
    if (null_pattern_[0]!='\0')
        throw std::invalid_argument("Supported native null pattern is not empty");
}

} // namespace bsp::game
