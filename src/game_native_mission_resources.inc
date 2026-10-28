// Opt-in mission composition of existing native resource producers. This file
// is included only in the renderer application translation unit, after the
// persistent RenderResourcesGraph. Construct this HOST graph only for the
// BSP_NATIVE_MISSION_SHIP=1 branch; its constructor performs no asset load.
struct MissionRenderResourcesGraph {
 enum class Phase { prepared, constructing, initializing, ready, failed, closed };
 RenderResourcesGraph& resources;
 void* const volatile& renderer;
 NativeLuaServiceBindings& lua;
 NativeShaderBinaryCacheContext& shader_cache;
 NativeShaderBinaryCacheOperation shader_creation;
 bool shader_cache_created{},shader_cache_released{};
 const volatile U& screen_negative_half;
 const volatile U& screen_one;
 NativeRenderResourceInitArguments arguments;
 NativeRenderResourceInitialization operation;
 NativeMaterialParameterAccess parameters;
 NativePostEffect20OwnerContext post20_lifetime;
 NativePostEffectOwnerContext post24_lifetime;
 NativePostEffect20ConstructionContext post20;
 NativePostEffectConstructionContext post24;
 NativeRenderEffectLifetimeContext effects_lifetime;
 // A1: the startup domain's environment when borrowed; owned otherwise.
 std::unique_ptr<NativeGuiSceneEnvironment> owned_scenes;
 NativeGuiSceneEnvironment& scenes;
 NativeDistortionLifetimeContext distortion_lifetime;
 NativeRenderPassCompanionContext pass_companions;
 NativeRenderPassInitializationContext passes;
 NativeDownscalePassInitializationContext downscale;
 NativeLuminanceInitializationContext luminance;
 NativeBrightPassInitializationContext bright;
 NativeBloomInitializationContext bloom;
 NativeRenderResourceInitEntryContext entry;
 NativeRenderResourceInitContinuationContext continuation;
 NativeDistortionInitializationContext distortion;
 NativeDistortionOwnerConstants distortion_constants;
 NativeRenderResourcesDepthContext depth;
 NativeRenderResourceInitDistortionContext distortion_entry;
 NativePostEffectDustContext dust;
 NativeRenderResourceInitPassthroughDustContext passthrough_dust;
 NativeRenderResourceInitDofNames dof_names;
 NativeRenderResourceInitPost658Names post658_names;
 NativeRenderResourceInitPost65cNames post65c_names;
 NativeRenderResourceInitPost664Names post664_names;
 NativeRenderResourceInitTailNames tail_names;
 NativeRenderResourceInitializerContext initializer;
 Phase phase{Phase::prepared};

 static const char* literal(DestructionGraph& g,U address) {
  return static_cast<const char*>(g.profiles.data.data_at(address,1));
 }
 MissionRenderResourcesGraph(RenderResourcesGraph& r,DestructionGraph& g,CameraGraph& camera,
     TextureLoadingGraph& loading,NativeVertexDeclarationCacheContext& declarations,
     GuiTextNativeLayoutServices& layouts,NativeStringRawPoolContext& raw,
     GameNativeVfsRawServices& vfs,GameSingletonHost& host,NativeRenderActualOwnerRegistry& owners,
     NativeMaterialEffectCacheContext& effect_cache,NativeMaterialDestructionAccess& materials,
     NativeModelEnvironment& models,NativeTracelineRenderAccess& draw_entries,
     void* const volatile& current_renderer,void* const volatile& configuration,NativeLuaServiceBindings& lua_services,
     NativeShaderBinaryCacheContext& binary_cache,NativeRenderResourceInitArguments args,
     NativeGuiSceneEnvironment* startup_scenes=nullptr)
  :resources(r),renderer(current_renderer),lua(lua_services),shader_cache(binary_cache),
   screen_negative_half(*g.profiles(0xce69d0)),screen_one(*g.profiles(0xd7a24c)),
   arguments(args),
   parameters{game_native_material_process().parameter_pool_00f8d3e4(),vfs.strings,
       &::_stricmp,g.profiles(0xd61a00)},
   post20_lifetime{owners,g.nodes,g.frame,camera.decrement,g.profiles(0xd61ec0),
       g.profiles(0xd5e520),g.profiles(0xd5e600)},
   post24_lifetime{owners,g.nodes,g.frame,camera.decrement,g.profiles(0xd61ec8),
       g.profiles(0xd61d6c),g.profiles(0xd5e520),g.profiles(0xd5e600)},
   post20{post20_lifetime,loading.meshes,
       {*g.profiles(0xce4adc),*g.profiles(0xce4970),*g.profiles(0xd7a24c)},
       loading.sections,*g.profiles(0xce4970),materials,effect_cache,loading.geometry,
       declarations,g.vertex,layouts,models,camera.environment,camera.node_constants,
       camera.viewports,draw_entries,raw,vfs.strings,renderer,g.profiles(0xd5f0a8),
       g.profiles(0xd619a0),g.profiles(0xd5e5f8),literal(g,0xd61ef8),
       literal(g,0xd61ee4),literal(g,0xd61ed0)},
   post24{post24_lifetime,loading.meshes,
       {*g.profiles(0xce4adc),*g.profiles(0xce4970),*g.profiles(0xd7a24c)},
       loading.sections,*g.profiles(0xce4970),materials,effect_cache,loading.geometry,
       declarations,g.vertex,layouts,models,camera.environment,camera.node_constants,
       camera.viewports,draw_entries,raw,vfs.strings,renderer,g.profiles(0xd5f0a8),
       g.profiles(0xd619a0),g.profiles(0xd5e5f8),literal(g,0xd61f0c),
       literal(g,0xd61ee4),literal(g,0xd61ed0)},
   effects_lifetime{owners,r.holders,camera.decrement,g.profiles(0xd61ec0),g.profiles(0xd61eb8)},
   owned_scenes(startup_scenes ? nullptr : std::make_unique<NativeGuiSceneEnvironment>(NativeGuiSceneEnvironment{
       g.nodes,host.weak_owners(),*g.profiles(0xd7a24c),g.profiles(0xd62d48)})),
   scenes(startup_scenes ? *startup_scenes : *owned_scenes),
   distortion_lifetime{effects_lifetime,g.nodes,g.frame,g.profiles(0xd5e600),g.profiles(0xd62d48),
       operation.distortion_scene_owner_3c()},
   pass_companions{owners,effects_lifetime,{g.profiles(0xd5e164),g.profiles(0xd5e178),
       g.profiles(0xd5e18c),g.profiles(0xd5e1a0),g.profiles(0xd5e1b4),g.profiles(0xd61fe0),
       g.profiles(0xd62150),g.profiles(0xd61f1c)},&distortion_lifetime},
   passes{post20,r.holders,parameters,*reinterpret_cast<const volatile double*>(g.profiles(0xd7a280)),
       *reinterpret_cast<const volatile double*>(g.profiles(0xd57da0)),literal(g,0xd620c0),
       literal(g,0xd5e41c),literal(g,0xd5e40c)},
   downscale{passes,configuration,
       {*reinterpret_cast<const volatile float*>(g.profiles(0xce3978)),
        *reinterpret_cast<const volatile double*>(g.profiles(0xce3d78)),
        *reinterpret_cast<const volatile double*>(g.profiles(0xcec9e0))},
       literal(g,0xd620e8),literal(g,0xd620fc),literal(g,0xd620d8)},
   luminance{post20,r.holders,parameters,{literal(g,0xd5e1d0),literal(g,0xd62050),
       literal(g,0xd6203c),literal(g,0xd6203c),literal(g,0xd62024),literal(g,0xd62008)},
       literal(g,0xd5e40c),literal(g,0xd61ff4)},
   bright{post20,r.holders,parameters,literal(g,0xd62138),{literal(g,0xd62124),
       literal(g,0xd62110),literal(g,0xd5e3f0),literal(g,0xd5e3c0),literal(g,0xd5e3b0)}},
   bloom{{r.holders,g.profiles(0xd61948),*reinterpret_cast<const volatile float*>(g.profiles(0xce3978)),
       *reinterpret_cast<const volatile double*>(g.profiles(0xd7a280))},post20,parameters,
       literal(g,0xd62180),literal(g,0xd62164),literal(g,0xd5e40c),
       literal(g,0xd61fac),literal(g,0xd62170)},
   entry{g.frame,g.profiles(0xd5e600),g.profiles(0xd5f0a8),camera.decrement},
   continuation{entry,passes,downscale,luminance,bright,bloom,effects_lifetime,pass_companions,
       g.profiles(0xd61948),*g.profiles(0xce6650),*g.profiles(0xce3854),
       {literal(g,0xd5e448),literal(g,0xd5e41c),literal(g,0xd5e3fc),literal(g,0xd5e360)},
       {literal(g,0xd5e430),literal(g,0xd5e40c),literal(g,0xd5e3f0),literal(g,0xd5e3e4),
        literal(g,0xd5e3d0),literal(g,0xd5e3c0),literal(g,0xd5e3b0),literal(g,0xd5e430),
        literal(g,0xd5e3a8),literal(g,0xd5e3a0),literal(g,0xd5e384),literal(g,0xd5e374)}},
   distortion{{bloom.textures,renderer,g.profiles(0xd5f0a8)},post20,parameters,scenes,
       literal(g,0xd61fcc),literal(g,0xd61fbc),literal(g,0xd5e40c),literal(g,0xd61fac),
       literal(g,0xd61f9c),literal(g,0xd61f80),literal(g,0xd61f70),literal(g,0xd61f64),
       literal(g,0xd61f50),literal(g,0xd61f40),literal(g,0xd61f30),literal(g,0xd5e448),literal(g,0xd5e430)},
   distortion_constants{*g.profiles(0xcf4848),*g.profiles(0xce3958),*g.profiles(0xd1f3c4)},
   depth{r.surfaces,g.profiles(0xd5f0a8),g.profiles(0xd619a0),camera.decrement},
   distortion_entry{distortion,distortion_constants,pass_companions,depth},
   dust{post24,literal(g,0xd620ac)},
   passthrough_dust{dust,literal(g,0xd5e314),literal(g,0xd5e2f8),literal(g,0xd5e2e4)},
   dof_names{literal(g,0xd5e348),literal(g,0xd5e338),literal(g,0xd5e324)},
   post658_names{literal(g,0xd5e2cc),literal(g,0xd5e2bc),literal(g,0xd5e2ac),
       literal(g,0xd5e2a0),literal(g,0xd5e294),literal(g,0xd5e28c)},
   post65c_names{literal(g,0xd5e26c),literal(g,0xd5e260),literal(g,0xd5e254)},
   post664_names{literal(g,0xd5e240),literal(g,0xd5e230),literal(g,0xd5e220),
       literal(g,0xd5e214),literal(g,0xd5e208),literal(g,0xd5e1fc)},
   tail_names{literal(g,0xd5e1e4),literal(g,0xd5e1d0)},
   initializer{continuation,distortion_entry,passthrough_dust,dof_names,
       post658_names,post65c_names,post664_names,tail_names} {}

 void* construct_and_initialize() {
  check(phase==Phase::prepared,"mission render resources cannot replay");
  // B4F0C0 preserves +34/+38/+3C. The original B4F560 false-result cleanup
  // cannot consume an arbitrary allocator preimage. Admit this success domain
  // only while its SAME serialized renderer supports the exact capability
  // prefix. Failure here precedes every service/native child allocation.
  auto* current=renderer;
  check(current && (check_native_renderer_device_format_00b21ec0(current,22,16,3,111) ||
      check_native_renderer_device_format_00b21ec0(current,22,16,3,114)) &&
      check_native_renderer_device_format_00b21ec0(current,22,16,3,112),
      "mission resources require the native distortion success capability domain");
  phase=Phase::constructing;
  try {
   NativeLuaServiceBindings::Activation activation(lua);
   // The original global cache is shared. Open a fresh native bracket only
   // when startup has closed its earlier bracket; never replace a live owner.
   if(!shader_cache.actual_cache_0108d6ec) {
    create_native_shader_binary_cache_00b3a600(shader_cache,shader_creation);
    shader_cache_created=true;
   }
   resources.lifetime.direct_terminals=&resources.direct_terminals;
   void* owner=resources.construct();
   check(owner!=nullptr,"mission render-resource allocation returned null");
   phase=Phase::initializing;
   initialize_native_render_resources_00b107f0(owner,arguments,initializer,operation);
   check(operation.phase()==NativeRenderResourceInitialization::Phase::normal_work_complete,
       "mission resources require the complete native initialization path");
   if(shader_cache_created) {
    check(shader_cache.actual_cache_0108d6ec==shader_creation.cache,
        "mission shader-cache release cannot retarget its created owner");
    release_native_shader_binary_cache_00b3b140(shader_cache);
    shader_cache_released=true;
   }
   phase=Phase::ready;
   return owner;
  } catch(...) {phase=Phase::failed;throw;}
 }
 bool requires_process_retention() const noexcept {
  return phase==Phase::constructing || phase==Phase::initializing || phase==Phase::failed ||
      operation.requires_process_retention() || (shader_cache_created && !shader_cache_released);
 }
 void inverse_half_dimension(U dimension,float& result) const noexcept {
  const volatile float* bias=&bloom.textures.unsigned_bias_00ce3978;
  const volatile double* half=&bloom.textures.half_00d7a280;
  float* output=&result;
  __asm {
   mov eax,dimension
   test eax,eax
   fild dword ptr dimension
   jge dimension_signed
   mov eax,bias
   fadd dword ptr [eax]
  dimension_signed:
   mov eax,half
   fdivr qword ptr [eax]
   mov eax,output
   fstp dword ptr [eax]
  }
 }
 // Extract the existing B1586D..B158D7 half-texel producer, followed by the
 // existing B15D1D..B15DDB screen-matrix producer. This is a bounded Source
 // fragment, not B156C0 entry/return or D5E13C admission. Current concrete
 // holder/texture dispatch, unsigned x87 conversion, FSTP32/reload and every
 // matrix store remain native. No main-scene target/camera flags are changed.
 void prepare_screen_matrix() {
  check(phase==Phase::ready,"mission screen matrix requires initialized resources");
  void* owner=resources.publication;
  check(owner!=nullptr,"mission screen matrix requires the live native service");
  // B1586D/B15875 capture HEIGHT source first, WIDTH source second.
  void* height_source=native_shadow_holder_texture_00b4cb10(get<void*>(owner,0x4c));
  void* width_source=native_shadow_holder_texture_00b4cb10(get<void*>(owner,0x4c));
  const auto* table=continuation.actual_texture_profile_00d61948;
  check(width_source && get<U>(width_source)==0xd61948 && table && table[0x3c/4]==0xb3ce50,
      "mission half-texel width requires the current native texture getter");
  const U width=get_raw_texture_width_00b3ce50(width_source);
  float saved_width,saved_height;
  inverse_half_dimension(width,saved_width);
  check(height_source && get<U>(height_source)==0xd61948 && table[0x40/4]==0xb3ce60,
      "mission half-texel height requires the current native texture getter");
  const U height=get_raw_texture_height_00b3ce60(height_source);
  inverse_half_dimension(height,saved_height);
  const volatile U* half_float=&resources.cells.actual_00ce3800;
  const volatile U* negative_half=&screen_negative_half;
  const volatile U* one=&screen_one;
  const volatile double* half=&bloom.textures.half_00d7a280;
  float translation_x,translation_y;
  __asm {
   mov eax,owner
   fld dword ptr saved_width
   fstp dword ptr [eax+4]
   fld dword ptr saved_height
   fstp dword ptr [eax+8]
   // B15D1D's non-lens arm establishes one, half and positive zero.
   mov edx,one
   movss xmm1,dword ptr [edx]
   mov edx,half
   fld qword ptr [edx]
   xorps xmm0,xmm0
   fld dword ptr [eax+4]
   mov edx,half_float
   movss xmm2,dword ptr [edx]
   fadd st(0),st(1)
   fstp dword ptr translation_x
   fadd dword ptr [eax+8]
   movss dword ptr [eax+1d8h],xmm2
   mov edx,negative_half
   movss xmm2,dword ptr [edx]
   movss dword ptr [eax+1ech],xmm2
   movss xmm2,dword ptr translation_x
   fstp dword ptr translation_y
   movss dword ptr [eax+208h],xmm2
   movss xmm2,dword ptr translation_y
   movss dword ptr [eax+1dch],xmm0
   movss dword ptr [eax+1e0h],xmm0
   movss dword ptr [eax+1e4h],xmm0
   movss dword ptr [eax+1e8h],xmm0
   movss dword ptr [eax+1f0h],xmm0
   movss dword ptr [eax+1f4h],xmm0
   movss dword ptr [eax+1f8h],xmm0
   movss dword ptr [eax+1fch],xmm0
   movss dword ptr [eax+200h],xmm1
   movss dword ptr [eax+204h],xmm0
   movss dword ptr [eax+20ch],xmm2
   movss dword ptr [eax+210h],xmm0
   movss dword ptr [eax+214h],xmm1
  }
 }
 void after_native_drain() {
  check(!requires_process_retention(),"mission resource initialization requires process retention");
  check(!resources.publication,"mission resource owner survived native drain");
  operation.close_children_after_host_quiescence();
  phase=Phase::closed;
 }
};
