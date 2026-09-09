#include "bsp/resource_enumeration.hpp"
#include <limits>

namespace bsp {
ResourceEnumerationMatch resource_enumeration_match_00bee340_fragment(
    std::string_view directory, std::string_view extension, std::uint32_t flags,
    std::string_view candidate) noexcept {
    const auto valid = [](std::string_view value) noexcept {
        return value.size() <= static_cast<std::size_t>(
            (std::numeric_limits<std::int32_t>::max)()) &&
            value.find('\0') == std::string_view::npos;
    };
    if (directory.empty() || !valid(directory) || !valid(extension) || !valid(candidate))
        return ResourceEnumerationMatch::unsupported;
    if (candidate.size() < directory.size() ||
        candidate.substr(0, directory.size()) != directory)
        return ResourceEnumerationMatch::no_match;
    if ((flags & 0xffu) == 0) {
        const auto slash = candidate.rfind('/');
        //00bee37b is JA: missing '/' (FFFFFFFF) also rejects.
        if (slash == std::string_view::npos || slash > directory.size())
            return ResourceEnumerationMatch::no_match;
    }
    if (extension.size() > candidate.size() ||
        candidate.substr(candidate.size() - extension.size()) != extension)
        return ResourceEnumerationMatch::no_match;
    return ResourceEnumerationMatch::match;
}
}
