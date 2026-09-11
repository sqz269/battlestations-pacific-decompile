#include "bsp/gui_native_geometry.hpp"
#include <cstring>
#include <exception>
#include <list>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
void require(bool condition, const char* reason) {
    if (!condition) throw std::invalid_argument(reason);
}
}

struct GuiNativeGeometryOwners::Impl {
    struct MeshEntry {
        explicit MeshEntry(Impl& owner) noexcept : domain(owner) {}
        Impl& domain;
        NativeMeshStorage* raw{};
        std::unique_ptr<NativeMeshReference> reference;
        bool registered{};
    };
    struct SectionEntry {
        explicit SectionEntry(Impl& owner) noexcept : domain(owner) {}
        Impl& domain;
        NativeMeshSectionStorage* raw{};
        std::unique_ptr<NativeMeshSectionReference> reference;
        bool registered{};
    };
    NativeMeshEnvironment& meshes;
    NativeMeshConstants constants;
    NativeMeshSectionEnvironment& sections;
    GuiNativeGeometryRegistration registration;
    std::list<MeshEntry> mesh_entries;
    std::list<SectionEntry> section_entries;

    Impl(NativeMeshEnvironment& mesh_environment, NativeMeshConstants supplied,
        NativeMeshSectionEnvironment& section_environment, GuiNativeGeometryRegistration registry)
        : meshes(mesh_environment), constants(supplied), sections(section_environment),
          registration(registry) {
        require(registration.bind && registration.unbind,
            "GUI native geometry requires canonical registration and retirement");
        require(&registration.owners == &meshes.retained_owners &&
            &registration.owners == &sections.retained_owners,
            "GUI native mesh and section must use the same actual owner domain");
    }
    ~Impl() {
        if (!mesh_entries.empty() || !section_entries.empty()) std::terminate();
    }
    static void retire_mesh(void* context, NativeMeshReference& reference) noexcept {
        auto& entry = *static_cast<MeshEntry*>(context);
        auto& self = entry.domain;
        if (entry.registered)
            self.registration.unbind(self.registration.context, entry.raw, reference);
        for (auto it = self.mesh_entries.begin(); it != self.mesh_entries.end(); ++it) {
            if (&*it == &entry) { self.mesh_entries.erase(it); return; }
        }
        std::terminate();
    }
    static void retire_section(void* context, NativeMeshSectionReference& reference) noexcept {
        auto& entry = *static_cast<SectionEntry*>(context);
        auto& self = entry.domain;
        if (entry.registered)
            self.registration.unbind(self.registration.context, entry.raw, reference);
        for (auto it = self.section_entries.begin(); it != self.section_entries.end(); ++it) {
            if (&*it == &entry) { self.section_entries.erase(it); return; }
        }
        std::terminate();
    }
    NativeMeshStorage* create_mesh() {
        auto it = mesh_entries.emplace(mesh_entries.end(), *this);
        try {
            void* raw = meshes.pool_0108fff8.allocate_slot_00b73a10();
            if (!raw) throw std::bad_alloc();
            it->raw = construct_native_mesh_00b73d70(raw, constants);
            it->reference = std::make_unique<NativeMeshReference>(*it->raw, meshes,
                NativeMeshCompanionDisposal{&*it, retire_mesh});
            registration.bind(registration.context, it->raw, *it->reference);
            it->registered = true;
            return it->raw;
        } catch (...) {
            if (it->reference) {
                // Registration may have failed; use this canonical companion
                // directly, not a lookup for an identity not yet inserted.
                release_render_command_reference(*it->reference); // erases it
            } else {
                if (it->raw) delete_native_mesh_00b74280(it->raw, meshes, 1);
                mesh_entries.erase(it);
            }
            throw;
        }
    }
    NativeMeshSectionStorage* create_section() {
        auto it = section_entries.emplace(section_entries.end(), *this);
        try {
            it->raw = create_native_mesh_section_00533fa0(
                sections.pool_010901d4, constants.maximum_00ce4970);
            if (!it->raw) throw std::bad_alloc();
            it->reference = std::make_unique<NativeMeshSectionReference>(*it->raw, sections,
                NativeMeshSectionCompanionDisposal{&*it, retire_section});
            registration.bind(registration.context, it->raw, *it->reference);
            it->registered = true;
            return it->raw;
        } catch (...) {
            if (it->reference) {
                release_render_command_reference(*it->reference); // erases it
            } else {
                if (it->raw) delete_native_mesh_section_00b86690(it->raw, sections, 1);
                section_entries.erase(it);
            }
            throw;
        }
    }
};

GuiNativeGeometryOwners::GuiNativeGeometryOwners(NativeMeshEnvironment& meshes,
    NativeMeshConstants constants, NativeMeshSectionEnvironment& sections,
    GuiNativeGeometryRegistration registration)
    : impl_(std::make_unique<Impl>(meshes, constants, sections, registration)) {}
GuiNativeGeometryOwners::~GuiNativeGeometryOwners() = default;
NativeMeshStorage* GuiNativeGeometryOwners::create_mesh() { return impl_->create_mesh(); }
NativeMeshSectionStorage* GuiNativeGeometryOwners::create_section() { return impl_->create_section(); }
NativeRenderActualOwners& GuiNativeGeometryOwners::actual_owners() noexcept {
    return impl_->registration.owners;
}
void GuiNativeGeometryOwners::construct_and_associate74_fragment(NativeModelOwner& model) {
    require(model.phase == NativeModelOwner::Phase::live &&
        &model.environment.retained_owners == &actual_owners(),
        "GUI native mesh association requires the live widget model and same owner domain");
    auto* mesh = create_mesh();
    const auto sentinel_bits = model.environment.constants.unchanged_00d7a260;
    float sentinel;
    std::memcpy(&sentinel, &sentinel_bits, sizeof(sentinel));
    try {
        set_native_model_geometry_00b75170(model, 0, mesh, sentinel, sentinel);
    } catch (...) {
        // Host allocation cleanup; the setter's already-published raw pointer
        // remains in place. This does not roll back a reentrant native setter.
        release_native_render_actual_owner(actual_owners(), mesh);
        throw;
    }
    release_native_render_actual_owner(actual_owners(), mesh);
}

GuiNativeSectionSelection acquire_gui_native_section_fragment(
    NativeMeshStorage& mesh, GuiNativeGeometryOwners& domain) {
    if (mesh.draw_sections_54.count_04 == 0) return {domain.create_section(), true};
    auto* section = static_cast<NativeMeshSectionStorage*>(mesh.draw_sections_54.data_00[0]);
    require(section != nullptr, "GUI existing native mesh section is null");
    section->references_04.fetch_add(1, std::memory_order_seq_cst);
    return {section, false};
}
void publish_gui_native_section_fragment(NativeMeshStorage& mesh,
    GuiNativeSectionSelection& selected, void*& material, NativeRenderActualOwners& owners,
    NativeMeshSectionLayoutServices& layouts) {
    require(selected.section && material && mesh.vertex_stream_count_7c > 0 &&
        mesh.vertex_streams_64[0], "GUI native publication requires actual section/material/stream0");
    set_native_mesh_section_material_00b864c0(*selected.section, owners, material);
    void* const consumed_material = std::exchange(material, nullptr);
    release_native_render_actual_owner(owners, consumed_material);
    rebuild_native_mesh_section_vertex_layout_00b865a0(*selected.section, owners, &mesh, layouts);
    require(selected.section->vertex_layout_50 != nullptr,
        "GUI native publication requires an owned actual renderer layout");
    if (selected.append_on_publish)
        append_native_mesh_draw_section_00b73c60(mesh, selected.section);
    auto* const consumed_section = std::exchange(selected.section, nullptr);
    release_native_render_actual_owner(owners, consumed_section);
}
} // namespace bsp
