#pragma once
#include <cstdint>
#include <string_view>

namespace bsp {
enum class ResourceEnumerationMatch { unsupported, no_match, match };

//00bee340: ECX directory, EDX extension, stack flag/candidate, AL result, RET8.
// Case-sensitive byte prefix/suffix. Low flag byte0 additionally requires the
// unsigned last '/' index <= directory length; absent slash is UINT32_MAX.
// No added slash/dot, normalization, case folding or basename extraction.
// Host domain: nonempty directory, all lengths <=INT32_MAX, no embedded NUL.
// Empty extension/candidate are supported; an empty candidate cannot match a
// nonempty directory. Unsupported is distinct from an ordinary native no-match.
ResourceEnumerationMatch resource_enumeration_match_00bee340_fragment(
    std::string_view directory, std::string_view extension, std::uint32_t flags,
    std::string_view candidate) noexcept;
}
