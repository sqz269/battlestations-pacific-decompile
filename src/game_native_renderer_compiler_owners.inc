// B3B3C0's producers share the application's native owner registry, pass pool,
// string pool, system-constant registry, support publication and renderer.
struct CompilerOwnersGraph {
 NativeRenderActualOwnerRegistry& owners;
 CanonicalProfiles profiles;
 NativeMaterialPassDestructionAccess pass_lifetime;
 NativeMaterialPassConstructionAccess passes;
 NativeD3dx9ShaderConstantTableImport reflection_import;
 NativeCompiledShaderReflectionContext reflection;
 NativeD3d9ShaderConstructionActualContext shaders;
 NativeD3d9ShaderLifetimeContext shader_lifetime;
 NativeShaderConstantHeaderContext lines;
 NativeShaderFieldInitializationContext source;
 NativeVertexShaderD3dxImports vertex_import;
 NativePixelShaderD3dxImports pixel_import;
 NativeVertexShaderCompilationContext vertex;
 NativePixelShaderCompilationContext pixel;
 std::atomic<std::uint32_t> live_companions{0};

 template<class Reference,class Disposal> struct Companion {
  CompilerOwnersGraph& graph;
  void* raw; // Identity only after the native terminal has freed its storage.
  Reference reference;
  template<class... Args> Companion(CompilerOwnersGraph& g,void* p,Args&&... args)
   :graph(g),raw(p),reference(std::forward<Args>(args)...,Disposal{this,&retire}) {}
  static void retire(void* context,Reference& reference) noexcept {
   auto* self=static_cast<Companion*>(context);
   if(&self->reference!=&reference)std::terminate();
   self->graph.owners.unbind(self->raw,reference);
   self->graph.live_companions.fetch_sub(1,std::memory_order_relaxed);
   delete self;
  }
 };
 template<class Reference,class Disposal,class... Args>
 void bind(void* raw,Args&&... args) noexcept {
  auto* entry=new Companion<Reference,Disposal>(*this,raw,std::forward<Args>(args)...);
  owners.bind(raw,entry->reference);
  live_companions.fetch_add(1,std::memory_order_relaxed);
 }
 static void bind_states(void* context,NativeMaterialPassBaseStorage& pass) noexcept {
  auto& self=*static_cast<CompilerOwnersGraph*>(context);
  for(auto* state:{pass.render_18,pass.third_1c,pass.sampler_20}) {
   const auto profile=get<U>(state);
   check(profile==0xd61a2c || profile==0xd61a34 || profile==0xd61a3c,
       "compiler state owner has an unsupported original profile");
   self.bind<NativeMaterialStateReference,NativeMaterialStateCompanionDisposal>(
       state,*state,self.profiles(profile));
  }
 }
 static void bind_pass(void* context,NativeMaterialPassStorage& pass) noexcept {
  auto& self=*static_cast<CompilerOwnersGraph*>(context);
  self.bind<NativeMaterialPassReference,NativeMaterialPassCompanionDisposal>(
      &pass,pass,self.pass_lifetime,self.profiles(0xd61be8));
 }
 static void bind_reflection(void* context,NativeCompiledShaderStorage& shader) noexcept {
  auto& self=*static_cast<CompilerOwnersGraph*>(context);
  self.bind<NativeCompiledShaderReference,NativeCompiledShaderCompanionDisposal>(
      &shader,shader,self.reflection.strings,self.profiles(0xd61810));
 }
 static void bind_shader(void* context,NativeD3d9ShaderStorage& shader) noexcept {
  auto& self=*static_cast<CompilerOwnersGraph*>(context);
  self.bind<NativeD3d9ShaderReference,NativeD3d9ShaderCompanionDisposal>(
      &shader,shader,self.shader_lifetime);
 }
 CompilerOwnersGraph(NativeRenderActualOwnerRegistry& registry,CanonicalProfiles p,
     GameNativeVfsRawServices& vfs,NativeStringRawPoolContext& raw,
     NativeResourceSupportRawContext& support,void* const volatile& renderer,
     void* const volatile& constants,HMODULE d3dx)
  :owners(registry),profiles(p),
   pass_lifetime{vfs.strings,registry,game_native_graphics_pool_process().material_pass_pool_0108fbf8()},
   passes{pass_lifetime,renderer,{this,&bind_states}},reflection_import(d3dx),
   reflection{vfs.strings,constants,reflection_import},
   shaders{raw,support.actual_published_0108fedc,renderer},
   shader_lifetime{support.actual_published_0108fedc,NativeResourceSupportLifetime(support),
       renderer,registry,profiles(0xd62a60),profiles(0xd62a70)},
   lines{vfs.strings,constants,game_native_shader_process().system_register_limit_00e13078(),
       game_native_shader_process().format_scratch_0108d6f8(),
       game_native_shader_process().source_empty_0108d6f2()},
   source{lines,game_native_shader_process().instance_empty_00e17654()},
   vertex_import(d3dx),pixel_import(d3dx),
   vertex{vfs.strings,renderer,vfs.actual_vfs_publication_0109ceec,vfs.bindings,vertex_import},
   pixel{vfs.strings,renderer,vfs.actual_vfs_publication_0109ceec,vfs.bindings,pixel_import} {}
 GameNativeMaterialCompilerOwners borrowed() noexcept {
  return {passes,reflection,shaders,{this,&bind_pass,&bind_reflection,&bind_shader}};
 }
 GameNativeMaterialCompilerSources borrowed_sources() noexcept {return {source,vertex,pixel};}
 bool quiescent() const noexcept {return live_companions.load(std::memory_order_relaxed)==0;}
 ~CompilerOwnersGraph(){if(!quiescent())std::terminate();}
};
