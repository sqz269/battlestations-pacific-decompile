#include "bsp/vfs_candidates.hpp"
#include "bsp/resource_path.hpp"
#include <algorithm>
#include <cstdint>
#include <utility>

namespace bsp {
namespace {
bool supported(const std::string& value) {
    return value.size() <= INT32_MAX && std::all_of(value.begin(), value.end(),
        [](unsigned char c) { return c != 0 && c < 128; });
}
char lower(char c) { return c >= 'A' && c <= 'Z' ? static_cast<char>(c + 32) : c; }
bool equal(const std::string& a, const std::string& b) {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(),
        [](char x, char y) { return lower(x) == lower(y); });
}
}
bool resolve_resource_candidates_00bddc80(
    std::string& name, const VfsCandidateRegistrations& registrations,
    const VfsDirectResolver& direct, const VfsCandidateExists& exists) {
    if (!direct || !exists || !supported(name)) return false;
    for (const auto& entry : registrations.extension_prefixes)
        if (!supported(entry.extension) || !supported(entry.prefix)) return false;
    for (const auto& group : registrations.groups) {
        for (const auto& extension : group.extensions) if (!supported(extension)) return false;
        for (const auto& prefix : group.prefixes) if (!supported(prefix)) return false;
    }
    lowercase_resource_name_004bcc00(name);
    for (auto& c : name) if (c == '\\') c = '/';
    std::string resolved;
    if (direct(name, resolved)) { name = std::move(resolved); return true; }
    const auto slash = name.find_last_of('/');
    const auto dot = name.find('.', slash == std::string::npos ? 0 : slash);
    if (dot == std::string::npos || dot == 0) return false;
    const auto full = name.substr(0, dot);
    const auto base = name.substr(slash == std::string::npos ? 0 : slash + 1,
        dot - (slash == std::string::npos ? 0 : slash + 1));
    const auto original = name.substr(dot + 1);
    const bool different = !equal(full, base);
    const auto candidate = [&](const std::string& prefix, const std::string& stem,
                               const std::string& extension) {
        const auto length = static_cast<std::uint64_t>(prefix.size()) + stem.size() + 1 + extension.size();
        if (length > INT32_MAX) return false;
        const std::string path = prefix + stem + "." + extension;
        if (!exists(path)) return false;
        name = path;
        return true;
    };
    const auto roots = [&](const std::string& stem, const std::string& extension) {
        for (const auto& entry : registrations.extension_prefixes)
            if (equal(entry.extension, extension) && candidate(entry.prefix, stem, extension)) return true;
        return false;
    };
    if (roots(full, original)) return true;
    const VfsCandidateGroup* selected = nullptr;
    for (const auto& group : registrations.groups) {
        if (std::any_of(group.extensions.begin(), group.extensions.end(),
            [&](const std::string& extension) { return equal(extension, original); })) {
            selected = &group;
            break;
        }
    }
    const auto paths = [&](const std::string& stem, const std::string& extension) {
        if (selected)
            for (const auto& prefix : selected->prefixes)
                if (candidate(prefix, stem, extension)) return true;
        return false;
    };
    if (different && paths(full, original)) return true;
    if (different && roots(base, original)) return true;
    if (!selected) return false;
    if (paths(base, original)) return true;
    if (different) {
        for (const auto& extension : selected->extensions)
            if (!equal(extension, original) && roots(full, extension)) return true;
        for (const auto& extension : selected->extensions)
            if (!equal(extension, original) &&
                (candidate("", full, extension) || paths(full, extension))) return true;
    }
    for (const auto& extension : selected->extensions)
        if (!equal(extension, original) && roots(base, extension)) return true;
    for (const auto& extension : selected->extensions)
        if (!equal(extension, original) && paths(base, extension)) return true;
    return false;
}
}
