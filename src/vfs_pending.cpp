#include "bsp/vfs_pending.hpp"
#include <cstring>
#include <limits>
#include <utility>

namespace bsp {
namespace {
// Reentrancy guard only: access from different threads must still be serialized
// by the caller. There is no creator-thread identity or thread-affinity rule.
thread_local bool pumping_manager{};

bool valid_string(const std::string& value) noexcept {
    return value.size() <= static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)())
        && value.find('\0') == std::string::npos;
}
bool matches_mount(const std::string& name, const std::string& prefix) noexcept {
    return prefix.empty() || (prefix.size() < name.size()
        && name[prefix.size()] == '/'
        && _strnicmp(name.c_str(), prefix.c_str(), prefix.size()) == 0);
}
void summarize(VfsPendingPumpReport& report) noexcept {
    report.completed = report.succeeded = report.failed = report.remaining_observations = 0;
    report.first_error = ERROR_SUCCESS;
    for (const auto& visit : report.visits) {
        if (!visit.invoked) continue;
        report.completed += visit.reads.completed;
        report.succeeded += visit.reads.succeeded;
        report.failed += visit.reads.failed;
        report.remaining_observations += visit.reads.remaining;
        const DWORD failure = visit.error != ERROR_SUCCESS ? visit.error
            : visit.reads.first_error != ERROR_SUCCESS ? visit.reads.first_error
            : visit.succeeded ? ERROR_SUCCESS : ERROR_GEN_FAILURE;
        if (report.first_error == ERROR_SUCCESS) report.first_error = failure;
    }
}
}

bool open_resource_pending_00bdda10_fragment(VfsMountContext& context,
    const std::string& first_name, const std::string& second_name,
    PhysicalReadCallback callback, std::uint32_t flags, DWORD& error) {
    error = ERROR_SUCCESS;
    if (!valid_string(first_name) || !valid_string(second_name) || !callback) {
        error = ERROR_INVALID_PARAMETER;
        return false;
    }
    for (const auto& mount : context.mounts) {
        if (!valid_string(mount.prefix) || !mount.submit_pending) {
            error = ERROR_NOT_SUPPORTED;
            return false;
        }
    }
    // Match native stack visitor ownership, even when a caller's input aliases
    // other state touched by provider calls. Never normalize either copy.
    const std::string first = first_name;
    const std::string second = second_name;
    context.error_code = -1;
    DWORD last_error = ERROR_SUCCESS;
    for (const auto& mount : context.mounts) {
        if (!matches_mount(second, mount.prefix)) continue;
        DWORD provider_error = ERROR_SUCCESS;
        if (mount.submit_pending(first, second, callback, flags, provider_error)) {
            error = provider_error;
            return true;
        }
        if (provider_error != ERROR_SUCCESS) last_error = provider_error;
    }
    error = last_error == ERROR_SUCCESS ? ERROR_FILE_NOT_FOUND : last_error;
    return false;
}

bool pump_pending_resources_00bdb0b0_fragment(VfsMountContext& context,
    VfsPendingPumpReport& report, DWORD& error) {
    error = ERROR_SUCCESS;
    if (pumping_manager) { error = ERROR_BUSY; return false; }
    report = {};
    for (const auto& mount : context.mounts) {
        if (!valid_string(mount.prefix) || !mount.pump_pending) {
            report.first_error = error = ERROR_NOT_SUPPORTED;
            return false;
        }
    }
    // Prepare stable report slots before any callback can run. Registry/report
    // mutation during callbacks remains outside this interface's domain.
    report.visits.reserve(context.mounts.size());
    for (std::size_t index = 0; index < context.mounts.size(); ++index) {
        VfsPendingPumpVisit visit;
        visit.mount_index = index;
        visit.prefix = context.mounts[index].prefix;
        report.visits.push_back(std::move(visit));
    }
    pumping_manager = true;
    struct Guard {
        VfsPendingPumpReport& report;
        DWORD& error;
        ~Guard() {
            summarize(report);
            error = report.first_error;
            pumping_manager = false;
        }
    } guard{report, error};
    for (std::size_t index = 0; index < context.mounts.size(); ++index) {
        auto& visit = report.visits[index];
        visit.invoked = true;
        try {
            visit.succeeded = context.mounts[index].pump_pending(visit.reads, visit.error);
        } catch (...) {
            if (visit.error == ERROR_SUCCESS) visit.error = ERROR_UNHANDLED_EXCEPTION;
            throw;
        }
    }
    summarize(report);
    return report.first_error == ERROR_SUCCESS;
}

bool begin_pending_shutdown_fragment(VfsMountContext& context, DWORD& error) {
    error = ERROR_SUCCESS;
    if (pumping_manager) { error = ERROR_BUSY; return false; }
    for (const auto& mount : context.mounts) {
        if (!mount.stop_pending_submissions) {
            error = ERROR_NOT_SUPPORTED;
            return false;
        }
    }
    DWORD first_error = ERROR_SUCCESS;
    for (const auto& mount : context.mounts) {
        DWORD provider_error = ERROR_SUCCESS;
        if (!mount.stop_pending_submissions(provider_error) && provider_error == ERROR_SUCCESS)
            provider_error = ERROR_GEN_FAILURE;
        if (first_error == ERROR_SUCCESS) first_error = provider_error;
    }
    error = first_error;
    return error == ERROR_SUCCESS;
}

bool bind_physical_pending_fragment(VfsMount& mount,
    const std::shared_ptr<PhysicalDirectory>& directory,
    const std::shared_ptr<PhysicalPendingReads>& reads) {
    if (!directory || !directory->supported() || !reads) return false;
    decltype(mount.submit_pending) submit = [directory, reads](const std::string& first,
        const std::string& second, PhysicalReadCallback callback, std::uint32_t flags, DWORD& error) {
        std::string physical_path;
        if (!directory->build_path_00bf3970(first, physical_path)) {
            error = ERROR_INVALID_NAME;
            return false;
        }
        return reads->submit_00bf43b0_fragment(physical_path, first, second,
            flags, std::move(callback), error);
    };
    decltype(mount.pump_pending) pump = [reads](PhysicalReadPumpReport& report, DWORD& error) {
        return reads->pump_00bf46b0(report, error);
    };
    decltype(mount.stop_pending_submissions) stop = [reads](DWORD& error) {
        return reads->begin_shutdown(error);
    };
    mount.submit_pending = std::move(submit);
    mount.pump_pending = std::move(pump);
    mount.stop_pending_submissions = std::move(stop);
    return true;
}
}
