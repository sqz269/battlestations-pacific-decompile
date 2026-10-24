// One diagnostic outer24h scene over the application's genuine weak, raw-name
// and actual-owner domains. Include after the resource/camera/model types.
// Construct only in the BSP_NATIVE_MISSION_SHIP=1 branch. Metadata construction
// does not allocate a native scene, change a model, or create another camera.
struct MissionShipSceneGraph {
 enum class Phase { prepared, creating, ready, attaching, retiring, released, failed, closed };
 struct Association {
  enum class Phase { prepared, retained, linked, detaching, released };
  void* raw{};
  RenderCommandReference* reference{};
  GeneratedModelNodeLifetime* node{};
  Phase phase{Phase::prepared};
  NativeRenderActualOwnerReleaseObservation release;
 };
 struct RootDispatch final : NativeNodeSceneChildDispatch {
  MissionShipSceneGraph& graph;
  explicit RootDispatch(MissionShipSceneGraph& value):graph(value) {}
  const volatile U* resolve_profile(U captured) const override {
   return graph.root_profile(captured);
  }
  void invoke_child(U target,void* raw,U requested,U recursion) override {
   // The actual outer scene's constructor supplies null+1C lighting. B6D890
   // therefore reaches the existing B6ED80 (null scene, recurse0) branch.
   // Execute that genuine model-bound Source method; never substitute a
   // successful no-op or interpret a raw3Ch resource as a logical SceneResource.
   check(target==0xb6ed80 && requested==0 && recursion==0,
       "mission scene admits only the native null-lighting model attachment");
   auto* model_reference=dynamic_cast<GeneratedModelNodeLifetime*>(&graph.owners.resolve_actual(raw));
   check(model_reference && model_reference->transform().raw_node_key()==bits(raw) &&
       graph.environment.nodes.attachments.find_actual_node(bits(raw))==model_reference &&
       &graph.environment.nodes.scenes.resolve_key(bits(raw))==&model_reference->scene_attachment() &&
       graph.root_profile(get<U>(raw))[0x50/4]==target &&
       get<void*>(raw,0x170)==nullptr,
       "mission scene attachment requires its live same-domain model companion");
   auto& binding=model_reference->scene_attachment();
   check(binding.attach_scene!=nullptr && binding.scene==nullptr,
       "mission scene model attachment binding is not the native null-scene branch");
   binding.attach_scene(graph.environment.nodes.scenes,binding,nullptr,false);
  }
 } dispatch;
 GameHostLog& log;
 NativeGuiSceneEnvironment& environment;
 NativeRenderActualOwnerRegistry& owners;
 NativeTextureSurfaceReferenceIncrement const volatile& increment;
 NativeTextureSurfaceReferenceIncrement const volatile& decrement;
 const char* const world_name;
 NativeGuiSceneStorageContext storage;
 NativeGuiSceneIdentityContext identity;
 NativeGuiSceneConstructFrame construction_frame{};
 NativeGuiSceneConstructAcquired construction;
 NativeGuiSceneDestroyFrame destruction_frame{};
 NativeGuiSceneLifetimeAcquired destruction;
 std::optional<NativeGuiSceneReference> reference;
 NativeRenderActualOwnerReleaseObservation creator_release;
 NativeString name;
 bool name_live{},name_return_started{};
 void* allocation{};
  std::vector<std::pair<U,const volatile U*>> root_profiles;
 std::vector<std::unique_ptr<Association>> associations;
 Phase phase{Phase::prepared};

 MissionShipSceneGraph(GameHostLog& host_log,NativeGuiSceneEnvironment& scenes,
     NativeRenderActualOwnerRegistry& actual_owners,
     NativeTextureSurfaceReferenceIncrement const volatile& actual_increment,
     NativeTextureSurfaceReferenceIncrement const volatile& actual_decrement,
     const char* actual_world_name_00ce7e10)
  :dispatch(*this),log(host_log),environment(scenes),owners(actual_owners),
   increment(actual_increment),decrement(actual_decrement),world_name(actual_world_name_00ce7e10),
   storage{dynamic_cast<NativeWeakOwnerDomain&>(scenes.weak_base),
       scenes.nodes.require_raw_name_pool(),actual_owners,actual_decrement,scenes.one_00d7a24c,nullptr},
   identity{storage,scenes.vtable_00d62d48} {
  check(world_name && increment && decrement,
      "mission scene requires genuine name and current native import cells");
 }
 void* create() {
  check(phase==Phase::prepared,"mission scene construction cannot replay");
  phase=Phase::creating;
  try {
   allocation=singleton_lifetime_allocate({SingletonAllocationKind::object,0x24,0x24});
   check(allocation!=nullptr,"mission scene native allocation returned null");
   construct_native_string_header_0041e870(&name,storage.strings,world_name);
   name_live=true;
   construction_frame.name_argument=bits(&name);
   construct_native_gui_scene_storage_00b724e0(allocation,construction_frame,storage,construction);
   reference.emplace(allocation,identity,destruction_frame,destruction);
   // Caller-header cleanup is consumed once. Failure retains this graph,
   // actual scene, acquisition diagnostics and canonical companion.
   name_return_started=true;name_live=false;
   destroy_native_string_header_0041dd20(&name,storage.strings);
   phase=Phase::ready;
   log.notef("native mission scene B724E0: owner=%p roots=%p lighting=%p",allocation,
       get<void*>(allocation,0x0c),get<void*>(allocation,0x1c));
   return allocation;
  } catch(...) {phase=Phase::failed;throw;}
 }
 void* raw() const {
  check(phase==Phase::ready && reference && !reference->retired(),
      "mission scene borrow requires its completed live owner");
  return allocation;
 }
 const volatile U* root_profile(U captured) const {
  check(captured==0xd62de8 || captured==0xd63590,
      "mission scene root requires a genuine ordinary or matrix-indexed model profile");
  for(const auto& item:root_profiles)if(item.first==captured)return item.second;
  throw std::logic_error("mission scene root has no stable captured profile binding");
 }
 void attach(void* model,RenderCommandReference& actual_reference,const volatile U* profile) {
  check(phase==Phase::ready && construction.complete && reference && !reference->retired(),
      "mission scene attachment requires a live completed scene");
  auto* node=dynamic_cast<GeneratedModelNodeLifetime*>(&actual_reference);
  check(model && node && node->transform().raw_node_key()==bits(model) &&
      environment.nodes.attachments.find_actual_node(bits(model))==node &&
      &environment.nodes.scenes.resolve_key(bits(model))==&node->scene_attachment() &&
      &node->scene_attachment().scene==reinterpret_cast<SceneResource**>(static_cast<unsigned char*>(model)+0x170) &&
      &owners.resolve_actual(model)==static_cast<RenderCommandReference*>(&actual_reference) &&
      reinterpret_cast<const void*>(&actual_reference.reference_count)==static_cast<unsigned char*>(model)+4 &&
      (get<U>(model)==0xd62de8 || get<U>(model)==0xd63590) && profile && profile[0x50/4]==0xb6ed80 &&
      get<void*>(model,0x30)==nullptr &&
      get<void*>(model,0x34)==nullptr && get<void*>(model,0xa4)==nullptr &&
      get<void*>(model,0x170)==nullptr && get<std::uint8_t>(model,0x44)==0 &&
      get<void*>(allocation,0x1c)==nullptr && actual_reference.reference_count.load()>0,
      "mission scene requires an unattached same-domain model with its original creator");
  bool found_profile=false;
  for(const auto& item:root_profiles)if(item.first==get<U>(model)) {
   check(item.second==profile,"mission scene cannot mix actual model profile domains");
   found_profile=true;
  }
  if(!found_profile)root_profiles.emplace_back(get<U>(model),profile);
  for(const auto& item:associations)check(item->raw!=model,"mission scene model cannot attach twice");
  auto item=std::make_unique<Association>();
  item->raw=model;item->reference=&actual_reference;item->node=node;
  associations.push_back(std::move(item)); // metadata allocation BEFORE native work
  auto& association=*associations.back();
  phase=Phase::attaching;
  try {
   // B6D890 itself does not retain. This diagnostic association holds one
   // explicit real native credit while the model application keeps its creator.
   association.phase=Association::Phase::retained;
   increment(reinterpret_cast<volatile long*>(static_cast<unsigned char*>(model)+4));
   propagate_native_node_root_storage_00b6d890(model,allocation,dispatch);
   association.phase=Association::Phase::linked;
   check(get<void*>(model,0xa4)==allocation && get<void*>(model,0x170)==nullptr,
       "mission scene root did not preserve its actual outer/root identities");
   phase=Phase::ready;
  } catch(...) {phase=Phase::failed;throw;}
 }
 void retire_before_native_drain() {
  if(phase==Phase::released || phase==Phase::closed)return;
  check(phase==Phase::ready,"mission scene retirement requires completed associations");
  phase=Phase::retiring;
  try {
   // Detach every actual root before legacy model terminals can inspect A4.
   // Current root links/backlinks are edited by the complete genuine B6D890.
   for(auto it=associations.rbegin();it!=associations.rend();++it) {
    auto& association=**it;
    check(association.phase==Association::Phase::linked &&
        get<void*>(association.raw,0xa4)==allocation &&
        association.node && environment.nodes.attachments.find_actual_node(bits(association.raw))==association.node &&
        association.reference->reference_count.load()>1 &&
        get<std::uint8_t>(association.raw,0x44)==0,
        "mission scene creator must survive its association retirement");
    association.phase=Association::Phase::detaching;
    propagate_native_node_root_storage_00b6d890(association.raw,nullptr,dispatch);
    check(get<void*>(association.raw,0xa4)==nullptr && get<void*>(association.raw,0x170)==nullptr,
        "mission scene raw root must be detached before model retirement");
    // This drops ONLY our additional credit. The model's B6F310 logical
    // creator release remains the model application's one-shot obligation.
    release_native_render_actual_owner_observed(owners,association.raw,association.release);
    check(association.release.previous_count>1,"mission scene association consumed a model creator");
    association.phase=Association::Phase::released;
   }
   check(get<void*>(allocation,0x0c)==nullptr,"mission scene roots survived explicit detachment");
   release_native_render_actual_owner_observed(owners,allocation,creator_release);
   // A queued command may still hold the actual scene. Its canonical terminal
   // later sees an empty root list, so no unbound raw node-tree deleter is used.
   phase=Phase::released;
  } catch(...) {phase=Phase::failed;throw;}
 }
 bool requires_process_retention() const noexcept {
  return phase==Phase::creating || phase==Phase::attaching || phase==Phase::retiring || phase==Phase::failed;
 }
 void after_native_drain() {
  if(phase==Phase::closed)return;
  if(phase==Phase::prepared){phase=Phase::closed;return;}
  check(phase==Phase::released && !name_live && reference && reference->retired() &&
      destruction.complete && destruction.freed,
      "mission scene native retirement did not complete");
  for(const auto& item:associations)check(item->phase==Association::Phase::released,
      "mission scene model association survived native drain");
  reference.reset();associations.clear();
  phase=Phase::closed;
 }
};
