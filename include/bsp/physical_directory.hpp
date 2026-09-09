#pragma once
#include <string>

namespace bsp {
// Physical-provider projection with native constructor flag+28=false and
// empty indexed-name tree (+34=0). Caller supplies root exactly, including any
// desired trailing separator; no implicit canonicalization or mount semantics.
class PhysicalDirectory {
public:
    explicit PhysicalDirectory(std::string root);
    bool supported() const noexcept { return supported_; }
    // ECX provider, stack out/suffix, RET8; root+suffix, then slash conversion
    // in suffix only. Host rejects embedded NUL and >INT32_MAX string length.
    bool build_path_00bf3970(const std::string& suffix, std::string& output) const;
    // ECX provider, stack suffix, RET4. Includes native last-success cache;
    // a matching name can remain true after the file disappears. Directories
    // count as existing. No OS query for empty name or matching cached name.
    bool exists_00bf3f70_fragment(const std::string& suffix);
    // Provider virtual+24, ECX provider, stack suffix/out, RET8. On success
    // copies the original logical suffix, NOT the constructed physical path.
    // Failure preserves output. Source/output aliasing is supported.
    bool resolve_00bf0fb0(const std::string& suffix, std::string& output);
private:
    std::string root_;
    std::string last_success_;
    bool supported_{};
};
}
