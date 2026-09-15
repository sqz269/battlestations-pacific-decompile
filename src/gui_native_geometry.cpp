#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_mesh_clone.hpp"
#include "bsp/native_vertex_declaration_loading.hpp"
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
    struct VertexEntry {
        explicit VertexEntry(Impl& owner) noexcept : domain(owner) {}
        Impl& domain;
        void* raw{};
        std::unique_ptr<NativeLogicalVertexReference> reference;
        bool registered{};
    };
    struct IndexEntry {
        explicit IndexEntry(Impl& owner) noexcept : domain(owner) {}
        Impl& domain;
        void* raw{};
        std::unique_ptr<NativeLogicalIndexReference> reference;
        bool registered{};
    };
    struct DeclarationEntry {
        explicit DeclarationEntry(Impl& owner) noexcept : domain(owner) {}
        Impl& domain;
        void* raw{};
        std::unique_ptr<NativeVertexDeclarationReference> reference;
        bool registered{};
    };
    NativeMeshEnvironment& meshes;
    NativeMeshConstants constants;
    NativeMeshSectionEnvironment& sections;
    GuiNativeGeometryRegistration registration;
    std::list<MeshEntry> mesh_entries;
    std::list<SectionEntry> section_entries;
    std::list<MaterialEntry> material_entries;
    std::list<VertexEntry> vertex_entries;
    std::list<IndexEntry> index_entries;
    std::list<DeclarationEntry> declaration_entries;

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
        if (!mesh_entries.empty() || !section_entries.empty() || !material_entries.empty() ||
            !vertex_entries.empty() || !index_entries.empty() || !declaration_entries.empty())
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
    static void retire_vertex(void* context, NativeLogicalVertexReference& reference) noexcept {
        auto& entry = *static_cast<VertexEntry*>(context);
        auto& self = entry.domain;
        if (entry.registered) self.registration.unbind(self.registration.context, entry.raw, reference);
        for (auto it = self.vertex_entries.begin(); it != self.vertex_entries.end(); ++it)
            if (&*it == &entry) { self.vertex_entries.erase(it); return; }
        std::terminate();
    }
    static void retire_index(void* context, NativeLogicalIndexReference& reference) noexcept {
        auto& entry = *static_cast<IndexEntry*>(context);
        auto& self = entry.domain;
        if (entry.registered) self.registration.unbind(self.registration.context, entry.raw, reference);
        for (auto it = self.index_entries.begin(); it != self.index_entries.end(); ++it)
            if (&*it == &entry) { self.index_entries.erase(it); return; }
        std::terminate();
    }
    static void retire_declaration(void* context, NativeVertexDeclarationReference& reference) noexcept {
        auto& entry = *static_cast<DeclarationEntry*>(context);
        auto& self = entry.domain;
        if (entry.registered) self.registration.unbind(self.registration.context, entry.raw, reference);
        for (auto it = self.declaration_entries.begin(); it != self.declaration_entries.end(); ++it)
            if (&*it == &entry) { self.declaration_entries.erase(it); return; }
        std::terminate();
    }
    void register_declaration(GuiNativeDeclarationAcquired& acquired,
        NativeVertexDeclarationLoadingContext& loading) {
        require(acquired.reference && !acquired.companion && !acquired.canonical_registration,
            "declaration registration requires one acquired actual reference");
        require(registration.find != nullptr,
            "declaration registration requires explicit same-domain canonical lookup");
        auto* const existing = registration.find(registration.context, acquired.reference);
        if (existing) {
            auto* declaration = dynamic_cast<NativeVertexDeclarationReference*>(existing);
            require(declaration && declaration->storage() == acquired.reference &&
                declaration->matches_context(loading.pool_0108fd38, loading.type_sizes_00d61cc0,
                    loading.declaration_vtable_00d61d1c),
                "cached declaration requires its canonical companion and same terminal context");
            acquired.companion = declaration;
            acquired.canonical_registration = true;
            acquired.reused_companion = true;
            return;
        }
        // An earlier failed transactional bind can leave an unregistered
        // companion. Never construct a second one for that actual reference.
        for (auto& entry : declaration_entries)
            require(entry.raw != acquired.reference,
                "declaration has an interrupted companion registration; resolve it before another load");
        auto it = declaration_entries.emplace(declaration_entries.end(), *this);
        it->raw = acquired.reference;
        try {
            it->reference = std::make_unique<NativeVertexDeclarationReference>(it->raw,
                loading.pool_0108fd38, loading.type_sizes_00d61cc0,
                loading.declaration_vtable_00d61d1c,
                NativeVertexDeclarationCompanionDisposal{&*it, retire_declaration});
        } catch (...) { declaration_entries.erase(it); throw; }
        acquired.companion = it->reference.get();
        registration.bind(registration.context, it->raw, *it->reference);
        it->registered = true;
        acquired.canonical_registration = true;
    }
    void register_stream(NativeStreamCloneAcquired& acquired, NativeStreamCloneServices& services) {
        require(acquired.creator && !acquired.companion && !acquired.canonical_registration,
            "stream registration requires one unbound actual creator");
        if (acquired.vertex) {
            auto it = vertex_entries.emplace(vertex_entries.end(), *this);
            it->raw = acquired.creator;
            try {
                it->reference = std::make_unique<NativeLogicalVertexReference>(it->raw, services.vertices,
                    NativeLogicalVertexCompanionDisposal{&*it, retire_vertex});
            } catch (...) { vertex_entries.erase(it); throw; } // No native effect or creator release.
            acquired.companion = it->reference.get();
            registration.bind(registration.context, it->raw, *it->reference);
            it->registered = true;
        } else {
            auto it = index_entries.emplace(index_entries.end(), *this);
            it->raw = acquired.creator;
            try {
                it->reference = std::make_unique<NativeLogicalIndexReference>(it->raw, services.indices,
                    NativeLogicalIndexCompanionDisposal{&*it, retire_index});
            } catch (...) { index_entries.erase(it); throw; }
            acquired.companion = it->reference.get();
            registration.bind(registration.context, it->raw, *it->reference);
            it->registered = true;
        }
        acquired.canonical_registration = true;
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
    NativeMeshSectionStorage* create_native_section(GuiNativeSectionAcquired& acquired) {
        require(!acquired.started && !acquired.creator && !acquired.companion &&
            !acquired.owner_record && !acquired.registered,
            "Native section admission requires one fresh acquired frame");
        acquired.started = true;
        auto it = section_entries.emplace(section_entries.end(), *this);
        try {
            it->raw = create_native_mesh_section_00533fa0(
                sections.pool_010901d4, constants.maximum_00ce4970);
        } catch (...) {
            section_entries.erase(it); // No completed native creator exists.
            throw;
        }
        if (!it->raw) { section_entries.erase(it); return nullptr; }
        acquired.creator = it->raw;
        acquired.owner_record = &*it;
        it->reference = std::make_unique<NativeMeshSectionReference>(*it->raw, sections,
            NativeMeshSectionCompanionDisposal{&*it, retire_section});
        acquired.companion = it->reference.get();
        registration.bind(registration.context, it->raw, *it->reference);
        it->registered = true;
        acquired.registered = true;
        return it->raw;
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
    void register_material_creator(NativeMaterialFactoryAcquired& acquired,
        NativeMaterialDestructionAccess& access, const volatile std::uint32_t* profile) {
        require(acquired.phase == NativeMaterialFactoryAcquired::Phase::complete &&
            acquired.material && !acquired.companion && !acquired.owner_record && !acquired.registered &&
            &access.retained_owners == &registration.owners && registration.find &&
            !registration.find(registration.context, acquired.material),
            "Native material admission requires one completed unregistered creator");
        auto it = material_entries.emplace(material_entries.end(), *this);
        it->raw = acquired.material;
        try {
            it->reference = std::make_unique<NativeMaterialReference>(*it->raw,
                access, profile, NativeMaterialCompanionDisposal{&*it, retire_material});
        } catch (...) {
            // Erase only unconstructed host metadata; raw native creator stays.
            material_entries.erase(it);
            throw;
        }
        acquired.owner_record = &*it;
        acquired.companion = it->reference.get();
        registration.bind(registration.context, it->raw, *it->reference);
        it->registered = true;
        acquired.registered = true;
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
NativeMeshSectionStorage* GuiNativeGeometryOwners::create_native_section_00533fa0(
    GuiNativeSectionAcquired& acquired) { return impl_->create_native_section(acquired); }
NativeMeshStorage* GuiNativeGeometryOwners::clone_mesh_for_text_00b742a0(
    NativeMeshStorage& source, const volatile std::uint32_t* mesh_profile,
    NativeMaterialDestructionAccess& materials,
    const volatile std::uint32_t* material_profile, NativeMeshCloneAcquired& acquired,
    NativeStreamCloneServices* streams) {
    require(!acquired.mesh && !acquired.section && !acquired.material && !acquired.stream.creator &&
        !acquired.stream.companion && (acquired.stream.phase == NativeStreamClonePhase::empty ||
        acquired.stream.phase == NativeStreamClonePhase::consumed),
        "GUI Text mesh clone requires empty acquired creator storage");
    auto* const source_reference = dynamic_cast<NativeMeshReference*>(
        &actual_owners().resolve_actual(&source));
    require(source.native_vtable_00 == 0x00d62d60 && source_reference &&
        &source_reference->storage() == &source,
        "GUI Text mesh clone requires its canonical actual D62D60 mesh");
    require(mesh_profile && mesh_profile[4] == 0x00b742a0,
        "GUI Text mesh has no implementation for its current clone virtual slot");
    acquired.mesh = create_mesh();
    if (streams) {
        copy_native_mesh_for_text_00b73f50_flags3e(*acquired.mesh, source, *this,
            impl_->meshes.strings, materials, material_profile, acquired, *streams);
    } else {
        copy_native_mesh_for_text_00b73f50(*acquired.mesh, source, *this,
            impl_->meshes.strings, materials, material_profile, acquired);
    }
    return acquired.mesh;
}
void GuiNativeGeometryOwners::register_stream_clone_creator(NativeStreamCloneAcquired& acquired,
    NativeStreamCloneServices& services) {
    require(&services.geometry == this && &services.vertices.actual_owners == &impl_->registration.owners,
        "stream companion registration must use the same canonical owner domain");
    impl_->register_stream(acquired, services);
}
void GuiNativeGeometryOwners::register_native_declaration_reference(GuiNativeDeclarationAcquired& acquired,
    NativeVertexDeclarationLoadingContext& loading) {
    impl_->register_declaration(acquired, loading);
}
const GuiNativeGeometryRegistration& GuiNativeGeometryOwners::registration() const noexcept {
    return impl_->registration;
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
void GuiNativeGeometryOwners::register_native_material_creator(NativeMaterialFactoryAcquired& acquired,
    NativeMaterialDestructionAccess& access, const volatile std::uint32_t* profile) {
    impl_->register_material_creator(acquired, access, profile);
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
