// ON-only lighting required by installed ship.shfx -> per_pixel_shadow.shfx.
// B46A70 reads these genuine native owners; no fallback register words are
// supplied. Include after the private renderer and mission-scene graph types.
struct MissionLightingGlobals {
 U half_guard{};
 SystemLightingWords4 half_color{};
 U diffuse_guard{};
 SystemLightingWords4 diffuse_color{};
 U specular_guard{};
 SystemLightingWords4 specular_color{};
 static MissionLightingGlobals& process() {
  // Installed PE loader-zero .data: E18AE0..E18B1B. These fallback guard/color
  // cells have no other application owner and are not read by our admitted
  // nonnull configuration branch. Settings and world-up have separate owners.
  static auto* const value=new MissionLightingGlobals;
  return *value;
 }
};

struct MissionShipLightingGraph {
 enum class Phase { prepared, creating, ready, roots_detached, retiring, retired, closed, failed };
 struct NoShadow final : SystemShadowOwnerResolver {
  SystemShadowMapOwner* resolve_shadow(void* actual) override {
   check(actual==nullptr,"mission directional light requires its native null shadow");
   return nullptr;
  }
 } shadows;
 GameHostLog& log;
 DestructionGraph& graph;
 CameraGraph& cameras;
 NativeRenderActualOwnerRegistry& owners;
 NativeStringRawPoolContext& strings;
 MissionLightingGlobals& globals;
 const std::uint8_t captured_scale_ambient;
 const SystemLightingWords3 fallback_direction;
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
 NativeSceneResourceConstructFrame resource_frame{};
 NativeSceneResourceAcquired resource_acquired;
 std::optional<NativeSceneResourceReference> resource_reference;
 NativeSceneRegistrationGateInsertFrame insert_frame{};
 NativeSceneRegistryRegistrationAcquired insertion;
 NativeSceneRegistrationGateEraseFrame erase_frame{};
 NativeRenderActualOwnerReleaseObservation resource_release;
 NativeString resource_name, sun_name;
 bool resource_name_live{},sun_name_live{},sun_scene_bound{},sun_retired{};
 void* outer{};
 void* resource{};
 void* sun{};
 DirectionalLightPool* sun_pool{};
 std::unique_ptr<DirectionalLightOwner> sun_owner;
 std::unique_ptr<DirectionalLightReference> sun_reference;
 volatile U published_resource{},null_resource{};
 Phase phase{Phase::prepared};

 MissionShipLightingGraph(GameHostLog& output,DestructionGraph& g,CameraGraph& c,
     NativeRenderActualOwnerRegistry& actual,NativeStringRawPoolContext& raw,
     std::uint8_t startup_scale_ambient)
  :log(output),graph(g),cameras(c),owners(actual),strings(raw),globals(MissionLightingGlobals::process()),
   // Source settings+8C aliases F88A0C and has already been loaded by startup.
   // Parent captures this exact byte in init arguments.word00; device/settings
   // reset is excluded while this graph is live. This is a stable read snapshot,
   // not a second writable global or a loader-zero default.
   captured_scale_ambient(startup_scale_ambient),
   // Native CD2280 initialized F8758C; preserve the existing source constant.
   // Nonnull 004BACD0 does not read this fallback direction.
   fallback_direction{float_bits(kWorldUp00f8758c[0]),float_bits(kWorldUp00f8758c[1]),
       float_bits(kWorldUp00f8758c[2])},
   registry_storage{allocation,deallocation,length_failure},mutation{registry_storage,invalid_parameter},
   registration{mutation,reinterpret_cast<const char*>(g.profiles(0xce38f8))},
   gate{registration,g.types,&c.types,g.profiles(0xd62c88),g.profiles(0xd62cf0),
       g.profiles(0xd62f58),g.profiles(0xd62fb0),nullptr},
   ambient{actual,game_native_renderer_scalar_process().increment_iat_00ce221c(),c.decrement},
   ambient_identity{actual,g.profiles(0xd62f3c)},
   resource_storage{raw,registry_storage,ambient,ambient_identity,g.profiles(0xd7a24c)},
   resource_identity{resource_storage,g.profiles(0xd63168)},
   scene_fields{actual,game_native_renderer_scalar_process().increment_iat_00ce221c(),c.decrement} {}
 ~MissionShipLightingGraph() {
  if(phase!=Phase::prepared && phase!=Phase::closed)std::terminate();
 }
 // Qualified stock substitution for the vendor STL throw site B82DD0.
 // Native message/length_error category are preserved; original FH3/RTTI and
 // private exception-object ABI are not claimed. This binding never returns.
 [[noreturn]] static void __cdecl qualified_length_failure() {
  throw std::length_error("vector<T> too long");
 }
 static bool directional_type(SceneAttachmentRuntime& runtime,SceneNodeAttachment&,U token) {
  return static_cast<ActualSceneRuntime&>(runtime).types.directional_is_type_00b7c6d0(token);
 }
 static bool light_type(SceneAttachmentRuntime& runtime,SceneNodeAttachment&,U token) {
  return static_cast<ActualSceneRuntime&>(runtime).types.light_is_type_00b7c580(token);
 }
 static void retired_sun(void* opaque,DirectionalLightReference& reference) noexcept {
  auto& self=*static_cast<MissionShipLightingGraph*>(opaque);
  self.owners.unbind(self.sun,reference);
  self.sun=nullptr;self.sun_retired=true;
 }
 static U float_bits(float value) noexcept {U result;std::memcpy(&result,&value,4);return result;}
 static SystemLightingWords4 color(float r,float g,float b) noexcept {
  return {float_bits(r),float_bits(g),float_bits(b),float_bits(1.0f)};
 }
 static LightingAmbientFields ambient_fields(void* actual) {
  auto* raw=static_cast<unsigned char*>(actual);
  // Start trivial typed field lifetimes AFTER B7C290, without initializing or
  // enlarging the 98h native allocation. Placement default initialization
  // preserves every native word, including the untouched mode3 preimage.
  return {*::new(raw+0x18) SystemLightingWords4,
      *::new(raw+0x28) SystemLightingWords4,
      *::new(raw+0x38) std::array<SystemLightingWords4,6>};
 }
 void configure_owners() {
  void* const actual_ambient=get<void*>(resource,0x10);
  check(actual_ambient && resource_acquired.ambient_reference && sun_owner,
      "mission lighting requires completed native ambient and sun owners");
  // Explicit source HDR illumination diagnostic for this linear backbuffer
  // view: ON11 clear-air authored diffuse/AO remains very dark, so these
  // genuine configuration inputs apply 8x the preceding ON10 illumination.
  // These values are not original mission/world lighting. The
  // installed ship shader decodes sRGB textures; the normal post20 exposure/
  // display conversion has not been composed in this bounded ordinary path.
  // These are real configuration inputs, not substitute constant-bank writes.
  const auto ambient_color=color(8.00f,8.96f,10.24f);
  std::array<SystemLightingWords4,6> cube;
  for(auto& face:cube)face=ambient_color;
  const auto diffuse=color(11.20f,10.88f,9.60f);
  const auto specular=color(1.92f,1.92f,1.92f);
  const U one=*graph.profiles(0xd7a24c);
  const U yaw=float_bits(0.7853981634f),elevation=float_bits(0.6154797087f);
  const LightingConfigurationFields configuration{ambient_color,ambient_color,cube,
      diffuse,diffuse,specular,one,one,yaw,elevation};
  LightingConfigurationGlobals global_fields{globals.half_guard,globals.half_color,
      globals.diffuse_guard,globals.diffuse_color,globals.specular_guard,globals.specular_color,
      fallback_direction,captured_scale_ambient,*graph.profiles(0xce3800),*graph.profiles(0xd7a24c)};
  auto& light=sun_owner->light;
  LightingDirectionalFields directional{light.diffuse_184,light.specular_194,
      light.base_diffuse_1a4,light.diffuse_mode3_1b4,light.base_specular_1c4,
      light.diffuse_scale_1d8,light.specular_scale_1dc,light.direction_1e0};
  apply_lighting_configuration_004bacd0(ambient_fields(actual_ambient),directional,
      &configuration,global_fields);
  check(light.shadow_174==nullptr,"mission sun unexpectedly acquired a shadow owner");
 }
 void create_and_bind(void* actual_outer_scene) {
  check(phase==Phase::prepared && actual_outer_scene &&
      get<U>(actual_outer_scene)==0xd62d48 && get<void*>(actual_outer_scene,0x1c)==nullptr,
      "mission lighting must bind after model roots on the native null-lighting scene");
  phase=Phase::creating;outer=actual_outer_scene;
  try {
   graph.types.initialize_light_00cd80a0();
   graph.types.initialize_directional_00cd80f0();
   auto& pools=game_native_resource_pool_process();
   pools.initialize_directional_light_once_00cd8080();
   sun_pool=&pools.directional_light_pool_01090154();
   construct_native_string_header_0041e870(&resource_name,strings,"Mission ship lighting");
   resource_name_live=true;
   resource=raw_allocate(0x3c);
   check(resource!=nullptr,"mission lighting resource allocation returned null");
   resource_frame.name_or_ambient_argument=bits(&resource_name);
   construct_native_scene_resource_00b83c50(resource,resource_frame,resource_storage,resource_acquired);
   resource_reference.emplace(resource,resource_identity);

   construct_native_string_header_0041e870(&sun_name,strings,"Mission sun");
   sun_name_live=true;
   sun=sun_pool->allocate_raw_slot_00b7bac0();
   check(sun!=nullptr,"mission sun pool returned null");
   auto admission=owners.prepare_binding(sun);
   const auto actual=construct_native_directional_light_00b7c6b0(sun,DirectionalLightPool::slot_bytes,
       &sun_name,strings,cameras.node_constants,*graph.profiles(0xce7820));
   sun_owner=std::make_unique<DirectionalLightOwner>(actual,*sun_pool,graph.nodes,
       &directional_type,&light_type,shadows);
   graph.nodes.scenes.bind(sun_owner->node.scene_attachment);sun_scene_bound=true;
   sun_reference=std::make_unique<DirectionalLightReference>(*sun_owner,
       DirectionalLightReferenceProfiles{graph.profiles(0xd62fb0),graph.profiles(0xd62f58),graph.profiles(0xd62c88)},
       DirectionalLightCompanionDisposal{this,&retired_sun});
   owners.publish(std::move(admission),*sun_reference);
   configure_owners();
   insert_frame.node_argument=bits(sun);
   register_native_scene_node_if_light_00b83d50(resource,insert_frame,gate,insertion);
   check(insertion.complete && get<U>(resource,0x20)==1,
       "mission lighting did not register its one native directional light");
   const void* const head=get<void*>(resource,0x1c);
   check(head && get<void*>(head)!=head && get<void*>(get<void*>(head),8)==sun,
       "mission lighting first native registry entry is not its sun");
   published_resource=bits(resource);
   set_native_gui_scene_resource_00b723f0(outer,published_resource,scene_fields);
   check(get<void*>(outer,0x1c)==resource,"mission outer scene did not retain native lighting resource");
   sun_name_live=false;destroy_native_string_header_0041dd20(&sun_name,strings);
   resource_name_live=false;destroy_native_string_header_0041dd20(&resource_name,strings);
   phase=Phase::ready;
   log.notef("native mission lighting ready: outer=%p resource=%p ambient=%p sun=%p first_light=1 "
       "configuration=004BACD0 illumination=source_HDR_diagnostic captured_scale_ambient=%u ambient_cube=8.00,8.96,10.24 diffuse=11.20,10.88,9.60 specular=1.92,1.92,1.92 shadow=null",
       outer,resource,get<void*>(resource,0x10),sun,unsigned(captured_scale_ambient));
  } catch(...) {phase=Phase::failed;throw;}
 }
 void begin_root_attachment() {
  check(phase==Phase::ready && get<void*>(outer,0x1c)==resource &&
      resource_reference->reference_count.load()==2,
      "mission root insertion requires its genuine live lighting credits");
  phase=Phase::roots_detached;
  set_native_gui_scene_resource_00b723f0(outer,null_resource,scene_fields);
  check(get<void*>(outer,0x1c)==nullptr && resource_reference->reference_count.load()==1,
      "mission root insertion did not preserve the lighting creator");
 }
 void end_root_attachment() {
  check(phase==Phase::roots_detached && get<void*>(outer,0x1c)==nullptr &&
      resource_reference->reference_count.load()==1,
      "mission root insertion lost its lighting creator");
  set_native_gui_scene_resource_00b723f0(outer,published_resource,scene_fields);
  check(get<void*>(outer,0x1c)==resource && resource_reference->reference_count.load()==2,
      "mission root insertion failed to restore native lighting");
  phase=Phase::ready;
 }
 bool requires_process_retention() const noexcept {
  return phase==Phase::creating || phase==Phase::roots_detached ||
      phase==Phase::retiring || phase==Phase::failed;
 }
 void retire_before_native_drain() {
  if(phase==Phase::prepared){phase=Phase::closed;return;}
  check(phase==Phase::ready && resource_reference && sun_reference,
      "mission lighting retirement requires its complete native graph");
  phase=Phase::retiring;
  try {
   check(get<void*>(outer,0x1c)==resource && resource_reference->reference_count.load()==2,
       "mission lighting has unexpected outer/resource credits at shutdown");
   // Restore native-null before A1 detaches its static roots. B723F0 has no
   // root propagation; model+170 remained null throughout this diagnostic.
   set_native_gui_scene_resource_00b723f0(outer,null_resource,scene_fields);
   check(get<void*>(outer,0x1c)==nullptr && resource_reference->reference_count.load()==1,
       "mission lighting failed to release the outer scene resource credit");
   erase_frame.node_argument=bits(sun);
   unregister_native_scene_node_if_light_00b83ec0(resource,erase_frame,gate);
   check(get<U>(resource,0x20)==0,"mission lighting registry retained its sun key");
   release_native_render_actual_owner_observed(owners,resource,resource_release);
   check(resource_reference->retired() && resource_acquired.ambient_reference->retired(),
       "mission lighting resource/ambient did not reach native terminals");
   resource=nullptr;
   check(sun_reference->reference_count.load()==1 && sun_owner->light.scenes_178.count_04==0 &&
       sun_owner->light.shadow_174==nullptr,"mission sun has unexpected native retained owners");
   unlink_and_release_render_model_00b6dfa0(*sun_reference);
   check(sun_retired,"mission sun did not reach its genuine deleting destructor");
   phase=Phase::retired;
   log.note("native mission lighting retired: outer_lighting=null resource=retired ambient=retired sun=retired registry=empty");
  } catch(...) {phase=Phase::failed;throw;}
 }
 void after_native_drain() {
  if(phase==Phase::closed)return;
  check(phase==Phase::retired && sun_retired && !resource_name_live && !sun_name_live,
      "mission lighting survived native drain");
  sun_reference.reset();sun_owner.reset();resource_reference.reset();
  resource_acquired.ambient_reference.reset();phase=Phase::closed;
 }
};
