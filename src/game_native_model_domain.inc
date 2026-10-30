// Included once by game_native_renderer_application.cpp, before Mission clients.
#include "bsp/game_native_model_domain.hpp"
#include "bsp/game_native_material_process.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_resource_pools.hpp"
#include "bsp/game_native_type_storage.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/native_camera_owner.hpp"
#include "bsp/native_model_storage_lifetime.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include <iterator>
#include <list>
#include <optional>
#include <stdexcept>

namespace bsp::game {
namespace model_domain_detail {
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
template<class T> const volatile T& cell(GameNativeReadOnlyData& data, std::uint32_t address) {
    return *static_cast<const volatile T*>(data.data_at(address, sizeof(T)));
}
const volatile std::uint32_t* table(GameNativeReadOnlyData& data,
    std::uint32_t address, std::size_t bytes) {
    return static_cast<const volatile std::uint32_t*>(data.data_at(address, bytes));
}
} // namespace model_domain_detail

struct GameNativeModelDomain::Impl {
    using Word = std::uint32_t;
    GameNativeRendererApplication& renderer;
    GameVfsHost& files;
    GameNativeVfsRawServices raw;
    NativeRenderActualOwnerRegistry& owners;
    GuiTextNativeRendererServices& graphics;
    const NativeNodeRawConstants& node_constants;
    LightTypeBootstrap node_types;
    ModelTypeBootstrap model_types;
    // Distinct immutable name interfaces over the SAME native providers.
    NativeModelEnvironment raw_models;
    NativeModelEnvironment instance_models;
    NativeMaterialDestructionAccess materials;
    const volatile Word* material_profile;
    struct ModelRecord {
        GameNativeModelClient::Impl* client;
        void* actual;
        GameNativeModelAcquisition acquisition;
        NativeModelStorageLifetimeFrame terminal_frame{};
        NativeModelStorageLifetimeAcquired terminal;
        NativeModelRawHostLifetime host;
        std::optional<NativeModelRawLifetimeBinding> raw_lifetime;
        std::unique_ptr<NativeModelOwner> owner;
        std::unique_ptr<NativeModelReference> reference;
        bool registered{};
        ModelRecord(GameNativeModelClient::Impl& source, void* slot) noexcept
            : client(&source), actual(slot) {}
    };
    struct StreamRecord {
        GameNativeModelClient::Impl* client;
        void* actual;
        std::unique_ptr<NativeLogicalVertexReference> reference;
        bool registered{};
    };
    std::list<ModelRecord> models;
    std::list<StreamRecord> streams;
    bool drained{};

    Impl(GameNativeRendererApplication& app, GameVfsHost& vfs, GameNativeReadOnlyData& data)
        : renderer(app), files(vfs), raw(vfs.borrow_raw_services()), owners(app.actual_owners()),
          graphics(app.game_grid_context().graphics), node_constants(app.node_constants()),
          node_types(vfs.native_owners().types(), vfs.native_types().light_types()),
          model_types(vfs.native_owners().types(), node_types, vfs.native_types().model_types()),
          raw_models{game_native_resource_pool_process().model_pool_01090054(),
              app.camera_environment().nodes, model_types, owners,
              {model_domain_detail::cell<Word>(data,0x00ce4adc),
               model_domain_detail::cell<Word>(data,0x00ce4970),
               model_domain_detail::cell<Word>(data,0x00d7a208),
               model_domain_detail::cell<Word>(data,0x00d7a260)},
              model_domain_detail::table(data,0x00d62de8,0x5c),
              model_domain_detail::table(data,0x00d62c88,0x58), nullptr},
          instance_models{raw_models.pool_01090054, raw_models.nodes, model_types,
              owners, raw_models.constants, raw_models.vtable_00d62de8,
              raw_models.vtable_00d62c88, &raw.strings},
          materials{owners, raw.strings, game_native_material_process().parameter_pool_00f8d3e4(),
              game_native_material_process().material_pool_00f8d3ac()},
          material_profile(app.borrow_gui_material_destruction_profile_00d5e520()) {
        model_domain_detail::require(model_types.storage().guard_01090030 != 0 &&
            &raw_models.nodes.require_raw_name_pool()==&vfs.raw_strings() &&
            &graphics.streams.geometry.actual_owners()==&owners &&
            &graphics.streams.geometry.registration().owners==&owners &&
            &raw_models.nodes==&app.camera_environment().nodes &&
            &node_constants.negative_bound_00ce4adc==&raw_models.constants.minimum_00ce4adc &&
            &node_constants.positive_bound_00ce4970==&raw_models.constants.maximum_00ce4970,
            "Model domain requires completed original type startup and the renderer's same providers");
    }
    void require_page_storage(NativeModelStorageContext& storage) {
        const auto scene_domain=renderer.startup_scene_domain();
        model_domain_detail::require(!drained && raw_models.actual_names==nullptr &&
            &storage.pool_01090054==&raw_models.pool_01090054 &&
            &storage.constants==&node_constants &&
            &storage.negative_zero_00d7a208==&raw_models.constants.negative_zero_00d7a208 &&
            &storage.node.strings==&files.raw_strings() &&
            &storage.node.strings==&raw_models.nodes.require_raw_name_pool() &&
            &storage.node.scenes.owners==&owners && &storage.node.trees.owners==&owners &&
            &storage.node.trees.point_lights==&raw_models.nodes.point_lights &&
            &storage.node.scenes.increment_00ce221c==&scene_domain.node.scenes.increment_00ce221c &&
            &storage.node.scenes.decrement_00ce2220==&scene_domain.node.scenes.decrement_00ce2220 &&
            &storage.node.trees.decrement_00ce2220==&scene_domain.node.trees.decrement_00ce2220,
            "Page Model client requires the renderer's exact raw lifetime/string/import domain");
    }
    bool empty_for(const GameNativeModelClient::Impl* client) const noexcept {
        for (const auto& record:models) if (record.client==client) return false;
        for (const auto& record:streams) if (record.client==client) return false;
        return true;
    }
    bool retains_for(const GameNativeModelClient::Impl* client) const noexcept {
        for (const auto& record:models) if (!client || record.client==client) {
            if (!record.registered || record.acquisition.caller_name_live || record.host.failed ||
                (record.terminal.started && !record.terminal.complete) ||
                (record.terminal.scalar_started && !record.terminal.scalar_complete)) return true;
        }
        for (const auto& record:streams)
            if ((!client || record.client==client) && !record.registered) return true;
        return false;
    }
};

struct GameNativeModelClient::Impl {
    GameNativeModelDomain::Impl& domain;
    NativeModelStorageContext* const raw_storage;
    const GameNativeModelRetirementObserver observer;
    NativeInstanceGeometryAccess raw;
    NativeInstanceGeometryAccess instance;
    Impl(GameNativeModelDomain::Impl& owner, NativeModelStorageContext* storage,
        GameNativeModelRetirementObserver notification)
        : domain(owner), raw_storage(storage), observer(notification),
          raw{owner.raw_models,owner.graphics.streams.geometry,owner.graphics.streams.vertices,
              owner.materials,owner.raw.strings,owner.raw_models.constants.unchanged_00d7a260,
              owner.material_profile,this,&prepare_model,&retire_failed_model,&bind_model,&bind_stream},
          instance{owner.instance_models,owner.graphics.streams.geometry,owner.graphics.streams.vertices,
              owner.materials,owner.raw.strings,owner.raw_models.constants.unchanged_00d7a260,
              owner.material_profile,this,&prepare_model,&retire_failed_model,&bind_model,&bind_stream} {}

    static NativeModelOwner& prepare_model(void* context, void* actual, NativeModelEnvironment& env) {
        auto& self=*static_cast<Impl*>(context);
        auto& domain=self.domain;
        model_domain_detail::require(!domain.drained && (&env==&domain.raw_models ||
            (!self.raw_storage && &env==&domain.instance_models)),
            "Model client preparation cannot retarget its environment or lifetime");
        for (const auto& record:domain.models)
            model_domain_detail::require(record.actual!=actual,"Model slot already has common owner metadata");
        domain.models.emplace_back(self,actual);
        auto it=std::prev(domain.models.end());
        try {
            if (self.raw_storage) {
                it->raw_lifetime.emplace(NativeModelRawLifetimeBinding{
                    *self.raw_storage,it->terminal_frame,it->terminal,it->host});
                it->owner=std::make_unique<NativeModelOwner>(actual,NativeModelPool::slot_bytes,env,*it->raw_lifetime);
            } else it->owner=std::make_unique<NativeModelOwner>(actual,NativeModelPool::slot_bytes,env);
        } catch (...) { domain.models.erase(it); throw; }
        return *it->owner;
    }
    static void retire_failed_model(void* context, NativeModelOwner& owner) noexcept {
        auto& self=*static_cast<Impl*>(context);
        for (auto it=self.domain.models.begin();it!=self.domain.models.end();++it)
            if (it->client==&self && it->owner.get()==&owner && !it->reference &&
                (owner.phase==NativeModelOwner::Phase::prepared || owner.phase==NativeModelOwner::Phase::dead)) {
                self.domain.models.erase(it);return;
            }
        std::terminate();
    }
    static void retire_model(void* context, NativeModelReference& reference) noexcept {
        auto& record=*static_cast<GameNativeModelDomain::Impl::ModelRecord*>(context);
        auto& self=*record.client;
        if (self.observer.retire)
            self.observer.retire(self.observer.context,record.actual,reference,record.acquisition.drawable_index);
        if (record.registered) {
            self.domain.owners.unbind(record.actual,reference);
            record.registered=false;
        }
        // The raw scalar body consumes its CURRENT flags. A flags0 terminal
        // ended payload lifetime without returning this slot; keep its sole
        // receipt/companion metadata for the existing failure disposition.
        if (record.raw_lifetime && !record.terminal.pool_returned) return;
        for (auto it=self.domain.models.begin();it!=self.domain.models.end();++it)
            if (&*it==&record) { self.domain.models.erase(it);return; }
        std::terminate();
    }
    static NativeModelReference& bind_model(void* context, NativeModelOwner& owner) {
        auto& self=*static_cast<Impl*>(context);
        for (auto& record:self.domain.models) if (record.client==&self && record.owner.get()==&owner) {
            model_domain_detail::require(!record.reference,"Model client cannot bind a second companion");
            record.reference=std::make_unique<NativeModelReference>(owner,
                NativeModelCompanionDisposal{&record,&retire_model});
            self.domain.owners.bind(record.actual,*record.reference);
            record.registered=true;
            return *record.reference;
        }
        throw std::logic_error("Model companion requires this client's prepared actual slot");
    }
    static void retire_stream(void* context, NativeLogicalVertexReference& reference) noexcept {
        auto& record=*static_cast<GameNativeModelDomain::Impl::StreamRecord*>(context);
        auto& self=*record.client;
        if (record.registered) self.domain.owners.unbind(record.actual,reference);
        for (auto it=self.domain.streams.begin();it!=self.domain.streams.end();++it)
            if (&*it==&record) { self.domain.streams.erase(it);return; }
        std::terminate();
    }
    static NativeLogicalVertexReference& bind_stream(void* context, void* actual,
        NativeLogicalVertexOwnerContext& env) {
        auto& self=*static_cast<Impl*>(context);
        model_domain_detail::require(!self.domain.drained && &env==&self.domain.graphics.streams.vertices,
            "Model client stream requires the canonical vertex domain");
        for (const auto& record:self.domain.streams)
            model_domain_detail::require(record.actual!=actual,"stream already has common owner metadata");
        self.domain.streams.push_back({&self,actual,{},false});
        auto& record=self.domain.streams.back();
        record.reference=std::make_unique<NativeLogicalVertexReference>(actual,env,
            NativeLogicalVertexCompanionDisposal{&record,&retire_stream});
        self.domain.owners.bind(actual,*record.reference);
        record.registered=true;
        return *record.reference;
    }
};

GameNativeModelClient::GameNativeModelClient(std::unique_ptr<Impl> state) noexcept : impl_(std::move(state)) {}
GameNativeModelClient::~GameNativeModelClient() noexcept {
    if (impl_ && !empty()) impl_.release();
}
NativeInstanceGeometryAccess& GameNativeModelClient::raw_access() noexcept { return impl_->raw; }
NativeInstanceGeometryAccess& GameNativeModelClient::instance_access() {
    model_domain_detail::require(!impl_->raw_storage,"Page Model client has only its immutable raw access");
    return impl_->instance;
}
GameNativeModelAcquisition& GameNativeModelClient::model_acquisition(NativeModelOwner& owner) {
    for (auto& record:impl_->domain.models)
        if (record.client==impl_.get() && record.owner.get()==&owner) return record.acquisition;
    throw std::logic_error("Model acquisition requires this client's exact prepared owner");
}
NativeModelReference& GameNativeModelClient::resolve_actual_model(void* actual) {
    for (auto& record:impl_->domain.models)
        if (record.client==impl_.get() && record.actual==actual && record.registered) return *record.reference;
    throw std::logic_error("actual Model is outside this client's canonical companion domain");
}
bool GameNativeModelClient::requires_process_retention() const noexcept { return impl_->domain.retains_for(impl_.get()); }
bool GameNativeModelClient::empty() const noexcept { return impl_->domain.empty_for(impl_.get()); }
void GameNativeModelClient::after_native_renderer_drain() {
    model_domain_detail::require(empty(),"Model client retains native records after drain");
}

GameNativeModelDomain::GameNativeModelDomain(GameNativeRendererApplication& app,
    GameVfsHost& files, GameNativeReadOnlyData& data) : impl_(std::make_unique<Impl>(app,files,data)) {}
GameNativeModelDomain::~GameNativeModelDomain() noexcept {
    if (impl_ && !empty()) impl_.release();
}
std::unique_ptr<GameNativeModelClient> GameNativeModelDomain::make_mission_client(GameNativeModelRetirementObserver observer) {
    model_domain_detail::require(!impl_->drained && observer.retire,"Mission Model client requires its retained observer");
    return std::unique_ptr<GameNativeModelClient>(new GameNativeModelClient(std::make_unique<GameNativeModelClient::Impl>(*impl_,nullptr,observer)));
}
std::unique_ptr<GameNativeModelClient> GameNativeModelDomain::make_page_client(NativeModelStorageContext& storage) {
    impl_->require_page_storage(storage);
    return std::unique_ptr<GameNativeModelClient>(new GameNativeModelClient(std::make_unique<GameNativeModelClient::Impl>(*impl_,&storage,GameNativeModelRetirementObserver{nullptr,nullptr})));
}
bool GameNativeModelDomain::requires_process_retention() const noexcept { return impl_->retains_for(nullptr); }
bool GameNativeModelDomain::empty() const noexcept { return impl_->models.empty() && impl_->streams.empty(); }
void GameNativeModelDomain::after_native_renderer_drain() {
    model_domain_detail::require(empty(),"shared Model domain retains native records after drain");
    impl_->drained=true;
}
} // namespace bsp::game
