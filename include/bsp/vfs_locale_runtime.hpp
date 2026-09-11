#pragma once
#include "bsp/language_catalog.hpp"
#include "bsp/resource_lookup.hpp"

namespace bsp {
// Binds recovered locale/catalog consumers to the application's mounted VFS.
// Mounts, registrations and ordered content suffixes remain live shared inputs.
// The hint-count query is required and evaluated once for each opened descriptor.
class VfsLocaleRuntime final : public LocaleTableSource, public LanguageCatalogSource {
public:
    VfsLocaleRuntime(VfsMountContext&, const VfsCandidateRegistrations&,
        const std::vector<std::string>& suffixes, std::function<std::uint32_t()> hint_count);
    std::vector<std::string> override_paths_00bdef90(const std::string&) override;
    bool read_file(const std::string&, std::uint32_t mode,
        std::vector<std::uint8_t>&) override;
    bool resolves_existing_name_00bdf4c0(const std::string&) override;
    std::vector<std::string> enumerate_descriptors_00886280(
        const std::string&, const std::string&, std::uint32_t) override;
    bool read_descriptor_00bef2e0(const std::string&, std::uint32_t,
        std::vector<std::uint8_t>&) override;
    std::uint32_t profile_hints_count_08() override;
private:
    bool read_bytes(const std::string&, std::uint32_t, std::vector<std::uint8_t>&);
    VfsMountContext& vfs_;
    const VfsCandidateRegistrations& registrations_;
    const std::vector<std::string>& suffixes_;
    std::function<std::uint32_t()> hint_count_;
};
}
