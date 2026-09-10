#pragma once
#include "bsp/memory_stream.hpp"
#include <array>
#include <memory>
#include <string>

namespace bsp {
// Host rejection states. Native wrappers do not provide these checks/errors.
enum class StructuredReaderError {
    none,
    missing_stream,
    uninitialized_stream,
    invalid_state,
    depth_limit,
    truncated_input,
    payload_out_of_bounds,
    out_of_memory,
    stream_failure,
    cursor_changed
};

class StructuredNode;

// Shared ownership retains the SAME stream/cursor; no clone, seek, or reset.
// Complete-transfer memory-stream projection, not the original native ABI.
// Evidence: docs/STRUCTURED_READER.md, reports/structured_reader_audit.json.
// Use one thread and leave the stream cursor/backing exclusively to this reader.
// Nodes have stable addresses and borrow their reader/parent; only the current
// leaf can read, create a child, skip, or close. Errors are sticky. Cleanup is
// still possible after error and never silently seeks unread data. A failed
// header can already have consumed earlier complete fields; there is no retry
// or rollback contract. Detached nodes return false/null from read and skip.
class StructuredReader {
public:
    explicit StructuredReader(std::shared_ptr<MemoryStream> stream) noexcept;
    // Host guard detaches surviving nodes without seeking before stream release.
    ~StructuredReader() noexcept;
    StructuredReader(const StructuredReader&) = delete;
    StructuredReader& operator=(const StructuredReader&) = delete;
    StructuredReader(StructuredReader&&) = delete;
    StructuredReader& operator=(StructuredReader&&) = delete;

    std::unique_ptr<StructuredNode> read_root_00bea700() noexcept;
    StructuredReaderError error() const noexcept { return error_; }
    std::uint32_t control_word() const noexcept { return control_word_; }
    std::uint32_t active_node_count() const noexcept { return path_count_; }

private:
    friend class StructuredNode;
    bool fail(StructuredReaderError error) noexcept;
    bool stream_ready() noexcept;
    bool check_leaf(const StructuredNode& node) noexcept;
    bool check_extent(std::uint32_t count) noexcept;
    bool read_exact(void* output, std::uint32_t count,
        std::uint32_t& budget, bool enforce_budget) noexcept;
    std::unique_ptr<StructuredNode> create_node(StructuredNode* parent) noexcept;
    bool close_node(StructuredNode& node) noexcept;
    void detach_leaf_no_seek() noexcept;
    void destroy_node(StructuredNode& node) noexcept;

    std::shared_ptr<MemoryStream> stream_;
    StructuredReaderError error_{StructuredReaderError::none};
    StructuredNode* leaf_{};
    std::array<std::string, 10> path_tags_;
    std::uint32_t path_count_{};
    std::uint32_t control_word_{0xffffffffu};
    std::int64_t expected_cursor_{};
};

class StructuredNode {
public:
    ~StructuredNode() noexcept;
    StructuredNode(const StructuredNode&) = delete;
    StructuredNode& operator=(const StructuredNode&) = delete;
    StructuredNode(StructuredNode&&) = delete;
    StructuredNode& operator=(StructuredNode&&) = delete;

    const std::string& tag() const noexcept { return tag_; }
    std::uint32_t declared_payload() const noexcept { return declared_; }
    std::uint32_t remaining() const noexcept { return remaining_; }
    std::uint32_t depth() const noexcept { return depth_; }
    bool attached() const noexcept { return reader_ != nullptr; }
    StructuredReaderError error() const noexcept;
    bool ready() noexcept;
    // Native predicate is just remaining != 0; always inspect reader.error()
    // separately. A closed node can retain a nonzero remaining count.
    bool has_remaining_00715bf0() const noexcept { return remaining_ != 0; }

    std::unique_ptr<StructuredNode> read_child_00bea680() noexcept;
    bool read_control_00be9a40() noexcept;
    // Outputs remain unchanged on host failure. These use nonnull actual-count
    // reads in the complete-transfer domain, never font-style short zero-fill.
    bool read_u32(std::uint32_t& output) noexcept;
    bool read_float(float& output) noexcept;
    //00bea010/00be9fe0/00bf0510: DWORD byte length, then raw string bytes.
    // Retains embedded NUL bytes; C-string consumers apply their own length.
    bool read_string(std::string& output) noexcept;
    // Advance by unread remaining bytes, set remaining=0, then detach.
    bool skip_00be9c40() noexcept;
    // Charge declared payload to parent and detach WITHOUT seeking. Leaves the
    // remaining value intact, like native destruction. Already closed is a no-op.
    bool close() noexcept;

private:
    friend class StructuredReader;
    StructuredNode(StructuredNode* parent, std::string tag,
        std::uint32_t declared, std::uint32_t depth) noexcept;

    StructuredReader* reader_{};
    StructuredNode* parent_{};
    std::string tag_;
    std::uint32_t declared_{};
    std::uint32_t remaining_{};
    std::uint32_t depth_{};
};
}
