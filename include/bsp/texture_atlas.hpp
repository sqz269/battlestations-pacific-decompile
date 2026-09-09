#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {
// New owning records, not the original 0x30-byte item layout. Texture handles
// are borrowed from the caller; this parser does not release or AddRef them.
struct TextureAtlasItem {
    std::string name; // Native constructor removes the final dot suffix (>0).
    std::string descriptor_path;
    void* texture{};
    std::array<float, 4> uv{}; // U1,V1,U2,V2; published only after all are read.
    std::array<std::uint16_t, 6> packed_uv{}; // U1,V1,U2,V2,V extent,U extent.
};
enum class TextureAtlasParseStatus {
    success,
    native_header_rejected,
    unsupported_malformed
};
struct TextureAtlasParseResult {
    TextureAtlasParseStatus status{TextureAtlasParseStatus::native_header_rejected};
    std::string detail;
    std::string texture_path;
    void* texture{};
    std::vector<TextureAtlasItem> items; // Earlier complete items survive failure.
};
using TextureAtlasLookup = std::function<void*(std::string_view, std::uint32_t)>;

// Semantic reconstruction of 00aeeaf0, originally ECX manager, stack text-buffer
// pointer, RET4/AL status. New interface and ownership; not ABI-compatible.
// Supplied lookup represents renderer vtable+64h; called once with flags 0.
// Filename joining is an explicit adapter for relative installed-asset paths.
TextureAtlasParseResult parse_texture_atlas_00aeeaf0(
    std::string_view text, std::string_view descriptor_path,
    const TextureAtlasLookup& lookup);
}
