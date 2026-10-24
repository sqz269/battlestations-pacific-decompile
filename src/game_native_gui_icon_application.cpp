#include "bsp/game_native_gui_icon_application.hpp"
#include "bsp/game_native_gui_effect_application.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/gui_native_clip_parameters.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/gui_text_native_layout.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_texture_saved_dimensions.hpp"
#include <array>
#include <cstring>
#include <list>
#include <stdexcept>
#include <unordered_map>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Icon composition requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
using Word = std::uint32_t;
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
template<class T> T read(const void* raw, Word offset = 0) {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(raw) + offset, sizeof result);
    return result;
}
std::atomic<std::int32_t>& actual_count(void* raw) {
    require(raw && !(reinterpret_cast<std::uintptr_t>(raw) & 3u),
        "Native Icon requires aligned actual storage");
    return *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(raw) + 4));
}
void copy_x87(float& destination, const float& source) noexcept {
    auto* destination_address = &destination;
    const auto* source_address = &source;
    __asm {
        mov eax, source_address
        mov edx, destination_address
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}
} // namespace

struct GameNativeGuiIconApplication::Impl {
    struct Rebuild {
        Word site{};
        bool running{}, complete{};
        NativeString format_name, material_name;
        bool format_name_live{}, material_name_live{};
        GuiNativeDeclarationAcquired declaration;
        NativeStreamCloneAcquired vertex;
        NativeMeshStorage* mesh{};
        void* stream{};
        void* mapping{};
        NativeStreamCloneMapPhase map_phase{NativeStreamCloneMapPhase::not_called};
        GuiNativeSectionSelection section{};
        void* material{};
        const GameNativeGuiMaterialFactoryOperation* material_factory{};
        // These headers stay address-stable even on an entered registration
        // failure. A consumed header is observation only, never replayed.
        std::list<NativeString> parameter_names;
        std::exception_ptr failure;
    };
    struct TextureLoad {
        NativeString name;
        bool name_live{};
        NativeTextureCacheAcquired cache;
        void* result{}; // Transferred caller reference; diagnostic identity.
    };
    struct Record {
        GuiWidgetOwner& owner;
        GameNativeGuiIconIdentity identity;
        std::shared_ptr<const void> page_token;
        std::unique_ptr<Rebuild> rebuild;
        std::list<TextureLoad> texture_loads;
        bool runtime_retired{}, borrow_guard_released{};
        Record(GuiWidgetOwner& owner_in, GameNativeGuiIconIdentity identity_in)
            : owner(owner_in), identity(identity_in) {}
    };
    GameNativeRendererApplication& renderer;
    GameNativeGuiEffectApplication& effects;
    GameNativeGuiIconApplicationBindings binding;
    NativeGameGridContext& grid;
    GuiNativeGeometryOwners& geometry;
    GuiTextNativeLayoutServices& layouts;
    std::unordered_map<GuiLayoutWidget*, std::unique_ptr<Record>> records;
    bool active{}, failed{};
    Word site{};
    std::exception_ptr failure;

    Impl(GameNativeRendererApplication& renderer_in, GameNativeGuiEffectApplication& effects_in,
        GameNativeGuiIconApplicationBindings supplied)
        : renderer(renderer_in), effects(effects_in), binding(std::move(supplied)),
          grid(renderer.game_grid_context()), geometry(grid.graphics.streams.geometry),
          layouts(renderer.section_layout_services()) {
        auto& actual = geometry.actual_owners();
        const auto& registration = geometry.registration();
        require(&actual == &renderer.actual_owners() &&
            &binding.widgets.environment().models.retained_owners == &actual &&
            binding.widgets.environment().actual_owners == &renderer.actual_owners() &&
            &binding.materials.retained_owners == &actual &&
            &binding.widget_materials.actual_owners == &actual &&
            &binding.widget_materials.widgets == &binding.widgets &&
            &binding.materials.parameter_names == &grid.strings &&
            &binding.parameters.parameter_names == &grid.strings &&
            &binding.materials.parameter_slots == &binding.parameters.parameter_slots &&
            registration.bind && registration.unbind && registration.find,
            "Native Icon requires the renderer's SAME widget/material/string/owner domain");
        require(binding.actual_identity && binding.widget_materials.retain_widget &&
            binding.find_atlas_item_00aefb20 && binding.platform_allows_point_filter,
            "Native Icon requires actual page identity, lifetime and atlas providers");
        require_gui_text_native_renderer_domain(grid.graphics, renderer.publication_00f8d394(),
            grid.strings, actual);
    }
    void require_idle() const {
        require(!active && !failed, "Native Icon has an entered or failed operation");
    }
    struct Entry {
        Impl& app;
        int exceptions{std::uncaught_exceptions()};
        Entry(Impl& value, Word entry) : app(value) {
            app.require_idle();
            app.active = true;
            app.site = entry;
        }
        ~Entry() {
            if (std::uncaught_exceptions() > exceptions) {
                app.failed = true;
            }
            app.active = false;
        }
    };
    void validate_identity(Record& record) const {
        auto& actual = geometry.actual_owners();
        const auto& identity = record.identity;
        require(identity.actual && &identity.widget == &record.owner &&
            &binding.widgets.owner(record.owner.layout()) == &record.owner &&
            read<Word>(identity.actual) == 0x00d5c4c0 &&
            &identity.reference.reference_count == &actual_count(identity.actual) &&
            identity.reference.reference_count.load() > 0 &&
            &actual.resolve_actual(identity.actual) == &identity.reference,
            "Native Icon requires its SAME live D5C4C0 raw allocation/count/companion");
    }
    NativeModelOwner& model(Record& record) {
        validate_identity(record);
        auto* reference = record.owner.model_reference();
        auto* node = record.owner.node_binding();
        require(reference && node && &reference->model_owner().node == node &&
            reference->reference_count.load() > 0,
            "Native Icon requires its sole actual primary Model");
        auto& result = reference->model_owner();
        require(result.phase == NativeModelOwner::Phase::live &&
            &result.environment == &binding.widgets.environment().models &&
            &result.environment.retained_owners == &geometry.actual_owners(),
            "Native Icon primary Model belongs to another native domain");
        return result;
    }
    void validate_texture(void* raw) const {
        require(raw && read<Word>(raw) == 0x00d61948,
            "Native Icon texture requires its actual D61948 2D owner");
        auto& reference = geometry.actual_owners().resolve_actual(raw);
        require(&reference.reference_count == &actual_count(raw) && reference.reference_count.load() > 0,
            "Native Icon texture requires its SAME canonical native reference count");
    }
    Word width(void* raw) const {
        validate_texture(raw);
        return get_native_texture_saved_width_00b3ce70(raw);
    }
    Word height(void* raw) const {
        validate_texture(raw);
        return get_native_texture_saved_height_00b3ce80(raw);
    }
    void* load_texture(Record& record, std::string_view spelling, Word flags) {
        Entry entered(*this, 0x00aa2660);
        validate_identity(record);
        require(!record.runtime_retired && !record.borrow_guard_released && flags == 0,
            "Native Icon texture load requires its live original flags0 caller");
        record.texture_loads.emplace_back();
        auto& load = record.texture_loads.back();
        const std::string name(spelling);
        load.name_live = true;
        load.name.assign_0041e870(grid.strings, name.c_str());
        void* receiver = renderer.publication_00f8d394();
        const auto* profile = grid.graphics.streams.vertices.actual_renderer_profile_00d5f0a8;
        require(receiver && read<Word>(receiver) == 0x00d5f0a8 && profile &&
            profile[0x64 / 4] == 0x00b319b0,
            "Native Icon requires current renderer64=B319B0");
        site = 0x00b319b0;
        try {
            load.result = load_native_renderer_texture_00b319b0(receiver, &load.name, flags,
                renderer.texture_cache(), &load.cache);
            validate_texture(load.result);
            destroy_native_string_header_0041dd20(&load.name, grid.strings);
            load.name_live = false;
            return load.result;
        } catch (...) {
            failure = std::current_exception();
            throw;
        }
    }
    void constructed74(Record& record) {
        Entry entered(*this, 0x00ab2540);
        try {
            require(!record.runtime_retired && !record.borrow_guard_released,
                "Native Icon current74 requires its live same-owner record");
            geometry.construct_and_associate74_fragment(model(record));
        } catch (...) { failure = std::current_exception(); throw; }
    }
    NativeMaterialStorage& material(void* raw) const {
        require(raw != nullptr, "Native Icon section has no actual material");
        auto* reference = dynamic_cast<NativeMaterialReference*>(
            &geometry.actual_owners().resolve_actual(raw));
        require(reference && &reference->storage() == raw && reference->reference_count.load() > 0 &&
            reference->storage().vtable_00 == 0x00d5e520,
            "Native Icon requires its canonical actual D5E520 material");
        return reference->storage();
    }
    void register_parameter(Rebuild& operation, NativeMaterialStorage& target,
        const char* spelling, const void* source, Word words) {
        operation.parameter_names.emplace_back();
        auto& name = operation.parameter_names.back();
        name.assign_0041e870(grid.strings, spelling);
        register_native_material_parameter_00b17e10(target, &name, source, words, 0, binding.parameters);
        destroy_native_string_header_0041dd20(&name, grid.strings);
    }
    void ensure_stream(Rebuild& operation) {
        auto& mesh = *operation.mesh;
        auto& owners = geometry.actual_owners();
        require(!mesh.index_stream_60 && mesh.vertex_stream_count_7c >= 0 &&
            mesh.vertex_stream_count_7c <= 1,
            "Native Icon requires its nonindexed, single-stream mesh");
        if (!mesh.vertex_stream_count_7c) {
            operation.format_name_live = true;
            operation.format_name.assign_0041e870(grid.strings, "SimpleColor.mvfm");
            site = operation.site = 0x00ab3d1f;
            load_gui_text_native_declaration_current38(renderer.publication_00f8d394(),
                operation.format_name, grid.graphics, operation.declaration);
            destroy_native_string_header_0041dd20(&operation.format_name, grid.strings);
            operation.format_name_live = false;
            site = operation.site = 0x00ab3d5a;
            void* stream = create_gui_text_native_vertex_current5c(renderer.publication_00f8d394(),
                4, 1, operation.declaration.reference, grid.graphics, operation.vertex);
            site = operation.site = 0x00ab3d63;
            set_native_mesh_vertex_stream_00b73bb0(mesh, owners, 0, stream);
            operation.vertex.creator = nullptr;
            operation.vertex.companion = nullptr;
            operation.vertex.phase = NativeStreamClonePhase::consumed;
            site = operation.site = 0x00ab3d6c;
            release_native_render_actual_owner(owners, stream);
            void* declaration = operation.declaration.reference;
            operation.declaration = {};
            site = operation.site = 0x00ab3d82;
            release_native_render_actual_owner(owners, declaration);
        }
        operation.stream = mesh.vertex_streams_64[0];
        auto* reference = operation.stream ? dynamic_cast<NativeLogicalVertexReference*>(
            &owners.resolve_actual(operation.stream)) : nullptr;
        require(reference && reference->storage() == operation.stream &&
            reference->reference_count.load() > 0 &&
            read<Word>(operation.stream) == 0x00d61d6c && read<Word>(operation.stream, 0x64) >= 4,
            "Native Icon requires its real canonical four-vertex HAL stream");
    }
    void write_quad(Record& record, GuiIconWidget& icon, Rebuild& operation) {
        GuiIconQuadSetup setup;
        require(gui_icon_build_quad_setup(icon, record.owner.layout().transform, setup),
            "Native Icon cannot resolve its selected state");
        auto& transform = record.owner.layout().transform;
        // AB41A9..41BE is reached even when widget+74 suppresses stream writes.
        if (setup.writes_widget_pivot) {
            copy_x87(transform.size.width, setup.size.width);
            copy_x87(transform.size.height, setup.size.height);
            copy_x87(transform.pivot_x, setup.pivot_x);
            copy_x87(transform.pivot_y, setup.pivot_y);
        }
        if (transform.bounds_enabled) return;
        if (setup.writes_widget_pivot) { // AB4249..4266 repeated native stores.
            copy_x87(transform.pivot_x, setup.pivot_x);
            copy_x87(transform.pivot_y, setup.pivot_y);
            copy_x87(transform.size.width, setup.size.width);
            copy_x87(transform.size.height, setup.size.height);
        }
        setup.quad.y_scale = binding.vertical_scale_00e12fd0;
        std::array<GuiQuadVertex, 4> vertices;
        const bool positions_valid = gui_write_cropped_quad_00ab1860(setup.quad, vertices);
        icon.partial_display_ratio = setup.quad.ratio;
        auto* bytes = static_cast<std::byte*>(operation.mapping);
        const Word stride = read<Word>(operation.stream, 0x0c);
        const Word position = read<Word>(operation.stream, 0x10);
        const Word uv = read<Word>(operation.stream, 0x28);
        const auto fits = [stride](Word offset, Word length) {
            return offset <= stride && length <= stride - offset;
        };
        require(bytes && fits(position, 12) && fits(uv, 8),
            "Native Icon mapped writer fields exceed the actual stream stride");
        if (positions_valid) for (Word index : {0u, 1u, 3u, 2u}) {
            const auto& vertex = vertices[index];
            const float xyz[]{vertex.x, vertex.y, vertex.z};
            const float texture_uv[]{vertex.u, vertex.v};
            std::memcpy(bytes + index * stride + position, xyz, sizeof xyz);
            std::memcpy(bytes + index * stride + uv, texture_uv, sizeof texture_uv);
        }
        const auto packed = read<std::int32_t>(operation.stream, 0x34);
        if (packed >= 0) {
            require(fits(static_cast<Word>(packed), 4), "Native Icon packed color offset exceeds stride");
            const Word white = 0xffffffffu;
            for (Word index = 0; index != 4; ++index)
                std::memcpy(bytes + index * stride + packed, &white, 4);
        } else {
            const float white = binding.texture_constants.one_00d7a24c;
            for (Word index = 0; index != 4; ++index) for (Word lane = 0; lane != 4; ++lane) {
                const Word offset = read<Word>(operation.stream, 0x38 + lane * 4);
                require(fits(offset, 4), "Native Icon float color offset exceeds stride");
                std::memcpy(bytes + index * stride + offset, &white, 4);
            }
        }
    }
    void rebuild(Record& record, GuiIconWidget& icon) {
        Entry entered(*this, 0x00ab3cb0);
        require(!record.runtime_retired && !record.borrow_guard_released,
            "Native Icon rebuild requires its same live page record");
        require(!record.rebuild || record.rebuild->complete,
            "Native Icon cannot replay an entered rebuild");
        record.rebuild = std::make_unique<Rebuild>();
        auto& operation = *record.rebuild;
        operation.running = true;
        try {
            // The loader publishes the sole child into the page tree before
            // current78/state rebuild. make_type/current74 precede that point
            // and create no borrowed material parameters.
            if (!record.page_token)
                record.page_token = binding.widget_materials.retain_widget(record.owner);
            require(record.page_token && record.page_token.get() == &record.owner,
                "Native Icon rebuild requires its authoritative same-page lifetime token");
            auto& primary = model(record);
            auto* raw = primary.storage.model.geometry_180;
            auto* mesh_reference = raw ? dynamic_cast<NativeMeshReference*>(
                &geometry.actual_owners().resolve_actual(raw)) : nullptr;
            require(mesh_reference && &mesh_reference->storage() == raw &&
                mesh_reference->reference_count.load() > 0,
                "Native Icon current74 must publish its SAME actual mesh first");
            operation.mesh = &mesh_reference->storage();
            ensure_stream(operation);
            site = operation.site = 0x00ab3dae;
            operation.map_phase = NativeStreamCloneMapPhase::call_in_progress;
            operation.mapping = lock_native_logical_vertex_stream_00b49980(operation.stream,
                grid.graphics.streams.mapping, 4, 0, 0);
            operation.map_phase = NativeStreamCloneMapPhase::returned;
            require(operation.mapping && read<void*>(operation.stream, 8) == operation.mapping,
                "Native Icon requires the actual HAL mapped write pointer");
            operation.section = acquire_gui_native_section_fragment(*operation.mesh, geometry);
            const bool existing = !operation.section.append_on_publish;
            const auto* selected = icon.has_texture ? gui_icon_state_at(icon, icon.current_state) : nullptr;
            require(!icon.has_texture || selected, "Native Icon state is outside its signed16 vector");
            const bool platform_point = icon.has_texture && icon.shader_name.empty()
                ? binding.platform_allows_point_filter() : false;
            bool differs = false;
            const auto& transform = record.owner.layout().transform;
            if (icon.has_texture && icon.shader_name.empty() && platform_point &&
                transform.rotate == 0.0f && transform.scale_x == 1.0f && transform.scale_y == 1.0f)
                differs = gui_icon_state_differs_from_native_size_00ab24b0(*selected,
                    width(selected->texture), height(selected->texture));
            const bool bilinear = gui_icon_prefers_bilinear_filter_00ab2600(icon, transform, platform_point, differs);
            if (existing && bilinear == icon.cached_prefers_bilinear) {
                operation.material = operation.section.section->material_20;
                material(operation.material).references_04.fetch_add(1, std::memory_order_seq_cst);
            } else {
                icon.cached_prefers_bilinear = bilinear;
                operation.material_name_live = true;
                operation.material_name.assign_0041e870(grid.strings, gui_icon_material_name(icon, bilinear));
                site = operation.site = 0x00535320;
                operation.material = effects.create_material_00535320(&operation.material_name,
                    geometry, binding.materials, operation.material_factory);
                destroy_native_string_header_0041dd20(&operation.material_name, grid.strings);
                operation.material_name_live = false;
            }
            auto& target = material(operation.material);
            site = operation.site = 0x00ab3f76;
            register_parameter(operation, target, "cOverbrightFactor", &record.owner.extra_fields().overbright_94, 1);
            register_parameter(operation, target, "cLowColor", record.owner.layout().low_color, 4);
            register_parameter(operation, target, "cHighColor", record.owner.layout().high_color, 4);
            register_parameter(operation, target, "cBlendFactor", &record.owner.layout().blend_factor, 1);
            site = operation.site = 0x00ab4114;
            register_native_gui_clip_parameters_00aa9f10(record.owner, target, binding.widgets,
                binding.widget_materials.aspect_ratio_00e12fc0, binding.parameters);
            validate_identity(record);
            require(record.page_token && record.page_token.get() == &record.owner,
                "Native Icon material requires the authoritative page lifetime token");
            site = operation.site = 0x00ab411e;
            set_native_material_parameter_owner_00b18a40(target, record.identity.actual, 1,
                geometry.actual_owners());
            auto& section = *operation.section.section;
            section.primitive_08 = 5;
            section.range_words_0c[0] = 0;
            section.range_words_0c[2] = 0;
            section.range_words_0c[1] = 4;
            section.range_words_0c[3] = 2;
            // Reload the same selected state after preceding material callbacks.
            selected = icon.has_texture ? gui_icon_state_at(icon, icon.current_state) : nullptr;
            require(!icon.has_texture || selected, "Native Icon lost its selected state during material binding");
            if (selected) validate_texture(selected->texture);
            set_native_material_texture_00b189f0(target, 0, selected ? selected->texture : nullptr,
                geometry.actual_owners());
            write_quad(record, icon, operation);
            site = operation.site = 0x00ab4431;
            operation.map_phase = NativeStreamCloneMapPhase::unlock_in_progress;
            unlock_native_logical_vertex_stream_00b49a80(operation.stream, grid.graphics.streams.mapping);
            operation.map_phase = NativeStreamCloneMapPhase::unlocked;
            record.owner.recompose_00aa7220();
            site = operation.site = 0x00ab443d;
            publish_gui_native_section_fragment(*operation.mesh, operation.section, operation.material,
                geometry.actual_owners(), layouts);
            site = operation.site = 0x00ab4495;
            set_gui_color_00aa6870(record.owner, record.owner.layout().color, binding.widget_materials);
            operation.running = false;
            operation.complete = true;
        } catch (...) {
            operation.failure = failure = std::current_exception();
            throw; // Preserve the actual map, creators, publications and frames.
        }
    }
    Record& record_for(GuiWidgetOwner& owner) {
        auto found = records.find(&owner.layout());
        require(found != records.end() && &found->second->owner == &owner,
            "Native Icon requires its original same-owner factory record");
        return *found->second;
    }
    void runtime_retiring(Record& record) {
        require_idle();
        validate_identity(record);
        require(record.borrow_guard_released && !record.page_token &&
            !record.owner.node_binding() && actual_count(record.identity.actual).load() == 1 &&
            (!record.rebuild || record.rebuild->complete) && !record.runtime_retired,
            "Native Icon fields cannot retire before its actual material borrowers and page guard");
        record.runtime_retired = true;
    }
};

GameNativeGuiIconApplication::GameNativeGuiIconApplication(GameHostLog&,
    GameNativeRendererApplication& renderer, GameNativeGuiEffectApplication& effects,
    GameNativeGuiIconApplicationBindings bindings)
    : impl_(std::make_unique<Impl>(renderer, effects, std::move(bindings))) {}
GameNativeGuiIconApplication::~GameNativeGuiIconApplication() noexcept {
    if (requires_process_retention()) std::terminate();
}
std::unique_ptr<GuiWidgetTypeImplementation> GameNativeGuiIconApplication::make_type(GuiWidgetOwner& owner) {
    auto& app = *impl_;
    app.require_idle();
    require(owner.layout().type == GuiWidgetType::Icon && owner.layout().transform.type_id == 6 &&
        &owner.runtime() == &app.binding.widgets && !owner.node_binding() &&
        !app.records.count(&owner.layout()),
        "Native Icon factory requires its fresh primary-null canonical type6 owner");
    auto record = std::make_unique<Impl::Record>(owner, app.binding.actual_identity(owner));
    app.validate_identity(*record);
    auto* saved = record.get();
    app.records.emplace(&owner.layout(), std::move(record));
    GuiIconRuntimeServices services{app.binding.crt_sse2_conversion};
    services.textures.resolve.find_atlas_item = [&app](std::string_view name) {
        auto* item = app.binding.find_atlas_item_00aefb20(name);
        if (item) app.validate_texture(item->texture);
        return item;
    };
    services.textures.resolve.load_texture = [&app, saved](std::string_view name, Word flags) {
        return app.load_texture(*saved, name, flags);
    };
    services.textures.resolve.width = [&app](void* raw) { return app.width(raw); };
    services.textures.resolve.height = [&app](void* raw) { return app.height(raw); };
    services.textures.resolve.retain = [&app](void* raw) {
        app.validate_texture(raw);
        actual_count(raw).fetch_add(1, std::memory_order_seq_cst);
    };
    services.textures.release = [&app](void* raw) {
        release_native_render_actual_owner(app.geometry.actual_owners(), raw);
    };
    services.platform_allows_point_filter = app.binding.platform_allows_point_filter;
    services.state_texture_constants = &app.binding.texture_constants;
    services.native_geometry.construct_and_associate74 = [&app, saved] { app.constructed74(*saved); };
    services.native_geometry.rebuild_00ab3cb0 = [&app, saved](GuiIconWidget& icon) { app.rebuild(*saved, icon); };
    services.native_geometry.before_runtime_destruction = [&app, saved] { app.runtime_retiring(*saved); };
    try { return std::make_unique<GuiIconTypeImplementation>(owner, std::move(services)); }
    catch (...) { app.failed = true; app.failure = std::current_exception(); throw; }
}
void GameNativeGuiIconApplication::release_material_borrow_guard(GuiWidgetOwner& owner) {
    auto& app = *impl_;
    app.require_idle();
    auto& record = app.record_for(owner);
    app.validate_identity(record);
    require(!record.runtime_retired && !record.borrow_guard_released && !owner.node_binding() &&
        actual_count(record.identity.actual).load() == 1 && (!record.rebuild || record.rebuild->complete),
        "Native Icon page guard requires completed actual scene/material retirement first");
    record.borrow_guard_released = true;
    record.page_token.reset();
}
void GameNativeGuiIconApplication::release_retired_icon(GuiLayoutWidget& layout) {
    auto& app = *impl_;
    app.require_idle();
    const auto found = app.records.find(&layout);
    require(found != app.records.end() && found->second->runtime_retired &&
        found->second->borrow_guard_released && !found->second->page_token,
        "Native Icon metadata requires completed same-type/page retirement");
    app.records.erase(found);
}
bool GameNativeGuiIconApplication::requires_process_retention() const noexcept {
    return impl_->active || impl_->failed || !impl_->records.empty();
}
std::uint32_t GameNativeGuiIconApplication::failure_entry() const noexcept { return impl_->site; }
const std::exception_ptr& GameNativeGuiIconApplication::failure() const noexcept { return impl_->failure; }
} // namespace bsp::game
