#pragma once

#include "bsp/vfs_mounts.hpp"
#include <array>

namespace bsp {
// Source interface for consumers still expressed in terms of VfsMountContext.
// Implementations use one retained actual manager and its native services;
// the context does not copy mounts, owners, registrations or numeric tables.
class NativeVfsAccess {
public:
    virtual ~NativeVfsAccess() = default;
    virtual bool exists(const std::string&) = 0;
    virtual bool resolve_existing(std::string&) = 0;
    virtual bool direct_resolve(const std::string&, std::string&) = 0;
    virtual VfsMemoryOpen open(const std::string&, std::uint32_t flags) = 0;
    virtual std::vector<std::string> enumerate(const std::string& directory,
        const std::string& extension, std::uint32_t flags) = 0;
    virtual std::array<std::uint32_t,5> file_date(const std::string&) = 0;
};
} // namespace bsp
