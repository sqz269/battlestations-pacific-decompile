#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <type_traits>

namespace bsp {
struct SceneRecord;
struct SetPendingSceneHost;

// The list embedded at GGame+5F0h, not the selected index at +60Ch.
// 004DDB90 initializes these three DWORDs at 004DDCAB/004DDCB1/004DDCB7.
// 004E1D70 fills them at 004E21C1 through 004C2C80. Payloads retain the
// existing semantic SceneRecord interface; the list body reads only pointers.
struct NativeSceneRecordNode {
    NativeSceneRecordNode* previous;
    NativeSceneRecordNode* next;
    SceneRecord* record;
};
struct NativeSceneRecordListImage {
    std::uint32_t count{};
    NativeSceneRecordNode* head{};
    NativeSceneRecordNode* tail{};
};
static_assert(sizeof(void*) == 4);
static_assert(offsetof(NativeSceneRecordNode, previous) == 0);
static_assert(offsetof(NativeSceneRecordNode, next) == 4);
static_assert(offsetof(NativeSceneRecordNode, record) == 8);
static_assert(sizeof(NativeSceneRecordNode) == 0xc);
static_assert(offsetof(NativeSceneRecordListImage, count) == 0);
static_assert(offsetof(NativeSceneRecordListImage, head) == 4);
static_assert(offsetof(NativeSceneRecordListImage, tail) == 8);
static_assert(sizeof(NativeSceneRecordListImage) == 0xc);

// Owns the live mission path's records and nodes. There is no vector mirror.
// Constructed with MissionLoadPathState before the front end can request a
// mission; 004E2770 clears it before replacement, and owner teardown clears
// remaining records through the same reconstructed 004BF930 node walk.
class NativeSceneRecordList final {
public:
    NativeSceneRecordList() = default;
    ~NativeSceneRecordList();
    NativeSceneRecordList(const NativeSceneRecordList&) = delete;
    NativeSceneRecordList& operator=(const NativeSceneRecordList&) = delete;

    template <bool IsConst> class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = SceneRecord;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const SceneRecord*, SceneRecord*>;
        using reference = std::conditional_t<IsConst, const SceneRecord&, SceneRecord&>;
        Iterator() = default;
        explicit Iterator(NativeSceneRecordNode* node) noexcept : node_(node) {}
        reference operator*() const noexcept { return *node_->record; }
        pointer operator->() const noexcept { return node_->record; }
        Iterator& operator++() noexcept { node_ = node_->next; return *this; }
        Iterator operator++(int) noexcept { auto old = *this; ++*this; return old; }
        bool operator==(const Iterator& other) const noexcept { return node_ == other.node_; }
        bool operator!=(const Iterator& other) const noexcept { return !(*this == other); }
    private:
        NativeSceneRecordNode* node_{};
    };
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    std::size_t size() const noexcept { return image_.count; }
    bool empty() const noexcept { return image_.count == 0; }
    SceneRecord& front() noexcept { return *image_.head->record; }
    const SceneRecord& front() const noexcept { return *image_.head->record; }
    const SceneRecord& operator[](std::size_t index) const noexcept;
    iterator begin() noexcept { return iterator(image_.head); }
    iterator end() noexcept { return iterator(); }
    const_iterator begin() const noexcept { return const_iterator(image_.head); }
    const_iterator end() const noexcept { return const_iterator(); }
    const NativeSceneRecordListImage& image() const noexcept { return image_; }

    void push_back(std::unique_ptr<SceneRecord> record);
    void clear(SetPendingSceneHost* host = nullptr);
private:
    NativeSceneRecordListImage image_{};
};
} // namespace bsp
