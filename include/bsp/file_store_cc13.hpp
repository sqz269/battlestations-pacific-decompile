#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Partition segment 97 (00BE4460..00C30570) reconstructions by cc13-files1. Evidence,
// original ABIs and uncertainty: docs/CC13_FILE_STORE_AND_PROFILE.md. These are new C++
// interfaces over host-owned data, not drop-in replacements for the native objects.

namespace bsp::cc13_files {

// The byte stream slots these routines use: read slot +24h (buffer, size, &read),
// write slot +28h (buffer, size, &written), tell slot +20h and seek slot +1Ch.
class ByteStream {
public:
    virtual ~ByteStream() = default;
    virtual std::size_t read(void* destination, std::size_t size) = 0;
    virtual std::size_t write(const void* source, std::size_t size) = 0;
    virtual std::int64_t tell() = 0;
    virtual void seek(std::int64_t absolute) = 0;
};

// 00BE4770, the stream slot after ReadLengthPrefixedString: append one byte at a
// time until a one-byte read returns nothing. A NUL byte does not stop it.
std::string read_stream_to_end_00be4770(ByteStream& stream);

// 00BEF8F0: read the whole stream and compact it in place, dropping a CR that is followed
// by LF or by another CR. The result keeps the input length: the compacted text is
// followed by the input's last (input - compacted) bytes, which the native never clears
// (CC13_AUDIT_2.md). No caller or data reference to 00BEF8F0 was found in the image.
std::string collapse_crlf_00bef8f0(const std::string& bytes);

// ---------------------------------------------------------------------------
// RAM image, vtable 00D68D88 (1Ch bytes: refcounted base, width +08h, height +0Ch,
// depth +10h, D3DFORMAT +14h, pixels +18h).
enum class D3DFormat : std::uint32_t {
    unknown = 0,
    r8g8b8 = 20,
    a8r8g8b8 = 21,
    x1r5g5b5 = 24,
    l8 = 50,
};


struct RamImage {
    std::uint32_t width{};   // +08h
    std::uint32_t height{};  // +0Ch
    std::uint32_t depth{};   // +10h, the third constructor argument
    D3DFormat format{};      // +14h
    std::vector<std::uint8_t> pixels;  // +18h, (bits*width*height >> 3) * depth bytes

    // 00BEFBB0
    RamImage(std::uint32_t width, std::uint32_t height, std::uint32_t depth, D3DFormat format);
    // 00BEFB40: the depth multiplier is not applied.
    std::uint32_t byte_size() const noexcept;
};

// 00BEFDC0. nullptr is never returned by the native routine (an allocation failure
// would crash it); this interface returns nullptr only for a short header read.
std::unique_ptr<RamImage> decode_tga_00befdc0(ByteStream& stream);

// 00BEFCB0 path rule: the path is upper-cased in place, and only a name containing
// ".TGA" anywhere (reverse find, not a suffix test) is loaded.
bool ram_image_path_is_tga_00befcb0(std::string& path_in_out);

// 00BF0050 body after a successful open (mode 5). The caller shows
// MessageBoxA(path, "File is write protected") when the open fails.
void encode_tga_00bf0050(const RamImage& image, ByteStream& stream);

// ---------------------------------------------------------------------------
// Chunked writer used by the terrain/landscape writers (00BF08A0, 00BF0780). The
// budget variants 00BF0340/00BF0360/00BF0390/00BF03B0 subtract the bytes the stream
// reports from a caller-owned counter.
class ChunkWriter {
public:
    explicit ChunkWriter(ByteStream& stream) noexcept : stream_(stream) {}
    // 00BF08A0: name length dword, the name characters, a zero size placeholder;
    // remembers {name, position after the placeholder}.
    void begin_chunk(const std::string& name);
    // 00BF0780: patch the placeholder with (end - start), return to the end, pop.
    // Native pops with no empty check; this throws std::logic_error when empty.
    void end_chunk();
    std::size_t depth() const noexcept { return open_.size(); }

private:
    struct Open { std::string name; std::int64_t start{}; };
    ByteStream& stream_;
    std::vector<Open> open_;
};

// ---------------------------------------------------------------------------
// Debug feature profiles (owner documented in docs/NATIVE_DEBUG_FEATURE_OWNER.md:
// feature names +04/+08, saved name +10, records +18/+1C, any-enabled +24, selected
// index +28, selected record +30). Every native method takes the 00BE8360 lock.
struct DebugFeatureProfile {
    std::string name;
    std::uint8_t flags[0x80]{};  // one byte per feature index
};

class DebugFeatureProfiles {
public:
    std::vector<std::string> features;     // +04
    std::string saved_profile_name;        // +10
    std::vector<DebugFeatureProfile> profiles;  // +18
    bool any_enabled{};                    // +24
    std::int32_t selected_index{-1};       // +28
    DebugFeatureProfile* selected{};       // +30

    // 00BE9060: case-insensitive; a new name is appended and its index returned.
    std::size_t find_or_add_feature(const std::string& name);
    // 00BE85C0. See the doc: the native scan's bound never changes, so it reads past
    // the 80h flags when every flag is zero; this version stops at the flag array.
    void refresh_any_enabled() noexcept;
    // 00BE8680
    void set_feature_enabled(std::uint32_t index, bool enabled) noexcept;
    // 00BE8620
    bool is_feature_enabled(std::uint32_t index) const noexcept;
    // 00BE8820
    bool select_profile_by_name(const std::string& name) noexcept;
    // 00BE8900 text, in the order the stream receives it.
    std::string save_text() const;
    // 00BE9100 after the existence check. Feature indexes >= 80h are ignored here;
    // the native store writes past the record (see the doc).
    void load_text(const std::string& text);
};

// 00BEEC40 / 00BEF0B0 token rule: sscanf "%d". ok is set for "0" and for any token
// whose value is 1; the value returned is (v == 1). "true"/"false" are not accepted.
bool scan_bool_token_00beec40(const std::string& token, bool& ok) noexcept;

// ---------------------------------------------------------------------------
// Error reporter (vtable 00D68E60, singleton 0109DBDC): a per-thread list of context
// strings and a message-box switch at +8.
// 00BF1B50: each context gets its first letter upper-cased (first entry) or lower-cased
// (later entries), loses one trailing '.' or '!', and is followed by " because:\n";
// then the message, or "Empty error message" when it is empty.
std::string compose_error_message_00bf1b50(const std::vector<std::string>& contexts,
    const std::string& message);

// ---------------------------------------------------------------------------
// cFileStore resident map (store +14h: key name, value stream pointer).
using FileStoreLog = std::function<void(const std::string&)>;
// 00BE5DF0: "cFileStore:" then "  <name>" for every resident entry in key order.
void dump_file_store_00be5df0(const std::map<std::string, const void*>& resident,
    const FileStoreLog& log);
// 00BE7210: erase every entry whose value is `stream`, logging
// "cFileStore::RemoveFile: <name> removed!" for each. Returns the number removed.
std::size_t remove_file_store_stream_00be7210(std::map<std::string, const void*>& resident,
    const void* stream, const FileStoreLog& log);

}  // namespace bsp::cc13_files
