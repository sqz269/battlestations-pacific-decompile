#pragma once
#include "bsp/locale_tables.hpp"

namespace bsp {
struct LanguageCatalogSource {
    virtual ~LanguageCatalogSource() = default;
    virtual std::vector<std::string> enumerate_descriptors_00886280(
        const std::string& directory, const std::string& extension, std::uint32_t flags) = 0;
    virtual bool read_descriptor_00bef2e0(const std::string& name, std::uint32_t mode,
        std::vector<std::uint8_t>& bytes) = 0;
    virtual std::uint32_t profile_hints_count_08() = 0;
};
// Native008d7bc0 has no arguments and returns RET; this explicit table replaces
// global00f88974/count00f88978/capacity00f8897c. Only a nonempty table suppresses
// another call. Enumerated descriptors are opened before the hint-count filter.
// Normal-flow VFS/token/value projection; no native allocator/object ABI claim.
void build_language_catalog_008d7bc0(std::vector<LanguageEntry>&, LanguageCatalogSource&);
// The five calls00553c80 and inlined sixth comparison are case-sensitive
// prefix tests, not filename equality. Nonzero hint count bypasses all six.
bool language_descriptor_allowed_008d7bc0(const std::string&, std::uint32_t hint_count);
LanguageEntry parse_native_language_descriptor_008d7bc0(const std::vector<std::uint8_t>&);
}
