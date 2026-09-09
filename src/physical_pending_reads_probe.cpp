// One real-file submission/pump/lifetime scenario, not a native differential test.
#include "bsp/physical_pending_reads.hpp"
#include "bsp/file_store_requests.hpp"
#include "bsp/vfs_provider_manager.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

namespace {
[[noreturn]] void pending_probe_exit(const char* reason, std::size_t pending) {
    std::fprintf(stderr, "Physical pending read probe: %s; pending=%zu; exiting process without destroying active I/O owners.\n",
        reason, pending);
    std::fflush(nullptr);
    std::_Exit(EXIT_FAILURE);
}

bool mounted_pending_fixture(const std::string& physical_path, const std::string& expected) {
    std::unique_ptr<bsp::VfsProviderManager> manager;
    bool may_have_pending = false;
    try {
        // Validate the supplied leaf hierarchy before deriving the game root.
        // No guessed drive/install path and no directory creation are involved.
        const auto file = std::filesystem::absolute(physical_path).lexically_normal();
        const auto tables = file.parent_path();
        const auto scripts = tables.parent_path();
        if (_stricmp(file.filename().string().c_str(), "inputs.lua") != 0
            || _stricmp(tables.filename().string().c_str(), "datatables") != 0
            || _stricmp(scripts.filename().string().c_str(), "scripts") != 0) return false;
        auto root = scripts.parent_path().string();
        if (root.empty()) return false;
        if (root.back() != '\\' && root.back() != '/') root += '\\';
        auto factories = std::make_shared<bsp::VfsProviderFactories>();
        manager = std::make_unique<bsp::VfsProviderManager>(factories);
        std::shared_ptr<bsp::VfsProviderIdentity> provider;
        std::string diagnostic;
        if (manager->mount_system_path_00be1890_fragment(root, ".", 0, 1, -1,
                provider, diagnostic) != bsp::VfsProviderCreateStatus::created
            || manager->mount_system_path_00be1890_fragment("filestore", ".", 300, 0, -1,
                provider, diagnostic) != bsp::VfsProviderCreateStatus::created) {
            std::fprintf(stderr, "Mounted pending fixture setup: %s\n", diagnostic.c_str());
            return false;
        }
        auto store = manager->file_store_00be80b0_fragment();
        auto requests = std::make_unique<bsp::FileStoreRequests>(store);
        auto& context = manager->context();
        if (context.mounts.size() != 2 || !context.mounts[0].provider
            || context.mounts[0].provider->kind != bsp::VfsProviderKind::file_store
            || !context.mounts[1].provider
            || context.mounts[1].provider->kind != bsp::VfsProviderKind::physical_directory) return false;
        const std::string original = "Scripts\\DataTables\\INPUTS.LUA";
        const std::string resolved = "scripts/datatables/inputs.lua";
        // Pending manager dispatch must not apply this open-only alias.
        context.aliases = {{resolved, "pending-probe-alias-must-not-be-opened.lua"}};
        unsigned cache_submissions{}, physical_submissions{}, synchronous_physical_opens{};
        unsigned callbacks{}, duplicate_callbacks{}, pumps{};
        bool checked = true;
        DWORD error{};
        for (std::size_t index = 0; index < context.mounts.size(); ++index) {
            auto submit = context.mounts[index].submit_pending;
            context.mounts[index].submit_pending = [&, index, submit](const std::string& first,
                const std::string& second, bsp::PhysicalReadCallback callback,
                std::uint32_t flags, DWORD& failure) {
                ++(index == 0 ? cache_submissions : physical_submissions);
                checked = checked && first == resolved && second == original && flags == 2;
                const bool accepted = submit(first, second, std::move(callback), flags, failure);
                if (index == 0) checked = checked && !accepted && failure == ERROR_SUCCESS;
                return accepted;
            };
        }
        const auto open_physical = context.mounts[1].open_read_only;
        context.mounts[1].open_read_only = [&, open_physical](const std::string& name, std::uint32_t flags) {
            ++synchronous_physical_opens;
            return open_physical(name, flags);
        };
        std::shared_ptr<bsp::MemoryStream> cached;
        const auto completed = [&](const std::string& first, const std::string& second) {
            ++callbacks;
            checked = checked && first == resolved && second == original
                && requests->pending_count() == 0 && store->exists_00be5c00(resolved);
            cached = store->open_00be5fa0(resolved, 2);
            checked = checked && cached && cached->fully_initialized()
                && cached->position_00bef580() == 0 && cached->size_00bef600() == expected.size()
                && std::memcmp(cached->data_00bef610(), expected.data(), expected.size()) == 0;
            // Reuse the manager pump's external DWORD. Its guard must restore
            // the actual pump result after this callback-local rejection.
            const bool recursive_shutdown = manager->stop_pending_submissions(error);
            checked = checked && !recursive_shutdown && error == ERROR_BUSY;
        };
        const auto duplicate = [&](const std::string&, const std::string&) { ++duplicate_callbacks; };
        may_have_pending = true; // Conservative fail-fast protection around submission.
        const auto accepted = requests->request_file_00be7cd0_fragment(context, {}, original, completed, error);
        may_have_pending = accepted == bsp::FileStoreRequestResult::queued;
        if (!may_have_pending) {
            std::fprintf(stderr, "Mounted FileStore request rejected before queueing: error=%lu\n", error);
            return false;
        }
        checked = checked && may_have_pending && error == ERROR_SUCCESS
            && callbacks == 0 && requests->pending_count() == 1;
        const auto repeated = requests->request_file_00be7cd0_fragment(context, {}, resolved, duplicate, error);
        checked = checked && repeated == bsp::FileStoreRequestResult::already_pending
            && callbacks == 0 && duplicate_callbacks == 0 && requests->pending_count() == 1
            && cache_submissions == 1 && physical_submissions == 1;
        const bool shutdown = manager->stop_pending_submissions(error);
        checked = checked && shutdown && error == ERROR_SUCCESS;
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
        std::size_t total_completed{}, total_failed{}, no_op_visits{}, physical_visits{};
        while (may_have_pending) {
            if (std::chrono::steady_clock::now() >= deadline)
                pending_probe_exit("mounted manager 15-second completion deadline", requests->pending_count());
            bsp::VfsPendingPumpReport report;
            const bool ok = bsp::pump_pending_resources_00bdb0b0_fragment(context, report, error);
            ++pumps;
            if (report.visits.size() != context.mounts.size()
                || !std::all_of(report.visits.begin(), report.visits.end(),
                    [](const auto& visit) { return visit.invoked; }))
                pending_probe_exit("manager pump did not inspect every pending owner", requests->pending_count());
            may_have_pending = report.remaining_observations != 0;
            total_completed += report.completed;
            total_failed += report.failed;
            checked = checked && ok && error == ERROR_SUCCESS && report.visits.size() == 2;
            for (const auto& visit : report.visits) {
                checked = checked && visit.invoked && visit.succeeded && visit.error == ERROR_SUCCESS;
                if (visit.mount_index == 0) {
                    ++no_op_visits;
                    checked = checked && visit.reads.completed == 0 && visit.reads.remaining == 0
                        && visit.reads.completions.empty();
                } else {
                    ++physical_visits;
                    for (const auto& read : visit.reads.completions)
                        checked = checked && read.first_name == resolved && read.second_name == original
                            && read.callback_invoked && read.transferred_size_valid
                            && read.logical_size == expected.size() && read.transferred_size == expected.size()
                            && read.error == ERROR_SUCCESS && read.close_error == ERROR_SUCCESS;
                }
            }
            if (!ok) std::fprintf(stderr, "Mounted pending pump: error=%lu failed=%zu remaining=%zu\n",
                error, report.failed, report.remaining_observations);
            if (may_have_pending) Sleep(1);
        }
        const auto resident = requests->request_file_00be7cd0_fragment(context, {}, original, duplicate, error);
        checked = checked && resident == bsp::FileStoreRequestResult::already_resident
            && callbacks == 1 && duplicate_callbacks == 0 && requests->pending_count() == 0
            && total_completed == 1 && total_failed == 0 && cache_submissions == 1 && physical_submissions == 1;
        context.aliases.clear();
        auto opened = bsp::open_resource_memory_00bdf310_fragment(context, resolved, 2);
        checked = checked && opened.provider_opened && opened.stream && opened.error.empty()
            && synchronous_physical_opens == 0 && cached
            && opened.stream->data_00bef610() == cached->data_00bef610();
        // All I/O is terminal before any factory/controller/provider owner dies.
        manager.reset();
        requests.reset();
        store.reset();
        factories.reset();
        opened.stream.reset();
        if (cached) {
            std::vector<char> actual(expected.size());
            std::uint32_t count{};
            checked = checked && cached->read_00bef590(actual.data(), static_cast<std::uint32_t>(actual.size()), &count)
                && count == expected.size() && std::memcmp(actual.data(), expected.data(), expected.size()) == 0;
        } else checked = false;
        std::printf("Mounted pending FileStore: callbacks=%u duplicate_callbacks=%u cache_declines=%u physical_submissions=%u pumps=%u no_op_visits=%zu physical_visits=%zu completed=%zu failed=%zu original_resolved_names_alias_bypass_and_cache_lifetime=%d\n",
            callbacks, duplicate_callbacks, cache_submissions, physical_submissions, pumps,
            no_op_visits, physical_visits, total_completed, total_failed, checked);
        return checked;
    } catch (const std::exception& exception) {
        if (may_have_pending) pending_probe_exit(exception.what(), 1);
        std::fprintf(stderr, "Mounted pending fixture exception: %s\n", exception.what());
        return false;
    } catch (...) {
        if (may_have_pending) pending_probe_exit("mounted fixture nonstandard exception", 1);
        return false;
    }
}
}

bool probe_physical_pending_reads(const std::string& physical_path) {
    std::unique_ptr<bsp::PhysicalPendingReads> owner;
    try {
        // Independent ordinary file I/O supplies the expected bytes. Do not use
        // the pending implementation or its returned memory to derive the oracle.
        std::ifstream input(physical_path, std::ios::binary);
        if (!input) {
            std::fprintf(stderr, "Physical pending read probe: cannot read oracle %s\n", physical_path.c_str());
            return false;
        }
        const std::string expected((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        if (input.bad() || expected.empty() ||
            expected.size() > static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)())) return false;
        input.close();
        const auto logical_size = static_cast<std::uint32_t>(expected.size());
        const auto requested_size = (logical_size + 0xffffu) & ~0xffffu;
        const std::array<std::string, 2> first_names{
            "scripts/datatables/inputs.lua", "second/scripts/datatables/inputs.lua"};
        const std::array<std::string, 2> second_names{
            "Scripts\\DataTables\\inputs.lua", "Second\\Original\\inputs.lua"};
        std::array<std::shared_ptr<bsp::MemoryStream>, 2> retained;
        owner = std::make_unique<bsp::PhysicalPendingReads>();
        unsigned callbacks{}, pumps{};
        std::size_t completed{}, succeeded{}, failed{};
        std::uint64_t transferred_total{};
        bool checked = true, second_submitted = false, stable_names = false;
        DWORD error{}, submit_error{};
        const auto verify_callback = [&](std::size_t index, const std::shared_ptr<bsp::MemoryStream>& memory,
            const std::string& first, const std::string& second) {
            checked = checked && first == first_names[index] && second == second_names[index]
                && memory && memory->fully_initialized() && memory->position_00bef580() == 0
                && memory->size_00bef600() == logical_size
                && std::memcmp(memory->data_00bef610(), expected.data(), expected.size()) == 0;
            retained[index] = memory;
        };
        const auto second_callback = [&](std::shared_ptr<bsp::MemoryStream> memory,
            const std::string& first, const std::string& second) {
            ++callbacks;
            checked = checked && callbacks == 2;
            verify_callback(1, memory, first, second);
            // Reuse pump's output DWORD: its final result must supersede this
            // callback-local guard error (the independent review regression).
            const bool shutdown_during_pump = owner->begin_shutdown(error);
            checked = checked && !shutdown_during_pump && error == ERROR_BUSY;
        };
        const auto first_callback = [&](std::shared_ptr<bsp::MemoryStream> memory,
            const std::string& first, const std::string& second) {
            ++callbacks;
            checked = checked && callbacks == 1;
            verify_callback(0, memory, first, second);
            second_submitted = owner->submit_00bf43b0_fragment(physical_path,
                first_names[1], second_names[1], 0x32, second_callback, submit_error);
            // The first entry is still queued: append grows its initial capacity1.
            // The second callback must not run inline, and current name refs must
            // remain valid across that growth in this explicit safe host interface.
            stable_names = first == first_names[0] && second == second_names[0];
            checked = checked && second_submitted && submit_error == ERROR_SUCCESS
                && callbacks == 1 && owner->pending_count() == 2 && stable_names;
        };
        const bool submitted = owner->submit_00bf43b0_fragment(physical_path,
            first_names[0], second_names[0], 2, first_callback, error);
        const DWORD initial_submit_error = error;
        if (!submitted)
            std::fprintf(stderr, "Physical pending submission failed: error=%lu path=%s\n", error, physical_path.c_str());
        checked = checked && submitted && error == ERROR_SUCCESS && callbacks == 0
            && owner->pending_count() == 1;
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
        const auto pump = [&] {
            if (std::chrono::steady_clock::now() >= deadline)
                pending_probe_exit("15-second completion deadline", owner->pending_count());
            bsp::PhysicalReadPumpReport report;
            const bool ok = owner->pump_00bf46b0(report, error);
            if (!ok)
                std::fprintf(stderr, "Physical pending pump failed: error=%lu completed=%zu failed=%zu remaining=%zu incomplete=%zu\n",
                    error, report.completed, report.failed, report.remaining, report.incomplete_observations);
            ++pumps;
            completed += report.completed; succeeded += report.succeeded; failed += report.failed;
            checked = checked && ok && error == ERROR_SUCCESS && report.failed == 0
                && report.incomplete_observations == 0 && report.remaining == owner->pending_count();
            for (const auto& result : report.completions) {
                transferred_total += result.transferred_size;
                const auto index = result.first_name == first_names[0] ? std::size_t{0} : std::size_t{1};
                checked = checked && result.first_name == first_names[index]
                    && result.second_name == second_names[index] && result.callback_invoked
                    && result.outcome == bsp::PhysicalReadOutcome::succeeded
                    && result.error == ERROR_SUCCESS && result.close_error == ERROR_SUCCESS
                    && result.logical_size == logical_size && result.requested_size == requested_size
                    && result.transferred_size_valid && result.transferred_size == logical_size;
                if (result.error != ERROR_SUCCESS || result.close_error != ERROR_SUCCESS)
                    std::fprintf(stderr, "Physical pending completion: name=%s logical=%u requested=%u actual=%u valid=%d error=%lu close_error=%lu\n",
                        result.first_name.c_str(), result.logical_size, result.requested_size,
                        result.transferred_size, result.transferred_size_valid, result.error, result.close_error);
            }
            if (owner->pending_count()) Sleep(1);
        };
        while (!second_submitted && owner->pending_count()) pump();
        const bool shutdown = owner->begin_shutdown(error);
        checked = checked && shutdown && error == ERROR_SUCCESS && !owner->accepting();
        while (owner->pending_count()) pump();
        owner.reset(); // Drained owner; retained copied streams must remain usable.
        checked = checked && callbacks == 2 && completed == 2 && succeeded == 2 && failed == 0
            && retained[0] && retained[1]
            && retained[0]->data_00bef610() != retained[1]->data_00bef610();
        if (checked) {
            std::vector<char> observed(expected.size());
            std::uint32_t actual{};
            checked = retained[0]->read_00bef590(observed.data(), logical_size, &actual)
                && actual == logical_size && std::memcmp(observed.data(), expected.data(), expected.size()) == 0
                && retained[1]->position_00bef580() == 0
                && retained[1]->fully_initialized()
                && std::memcmp(retained[1]->data_00bef610(), expected.data(), expected.size()) == 0;
        }
        std::printf("Installed physical pending reads: logical=%u rounded=%u callbacks=%u pumps=%u completed=%zu succeeded=%zu failed=%zu transferred=%llu stable_callback_names=%d deferred_append_shutdown_and_owned_copy=%d initial_submit_error=%lu append_error=%lu\n",
            logical_size, requested_size, callbacks, pumps, completed, succeeded, failed,
            static_cast<unsigned long long>(transferred_total), stable_names, checked, initial_submit_error, submit_error);
        return checked && mounted_pending_fixture(physical_path, expected);
    } catch (const std::exception& exception) {
        if (owner && owner->pending_count()) pending_probe_exit(exception.what(), owner->pending_count());
        std::fprintf(stderr, "Physical pending read probe exception: %s\n", exception.what());
        return false;
    } catch (...) {
        if (owner && owner->pending_count()) pending_probe_exit("nonstandard exception", owner->pending_count());
        std::fprintf(stderr, "Physical pending read probe: nonstandard exception\n");
        return false;
    }
}
