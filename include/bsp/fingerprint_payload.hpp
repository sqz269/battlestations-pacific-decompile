#pragma once
#include "bsp/memory_stream.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace bsp {
// Payload passed to Fingerprint_Text by0053c740. Its display meaning is unknown;
// the prior fallback-glyph/width-table interpretation is contradicted by this use.
// 00be9630: two stack pointers, RET8, ECX ignored. XOR 16-bit source/key words
// 16..255, compact their odd bits into 240 output bytes. Native copies another
// 272 uninitialized stack bytes; this new C++ interface exposes only defined data.
using FallbackEncodedBytes = std::array<std::uint8_t, 512>;
using FallbackDecodedPrefix = std::array<std::uint8_t, 240>;
FallbackDecodedPrefix decode_fallback_payload_00be9630(
    const FallbackEncodedBytes& source, const FallbackEncodedBytes& key) noexcept;

class FingerprintPayload {
public:
    // Successful-flow projection of ctor00be9760 after opening fonts/arial19.dat
    // with VFS mode2. Exactly512 bytes are required; another length defines only
    // byte0=0. Short reads throw rather than using the native uninitialized tail.
    void load_00be9760(MemoryStream&);
    bool decoded() const noexcept { return decoded_; }
    std::size_t defined_size() const noexcept { return prefix_.size(); }
    std::optional<std::uint8_t> byte(std::size_t index) const noexcept;
private:
    std::vector<std::uint8_t> prefix_;
    bool decoded_{};
};
}
