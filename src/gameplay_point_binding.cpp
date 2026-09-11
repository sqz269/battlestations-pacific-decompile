#include "bsp/gameplay_point_binding.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T read(const void* owner, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(owner) + offset, sizeof value);
    return value;
}
template<class T> T& field(void* owner, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::byte*>(owner) + offset);
}
} // namespace

NativeGameplayEffectDefinitionReference::NativeGameplayEffectDefinitionReference(
    GameplayEffectDefinition& raw, GameplayDefinitionReferences& owner) noexcept
    : RenderCommandReference(raw.references_04()), storage_(raw), owner_(owner) {}
void NativeGameplayEffectDefinitionReference::release_zero_references() noexcept {
    owner_.release_zero(*this); // May delete this; no following owner access.
}
GameplayDefinitionReferences::GameplayDefinitionReferences(
    GameplayEffectDefinitionContext& context, const volatile std::uint32_t* table)
    : context_(context), table_(table) {
    if (!table_) throw std::invalid_argument("Gameplay definition requires its actual D0DA58 table");
}
GameplayDefinitionReferences::~GameplayDefinitionReferences() {
    if (!references_.empty()) std::terminate();
}
void GameplayDefinitionReferences::require_slot(GameplayEffectDefinition& raw,
    std::size_t index, std::uint32_t expected) const {
    if (read<std::uint32_t>(raw.native.data(), 0) != 0x00d0da58u || table_[index] != expected)
        throw std::invalid_argument("Unsupported current gameplay-definition virtual implementation");
}
NativeGameplayEffectDefinitionReference& GameplayDefinitionReferences::bind(
    GameplayEffectDefinition& raw) {
    require_slot(raw, 0, 0x00bd30e0u);
    require_slot(raw, 1, 0x00871440u);
    if (raw.references_04().load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("Gameplay definition binding requires a live actual reference");
    for (auto& reference : references_)
        if (&reference->storage_ == &raw) return *reference;
    auto reference = std::unique_ptr<NativeGameplayEffectDefinitionReference>(
        new NativeGameplayEffectDefinitionReference(raw, *this));
    auto& result = *reference;
    references_.push_back(std::move(reference));
    return result;
}
GameplayEffectDefinition& GameplayDefinitionReferences::definition_for(
    RenderCommandReference& value) const noexcept {
    for (const auto& reference : references_)
        if (reference.get() == &value) return reference->storage_;
    std::terminate();
}
void GameplayDefinitionReferences::release_zero(
    NativeGameplayEffectDefinitionReference& reference) noexcept {
    auto& raw = reference.storage_;
    if (reference.reference_count.load(std::memory_order_relaxed) != 0) std::terminate();
    require_slot(raw, 0, 0x00bd30e0u); // Actual BD30E0 forwards to CURRENT scalar+4(flags1).
    require_slot(raw, 1, 0x00871440u);
    scalar_delete_gameplay_effect_definition_00871440(&raw, 1, context_);
    for (auto it = references_.begin(); it != references_.end(); ++it) {
        if (it->get() == &reference) {
            references_.erase(it); // Deletes only the host companion, after native free.
            return;
        }
    }
    std::terminate();
}

GameplayPointRows::GameplayPointRows(GameplayDefinitionReferences& definitions,
    const GameplayPointComponentTable* tables, std::size_t count,
    GameplayPointReferenceLookup& reference, ForceEventSpatialHost& spatial,
    GameplayPointRemainingComponents& remaining)
    : definitions_(definitions), tables_(tables), table_count_(count), reference_(reference),
      spatial_(spatial), remaining_(remaining) {
    if ((count && !tables) || !reference)
        throw std::invalid_argument("Gameplay point rows require live table and reference bindings");
    for (std::size_t i = 0; i < count; ++i) {
        if (!tables[i].actual_words || tables[i].word_count < 8)
            throw std::invalid_argument("Gameplay component table must expose slots through1C");
        for (std::size_t j = 0; j < i; ++j)
            if (tables[j].original_identity == tables[i].original_identity)
                throw std::invalid_argument("Gameplay component table identity must be unique");
    }
}
EffectAdmissionTemplateView GameplayPointRows::template_rows(RenderCommandReference& value) noexcept {
    void* const raw = definitions_.definition_for(value).native.data();
    return {field<void* const* const>(raw, 8), field<const std::uint32_t>(raw, 0xc)};
}
PointEffectFactoryRowView GameplayPointRows::row_fields(void* raw) noexcept {
    return {field<const std::uint8_t>(raw, 0x10), field<const std::uint8_t>(raw, 0x1c)};
}
EffectAdmissionRowView GameplayPointRows::project_row(void* raw) noexcept {
    return {field<const std::uint8_t>(raw, 0x10), field<const float>(raw, 0x18),
        field<std::uint8_t>(raw, 0x1c)};
}
CameraTransform& GameplayPointRows::reference_e188a8_19fc() noexcept { return reference_(); }
std::uint32_t GameplayPointRows::current_virtual(void* raw, std::size_t index) const {
    const auto identity = read<std::uint32_t>(raw, 0);
    for (std::size_t i = 0; i < table_count_; ++i)
        if (tables_[i].original_identity == identity) return tables_[i].actual_words[index];
    throw std::invalid_argument("Current gameplay component table has no actual binding");
}
std::uint8_t GameplayPointRows::virtual_1c(void* row, EffectPointView point,
    CameraTransform& reference) {
    const auto function = current_virtual(row, 7);
    if (function == 0x0086b7d0u) return admit_effect_component_0086b7d0(row, point, reference);
    return remaining_.admit_current(function, row, point, reference);
}
RenderCommandReference* GameplayPointRows::create_virtual_18(void* row,
    PointEffectInstanceStorage& effect) {
    const auto function = current_virtual(row, 6);
    if (function == 0x0086a820u) return create_point_shake_0086a820(row, &effect, spatial_);
    return remaining_.create_current(function, row, effect);
}
PointEffectRestartRowView GameplayPointRows::restart_fields(void* row) noexcept {
    return {field<const std::uint8_t>(row, 0x10), field<const std::uint32_t>(row, 0x14)};
}
std::uint8_t GameplayPointRows::restart_virtual_08(void* row) {
    const auto function = current_virtual(row, 2);
    if (function == 0x0086b7b0u) return effect_component_restart_false_0086b7b0(row);
    return remaining_.restart_current(function, row);
}

GameplayPointRumbleComponents::GameplayPointRumbleComponents(NativeGamepadForceEvents& events,
    GameplayPointRemainingComponents& remaining) noexcept : events_(events), remaining_(remaining) {}
std::uint8_t GameplayPointRumbleComponents::admit_current(std::uint32_t function, void* row,
    EffectPointView point, CameraTransform& reference) {
    return remaining_.admit_current(function, row, point, reference);
}
std::uint8_t GameplayPointRumbleComponents::restart_current(std::uint32_t function, void* row) {
    return remaining_.restart_current(function, row);
}
RenderCommandReference* GameplayPointRumbleComponents::create_current(std::uint32_t function,
    void* row, PointEffectInstanceStorage& effect) {
    switch (function) {
    case 0x00869010:
    case 0x008690f0:
    case 0x00869290:
        return events_.create(function, row, &effect);
    default: return remaining_.create_current(function, row, effect);
    }
}

GameplayPointChildEvents::GameplayPointChildEvents(NativeGamepadForceEvents& events,
    PointEffectChildEvents& remaining) noexcept : events_(events), remaining_(remaining) {}
PointEffectChildView GameplayPointChildEvents::child_fields(RenderCommandReference& value) noexcept {
    if (auto* event = events_.find(value)) {
        auto& raw = event->storage();
        return {raw.active_0c, raw.event_type_18};
    }
    return remaining_.child_fields(value);
}
void GameplayPointChildEvents::update_virtual_28(RenderCommandReference& value, float delta, void* reference) {
    if (auto* event = events_.find(value)) events_.update(*event, delta, reference);
    else remaining_.update_virtual_28(value, delta, reference);
}
std::uint8_t GameplayPointChildEvents::complete_virtual_08(RenderCommandReference& value) {
    if (auto* event = events_.find(value)) return events_.complete(*event) ? 1 : 0;
    return remaining_.complete_virtual_08(value);
}
void GameplayPointChildEvents::deactivate_virtual_30(RenderCommandReference& value) {
    if (auto* event = events_.find(value)) events_.deactivate(*event);
    else remaining_.deactivate_virtual_30(value);
}

GameplayPointConstruction::GameplayPointConstruction(GameplayDefinitionReferences& definitions,
    GameplayPointRows& rows, PointEffectConstructorBindings& bindings)
    : definitions_(definitions), rows_(rows), bindings_(bindings) {
    if (&bindings.rows != &rows || &rows.definitions_ != &definitions ||
        &bindings.strings != &definitions.context_.strings)
        throw std::invalid_argument("Point construction requires the same gameplay rows, definition and string bindings");
}
PointEffectManagerView GameplayPointConstruction::manager_00866440() {
    auto* const manager = effect_manager_singleton_00866440(
        bindings_.actual_insertion_lock_00f87650, bindings_.insertion_lifetime);
    return {manager->section_04};
}
CameraTransform& GameplayPointConstruction::reference_transform_e188a8_19fc() {
    return rows_.reference_e188a8_19fc();
}
bool GameplayPointConstruction::eligible_0086a650(RenderCommandReference& definition,
    EffectPointView point, CameraTransform& reference) {
    return admit_point_effect_0086a650(rows_.template_rows(definition), point, reference, rows_);
}
void* GameplayPointConstruction::allocate_00bf681b(std::size_t bytes) {
    if (bytes != 0x114) throw std::invalid_argument("Point allocation must be actual114h");
    return singleton_lifetime_allocate({SingletonAllocationKind::object, 0x114,
        sizeof(PointEffectInstanceStorage)});
}
void GameplayPointConstruction::free_00bf65ac(void* raw) noexcept { singleton_lifetime_free(raw); }
RenderCommandReference& GameplayPointConstruction::construct_008680b0(void* raw,
    RenderCommandReference* consumed, CameraTransform* parent, std::uint32_t third_word,
    const CameraMatrix& matrix, std::uint8_t transform_byte, std::uint8_t option_byte,
    std::uint32_t tail_word) {
    // Pure identity/address projection: no extra retain, name read or source
    // snapshot before the complete constructor takes its consumed argument.
    const void* const name = definitions_.definition_for(*consumed).native.data() + 0x1c;
    return construct_point_effect_instance_008680b0(raw, consumed, parent, third_word,
        matrix, transform_byte, option_byte, tail_word, name, bindings_);
}
} // namespace bsp
