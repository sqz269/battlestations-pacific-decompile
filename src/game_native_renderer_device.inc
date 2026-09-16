// Private application device graph, retained through native drain.
struct DeviceGraph {
 DestructionGraph& graph;
 const void* volatile& renderer;
 const void* volatile platform_publication;
 volatile U retries=0,render_thread=0;
 volatile std::uint8_t srgb=0;
 std::array<U,4> null_buffer_sentinel{};
 NativeRendererResourceRestoreProfiles resource_profiles;
 NativeRendererResourceReleaseContext release;
 NativeRendererResourceRestoreContext restore;
 NativeRendererFrameTargetsContext targets;
 NativePhysicalBufferLockContext lock;
 NativeLogicalBufferDeviceSaveProfiles save_profiles;
 NativeLogicalBufferDeviceSaveContext save;
 NativeLogicalBufferDeviceRestoreContext logical_restore;
 NativeHardwareLayoutConstructContext hardware;
 NativeRendererResetReadinessProfiles readiness;
 NativeCrtPowFallbackContext fallback;
 NativeCrtPowDispatchContext power;
 NativeRendererGammaContext gamma;
 HMODULE d3dx;
 CreateTextureFromMemory volatile texture_import;
 NativeD3dx9CubeVolumeMemoryImports cube_import;
 NativeTextureResetSurfaceProfile reset_surface;
 NativeTexture2DRetainedRecreationContext texture;
 NativeCubeVolumeRetainedRecreationContext cube_volume;
 NativeRendererDeviceRecreationContext recreation;
 NativeRendererResetProcessContext reset;
 NativeRendererDeviceStartupContext startup;
 static HMODULE load_d3dx() {
  wchar_t directory[MAX_PATH]; const auto count=GetSystemDirectoryW(directory,MAX_PATH);
  check(count && count<MAX_PATH,"system D3DX directory");
  const auto path=std::filesystem::path(directory)/L"d3dx9_40.dll";
  const auto module=LoadLibraryExW(path.c_str(),nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
  check(module!=nullptr,"actual system D3DX module");return module;
 }
 static CreateTextureFromMemory resolve_texture(HMODULE module) {
  const auto address=GetProcAddress(module,"D3DXCreateTextureFromFileInMemoryEx");check(address!=nullptr,"actual D3DX texture import");
  CreateTextureFromMemory result;std::memcpy(&result,&address,sizeof result);return result;
 }
 DeviceGraph(DestructionGraph& g,NativeRendererConstructorContext& c,GameSingletonHost& singletons,ActualNativeStringPoolStorage& strings, const void* platform)
  :graph(g),renderer(const_cast<const void* volatile&>(c.current_renderer_00f8d394)),platform_publication(platform),
   resource_profiles{g.profiles(0xd61948),g.profiles(0xd61870),g.profiles(0xd618b0),g.profiles(0xd619a0),g.profiles(0xd62ad0)},
   release{g.reset,g.cache,resource_profiles},restore{resource_profiles,const_cast<void**>(&c.current_renderer_00f8d394)},
   targets{g.sync,g.frame,g.profiles(0xd5e600),srgb},
   lock{singletons.diagnostic_publication_0109cf14(),singletons.sound_lifetime(),null_buffer_sentinel.data()},
   save_profiles{g.profiles(0xd61e10),g.profiles(0xd61e34),g.profiles(0xd61e58),g.profiles(0xd61e7c)},save{lock,save_profiles},
   logical_restore{renderer,g.physical,lock,g.physical_profiles,0},hardware{g.hardware,strings},
   readiness{g.profiles(0xd61e58),g.profiles(0xd61e7c),g.profiles(0xd61e10),g.profiles(0xd61e34)},
   fallback{application_camera_axes_crt().dispatch_bypass_0109dd78,reinterpret_cast<void*>(0xd6a684),reinterpret_cast<void*>(0xd7a280),reinterpret_cast<void*>(0xe165a0)},
   power{reinterpret_cast<const volatile U*>(0x109eea0),&fallback,nullptr},
   gamma{g.sync,power,*reinterpret_cast<const volatile double*>(0xce3dc0),*reinterpret_cast<const volatile double*>(0xce3d88),*reinterpret_cast<const volatile double*>(0xce4b48),*reinterpret_cast<const volatile double*>(0xce5f90),*reinterpret_cast<const volatile float*>(0xd7a24c)},
   d3dx(load_d3dx()),texture_import(resolve_texture(d3dx)),cube_import(d3dx),reset_surface{g.profiles(0xd619a0)},
   texture{renderer,g.retained,g.profiles(0xd61948),reset_surface,texture_import},cube_volume{const_cast<void**>(&c.current_renderer_00f8d394),g.retained,cube_import},
   recreation{release,restore,targets,save,logical_restore,hardware,game_native_hardware_layout_tree_process().invalid_parameters(),game_native_hardware_layout_tree_process().tree_0108d530(),readiness,gamma,texture,cube_volume,nullptr,nullptr},
   reset{recreation,platform_publication,c.frame_flag_0108d4b8,c.device_lost_0108d4b9,retries,render_thread},
   startup{reset,g.defaults,g.physical,render_thread,*reinterpret_cast<const volatile float*>(0xce3978),*reinterpret_cast<const volatile double*>(0xcf5750)} {
    check(*power.dispatch_0109eea0==0,"actual canonical loader-zero power dispatch");
    g.recreation.context=&recreation;
   }
 ~DeviceGraph(){graph.recreation.context=nullptr;FreeLibrary(d3dx);}
};
