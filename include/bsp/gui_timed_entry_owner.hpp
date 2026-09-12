#pragma once
#include "bsp/gui_timed_entry_types.hpp"
#include <cstdint>
#include <unordered_map>

namespace bsp {
// Owns ONLY newly allocated backing/entry storage, in the existing source CRT
// malloc/new-handler/free domain. The live array is the owner's SAME +88/+8C/+90
// projection. No foreign header adoption, copied vector or second widget tree.
// Source CRT handler/EH identity and raw widget ABI remain explicit boundaries.
class GuiTimedEntryOwner final {
public:
    GuiTimedEntryOwner(GuiWidgetOwner&, const volatile float& one_00d7a24c);
    ~GuiTimedEntryOwner() noexcept;
    GuiTimedEntryOwner(const GuiTimedEntryOwner&) = delete;
    GuiTimedEntryOwner& operator=(const GuiTimedEntryOwner&) = delete;
    const volatile float& constructor_one() const noexcept { return one_; }
    GuiTimedEntryStorage** data() const noexcept;
    std::int32_t count() const noexcept;
    std::int32_t capacity() const noexcept;
    void validate_live() const;
    void reserve_00aa6f30(std::int32_t);
    void resize_00aa77b0(std::int32_t);
    GuiTimedEntryStorage* create_by_index_00ad3a80(std::int32_t);
    GuiTimedEntryStorage* get_or_create_00aa8b00(std::int32_t);
    // Exact current0 profile teardown; flags0 preserves allocation until a
    // later explicit flags1 return. Never interprets a numeric vtable as code.
    void delete_entry(GuiTimedEntryStorage&, std::uint32_t flags);
    // AA884D..AA892C only, AFTER the original child-update traversal. Borrowed
    // widget argument remains the original caller. No parent listener tail.
    void update_entries_00aa87b0_fragment(float seconds, const GuiTimedEntryConstants&);
    // AA97F6..AA9950. Captured slots clear only after actual entry deletion.
    // Final count=0; data/capacity remain native dangling values after free.
    // Provenance metadata prevents repeated final free. No automatic rollback.
    void destroy_entries_00aa9730_fragment();
    bool retired() const noexcept { return retired_; }
    bool operation_active() const noexcept { return active_calls_ || draining_ || updating_; }
    std::size_t retained_entry_allocations() const noexcept { return entries_.size(); }
private:
    struct Entry { bool base_destroyed{}; };
    GuiWidgetOwner& widget_;
    const volatile float& one_;
    std::unordered_map<GuiTimedEntryStorage**, std::uint32_t> backing_;
    std::unordered_map<GuiTimedEntryStorage*, Entry> entries_;
    bool retired_{};
    bool draining_{};
    bool updating_{};
    std::size_t active_calls_{};
    GuiTimedEntryStorage* updating_entry_{};
    void store_data(GuiTimedEntryStorage**) noexcept;
    void store_count(std::int32_t) noexcept;
    void store_capacity(std::int32_t) noexcept;
    GuiTimedEntryStorage*& slot(std::int32_t) const;
    void require_owned_entry(GuiTimedEntryStorage*) const;
    GuiTimedEntryStorage** allocate_backing(std::uint32_t bytes);
    void free_backing(GuiTimedEntryStorage**);
};
} // namespace bsp
