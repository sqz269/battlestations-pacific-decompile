#pragma once
// Localisation ("lockit") table loading and lookup.
// Addresses: 008d4870, 008d4890, 00aa09d0, 00aa06d0, 00aa0020, 00aa0d30,
//            00a9fc30, 00a9ec70, 0073c240, 008d7bc0, 0073bae0.
// Every name below is a hypothesis, not a recovered symbol. See
// docs/APP_INIT_LOCALE.md for the evidence behind each claim.
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace bsp {

// One row of the runtime language table the native build keeps behind the
// pointer global 00f88974, stride 0x20 (008d4870 shifts the index left by 5).
// Field offsets are the four {size, pointer} native string pairs at +0, +8,
// +0x10 and +0x18; 008d7bc0 fills them from the four keys of a lockit/*.lng
// descriptor. Empty means the descriptor omitted the key.
struct LanguageEntry {
    std::string lanfile;    // entry +0x00/+0x04, returned by 008d4870
    std::string lockit_id;  // entry +0x08/+0x0c
    std::string voice_dir;  // entry +0x10/+0x14
    std::string font_path;  // entry +0x18/+0x1c, returned by 008d4890
};

// Whitespace-separated "<key> <value>" descriptor read by 008d7bc0. Only the
// four recognised keys are stored; every other token pair is ignored, matching
// the native chain of case-insensitive comparisons that falls through. Never
// fails: a truncated trailing key simply contributes nothing.
LanguageEntry parse_language_descriptor_008d7bc0(const std::string& text);

// 008d4870: __thiscall, ECX = the settings object, no stack arguments, RET.
// Reads the language index from settings +4, indexes the table with stride
// 0x20 and returns entry +4. The native fallback for a null pointer is the
// empty string at 00f88a3c, so an out-of-range index yields "" here.
const std::string& language_name_008d4870(const std::vector<LanguageEntry>& table,
    std::size_t language_index) noexcept;

// 008d4890: same shape as 008d4870 for entry +0x18/+0x1c. Returns "" when the
// descriptor had no fontpath. 0073bae0 feeds this into the font descriptor
// load, which is the only locale input to the font system.
const std::string& language_font_path_008d4890(const std::vector<LanguageEntry>& table,
    std::size_t language_index) noexcept;

// One parsed record of a lockit .lan table.
struct LocaleRecord {
    std::string key;      // "<category>.<name>", composed with '.' (00ce3a70)
    std::u16string text;  // UTF-16LE code units, no terminator
};

// Native stack buffers in 00aa0020. The parser writes without bounds checks,
// so records over these limits corrupt its frame; the port rejects them.
inline constexpr std::size_t kLockitKeyBufferBytes = 152;      // acStack_4f5c
inline constexpr std::size_t kLockitValueBufferBytes = 19994;  // auStack_4e2a

// Record scanner of 00aa0020, phase one. Grammar recovered from the three
// state values of local_4f80 and confirmed against the shipped table:
//   record := category ' ' name ' ' utf16le-text 00 0A 00 0D
// The two tokens are raw bytes up to the first space; the text is 16-bit
// units terminated by the unit pair 0x0A00, 0x0D00 (bytes 00 0A 00 0D).
// Trailing bytes that do not form a complete record are reported as an error
// and leave `output` holding the records parsed so far, because the native
// loop only stops on the stream position check.
bool parse_lockit_table_00aa0020(const std::vector<std::uint8_t>& bytes,
    std::vector<LocaleRecord>& output, std::string& error);

// Record scanner of 00aa0020, phase two: the ".lanx" sidecar, a flat array of
// 16-bit pairs appended to the two vectors at manager +0x4020 and +0x4030.
// A trailing partial pair is an error. The shipped sidecar is zero bytes.
bool parse_lockit_index_00aa0020(const std::vector<std::uint8_t>& bytes,
    std::vector<std::uint16_t>& first, std::vector<std::uint16_t>& second,
    std::string& error);

// "lockit/" (00d5bcd0) + language + ".lan" (00d5bcc8), built by both loaders.
std::string lockit_table_path_00aa09d0(const std::string& language);

// The chained hash map at manager +0x14: 0x1000 bucket heads, the live count
// at +0x4000 and 0x1c-byte nodes whose links are +0x10 (prev) and +0x14 (next).
// Insertion (00a9fc30) pushes at the head and returns an existing node when the
// key already matches, so a repeated key overwrites the earlier text.
class LocaleTables {
public:
    // Injected file access. Returns false when the name does not resolve; the
    // native side asks BSP_VFS_ResolveExistingName (00bdf4c0) first and then
    // opens through the mount manager, which merges every mount that holds the
    // name. Supply the repo's VFS or physical-file bindings here; this module
    // deliberately owns no file code of its own.
    using FileReader = std::function<bool(const std::string& name,
        std::vector<std::uint8_t>& bytes)>;

    static constexpr std::size_t kBucketCount = 0x1000;
    static constexpr std::size_t kMaxNumberedSuffix = 99;  // native loop i < 99

    // 00a9fc30 / 00a9ec70 prologue: h = h * 0x83 + (signed char)lowercase(c),
    // then masked with 0xfff. Only 'A'..'Z' are folded.
    static std::uint32_t bucket_00a9fc30(const std::string& key) noexcept;

    // 00aa0d30: `ADD ECX,4; JMP 00450540`, a push_back onto the vector of
    // native strings at manager +8. Duplicates are not filtered. The two
    // loaders read only whether the vector is empty.
    void register_table_00aa0d30(std::string table_name);
    const std::vector<std::string>& registered_tables() const noexcept { return tables_; }

    // 0073c240: frees every chain and zeroes the 0x4000-byte bucket array and
    // the count. Leaves the language name and the sidecar vectors alone.
    void clear_0073c240() noexcept;

    // 00aa09d0: __thiscall, ECX = manager, one stack argument, RET 4.
    // Case-insensitive compare against the stored name (00435c40); an equal
    // name is a no-op and reports `changed = false`. Otherwise the name is
    // stored, the map cleared, and - only when at least one table name has
    // been registered and the map is empty - the .lan files are loaded.
    // The native tail refreshes the GUI (004c12b0, 00aa4650); that is not
    // reproduced here.
    bool set_language_00aa09d0(const FileReader& reader, const std::string& language,
        bool& changed, std::string& error);

    // 00aa06d0: __thiscall, ECX = manager, one char stack argument, RET 4.
    // Loads when the map is empty or `force` is set, using the language name
    // already stored. No GUI refresh and no registered-table gate.
    bool reload_00aa06d0(const FileReader& reader, bool force, std::string& error);

    // 00a9ec70: hash, then walk the chain comparing length and _stricmp.
    // Returns nullptr on a miss. The pointer is invalidated by any mutation.
    const std::u16string* find_00a9ec70(const std::string& key) const noexcept;

    // 00a9fad0 splits a lookup key on '|' and resolves each piece; this is that
    // split alone, without the '^'/'.' prefix handling and the '#' substitution
    // pass of 00a9f4b0, which this packet did not reconstruct.
    static std::vector<std::string> split_lookup_key_00a9fad0(const std::string& key);

    const std::string& language() const noexcept { return language_; }
    std::size_t size() const noexcept { return count_; }
    // Files actually read by the last load, in native probe order.
    const std::vector<std::string>& loaded_files() const noexcept { return loaded_files_; }
    const std::vector<std::uint16_t>& sidecar_first() const noexcept { return sidecar_first_; }
    const std::vector<std::uint16_t>& sidecar_second() const noexcept { return sidecar_second_; }

private:
    struct Node {
        std::string key;
        std::u16string text;
        Node* next = nullptr;
    };
    bool load_current_language(const FileReader& reader, std::string& error);
    bool load_one_file(const FileReader& reader, const std::string& name, std::string& error);
    void insert_00a9fc30(std::string key, std::u16string text);

    std::vector<std::unique_ptr<Node>> storage_;
    Node* buckets_[kBucketCount] = {};
    std::size_t count_ = 0;                 // manager +0x4014
    std::vector<std::string> tables_;       // manager +8
    std::string language_;                  // manager +0x4018
    std::vector<std::uint16_t> sidecar_first_;   // manager +0x4020
    std::vector<std::uint16_t> sidecar_second_;  // manager +0x4030
    std::vector<std::string> loaded_files_;
};

}  // namespace bsp
