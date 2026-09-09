#pragma once
#include "bsp/file_store.hpp"
#include "bsp/vfs_pending.hpp"
#include "bsp/vfs_candidates.hpp"

namespace bsp {
using FileStoreRequestCallback = std::function<void(
    const std::string& first_name, const std::string& second_name)>;
enum class FileStoreRequestResult { rejected, already_resident, already_pending, queued };

// Secondary FileStore tree+20h projected separately from its resident tree.
// STABLE FACTORY CONTRACT: stable_factory_store is the provider at the current
// singleton factory+8. That factory/store identity must remain unchanged until
// all physical queues accepted through this controller have drained. Native
//00be7b20 re-reads the CURRENT singleton; it does not retain an originating store.
// Use one controller per shared factory store, not one per mount record.
// Caller serializes all access. No recursive pump or destruction during callback.
class FileStoreRequests {
public:
    explicit FileStoreRequests(std::shared_ptr<FileStore> stable_factory_store);
    // Drain accepted physical queues before destruction. Pending keys alone do
    // not measure active OS I/O: terminal physical failures retain their key.
    // No wait, cancellation, callback dispatch or failure-state repair occurs.
    ~FileStoreRequests();
    FileStoreRequests(const FileStoreRequests&) = delete;
    FileStoreRequests& operator=(const FileStoreRequests&) = delete;
    FileStoreRequests(FileStoreRequests&&) = delete;
    FileStoreRequests& operator=(FileStoreRequests&&) = delete;

    //00be7cd0: ECX store; original name,user callback; AL; RET8.
    // Resolve a normalized copy BEFORE checking resident/pending keys. Duplicate
    // states mean native true, with no new submission or callback registration.
    // A new key stores only the first callback and submits flags2 with resolved
    // first name and UNCHANGED original second name. Immediate submission
    // rejection removes that key; terminal pump failure leaves it observable.
    // Completion00be78b0 removes key, adds stream, then calls user(first,second).
    // AddFile's existing resident wins if another insertion occurred meanwhile.
    // Host requires callable callbacks, initialized positive completion streams,
    // and the existing resolver's string domain. Guard failures reject/error.
    // Allocation/provider/user exceptions propagate. Submission exceptions from
    // the supported providers occur before acceptance and remove the new key.
    FileStoreRequestResult request_file_00be7cd0_fragment(VfsMountContext&,
        const VfsCandidateRegistrations&, const std::string& original_name,
        FileStoreRequestCallback, DWORD& error);

    // Includes keys stranded by completed physical I/O failure; no implicit
    // retry, failure callback, cancellation or clearing operation is provided.
    std::size_t pending_count() const noexcept;

private:
    struct Impl;
    std::shared_ptr<Impl> impl_;
};
}
