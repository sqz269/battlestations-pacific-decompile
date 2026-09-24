// Persistent construction/acquisition and terminal providers. Native allocation
// is explicit, once-only, and preserves every constructor-unwritten byte.
struct RenderResourcesGraph {
 void* volatile publication{};
 NativeRenderServiceBaseContext base;
 NativeRenderServiceTextureConstructionContext textures;
 NativeRenderResourcesInitialCells cells;
 NativeRenderServiceParameterConstants parameters;
 NativeRenderResourcesConstructionContext constructor;
 NativeRenderResourcesConstructionAcquired acquired;
 NativeRenderServiceTextureLifetimeContext texture_lifetime;
 GuiNativeGeometryRegistration registration;
 std::optional<NativeRenderServiceTextureReference> texture_reference;
 NativeRenderResourcesLifetimeContext lifetime;
 // No native allocation/attempt here. Every context borrows the established
 // application domains and the process's canonical accounting cells.
 NativeRuntimeTextureConstructionContext runtime_construction;
 NativeRuntimeTextureCreationContext runtime_textures;
 NativeRendererSurfaceFactoryContext surfaces;
 NativeTextureSurfaceGetterContext texture_surfaces;
 NativeRenderTextureSurfaceOwnerContext holders;
 NativeRenderResourcesDirectTerminalDomain direct_terminals;
 bool attempted{};

 RenderResourcesGraph(DestructionGraph& g,DeviceGraph& devices,CameraGraph& camera,TextureLoadingGraph& loading,
     GameSingletonHost& host,NativeStringRawPoolContext& raw,GameNativeVfsRawServices& vfs,
     NativeRenderActualOwnerRegistry& owners)
  :base{host.manager_publication_01090aa0(),publication},
   textures(loading.cache,raw,vfs.name_resolution,g.profiles(0xd5f0a8),g.profiles(0xd61948),
       g.profiles(0xd620a0),g.profiles(0xd62094),g.profiles(0xd62088),g.profiles(0xd6207c)),
   cells{*g.profiles(0xce54a0),*g.profiles(0xce3804),*g.profiles(0xce4bc4),*g.profiles(0xce74f8),
       *g.profiles(0xce8198),*g.profiles(0xce6650),*g.profiles(0xd7a24c),*g.profiles(0xce3854),
       *g.profiles(0xce38b8),*g.profiles(0xce3800),*g.profiles(0xce3d08),*g.profiles(0xd1f980),
       *g.profiles(0xce3d34),*g.profiles(0xce89d0),*g.profiles(0xce89cc),*g.profiles(0xce89c8),
       *g.profiles(0xd7a238),*g.profiles(0xce3d30),*g.profiles(0xce7628),*g.profiles(0xce6a04)},
   parameters{*g.profiles(0xce4788),*g.profiles(0xd5e138),*g.profiles(0xce3854),*g.profiles(0xd5e134),
       *g.profiles(0xce3850),*g.profiles(0xce3958),*g.profiles(0xcef1b0),*g.profiles(0xce54a0),
       *g.profiles(0xce7804),*g.profiles(0xce69c8),*g.profiles(0xced318),*g.profiles(0xcf2548),
       *g.profiles(0xcef0b8),*g.profiles(0xd7a2f0),*g.profiles(0xd5e130),*g.profiles(0xce3930),
       *g.profiles(0xd5e12c),*g.profiles(0xce380c),*g.profiles(0xce3800),*g.profiles(0xd7a24c),*g.profiles(0xce3d34)},
   constructor{base,textures,g.frame,cells,parameters,camera.environment,camera.node_constants,
       camera.viewports,camera.viewport_release,g.profiles(0xd61854),
       g.profiles(0xd5e474),g.profiles(0xd5e468),g.profiles(0xd5e460)},
   texture_lifetime{owners,camera.decrement,g.profiles(0xd61948),g.profiles(0xd61ec8),g.profiles(0xd62074)},
   registration{owners,&owners,&NativeRenderActualOwnerRegistry::bind_callback,
       &NativeRenderActualOwnerRegistry::unbind_callback,&NativeRenderActualOwnerRegistry::find_callback},
   lifetime{base,raw,texture_lifetime,g.frame,acquired,g.profiles(0xd5e600)},
   runtime_construction{g.texture,g.profiles(0xd619e4)},
   runtime_textures{runtime_construction,g.sync,&devices.recreation,
       game_native_renderer_scalar_process().texture_allocation_bytes_0108d4bc(),
       *reinterpret_cast<const volatile float*>(g.profiles(0xce3978)),
       *reinterpret_cast<const volatile double*>(g.profiles(0xd57da0))},
   surfaces{g.surface,g.sync,&devices.recreation,
       game_native_renderer_scalar_process().surface_allocation_bytes_0108d4c0(),
       runtime_textures.unsigned_dword_fix_00ce3978,runtime_textures.unsigned_counter_fix_00d57da0},
   texture_surfaces{g.surface,game_native_renderer_scalar_process().increment_iat_00ce221c()},
   holders{runtime_textures,texture_surfaces,surfaces,camera.decrement},
   direct_terminals{owners,holders,g.profiles(0xd61eb8)} {}
 GameNativeRenderResourceProviders borrowed_providers() noexcept {
  return {runtime_textures,surfaces,texture_surfaces,holders,direct_terminals};
 }
 void* construct() {
  check(!attempted,"render-resource constructor already attempted");attempted=true;
  void* owner=raw_allocate(0x6ac); // 73DEE4 operator_new; no blanket zero fill.
  if(owner) {
   construct_native_render_resources_00b14a10(owner,constructor,acquired);
   // Host-only canonical binding after native construction succeeds. Retain
   // completed native storage if this admission throws; never replay cleanup.
   void* helper=get<void*>(owner,0x34);
   if(helper)texture_reference.emplace(helper,texture_lifetime,registration);
  }
  return owner;
 }
 void after_native_drain() {
  check(publication==nullptr,"render resources survived native drain");
  if(!acquired.owner)return;
  check(acquired.phase==NativeRenderResourcesConstructionAcquired::Phase::complete,
      "incomplete render-resource construction requires process retention");
  check(!texture_reference || texture_reference->retired(),"render-service texture companion survived drain");
  check(!acquired.helper_completed || acquired.helper_retired,"cockpit helper survived drain");
  acquired.forget_retired_helper_after_host_quiescence();
  acquired.cockpit.reset_after_host_quiescence();
 }
};
