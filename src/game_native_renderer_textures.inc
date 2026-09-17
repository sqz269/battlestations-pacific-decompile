// Loading and deletion borrow the same renderer, VFS, pools and actual owners.
// This composition creates only host metadata; native loads occur at callers.
struct TextureLoadingGraph {
 DestructionGraph& destruction;
 DeviceGraph& devices;
 NativeMeshEnvironment meshes;
 NativeMeshSectionEnvironment sections;
 GuiNativeGeometryOwners geometry;
 NativeNamedVolumeTextureContext named_volume;
 NativeTextureSpecialOwnerContexts specials;
 NativeTextureLoadOwners owners;
 // B2C2D0 uses only the terminating byte of original0108D5A4. Keep the
 // source equivalent stable; it is not a substitute renderer or name resolver.
 const char empty_name[1]{0};
 NativeTextureLoadingContext loading;
 NativeTextureCacheContext cache;

 template<class T> static T import(HMODULE module,const char* name) {
  const auto address=GetProcAddress(module,name);check(address!=nullptr,"missing actual D3DX texture loading import");
  T result;static_assert(sizeof(result)==sizeof(address));std::memcpy(&result,&address,sizeof result);return result;
 }
 static void retry(void* context,const void* renderer) {
  auto& self=*static_cast<TextureLoadingGraph*>(context);
  recreate_native_renderer_device_00b29670(const_cast<void*>(renderer),self.devices.recreation);
 }
 TextureLoadingGraph(DestructionGraph& g,DeviceGraph& d,NativeRenderActualOwnerRegistry& registry,
  GameNativeVfsRawServices& vfs,ResourceLoadEventHost* const volatile& platform,
  const SingletonLifetimeCallbacks& validation,const NativeRenderResourceAccountingTables& accounting)
  :destruction(g),devices(d),
   meshes{game_native_resource_pool_process().mesh_pool_0108fff8(),registry,vfs.strings,g.profiles(0xd62d60)},
   sections{game_native_resource_pool_process().section_pool_010901d4(),registry,g.profiles(0xd63194)},
   geometry(meshes,{*g.profiles(0xce4adc),*g.profiles(0xce4970),*g.profiles(0xd7a24c)},sections,
    {registry,&registry,&NativeRenderActualOwnerRegistry::bind_callback,
     &NativeRenderActualOwnerRegistry::unbind_callback,&NativeRenderActualOwnerRegistry::find_callback}),
   named_volume{g.volume,g.serial},
   specials{g.cube,named_volume,g.profiles(0xd61870),g.profiles(0xd618b0)},
   owners(geometry,g.texture,g.profiles(0xd61948),&specials),
   loading{vfs.strings,owners,vfs.bindings,vfs.opens,vfs.conversion,vfs.actual_vfs_publication_0109ceec,
    g.notification.actual_renderer_00f8d394,g.sync,
    import<ReadImageInfoFromMemory>(d.d3dx,"D3DXGetImageInfoFromFileInMemory"),d.texture_import,
    nullptr,nullptr,empty_name,this,&retry,nullptr,nullptr,
    import<CreateNativeCubeTextureFromMemory>(d.d3dx,"D3DXCreateCubeTextureFromFileInMemory"),
    import<CreateNativeVolumeTextureFromMemory>(d.d3dx,"D3DXCreateVolumeTextureFromFileInMemory")},
   cache{vfs.strings,validation,loading,vfs.dates,platform,g.sync,g.profiles(0xd5f088),accounting} {
    loading.cache=&cache;
    bind_native_texture_vfs_name_resolution(loading,vfs.name_resolution);
   }
};
