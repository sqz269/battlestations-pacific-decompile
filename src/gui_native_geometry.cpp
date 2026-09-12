#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_mesh_clone.hpp"
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
    struct MaterialEntry {
        explicit MaterialEntry(Impl& owner) noexcept : domain(owner) {}
        Impl& domain;
        NativeMaterialStorage* raw{};
        std::unique_ptr<NativeMaterialReference> reference;
        bool registered{};
    };
    NativeMeshEnvironment& meshes;
    NativeMeshConstants constants;
    NativeMeshSectionEnvironment& sections;
    GuiNativeGeometryRegistration registration;
    std::list<MeshEntry> mesh_entries;
    std::list<SectionEntry> section_entries;
    std::list<MaterialEntry> material_entries;

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
        if (!mesh_entries.empty() || !section_entries.empty() || !material_entries.empty())
            std::terminate();
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
    static void retire_material(void* context, NativeMaterialReference& reference) noexcept {
        auto& entry = *static_cast<MaterialEntry*>(context);
        auto& self = entry.domain;
        if (entry.registered)
            self.registration.unbind(self.registration.context, entry.raw, reference);
        for (auto it = self.material_entries.begin(); it != self.material_entries.end(); ++it) {
            if (&*it == &entry) { self.material_entries.erase(it); return; }
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
    NativeMeshSectionStorage* clone_section(const NativeMeshSectionStorage& source) {
        auto it = section_entries.emplace(section_entries.end(), *this);
        void* allocated = nullptr;
        try {
            allocated = allocate_native_mesh_section_slot_00b85ee0(sections.pool_010901d4);
            if (!allocated) throw std::bad_alloc();
            it->raw = copy_construct_native_mesh_section_00b85ef0(allocated, source,
                registration.owners, constants.maximum_00ce4970);
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
                else if (allocated)
                    return_native_mesh_section_slot_00b85b20(sections.pool_010901d4, allocated);
                // A failed native copy constructor performs base-only unwind;
                // it is not followed by a full section destructor.
                section_entries.erase(it);
            }
            throw;
        }
    }
    NativeMaterialStorage* clone_material(const NativeMaterialStorage& source,
        NativeMaterialDestructionAccess& access, const volatile std::uint32_t* profile) {
        require(&access.retained_owners == &registration.owners,
            "GUI native material clone requires the same actual owner domain");
        require(profile && profile[0] == 0x00bd30e0 && profile[1] == 0x00b194b0,
            "GUI native material clone requires the actual current D5E520 lifetime profile");
        auto it = material_entries.emplace(material_entries.end(), *this);
        void* allocated = nullptr;
        try {
            allocated = allocate_native_material_slot_00b18780(access.material_slots);
            if (!allocated) throw std::bad_alloc();
            it->raw = clone_native_material_00b18b60(allocated, source, registration.owners);
            it->reference = std::make_unique<NativeMaterialReference>(*it->raw,
                access, profile, NativeMaterialCompanionDisposal{&*it, retire_material});
            registration.bind(registration.context, it->raw, *it->reference);
            it->registered = true;
            return it->raw;
        } catch (...) {
            if (it->reference) {
                release_render_command_reference(*it->reference); // erases it
            } else {
                if (it->raw) delete_native_material_00b194b0(it->raw, access, 1);
                else if (allocated) access.material_slots.return_slot_00b17a80(allocated);
                material_entries.erase(it);
            }
            throw;
        }
    }
    NativeMaterialStorage* create_material(NativeString& name,
        void* const volatile& renderer, NativeMaterialDestructionAccess& access,
        const volatile std::uint32_t* profile) {
        require(&access.retained_owners == &registration.owners,
            "GUI native material must use the same actual owner domain");
        auto it = material_entries.emplace(material_entries.end(), *this);
        try {
            it->raw = bsp::create_native_material_for_effect_00535320(
                name, renderer, access.material_slots, access.retained_owners);
            if (!it->raw) throw std::bad_alloc();
            it->reference = std::make_unique<NativeMaterialReference>(*it->raw,
                access, profile, NativeMaterialCompanionDisposal{&*it, retire_material});
            registration.bind(registration.context, it->raw, *it->reference);
            it->registered = true;
            return it->raw;
        } catch (...) {
            if (it->reference) {
                release_render_command_reference(*it->reference); // erases entry
            } else {
                if (it->raw) delete_native_material_00b194b0(it->raw, access, 1);
                material_entries.erase(it);
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
NativeMeshStorage* GuiNativeGeometryOwners::clone_mesh_for_text_00b742a0(
    NativeMeshStorage& source, const volatile std::uint32_t* mesh_profile,
    NativeMaterialDestructionAccess& materials,
    const volatile std::uint32_t* material_profile, NativeMeshCloneAcquired& acquired) {
    require(!acquired.mesh && !acquired.section && !acquired.material,
        "GUI Text mesh clone requires empty acquired creator storage");
    auto* const source_reference = dynamic_cast<NativeMeshReference*>(
        &actual_owners().resolve_actual(&source));
    require(source.native_vtable_00 == 0x00d62d60 && source_reference &&
        &source_reference->storage() == &source,
        "GUI Text mesh clone requires its canonical actual D62D60 mesh");
    require(mesh_profile && mesh_profile[4] == 0x00b742a0,
        "GUI Text mesh has no implementation for its current clone virtual slot");
    acquired.mesh = create_mesh();
    copy_native_mesh_for_text_00b73f50(*acquired.mesh, source, *this,
        impl_->meshes.strings, materials, material_profile, acquired);
    return acquired.mesh;
}
NativeMeshSectionStorage* GuiNativeGeometryOwners::clone_section_00b85ef0(
    const NativeMeshSectionStorage& source) { return impl_->clone_section(source); }
NativeMaterialStorage* GuiNativeGeometryOwners::clone_material_00b18b60(
    const NativeMaterialStorage& source, NativeMaterialDestructionAccess& materials,
    const volatile std::uint32_t* material_profile) {
    return impl_->clone_material(source, materials, material_profile);
}
NativeMaterialStorage* GuiNativeGeometryOwners::create_material_for_effect_00535320(
    NativeString& name, void* const volatile& renderer,
    NativeMaterialDestructionAccess& access, const volatile std::uint32_t* profile) {
    return impl_->create_material(name, renderer, access, profile);
}
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
