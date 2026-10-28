// cc15-gui4 A1 (kNativeRendererStartupCc15Bound): the application's ONE raw
// scene domain. The image keeps a single registry/gate, node, camera,
// viewport, light and fog domain for the process. Startup builds it once at
// the 0073DD12 position; the opt-in mission graph and the GUI page chain
// borrow it. This is a host composition of existing native bodies, not a
// recovered type; field order is not an original ABI.
//
// Include after the camera graph and before every mission graph.

// The registry/gate recipe previously built inside MissionShipLightingGraph.
// Pure bindings: no native state, no allocation at construction.
struct SceneRegistryContexts {
 [[noreturn]] static void __cdecl qualified_length_failure() {
  // Qualified stock substitution for the vendor STL throw site B82DD0, as in
  // the mission lighting graph; FH3/RTTI and exception-object ABI not claimed.
  throw std::length_error("vector<T> too long");
 }
 NativeSceneRegistryAllocator const allocation{&raw_allocate};
 NativeSceneRegistryFree const deallocation{&raw_free};
 NativeSceneRegistryLengthFailure const length_failure{&qualified_length_failure};
 NativeSceneRegistryInvalidParameter const invalid_parameter{&invoke_native_invalid_parameter_00bf6713};
 NativeSceneRegistryStorageBindings registry_storage;
 NativeSceneRegistryVectorMutationBindings mutation;
 NativeSceneRegistryRegistrationContext registration;
 NativeSceneRegistrationGateContext gate;
 NativeSceneResourceAmbientContext ambient;
 NativeAmbientIdentityContext ambient_identity;
 NativeSceneResourceStorageContext resource_storage;
 NativeSceneResourceIdentityContext resource_identity;
 NativeGuiSceneFieldsContext scene_fields;
 SceneRegistryContexts(DestructionGraph& g,CameraGraph& c,NativeRenderActualOwnerRegistry& actual,
     NativeStringRawPoolContext& raw)
  :registry_storage{allocation,deallocation,length_failure},mutation{registry_storage,invalid_parameter},
   registration{mutation,reinterpret_cast<const char*>(g.profiles(0xce38f8))},
   gate{registration,g.types,&c.types,g.profiles(0xd62c88),g.profiles(0xd62cf0),
       g.profiles(0xd62f58),g.profiles(0xd62fb0),nullptr},
   ambient{actual,game_native_renderer_scalar_process().increment_iat_00ce221c(),c.decrement},
   ambient_identity{actual,g.profiles(0xd62f3c)},
   resource_storage{raw,registry_storage,ambient,ambient_identity,g.profiles(0xd7a24c)},
   resource_identity{resource_storage,g.profiles(0xd63168)},
   scene_fields{actual,game_native_renderer_scalar_process().increment_iat_00ce221c(),c.decrement} {}
};

struct StartupSceneDomain {
 // Profiles the raw dispatchers admit: node, camera, light, directional,
 // model and group. Anything else is outside this domain and rejects.
 static bool admitted_profile(U profile) noexcept {
  return profile==0xd62c88 || profile==0xd62cf0 || profile==0xd62f58 ||
      profile==0xd62fb0 || profile==0xd62de8 || profile==0xd634f8;
 }
 [[noreturn]] static void reject(const char* slot,U target) {
  char message[160];
  std::snprintf(message,sizeof message,
      "startup scene domain has no admitted native body for %s target %08x",slot,target);
  throw std::runtime_error(message);
 }
 // Pose (30/34) and world (3C/40) slots of the raw node families. Each target
 // is the exact current table entry the caller loaded; the bodies are the
 // existing raw reconstructions. Scratch backing is persistent and disjoint
 // per body; nested calls never share one body's live frame.
 struct NodeDispatch final : NativeCameraPoseDispatch {
  CanonicalProfiles& profiles;
  const NativeNodeMappedRawTransformProfiles bounds;
  std::array<volatile U,6> position_direction{};
  std::array<volatile U,32> camera_derive{};
  std::array<volatile U,6> camera_direction{};
  std::array<volatile U,32> node_derive{};
  volatile U position_base{};
  const NativeCameraPositionFrame position{&position_base,{position_direction.data()}};
  const NativeCameraRawWorldEditFrame camera_world{{camera_derive.data()},{camera_direction.data()}};
  const NativeCameraDeriveLocalFrame node_world{node_derive.data()};
  NodeDispatch(CanonicalProfiles& p,GameNativeReadOnlyData& data):profiles(p),bounds(data) {}
  const volatile U* resolve_profile(U profile) const override {
   if(!admitted_profile(profile))reject("profile",profile);
   return profiles(profile);
  }
  void invoke_virtual30(U target,void* actual,volatile U& argument) override {
   if(target==0xb71400){set_native_raw_camera_position_00b71400(actual,argument,position,*this);return;}
   if(target==0xb6dae0){set_native_raw_world_position_00b6dae0(actual,argument,*this);return;}
   reject("pose+30",target);
  }
  void invoke_virtual34(U target,void* actual,volatile U& argument) override {
   if(target==0xb71460){set_native_raw_camera_world_00b71460(actual,argument,camera_world,*this);return;}
   if(target==0xb6e870){set_native_raw_world_matrix_00b6e870(actual,argument,node_world,*this);return;}
   reject("pose+34",target);
  }
  void invoke_virtual40(U target,void* actual) override {
   if(target==0xb6dbe0){notify_native_raw_world_00b6dbe0(actual,*this);return;}
   if(target==0xb8e6b0){invalidate_native_group_world_00b8e6b0(actual);return;}
   reject("world+40",target);
  }
  void invoke_virtual3c(U target,void* actual) override {
   if(target==0xb6dbc0){notify_raw_bounds_00b6dbc0(actual,bounds);return;}
   reject("world+3C",target);
  }
 };
 // Prepared persistent frames for recursive tree retirement and scene
 // attachment. Acquisition is pure metadata over a fixed reservation; an
 // exhausted pool rejects rather than allocating inside a native walk.
 static constexpr std::size_t frame_slots=256;
 struct TreeSlot { NativeNodeTreeRetirementFrame frame{}; NativeNodeTreeRetirementAcquired acquired{}; };
 struct SceneSlot { NativeNodeSceneAttachmentFrame frame{}; NativeNodeSceneAttachmentAcquired acquired{}; };
 struct Trees final : NativeNodeTreeRetirementDispatch, NativeNodeTreeRetirementFrames {
  NodeDispatch& nodes;
  NativeNodeTreeRetirementContext* context{};
  std::unique_ptr<std::array<TreeSlot,frame_slots>> slots{std::make_unique<std::array<TreeSlot,frame_slots>>()};
  std::size_t used{};
  explicit Trees(NodeDispatch& n):nodes(n) {}
  TreeSlot& next() {
   check(used<frame_slots,"startup scene domain exhausted its prepared tree frames");
   return (*slots)[used++];
  }
  NativeNodeTreeRetirementInvocation recursive_unregister(void*) override {
   auto& slot=next();return {slot.frame,slot.acquired};
  }
  const volatile U* resolve_profile(U profile) const override {return nodes.resolve_profile(profile);}
  void invoke_virtual18(U target,void* receiver) override {
   check(context!=nullptr,"startup tree dispatch is unbound");
   if(target==0xb6f310){
    auto& slot=next();
    release_native_node_tree_00b6f310(receiver,slot.frame,*context,slot.acquired);
    return;
   }
   reject("tree+18",target);
  }
  void invoke_virtual0(U target,void* receiver,RenderCommandReference& owner) override {
   // Generic BD30E0 release on an observed zero count. The canonical owner's
   // family terminal (camera B71FE0, directional B7C820, ...) runs from its
   // own companion; only its scene-null domain is admitted here.
   if(target!=0xbd30e0)reject("tree+0",target);
   check(static_cast<void*>(&owner.reference_count)==static_cast<unsigned char*>(receiver)+4 &&
       get<U>(receiver,4)==0 && get<U>(receiver,0x170)==0,
       "startup tree terminal requires the same zero-count scene-null actual owner");
   owner.release_zero_references();
  }
 };
 // Scene-child slot +50 for the raw camera/directional families. Families
 // owned by another graph (GUI Group, Model) go to the bound delegate.
 struct Children final : NativeNodeSceneChildDispatch {
  NodeDispatch& nodes;
  NativeRenderActualOwnerRegistry& owners;
  NativeNodeSceneAttachmentContext* context{};
  NativeNodeSceneChildDispatch* delegate{};
  std::unique_ptr<std::array<SceneSlot,frame_slots>> slots{std::make_unique<std::array<SceneSlot,frame_slots>>()};
  std::size_t used{};
  Children(NodeDispatch& n,NativeRenderActualOwnerRegistry& o):nodes(n),owners(o) {}
  const volatile U* resolve_profile(U profile) const override {
   if(delegate && !admitted_profile(profile))return delegate->resolve_profile(profile);
   return nodes.resolve_profile(profile);
  }
  void invoke_child(U target,void* child,U scene,U recursion) override {
   check(context!=nullptr,"startup scene-child dispatch is unbound");
   auto* reference=owners.find(child);
   const bool is_camera=reference && dynamic_cast<NativeCameraStorageReference*>(reference);
   const bool is_directional=reference && dynamic_cast<NativeDirectionalLightStorageReference*>(reference);
   if((target==0xb6ed80 && is_camera) || (target==0xb7c020 && is_directional)){
    check(used<frame_slots,"startup scene domain exhausted its prepared attachment frames");
    auto& slot=(*slots)[used++];
    slot.frame.scene_argument=scene;slot.frame.recursion_argument=recursion;
    if(is_camera)set_native_node_scene_storage_00b6ed80(child,slot.frame,*context,slot.acquired);
    else attach_native_light_scene_storage_00b7c020(child,slot.frame,*context,slot.acquired);
    return;
   }
   if(delegate){delegate->invoke_child(target,child,scene,recursion);return;}
   reject("scene+50",target);
  }
 };
 // Camera+438 retained owner: no family is admitted yet. A reached nonnull
 // owner rejects instead of guessing a terminal.
 struct Retained final : NativeCameraRetainedLifetime {
  NodeDispatch& nodes;
  explicit Retained(NodeDispatch& n):nodes(n) {}
  const volatile U* resolve_profile(U profile) const override {return nodes.resolve_profile(profile);}
  void invoke_virtual0(U target,void*) override {reject("camera+438",target);}
 };
 // D5F0A8+30 on the actual current renderer: B1FF60 returns actual+1A14.
 struct Viewports final : NativeViewportRawRendererAccess {
  CanonicalProfiles& profiles;
  explicit Viewports(CanonicalProfiles& p):profiles(p) {}
  const volatile U* resolve_profile(U profile) const override {
   if(profile!=0xd5f0a8)reject("renderer profile",profile);
   return profiles(profile);
  }
  void* invoke_virtual30(U target,void* renderer) override {
   if(target!=0xb1ff60)reject("renderer+30",target);
   return native_renderer_parameters_00b1ff60(renderer);
  }
 };

 SceneRegistryContexts registry;
 NativeGuiSceneEnvironment scenes;
 NativePointLightLinksRuntime point_lights;
 NodeDispatch nodes;
 Trees trees_dispatch;
 Children children;
 Retained retained;
 Viewports viewport_access;
 NativeNodeSceneAttachmentContext node_scenes;
 NativeNodeTreeRetirementContext trees;
 NativeNodeBaseDestructionContext node;
 NativeViewportRawEnvironment viewport;
 NativeCameraLookAtBindings look_at;
 NativeCameraStorageLifetimeContext camera_lifetime;
 NativeCameraStorageConstructContext camera;
 NativeLightLifetimeContext light;
 NativeDirectionalLightIdentityContext directional;
 NativeSystemFogConstants fog_constants;
 NativeSystemFogStorageContext fog;
 NativeGuiSceneStorageContext scene_storage;
 NativeGuiSceneIdentityContext scene_identity;

 static DirectionalLightPool& open_directional_pool() {
  // CRT initializers CD80A0/CD80F0/CD8080 in the image; startup runs them
  // once here so no later graph constructs them. CD8080 registers its atexit.
  auto& pools=game_native_resource_pool_process();
  pools.initialize_directional_light_once_00cd8080();
  return pools.directional_light_pool_01090154();
 }
 static DestructionGraph& initialize_types(DestructionGraph& g) {
  g.types.initialize_light_00cd80a0();
  g.types.initialize_directional_00cd80f0();
  return g;
 }
 StartupSceneDomain(DestructionGraph& g,CameraGraph& c,GameSingletonHost& host,
     NativeRenderActualOwnerRegistry& actual,NativeStringRawPoolContext& raw,
     void* const volatile& renderer)
  :registry(initialize_types(g),c,actual,raw),
   scenes{g.nodes,host.weak_owners(),*g.profiles(0xd7a24c),g.profiles(0xd62d48)},
   nodes(g.profiles,g.profiles.data),trees_dispatch(nodes),children(nodes,actual),
   retained(nodes),viewport_access(g.profiles),
   node_scenes{registry.gate,actual,game_native_renderer_scalar_process().increment_iat_00ce221c(),
       c.decrement,children},
   trees{point_lights,actual,c.decrement,trees_dispatch,trees_dispatch},
   node{node_scenes,trees,raw,nodes},
   viewport{renderer,viewport_access,*g.profiles(0xd7a24c)},
   look_at{application_camera_axes_crt(),*g.profiles(0xd7a208),
       *reinterpret_cast<const volatile std::uint64_t*>(g.profiles(0xd62ba0)),
       *reinterpret_cast<const volatile std::uint64_t*>(g.profiles(0xd7a3a0)),*g.profiles(0xd7a24c)},
   camera_lifetime{node,game_native_resource_pool_process().camera_pool_0108ffb0(),
       game_native_renderer_scalar_process().increment_iat_00ce221c(),c.decrement,
       g.profiles(0xd62cf0),g.profiles(0xd5e5f8),g.profiles(0xd63180),retained},
   camera{c.node_constants,viewport,nodes,look_at,camera_lifetime,
       *g.profiles(0xce3d08),*g.profiles(0xce7d20),*g.profiles(0xd0c5f8),
       *g.profiles(0xd5bd98),*g.profiles(0xce77fc),*g.profiles(0xce3c88)},
   light{node,nullptr},
   directional{light,actual,open_directional_pool(),g.profiles(0xd62fb0)},
   fog_constants{*g.profiles(0xce386c),*g.profiles(0xd63188),*g.profiles(0xce7804),
       *g.profiles(0xce77f8),*g.profiles(0xce3950),*g.profiles(0xce77e8),
       *g.profiles(0xce77e4),*g.profiles(0xce77e0),*g.profiles(0xce77dc)},
   fog{game_native_renderer_scalar_process().increment_iat_00ce221c(),c.decrement,g.profiles(0xd63180)},
   scene_storage{dynamic_cast<NativeWeakOwnerDomain&>(scenes.weak_base),raw,actual,c.decrement,
       *g.profiles(0xd7a24c),&trees},
   scene_identity{scene_storage,g.profiles(0xd62d48)} {
  trees_dispatch.context=&trees;
  children.context=&node_scenes;
 }
 StartupSceneDomain(const StartupSceneDomain&)=delete;
 StartupSceneDomain& operator=(const StartupSceneDomain&)=delete;
 // The fixed reservation this domain holds for its whole process lifetime.
 static constexpr std::size_t reservation_bytes() noexcept {
  return sizeof(StartupSceneDomain)+frame_slots*(sizeof(TreeSlot)+sizeof(SceneSlot));
 }
};
