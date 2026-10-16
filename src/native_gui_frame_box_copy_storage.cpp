#include "bsp/native_gui_frame_box_copy_storage.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <array>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

static_assert(sizeof(void*) == 4 && sizeof(std::size_t) == 4);

namespace bsp {
namespace {
using Word = std::uint32_t;
using Operation = NativeGuiFrameBoxCopyStorageOperation;
using Phase = NativeGuiFrameBoxCopyStoragePhase;
using Services = NativeGuiFrameBoxCopyStorageServices;
using Frames = NativeGuiFrameBoxCopyStorageFrames;
struct Region { std::uintptr_t first, last; };
Region region(const volatile void* pointer, std::size_t size) {
    const auto first = reinterpret_cast<std::uintptr_t>(pointer);
    if ((first & 3u) != 0 || (size != 0 && first == 0) ||
        size > (std::numeric_limits<std::uintptr_t>::max)() - first)
        throw std::invalid_argument("FrameBox copy requires aligned nonwrapping live regions");
    return {first, first + size};
}
bool overlap(Region a, Region b) noexcept {
    return a.first < b.last && b.first < a.last;
}
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void* at(void* p, Word offset) noexcept {
    return static_cast<std::byte*>(p) + offset;
}
const void* at(const void* p, Word offset) noexcept {
    return static_cast<const std::byte*>(p) + offset;
}
volatile Word& word(void* p, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
template<class T> T* object(std::byte* storage) noexcept {
    return std::launder(reinterpret_cast<T*>(storage));
}

void* validate_preparation(Operation* metadata, void* destination, const void* source,
    Services& s, Frames& f, NativeGuiFrameBoxVectorCopyOperation& vector,
    NativeGuiFrameBoxRecordReleaseOperation* records, std::size_t capacity) {
    if (capacity > (std::numeric_limits<std::size_t>::max)() / sizeof(records[0]))
        throw std::invalid_argument("FrameBox copy destruction metadata extent wraps");
    const std::array<Region, 20> control{{region(destination, 0x120), region(source, 0x120),
        region(metadata, sizeof(*metadata)), region(&s, sizeof(s)), region(&f, sizeof(f)),
        region(&vector, sizeof(vector)), region(records, capacity * sizeof(records[0])),
        region(&f.state, sizeof(f.state)), region(&f.constructed, sizeof(f.constructed)),
        region(&f.base_acquired, sizeof(f.base_acquired)), region(&f.state_acquired, sizeof(f.state_acquired)),
        region(&f.constructed_acquired, sizeof(f.constructed_acquired)),
        region(&f.state.locals, sizeof(f.state.locals)), region(&f.state.argument_slot, sizeof(Word)),
        region(&f.state.geometry, sizeof(f.state.geometry)), region(&f.state.transform, sizeof(f.state.transform)),
        region(&f.state.clip_storage, sizeof(f.state.clip_storage)),
        region(&f.constructed.locals, sizeof(f.constructed.locals)),
        region(&f.constructed.transform, sizeof(f.constructed.transform)),
        region(&f.constructed.bounds, sizeof(f.constructed.bounds))}};
    for (std::size_t i = 0; i != control.size(); ++i)
        for (std::size_t j = i + 1; j != control.size(); ++j)
            if (overlap(control[i], control[j]))
                throw std::invalid_argument("FrameBox copy known control/raw regions overlap");
    const std::array<Region, 16> providers{{region(&s.base, sizeof(s.base)),
        region(&s.widgets, sizeof(s.widgets)), region(&s.constructed, sizeof(s.constructed)),
        region(&s.constructed.owners, sizeof(s.constructed.owners)),
        region(&s.materials, sizeof(s.materials)), region(&s.effects, sizeof(s.effects)),
        region(&s.strings, sizeof(s.strings)), region(&s.string_process, sizeof(s.string_process)),
        region(&s.renderer, sizeof(s.renderer)), region(&s.layout, sizeof(s.layout)),
        region(&s.mapping, sizeof(s.mapping)), region(&s.geometry_storage, sizeof(s.geometry_storage)),
        region(&s.clip, sizeof(s.clip)), region(&s.matching_pool, sizeof(s.matching_pool)),
        region(s.actual_profile_00d5d130, 0x84),
        region(&s.base.clone_flags_00d5c0b8, sizeof(s.base.clone_flags_00d5c0b8))}};
    for (const auto provider : providers)
        for (const auto raw_or_control : control)
            if (overlap(provider, raw_or_control))
                throw std::invalid_argument("FrameBox copy control aliases known provider storage");
    require(vector.phase() == NativeGuiFrameBoxVectorCopyPhase::fresh &&
        vector.destination_header() == at(destination, 0xf0) &&
        vector.source_header() == at(source, 0xf0), "FrameBox copy requires its exact fresh138 operation");
    require(!f.base_acquired.started && !f.base_acquired.complete &&
        !f.base_acquired.model_clone.started && !f.state_acquired.started &&
        !f.state_acquired.complete && !f.state_acquired.failed &&
        !f.constructed_acquired.started && !f.constructed_acquired.complete &&
        !f.constructed_acquired.failed, "FrameBox copy requires fresh external acquisitions");
    require(&s.base.model_clone.access.geometry == &s.constructed.geometry &&
        &s.base.model_clone.access.models == &s.constructed.models &&
        &s.constructed.geometry.actual_owners() == &s.constructed.owners &&
        &s.materials.retained_owners == &s.constructed.owners,
        "FrameBox copy requires the SAME base/model/geometry/render domains");
    for (std::size_t i = 0; i != capacity; ++i)
        require(records[i].phase() == NativeGuiFrameBoxRecordReleasePhase::fresh,
            "FrameBox copy requires fresh distinct destruction111 metadata");
    return destination; // No raw header/profile/count/payload read above.
}
} // namespace

Operation::NativeGuiFrameBoxCopyStorageOperation(void* destination, const void* source,
    Services& services, Frames& frames, NativeGuiFrameBoxVectorCopyOperation& vector,
    NativeGuiFrameBoxRecordReleaseOperation* records, std::size_t capacity)
    : destination_(validate_preparation(this, destination, source, services, frames, vector, records, capacity)),
      source_(source), services_(services), frames_(frames), vector_(vector), records_(records), capacity_(capacity),
      admission_(services.constructed.owners.prepare_binding(destination)) {}

NativeGuiFrameBoxVectorTidyOperation* Operation::tidy() noexcept {
    return object<NativeGuiFrameBoxVectorTidyOperation>(tidy_storage_);
}
NativeGuiFrameBoxLifetimeContext* Operation::lifetime() noexcept {
    return object<NativeGuiFrameBoxLifetimeContext>(lifetime_storage_);
}
NativeGuiFrameBoxIdentityContext* Operation::identity_context() noexcept {
    return object<NativeGuiFrameBoxIdentityContext>(identity_storage_);
}
NativeGuiFrameBoxStateStorageContext* Operation::state_context() noexcept {
    return object<NativeGuiFrameBoxStateStorageContext>(state_storage_);
}
NativeGuiFrameBoxReference* Operation::canonical_reference() noexcept {
    return reference_constructed_ ? object<NativeGuiFrameBoxReference>(reference_storage_) : nullptr;
}
const NativeGuiFrameBoxVectorTidyOperation* Operation::tidy_operation() const noexcept {
    return tidy_constructed_ ? std::launder(
        reinterpret_cast<const NativeGuiFrameBoxVectorTidyOperation*>(tidy_storage_)) : nullptr;
}

Operation::~NativeGuiFrameBoxCopyStorageOperation() noexcept {
    if (phase_ == Phase::entered || phase_ == Phase::failed) std::terminate();
    if (reference_constructed_ && !canonical_reference()->retired()) std::terminate();
    if (tidy_constructed_ && (tidy()->phase() == NativeGuiFrameBoxVectorTidyPhase::entered ||
        tidy()->phase() == NativeGuiFrameBoxVectorTidyPhase::failed)) std::terminate();
    if (state_context_constructed_) state_context()->~NativeGuiFrameBoxStateStorageContext();
    // Retired163 AND its disengaged-optional borrowed base inspect metadata only.
    if (reference_constructed_) canonical_reference()->~NativeGuiFrameBoxReference();
    if (identity_context_constructed_) identity_context()->~NativeGuiFrameBoxIdentityContext();
    if (lifetime_constructed_) lifetime()->~NativeGuiFrameBoxLifetimeContext();
    if (tidy_constructed_) tidy()->~NativeGuiFrameBoxVectorTidyOperation();
    // admission_ cancels only an unpublished opaque entry; no external frame or
    // vector/acquisition/clip/creator destructor is run by this operation.
}

void Operation::prepare_tidy() {
    require(vector_.phase() == NativeGuiFrameBoxVectorCopyPhase::returned,
        "FrameBox copy requires genuinely returned138 before119 preparation");
    void* first = nullptr;
    void* last = nullptr;
    std::size_t count = 0;
    if (vector_.initial_count() != 0) {
        const auto* const copied = vector_.copy_operation();
        require(copied != nullptr && copied->phase() == NativeGuiFrameBoxRecordCopyPhase::returned,
            "FrameBox copy has no returned128 constructed span");
        first = copied->destination_first();
        last = copied->committed_end();
        count = copied->committed_records();
        require(first == vector_.allocation_identity() && first == vector_.captured_destination(),
            "FrameBox copy span no longer denotes its SAME returned allocation");
    }
    require(count <= capacity_, "FrameBox copy constructed span exceeds prepared destruction111 capacity");
    ::new (static_cast<void*>(tidy_storage_)) NativeGuiFrameBoxVectorTidyOperation(
        at(destination_, 0xf0), first, last, records_, count);
    tidy_constructed_ = true;
}

void Operation::bind_ready() {
    auto& s = services_;
    ::new (static_cast<void*>(lifetime_storage_)) NativeGuiFrameBoxLifetimeContext{
        s.widgets, s.strings, s.constructed.owners, *tidy()};
    lifetime_constructed_ = true;
    ::new (static_cast<void*>(identity_storage_)) NativeGuiFrameBoxIdentityContext{
        s.constructed.owners, *lifetime(), s.matching_pool, s.actual_profile_00d5d130};
    identity_context_constructed_ = true;
    ::new (static_cast<void*>(reference_storage_)) NativeGuiFrameBoxReference(
        destination_, *identity_context(), std::move(admission_));
    reference_constructed_ = true;
    ::new (static_cast<void*>(state_storage_)) NativeGuiFrameBoxStateStorageContext{
        s.constructed.geometry, s.materials, s.effects, s.strings, s.string_process,
        s.constructed.owners, *canonical_reference(), s.constructed.data, s.renderer,
        s.layout, s.mapping, s.geometry_storage, s.constructed.position,
        s.constructed.dispatch, s.clip, s.increment_00ce221c, s.constructed.decrement_00ce2220};
    state_context_constructed_ = true;
}

void Operation::unwind_members() noexcept {
    unwind_entered_ = true;
    try {
        if (cleanup_state_ == 2) {
            cleanup_state_ = 1; // Consume BEFORE current-name41DD20.
            active_call_site_ = 0x00cb92df;
            destroy_native_string_header_0041dd20(at(destination_, 0x104), services_.strings);
        }
        if (cleanup_state_ == 1) {
            cleanup_state_ = 0; // Consume BEFORE genuine SAME119 tail.
            active_call_site_ = 0x00cb92d1;
            if (!tidy_constructed_) std::terminate();
            cleanup_native_gui_frame_box_vector_00ad2320(at(destination_, 0xf0),
                services_.constructed.owners, services_.strings, *tidy());
        }
        if (cleanup_state_ == 0) {
            cleanup_state_ = -1; // Consume BEFORE genuine base, no125 wrapper.
            active_call_site_ = 0x00cb92c3;
            destroy_native_gui_widget_base_00aa9730(destination_, services_.widgets);
        }
        // Saved metadata only, AFTER every applicable genuine action returned.
        if (reference_constructed_) canonical_reference()->retire_after_copy_unwind();
        unwind_returned_ = true;
    } catch (...) { std::terminate(); }
}

void* copy_native_gui_frame_box_storage_00ad27a0(void* destination,
    const void* source, Operation& operation) {
    if (operation.phase_ != Phase::fresh || destination != operation.destination_ || source != operation.source_)
        throw std::logic_error("FrameBox copy requires its fresh exact-identity operation");
    (void)validate_preparation(&operation, destination, source, operation.services_, operation.frames_,
        operation.vector_, operation.records_, operation.capacity_);
    operation.phase_ = Phase::entered;
    auto& s = operation.services_;
    auto& f = operation.frames_;
    try {
        operation.active_call_site_ = 0x00ad27c4;
        construct_native_gui_widget_copy_00aa9520(destination, source, s.base, f.base_acquired);
        word(destination) = 0x00d5d130u;
        const auto state = *static_cast<const volatile std::uint16_t*>(at(source, 0xec));
        operation.cleanup_state_ = 0;
        *static_cast<volatile std::uint16_t*>(at(destination, 0xec)) = state;
        operation.active_call_site_ = 0x00ad27f0;
        construct_native_gui_frame_box_vector_copy_00ad2230(at(destination, 0xf0), at(source, 0xf0),
            s.strings, s.constructed.owners, operation.vector_);
        operation.prepare_tidy(); // Metadata-only; no early raw header/payload read.
        *static_cast<volatile std::uint8_t*>(at(destination, 0x100)) =
            *static_cast<const volatile std::uint8_t*>(at(source, 0x100));
        word(destination, 0x104) = 0;
        word(destination, 0x108) = 0;
        __asm {
            mov ecx, source
            mov edx, destination
            fld dword ptr [ecx + 010ch]
            fstp dword ptr [edx + 010ch]
        }
        operation.cleanup_state_ = 2;
        __asm {
            mov ecx, source
            mov edx, destination
            fld dword ptr [ecx + 0110h]
            fstp dword ptr [edx + 0110h]
            fld dword ptr [ecx + 0114h]
            fstp dword ptr [edx + 0114h]
            fld dword ptr [ecx + 0118h]
            fstp dword ptr [edx + 0118h]
        }
        operation.bind_ready(); // All reached fields are now ready; still NOT complete.
        const Word incoming = *static_cast<const volatile std::uint16_t*>(at(destination, 0xec));
        auto* const table = reinterpret_cast<const volatile Word*>(word(destination));
        require(table == s.actual_profile_00d5d130,
            "FrameBox copy current80 requires its genuine current D5D130 profile");
        const Word target = table[0x80 / 4];
        operation.target80_ = target;
        require(target == 0x00ad0d80u, "FrameBox copy has no provider for its current80 target");
        f.state.argument_slot = incoming; // Zeroextended WORD, new Source argument backing.
        operation.active_call_site_ = 0x00ad2854;
        rebuild_native_gui_frame_box_state_storage_00ad0d80(destination, f.state,
            *operation.state_context(), f.state_acquired);
        *static_cast<volatile std::uint8_t*>(at(destination, 0x74)) = 0;
        operation.active_call_site_ = 0x00ad285b;
        constructed_native_gui_frame_box_storage_00acf8f0(destination, f.constructed,
            s.constructed, f.constructed_acquired);
        operation.phase_ = Phase::returned; // Normal completion provenance, metadata ONLY.
        return destination;
    } catch (...) {
        operation.phase_ = Phase::failed;
        operation.unwind_members();
        operation.phase_ = Phase::failed_unwind_returned;
        throw;
    }
}
} // namespace bsp
