#include "bsp/native_gamepad_force_event.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <new>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeGamepadForceEventStorage) == 0x20);
static_assert(sizeof(std::atomic<std::int32_t>) == 4 && std::atomic<std::int32_t>::is_always_lock_free);
static_assert(offsetof(NativeGamepadForceEventStorage, references_04) == 4);
static_assert(offsetof(NativeGamepadForceEventStorage, secondary_table_08) == 8);
static_assert(offsetof(NativeGamepadForceEventStorage, active_0c) == 0xc);
static_assert(offsetof(NativeGamepadForceEventStorage, subject_10) == 0x10);
static_assert(offsetof(NativeGamepadForceEventStorage, definition_14) == 0x14);
static_assert(offsetof(NativeGamepadForceEventStorage, event_type_18) == 0x18);
static_assert(offsetof(NativeGamepadForceEventStorage, request_handle_1c) == 0x1c);
template<class T> const T& field(const void* definition, std::size_t offset) noexcept {
    return *reinterpret_cast<const T*>(static_cast<const std::byte*>(definition) + offset);
}
void prefix(NativeGamepadForceEventStorage& event, const void* definition,
    void* subject, std::uint32_t table, std::uint32_t secondary) noexcept {
    event.table_00 = 0x00ceb130;
    event.references_04.store(1, std::memory_order_relaxed);
    event.secondary_table_08 = 0x00d0c8c0;
    event.active_0c = 1;
    event.subject_10 = subject;
    event.definition_14 = definition;
    event.event_type_18 = 6;
    event.table_00 = table;
    event.secondary_table_08 = secondary;
}
class BaseUnwind final {
public:
    explicit BaseUnwind(NativeGamepadForceEventStorage& event) noexcept : event_(event) {}
    ~BaseUnwind() { if (armed) destroy_native_force_event_base_00872c90(event_); }
    bool armed{true};
private:
    NativeGamepadForceEventStorage& event_;
};
template<class Construct> NativeGamepadForceEventStorage* create_raw(Construct construct) {
    void* const raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x20, 0x20});
    if (!raw) return nullptr;
    auto* const event = ::new (raw) NativeGamepadForceEventStorage;
    try { construct(*event); }
    catch (...) {
        event->~NativeGamepadForceEventStorage();
        singleton_lifetime_free(raw); // AFTER constructor's base-only unwind.
        throw;
    }
    return event;
}
std::uint32_t scalar_for(std::uint32_t table) {
    switch (table) {
    case 0x00d0dcd0: return 0x00873530;
    case 0x00d0dd08: return 0x00873720;
    case 0x00d0dd40: return 0x00873860;
    default: throw std::invalid_argument("Unbound native rumble event class");
    }
}
}

NativeGamepadForceEventStorage& construct_native_constant_force_event_00873450(
    NativeGamepadForceEventStorage& event, const void* definition, void* subject,
    GamepadForceContext& context) {
    prefix(event, definition, subject, 0x00d0dcd0, 0x00d0dccc);
    BaseUnwind unwind(event);
    event.request_handle_1c = submit_constant_force_event_request({field<std::uint32_t>(definition, 0x20),
        field<float>(definition, 0x24), field<float>(definition, 0x2c)}, context);
    unwind.armed = false;
    return event;
}
NativeGamepadForceEventStorage& construct_native_fading_force_event_00873560(
    NativeGamepadForceEventStorage& event, const void* definition, void* subject,
    GamepadForceContext& context, ForceEventSpatialHost& spatial) {
    prefix(event, definition, subject, 0x00d0dd08, 0x00d0dd04);
    BaseUnwind unwind(event);
    event.request_handle_1c = submit_fading_force_event_request({field<std::uint32_t>(definition, 0x20),
        field<float>(definition, 0x24), field<float>(definition, 0x28), field<float>(definition, 0x2c)},
        subject, context, spatial);
    unwind.armed = false;
    return event;
}
NativeGamepadForceEventStorage& construct_native_alternating_force_event_00873750(
    NativeGamepadForceEventStorage& event, const void* definition, void* subject,
    GamepadForceContext& context) {
    prefix(event, definition, subject, 0x00d0dd40, 0x00d0dd3c);
    BaseUnwind unwind(event);
    event.request_handle_1c = submit_alternating_force_event_request({field<std::uint32_t>(definition, 0x20),
        field<float>(definition, 0x24), field<unsigned char>(definition, 0x2c),
        field<float>(definition, 0x30), field<float>(definition, 0x34),
        field<float>(definition, 0x38), field<float>(definition, 0x3c)}, context);
    unwind.armed = false;
    return event;
}
NativeGamepadForceEventStorage* create_native_constant_force_event_00869010(
    const void* definition, void* subject, GamepadForceContext& context) {
    return create_raw([&](auto& event) { construct_native_constant_force_event_00873450(event, definition, subject, context); });
}
NativeGamepadForceEventStorage* create_native_fading_force_event_008690f0(
    const void* definition, void* subject, GamepadForceContext& context, ForceEventSpatialHost& spatial) {
    return create_raw([&](auto& event) { construct_native_fading_force_event_00873560(event, definition, subject, context, spatial); });
}
NativeGamepadForceEventStorage* create_native_alternating_force_event_00869290(
    const void* definition, void* subject, GamepadForceContext& context) {
    return create_raw([&](auto& event) { construct_native_alternating_force_event_00873750(event, definition, subject, context); });
}
void destroy_native_force_event_base_00872c90(NativeGamepadForceEventStorage& event) noexcept {
    event.table_00 = 0x00d0c88c;
    event.secondary_table_08 = 0x00d0c888;
    event.table_00 = 0x00ceb130; // BD30F0. No borrowed pointer/handle/count writes.
}
NativeGamepadForceEventStorage* delete_native_force_event(NativeGamepadForceEventStorage& event,
    std::uint32_t flags) noexcept {
    destroy_native_force_event_base_00872c90(event);
    event.~NativeGamepadForceEventStorage();
    if (flags & 1u) singleton_lifetime_free(&event);
    return &event;
}
bool native_force_event_complete_00872180(NativeGamepadForceEventStorage& event, GamepadForceContext& context) {
    validate_gamepad_force_handle_00a957d0(event.request_handle_1c, context);
    return event.request_handle_1c == 0;
}
void cancel_native_force_event_00872160(NativeGamepadForceEventStorage& event, GamepadForceContext& context) {
    if (event.request_handle_1c) remove_current_gamepad_force_request_00a957f0(event.request_handle_1c, context);
    event.request_handle_1c = 0;
}
void native_force_event_callback_noop_00872150(NativeGamepadForceEventStorage&,
    std::uintptr_t, std::uintptr_t) noexcept {}

NativeGamepadForceEventReference::NativeGamepadForceEventReference(NativeGamepadForceEventStorage& storage,
    NativeGamepadForceEvents& owner) noexcept
    : RenderCommandReference(storage.references_04), storage_(storage), owner_(owner) {}
void NativeGamepadForceEventReference::release_zero_references() noexcept { owner_.release_zero(*this); }
NativeGamepadForceEvents::NativeGamepadForceEvents(GamepadForceContext& context,
    ForceEventSpatialHost& spatial, const NativeGamepadForceEventTable* tables, std::size_t count)
    : context_(context), spatial_(spatial), tables_(tables), table_count_(count) {
    if (!tables || !count) throw std::invalid_argument("Actual rumble event tables required");
    for (std::size_t i = 0; i < count; ++i) {
        scalar_for(tables[i].original_identity);
        if (!tables[i].actual_words || tables[i].word_count < 13)
            throw std::invalid_argument("Incomplete rumble event table");
        for (std::size_t j = 0; j < i; ++j)
            if (tables[i].original_identity == tables[j].original_identity)
                throw std::invalid_argument("Duplicate rumble event table");
    }
}
NativeGamepadForceEvents::~NativeGamepadForceEvents() { if (references_) std::terminate(); }
const NativeGamepadForceEventTable& NativeGamepadForceEvents::table_for(
    const NativeGamepadForceEventStorage& event) const {
    for (std::size_t i = 0; i < table_count_; ++i)
        if (tables_[i].original_identity == event.table_00) return tables_[i];
    throw std::invalid_argument("Current rumble event table is not bound");
}
void NativeGamepadForceEvents::require_slot(const NativeGamepadForceEventStorage& event,
    std::size_t index, std::uint32_t expected) const {
    if (table_for(event).actual_words[index] != expected)
        throw std::invalid_argument("Current rumble event virtual implementation is not bound");
}
NativeGamepadForceEventReference& NativeGamepadForceEvents::bind(NativeGamepadForceEventStorage& event) {
    for (auto* ref = references_; ref; ref = ref->next_)
        if (&ref->storage_ == &event) return *ref;
    if (event.references_04.load() <= 0) throw std::invalid_argument("Native event construction/count required");
    require_slot(event, 0, 0x00bd30e0);
    require_slot(event, 1, scalar_for(event.table_00));
    auto* ref = new NativeGamepadForceEventReference(event, *this);
    ref->next_ = references_;
    references_ = ref;
    return *ref;
}
NativeGamepadForceEventReference* NativeGamepadForceEvents::create(std::uint32_t function,
    const void* definition, void* subject) {
    std::uint32_t identity;
    switch (function) {
    case 0x00869010: identity = 0x00d0dcd0; break;
    case 0x008690f0: identity = 0x00d0dd08; break;
    case 0x00869290: identity = 0x00d0dd40; break;
    default: throw std::invalid_argument("Unknown native rumble event factory");
    }
    const NativeGamepadForceEventTable* profile = nullptr;
    for (std::size_t i = 0; i < table_count_; ++i)
        if (tables_[i].original_identity == identity) profile = &tables_[i];
    if (!profile || profile->actual_words[0] != 0x00bd30e0 ||
        profile->actual_words[1] != scalar_for(identity))
        throw std::invalid_argument("Native rumble factory requires its actual terminal table");
    // No metadata allocation remains after the original factory's return.
    void* const metadata = ::operator new(sizeof(NativeGamepadForceEventReference));
    NativeGamepadForceEventStorage* event;
    try {
        switch (function) {
        case 0x00869010: event = create_native_constant_force_event_00869010(definition, subject, context_); break;
        case 0x008690f0: event = create_native_fading_force_event_008690f0(definition, subject, context_, spatial_); break;
        case 0x00869290: event = create_native_alternating_force_event_00869290(definition, subject, context_); break;
        default: throw std::invalid_argument("Unknown native rumble event factory");
        }
    } catch (...) { ::operator delete(metadata); throw; }
    if (!event) { ::operator delete(metadata); return nullptr; }
    auto* const ref = ::new (metadata) NativeGamepadForceEventReference(*event, *this);
    ref->next_ = references_;
    references_ = ref;
    return ref;
}
void NativeGamepadForceEvents::release_zero(NativeGamepadForceEventReference& ref) noexcept {
    if (&ref.owner_ != this || ref.storage_.references_04.load() != 0) std::terminate();
    require_slot(ref.storage_, 0, 0x00bd30e0);
    require_slot(ref.storage_, 1, scalar_for(ref.storage_.table_00));
    delete_native_force_event(ref.storage_, 1);
    auto** slot = &references_;
    while (*slot && *slot != &ref) slot = &(*slot)->next_;
    if (!*slot) std::terminate();
    *slot = ref.next_;
    delete &ref;
}
bool NativeGamepadForceEvents::complete(NativeGamepadForceEventReference& ref) {
    if (&ref.owner_ != this) throw std::invalid_argument("Wrong rumble event domain");
    require_slot(ref.storage_, 2, 0x00872180);
    return native_force_event_complete_00872180(ref.storage_, context_);
}
void NativeGamepadForceEvents::cancel(NativeGamepadForceEventReference& ref) {
    if (&ref.owner_ != this) throw std::invalid_argument("Wrong rumble event domain");
    require_slot(ref.storage_, 12, 0x00872160);
    cancel_native_force_event_00872160(ref.storage_, context_);
}
NativeGamepadForceEventReference* NativeGamepadForceEvents::find(RenderCommandReference& value) const noexcept {
    for (auto* ref = references_; ref; ref = ref->next_)
        if (ref == &value) return ref;
    return nullptr;
}
void NativeGamepadForceEvents::update(NativeGamepadForceEventReference& ref, float,
    void*) {
    if (&ref.owner_ != this) throw std::invalid_argument("Wrong rumble event domain");
    require_slot(ref.storage_, 10, 0x00872150);
    // Established current virtual+28 is the complete RET8 no-op; no fields read.
    native_force_event_callback_noop_00872150(ref.storage_, 0, 0);
}
void NativeGamepadForceEvents::deactivate(NativeGamepadForceEventReference& ref) {
    if (&ref.owner_ != this) throw std::invalid_argument("Wrong rumble event domain");
    require_slot(ref.storage_, 12, 0x00872160);
    ref.storage_.active_0c = 0;
    cancel_native_force_event_00872160(ref.storage_, context_);
}
std::size_t NativeGamepadForceEvents::binding_count() const noexcept {
    std::size_t count = 0;
    for (auto* ref = references_; ref; ref = ref->next_) ++count;
    return count;
}
} // namespace bsp
