#include "bsp/structured_reader.hpp"
#include <new>
#include <utility>

namespace bsp {
StructuredReader::StructuredReader(std::shared_ptr<MemoryStream> stream) noexcept
    : stream_(std::move(stream)) {
    if (!stream_ || !stream_->has_backing()) {
        fail(StructuredReaderError::missing_stream);
    } else if (!stream_->fully_initialized()) {
        fail(StructuredReaderError::uninitialized_stream);
    } else {
        expected_cursor_ = stream_->position_00bef580();
    }
}

StructuredReader::~StructuredReader() noexcept {
    // Host lifetime guard: surviving node handles become detached before the
    // retained stream is released. This does not traverse or skip their bytes.
    while (leaf_) detach_leaf_no_seek();
}

bool StructuredReader::fail(StructuredReaderError error) noexcept {
    if (error_ == StructuredReaderError::none) error_ = error;
    return false;
}

bool StructuredReader::stream_ready() noexcept {
    if (error_ != StructuredReaderError::none) return false;
    if (!stream_ || !stream_->has_backing())
        return fail(StructuredReaderError::missing_stream);
    if (!stream_->fully_initialized())
        return fail(StructuredReaderError::uninitialized_stream);
    if (stream_->position_00bef580() != expected_cursor_)
        return fail(StructuredReaderError::cursor_changed);
    return true;
}

bool StructuredReader::check_leaf(const StructuredNode& node) noexcept {
    if (error_ != StructuredReaderError::none) return false;
    if (node.reader_ != this || leaf_ != &node)
        return fail(StructuredReaderError::invalid_state);
    return stream_ready();
}

bool StructuredReader::check_extent(std::uint32_t count) noexcept {
    if (!stream_ready()) return false;
    const std::int64_t size = stream_->size_00bef600();
    if (expected_cursor_ < 0 || expected_cursor_ > size ||
        static_cast<std::uint64_t>(count) >
            static_cast<std::uint64_t>(size - expected_cursor_))
        return fail(StructuredReaderError::truncated_input);
    return true;
}

bool StructuredReader::read_exact(void* output, std::uint32_t count,
    std::uint32_t& budget, bool enforce_budget) noexcept {
    if (enforce_budget && count > budget)
        return fail(StructuredReaderError::payload_out_of_bounds);
    if (!check_extent(count)) return false;
    std::uint32_t actual = 0;
    const bool success = stream_->read_00bef590(output, count, &actual);
    expected_cursor_ = stream_->position_00bef580();
    budget -= actual;
    if (!success) return fail(StructuredReaderError::stream_failure);
    if (actual != count) return fail(StructuredReaderError::truncated_input);
    return true;
}

std::unique_ptr<StructuredNode> StructuredReader::read_root_00bea700() noexcept {
    return create_node(nullptr);
}

std::unique_ptr<StructuredNode> StructuredReader::create_node(
    StructuredNode* parent) noexcept {
    if (!stream_ready()) return {};
    if (leaf_ != parent || (parent && parent->reader_ != this)) {
        fail(StructuredReaderError::invalid_state);
        return {};
    }
    if (path_count_ >= path_tags_.size()) {
        fail(StructuredReaderError::depth_limit);
        return {};
    }

    // Native root's1000 is only scratch accounting. It is deliberately not a
    // host tag-size restriction. Child headers charge their parent's budget.
    std::uint32_t root_header_budget = 1000;
    std::uint32_t& budget = parent ? parent->remaining_ : root_header_budget;
    const bool enforce_budget = parent != nullptr;
    std::uint32_t tag_length = 0;
    if (!read_exact(&tag_length, 4, budget, enforce_budget)) return {};

    // Reject the malformed domain before allocating. Include the following
    // payload-length DWORD without overflowing the serialized32-bit length.
    const std::uint64_t header_tail = static_cast<std::uint64_t>(tag_length) + 4;
    const std::int64_t bytes_left = stream_->size_00bef600() - expected_cursor_;
    if (header_tail > static_cast<std::uint64_t>(bytes_left)) {
        fail(StructuredReaderError::truncated_input);
        return {};
    }
    if (enforce_budget && header_tail > budget) {
        fail(StructuredReaderError::payload_out_of_bounds);
        return {};
    }

    try {
        // Native00be4620 seeds logical string bytes with spaces before reading.
        std::string tag(tag_length, ' ');
        if (tag_length != 0 &&
            !read_exact(tag.data(), tag_length, budget, enforce_budget)) return {};
        std::uint32_t payload_length = 0;
        if (!read_exact(&payload_length, 4, budget, enforce_budget)) return {};
        if (enforce_budget && payload_length > budget) {
            fail(StructuredReaderError::payload_out_of_bounds);
            return {};
        }
        if (!check_extent(payload_length)) return {};

        const std::uint32_t depth = parent ? parent->depth_ + 1 : 0;
        auto node = std::unique_ptr<StructuredNode>(
            new StructuredNode(parent, std::move(tag), payload_length, depth));
        // Commit attachment only after both owned string allocations succeed.
        path_tags_[path_count_] = node->tag_;
        node->reader_ = this;
        leaf_ = node.get();
        ++path_count_;
        return node;
    } catch (const std::bad_alloc&) {
        fail(StructuredReaderError::out_of_memory);
        return {};
    }
}

void StructuredReader::detach_leaf_no_seek() noexcept {
    StructuredNode* node = leaf_;
    StructuredNode* parent = node->parent_;
    if (parent) {
        if (node->declared_ > parent->remaining_)
            fail(StructuredReaderError::payload_out_of_bounds);
        parent->remaining_ -= node->declared_;
    }
    if (path_count_ != 0) --path_count_;
    else fail(StructuredReaderError::invalid_state);
    // Native path string remains in its slot until overwritten or destroyed.
    leaf_ = parent;
    node->reader_ = nullptr;
    node->parent_ = nullptr;
}

bool StructuredReader::close_node(StructuredNode& node) noexcept {
    if (leaf_ != &node || node.reader_ != this)
        return fail(StructuredReaderError::invalid_state);
    detach_leaf_no_seek();
    return error_ == StructuredReaderError::none;
}

void StructuredReader::destroy_node(StructuredNode& node) noexcept {
    if (leaf_ != &node) {
        // Native requires parents to outlive active children. On host misuse,
        // detach descendants without seeking so their later destructors cannot
        // dereference a freed parent. The sticky error prevents more parsing.
        fail(StructuredReaderError::invalid_state);
        while (leaf_ && leaf_ != &node) detach_leaf_no_seek();
    }
    if (leaf_ == &node) detach_leaf_no_seek();
}

StructuredNode::StructuredNode(StructuredNode* parent, std::string tag,
    std::uint32_t declared, std::uint32_t depth) noexcept
    : parent_(parent), tag_(std::move(tag)), declared_(declared),
      remaining_(declared), depth_(depth) {}

StructuredNode::~StructuredNode() noexcept {
    if (reader_) reader_->destroy_node(*this);
}

StructuredReaderError StructuredNode::error() const noexcept {
    return reader_ ? reader_->error() : StructuredReaderError::invalid_state;
}

bool StructuredNode::ready() noexcept {
    return reader_ && reader_->check_leaf(*this);
}

std::unique_ptr<StructuredNode> StructuredNode::read_child_00bea680() noexcept {
    if (!reader_ || !reader_->check_leaf(*this)) return {};
    return reader_->create_node(this);
}

bool StructuredNode::read_control_00be9a40() noexcept {
    std::uint32_t value;
    if (!read_u32(value)) return false;
    reader_->control_word_ = value;
    return true;
}

bool StructuredNode::read_u32(std::uint32_t& output) noexcept {
    if (!reader_ || !reader_->check_leaf(*this)) return false;
    std::uint32_t value;
    if (!reader_->read_exact(&value, 4, remaining_, true)) return false;
    output = value;
    return true;
}

bool StructuredNode::read_float(float& output) noexcept {
    std::uint32_t bits;
    if (!read_u32(bits)) return false;
    //00be4360 FLD32 -> ST0;00bf02c0 rounds through a float temporary. Full
    // reads erase the pointer-seeded argument-slot distinction. Short reads
    // are rejected above, not replaced with the font adapter's zero-fill.
    float value;
    __asm {
        fld dword ptr bits
        fstp dword ptr value
    }
    output = value;
    return true;
}

bool StructuredNode::read_string(std::string& output) noexcept {
    std::uint32_t length;
    if (!read_u32(length)) return false;
    if (length > remaining_)
        return reader_->fail(StructuredReaderError::payload_out_of_bounds);
    if (!reader_->check_extent(length)) return false;
    try {
        std::string value(length, ' ');
        if (length != 0 && !reader_->read_exact(value.data(), length, remaining_, true))
            return false;
        output = std::move(value);
        return true;
    } catch (const std::bad_alloc&) {
        return reader_->fail(StructuredReaderError::out_of_memory);
    }
}

bool StructuredNode::skip_00be9c40() noexcept {
    if (!reader_ || !reader_->check_leaf(*this)) return false;
    if (!reader_->check_extent(remaining_)) return false;
    if (remaining_ != 0) {
        if (!reader_->stream_->seek_00bef540(remaining_, 1))
            return reader_->fail(StructuredReaderError::stream_failure);
        reader_->expected_cursor_ = reader_->stream_->position_00bef580();
        remaining_ = 0;
    }
    return reader_->close_node(*this);
}

bool StructuredNode::close() noexcept {
    return !reader_ || reader_->close_node(*this);
}
}
