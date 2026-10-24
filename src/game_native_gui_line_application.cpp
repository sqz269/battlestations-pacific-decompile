#include "bsp/game_native_gui_line_application.hpp"
#include "bsp/game_native_gui_effect_application.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/gui_text_content.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/gui_text_native_layout.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Line composition requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
using Word = std::uint32_t;
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
template<class T> T read(const void* actual, Word offset = 0) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(actual) + offset, sizeof value);
    return value;
}
template<class T> void write(void* actual, Word offset, const T& value) {
    std::memcpy(static_cast<std::byte*>(actual) + offset, &value, sizeof value);
}
Word address(const void* actual) { return static_cast<Word>(reinterpret_cast<std::uintptr_t>(actual)); }
void* pointer(Word bits) { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits)); }
std::atomic<std::int32_t>& count(void* actual) {
    require(actual && !(address(actual) & 3u), "native Line requires aligned actual storage");
    return *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(actual) + 4));
}
// AC2B5A..2B8F: two separate FLD/FSTP32 passes and X/Y source order.
void write_position(float* output, const float* point_x, const float* point_y,
    const float* pivot_x, const float* pivot_y, const volatile float* y_scale) {
    float x, y;
    __asm {
        mov eax, point_x
        mov edx, pivot_x
        fld dword ptr [eax]
        fsub dword ptr [edx]
        fstp x
        mov eax, point_y
        mov edx, y_scale
        fld dword ptr [eax]
        fmul dword ptr [edx]
        mov edx, pivot_y
        fsub dword ptr [edx]
        fstp y
        mov eax, output
        fld x
        fstp dword ptr [eax]
        xorps xmm1, xmm1
        movss dword ptr [eax + 8], xmm1
        fld y
        fstp dword ptr [eax + 4]
    }
}
} // namespace

struct GameNativeGuiLineApplication::Impl {
    struct Operation {
        Word site{};
        bool complete{};
        NativeString format_name, material_name;
        bool format_name_live{}, material_name_live{};
        GuiNativeDeclarationAcquired declaration;
        NativeStreamCloneAcquired vertex, index;
        GuiNativeSectionAcquired acquired_section;
        NativeMeshStorage* mesh{};
        NativeMeshSectionStorage* section{};
        void* owned_material{};
        void* material{}; // Borrow after section publication/release of creator.
        const GameNativeGuiMaterialFactoryOperation* material_factory{};
        void* old_vertex{};
        void* old_index{};
        void* source_map{};
        void* destination_map{};
        NativeStreamCloneMapPhase source_phase{NativeStreamCloneMapPhase::not_called};
        NativeStreamCloneMapPhase destination_phase{NativeStreamCloneMapPhase::not_called};
        void* flush_vertex{};
        void* flush_index{};
        void* flush_index_map{};
        void* flush_vertex_map{};
        NativeStreamCloneMapPhase flush_index_phase{NativeStreamCloneMapPhase::not_called};
        NativeStreamCloneMapPhase flush_vertex_phase{NativeStreamCloneMapPhase::not_called};
        std::exception_ptr failure;
    };
    struct Record {
        GameNativeGuiLineIdentity identity;
        std::unique_ptr<Operation> operation;
        bool constructed{}, guard_released{};
        explicit Record(GameNativeGuiLineIdentity supplied) : identity(std::move(supplied)) {}
    };
    GameNativeRendererApplication& renderer;
    GameNativeGuiEffectApplication& effects;
    GameNativeGuiLineApplicationBindings binding;
    NativeGameGridContext& grid;
    GuiNativeGeometryOwners& geometry;
    GuiTextNativeLayoutServices& layouts;
    std::unordered_map<GuiLayoutWidget*, std::unique_ptr<Record>> records;
    bool active{}, failed{};
    Word site{};
    std::exception_ptr failure;

    Impl(GameNativeRendererApplication& app, GameNativeGuiEffectApplication& shaders,
        GameNativeGuiLineApplicationBindings supplied)
        : renderer(app), effects(shaders), binding(std::move(supplied)),
          grid(app.game_grid_context()), geometry(grid.graphics.streams.geometry),
          layouts(app.section_layout_services()) {
        require(binding.actual_identity && binding.initial_manager_texture_28 &&
            &geometry.actual_owners() == &renderer.actual_owners() &&
            &binding.widgets.environment().models.retained_owners == &renderer.actual_owners() &&
            &binding.colors.widgets == &binding.widgets &&
            &binding.colors.actual_owners == &renderer.actual_owners(),
            "native Line requires canonical identity/resource/widget/renderer services");
    }
    void require_idle() const { require(!active && !failed, "native Line cannot reenter/replay interrupted work"); }
    void validate(Record& record, bool need_model = true) {
        auto& identity = record.identity;
        auto& owner = identity.widget;
        require(!record.guard_released && identity.page_token && identity.page_token.get() == &owner &&
            owner.layout().type == GuiWidgetType::Line &&
            &owner.runtime() == &binding.widgets && &binding.widgets.owner(owner.layout()) == &owner &&
            read<Word>(identity.actual) == 0x00d5c9e8 &&
            &identity.reference.reference_count == &count(identity.actual) &&
            identity.reference.reference_count.load() > 0 &&
            &renderer.actual_owners().resolve_actual(identity.actual) == &identity.reference,
            "native Line requires its same canonical live raw104h page allocation");
        if (need_model) (void)model(record);
    }
    Record& record(GuiWidgetOwner& owner) {
        require_idle();
        const auto found = records.find(&owner.layout());
        if (found != records.end()) { validate(*found->second); return *found->second; }
        auto identity = binding.actual_identity(owner); // Pure authority admission, before GPU effects.
        require(&identity.widget == &owner, "native Line authority returned a different owner");
        auto value = std::make_unique<Record>(std::move(identity));
        validate(*value);
        auto* result = value.get();
        records.emplace(&owner.layout(), std::move(value));
        return *result;
    }
    NativeModelOwner& model(Record& record) {
        auto& owner = record.identity.widget;
        auto* reference = owner.model_reference();
        require(reference && owner.node_binding() == &reference->model_owner().node &&
            reference->model_owner().phase == NativeModelOwner::Phase::live &&
            &reference->model_owner().environment.retained_owners == &renderer.actual_owners() &&
            read<void*>(record.identity.actual, 0x4c) == &reference->model_owner().storage.node &&
            &renderer.actual_owners().resolve_actual(&reference->model_owner().storage.node) == reference,
            "native Line raw4C must be the same live owner Model");
        return reference->model_owner();
    }
    NativeMeshStorage& mesh(Record& record) {
        auto& primary = model(record);
        void* actual = gui_model_geometry_00b74640(primary.storage.model, 0);
        auto* reference = actual ? dynamic_cast<NativeMeshReference*>(
            &renderer.actual_owners().resolve_actual(actual)) : nullptr;
        require(reference && &reference->storage() == actual && reference->reference_count.load() > 0,
            "native Line requires its same actual mesh");
        return reference->storage();
    }
    NativeMeshSectionStorage& section(NativeMeshStorage& mesh) {
        auto* actual = static_cast<NativeMeshSectionStorage*>(gui_geometry_element_00b732c0(&mesh, 0));
        auto* reference = actual ? dynamic_cast<NativeMeshSectionReference*>(
            &renderer.actual_owners().resolve_actual(actual)) : nullptr;
        require(reference && &reference->storage() == actual && reference->reference_count.load() > 0,
            "native Line requires canonical actual section0");
        return *actual;
    }
    NativeMaterialStorage& material(void* actual) {
        auto* reference = actual ? dynamic_cast<NativeMaterialReference*>(
            &renderer.actual_owners().resolve_actual(actual)) : nullptr;
        require(reference && &reference->storage() == actual && reference->reference_count.load() > 0 &&
            reference->storage().vtable_00 == 0x00d5e520,
            "native Line requires its actual canonical material");
        return reference->storage();
    }
    struct Entry {
        Impl& app;
        Record& record;
        Entry(Impl& value, Record& retained, Word entry) : app(value), record(retained) {
            app.require_idle();
            require(!record.operation || record.operation->complete, "native Line operation cannot be replayed");
            record.operation = std::make_unique<Operation>(); // Stable before all native effects.
            record.operation->site = app.site = entry;
            app.active = true;
        }
        ~Entry() { app.active = false; }
        Operation& operation() { return *record.operation; }
        void complete() { record.operation->complete = true; }
    };
    void fail(Operation& operation) noexcept {
        operation.failure = failure = std::current_exception(); failed = true;
    }
    void at(Operation& operation, Word value) noexcept { operation.site = site = value; }
    void format(Operation& operation) {
        require(!operation.format_name_live && !operation.declaration.reference,
            "native Line format acquisition must be fresh");
        operation.format_name_live = true;
        operation.format_name.assign_0041e870(grid.strings, "SimpleColor.mvfm");
        load_gui_text_native_declaration_current38(renderer.publication_00f8d394(),
            operation.format_name, grid.graphics, operation.declaration);
        operation.format_name_live = false; // Consume before throwing raw release.
        destroy_native_string_header_0041dd20(&operation.format_name, grid.strings);
    }
    void release_stream(NativeStreamCloneAcquired& acquired) {
        void* actual = acquired.creator;
        acquired.creator = nullptr; acquired.companion = nullptr;
        acquired.phase = NativeStreamClonePhase::consumed;
        release_native_render_actual_owner(renderer.actual_owners(), actual);
    }
    void release_declaration(Operation& operation) {
        void* actual = operation.declaration.reference;
        operation.declaration = {};
        release_native_render_actual_owner(renderer.actual_owners(), actual);
    }
    void* vertex(NativeMeshStorage& current) {
        void* actual = native_mesh_vertex_stream_00b73260(current, 0);
        auto* reference = actual ? dynamic_cast<NativeLogicalVertexReference*>(
            &renderer.actual_owners().resolve_actual(actual)) : nullptr;
        require(reference && reference->storage() == actual && read<Word>(actual) == 0x00d61d6c,
            "native Line requires canonical actual vertex stream0");
        return actual;
    }
    void* index(NativeMeshStorage& current) {
        void* actual = current.index_stream_60;
        auto* reference = actual ? dynamic_cast<NativeLogicalIndexReference*>(
            &renderer.actual_owners().resolve_actual(actual)) : nullptr;
        require(reference && reference->storage() == actual && read<Word>(actual) == 0x00d61de0,
            "native Line requires canonical actual index stream");
        return actual;
    }
    std::int32_t point_count(Record& record) {
        void* actual = record.identity.actual;
        const Word first = read<Word>(actual, 0xf0), last = read<Word>(actual, 0xf4), end = read<Word>(actual, 0xf8);
        require(first <= last && last <= end && ((last - first) & 7u) == 0 &&
            ((end - first) & 7u) == 0 && (first || (!last && !end)),
            "native Line requires its valid same raw point-vector descriptor");
        return static_cast<std::int32_t>(last - first) >> 3;
    }
    void clear_points(Record& record) {
        (void)point_count(record);
        // Native erase(begin,end), valid full range: zero copies, keep allocation/proxy.
        write(record.identity.actual, 0xf4, read<Word>(record.identity.actual, 0xf0));
    }
    void create(Record& record, Operation& operation);
    void resize(Record& record, Operation& operation, std::int32_t requested);
    void flush(Record& record, Operation& operation);
};

void GameNativeGuiLineApplication::Impl::create(Record& record, Operation& operation) {
    require(!record.constructed, "native Line current74 is a single constructor continuation");
    // AC244A..24A8 is the same established mesh/sentinel/setter/release fragment.
    geometry.construct_and_associate74_fragment(model(record));
    operation.mesh = &mesh(record);
    require(operation.mesh->draw_sections_54.count_04 == 0,
        "native Line current74 requires the fresh mesh it just constructed");
    at(operation, 0x00ac24aa);
    format(operation);
    at(operation, 0x00ac2530);
    void* vertices = create_gui_text_native_vertex_current5c(renderer.publication_00f8d394(),
        4, 1, operation.declaration.reference, grid.graphics, operation.vertex);
    at(operation, 0x00ac253a);
    set_native_mesh_vertex_stream_00b73bb0(*operation.mesh, geometry.actual_owners(), 0, vertices);
    at(operation, 0x00ac2550);
    void* indices = create_gui_text_native_index_current60(renderer.publication_00f8d394(),
        8, 1, 0x65, grid.graphics, operation.index);
    at(operation, 0x00ac2557);
    set_native_mesh_index_stream_00b73b70(*operation.mesh, geometry.actual_owners(), indices);
    at(operation, 0x00ac255c);
    operation.section = geometry.create_native_section_00533fa0(operation.acquired_section);
    operation.material_name_live = true;
    operation.material_name.assign_0041e870(grid.strings, "guidefault.mshd");
    at(operation, 0x00535320);
    operation.owned_material = effects.create_material_00535320(&operation.material_name,
        geometry, binding.materials, operation.material_factory);
    operation.material_name_live = false;
    destroy_native_string_header_0041dd20(&operation.material_name, grid.strings);
    operation.material = operation.owned_material;
    auto& current_section = *operation.section;
    current_section.primitive_08 = 2;
    current_section.range_words_0c[0] = 0;
    current_section.range_words_0c[2] = 0;
    at(operation, 0x00ac25e2);
    set_native_mesh_section_material_00b864c0(current_section, geometry.actual_owners(), operation.material);
    void* creator = operation.owned_material;
    operation.owned_material = nullptr;
    at(operation, 0x00ac25eb);
    release_native_render_actual_owner(geometry.actual_owners(), creator);
    at(operation, 0x00ac2600);
    rebuild_native_mesh_section_vertex_layout_00b865a0(current_section,
        geometry.actual_owners(), operation.mesh, layouts);
    at(operation, 0x00ac2608);
    append_native_mesh_draw_section_00b73c60(*operation.mesh, &current_section);
    // Explicit same-resource-app substitution for AC260D -> manager+28 only.
    // The original material creator has already been consumed; section owns it.
    at(operation, 0x00ac260d);
    void* texture = binding.initial_manager_texture_28();
    require(texture && read<Word>(texture) == 0x00d61948 &&
        &geometry.actual_owners().resolve_actual(texture).reference_count == &count(texture) &&
        count(texture).load() > 0,
        "native Line requires exact live Step2 manager28 actual texture");
    at(operation, 0x00ac261a);
    set_native_material_texture_00b189f0(material(operation.material), 0, texture, geometry.actual_owners());
    at(operation, 0x00ac2629);
    release_stream(operation.index);
    at(operation, 0x00ac2640);
    release_stream(operation.vertex);
    at(operation, 0x00ac2657);
    release_declaration(operation);
    void* selected = operation.section;
    operation.section = nullptr;
    operation.acquired_section.creator = nullptr;
    operation.acquired_section.companion = nullptr;
    at(operation, 0x00ac2669);
    release_native_render_actual_owner(geometry.actual_owners(), selected);
    record.constructed = true;
}

void GameNativeGuiLineApplication::Impl::resize(Record& record, Operation& operation,
    std::int32_t requested) {
    void* actual = record.identity.actual;
    const std::int32_t desired = requested < 5 ? 4 : requested;
    if (read<std::int32_t>(actual, 0x100) == desired) return;
    write(actual, 0x100, desired); // AC26D9 BEFORE getter/factory/locks.
    operation.mesh = &mesh(record);
    at(operation, 0x00ac26ef);
    format(operation);
    at(operation, 0x00ac2752);
    const Word capacity = read<Word>(actual, 0x100);
    void* vertices = create_gui_text_native_vertex_current5c(renderer.publication_00f8d394(),
        capacity * 2u, 1, operation.declaration.reference, grid.graphics, operation.vertex);
    operation.old_vertex = vertex(*operation.mesh);
    const auto copied = (std::min)(read<std::int32_t>(actual, 0xfc), read<std::int32_t>(actual, 0x100));
    require(copied >= 0, "native Line valid committed/capacity domain is nonnegative");
    auto& mapping = grid.graphics.streams.mapping;
    operation.source_phase = NativeStreamCloneMapPhase::call_in_progress;
    at(operation, 0x00ac2789);
    operation.source_map = lock_native_logical_vertex_stream_00b49980(operation.old_vertex,
        mapping, static_cast<Word>(copied), 0, 0);
    operation.source_phase = NativeStreamCloneMapPhase::returned;
    operation.destination_phase = NativeStreamCloneMapPhase::call_in_progress;
    at(operation, 0x00ac279f);
    operation.destination_map = lock_native_logical_vertex_stream_00b49980(vertices,
        mapping, static_cast<Word>(copied), 0, 0);
    operation.destination_phase = NativeStreamCloneMapPhase::returned;
    // AC27A1 reads the STILL retained declaration, not a copied stream stride.
    const Word bytes = read<Word>(operation.declaration.reference, 0xcc) * static_cast<Word>(copied);
    require(!bytes || (operation.source_map && operation.destination_map),
        "native Line vertex copy requires actual HAL mappings");
    at(operation, 0x00ac27b7);
    if (bytes) std::memcpy(operation.destination_map, operation.source_map, bytes);
    operation.source_phase = NativeStreamCloneMapPhase::unlock_in_progress;
    at(operation, 0x00ac27c6);
    unlock_native_logical_vertex_stream_00b49a80(operation.old_vertex, mapping);
    operation.source_phase = NativeStreamCloneMapPhase::unlocked;
    operation.destination_phase = NativeStreamCloneMapPhase::unlock_in_progress;
    at(operation, 0x00ac27cf);
    unlock_native_logical_vertex_stream_00b49a80(vertices, mapping);
    operation.destination_phase = NativeStreamCloneMapPhase::unlocked;
    at(operation, 0x00ac27d6);
    set_native_mesh_vertex_stream_00b73bb0(*operation.mesh, geometry.actual_owners(), 0, vertices);
    at(operation, 0x00ac27f3);
    void* indices = create_gui_text_native_index_current60(renderer.publication_00f8d394(),
        read<Word>(actual, 0x100) * 2u, 1, 0x65, grid.graphics, operation.index);
    operation.old_index = index(*operation.mesh);
    const auto index_copied = (std::min)(read<std::int32_t>(actual, 0xfc), read<std::int32_t>(actual, 0x100));
    require(index_copied >= 0, "native Line index copy count requires nonnegative native counters");
    const Word count = static_cast<Word>(index_copied) * 2u;
    operation.source_phase = NativeStreamCloneMapPhase::call_in_progress;
    at(operation, 0x00ac281d);
    operation.source_map = lock_native_logical_index_stream_00b49b60(operation.old_index, mapping, count, 0, 0);
    operation.source_phase = NativeStreamCloneMapPhase::returned;
    operation.destination_phase = NativeStreamCloneMapPhase::call_in_progress;
    at(operation, 0x00ac282f);
    operation.destination_map = lock_native_logical_index_stream_00b49b60(indices, mapping, count, 0, 0);
    operation.destination_phase = NativeStreamCloneMapPhase::returned;
    require(!count || (operation.source_map && operation.destination_map),
        "native Line index copy requires actual HAL mappings");
    at(operation, 0x00ac283b);
    if (count) std::memcpy(operation.destination_map, operation.source_map, count * 2u);
    operation.source_phase = NativeStreamCloneMapPhase::unlock_in_progress;
    at(operation, 0x00ac284c);
    unlock_native_logical_index_stream_00b49c70(operation.old_index, mapping);
    operation.source_phase = NativeStreamCloneMapPhase::unlocked;
    operation.destination_phase = NativeStreamCloneMapPhase::unlock_in_progress;
    at(operation, 0x00ac2855);
    unlock_native_logical_index_stream_00b49c70(indices, mapping);
    operation.destination_phase = NativeStreamCloneMapPhase::unlocked;
    at(operation, 0x00ac285a);
    set_native_mesh_index_stream_00b73b70(*operation.mesh, geometry.actual_owners(), indices);
    auto& selected = section(*operation.mesh);
    at(operation, 0x00ac286b);
    rebuild_native_mesh_section_vertex_layout_00b865a0(selected, geometry.actual_owners(), operation.mesh, layouts);
    at(operation, 0x00ac287a);
    release_stream(operation.index);
    at(operation, 0x00ac288c);
    release_stream(operation.vertex);
    at(operation, 0x00ac28a2);
    release_declaration(operation);
}

void GameNativeGuiLineApplication::Impl::flush(Record& record, Operation& operation) {
    void* actual = record.identity.actual;
    if (!read<void*>(actual, 0xf0)) return; // AC29DC: no second descriptor read.
    const std::int32_t total = point_count(record);
    if (!total) return;
    if (total >= read<std::int32_t>(actual, 0x100)) resize(record, operation, total);
    operation.mesh = &mesh(record);
    operation.flush_vertex = vertex(*operation.mesh);
    operation.flush_index = index(*operation.mesh);
    operation.section = &section(*operation.mesh);
    operation.section->range_words_0c[1] = static_cast<Word>(total);
    operation.section->range_words_0c[3] = static_cast<Word>(total) - 1u;
    auto& mapping = grid.graphics.streams.mapping;
    const Word committed_for_index = read<Word>(actual, 0xfc);
    operation.flush_index_phase = NativeStreamCloneMapPhase::call_in_progress;
    at(operation, 0x00ac2a5e);
    operation.flush_index_map = lock_native_logical_index_stream_00b49b60(operation.flush_index, mapping,
        (static_cast<Word>(total) - committed_for_index) * 2u, committed_for_index * 2u, 0);
    operation.flush_index_phase = NativeStreamCloneMapPhase::returned;
    const Word committed_for_vertex = read<Word>(actual, 0xfc);
    operation.flush_vertex_phase = NativeStreamCloneMapPhase::call_in_progress;
    at(operation, 0x00ac2a79);
    operation.flush_vertex_map = lock_native_logical_vertex_stream_00b49980(operation.flush_vertex, mapping,
        static_cast<Word>(total) - committed_for_vertex, committed_for_vertex, 0);
    operation.flush_vertex_phase = NativeStreamCloneMapPhase::returned;
    // DWORD index address avoids manufacturing an out-of-allocation C++ pointer
    // for native map-2, which is not dereferenced for the first uncommitted point.
    Word output_index = address(operation.flush_index_map) - 2u;
    Word one_bits = 0;
    if (static_cast<std::int32_t>(static_cast<Word>(total) - read<Word>(actual, 0xfc)) > 0) {
        const float one = binding.one_00d7a24c; // AC2A94 only on the native write-loop arm.
        std::memcpy(&one_bits, &one, 4);
    }
    for (Word k = 0; static_cast<std::int32_t>(k) <
        static_cast<std::int32_t>(static_cast<Word>(total) - read<Word>(actual, 0xfc)); ++k) {
        const Word vertex_number = read<Word>(actual, 0xfc) + k;
        if (static_cast<std::int32_t>(vertex_number) >= 1) {
            require(operation.flush_index_map != nullptr, "native Line index writer requires an actual HAL mapping");
            const auto previous = static_cast<std::uint16_t>(vertex_number - 1u);
            const auto current = static_cast<std::uint16_t>(read<Word>(actual, 0xfc) + k);
            write(pointer(output_index - 2u), 0, previous);
            write(pointer(output_index), 0, current);
        }
        require(k < static_cast<Word>(point_count(record)), "native Line point iterator is outside its actual vector");
        const auto* point_y = static_cast<const float*>(pointer(read<Word>(actual, 0xf0) + k * 8u + 4u));
        require(k < static_cast<Word>(point_count(record)), "native Line current X iterator is outside its actual vector");
        const auto* point_x = static_cast<const float*>(pointer(read<Word>(actual, 0xf0) + k * 8u));
        const Word stride = read<Word>(operation.flush_vertex, 0x0c);
        const Word position = read<Word>(operation.flush_vertex, 0x10);
        const Word uv = read<Word>(operation.flush_vertex, 0x28);
        const auto fits = [stride](Word offset, Word bytes) { return offset <= stride && bytes <= stride - offset; };
        require(fits(position, 12) && fits(uv, 8) && read<void*>(operation.flush_vertex, 8),
            "native Line semantic offsets must fit the actual mapped native stream");
        auto& transform = record.identity.widget.layout().transform;
        auto* position_out = static_cast<float*>(pointer(read<Word>(operation.flush_vertex, 8) + stride * k + position));
        at(operation, 0x00ac2b5a);
        write_position(position_out, point_x, point_y, &transform.pivot_x, &transform.pivot_y,
            &binding.vertical_scale_00e12ff4);
        const Word zero = 0;
        const Word uv_base = read<Word>(operation.flush_vertex, 8) + read<Word>(operation.flush_vertex, 0x0c) * k +
            read<Word>(operation.flush_vertex, 0x28);
        write(pointer(uv_base), 0, zero); write(pointer(uv_base), 4, zero);
        const auto packed = read<std::int32_t>(operation.flush_vertex, 0x34);
        const Word color_base = read<Word>(operation.flush_vertex, 8) + read<Word>(operation.flush_vertex, 0x0c) * k;
        if (packed >= 0) {
            require(fits(static_cast<Word>(packed), 4), "native Line packed color lies outside the native stream stride");
            const Word white = 0xffffffffu;
            write(pointer(color_base + static_cast<Word>(packed)), 0, white);
        } else for (Word lane = 0; lane != 4; ++lane) {
            const Word offset = read<Word>(operation.flush_vertex, 0x38 + lane * 4u);
            require(fits(offset, 4), "native Line float color lies outside the native stream stride");
            write(pointer(color_base + offset), 0, one_bits);
        }
        output_index += 4u;
    }
    operation.flush_index_phase = NativeStreamCloneMapPhase::unlock_in_progress;
    at(operation, 0x00ac2c10);
    unlock_native_logical_index_stream_00b49c70(operation.flush_index, mapping);
    operation.flush_index_phase = NativeStreamCloneMapPhase::unlocked;
    operation.flush_vertex_phase = NativeStreamCloneMapPhase::unlock_in_progress;
    at(operation, 0x00ac2c19);
    unlock_native_logical_vertex_stream_00b49a80(operation.flush_vertex, mapping);
    operation.flush_vertex_phase = NativeStreamCloneMapPhase::unlocked;
    write(actual, 0xfc, static_cast<Word>(total)); // AC2C1B, before clearing SAME vector.
    clear_points(record);
    at(operation, 0x00ac2c88);
    rebuild_native_mesh_section_vertex_layout_00b865a0(*operation.section,
        geometry.actual_owners(), operation.mesh, layouts);
}

GameNativeGuiLineApplication::GameNativeGuiLineApplication(GameNativeRendererApplication& renderer,
    GameNativeGuiEffectApplication& effects, GameNativeGuiLineApplicationBindings bindings)
    : impl_(std::make_unique<Impl>(renderer, effects, std::move(bindings))) {}
GameNativeGuiLineApplication::~GameNativeGuiLineApplication() noexcept {
    if (requires_process_retention()) std::terminate();
}
void GameNativeGuiLineApplication::constructed74_00ac2430(GuiWidgetOwner& owner) {
    auto& p = *impl_; auto& record = p.record(owner); Impl::Entry entry(p, record, 0x00ac2430);
    try { p.create(record, entry.operation()); entry.complete(); }
    catch (...) { p.fail(entry.operation()); throw; }
}
void GameNativeGuiLineApplication::ensure_capacity_00ac2690(GuiWidgetOwner& owner, std::int32_t requested) {
    auto& p = *impl_; auto& record = p.record(owner); Impl::Entry entry(p, record, 0x00ac2690);
    try { p.resize(record, entry.operation(), requested); entry.complete(); }
    catch (...) { p.fail(entry.operation()); throw; }
}
void GameNativeGuiLineApplication::flush80_00ac29d0(GuiWidgetOwner& owner) {
    auto& p = *impl_; auto& record = p.record(owner); Impl::Entry entry(p, record, 0x00ac29d0);
    try { p.flush(record, entry.operation()); entry.complete(); }
    catch (...) { p.fail(entry.operation()); throw; }
}
void GameNativeGuiLineApplication::reset7c_00ac2950(GuiWidgetOwner& owner) {
    auto& p = *impl_; auto& record = p.record(owner); Impl::Entry entry(p, record, 0x00ac2950);
    try {
        p.clear_points(record);
        write(record.identity.actual, 0xfc, Word(0));
        p.resize(record, entry.operation(), 4);
        auto& selected = p.section(p.mesh(record));
        selected.range_words_0c[1] = 0; selected.range_words_0c[3] = 0;
        entry.complete();
    } catch (...) { p.fail(entry.operation()); throw; }
}
void GameNativeGuiLineApplication::loaded78_00ac1bb0(GuiWidgetOwner& owner) {
    auto& p = *impl_; auto& record = p.record(owner); Impl::Entry entry(p, record, 0x00ac1bb0);
    try {
        owner.base_loaded78_00aa7170();
        p.at(entry.operation(), 0x00ac1bc3);
        set_gui_color_00aa6870(owner, owner.layout().color, p.binding.colors);
        p.at(entry.operation(), 0x00ac1bcf);
        p.flush(record, entry.operation());
        entry.complete();
    } catch (...) { p.fail(entry.operation()); throw; }
}
void GameNativeGuiLineApplication::release_line_borrow_guard(GuiWidgetOwner& owner) {
    auto& p = *impl_; p.require_idle();
    const auto found = p.records.find(&owner.layout());
    require(found != p.records.end(), "native Line borrow retirement requires its existing record");
    auto& record = *found->second; p.validate(record, false);
    require(!owner.node_binding() && !owner.model_reference() &&
        record.identity.reference.reference_count.load() == 1 &&
        (!record.operation || record.operation->complete),
        "native Line borrow retirement requires drained Model/scene/queue and sole widget creator");
    record.identity.page_token.reset(); record.guard_released = true;
}
void GameNativeGuiLineApplication::release_retired_line(GuiLayoutWidget& layout) {
    auto& p = *impl_; p.require_idle();
    const auto found = p.records.find(&layout);
    require(found != p.records.end() && found->second->guard_released &&
        !found->second->identity.page_token && (!found->second->operation || found->second->operation->complete),
        "native Line metadata must follow its same native resource/borrow retirement");
    p.records.erase(found);
}
bool GameNativeGuiLineApplication::requires_process_retention() const noexcept {
    return impl_->active || impl_->failed || !impl_->records.empty();
}
std::uint32_t GameNativeGuiLineApplication::failure_entry() const noexcept { return impl_->site; }
const std::exception_ptr& GameNativeGuiLineApplication::failure() const noexcept { return impl_->failure; }
} // namespace bsp::game
