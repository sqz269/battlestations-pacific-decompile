#include "bsp/file_store_requests.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/resource_path.hpp"
#include <cstring>
#include <map>
#include <stdexcept>
#include <utility>

namespace bsp {
struct FileStoreRequests::Impl {
    struct NameLess {
        bool operator()(const std::string& left, const std::string& right) const noexcept {
            //00be5530/00443d00: empty gates, then CRT case-insensitive order;
            // no stored-length tie-break after the string comparison.
            if (left.empty()) return !right.empty();
            if (right.empty()) return false;
            return _stricmp(left.c_str(), right.c_str()) < 0;
        }
    };
    explicit Impl(std::shared_ptr<FileStore> source) : store(std::move(source)) {}
    std::shared_ptr<FileStore> store;
    std::map<std::string, FileStoreRequestCallback, NameLess> pending;

    void complete_00be78b0_fragment(std::shared_ptr<MemoryStream> stream,
        const std::string& first_name, const std::string& second_name) {
        // Own the callback's names before any map mutation/user reentry. This
        // does not turn the caller callback into a stream-delivery interface.
        const std::string first = first_name;
        const std::string second = second_name;
        auto normalized = first;
        if (!normalize_resource_path_00bee690(normalized) || !stream
            || !stream->fully_initialized() || stream->size_00bef600() <= 0)
            throw std::runtime_error("FileStore completion received an unsupported name or stream.");
        const auto entry = pending.find(normalized);
        if (entry == pending.end())
            throw std::logic_error("FileStore completion has no matching pending request.");
        auto callback = std::move(entry->second);
        pending.erase(entry); //00be7922, before insertion and user callback.
        const auto inserted = store->add_file_00be7760(normalized, stream);
        if (inserted == FileStoreInsertResult::invalid)
            throw std::runtime_error("FileStore completion could not insert its stream.");
        // A resident added since submission is deliberately retained by AddFile.
        // Native ignores its insertion result and still invokes this callback.
        callback(first, second);
    }
};

FileStoreRequests::FileStoreRequests(std::shared_ptr<FileStore> stable_factory_store) {
    if (!stable_factory_store)
        throw std::invalid_argument("FileStore requests require the shared factory's store.");
    impl_ = std::make_shared<Impl>(std::move(stable_factory_store));
}
FileStoreRequests::~FileStoreRequests() = default;

FileStoreRequestResult FileStoreRequests::request_file_00be7cd0_fragment(
    VfsMountContext& context, const VfsCandidateRegistrations& registrations,
    const std::string& original_name, FileStoreRequestCallback callback, DWORD& error) {
    error = ERROR_SUCCESS;
    if (!callback) { error = ERROR_INVALID_PARAMETER; return FileStoreRequestResult::rejected; }
    // Native00bee780 makes a normalized local copy;00bdf4c0 normalizes again
    // and resolves candidates. The original second name remains untouched.
    const std::string original = original_name;
    auto resolved = original;
    if (!normalize_resource_path_00bee690(resolved)) {
        error = ERROR_INVALID_NAME;
        return FileStoreRequestResult::rejected;
    }
    if (!resolve_existing_resource_00bdf4c0_fragment(context, registrations, resolved)) {
        error = ERROR_FILE_NOT_FOUND;
        return FileStoreRequestResult::rejected;
    }
    if (impl_->store->exists_00be5c00(resolved)) return FileStoreRequestResult::already_resident;
    if (impl_->pending.find(resolved) != impl_->pending.end()) return FileStoreRequestResult::already_pending;
    impl_->pending.emplace(resolved, std::move(callback));
    try {
        // Native adapter00be7b20 reloads the global factory/store. This shared
        // state is valid only under the header's stable-factory-through-drain
        // contract. It owns no context/provider/queue, avoiding a built-in cycle.
        PhysicalReadCallback complete = [state = impl_](std::shared_ptr<MemoryStream> stream,
            const std::string& first, const std::string& second) {
            state->complete_00be78b0_fragment(std::move(stream), first, second);
        };
        if (open_resource_pending_00bdda10_fragment(context, resolved, original,
            std::move(complete), 2, error)) return FileStoreRequestResult::queued;
    } catch (...) {
        // Supported providers do not throw after accepting I/O or call back
        // inline. Their allocation/callback-copy exceptions precede ReadFile.
        impl_->pending.erase(resolved);
        throw;
    }
    //00be7f1d: remove newly inserted key on immediate submission rejection.
    // No terminal-pump observer removes keys; failed completed I/O stays pending.
    impl_->pending.erase(resolved);
    return FileStoreRequestResult::rejected;
}

std::size_t FileStoreRequests::pending_count() const noexcept {
    return impl_->pending.size();
}
}
