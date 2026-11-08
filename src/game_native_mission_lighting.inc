// ON-only lighting required by installed ship.shfx -> per_pixel_shadow.shfx.
// B46A70 reads these genuine native owners; no fallback register words are
// supplied. Include after the private renderer and mission-scene graph types.

// The selected mission's g_Environment header, read once by the whole
// 004EE450 body (BSP_SceneRecord_ReadSkyAndLights), the call 004F1D70 makes at
// 004F230B when the header has a g_Environment group (004F22F1, type 6). Its
// outputs are the scene record's sky strings (+B98h ReflCubemap, +BA0h ocean
// cubemap, +C54h Skybox, +C5Ch/+C60h, +1044h Sun, +104Ch ThunderStorm) and the
// lighting block +F48h..+1010h its Lights arm fills through 004EA870 (004EE4A3,
// ECX = Lights bag, EDX = record), plus the foliage settings 004EA870 writes.
// The lighting graph takes the Lights block (004BACD0 below) and the material
// refresh publishes +B98h as ReflectionCubeMap; both used to read the header
// by hand. The linearize flag is the byte [00F88A0C] 004EE450 tests at
// 004EE4C4, which the init arguments' word00 captures.
// Evidence: bsp.py show 004EE450 --asm 004EE475..004EE4C2 (ECX = bag in EBX,
// EDX = record in EBP; 004E7220 first, then 008F2260 "Lights"). Original ABI
// __fastcall(ECX = g_Environment bag, EDX = scene record), RET; this adapter is
// a semantic C++ interface, not an Original ABI replacement. The semantic
// fields do not advertise a native D5D270 derived foliage owner. The mission
// graph admits no landscape/bush groups; its genuine D5D220 base does own the
// normal-allowed byte read by B46A70.
struct MissionSceneEnvironment final : scene_weather_cc13::SceneSkyHost {
 GameHostLog& log;
 MissionShipFoliageGraph& foliage;
 scene_weather_cc13::SceneSkyFields sky;
 scene_weather_cc13::SceneLightsFields lights;
 bool lights_authored{};
 bool water_tracer_color_authored{};

 MissionSceneEnvironment(GameHostLog& output,MissionShipFoliageGraph& actual)
  : log(output),foliage(actual) {}
 void set_bush_diffuse_factor_00ad7140(float factor) override {
  foliage.set_scene_bush_diffuse_factor(factor);
 }
 void set_bush_texture_name_00ad7fe0(const std::string& name) override {
  foliage.set_scene_bush_texture_name(name);
 }
 void set_bush_normal_allowed_00ad5730(std::uint8_t allowed) override {
  foliage.set_scene_bush_normal_allowed(allowed);
 }
 // 004EE481 CALL 004E7220 with the g_Environment bag (ECX) and the record
 // (EDX): its AreaNN entries into record+C84h.., then the 004BA870 scatter tail
 // (004E7403). This renderer graph owns no scene record cloud block and no
 // atmosphere or cloud entities to scatter, so the areas are not loaded here.
 void load_cloud_areas(const ScenePropertyBagModel&,int) override {
  log.unimplemented("MissionSceneEnvironment::load_cloud_areas","004e7220");
 }
 void load(const ScenePropertyBlock& root,bool linearize) {
  const auto model=scene_weather_cc13::scene_bag_from_block(root);
  const auto* environment=scene_property_bag_find(model,0,"g_Environment");
  const bool group=environment && environment->value.type==ScenePropertyType::SubBag;
  const auto* lights_entry=group ? scene_property_bag_find(model,environment->sub_bag,"Lights") : nullptr;
  lights_authored=lights_entry && lights_entry->value.type==ScenePropertyType::SubBag;
  // 004EA870 only writes scene+C44h when this optional property exists.
  // Preserve the existing parsed RGBA; its absent-property preimage is unknown.
  water_tracer_color_authored=lights_authored &&
   scene_property_bag_find(model,lights_entry->sub_bag,"WaterTracerColor")!=nullptr;
  // Without the group 004EE450 is not called; without Lights it copies the
  // 004B6800 defaults over +F48h, and 004B6800 leaves +20h..+7Fh and +B8h
  // untouched. This owner has no recovered prior values for that
  // cube/specular state, so it admits neither. Reject before reading.
  if(!lights_authored)
   throw std::runtime_error("selected scene lighting requires a g_Environment.Lights sub-bag");
  std::string error;
  // The single existing 004EE450 body, including 004EA870's native type
  // checks, alpha/luminance handling, PC24 arithmetic and scene angle
  // conversion, and the HDR_ prefixes. Never fall back on failure.
  if(!scene_weather_cc13::read_sky_and_lights_004ee450(
      model,environment->sub_bag,linearize,sky,lights,*this,error))
   throw std::runtime_error("selected scene environment: "+error);
  log.implemented("MissionSceneEnvironment::read_sky_and_lights","004ee450");
 }
};

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
 // The record's +F48h..+1010h lighting block as 004EE450 left it.
 std::optional<scene_weather_cc13::SceneLightsFields> scene_lights;
 volatile U published_resource{},null_resource{};
 Phase phase{Phase::prepared};

 // A1: the startup domain's registry/gate recipe when borrowed (never under
 // kNativeRendererStartupCc15Bound OFF); the members above stay unused then.
 SceneRegistryContexts* const borrowed;
 NativeSceneRegistrationGateContext& gates() noexcept {return borrowed?borrowed->gate:gate;}
 NativeSceneResourceStorageContext& storage_context() noexcept {
  return borrowed?borrowed->resource_storage:resource_storage;
 }
 NativeSceneResourceIdentityContext& identity_context() noexcept {
  return borrowed?borrowed->resource_identity:resource_identity;
 }
 NativeGuiSceneFieldsContext& fields() noexcept {return borrowed?borrowed->scene_fields:scene_fields;}

 MissionShipLightingGraph(GameHostLog& output,DestructionGraph& g,CameraGraph& c,
     NativeRenderActualOwnerRegistry& actual,NativeStringRawPoolContext& raw,
     std::uint8_t startup_scale_ambient,SceneRegistryContexts* startup_registry=nullptr)
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
   scene_fields{actual,game_native_renderer_scalar_process().increment_iat_00ce221c(),c.decrement},
   borrowed(startup_registry) {}
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
 void load_scene_lights(const MissionSceneEnvironment& environment) {
  check(phase==Phase::prepared && !scene_lights,
      "mission lighting can load its selected scene only once before creation");
  check(environment.lights_authored,
      "mission lighting requires the selected scene's authored Lights block");
  scene_lights=environment.lights;
 }
 static SystemLightingWords4 scene_color(const scene_weather_cc13::Rgba& value) {
  // Preserve the authored fourth word: 004BACD0 uses it as a multiplier.
  return {float_bits(value.r),float_bits(value.g),float_bits(value.b),float_bits(value.a)};
 }
 void configure_scene_owners() {
  void* const actual_ambient=get<void*>(resource,0x10);
  check(actual_ambient && sun_owner && scene_lights,
      "mission lighting requires its selected scene and current native light owners");
  const auto& fields=*scene_lights;
  const auto ambient_words=scene_color(fields.ambient_f48);
  const auto underwater_ambient=scene_color(fields.ambient_mode3_f58);
  std::array<SystemLightingWords4,6> cube;
  for(std::size_t face=0;face<cube.size();++face)cube[face]=scene_color(fields.ambient_cube_f68[face]);
  const auto diffuse=scene_color(fields.base_diffuse_fc8);
  const auto underwater_diffuse=scene_color(fields.diffuse_mode3_fd8);
  const auto specular=scene_color(fields.base_specular_fe8);
  const U diffuse_scale=float_bits(fields.diffuse_scale_ffc);
  const U specular_scale=float_bits(fields.specular_scale_1000);
  const U horizontal=float_bits(fields.horizontal_angle_1004);
  const U vertical=float_bits(fields.vertical_angle_1008);
  const LightingConfigurationFields configuration{ambient_words,underwater_ambient,cube,
      diffuse,underwater_diffuse,specular,diffuse_scale,specular_scale,horizontal,vertical};
  LightingConfigurationGlobals global_fields{globals.half_guard,globals.half_color,
      globals.diffuse_guard,globals.diffuse_color,globals.specular_guard,globals.specular_color,
      fallback_direction,captured_scale_ambient,*graph.profiles(0xce3800),*graph.profiles(0xd7a24c)};
  auto& light=sun_owner->light;
  LightingDirectionalFields directional{light.diffuse_184,light.specular_194,
      light.base_diffuse_1a4,light.diffuse_mode3_1b4,light.base_specular_1c4,
      light.diffuse_scale_1d8,light.specular_scale_1dc,light.direction_1e0};
  apply_lighting_configuration_004bacd0(
      borrow_native_ambient_lighting_fields(actual_ambient,owners),directional,
      &configuration,global_fields);
  check(light.shadow_174==nullptr,"mission sun unexpectedly acquired a shadow owner");
 }
 void configure_owners() {
  configure_scene_owners();
 }
 void create_and_bind(void* actual_outer_scene) {
  check(scene_lights.has_value(),
      "mission light binding requires a completely read authored Lights header");
  check(phase==Phase::prepared && actual_outer_scene &&
      get<U>(actual_outer_scene)==0xd62d48 && get<void*>(actual_outer_scene,0x1c)==nullptr,
      "mission lighting must bind after model roots on the native null-lighting scene");
  phase=Phase::creating;outer=actual_outer_scene;
  try {
   auto& pools=game_native_resource_pool_process();
   if(!borrowed){
    graph.types.initialize_light_00cd80a0();
    graph.types.initialize_directional_00cd80f0();
    pools.initialize_directional_light_once_00cd8080();
   } // A1 startup already ran these image CRT initializers once.
   sun_pool=&pools.directional_light_pool_01090154();
   construct_native_string_header_0041e870(&resource_name,strings,"Mission ship lighting");
   resource_name_live=true;
   resource=raw_allocate(0x3c);
   check(resource!=nullptr,"mission lighting resource allocation returned null");
   resource_frame.name_or_ambient_argument=bits(&resource_name);
   construct_native_scene_resource_00b83c50(resource,resource_frame,storage_context(),resource_acquired);
   resource_reference.emplace(resource,identity_context(),resource_acquired);

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
   register_native_scene_node_if_light_00b83d50(resource,insert_frame,gates(),insertion);
   check(insertion.complete && get<U>(resource,0x20)==1,
       "mission lighting did not register its one native directional light");
   const void* const head=get<void*>(resource,0x1c);
   check(head && get<void*>(head)!=head && get<void*>(get<void*>(head),8)==sun,
       "mission lighting first native registry entry is not its sun");
   published_resource=bits(resource);
   set_native_gui_scene_resource_00b723f0(outer,published_resource,fields());
   check(get<void*>(outer,0x1c)==resource,"mission outer scene did not retain native lighting resource");
   sun_name_live=false;destroy_native_string_header_0041dd20(&sun_name,strings);
   resource_name_live=false;destroy_native_string_header_0041dd20(&resource_name,strings);
   phase=Phase::ready;
   log.notef("native mission lighting ready: outer=%p resource=%p ambient=%p sun=%p first_light=1 "
       "configuration=004EE450->004BACD0 illumination=selected_scene authored=%u captured_scale_ambient=%u shadow=null",
       outer,resource,get<void*>(resource,0x10),sun,unsigned(scene_lights.has_value()),unsigned(captured_scale_ambient));
  } catch(...) {phase=Phase::failed;throw;}
 }
 void begin_root_attachment() {
  check(phase==Phase::ready && get<void*>(outer,0x1c)==resource &&
      resource_reference->reference_count.load()==2,
      "mission root insertion requires its genuine live lighting credits");
  phase=Phase::roots_detached;
  set_native_gui_scene_resource_00b723f0(outer,null_resource,fields());
  check(get<void*>(outer,0x1c)==nullptr && resource_reference->reference_count.load()==1,
      "mission root insertion did not preserve the lighting creator");
 }
 void end_root_attachment() {
  check(phase==Phase::roots_detached && get<void*>(outer,0x1c)==nullptr &&
      resource_reference->reference_count.load()==1,
      "mission root insertion lost its lighting creator");
  set_native_gui_scene_resource_00b723f0(outer,published_resource,fields());
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
   set_native_gui_scene_resource_00b723f0(outer,null_resource,fields());
   check(get<void*>(outer,0x1c)==nullptr && resource_reference->reference_count.load()==1,
       "mission lighting failed to release the outer scene resource credit");
   erase_frame.node_argument=bits(sun);
   unregister_native_scene_node_if_light_00b83ec0(resource,erase_frame,gates());
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
