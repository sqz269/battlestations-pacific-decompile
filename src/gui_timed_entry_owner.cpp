#include "bsp/gui_timed_entry_owner.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
class Busy {
public:
    explicit Busy(bool& flag) : flag_(flag) {
        require(!flag_, "timed-entry operation is already executing"); flag_ = true;
    }
    ~Busy() { flag_ = false; }
private: bool& flag_;
};
class AllocationCall {
public:
    explicit AllocationCall(std::size_t& count) noexcept : count_(count) { ++count_; }
    ~AllocationCall() { --count_; }
private: std::size_t& count_;
};
class BorrowedEntry {
public:
    BorrowedEntry(GuiTimedEntryStorage*& field, GuiTimedEntryStorage* entry) noexcept
        : field_(field) { field_ = entry; }
    ~BorrowedEntry() { field_ = nullptr; }
private: GuiTimedEntryStorage*& field_;
};
float float_argument(float source) noexcept {
    float result;
    __asm {
        fld source
        fstp result
    }
    return result;
}
}
GuiTimedEntryOwner::GuiTimedEntryOwner(GuiWidgetOwner& widget, const volatile float& one)
    : widget_(widget), one_(one) {
    static_assert(sizeof(void*) == 4 && sizeof(GuiTimedEntryStorage) == 0x14);
    for (auto word : widget_.extra_fields().pointers_88_90)
        require(word == nullptr, "timed-entry owner requires the original empty canonical header");
}
GuiTimedEntryOwner::~GuiTimedEntryOwner() noexcept {
    if (!backing_.empty() || !entries_.empty() || operation_active()) std::terminate();
}
GuiTimedEntryStorage** GuiTimedEntryOwner::data() const noexcept {
    GuiTimedEntryStorage** result;
    std::memcpy(&result, &widget_.extra_fields().pointers_88_90[0], 4); return result;
}
std::int32_t GuiTimedEntryOwner::count() const noexcept {
    std::int32_t result;
    std::memcpy(&result, &widget_.extra_fields().pointers_88_90[1], 4); return result;
}
std::int32_t GuiTimedEntryOwner::capacity() const noexcept {
    std::int32_t result;
    std::memcpy(&result, &widget_.extra_fields().pointers_88_90[2], 4); return result;
}
void GuiTimedEntryOwner::store_data(GuiTimedEntryStorage** value) noexcept {
    std::memcpy(&widget_.extra_fields().pointers_88_90[0], &value, 4);
}
void GuiTimedEntryOwner::store_count(std::int32_t value) noexcept {
    std::memcpy(&widget_.extra_fields().pointers_88_90[1], &value, 4);
}
void GuiTimedEntryOwner::store_capacity(std::int32_t value) noexcept {
    std::memcpy(&widget_.extra_fields().pointers_88_90[2], &value, 4);
}
void GuiTimedEntryOwner::validate_live() const {
    require(!retired_, "timed-entry header has already been retired");
    require(count() >= 0, "negative timed count addresses outside the owned backing");
    const auto current = data();
    if (!current) {
        require(count() == 0 && capacity() <= 0, "timed header has no owned backing");
        return;
    }
    const auto found = backing_.find(current);
    require(found != backing_.end(), "timed header names foreign or already freed backing");
    require(static_cast<std::uint32_t>(count()) <= found->second / 4,
        "timed count exceeds its physical allocation");
    require(capacity() < 0 || static_cast<std::uint32_t>(capacity()) <= found->second / 4,
        "timed capacity exceeds its physical allocation");
}
GuiTimedEntryStorage*& GuiTimedEntryOwner::slot(std::int32_t index) const {
    validate_live();
    require(index >= 0 && index < count(), "timed slot is outside the current live header");
    return data()[index];
}
void GuiTimedEntryOwner::require_owned_entry(GuiTimedEntryStorage* entry) const {
    if (!entry) return;
    require(entries_.find(entry) != entries_.end(), "timed slot names a foreign or freed entry");
    // Allocation provenance does not freeze the borrowed widget field. Alpha
    // dispatch reloads +8 after callbacks; deletion never reads it natively.
}
GuiTimedEntryStorage** GuiTimedEntryOwner::allocate_backing(std::uint32_t bytes) {
    auto* raw = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    auto** result = static_cast<GuiTimedEntryStorage**>(raw);
    require(result != nullptr, "native null timed-array allocation is outside this interface");
    try {
        const auto inserted = backing_.emplace(result, bytes).second;
        if (!inserted) std::terminate(); // allocator returned another live allocation
    } catch (...) { singleton_lifetime_free(result); throw; }
    return result;
}
void GuiTimedEntryOwner::free_backing(GuiTimedEntryStorage** pointer) {
    if (pointer) {
        const auto found = backing_.find(pointer);
        require(found != backing_.end(), "cannot free foreign timed backing");
        backing_.erase(found);
    }
    singleton_lifetime_free(pointer);
}
void GuiTimedEntryOwner::reserve_00aa6f30(std::int32_t requested) {
    AllocationCall active(active_calls_);
    validate_live();
    const auto target = requested < 1 ? 1 : requested;
    if (capacity() >= target) return;
    // Overflowed native allocation sizes cannot contain the requested array.
    require(static_cast<std::uint32_t>(target) <= 0x3fffffffu,
        "wrapped timed allocation size is outside the physical backing domain");
    auto** fresh = allocate_backing(static_cast<std::uint32_t>(target) * 4u);
    // Allocation/new-handler may change the header. Read count/backing live.
    for (std::int32_t i = 0; i < count(); ++i) {
        require(i < target, "allocation callback grew timed count beyond the new allocation");
        fresh[i] = slot(i);
    }
    free_backing(data()); // free BEFORE pointer/capacity publication
    store_data(fresh);
    store_capacity(target);
}
void GuiTimedEntryOwner::resize_00aa77b0(std::int32_t requested) {
    AllocationCall active(active_calls_);
    validate_live();
    require(requested >= 0, "negative timed resize is outside the physical backing domain");
    if (capacity() < requested) reserve_00aa6f30(requested);
    for (auto i = count(); i < requested; ++i) data()[i] = nullptr;
    while (requested < count()) store_count(count() - 1);
    store_count(requested);
}
GuiTimedEntryStorage* GuiTimedEntryOwner::create_by_index_00ad3a80(std::int32_t index) {
    AllocationCall active(active_calls_);
    validate_live();
    if (index < 0 || index > 2) return nullptr;
    void* raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x14, 0x14});
    if (!raw) return nullptr;
    auto* entry = ::new(raw) GuiTimedEntryStorage;
    try {
        if (!entries_.emplace(entry, Entry{}).second) std::terminate();
    } catch (...) { singleton_lifetime_free(entry); throw; }
    if (index != 2) construct_gui_timed_entry_alpha_00ac2ed0(*entry, widget_.layout(), one_);
    else {
        // AD3ABA..AD3AD0: inline index2 producer, order is significant.
        entry->widget_08 = &widget_.layout();
        entry->remaining_04 = 0.0f;
        entry->profile_00 = 0x00d5d210;
        entry->target_0c = 0.0f;
        entry->rate_10 = 0.0f;
    }
    return entry;
}
GuiTimedEntryStorage* GuiTimedEntryOwner::get_or_create_00aa8b00(std::int32_t index) {
    AllocationCall active(active_calls_);
    validate_live();
    require(index >= 0 && index < std::numeric_limits<std::int32_t>::max(),
        "native negative or wrapped timed index is outside the backing domain");
    if (count() <= index) resize_00aa77b0(index + 1);
    if (!slot(index)) {
        if (count() <= index) resize_00aa77b0(index + 1);
        auto** captured_backing = data();
        auto** captured_slot = &slot(index);
        auto* entry = create_by_index_00ad3a80(index);
        const auto allocation = backing_.find(captured_backing);
        require(allocation != backing_.end() &&
            static_cast<std::uint32_t>(index) < allocation->second / 4,
            "allocation callback invalidated the captured timed destination slot");
        *captured_slot = entry; // original slot, before current-header reload
    }
    if (count() <= index) resize_00aa77b0(index + 1);
    auto* result = slot(index); require_owned_entry(result); return result;
}
void GuiTimedEntryOwner::delete_entry(GuiTimedEntryStorage& entry, std::uint32_t flags) {
    require(updating_entry_ != &entry, "cannot destroy the timed entry executing its current8 callback");
    require_owned_entry(&entry);
    auto& record = entries_.at(&entry);
    if (!record.base_destroyed) {
        destroy_gui_timed_entry_profile(entry);
        record.base_destroyed = true;
    } else {
        require(entry.profile_00 == kGuiTimedEntryBaseProfile,
            "flags0 storage no longer carries its completed base profile");
    }
    if (flags & 1u) {
        entries_.erase(&entry); // consume provenance BEFORE terminal free
        singleton_lifetime_free(&entry);
    }
}
void GuiTimedEntryOwner::update_entries_00aa87b0_fragment(float seconds,
    const GuiTimedEntryConstants& constants) {
    validate_live(); require(!draining_, "cannot update a draining timed owner");
    require(&constants.one_00d7a24c == &one_, "timed update requires the same live constant domain");
    Busy operation(updating_);
    for (std::int32_t i = 0; i < count(); ++i) {
        if (i >= count()) resize_00aa77b0(i + 1);
        if (slot(i)) {
            if (i >= count()) resize_00aa77b0(i + 1);
            auto* entry = slot(i); require_owned_entry(entry);
            require(!entries_.at(entry).base_destroyed, "cannot update an ended timed lifetime");
            bool live;
            {
                BorrowedEntry borrowed(updating_entry_, entry);
                live = advance_gui_timed_entry_00ad39a0(*entry, widget_,
                    float_argument(seconds), &widget_.layout(), constants);
            }
            if (!live) {
                if (i >= count()) resize_00aa77b0(i + 1);
                auto** captured_slot = &slot(i);
                if (*captured_slot) {
                    delete_entry(**captured_slot, 1);
                    *captured_slot = nullptr;
                }
            }
        }
    }
}
void GuiTimedEntryOwner::destroy_entries_00aa9730_fragment() {
    validate_live();
    require(!updating_ && active_calls_ == 0,
        "cannot destroy timed entries during update or allocation callbacks");
    Busy operation(draining_);
    for (std::int32_t i = 0; i < count(); ++i) {
        if (i >= count()) resize_00aa77b0(i + 1);
        auto** captured_slot = &slot(i);
        if (*captured_slot) {
            delete_entry(**captured_slot, 1);
            *captured_slot = nullptr;
        }
    }
    // Native negative-capacity repair is a reserve1 before final resize/free.
    // Negative count's pre-backing writes are outside valid allocated objects.
    if (capacity() < 0) reserve_00aa6f30(1);
    while (count() > 0) store_count(count() - 1);
    auto** final_backing = data();
    store_count(0);
    free_backing(final_backing);
    retired_ = true; // no native pointer/capacity clear after AA994C
}
} // namespace bsp
