#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace bsp {
enum class PackageMountStatus { mounted, declined, failed };
enum class PackageScanDisposition { already_mounted, mounted, declined, failed };

// Bind these operations to the CURRENT manager/registrations. Enumeration is
// the00bdd990 aggregation contract, including provider order and first-spelling
// deduplication. It must finish before mount mutates registrations. The lookup
// is00bdb120's equal-length/_stricmp provider SYSTEM name match, not a prefix or
// basename lookup. Mount projects00be1890, including real factory selection.
struct PackageScanCallbacks {
    std::function<bool(const std::string& directory, const std::string& extension,
        std::uint32_t flags, std::vector<std::string>& output, std::string& error)> enumerate;
    std::function<bool(const std::string& system_name)> already_mounted;
    std::function<PackageMountStatus(const std::string& system_name,
        const std::string& virtual_path, std::int32_t priority,
        std::uint8_t native_ownership_flag, std::int32_t device_id,
        std::string& error)> mount;
};

struct PackageScanEntry {
    std::string system_name; // Original enumeration spelling, without normalization.
    std::int32_t priority{};
    PackageScanDisposition disposition = PackageScanDisposition::failed;
    std::string error;
};
struct PackageScanPass {
    bool enumerated = false;
    std::vector<PackageScanEntry> entries; // Processing order, including skips/failures.
    std::string error; // Query/contract failure or the first unsuccessful entry.
};

//0073cb10: no stack arguments, incoming ECX unused, plain RET; host bool/report
// are new. Query(".", "mpkg", 0) exactly once into a private list, then process
// its fixed order. Compute whole-name priority BEFORE current system-name lookup.
// For every new name call mount(name, ".", priority, 0, -1); names stay unchanged.
// Returning declines/failures are reported and do not stop later entries. True
// means enumeration succeeded and every entry was already mounted or mounted.
// Missing callbacks/query failure perform no mounts. Unsupported priority names
// become failed entries without lookup/mount. These are explicit host guards.
// Reset report on entry; successful work is never rolled back. Exceptions
// propagate separately and can leave earlier mounts and a partial report.
bool scan_packages_0073cb10_fragment(const PackageScanCallbacks&, PackageScanPass& report);

// Startup calls0073d881/0073d888: exactly TWO fresh scans, including after a
// reported first-pass failure. The second sees current registrations; it never
// reuses the first list. Callback/allocation exceptions still propagate.
bool startup_scan_packages_0073d881_fragment(const PackageScanCallbacks&,
    std::array<PackageScanPass, 2>& reports);
}
