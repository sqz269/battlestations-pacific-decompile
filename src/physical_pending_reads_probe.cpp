// One real-file submission/pump/lifetime scenario, not a native differential test.
#include "bsp/physical_pending_reads.hpp"
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
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
        return checked;
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
