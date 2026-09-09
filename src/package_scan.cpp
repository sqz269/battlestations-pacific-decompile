#include "bsp/package_scan.hpp"
#include "bsp/package_mounts.hpp"
#include <utility>

namespace bsp {
bool scan_packages_0073cb10_fragment(const PackageScanCallbacks& callbacks,
    PackageScanPass& report) {
    report = {};
    if (!callbacks.enumerate || !callbacks.already_mounted || !callbacks.mount) {
        report.error = "Package scanning requires current enumeration, system-name lookup and mount callbacks.";
        return false;
    }
    const std::string directory = ".";
    const std::string extension = "mpkg";
    std::vector<std::string> names;
    if (!callbacks.enumerate(directory, extension, 0, names, report.error)) {
        if (report.error.empty()) report.error = "Package enumeration failed.";
        return false;
    }
    report.enumerated = true;
    report.error.clear();
    report.entries.reserve(names.size());
    bool success = true;
    for (auto& name : names) {
        report.entries.emplace_back();
        auto& entry = report.entries.back();
        entry.system_name = std::move(name);
        // Native0073cc75..0073cd0e precedes the00bdb120 call at0073cd19.
        if (!package_mount_priority_0073cb10_fragment(entry.system_name, entry.priority)) {
            entry.error = "Package priority requires an ASCII name without embedded NUL, at most INT32_MAX bytes.";
        } else if (callbacks.already_mounted(entry.system_name)) {
            entry.disposition = PackageScanDisposition::already_mounted;
        } else {
            switch (callbacks.mount(entry.system_name, directory, entry.priority, 0, -1, entry.error)) {
            case PackageMountStatus::mounted:
                entry.disposition = PackageScanDisposition::mounted;
                entry.error.clear();
                break;
            case PackageMountStatus::declined:
                entry.disposition = PackageScanDisposition::declined;
                if (entry.error.empty()) entry.error = "No provider factory accepted the package logical name.";
                break;
            case PackageMountStatus::failed:
                if (entry.error.empty()) entry.error = "Package construction or registration failed.";
                break;
            default:
                if (entry.error.empty()) entry.error = "Package mount callback returned an unsupported status.";
                break;
            }
        }
        if (entry.disposition == PackageScanDisposition::declined ||
            entry.disposition == PackageScanDisposition::failed) {
            success = false;
            if (report.error.empty()) report.error = entry.error;
        }
        //0073cd73 ignores the mount's returned provider and proceeds to cleanup
        // and the next front entry. Never re-enumerate inside this loop.
    }
    return success;
}

bool startup_scan_packages_0073d881_fragment(const PackageScanCallbacks& callbacks,
    std::array<PackageScanPass, 2>& reports) {
    reports = {};
    const bool first = scan_packages_0073cb10_fragment(callbacks, reports[0]);
    const bool second = scan_packages_0073cb10_fragment(callbacks, reports[1]);
    return first && second;
}
}
