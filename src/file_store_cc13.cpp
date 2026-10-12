#include "bsp/file_store_cc13.hpp"

#include "bsp/native_render_resources_surfaces.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

// Segment 97 routines. Evidence and ABIs: docs/CC13_FILE_STORE_AND_PROFILE.md.

namespace bsp::cc13_files {
namespace {

int compare_no_case(const std::string& a, const std::string& b) noexcept {
    // __stricmp over the two buffers; the native callers first compare lengths.
    for (std::size_t i = 0;; ++i) {
        const int ca = i < a.size() ? std::tolower(static_cast<unsigned char>(a[i])) : 0;
        const int cb = i < b.size() ? std::tolower(static_cast<unsigned char>(b[i])) : 0;
        if (ca != cb) return ca - cb;
        if (ca == 0) return 0;
    }
}

// The native name compares in 00BE8820 and 00BE9100: unequal lengths never match,
// two empty strings match, otherwise __stricmp.
bool same_name(const std::string& a, const std::string& b) noexcept {
    return a.size() == b.size() && (a.empty() || compare_no_case(a, b) == 0);
}

void write_u8(std::uint8_t* header, std::size_t at, std::uint8_t v) noexcept { header[at] = v; }
void write_u16(std::uint8_t* header, std::size_t at, std::uint32_t v) noexcept {
    header[at] = static_cast<std::uint8_t>(v);
    header[at + 1] = static_cast<std::uint8_t>(v >> 8);
}

void write_dword(ByteStream& stream, std::uint32_t value) {
    std::uint8_t bytes[4] = {static_cast<std::uint8_t>(value), static_cast<std::uint8_t>(value >> 8),
        static_cast<std::uint8_t>(value >> 16), static_cast<std::uint8_t>(value >> 24)};
    stream.write(bytes, sizeof bytes);
}

// Whitespace tokens. The native TextScanner (docs/LOCALE_STARTUP_INTEGRATION.md) also
// handles quotes, separators and comments; profile text never needs them.
std::vector<std::string> split_tokens(const std::string& text) {
    std::vector<std::string> tokens;
    std::size_t i = 0;
    while (i < text.size()) {
        while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i]))) ++i;
        const std::size_t start = i;
        while (i < text.size() && !std::isspace(static_cast<unsigned char>(text[i]))) ++i;
        if (i > start) tokens.emplace_back(text, start, i - start);
    }
    return tokens;
}

}  // namespace

std::string read_stream_to_end_00be4770(ByteStream& stream) {
    std::string out;
    char byte = 0;
    while (stream.read(&byte, 1) != 0) out.push_back(byte);
    return out;
}

std::string collapse_crlf_00bef8f0(const std::string& bytes) {
    // In-place compaction: a CR sets a pending flag; the next byte writes the CR back
    // first unless it is LF, and a CR after a CR replaces it. A final pending CR is not
    // written back by the loop.
    std::string out;
    out.reserve(bytes.size());
    bool pending_cr = false;
    for (const char c : bytes) {
        if (c == '\r') {
            pending_cr = true;
            continue;
        }
        if (c != '\n' && pending_cr) out.push_back('\r');
        out.push_back(c);
        pending_cr = false;
    }
    // 00BEF988: the compacted buffer is wrapped by 00BEF6D0, whose end is base + the
    // backing length +0Ch set by 008D43C0 to the full stream length and never shortened.
    // The bytes past the write cursor keep their original values, so the result is as
    // long as the input and ends in its last (input - compacted) bytes.
    out.append(bytes, out.size(), std::string::npos);
    return out;
}

RamImage::RamImage(std::uint32_t w, std::uint32_t h, std::uint32_t d, D3DFormat f)
    : width(w), height(h), depth(d), format(f),
      pixels(static_cast<std::size_t>(byte_size()) * d) {}

std::uint32_t RamImage::byte_size() const noexcept {
    return native_format_storage_bits_00b21210(static_cast<std::uint32_t>(format)) * height * width >> 3;
}

std::unique_ptr<RamImage> decode_tga_00befdc0(ByteStream& stream) {
    std::uint8_t header[18] = {};
    if (stream.read(header, sizeof header) != sizeof header) return nullptr;
    const std::uint8_t image_type = header[2];
    const std::uint32_t width = header[12] | (header[13] << 8);
    const std::uint32_t height = header[14] | (header[15] << 8);
    const std::uint8_t bpp = header[16];
    D3DFormat format = D3DFormat::unknown;
    switch (bpp) {
    case 8: if (image_type == 3 || image_type == 11) format = D3DFormat::l8; break;
    case 16: format = D3DFormat::x1r5g5b5; break;
    case 24: format = D3DFormat::r8g8b8; break;
    case 32: format = D3DFormat::a8r8g8b8; break;
    default: break;
    }
    auto image = std::make_unique<RamImage>(width, height, 1u, format);
    // Native reads ByteSize bytes straight after the header: no ID field skip, no colour
    // map, no RLE decode (type 11 is accepted but read raw) and no origin-bit test.
    const std::uint32_t total = image->byte_size();
    stream.read(image->pixels.data(), std::min<std::size_t>(total, image->pixels.size()));
    // Then swap rows i and height-1-i through a row buffer; the row is (bpp>>3)*width,
    // independent of the format. Native overruns the buffer when the format is unknown
    // (ByteSize 0); this version skips the flip when the rows do not fit.
    const std::size_t row = static_cast<std::size_t>(bpp >> 3) * width;
    if (row * height <= image->pixels.size()) {
        std::vector<std::uint8_t> scratch(row);
        for (std::uint32_t i = 0; i < height / 2; ++i) {
            std::uint8_t* top = image->pixels.data() + i * row;
            std::uint8_t* bottom = image->pixels.data() + (height - 1 - i) * row;
            std::memcpy(scratch.data(), top, row);
            std::memcpy(top, bottom, row);
            std::memcpy(bottom, scratch.data(), row);
        }
    }
    return image;
}

bool ram_image_path_is_tga_00befcb0(std::string& path) {
    for (char& c : path) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 0x20);  // 004FB1F0
    }
    return path.rfind(".TGA") != std::string::npos;
}

void encode_tga_00bf0050(const RamImage& image, ByteStream& stream) {
    std::uint8_t header[18] = {};
    // The switch stores what looks like a bit depth into the image-type byte (+2);
    // only L8 gets a valid type (3). Other formats keep the native value.
    switch (image.format) {
    case D3DFormat::r8g8b8: write_u8(header, 2, 0x18); break;
    case D3DFormat::a8r8g8b8: write_u8(header, 2, 0x20); break;
    case D3DFormat::x1r5g5b5: write_u8(header, 2, 0x10); break;
    case D3DFormat::l8: write_u8(header, 2, 3); break;
    default: break;
    }
    write_u16(header, 12, image.width);
    write_u16(header, 14, image.height);
    // bpp and the descriptor byte both receive depth*8 (8 for every loaded image).
    write_u8(header, 16, static_cast<std::uint8_t>(image.depth << 3));
    write_u8(header, 17, static_cast<std::uint8_t>(image.depth << 3));
    stream.write(header, sizeof header);
    const std::size_t row = static_cast<std::size_t>(header[16] >> 3) * image.width;
    // Rows bottom-up. Native also allocates and frees an unused row buffer.
    for (std::int32_t y = static_cast<std::int32_t>(image.height) - 1; y >= 0; --y) {
        const std::size_t at = static_cast<std::size_t>(y) * row;
        if (at + row > image.pixels.size()) continue;  // native reads past the buffer
        stream.write(image.pixels.data() + at, row);
    }
}

void ChunkWriter::begin_chunk(const std::string& name) {
    write_dword(stream_, static_cast<std::uint32_t>(name.size()));
    stream_.write(name.data(), name.size());
    write_dword(stream_, 0);
    open_.push_back({name, stream_.tell()});
}

void ChunkWriter::end_chunk() {
    if (open_.empty()) throw std::logic_error("end_chunk without begin_chunk");
    const std::int64_t start = open_.back().start;
    const std::int64_t end = stream_.tell();
    stream_.seek(start - 4);
    write_dword(stream_, static_cast<std::uint32_t>(end - start));
    stream_.seek(end);
    open_.pop_back();
}

std::size_t DebugFeatureProfiles::find_or_add_feature(const std::string& name) {
    // BSP_NativeStringVector_FindCaseInsensitive, then append at the old count.
    for (std::size_t i = 0; i < features.size(); ++i) {
        if (same_name(features[i], name)) return i;
    }
    features.push_back(name);
    return features.size() - 1;
}

void DebugFeatureProfiles::refresh_any_enabled() noexcept {
    any_enabled = false;
    if (features.empty() || selected == nullptr) return;
    // Native: for (p = selected->flags; count != 0; ++p) if (*p) { any = 1; break; }
    for (const std::uint8_t flag : selected->flags) {
        if (flag != 0) {
            any_enabled = true;
            return;
        }
    }
}

void DebugFeatureProfiles::set_feature_enabled(std::uint32_t index, bool enabled) noexcept {
    if (index >= features.size() || selected == nullptr || index >= sizeof selected->flags) return;
    selected->flags[index] = enabled ? 1 : 0;
    if (!enabled) {
        if (any_enabled) refresh_any_enabled();
    } else {
        any_enabled = true;
    }
}

bool DebugFeatureProfiles::is_feature_enabled(std::uint32_t index) const noexcept {
    if (!any_enabled || index >= features.size() || selected == nullptr || index >= sizeof selected->flags)
        return false;
    return selected->flags[index] != 0;
}

bool DebugFeatureProfiles::select_profile_by_name(const std::string& name) noexcept {
    for (std::size_t i = 0; i < profiles.size(); ++i) {
        if (!same_name(profiles[i].name, name)) continue;
        if (static_cast<std::int32_t>(i) != selected_index || selected == nullptr) {
            selected_index = static_cast<std::int32_t>(i);
            selected = &profiles[i];
            refresh_any_enabled();
        }
        return true;
    }
    return false;
}

std::string DebugFeatureProfiles::save_text() const {
    // The feature name and the value are written back to back: no separator.
    std::string text;
    for (const DebugFeatureProfile& profile : profiles) {
        text += "Profile " + profile.name + "\n";
        for (std::size_t i = 0; i < features.size(); ++i) {
            const bool on = i < sizeof profile.flags && profile.flags[i] != 0;
            text += "\tFlag " + features[i] + (on ? "true" : "false") + "\n";
        }
        text += "End\n";
    }
    return text;
}

bool scan_bool_token_00beec40(const std::string& token, bool& ok) noexcept {
    // The native out slot is uninitialised (it holds the scanner pointer) when sscanf
    // fails; any such value is neither 0 nor 1, which this models with -1.
    // strtol stands in for sscanf "%d" (leading space, sign, decimal digits).
    char* end = nullptr;
    const long parsed = std::strtol(token.c_str(), &end, 10);
    const int fields = end != token.c_str() ? 1 : 0;
    const int value = fields == 1 ? static_cast<int>(parsed) : -1;
    ok = (fields == 1 && value == 0) || value == 1;
    return value == 1;
}

void DebugFeatureProfiles::load_text(const std::string& text) {
    const std::vector<std::string> tokens = split_tokens(text);
    std::size_t at = 0;
    auto peek = [&]() -> std::string { return at < tokens.size() ? tokens[at] : std::string(); };
    auto accept = [&](const char* keyword) {
        if (at < tokens.size() && compare_no_case(tokens[at], keyword) == 0) {
            ++at;
            return true;
        }
        return false;
    };
    auto read_string = [&]() { return at < tokens.size() ? tokens[at++] : std::string(); };
    while (at < tokens.size()) {
        accept("Profile");  // 00BEEF60; the result is ignored
        DebugFeatureProfile profile;
        profile.name = read_string();
        profiles.push_back(profile);  // 00BE8FD0; the name is assigned after the append
        std::size_t record = profiles.size() - 1;
        while (accept("Flag")) {  // 00BEED60
            const std::string feature = read_string();
            bool ok = false;
            const bool value = scan_bool_token_00beec40(peek(), ok);
            if (ok) ++at;  // 00BEF0B0 consumes only a parsed token
            const std::size_t index = find_or_add_feature(feature);
            // Native stores at record + 8 + index with no bound: an index >= 80h
            // writes into the next record or past the array.
            if (index < sizeof profiles[record].flags) profiles[record].flags[index] = ok && value ? 1 : 0;
        }
        accept("End");  // result ignored
    }
    selected_index = -1;
    selected = nullptr;
    if (!profiles.empty()) {
        selected_index = 0;
        selected = &profiles[0];
        refresh_any_enabled();
    }
    if (!saved_profile_name.empty()) select_profile_by_name(saved_profile_name);
}

std::string compose_error_message_00bf1b50(const std::vector<std::string>& contexts,
    const std::string& message) {
    std::string out;
    for (std::size_t i = 0; i < contexts.size(); ++i) {
        std::string context = contexts[i];
        if (context.empty()) continue;  // native dereferences the empty buffer; see doc
        context[0] = static_cast<char>(i == 0 ? std::toupper(static_cast<unsigned char>(context[0]))
                                              : std::tolower(static_cast<unsigned char>(context[0])));
        if (context.back() == '.' || context.back() == '!') context.pop_back();
        out += context + " because:\n";
    }
    out += message.empty() ? std::string("Empty error message") : message;
    return out;
}

void dump_file_store_00be5df0(const std::map<std::string, const void*>& resident,
    const FileStoreLog& log) {
    log("cFileStore:");
    for (const auto& entry : resident) log("  " + entry.first);
}

std::size_t remove_file_store_stream_00be7210(std::map<std::string, const void*>& resident,
    const void* stream, const FileStoreLog& log) {
    std::size_t removed = 0;
    for (auto it = resident.begin(); it != resident.end();) {
        if (it->second == stream) {
            log("cFileStore::RemoveFile: " + it->first + " removed!");
            it = resident.erase(it);  // native 00BE6760 also releases the payload
            ++removed;
        } else {
            ++it;
        }
    }
    return removed;
}

}  // namespace bsp::cc13_files
