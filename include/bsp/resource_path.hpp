#pragma once
#include <string>

namespace bsp {
// ECX native string, RET. Length-based ASCII lowering, including bytes after NUL.
void lowercase_resource_name_004bcc00(std::string&);
// ECX native string, RET. Lowercase, slash conversion, trim ASCII spaces.
// Substring copy preserves length but zero-pads after embedded NUL (strncpy).
// Typed adapter rejects lengths above INT32_MAX before changing the string.
bool normalize_resource_path_00bee690(std::string&);
}
