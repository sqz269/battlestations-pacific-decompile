// Grid construction borrows the application's existing renderer, declaration
// cache, geometry registry, pools, locks and actual device recreation service.
// This graph only composes contexts; native allocations remain at 70BD70/70B330.
struct GameGridGraph {
 NativeLogicalIndexCreationContext indices;
 NativeLogicalBufferMappingContext mapping;
 NativeStreamCloneServices streams;
 GuiTextNativeRendererServices graphics;
 NativeGameGridCpuCalls cpu;
 NativeGameGridContext context;

 GameGridGraph(DestructionGraph& g,DeviceGraph& d,TextureLoadingGraph& textures,
  NativeVertexDeclarationCacheContext& declarations,NativeStringStorage& strings,
  void* const volatile& renderer)
  :indices{g.index,g.profiles(0xd5f0a8),g.profiles(0xd61de0),g.recreation,0},
   mapping{g.vertex.actual_renderer_00f8d394,g.sync,d.lock,g.physical_profiles},
   streams{textures.geometry,g.vertex,indices,mapping},graphics{declarations,streams},
   context{graphics,strings,cpu,
    {*g.profiles(0xd7a24c),*g.profiles(0xcfd470),*g.profiles(0xd7a260),
     *g.profiles(0xce5380),*g.profiles(0xce397c),*g.profiles(0xcfd46c),
     game_native_geometry_globals().zero_vector_00f87574()[0],
     game_native_geometry_globals().zero_vector_00f87574()[1],
     game_native_geometry_globals().zero_vector_00f87574()[2],
     *static_cast<const double*>(g.profiles.data.data_at(0xce3d28,8)),
     *static_cast<const double*>(g.profiles.data.data_at(0xd7a3a0,8)),
     static_cast<const char*>(g.profiles.data.data_at(0xcfd4e4,11))}} {
   // The source application's explicit pool-header preimage above is not a
   // recovered native default. Grid index flags=1 never consume that input.
   require_gui_text_native_renderer_domain(graphics,renderer,strings,textures.geometry.actual_owners());
  }
};
