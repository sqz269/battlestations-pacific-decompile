#include "bsp/native_mesh_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(NativeMeshStorage) == 0xbc);
static_assert(sizeof(NativeMeshStorage) == NativeMeshPool::object_bytes);
static_assert(NativeMeshPool::slot_chunk_index_offset == 0xbc);
static_assert(NativeMeshPool::slot_bytes == 0xc0);
static_assert(sizeof(NativeMeshLodPhaseStorage) == 0x10);
static_assert(sizeof(NativeMeshWeightNameStorage) == 8);
static_assert(sizeof(NativeMeshWeightNamesStorage) == 0x0c);
static_assert(offsetof(NativeMeshStorage, references_04) == 4);
static_assert(offsetof(NativeMeshStorage, lod_phases_10) == 0x10);
static_assert(offsetof(NativeMeshStorage, lod_count_50) == 0x50);
static_assert(offsetof(NativeMeshStorage, draw_sections_54) == 0x54);
static_assert(offsetof(NativeMeshStorage, index_stream_60) == 0x60);
static_assert(offsetof(NativeMeshStorage, vertex_streams_64) == 0x64);
static_assert(offsetof(NativeMeshStorage, vertex_stream_count_7c) == 0x7c);
static_assert(offsetof(NativeMeshStorage, flag_80) == 0x80);
static_assert(offsetof(NativeMeshStorage, zero_words_84) == 0x84);
static_assert(offsetof(NativeMeshStorage, flag_94) == 0x94);
static_assert(offsetof(NativeMeshStorage, weight_names_b0) == 0xb0);

namespace {
constexpr std::uint32_t mesh_table = 0x00d62d60;
void retain_raw(void* identity) noexcept {
    auto* actual = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    actual->fetch_add(1, std::memory_order_seq_cst);
}
void assign_raw(void*& field, void* incoming, NativeRenderActualOwners& owners) {
    void* const old = field;
    if (old == incoming) return;
    field = incoming;
    if (incoming) retain_raw(incoming);
    if (old) release_native_render_actual_owner(owners, old);
}
class MeshMemberCleanup final {
public:
    MeshMemberCleanup(NativeMeshStorage& mesh, NativeStringStorage& strings) noexcept
        : mesh_(mesh), strings_(strings) {}
    ~MeshMemberCleanup() { finish(); }
    void finish() {
        if (state_ == 2) {
            state_ = 1;
            destroy_native_mesh_weight_names_00427880(mesh_.weight_names_b0, strings_);
        }
        if (state_ == 1) {
            state_ = 0;
            destroy_native_mesh_section_pointers_00b73cb0(mesh_.draw_sections_54);
        }
        if (state_ == 0) {
            state_ = -1;
            mesh_.native_vtable_00 = 0x00d5c104; // AA6E10 / normal B73F31
            mesh_.native_vtable_00 = 0x00ceb130; // full BD30F0
        }
    }
private:
    NativeMeshStorage& mesh_;
    NativeStringStorage& strings_;
    int state_{2};
};
} // namespace

NativeMeshStorage* construct_native_mesh_00b73d70(void* actual, NativeMeshConstants constants) noexcept {
    const auto minimum = constants.minimum_00ce4adc;
    const auto maximum = constants.maximum_00ce4970;
    // Placement starts typed lifetime, without allowing atomic initialization
    // to erase any unknown payload bytes before the established native stores.
    std::array<std::byte, sizeof(NativeMeshStorage)> preimage;
    std::memcpy(preimage.data(), actual, preimage.size());
    auto* mesh = ::new (actual) NativeMeshStorage;
    std::memcpy(actual, preimage.data(), preimage.size());
    mesh->native_vtable_00 = 0x00ceb130;
    mesh->references_04.store(1, std::memory_order_relaxed);
    mesh->native_vtable_00 = mesh_table;
    mesh->word_08 = 0;
    for (auto& phase : mesh->lod_phases_10) phase.minimum_00 = minimum;
    for (auto& phase : mesh->lod_phases_10) phase.maximum_04 = maximum;
    mesh->lod_count_50 = 0;
    mesh->draw_sections_54.data_00 = nullptr;
    mesh->draw_sections_54.count_04 = 0;
    mesh->draw_sections_54.capacity_08 = 0;
    mesh->index_stream_60 = nullptr;
    mesh->vertex_stream_count_7c = 0;
    mesh->weight_names_b0.data_00 = nullptr;
    mesh->weight_names_b0.count_04 = 0;
    mesh->weight_names_b0.capacity_08 = 0;
    mesh->flag_80 = 0;
    for (auto& word : mesh->zero_words_84) word = 0;
    mesh->lod_bits_0c = constants.lod_00d7a24c;
    mesh->flag_94 = 0;
    return mesh;
}

void reserve_native_mesh_section_pointers_00b72e10(
    NativeRenderPointerArrayStorage& array, std::int32_t requested) {
    if (requested < 4) requested = 4;
    if (array.capacity_08 >= requested) return;
    auto** replacement = static_cast<void**>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, static_cast<std::size_t>(requested) * 4,
        static_cast<std::size_t>(requested) * sizeof(void*)}));
    for (std::int32_t index = 0; index < array.count_04; ++index)
        replacement[index] = array.data_00[index];
    singleton_lifetime_free(array.data_00);
    array.data_00 = replacement; // Hidden by saved B72E5C CALL_RETURN.
    array.capacity_08 = requested;
}
void resize_native_mesh_section_pointers_00b73840(
    NativeRenderPointerArrayStorage& array, std::int32_t requested) {
    if (requested > array.capacity_08) reserve_native_mesh_section_pointers_00b72e10(array, requested);
    for (std::int32_t index = array.count_04; index < requested; ++index)
        array.data_00[index] = nullptr;
    while (requested < array.count_04) --array.count_04;
    array.count_04 = requested;
}
void destroy_native_mesh_section_pointers_00b73cb0(NativeRenderPointerArrayStorage& array) {
    resize_native_mesh_section_pointers_00b73840(array, 0);
    singleton_lifetime_free(array.data_00);
}
void set_native_mesh_index_stream_00b73b70(
    NativeMeshStorage& mesh, NativeRenderActualOwners& owners, void* incoming) {
    assign_raw(mesh.index_stream_60, incoming, owners);
}
void set_native_mesh_vertex_stream_00b73bb0(
    NativeMeshStorage& mesh, NativeRenderActualOwners& owners, std::int32_t index, void* incoming) {
    while (mesh.vertex_stream_count_7c <= index) {
        mesh.vertex_streams_64[mesh.vertex_stream_count_7c] = nullptr;
        ++mesh.vertex_stream_count_7c;
    }
    assign_raw(mesh.vertex_streams_64[index], incoming, owners);
}
void append_native_mesh_draw_section_00b73c60(NativeMeshStorage& mesh, void* section) {
    auto& array = mesh.draw_sections_54;
    if (array.count_04 == array.capacity_08) {
        auto capacity = array.capacity_08 * 2;
        if (capacity <= 4) capacity = 4;
        reserve_native_mesh_section_pointers_00b72e10(array, capacity);
    }
    array.data_00[array.count_04] = section;
    ++array.count_04;
    retain_raw(section);
}
void clear_native_mesh_draw_sections_00b73c10(
    NativeMeshStorage& mesh, NativeRenderActualOwners& owners) {
    for (std::int32_t index = 0; index < mesh.draw_sections_54.count_04; ++index)
        release_native_render_actual_owner(owners, mesh.draw_sections_54.data_00[index]);
    resize_native_mesh_section_pointers_00b73840(mesh.draw_sections_54, 0);
}
void clear_native_mesh_weight_names_00427110_fragment(
    NativeMeshWeightNamesStorage& names, NativeStringStorage& strings) noexcept {
    while (names.count_04 > 0) {
        --names.count_04;
        const auto& entry = names.data_00[names.count_04];
        char* const data = entry.data_04;
        if (data) strings.release(data, entry.length_00 + 1u);
    }
    names.count_04 = 0;
}
void destroy_native_mesh_weight_names_00427880(
    NativeMeshWeightNamesStorage& names, NativeStringStorage& strings) noexcept {
    clear_native_mesh_weight_names_00427110_fragment(names, strings);
    singleton_lifetime_free(names.data_00);
}
void destroy_native_mesh_00b73e60(NativeMeshStorage& mesh, NativeMeshEnvironment& environment) {
    mesh.native_vtable_00 = mesh_table;
    MeshMemberCleanup cleanup(mesh, environment.strings);
    if (void* const index = mesh.index_stream_60) {
        release_native_render_actual_owner(environment.retained_owners, index);
        mesh.index_stream_60 = nullptr;
    }
    void** const end = mesh.vertex_streams_64 + mesh.vertex_stream_count_7c;
    for (void** current = mesh.vertex_streams_64; current != end; ++current)
        if (void* const stream = *current)
            release_native_render_actual_owner(environment.retained_owners, stream);
    clear_native_mesh_draw_sections_00b73c10(mesh, environment.retained_owners);
    cleanup.finish();
}
NativeMeshStorage* delete_native_mesh_00b74280(
    NativeMeshStorage* mesh, NativeMeshEnvironment& environment, std::uint32_t flags) {
    destroy_native_mesh_00b73e60(*mesh, environment);
    if (flags & 1) environment.pool_0108fff8.return_slot_00b72da0(mesh);
    return mesh;
}

NativeMeshReference::NativeMeshReference(NativeMeshStorage& mesh,
    NativeMeshEnvironment& environment, NativeMeshCompanionDisposal disposal)
    : RenderCommandReference(mesh.references_04), storage_(mesh), environment_(environment), disposal_(disposal) {
    if (!disposal.retire || mesh.references_04.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("mesh companion requires a live actual owner and explicit retirement");
    require_current_profile();
}
NativeMeshReference::~NativeMeshReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeMeshReference::require_current_profile() const noexcept {
    const auto* table = environment_.vtable_00d62d60;
    if (storage_.native_vtable_00 != mesh_table || !table ||
        table[0] != 0x00bd30e0 || table[1] != 0x00b74280) std::terminate();
}
void NativeMeshReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_current_profile();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    delete_native_mesh_00b74280(&storage_, environment_, 1);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
} // namespace bsp
