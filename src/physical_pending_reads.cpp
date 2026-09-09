#include "bsp/physical_pending_reads.hpp"
#include <cstdlib>
#include <exception>
#include <limits>
#include <utility>

namespace bsp {
namespace {
constexpr ULONG_PTR native_pending_status = 0x103;
constexpr std::uint32_t alignment = 0x10000;
constexpr std::size_t maximum_native_records =
    static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)()) / 0x38;
static_assert(sizeof(void*) == 4 && sizeof(OVERLAPPED) == 20,
    "Physical pending reads target native Win32 OVERLAPPED layout.");

struct FreeStaging {
    void operator()(std::uint8_t* value) const noexcept { std::free(value); }
};

struct Request {
    HANDLE handle{INVALID_HANDLE_VALUE};
    std::unique_ptr<std::uint8_t, FreeStaging> staging;
    std::uint8_t* aligned{};
    std::unique_ptr<OVERLAPPED> overlapped;
    std::string first_name, second_name;
    PhysicalReadCallback callback;
    std::uint32_t logical_size{}, requested_size{};
    bool io_may_be_pending{};

    ~Request() {
        // Never let a host exception unwind allocations still owned by the OS.
        if (io_may_be_pending) std::terminate();
        if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
    }
    DWORD release_terminal() noexcept {
        if (io_may_be_pending) std::terminate();
        // Native00bf4785..47b0: original allocation, OVERLAPPED, then handle.
        staging.reset();
        aligned = nullptr;
        overlapped.reset();
        if (handle == INVALID_HANDLE_VALUE) return ERROR_SUCCESS;
        const bool closed = CloseHandle(handle) != FALSE;
        const DWORD error = closed ? ERROR_SUCCESS : GetLastError();
        handle = INVALID_HANDLE_VALUE;
        return error;
    }
};

bool has_nul(std::string_view value) noexcept {
    return value.find('\0') != std::string_view::npos;
}

bool still_pending(const Request& request) noexcept {
    const volatile OVERLAPPED* observed = request.overlapped.get();
    return observed->Internal == native_pending_status;
}
}

struct PhysicalPendingReads::Impl {
    std::vector<std::unique_ptr<Request>> requests;
    bool accepting{true};
    bool pumping{};
};

PhysicalPendingReads::PhysicalPendingReads() : impl_(std::make_unique<Impl>()) {}

PhysicalPendingReads::~PhysicalPendingReads() {
    if (impl_->pumping || !impl_->requests.empty()) std::terminate();
}

bool PhysicalPendingReads::submit_00bf43b0_fragment(std::string_view physical_path,
    std::string_view first_name, std::string_view second_name, std::uint32_t flags,
    PhysicalReadCallback callback, DWORD& error) {
    error = ERROR_SUCCESS;
    if (!impl_->accepting) { error = ERROR_OPERATION_ABORTED; return false; }
    if ((flags & 1u) != 0 || (flags & 0xeu) != 2u || !callback ||
        has_nul(physical_path) || has_nul(first_name) || has_nul(second_name)) {
        error = ERROR_INVALID_PARAMETER;
        return false;
    }
    if (impl_->requests.size() >= maximum_native_records) {
        error = ERROR_NOT_ENOUGH_MEMORY;
        return false;
    }
    // Reserve/copy before ReadFile: once accepted, append cannot throw and no
    // stack-owned state or exception can discard an outstanding OS operation.
    if (impl_->requests.size() == impl_->requests.capacity()) {
        const auto capacity = impl_->requests.capacity();
        const auto wanted = capacity == 0 ? std::size_t{1}
            : (capacity > maximum_native_records / 2 ? maximum_native_records : capacity * 2);
        impl_->requests.reserve(wanted);
    }
    auto request = std::make_unique<Request>();
    request->first_name.assign(first_name);
    request->second_name.assign(second_name);
    request->callback = std::move(callback);
    request->overlapped = std::make_unique<OVERLAPPED>(); // All five DWORDs zero.
    const std::string path(physical_path);
    request->handle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, nullptr);
    if (request->handle == INVALID_HANDLE_VALUE) { error = GetLastError(); return false; }
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(request->handle, &size)) { error = GetLastError(); return false; }
    if (size.QuadPart <= 0 || size.QuadPart > (std::numeric_limits<std::int32_t>::max)()) {
        error = ERROR_NOT_SUPPORTED;
        return false;
    }
    request->logical_size = static_cast<std::uint32_t>(size.QuadPart);
    request->requested_size = (request->logical_size + alignment - 1u) & ~(alignment - 1u);
    request->staging.reset(static_cast<std::uint8_t*>(
        std::malloc(static_cast<std::size_t>(request->logical_size) + 0x20000u)));
    if (!request->staging) { error = ERROR_NOT_ENOUGH_MEMORY; return false; }
    const auto base = reinterpret_cast<std::uintptr_t>(request->staging.get());
    const auto aligned_address = (base + alignment - 1u) &
        ~static_cast<std::uintptr_t>(alignment - 1u);
    request->aligned = reinterpret_cast<std::uint8_t*>(aligned_address);
    const bool immediate = ReadFile(request->handle, request->aligned,
        request->requested_size, nullptr, request->overlapped.get()) != FALSE;
    if (!immediate) {
        const DWORD read_error = GetLastError();
        if (read_error != ERROR_IO_PENDING) { error = read_error; return false; }
    }
    request->io_may_be_pending = true;
    impl_->requests.push_back(std::move(request)); // Capacity already reserved.
    return true;
}

bool PhysicalPendingReads::pump_00bf46b0(PhysicalReadPumpReport& report, DWORD& error) {
    error = ERROR_SUCCESS;
    if (impl_->pumping) { error = ERROR_BUSY; return false; }
    report = {};
    impl_->pumping = true;
    struct PumpGuard {
        Impl& state;
        PhysicalReadPumpReport& report;
        DWORD& error;
        ~PumpGuard() {
            report.remaining = state.requests.size();
            error = report.first_error; // A callback may reuse the caller's DWORD.
            state.pumping = false;
        }
    } guard{*impl_, report, error};
    const auto observe_error = [&](DWORD value) {
        if (report.first_error == ERROR_SUCCESS) report.first_error = value;
        error = report.first_error;
    };
    std::size_t index = 0;
    while (index < impl_->requests.size()) {
        Request& request = *impl_->requests[index];
        if (still_pending(request)) { ++index; continue; }
        DWORD transferred{};
        const bool read_ok = GetOverlappedResult(request.handle,
            request.overlapped.get(), &transferred, FALSE) != FALSE;
        const DWORD read_error = read_ok ? ERROR_SUCCESS : GetLastError();
        if (!read_ok && (read_error == ERROR_IO_INCOMPLETE || read_error == ERROR_IO_PENDING ||
            still_pending(request))) {
            // Native would clean up after every failed result. A still-pending
            // observation must instead preserve allocations until OS completion.
            ++report.incomplete_observations;
            observe_error(read_error);
            ++index;
            continue;
        }
        request.io_may_be_pending = false;
        PhysicalReadCompletion completion;
        completion.first_name = request.first_name;
        completion.second_name = request.second_name;
        completion.logical_size = request.logical_size;
        completion.requested_size = request.requested_size;
        completion.transferred_size = transferred;
        completion.transferred_size_valid = read_ok;
        std::shared_ptr<MemoryStream> stream;
        if (!read_ok) {
            completion.outcome = PhysicalReadOutcome::io_failed;
            completion.error = read_error;
        } else if (transferred < request.logical_size) {
            completion.outcome = PhysicalReadOutcome::short_read;
            completion.error = ERROR_HANDLE_EOF;
        } else if (transferred > request.requested_size) {
            completion.outcome = PhysicalReadOutcome::invalid_transfer_count;
            completion.error = ERROR_INVALID_DATA;
        } else {
            stream = std::make_shared<MemoryStream>();
            if (!memory_stream_from_bytes_00befa40_fragment(request.aligned,
                request.logical_size, *stream, completion.error)) {
                completion.outcome = PhysicalReadOutcome::copy_failed;
                stream.reset();
            }
        }
        // Report allocation may throw before callback; this terminal request
        // remains owned and may be retried. No callback has run at that point.
        report.completions.push_back(std::move(completion));
        auto& completed = report.completions.back();
        ++report.completed;
        std::exception_ptr callback_error;
        if (stream) {
            completed.callback_invoked = true;
            try {
                request.callback(stream, request.first_name, request.second_name);
            } catch (...) {
                completed.outcome = PhysicalReadOutcome::callback_threw;
                completed.error = ERROR_UNHANDLED_EXCEPTION;
                callback_error = std::current_exception();
            }
        }
        stream.reset(); // Provider's temporary reference; callback may retain it.
        completed.close_error = request.release_terminal();
        if (completed.close_error != ERROR_SUCCESS && completed.error == ERROR_SUCCESS) {
            completed.outcome = PhysicalReadOutcome::close_failed;
            completed.error = completed.close_error;
        }
        if (completed.error != ERROR_SUCCESS) {
            ++report.failed;
            observe_error(completed.error);
        } else {
            ++report.succeeded;
        }
        impl_->requests.erase(impl_->requests.begin() + static_cast<std::ptrdiff_t>(index));
        if (callback_error) std::rethrow_exception(callback_error);
        // Same index now names its ordered successor, including new submissions.
    }
    return report.first_error == ERROR_SUCCESS;
}

bool PhysicalPendingReads::begin_shutdown(DWORD& error) noexcept {
    error = ERROR_SUCCESS;
    if (impl_->pumping) { error = ERROR_BUSY; return false; }
    impl_->accepting = false;
    return true;
}

bool PhysicalPendingReads::accepting() const noexcept { return impl_->accepting; }
std::size_t PhysicalPendingReads::pending_count() const noexcept { return impl_->requests.size(); }
}
