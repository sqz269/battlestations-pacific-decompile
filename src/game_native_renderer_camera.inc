// Host companions over the application's existing raw renderer and node domain.
// This graph does not allocate a native camera; its producers own those lifetimes.
struct CameraGraph {
 struct RawViewportAccess final : NativeViewportRendererAccess {
  const volatile U* table;
  explicit RawViewportAccess(const volatile U* profile) : table(profile) {}
  NativeViewportRendererParameters parameters_00b1ff60(D3D9StateCache& captured) override {
   // The legacy interface carries an address. Never invoke semantic renderer
   // methods on this raw owner. Re-read its profile/slot on EACH native call.
   auto* actual=reinterpret_cast<unsigned char*>(&captured);
   check(get<U>(actual)==0xd5f0a8 && table[0x30/4]==0xb1ff60,
       "viewport current renderer parameter slot is outside the concrete profile");
   // Match the borrowed DWORD type exactly: uint32_t is unsigned int under
   // MSVC, while DWORD is unsigned long; conversion would dangle references.
   return {*reinterpret_cast<const DWORD*>(actual+0x1a20),
       *reinterpret_cast<const DWORD*>(actual+0x1a24)};
  }
 } access;
 CameraTypeBootstrap types;
 NativeViewportRegistry viewports;
 NativeViewportRegistryBinding installed_viewports;
 NativeViewportEnvironment viewport;
 NativeNodeRawConstants node_constants;
 NativeCameraEnvironment environment;
 // Source equivalent of the imported CE2220 cell; resolve the real Windows
 // export once, then retain the callable cell for current-value reads.
 NativeCockpitInterlockedDecrement const decrement;
 NativeCockpitViewportReleaseContext viewport_release;
 static NativeCockpitInterlockedDecrement resolve_decrement() {
  const auto proc=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"InterlockedDecrement");
  check(proc!=nullptr,"missing actual InterlockedDecrement export");
  NativeCockpitInterlockedDecrement result;
  static_assert(sizeof(result)==sizeof(proc));std::memcpy(&result,&proc,sizeof result);return result;
 }
 CameraGraph(DestructionGraph& g,void* const volatile& renderer,
     TypeIdCounterLifetime& counter,CameraTypeBootstrapStorage shared_types)
  :access(g.profiles(0xd5f0a8)),types(counter,g.types,shared_types),
   installed_viewports(viewports),
   viewport{reinterpret_cast<D3D9StateCache* const volatile&>(renderer),access,*g.profiles(0xd7a24c)},
   node_constants{*g.profiles(0xce4970),*g.profiles(0xd7a24c),*g.profiles(0xce4adc)},
   environment{game_native_resource_pool_process().camera_pool_0108ffb0(),g.nodes,types,
       viewport,viewports,application_camera_axes_crt(),
       {*g.profiles(0xce3d08),*g.profiles(0xce7d20),*g.profiles(0xd0c5f8),
        *g.profiles(0xd5bd98),*g.profiles(0xce77fc),*g.profiles(0xce3c88)},
       g.profiles(0xd62cf0),g.profiles(0xd62c88)},
   decrement(resolve_decrement()),viewport_release{decrement,g.profiles(0xd5e5f8)} {}
};
