#pragma once
#include "bsp/memory_stream.hpp"
#include <map>
#include <string>
#include <vector>

namespace bsp {
enum class FileStoreInsertResult { inserted, duplicate, invalid };

// Named in-memory provider, not a disk archive. Native table00d689e8,
// primary name/stream tree+14. Secondary tree and batch APIs are not modeled.
class FileStore {
public:
    // Normalize a copy via00bee780/00bee690, retain the same supplied wrapper,
    // first duplicate wins. Host requires positive backing and no embedded NUL.
    FileStoreInsertResult add_file_00be7760(const std::string& name,
        const std::shared_ptr<MemoryStream>& source);
    // Lookup does not normalize input; CRT case-insensitive tree equivalence.
    bool exists_00be5c00(const std::string& name) const;
    // flags bit0 rejects; a successful memory-backed open clones cursor0 while
    // sharing backing, matching00bef750's memory-stream branch. No tail repair.
    std::shared_ptr<MemoryStream> open_00be5fa0(const std::string& name,
        std::uint32_t flags) const;
    // Provider+24: success copies the requested logical name, failure leaves
    // output unchanged. No physical filename, directory prefix or extension.
    bool resolve_00bf0fb0(const std::string& name, std::string& output) const;
    // ECX provider; directory/extension/flags/output stack, RET10h. Primary
    // tree order, case-sensitive first substring tests, flags unused. Includes
    // the native absent-extension/unsigned length+1 sentinel collision. Host
    // strings have nonnull data even when empty; guarded failure keeps output.
    bool enumerate_00be6480(const std::string& directory, const std::string& extension,
        std::uint32_t flags, std::vector<std::string>& output, std::string& error) const;
    std::size_t size() const noexcept { return files_.size(); }
private:
    struct NameLess {
        bool operator()(const std::string&, const std::string&) const noexcept;
    };
    std::map<std::string, std::shared_ptr<MemoryStream>, NameLess> files_;
};
}
