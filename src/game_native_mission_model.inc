// Included once, at file scope, by game_native_renderer_application.cpp.
// This is mission composition of existing native bodies, not another D3D path.
#include "bsp/game_native_mission_model.hpp"
#include "bsp/game_native_gui_effect_application.hpp"
#include "bsp/game_native_gui_sampler_application.hpp"
#include "bsp/game_native_type_storage.hpp"
#include "bsp/game_native_resource_pools.hpp"
#include "bsp/game_native_material_process.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/native_camera_owner.hpp"
#include "bsp/native_instance_geometry.hpp"
#include "bsp/native_traceline_render.hpp"
#include "bsp/native_render_group_lifetime.hpp"
#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_resource_reader_owner.hpp"
#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_resource_root_owner.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_node_raw_transform.hpp"
#include "bsp/native_node_visibility_factor.hpp"
#include "bsp/native_plain_node.hpp"
#include "bsp/native_resource_builtin_factories.hpp"
#include "bsp/native_resource_instance_bones.hpp"
#include "bsp/native_matrix_indexed_model_owner.hpp"
#include "bsp/gun_fire_points.hpp"
#include "bsp/structured_hierarchy.hpp"
#include "bsp/structured_resource.hpp"
#include "bsp/camera_multiply.hpp"
#include <algorithm>
#include <cstring>
#include <iterator>
#include <list>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace bsp::game {
namespace mission_model_detail {
using Word = std::uint32_t;
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
template<class T> const volatile T& cell(GameNativeReadOnlyData& data, Word address) {
    return *static_cast<const volatile T*>(data.data_at(address, sizeof(T)));
}
const volatile Word* table(GameNativeReadOnlyData& data, Word address, std::size_t bytes) {
    return static_cast<const volatile Word*>(data.data_at(address, bytes));
}
template<class T> T read(const void* actual, std::size_t offset = 0) {
    T result;
    std::memcpy(&result, static_cast<const unsigned char*>(actual) + offset, sizeof result);
    return result;
}
bool named(void* node, const char* expected) {
    const auto length = read<Word>(node, 0x10);
    const auto* name = read<const char*>(node, 0x14);
    return name && length == std::strlen(expected) && std::memcmp(name, expected, length) == 0;
}
float float_word(const volatile Word& actual) {
    const Word word = actual;
    float result;
    std::memcpy(&result, &word, sizeof result);
    return result;
}
std::vector<std::pair<Word,unsigned char>> read_plane_cockpit_notes(
    const std::vector<std::uint8_t>& bytes) {
    // Source metadata only. Resource ordinal and Note text preserve the
    // B89E90/71AD50 association; geometry still follows the actual loader.
    auto stream=std::make_shared<MemoryStream>(
        memory_stream_from_complete_bytes(bytes.data(),bytes.size()));
    StructuredReader reader(stream);
    auto root=reader.read_root_00bea700();
    require(root && ::_stricmp(root->tag().c_str(),"MMOD")==0 && root->read_control_00be9a40(),
        "plane visibility metadata requires a complete MMOD root");
    std::vector<std::pair<Word,unsigned char>> notes;
    Word ordinal{};
    while(root->has_remaining_00715bf0()) {
        auto section=root->read_child_00bea680();
        require(section!=nullptr,"plane visibility metadata has an invalid root child");
        if(::_stricmp(section->tag().c_str(),"Resource")==0) {
            while(section->has_remaining_00715bf0()) {
                auto entry=section->read_child_00bea680();
                require(entry!=nullptr,"plane visibility metadata has an invalid resource");
                if(::_stricmp(entry->tag().c_str(),"Note")==0) {
                    std::string text;
                    require(read_note_text_00718f50_fragment(*entry,text),
                        "plane visibility metadata has an invalid Note");
                    if(text=="belsocockpit") notes.emplace_back(ordinal,1);
                    else if(text=="kulsocockpit") notes.emplace_back(ordinal,2);
                }
                require(entry->skip_00be9c40() && entry->close(),
                    "plane visibility metadata could not close a resource");
                ++ordinal;
            }
        } else require(section->skip_00be9c40(),"plane visibility metadata could not skip a root child");
        require(section->close(),"plane visibility metadata could not close a root child");
    }
    require(root->close() && reader.error()==StructuredReaderError::none,
        "plane visibility metadata reader failed");
    return notes;
}
} // namespace mission_model_detail

struct GameNativeMissionModel::Impl final : NativeRenderGroupModels {
    using Word = std::uint32_t;
    enum class Phase { ready, loading, loaded, retiring, retired, failed, closed };
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& files;
    GameNativeReadOnlyData& data;
    GameNativeVfsRawServices raw;
    NativeRenderActualOwnerRegistry& owners;
    GuiTextNativeRendererServices& graphics;
    const GameNativeMeshFieldServices& fields;
    LightTypeBootstrap node_types;
    ModelTypeBootstrap model_types;
    // These are two borrowing interfaces over the SAME native pool, types,
    // node runtime and strings. B4C8D0's existing interface requires the typed
    // ActualNativeStringPoolStorage name service; post-effect raw construction
    // requires the raw-AA8/AA4/AA0 form. Neither owns or copies native storage.
    NativeModelEnvironment raw_models;
    NativeModelEnvironment instance_models;
    NativeMaterialDestructionAccess materials;
    std::unique_ptr<GameNativeGuiSamplerApplication> samplers;
    std::unique_ptr<GameNativeGuiEffectApplication> effects;
    NativeInstanceGeneratorOwners generators;
    std::unique_ptr<NativeMeshSubsetLoadingContext> subsets;
    std::unique_ptr<NativeMeshLoadingContext> loading;
    NativeNodeMappedRawTransformProfiles transform_profiles;
    std::unique_ptr<NativeMatrixIndexedModelEnvironment> matrix_models;

    struct ModelRecord {
        Impl* domain;
        void* actual;
        std::unique_ptr<NativeModelOwner> owner;
        std::unique_ptr<NativeModelReference> reference;
        bool registered{};
        NativeString caller_name;
        bool caller_name_live{};
        std::size_t drawable_index{static_cast<std::size_t>(-1)};
    };
    struct StreamRecord {
        Impl* domain;
        void* actual;
        std::unique_ptr<NativeLogicalVertexReference> reference;
        bool registered{};
    };
    std::list<ModelRecord> models;
    struct Unit;
    struct BoneRecord {
        Impl* domain{};
        Unit* unit{};
        void* actual{};
        std::unique_ptr<NativeMatrixIndexedModelOwner> owner;
        std::unique_ptr<NativeMatrixIndexedModelReference> reference;
        bool registered{}, caller_name_live{};
        NativeString caller_name;
        std::size_t drawable_index{static_cast<std::size_t>(-1)};
    };
    struct PlainRecord {
        Impl* domain{};
        NativeNodeStorage* actual{};
        std::optional<NativeNodeBinding> binding;
        std::optional<NativePlainNodeReference> reference;
        bool registered{}, caller_name_live{};
        NativeString caller_name;
    };
    std::list<BoneRecord> bone_models;
    std::list<PlainRecord> bone_nodes;
    std::list<StreamRecord> streams;
    NativeInstanceGeometryAccess instance;
    struct RenderServices final : NativeTracelineRenderServices {
        GameNativeMissionModel& facade;
        explicit RenderServices(GameNativeMissionModel& source) noexcept :facade(source) {}
        const volatile Word* profile(const void* actual) override {return facade.profile(actual);}
        const void* call_virtual48(void*,Word) override {
            throw std::logic_error("mission model reached unsupported native virtual48");
        }
        Word call_virtual58(void*,Word,void*,float) override {
            throw std::logic_error("mission model reached unsupported native virtual58");
        }
        void call_virtual20(void*,Word,void*,float,float,Word) override {
            throw std::logic_error("mission model reached unsupported native virtual20");
        }
        void collect_entry_00b1dff0(void*,void*) override {
            throw std::logic_error("mission model collection requires the ordinary command's actual collector");
        }
    } render_profiles;

    struct MeshCall {
        // B94710 publishes pair[0] and B944E0 writes pair[1]. Initial zeros are
        // source bookkeeping and are not supplied as native stack preimages.
        Word pair[2]{};
        NativeMeshLoadingAcquired acquired;
        std::uint32_t resource_ordinal{};
        bool matrix_indexed{};
        bool creator_consumed{};
    };
    struct Drawable {
        void* actual;
        RenderCommandReference* companion;
        GeneratedModelNodeLifetime* node_lifetime;
        std::size_t unit_index;
        std::size_t hierarchy_index;
        MeshCall* mesh;
        bool creator_consumed{}, retired{};
    };
    struct Asset {
        struct PlaneDefaults {
            std::optional<std::size_t> inner_cockpit,outer_cockpit;
            std::vector<std::size_t> broken_wings;
            std::vector<std::size_t> inherited_hidden;
            std::vector<unsigned char> visible;
        };
        std::string name;
        std::vector<std::unique_ptr<MeshCall>> meshes;
        std::vector<HierarchyItem> hierarchy;
        std::vector<std::pair<Word,unsigned char>> cockpit_notes;
        std::optional<PlaneDefaults> plane_defaults;
        std::vector<CameraMatrix> world_matrices;
        std::vector<unsigned char> matrix_states;
        std::size_t sections{};
        bool matrix_indexed{};
        bool complete{}, reader_live{}, name_live{}, open_creator_live{};
        NativeString actual_name;
        NativeVfsNameResolutionAcquired resolution;
        alignas(4) unsigned char reader[native_structured_resource_reader_bytes];
        void* root{};
        void* container{};
        void* child{};
        void* opened_stream{};
    };
    struct Unit {
        Asset* asset{};
        std::size_t first_drawable{};
        std::size_t drawable_count{};
        bool complete{};
        bool plane_defaults_configured{};
        std::vector<void*> hierarchy_nodes;
        std::vector<std::pair<std::size_t,void*>> rest_nodes;
        std::vector<RenderCommandReference*> palette_retains;
        std::size_t palette_models_remaining{};
    };
    std::vector<std::unique_ptr<Asset>> assets;
    std::unordered_map<std::size_t, std::unique_ptr<Unit>> units;
    std::vector<Drawable> drawables;
    std::vector<void*> attempted_models; // Opaque allocated identities, including failed preparation.
    std::size_t sections{}, completed_units{};
    Phase phase{Phase::ready};
    bool entered{}, failed{};

    Impl(GameNativeMissionModel& facade,GameHostLog& l, GameNativeRendererApplication& app, GameVfsHost& vfs,
        GameNativeReadOnlyData& mapped)
        : log(l), renderer(app), files(vfs), data(mapped), raw(vfs.borrow_raw_services()),
          owners(app.actual_owners()), graphics(app.game_grid_context().graphics),
          fields(app.borrow_mesh_field_services()),
          node_types(vfs.native_owners().types(), vfs.native_types().light_types()),
          model_types(vfs.native_owners().types(), node_types, vfs.native_types().model_types()),
          raw_models{game_native_resource_pool_process().model_pool_01090054(),
              app.camera_environment().nodes, model_types, owners,
              {mission_model_detail::cell<Word>(data,0x00ce4adc),
               mission_model_detail::cell<Word>(data,0x00ce4970),
               mission_model_detail::cell<Word>(data,0x00d7a208),
               mission_model_detail::cell<Word>(data,0x00d7a260)},
              mission_model_detail::table(data,0x00d62de8,0x5c),
              mission_model_detail::table(data,0x00d62c88,0x58), nullptr},
          instance_models{raw_models.pool_01090054, raw_models.nodes, model_types,
              owners, raw_models.constants, raw_models.vtable_00d62de8,
              raw_models.vtable_00d62c88, &raw.strings},
          materials{owners, raw.strings, game_native_material_process().parameter_pool_00f8d3e4(),
              game_native_material_process().material_pool_00f8d3ac()},
          generators(app.mission_instance_generator_context()),
          transform_profiles(data),
          instance{instance_models,graphics.streams.geometry,graphics.streams.vertices,
              materials,raw.strings,raw_models.constants.unchanged_00d7a260,
              app.borrow_gui_material_destruction_profile_00d5e520(),this,
              &prepare_model,&retire_failed_model,&bind_model,&bind_stream},render_profiles(facade) {
        mission_model_detail::require(&fields.metadata.reads == &fields.textures.reads &&
            &fields.textures.owners == &owners && &fields.textures.textures == &app.texture_cache() &&
            &instance_models.nodes == &app.camera_environment().nodes,
            "mission model requires the application's same native reader/geometry/owner domains");
    }
    void bind_effects(GameSingletonHost& singletons,GameInputRuntime& input) {
        samplers=std::make_unique<GameNativeGuiSamplerApplication>(log,renderer,files,singletons,data,
            GameNativeGuiSamplerApplicationBindings{input,renderer.borrow_platform_publication_0109cf04()});
        effects=std::make_unique<GameNativeGuiEffectApplication>(log,renderer,files,samplers->effect_bindings());
        subsets=std::make_unique<NativeMeshSubsetLoadingContext>(NativeMeshSubsetLoadingContext{
            graphics.streams.geometry,materials,effects->native_effect_cache(),fields.textures,
            fields.lighting,generators,renderer.borrow_gui_material_destruction_profile_00d5e520()});
        loading=std::make_unique<NativeMeshLoadingContext>(NativeMeshLoadingContext{*subsets,fields.metadata});
    }

    static NativeModelOwner& prepare_model(void* context, void* actual, NativeModelEnvironment& env) {
        auto& self = *static_cast<Impl*>(context);
        mission_model_detail::require(&env == &self.raw_models || &env == &self.instance_models,
            "mission model preparation cannot retarget native environment");
        self.models.push_back({&self,actual,{},{},false});
        auto it = std::prev(self.models.end());
        try { it->owner = std::make_unique<NativeModelOwner>(actual,NativeModelPool::slot_bytes,env); }
        catch (...) { self.models.erase(it); throw; }
        return *it->owner;
    }
    static void retire_failed_model(void* context, NativeModelOwner& owner) noexcept {
        auto& self = *static_cast<Impl*>(context);
        for (auto it=self.models.begin(); it!=self.models.end(); ++it)
            if (it->owner.get()==&owner && !it->reference &&
                (owner.phase==NativeModelOwner::Phase::prepared || owner.phase==NativeModelOwner::Phase::dead)) {
                self.models.erase(it); return;
            }
        std::terminate();
    }
    static void retire_model(void* context, NativeModelReference& reference) noexcept {
        auto& record=*static_cast<ModelRecord*>(context);
        auto& self=*record.domain;
        if(record.drawable_index<self.drawables.size()) {
            auto& drawable=self.drawables[record.drawable_index];
            if(drawable.companion!=&reference || drawable.actual!=record.actual) std::terminate();
            drawable.creator_consumed=true;drawable.retired=true;
        }
        if (record.registered) self.owners.unbind(record.actual,reference);
        for (auto it=self.models.begin(); it!=self.models.end(); ++it)
            if (&*it==&record) { self.models.erase(it); return; }
        std::terminate();
    }
    static NativeModelReference& bind_model(void* context, NativeModelOwner& owner) {
        auto& self=*static_cast<Impl*>(context);
        for (auto& record:self.models) if (record.owner.get()==&owner) {
            mission_model_detail::require(!record.reference,"mission model cannot bind a second companion");
            record.reference=std::make_unique<NativeModelReference>(owner,
                NativeModelCompanionDisposal{&record,&retire_model});
            self.owners.bind(record.actual,*record.reference);
            record.registered=true;
            return *record.reference;
        }
        throw std::logic_error("mission model companion requires its prepared actual slot");
    }
    static void retire_bone_model(void* context, NativeMatrixIndexedModelReference& reference) noexcept {
        auto& record=*static_cast<BoneRecord*>(context);
        auto& self=*record.domain;
        auto* const unit=record.unit;
        if(record.drawable_index<self.drawables.size()) {
            auto& drawable=self.drawables[record.drawable_index];
            if(drawable.companion!=&reference || drawable.actual!=record.actual) std::terminate();
            drawable.creator_consumed=true;drawable.retired=true;
        }
        if(record.registered) self.owners.unbind(record.actual,reference);
        for(auto it=self.bone_models.begin();it!=self.bone_models.end();++it)
            if(&*it==&record) {
                self.bone_models.erase(it);
                if(!unit || unit->palette_models_remaining==0) {
                    self.failed=true;self.phase=Phase::failed;return;
                }
                if(--unit->palette_models_remaining==0) self.retire_unit_palette(*unit);
                return;
            }
        std::terminate();
    }
    void retire_unit_palette(Unit& unit) noexcept {
        // This runs from the LAST physical matrix-model disposal callback,
        // during renderer/cache retirement while raw names/pools still admit
        // node terminals. Never defer these operations past all singletons.
        while(!unit.palette_retains.empty()) {
            auto* const reference=unit.palette_retains.back();
            unit.palette_retains.pop_back(); // consume this real +04 credit first
            release_render_command_reference(*reference);
        }
        while(!unit.rest_nodes.empty()) {
            auto* const actual=unit.rest_nodes.back().second;
            NativePlainNodeReference* reference{};
            for(auto& record:bone_nodes) if(record.actual==actual && record.registered && record.reference) {
                reference=&*record.reference;break;
            }
            if(!reference) {failed=true;phase=Phase::failed;return;}
            unit.rest_nodes.pop_back(); // the terminal may erase its record
            reference->release_model_virtual18_00b6f310();
        }
    }
    static void retire_bone_node(void* context, NativePlainNodeReference& reference) noexcept {
        auto& record=*static_cast<PlainRecord*>(context);
        auto& self=*record.domain;
        if(record.registered) self.owners.unbind(record.actual,reference);
        for(auto it=self.bone_nodes.begin();it!=self.bone_nodes.end();++it)
            if(&*it==&record) {self.bone_nodes.erase(it);return;}
        std::terminate();
    }
    void construct_caller_name(NativeString& header, bool& live, const std::string& name) {
        live=true;
        mission_model_detail::require(name.size()<=0xffffffffu,
            "installed hierarchy name exceeds the native string domain");
        resize_native_string_header_0041dd40(&header,files.raw_strings(),static_cast<Word>(name.size()),true);
        if(!name.empty()) std::memmove(header.data(),name.data(),name.size());
    }
    void* materialize_bone_node(Unit& unit, std::size_t hierarchy_index) {
        auto& pools=game_native_resource_pool_process();
        (void)pools.initialize_plain_node_once_00cd7d10();
        auto* pool=pools.plain_node_pool_storage_0108ff58();
        bone_nodes.emplace_back();auto& record=bone_nodes.back();record.domain=this;
        const auto& item=unit.asset->hierarchy.at(hierarchy_index);
        construct_caller_name(record.caller_name,record.caller_name_live,item.name);
        record.actual=create_native_plain_node_00b866c0(pool,&record.caller_name,
            files.raw_strings(),renderer.node_constants());
        mission_model_detail::require(record.actual!=nullptr,"native authored bone node allocation is null");
        auto& nodes=raw_models.nodes;
        record.binding.emplace(*record.actual,nodes.node_virtual_0c,set_node_scene_00b6ed80,nullptr);
        record.binding->scene_attachment.world_changed=native_node_world_changed_00b6dbe0;
        nodes.scenes.bind(record.binding->scene_attachment);
        record.reference.emplace(*record.binding,nodes,files.raw_strings(),pool,raw_models.vtable_00d62c88,
            NativePlainNodeCompanionDisposal{&record,&retire_bone_node});
        owners.bind(record.actual,*record.reference);record.registered=true;
        record.caller_name_live=false;destroy_native_string_header_0041dd20(&record.caller_name,files.raw_strings());
        unit.hierarchy_nodes.at(hierarchy_index)=record.actual;
        unit.rest_nodes.emplace_back(hierarchy_index,record.actual);
        set_raw_local_matrix_00b6db10(record.actual,transform_profiles,
            &authored_world(*unit.asset,hierarchy_index));
        return record.actual;
    }
    static void retire_stream(void* context, NativeLogicalVertexReference& reference) noexcept {
        auto& record=*static_cast<StreamRecord*>(context);
        auto& self=*record.domain;
        if(record.registered) self.owners.unbind(record.actual,reference);
        for(auto it=self.streams.begin();it!=self.streams.end();++it)
            if(&*it==&record) {self.streams.erase(it);return;}
        std::terminate();
    }
    static NativeLogicalVertexReference& bind_stream(void* context,void* actual,
        NativeLogicalVertexOwnerContext& env) {
        auto& self=*static_cast<Impl*>(context);
        mission_model_detail::require(&env==&self.graphics.streams.vertices,
            "mission instance stream requires the canonical vertex domain");
        self.streams.push_back({&self,actual,{},false});
        auto& record=self.streams.back();
        record.reference=std::make_unique<NativeLogicalVertexReference>(actual,env,
            NativeLogicalVertexCompanionDisposal{&record,&retire_stream});
        self.owners.bind(actual,*record.reference);
        record.registered=true;
        return *record.reference;
    }
    NativeModelReference& resolve_actual_model(void* actual) override {
        for(auto& record:models) if(record.actual==actual && record.registered) return *record.reference;
        throw std::logic_error("render group contains a model outside the mission canonical companion domain");
    }
    GeneratedModelNodeLifetime& resolve_actual_model_node(void* actual) override {
        const auto current=mission_model_detail::read<Word>(actual);
        mission_model_detail::require(current==0x00d62de8 || current==0x00d63590,
            "render group contains an unsupported native model profile");
        auto& reference=owners.resolve_actual(actual);
        auto* node=dynamic_cast<GeneratedModelNodeLifetime*>(&reference);
        mission_model_detail::require(node && node->transform().raw_node_key()==reinterpret_cast<Word>(actual),
            "render group model does not have its canonical actual node lifetime");
        return *node;
    }
    const CameraMatrix& authored_world(Asset& asset, std::size_t index) {
        auto& hierarchy=asset.hierarchy;
        auto& matrix_states=asset.matrix_states;
        auto& world_matrices=asset.world_matrices;
        mission_model_detail::require(index<hierarchy.size() && hierarchy[index].matrix.has_value(),
            "installed visual hierarchy requires its authored matrix");
        if(matrix_states[index]==2) return world_matrices[index];
        mission_model_detail::require(matrix_states[index]==0,"installed visual hierarchy is cyclic");
        matrix_states[index]=1;
        CameraMatrix local;
        static_assert(sizeof(local)==sizeof(*hierarchy[index].matrix));
        std::memcpy(&local,hierarchy[index].matrix->data(),sizeof local);
        const auto parent=hierarchy[index].parent;
        if(parent==0xffffffffu) world_matrices[index]=local;
        else {
            mission_model_detail::require(parent<hierarchy.size(),"installed hierarchy parent is out of range");
            multiply_camera_matrices_00413920(world_matrices[index],local,authored_world(asset,parent));
        }
        matrix_states[index]=2;
        return world_matrices[index];
    }
    void materialize_matrix_drawable(Unit& unit, std::size_t unit_index,
        std::size_t index, MeshCall& call) {
        if(!matrix_models) {
            initialize_native_matrix_indexed_model_type_00cd8570(files.native_owners().types(),model_types);
            (void)initialize_matrix_indexed_model_pool_once_00cd8550();
            matrix_models=std::make_unique<NativeMatrixIndexedModelEnvironment>(NativeMatrixIndexedModelEnvironment{
                raw_models,matrix_indexed_model_pool_01090390(),
                mission_model_detail::table(data,0x00d63590,0x5c),
                native_matrix_indexed_model_type_ids_010903c8()});
        }
        bone_models.emplace_back();auto& record=bone_models.back();record.domain=this;record.unit=&unit;
        record.actual=matrix_models->pool_01090390.allocate_raw_slot_00b90ac0();
        mission_model_detail::require(record.actual!=nullptr,"native matrix-indexed model allocation is null");
        record.owner=std::make_unique<NativeMatrixIndexedModelOwner>(record.actual,
            NativeMatrixIndexedModelPool::slot_bytes,*matrix_models);
        const auto& item=unit.asset->hierarchy[index];
        construct_caller_name(record.caller_name,record.caller_name_live,item.name);
        construct_native_matrix_indexed_model_00b91220(*record.owner,&record.caller_name,renderer.node_constants());
        record.reference.reset(new NativeMatrixIndexedModelReference(*record.owner,{&record,&retire_bone_model}));
        owners.bind(record.actual,*record.reference);record.registered=true;
        ++unit.palette_models_remaining;
        drawables.push_back({record.actual,record.reference.get(),record.reference.get(),unit_index,index,&call,false,false});
        record.drawable_index=drawables.size()-1;++unit.drawable_count;
        record.caller_name_live=false;destroy_native_string_header_0041dd20(&record.caller_name,files.raw_strings());
        const float sentinel=mission_model_detail::float_word(raw_models.constants.unchanged_00d7a260);
        set_native_model_geometry_00b75170(record.owner->base,0,
            reinterpret_cast<void*>(call.pair[0]),sentinel,sentinel);
        std::memcpy(static_cast<unsigned char*>(record.actual)+8,item.sphere.data(),sizeof(item.sphere));
        record.owner->base.storage.node.auxiliary_flags_138 &= ~0x30u;
        set_raw_local_matrix_00b6db10(record.actual,transform_profiles,&authored_world(*unit.asset,index));
        mission_model_detail::require(unit.hierarchy_nodes.at(index)==nullptr,
            "matrix-indexed hierarchy item has ambiguous native visual nodes");
        unit.hierarchy_nodes[index]=record.actual;
    }
    void bind_matrix_bones(Unit& unit) {
        const auto& hierarchy=unit.asset->hierarchy;
        for(std::size_t at=unit.first_drawable;at<unit.first_drawable+unit.drawable_count;++at) {
            auto& drawable=drawables[at];
            if(!drawable.mesh->matrix_indexed) continue;
            auto* names=get_native_mesh_weight_names_00b72730(reinterpret_cast<void*>(drawable.mesh->pair[0]));
            mission_model_detail::require(names && names->count_04>=0 &&
                (names->count_04==0 || names->data_00),"matrix-indexed mesh has invalid native WeightMapNames");
            resize_native_model_bone_nodes_00b90e30(drawable.actual,names->count_04);
            for(std::int32_t bone=0;bone<names->count_04;++bone) {
                const auto& name=names->data_00[bone];
                std::size_t matched=hierarchy.size();
                for(std::size_t node=0;node<hierarchy.size();++node) {
                    if(hierarchy[node].name.size()!=name.length_00) continue;
                    if(name.length_00 && (!name.data_04 || ::_stricmp(hierarchy[node].name.c_str(),name.data_04)!=0)) continue;
                    matched=node;break; // Original B87E80: first equal-length CRT-insensitive match.
                }
                mission_model_detail::require(matched<hierarchy.size(),
                    "matrix-indexed WeightMapNames entry has no authored hierarchy node");
                auto* actual=unit.hierarchy_nodes[matched];
                if(!actual) actual=materialize_bone_node(unit,matched);
                else if(std::none_of(unit.rest_nodes.begin(),unit.rest_nodes.end(),
                    [actual](const auto& rest) {return rest.second==actual;})) {
                    mission_model_detail::require(mission_model_detail::read<Word>(actual)==0x00d62de8,
                        "matrix-indexed palette has an unsupported model-to-model dependency");
                    auto& reference=owners.resolve_actual(actual);
                    mission_model_detail::require(static_cast<void*>(&reference.reference_count)==
                        static_cast<unsigned char*>(actual)+4,"palette node must borrow its actual +04 credit");
                    if(std::find(unit.palette_retains.begin(),unit.palette_retains.end(),&reference)==unit.palette_retains.end()) {
                        unit.palette_retains.push_back(&reference);
                        retain_render_command_reference(reference);
                    }
                }
                set_native_model_bone_node_00b90600(drawable.actual,nullptr,static_cast<Word>(bone),actual);
            }
            log.notef("native mission matrix palette: unit=%zu asset=%s model=%p bones=%d rest_nodes=%zu path=B90E30/B90600 pose=authored-rest",
                drawable.unit_index,unit.asset->name.c_str(),drawable.actual,names->count_04,unit.rest_nodes.size());
        }
    }
    const Asset::PlaneDefaults& prepare_plane_defaults(Asset& asset) {
        if(asset.plane_defaults) return *asset.plane_defaults;
        const auto& hierarchy=asset.hierarchy;
        mission_model_detail::require(!hierarchy.empty() && hierarchy[0].parent==0xffffffffu,
            "plane creator defaults require the original first root hierarchy item");
        Asset::PlaneDefaults defaults;
        defaults.visible.assign(hierarchy.size(),1);
        // B891A0 publishes Note/node pairs in hierarchy/resource order. 71AD50
        // selects the FIRST exact case-sensitive Note name, not a node label.
        for(std::size_t item=0;item<hierarchy.size();++item)
            for(const auto resource:hierarchy[item].resources)
                for(const auto& note:asset.cockpit_notes) if(note.first==resource) {
                    if(note.second==1 && !defaults.inner_cockpit) defaults.inner_cockpit=item;
                    if(note.second==2 && !defaults.outer_cockpit) defaults.outer_cockpit=item;
                }
        if(defaults.inner_cockpit) defaults.visible[*defaults.inner_cockpit]=0;
        if(defaults.outer_cockpit) defaults.visible[*defaults.outer_cockpit]=1;
        std::vector<std::vector<std::size_t>> children(hierarchy.size());
        for(std::size_t item=1;item<hierarchy.size();++item) {
            const auto parent=hierarchy[item].parent;
            if(parent==0xffffffffu) continue;
            mission_model_detail::require(parent<hierarchy.size(),
                "plane visibility hierarchy parent is out of range");
            children[parent].push_back(item);
        }
        // B6E010 prepends each authored child. B6F9A0 walks that native sibling
        // order, visits descendants before their parent, and excludes the root.
        std::vector<std::pair<std::size_t,bool>> pending{{0,false}};
        while(!pending.empty()) {
            const auto entry=pending.back();pending.pop_back();
            const auto item=entry.first;
            if(!entry.second) {
                pending.emplace_back(item,true);
                for(const auto child:children[item]) pending.emplace_back(child,false);
            } else if(item!=0 && std::strstr(hierarchy[item].name.c_str(),"szarny letorik")) {
                defaults.broken_wings.push_back(item);defaults.visible[item]=0;
            }
        }
        // B6D990 gates/multiplies the parent's factor before child render20.
        // Actual mission nodes are flattened, so preserve that static gate in
        // each affected descendant's immutable mask instead of another graph.
        for(std::size_t item=0;item<hierarchy.size();++item) {
            auto parent=hierarchy[item].parent;
            std::size_t depth{};
            bool inherited=false;
            while(parent!=0xffffffffu) {
                mission_model_detail::require(parent<hierarchy.size() && depth++<hierarchy.size(),
                    "plane visibility hierarchy is invalid or cyclic");
                if(!defaults.visible[parent]) inherited=true;
                parent=hierarchy[parent].parent;
            }
            if(inherited && defaults.visible[item]) {
                defaults.visible[item]=0;defaults.inherited_hidden.push_back(item);
            }
        }
        asset.plane_defaults.emplace(std::move(defaults));
        return *asset.plane_defaults;
    }
    void publish_hierarchy_visibility(Unit& unit,std::size_t unit_index,
        std::size_t item,bool visible) {
        for(std::size_t at=unit.first_drawable;at<unit.first_drawable+unit.drawable_count;++at) {
            auto& drawable=drawables.at(at);
            if(drawable.hierarchy_index!=item) continue;
            mission_model_detail::require(!drawable.retired && drawable.unit_index==unit_index,
                "plane default drawable changed identity");
            set_native_node_visibility_factor_00b6da70(
                static_cast<NativeNodeStorage*>(drawable.actual),nullptr,visible?1.0f:0.0f,0);
        }
        for(const auto& node:unit.rest_nodes) if(node.first==item)
            set_native_node_visibility_factor_00b6da70(
                static_cast<NativeNodeStorage*>(node.second),nullptr,visible?1.0f:0.0f,0);
    }
    void materialize_drawables(Unit& unit, std::size_t unit_index) {
        auto& asset=*unit.asset;
        const auto& hierarchy=asset.hierarchy;
        unit.first_drawable=drawables.size();
        if(asset.matrix_indexed) unit.hierarchy_nodes.resize(hierarchy.size());
        for(std::size_t index=0;index<hierarchy.size();++index) {
            const auto& item=hierarchy[index];
            for(auto& call:asset.meshes) {
                if(std::find(item.resources.begin(),item.resources.end(),call->resource_ordinal)==item.resources.end()) continue;
                if(call->matrix_indexed) {materialize_matrix_drawable(unit,unit_index,index,*call);continue;}
                attempted_models.push_back(nullptr); // Publish bookkeeping before acquiring a slot.
                auto* actual=raw_models.pool_01090054.allocate_raw_slot_00b74d00();
                attempted_models.back()=actual;
                if(!actual) throw std::bad_alloc();
                NativeModelOwner* owner=&prepare_model(this,actual,raw_models);
                auto& record=models.back();
                // Persist this caller header BEFORE native name construction.
                // A failed name/model/registration call preserves this same
                // record and raw slot for process retention, without replay.
                record.caller_name_live=true;
                mission_model_detail::require(item.name.size()<=0xffffffffu,
                    "installed hierarchy name exceeds the native string domain");
                resize_native_string_header_0041dd40(&record.caller_name,files.raw_strings(),
                    static_cast<Word>(item.name.size()),true);
                if(!item.name.empty()) std::memmove(record.caller_name.data(),item.name.data(),item.name.size());
                construct_native_model_00b75030(*owner,&record.caller_name,renderer.node_constants());
                auto& reference=bind_model(this,*owner);
                drawables.push_back({actual,&reference,&reference,unit_index,index,call.get(),false,false});
                record.drawable_index=drawables.size()-1;
                ++unit.drawable_count;
                record.caller_name_live=false;
                destroy_native_string_header_0041dd20(&record.caller_name,files.raw_strings());
                const float sentinel=mission_model_detail::float_word(raw_models.constants.unchanged_00d7a260);
                set_native_model_geometry_00b75170(*owner,0,
                    reinterpret_cast<void*>(call->pair[0]),sentinel,sentinel);
                // Explicit authored-state adapter: actual model local sphere
                // at08 and asset-relative authored matrix. Unit association is
                // host bookkeeping only; no simulation state overlays the owner.
                std::memcpy(static_cast<unsigned char*>(actual)+8,item.sphere.data(),sizeof(item.sphere));
                owner->storage.node.auxiliary_flags_138 &= ~0x30u;
                set_raw_local_matrix_00b6db10(actual,transform_profiles,&authored_world(asset,index));
                if(asset.matrix_indexed && !unit.hierarchy_nodes[index]) unit.hierarchy_nodes[index]=actual;
            }
        }
        mission_model_detail::require(unit.drawable_count!=0,"installed model has no authored Mesh visual associations");
        if(asset.matrix_indexed) bind_matrix_bones(unit);
        // Retain each asset mesh's creator until mission retirement. Later
        // spawns can borrow this SAME geometry even after earlier units hide.
    }
    bool open_asset_calls() const noexcept {
        for(const auto& asset:assets)
            if(asset->reader_live || asset->name_live || asset->open_creator_live) return true;
        return false;
    }
};

GameNativeMissionModel::GameNativeMissionModel(GameHostLog& log,GameNativeRendererApplication& renderer,
    GameVfsHost& files,GameSingletonHost& singletons,GameNativeReadOnlyData& data,GameInputRuntime& input)
    :impl_(std::make_unique<Impl>(*this,log,renderer,files,data)) {
    // Publish the outer retention root before an exported sampler binding can
    // acquire child obligations. An entered construction failure keeps every
    // borrowed environment and operation alive, including half-bound graphs.
    try {impl_->bind_effects(singletons,input);}
    catch(...) {impl_->failed=true;impl_.release();throw;}
}
GameNativeMissionModel::~GameNativeMissionModel() noexcept {
    if(impl_ && (requires_process_retention() || impl_->phase!=Impl::Phase::closed ||
        !impl_->models.empty() || !impl_->bone_models.empty() || !impl_->bone_nodes.empty() ||
        !impl_->streams.empty())) impl_.release();
}
NativeMaterialEffectCacheContext& GameNativeMissionModel::effect_cache() {return impl_->effects->native_effect_cache();}
NativeMaterialDestructionAccess& GameNativeMissionModel::material_lifetime() noexcept {return impl_->materials;}
NativeModelEnvironment& GameNativeMissionModel::model_environment() noexcept {return impl_->raw_models;}
NativeLuaServiceBindings& GameNativeMissionModel::lua_services() noexcept {return impl_->renderer.lua_service_binding();}
NativeInstanceGeometryAccess& GameNativeMissionModel::instance_geometry() noexcept {return impl_->instance;}
NativeRenderGroupModels& GameNativeMissionModel::group_models() noexcept {return *impl_;}
NativeTracelineRenderServices& GameNativeMissionModel::render_services() noexcept {return impl_->render_profiles;}

const volatile std::uint32_t* GameNativeMissionModel::profile(const void* actual) const {
    const auto address=mission_model_detail::read<std::uint32_t>(actual);
    std::size_t bytes{};
    switch(address) {
    case 0x00d62de8: case 0x00d63590: bytes=0x5c;break;
    case 0x00d62c88: bytes=0x58;break;
    case 0x00d62cf0: bytes=0x58;break;
    case 0x00d62d60: bytes=0x14;break;
    case 0x00d61d6c: bytes=0x28;break;
    case 0x00d61bfc: case 0x00d61c1c: bytes=0x0c;break;
    case 0x00d619f8: bytes=8;break;
    case 0x00d61a00: bytes=0x18;break;
    default: throw std::logic_error("mission rendering reached an unsupported current native profile");
    }
    return mission_model_detail::table(impl_->data,address,bytes);
}

bool GameNativeMissionModel::load_installed_unit(const std::string& logical_name, std::size_t unit_index) {
    using namespace mission_model_detail;
    auto& p=*impl_;
    require((p.phase==Impl::Phase::ready || p.phase==Impl::Phase::loaded) &&
        !logical_name.empty() && logical_name.find('\0')==std::string::npos,
        "mission unit load requires a healthy domain and a VFS logical name");
    const auto existing=p.units.find(unit_index);
    if(existing!=p.units.end()) {
        require(existing->second->complete && existing->second->asset->name==logical_name,
            "mission unit index cannot be rebound or replay an incomplete load");
        return false;
    }
    p.phase=Impl::Phase::loading;p.entered=true;
    Impl::Asset* asset{};
    try {
        for(auto& candidate:p.assets) if(candidate->name==logical_name) {
            require(candidate->complete,"mission asset load cannot replay");
            asset=candidate.get();break;
        }
        const bool new_asset=asset==nullptr;
        if(new_asset) {
            p.assets.push_back(std::make_unique<Impl::Asset>());
            asset=p.assets.back().get();asset->name=logical_name;
        }
        auto inserted=p.units.emplace(unit_index,std::make_unique<Impl::Unit>());
        auto& unit=*inserted.first->second;unit.asset=asset;
        auto& a=*asset;
        NativeLuaServiceBindings::Activation active(p.renderer.lua_service_binding());
        if(new_asset) {
            // The existing source hierarchy reader supplies authored association
            // metadata only. Every visual mesh/buffer/material below uses actual
            // native storage and the original VFS/renderer/cache domains.
            std::string resolved=a.name,error;
            require(p.files.resolve_existing(resolved),"installed unit could not resolve in the active VFS");
            auto opened=p.files.open(resolved,2);
            require(opened.provider_opened && opened.stream && opened.stream->fully_initialized(),
                "installed unit hierarchy source is unavailable");
            const auto size=opened.stream->size_00bef600();
            require(size>0 && size<=0xffffffffll,"installed unit source size is outside the native DWORD domain");
            const auto* first=opened.stream->data_00bef610();
            std::vector<std::uint8_t> bytes(first,first+static_cast<std::size_t>(size));
            if(!read_mmod_hierarchy_items(bytes,a.hierarchy,error)) throw std::runtime_error(error);
            a.cockpit_notes=read_plane_cockpit_notes(bytes);
            a.world_matrices.resize(a.hierarchy.size());a.matrix_states.resize(a.hierarchy.size());
            construct_native_string_header_0041e870(&a.actual_name,p.files.raw_strings(),a.name.c_str());a.name_live=true;
            require(resolve_native_vfs_existing_name_00bdf4c0(p.raw.actual_vfs_publication_0109ceec,
                &a.actual_name,p.raw.name_resolution,a.resolution),"native installed unit name did not resolve");
            auto* manager=p.raw.actual_vfs_publication_0109ceec;
            const auto* manager_table=reinterpret_cast<const volatile std::uint32_t*>(read<std::uint32_t>(manager));
            a.opened_stream=p.raw.bindings.open_manager_entry(manager_table[1],manager,&a.actual_name,2);
            require(a.opened_stream!=nullptr,"native installed unit stream is null");a.open_creator_live=true;
            auto& reads=p.fields.metadata.reads;
            construct_native_structured_resource_reader_00bea150(a.reader,reads.strings,reads.streams);a.reader_live=true;
            assign_native_resource_reader_stream_00bf0430(a.reader,a.opened_stream,reads.streams);
            a.open_creator_live=false;
            void* stream_slot=a.opened_stream;
            release_native_resource_slot_00483850(&stream_slot,reads.streams);
            create_native_resource_root_00bea700(a.reader,&a.root,reads);
            require(a.root && named(a.root,"MMOD"),"installed unit root is not MMOD");
            read_native_resource_node_control_00be9a40(&a.root,reads);
            std::uint32_t ordinal{};
            while(native_resource_node_has_remaining_00715bf0(&a.root)) {
                create_native_resource_child_00bea680(&a.root,&a.container,reads);
                if(named(a.container,"Resource")) {
                    while(native_resource_node_has_remaining_00715bf0(&a.container)) {
                        create_native_resource_child_00bea680(&a.container,&a.child,reads);
                        if(named(a.child,"Mesh") || named(a.child,"MatrixIndexedMesh")) {
                            a.meshes.push_back(std::make_unique<Impl::MeshCall>());
                            auto& call=*a.meshes.back();call.resource_ordinal=ordinal;
                            call.matrix_indexed=named(a.child,"MatrixIndexedMesh");
                            a.matrix_indexed=a.matrix_indexed || call.matrix_indexed;
                            auto* mesh=static_cast<NativeMeshStorage*>(construct_native_mesh_from_node_00b94710(
                                call.pair,&a.child,*p.loading,call.acquired));
                            require(mesh!=nullptr,"native installed unit mesh allocation is null");
                            a.sections+=static_cast<std::size_t>(mesh->draw_sections_54.count_04);
                        } else {
                            require(!named(a.child,"SkinedMesh"),
                                "installed unit requires an unsupported native skinned-model record/palette path");
                            skip_native_resource_node_00be9c40(&a.child,reads);
                        }
                        release_native_structured_node_handle_00be9ed0(&a.child,reads.streams);
                        ++ordinal;
                    }
                } else skip_native_resource_node_00be9c40(&a.container,reads);
                release_native_structured_node_handle_00be9ed0(&a.container,reads.streams);
            }
            release_native_structured_node_handle_00be9ed0(&a.root,reads.streams);
            a.reader_live=false;destroy_native_structured_resource_reader_00be9f10(a.reader,reads.strings,reads.streams);
            a.name_live=false;destroy_native_string_header_0041dd20(&a.actual_name,p.files.raw_strings());
            a.complete=true;p.sections+=a.sections;
        }
        p.materialize_drawables(unit,unit_index);unit.complete=true;++p.completed_units;p.phase=Impl::Phase::loaded;
        p.log.notef("native mission unit loaded unit=%zu asset=%s native_meshes=%zu native_sections=%zu drawables=%zu shared_asset=%u",
            unit_index,a.name.c_str(),a.meshes.size(),a.sections,unit.drawable_count,new_asset?0u:1u);
        return true;
    } catch(...) {
        p.failed=true;p.phase=Impl::Phase::failed;
        const auto* mesh=!asset || asset->meshes.empty()?nullptr:&asset->meshes.back()->acquired;
        const auto* subset=mesh && !mesh->fields.subsets.empty()?mesh->fields.subsets.back().get():nullptr;
        p.log.notef("native mission unit failed unit=%zu asset=%s drawables=%zu mesh_site=%08x subset_site=%08x retention=process",
            unit_index,logical_name.c_str(),p.drawables.size(),mesh?mesh->native_site:0u,subset?subset->native_site:0u);
        throw;
    }
}
void GameNativeMissionModel::load_installed_ship(const char* logical_name) {
    mission_model_detail::require(logical_name && *logical_name && impl_->units.empty(),
        "mission ship diagnostic requires an empty domain");
    load_installed_unit(logical_name,0);
}
bool GameNativeMissionModel::unit_loaded(std::size_t index) const noexcept {
    const auto found=impl_->units.find(index);
    return impl_->phase==Impl::Phase::loaded && found!=impl_->units.end() && found->second->complete;
}
std::size_t GameNativeMissionModel::unit_count() const noexcept {return impl_->completed_units;}
std::size_t GameNativeMissionModel::unit_drawable_count(std::size_t index) const noexcept {
    const auto found=impl_->units.find(index);
    return found!=impl_->units.end() && found->second->complete?found->second->drawable_count:0;
}
std::size_t GameNativeMissionModel::drawable_unit_index(std::size_t index) const {
    return impl_->drawables.at(index).unit_index;
}
std::size_t GameNativeMissionModel::unit_hierarchy_count(std::size_t index) const noexcept {
    const auto found=impl_->units.find(index);
    return found!=impl_->units.end() && found->second->complete?found->second->asset->hierarchy.size():0;
}
std::size_t GameNativeMissionModel::drawable_hierarchy_index(std::size_t index) const {
    return impl_->drawables.at(index).hierarchy_index;
}
bool GameNativeMissionModel::update_unit_transform(std::size_t index, const CameraMatrix& transform) {
    auto& p=*impl_;
    const auto found=p.units.find(index);
    if(found==p.units.end()) return false;
    auto& unit=*found->second;
    mission_model_detail::require(p.phase==Impl::Phase::loaded && unit.complete,
        "mission unit transform requires completed live loading");
    try {
        for(std::size_t at=unit.first_drawable;at<unit.first_drawable+unit.drawable_count;++at) {
            auto& drawable=p.drawables.at(at);
            mission_model_detail::require(!drawable.retired && drawable.unit_index==index,
                "mission unit drawable range changed identity");
            CameraMatrix local;
            multiply_camera_matrices_00413920(local,p.authored_world(*unit.asset,drawable.hierarchy_index),transform);
            // Actual B6DB10 copies local, clears world/bounds flags, notifies
            // attached +A0/current virtual3C, then invalidates descendants.
            // Never write cached world matrices or maintain a simulation pose.
            set_raw_local_matrix_00b6db10(drawable.actual,p.transform_profiles,&local);
        }
        for(const auto& node:unit.rest_nodes) {
            CameraMatrix local;
            multiply_camera_matrices_00413920(local,p.authored_world(*unit.asset,node.first),transform);
            set_raw_local_matrix_00b6db10(node.second,p.transform_profiles,&local);
        }
        return true;
    } catch(...) {
        p.failed=true;p.phase=Impl::Phase::failed;
        p.log.notef("native mission unit transform failed unit=%zu asset=%s retention=process",
            index,unit.asset->name.c_str());
        throw;
    }
}
bool GameNativeMissionModel::update_hierarchy_transform(std::size_t index,
    std::size_t hierarchy_item, const CameraMatrix& composed_world) {
    auto& p=*impl_;
    const auto found=p.units.find(index);
    if(found==p.units.end()) return false;
    auto& unit=*found->second;
    mission_model_detail::require(p.phase==Impl::Phase::loaded && unit.complete &&
        hierarchy_item<unit.asset->hierarchy.size(),
        "mission hierarchy transform requires a completed original hierarchy item");
    try {
        bool published=false;
        for(std::size_t at=unit.first_drawable;at<unit.first_drawable+unit.drawable_count;++at) {
            auto& drawable=p.drawables.at(at);
            if(drawable.hierarchy_index!=hierarchy_item) continue;
            mission_model_detail::require(!drawable.retired && drawable.unit_index==index,
                "mission hierarchy drawable changed identity");
            set_raw_local_matrix_00b6db10(drawable.actual,p.transform_profiles,&composed_world);
            published=true;
        }
        for(const auto& node:unit.rest_nodes) if(node.first==hierarchy_item) {
            set_raw_local_matrix_00b6db10(node.second,p.transform_profiles,&composed_world);
            published=true;
        }
        return published;
    } catch(...) {
        p.failed=true;p.phase=Impl::Phase::failed;
        p.log.notef("native mission hierarchy transform failed unit=%zu item=%zu asset=%s retention=process",
            index,hierarchy_item,unit.asset->name.c_str());
        throw;
    }
}
bool GameNativeMissionModel::set_unit_visible(std::size_t index, bool visible) {
    auto& p=*impl_;
    const auto found=p.units.find(index);
    if(found==p.units.end()) return false;
    const auto& unit=*found->second;
    mission_model_detail::require(p.phase==Impl::Phase::loaded && unit.complete,
        "mission unit visibility requires completed live loading");
    try {
        for(std::size_t at=unit.first_drawable;at<unit.first_drawable+unit.drawable_count;++at) {
            auto& drawable=p.drawables.at(at);
            mission_model_detail::require(!drawable.retired && drawable.unit_index==index,
                "mission unit drawable range changed identity");
            const bool part_visible=!unit.plane_defaults_configured ||
                unit.asset->plane_defaults->visible.at(drawable.hierarchy_index)!=0;
            set_native_node_visibility_factor_00b6da70(
                static_cast<NativeNodeStorage*>(drawable.actual),nullptr,visible && part_visible?1.0f:0.0f,0);
        }
        return true;
    } catch(...) {
        p.failed=true;p.phase=Impl::Phase::failed;
        p.log.notef("native mission unit visibility failed unit=%zu asset=%s retention=process",
            index,unit.asset->name.c_str());
        throw;
    }
}
bool GameNativeMissionModel::configure_plane_default_visibility(std::size_t index) {
    auto& p=*impl_;
    const auto found=p.units.find(index);
    if(found==p.units.end()) return false;
    auto& unit=*found->second;
    mission_model_detail::require(p.phase==Impl::Phase::loaded && unit.complete,
        "plane defaults require a completed loaded unit");
    if(unit.plane_defaults_configured) return true;
    try {
        const auto& defaults=p.prepare_plane_defaults(*unit.asset);
        unit.plane_defaults_configured=true;
        if(defaults.inner_cockpit) p.publish_hierarchy_visibility(unit,index,*defaults.inner_cockpit,false);
        if(defaults.outer_cockpit) p.publish_hierarchy_visibility(unit,index,*defaults.outer_cockpit,true);
        for(const auto item:defaults.broken_wings) p.publish_hierarchy_visibility(unit,index,item,false);
        for(const auto item:defaults.inherited_hidden) p.publish_hierarchy_visibility(unit,index,item,false);
        p.log.notef("native mission plane defaults: unit=%zu asset=%s inner=%u outer=%u broken=%zu path=7D741E/7D745D/7D76D3",
            index,unit.asset->name.c_str(),defaults.inner_cockpit?1u:0u,defaults.outer_cockpit?1u:0u,
            defaults.broken_wings.size());
        return true;
    } catch(...) {
        p.failed=true;p.phase=Impl::Phase::failed;
        p.log.notef("native mission plane defaults failed unit=%zu asset=%s retention=process",
            index,unit.asset->name.c_str());
        throw;
    }
}
std::size_t GameNativeMissionModel::drawable_count() const noexcept {return impl_->drawables.size();}
void* GameNativeMissionModel::drawable(std::size_t index) const {
    const auto& drawable=impl_->drawables.at(index);
    mission_model_detail::require(!drawable.retired,"mission drawable has already retired");
    return drawable.actual;
}
RenderCommandReference& GameNativeMissionModel::drawable_reference(std::size_t index) const {
    const auto& drawable=impl_->drawables.at(index);
    mission_model_detail::require(!drawable.retired,"mission drawable companion has already retired");
    auto& reference=impl_->owners.resolve_actual(drawable.actual);
    mission_model_detail::require(&reference==drawable.companion,"mission drawable identity was reused");
    return reference;
}
std::size_t GameNativeMissionModel::section_count() const noexcept {return impl_->sections;}
const char* GameNativeMissionModel::installed_name() const noexcept {
    return impl_->assets.empty()?"":impl_->assets.front()->name.c_str();
}
bool GameNativeMissionModel::requires_process_retention() const noexcept {
    const auto& p=*impl_;
    // Healthy canonical models/layouts remain alive until the ordinary native
    // drain. Only incomplete acquisition/retirement prohibits that drain.
    for(const auto& record:p.models)
        if(!record.registered || record.caller_name_live) return true;
    for(const auto& record:p.bone_models)
        if(!record.registered || record.caller_name_live) return true;
    for(const auto& record:p.bone_nodes)
        if(!record.registered || record.caller_name_live) return true;
    for(const auto& record:p.streams) if(!record.registered) return true;
    return p.failed || p.phase==Impl::Phase::loading || p.phase==Impl::Phase::retiring ||
        p.open_asset_calls() ||
        (p.effects && p.effects->requires_process_retention()) ||
        (p.samplers && p.samplers->requires_process_retention());
}
void GameNativeMissionModel::retire_drawables() {
    auto& p=*impl_;
    mission_model_detail::require(p.phase==Impl::Phase::ready || p.phase==Impl::Phase::loaded,
        "mission model retirement requires a healthy completed domain");
    p.phase=Impl::Phase::retiring;
    for(auto& drawable:p.drawables) {
        if(drawable.retired) continue;
        drawable.creator_consumed=true;
        auto* registered=p.owners.find(drawable.actual);
        mission_model_detail::require(registered==drawable.companion,
            "mission drawable changed canonical owner identity before retirement");
        drawable.node_lifetime->release_model_virtual18_00b6f310();
        mission_model_detail::require(!p.failed,"native matrix palette retirement failed");
    }
    for(auto& asset:p.assets) for(auto& call:asset->meshes) {
        mission_model_detail::require(!call->creator_consumed,"mission asset mesh creator cannot retire twice");
        call->creator_consumed=true;
        release_native_render_actual_owner(p.owners,reinterpret_cast<void*>(call->pair[0]));
    }
    mission_model_detail::require(!p.failed,"native model retirement failed");
    p.phase=Impl::Phase::retired;
}
void GameNativeMissionModel::after_native_renderer_drain() {
    auto& p=*impl_;
    mission_model_detail::require(!p.failed && p.models.empty() && p.bone_models.empty() && p.streams.empty() &&
        !p.open_asset_calls(),"mission model retains native work after drain");
    mission_model_detail::require(p.bone_nodes.empty(),"matrix-model retirement retained palette-node creators");
    for(const auto& entry:p.units) {
        const auto& unit=*entry.second;
        mission_model_detail::require(unit.palette_models_remaining==0 &&
            unit.palette_retains.empty() && unit.rest_nodes.empty(),
            "matrix-model retirement retained unit palette obligations");
    }
    p.effects->after_native_renderer_drain();p.samplers->after_native_renderer_drain();p.phase=Impl::Phase::closed;
}
} // namespace bsp::game
