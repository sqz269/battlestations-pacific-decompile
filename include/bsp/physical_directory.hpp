#pragma once
#include <memory>
#include <string>
#include <cstdint>
#include <vector>

namespace bsp {
// Physical-provider projection with empty indexed-name tree (+34=0).
// Caller supplies root exactly, including any
// desired trailing separator; no implicit canonicalization or mount semantics.
class PhysicalDirectory {
public:
    explicit PhysicalDirectory(std::string root, bool accept_nonempty_names = false);
    bool supported() const noexcept { return supported_; }
    bool accepts_nonempty_names() const noexcept { return accept_nonempty_names_; }
    // ECX provider, stack out/suffix, RET8; root+suffix, then slash conversion
    // in suffix only. Host rejects embedded NUL and >INT32_MAX string length.
    bool build_path_00bf3970(const std::string& suffix, std::string& output) const;
    // ECX provider, stack suffix, RET4. Includes native last-success cache;
    // a matching name can remain true after the file disappears. Directories
    // count as existing. No OS query for empty name or matching cached name.
    // Constructor flag+28 also accepts every nonempty name without an OS query.
    bool exists_00bf3f70_fragment(const std::string& suffix);
    // Provider virtual+24, ECX provider, stack suffix/out, RET8. On success
    // copies the original logical suffix, NOT the constructed physical path.
    // Failure preserves output. Source/output aliasing is supported.
    bool resolve_00bf0fb0(const std::string& suffix, std::string& output);
    // ECX provider; directory/extension/flags/output stack, RET10h. Win32 find
    // order with depth-first recursion when the low flag byte is nonzero;
    // suffix matching is case-insensitive, joined output is00bee520 canonical.
    // Native FindFirst/FindNext failures end that scan without a status. Host
    // guards reject unrepresentable/empty physical paths and keep output.
    bool enumerate_00bf47e0_fragment(const std::string& directory,
        const std::string& extension, std::uint32_t flags,
        std::vector<std::string>& output, std::string& error) const;
private:
    std::string root_;
    std::string last_success_;
    bool supported_{};
    bool accept_nonempty_names_{};
};
// Factory00bf4df0, ECX singleton, stack system/virtual paths, RET8. Only a
// nonempty system path ending '\\' is accepted; virtual 'persistent_data'
// (CRT case-insensitive) selects constructor flag+28=true. No existence probe.
// Host shared ownership replaces native pooling. Allocation exceptions propagate.
std::shared_ptr<PhysicalDirectory> create_physical_directory_00bf4df0_fragment(
    const std::string& system_path, const std::string& virtual_path);
}
