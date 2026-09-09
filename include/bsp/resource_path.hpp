#pragma once
#include <string>

namespace bsp {
// ECX native string, RET. Length-based ASCII lowering, including bytes after NUL.
void lowercase_resource_name_004bcc00(std::string&);
// ECX native string, RET. Lowercase, slash conversion, trim ASCII spaces.
// Substring copy preserves length but zero-pads after embedded NUL (strncpy).
// Typed adapter rejects lengths above INT32_MAX before changing the string.
bool normalize_resource_path_00bee690(std::string&);
// ECX output, EDX input, RET. Lexical ASCII slash/dot canonicalization, keeping
// trailing separators. This is00bee390 only; mount trimming is separate.
bool canonicalize_resource_path_00bee390_fragment(const std::string&, std::string& output);
// ECX output, EDX directory, stack child, RET4. Join nonempty operands with '/',
// then00bee390; no filesystem lookup or trailing-slash trim. Alias-safe.
bool join_resource_path_00bee520_fragment(const std::string& directory,
    const std::string& child, std::string& output);
// Name fragment of00b2ebb0 using004cad40: replace every case-sensitive .mshd
// substring with .shfx, including occurrences inside path components. Caller
// supplies renderer-lowercased name; this is not an extension-only fallback.
// Host rejects embedded NUL and lengths above INT32_MAX, preserving output.
bool shader_descriptor_name_00b2ebb0_fragment(const std::string&, std::string& output);
}
