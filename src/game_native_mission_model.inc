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
#include "bsp/gun_fire_points.hpp"
#include "bsp/structured_hierarchy.hpp"
#include "bsp/camera_multiply.hpp"
#include <algorithm>
#include <cstring>
#include <iterator>
#include <list>
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
        bool creator_consumed{};
    };
    struct Drawable {
        void* actual;
        NativeModelReference* companion;
        std::size_t unit_index;
        std::size_t hierarchy_index;
        bool creator_consumed{}, retired{};
    };
    struct Asset {
        std::string name;
        std::vector<std::unique_ptr<MeshCall>> meshes;
        std::vector<HierarchyItem> hierarchy;
        std::vector<CameraMatrix> world_matrices;
        std::vector<unsigned char> matrix_states;
        std::size_t sections{};
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
    void materialize_drawables(Unit& unit, std::size_t unit_index) {
        auto& asset=*unit.asset;
        const auto& hierarchy=asset.hierarchy;
        unit.first_drawable=drawables.size();
        for(std::size_t index=0;index<hierarchy.size();++index) {
            const auto& item=hierarchy[index];
            for(auto& call:asset.meshes) {
                if(std::find(item.resources.begin(),item.resources.end(),call->resource_ordinal)==item.resources.end()) continue;
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
                drawables.push_back({actual,&reference,unit_index,index,false,false});
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
            }
        }
        mission_model_detail::require(unit.drawable_count!=0,"installed model has no authored Mesh visual associations");
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
        !impl_->models.empty() || !impl_->streams.empty())) impl_.release();
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
    case 0x00d62de8: bytes=0x5c;break;
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
                        if(named(a.child,"Mesh")) {
                            a.meshes.push_back(std::make_unique<Impl::MeshCall>());
                            auto& call=*a.meshes.back();call.resource_ordinal=ordinal;
                            auto* mesh=static_cast<NativeMeshStorage*>(construct_native_mesh_from_node_00b94710(
                                call.pair,&a.child,*p.loading,call.acquired));
                            require(mesh!=nullptr,"native installed unit mesh allocation is null");
                            a.sections+=static_cast<std::size_t>(mesh->draw_sections_54.count_04);
                        } else skip_native_resource_node_00be9c40(&a.child,reads);
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
        return true;
    } catch(...) {
        p.failed=true;p.phase=Impl::Phase::failed;
        p.log.notef("native mission unit transform failed unit=%zu asset=%s retention=process",
            index,unit.asset->name.c_str());
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
            set_native_node_visibility_factor_00b6da70(
                static_cast<NativeNodeStorage*>(drawable.actual),nullptr,visible?1.0f:0.0f,0);
        }
        return true;
    } catch(...) {
        p.failed=true;p.phase=Impl::Phase::failed;
        p.log.notef("native mission unit visibility failed unit=%zu asset=%s retention=process",
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
NativeModelReference& GameNativeMissionModel::drawable_reference(std::size_t index) const {
    const auto& drawable=impl_->drawables.at(index);
    mission_model_detail::require(!drawable.retired,"mission drawable companion has already retired");
    auto& reference=impl_->resolve_actual_model(drawable.actual);
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
        drawable.companion->release_model_virtual18_00b6f310();
    }
    for(auto& asset:p.assets) for(auto& call:asset->meshes) {
        mission_model_detail::require(!call->creator_consumed,"mission asset mesh creator cannot retire twice");
        call->creator_consumed=true;
        release_native_render_actual_owner(p.owners,reinterpret_cast<void*>(call->pair[0]));
    }
    p.phase=Impl::Phase::retired;
}
void GameNativeMissionModel::after_native_renderer_drain() {
    auto& p=*impl_;
    mission_model_detail::require(!p.failed && p.models.empty() && p.streams.empty() &&
        !p.open_asset_calls(),"mission model retains native work after drain");
    p.effects->after_native_renderer_drain();p.samplers->after_native_renderer_drain();p.phase=Impl::Phase::closed;
}
} // namespace bsp::game
