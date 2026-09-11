#pragma once

#include "bsp/gamepad_force_events.hpp"
#include "bsp/render_command_queue.hpp"

#include <atomic>
#include <cstddef>

namespace bsp {

// Actual20h storage. Default initialization intentionally leaves the handle
// and padding untouched; native construction writes the handle only on return
// from A95BF0. The actual atomic+04 is the sole event ownership counter.
struct NativeGamepadForceEventStorage {
    std::uint32_t table_00;
    std::atomic<std::int32_t> references_04;
    std::uint32_t secondary_table_08;
    std::uint8_t active_0c;
    std::byte untouched_0d[3];
    void* subject_10;
    const void* definition_14;
    std::uint32_t event_type_18;
    std::uint32_t request_handle_1c;
};

// ECX=event, stack(definition,subject), RET8. Definitions are the existing
// raw component owners (30h/30h/40h), not copied parameter structs.
NativeGamepadForceEventStorage& construct_native_constant_force_event_00873450(
    NativeGamepadForceEventStorage&, const void* definition, void* subject, GamepadForceContext&);
NativeGamepadForceEventStorage& construct_native_fading_force_event_00873560(
    NativeGamepadForceEventStorage&, const void* definition, void* subject,
    GamepadForceContext&, ForceEventSpatialHost&);
NativeGamepadForceEventStorage& construct_native_alternating_force_event_00873750(
    NativeGamepadForceEventStorage&, const void* definition, void* subject, GamepadForceContext&);

// ECX=definition, stack subject, RET4; actual20h allocation and constructor
// unwind/free. Borrowed definition/subject are neither retained nor released.
NativeGamepadForceEventStorage* create_native_constant_force_event_00869010(
    const void*, void*, GamepadForceContext&);
NativeGamepadForceEventStorage* create_native_fading_force_event_008690f0(
    const void*, void*, GamepadForceContext&, ForceEventSpatialHost&);
NativeGamepadForceEventStorage* create_native_alternating_force_event_00869290(
    const void*, void*, GamepadForceContext&);

void destroy_native_force_event_base_00872c90(NativeGamepadForceEventStorage&) noexcept;
// Identical bodies873530/873720/873860: restore base tables; bit0 free. No cancel.
NativeGamepadForceEventStorage* delete_native_force_event(
    NativeGamepadForceEventStorage&, std::uint32_t flags) noexcept;
bool native_force_event_complete_00872180(NativeGamepadForceEventStorage&, GamepadForceContext&);
void cancel_native_force_event_00872160(NativeGamepadForceEventStorage&, GamepadForceContext&);
void native_force_event_callback_noop_00872150(NativeGamepadForceEventStorage&,
    std::uintptr_t, std::uintptr_t) noexcept;

struct NativeGamepadForceEventTable {
    std::uint32_t original_identity;
    const volatile std::uint32_t* actual_words; // At least13 words, through+30.
    std::size_t word_count;
};
class NativeGamepadForceEvents;
class NativeGamepadForceEventReference final : public RenderCommandReference {
public:
    NativeGamepadForceEventStorage& storage() noexcept { return storage_; }
    void release_zero_references() noexcept override;
private:
    friend class NativeGamepadForceEvents;
    NativeGamepadForceEventReference(NativeGamepadForceEventStorage&,
        NativeGamepadForceEvents&) noexcept;
    NativeGamepadForceEventStorage& storage_;
    NativeGamepadForceEvents& owner_;
    NativeGamepadForceEventReference* next_{}; // HOST companion list only.
};

// One canonical companion domain; all bound event terminals use it. Supplied
// immutable bindings borrow current D0DCD0/D0DD08/D0DD40 table words. Requests
// use the application's existing device/context and actual spatial associations.
class NativeGamepadForceEvents final {
public:
    NativeGamepadForceEvents(GamepadForceContext&, ForceEventSpatialHost&,
        const NativeGamepadForceEventTable*, std::size_t count);
    ~NativeGamepadForceEvents(); // Requires all native owners retired.
    NativeGamepadForceEvents(const NativeGamepadForceEvents&) = delete;
    NativeGamepadForceEvents& operator=(const NativeGamepadForceEvents&) = delete;
    // First bind may allocate HOST metadata; on failure raw ownership is unchanged.
    NativeGamepadForceEventReference& bind(NativeGamepadForceEventStorage&);
    // Prepare HOST metadata before entering the native factory, so no new
    // allocation/failure boundary follows its successful request submission.
    NativeGamepadForceEventReference* create(std::uint32_t factory_function,
        const void* actual_definition, void* actual_subject);
    bool complete(NativeGamepadForceEventReference&);
    void cancel(NativeGamepadForceEventReference&);
    std::size_t binding_count() const noexcept;
private:
    friend class NativeGamepadForceEventReference;
    const NativeGamepadForceEventTable& table_for(const NativeGamepadForceEventStorage&) const;
    void require_slot(const NativeGamepadForceEventStorage&, std::size_t, std::uint32_t) const;
    void release_zero(NativeGamepadForceEventReference&) noexcept;
    GamepadForceContext& context_;
    ForceEventSpatialHost& spatial_;
    const NativeGamepadForceEventTable* tables_;
    std::size_t table_count_;
    NativeGamepadForceEventReference* references_{};
};
// Native bytes and ownership, with new C++ dispatch/request interfaces. Not a
// drop-in original vtable/SEH ABI or game validation.
} // namespace bsp
