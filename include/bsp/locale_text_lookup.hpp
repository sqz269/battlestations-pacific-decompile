#pragma once

#include "bsp/locale_tables.hpp"

#include <string>

namespace bsp {

// Required live dependencies of 00a9f4b0/00a9eba0. The native resolver obtains
// its current Lua owner through global 00e1ae90, virtual +0x14, then reads the
// owner's globals. Neither an absent context nor a conversion failure is a
// successful empty replacement. Implementations must preserve 00b692c0's
// path/value conversion contract; see docs/LOCALE_TEXT_LOOKUP.md.
class LocaleTextRuntimeHost {
public:
    virtual ~LocaleTextRuntimeHost() = default;
    virtual std::string context_string_00b692c0(const std::string& path) = 0;
    virtual char16_t crt_uppercase_00c0391c(char16_t code_unit) = 0;
};

// 00a9eba0: first sidecar mapping wins, otherwise use the native CRT's
// towupper behavior. A missing paired sidecar entry throws here; native calls
// its vector range-error path. New C++ interface, not the original ABI.
char16_t locale_uppercase_00a9eba0(const LocaleTables& tables,
    LocaleTextRuntimeHost& host, char16_t code_unit);

// Native 00a9fad0 / 00a9f4b0 are thiscall(manager, WideString* output,
// const NativeString* key), RET 8. They APPEND to the existing output.
// Tables and host must outlive the resolver and remain valid during callbacks.
// This projection accepts null-free strings of at most INT32_MAX code units.
// Unmatched '#' and narrowing that would terminate partway through a wide
// token throw instead of reproducing native unbounded expansion/unread bytes.
class LocaleTextResolver {
public:
    LocaleTextResolver(const LocaleTables& tables, LocaleTextRuntimeHost& host)
        : tables_(tables), host_(host) {}

    void append_key_list_00a9fad0(std::u16string& output,
        const std::string& keys) const;
    void append_key_00a9f4b0(std::u16string& output,
        const std::string& key) const;
    std::u16string resolve(const std::string& keys) const;

private:
    const LocaleTables& tables_;
    LocaleTextRuntimeHost& host_;
};

} // namespace bsp
