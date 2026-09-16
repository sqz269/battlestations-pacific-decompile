// Private application composition; all shared cells are borrowed from the canonical owners.
struct ActualRecreation final : NativeLogicalVertexDeviceRecreation {
 NativeRendererDeviceRecreationContext* context=nullptr;
 void call_00b29670(void* renderer) override {
  check(context!=nullptr,"renderer device recreation context is unbound");
  recreate_native_renderer_device_00b29670(renderer,*context);
 }
};
struct ActualSceneRuntime final : SceneAttachmentRuntime {
 LightTypeBootstrap& types;
 explicit ActualSceneRuntime(LightTypeBootstrap& value):SceneAttachmentRuntime(0,{0,0,0}),types(value){}
 static bool node_type(SceneAttachmentRuntime& runtime,SceneNodeAttachment&,U token){
  return static_cast<ActualSceneRuntime&>(runtime).types.node_is_type_00b6f570(token);
 }
};
struct DestructionGraph {
 CanonicalProfiles profiles;
 NativeRendererSynchronizationGlobals& sync=game_native_renderer_scalar_process().synchronization_0108d6dc();
 D3D9SurfacePool& surface_pool=game_native_surface_pool_process().surface_pool_0108db00();
 D3D9SurfacePool& volume_pool=game_native_graphics_pool_process().volume_texture_pool_0108dba8();
 D3D9Texture2DPool& texture_pool=game_native_texture_pool_process().texture2d_pool_0108db38();
 volatile U& texture_tracking=game_native_renderer_scalar_process().texture_tracking_counter_0108daf8();
 volatile U next_vertex=0;
 U& surface_tracking=game_native_renderer_scalar_process().surface_tracking_counter_0108dafc();
 U& serial=game_native_renderer_scalar_process().logical_texture_serial_0108d6e8();
 ActualRecreation recreation;
 LightTypeBootstrap types;
 ActualSceneRuntime scenes{types};
 GeneratedModelLifetimeRuntime attachments{scenes};
 NativeNodeDestructionRuntime nodes;
 NativeSurfaceOwnerContext surface;
 NativeRetainedMemoryOwnerContext& retained;
 NativeRendererTextureNameNotificationContext notification;
 NativeTexture2DOwnerContext texture;
 NativeCubeTextureOwnerContext cube;
 NativeVolumeTextureOwnerContext volume;
 NativeRendererTextureBindingProfiles texture_profiles;
 NativeRendererTextureBindingContext texture_binding;
 NativePhysicalBufferOwnerContext physical;
 NativeLogicalBufferDeviceRestoreProfiles physical_profiles;
 NativeLogicalVertexOwnerContext vertex;
 NativeRendererVertexBindingContext vertex_binding;
 NativeLogicalIndexPhysicalProfiles index_profiles;
 NativeLogicalIndexOwnerContext index;
 NativeRendererIndexBindingContext index_binding;
 NativeHardwareLayoutOwnerContext hardware;
 NativeRendererVertexLayoutBindingContext layout;
 NativeRendererMaterialStateBindingContext material;
 NativeRendererThirdStateBindingContext third;
 NativeFrameTargetOwnerContext frame;
 NativeRendererCacheClearContext cache;
 NativeRendererBindingResetContext reset;
 NativeRendererQueryTerminalContext queries;
 NativeRendererDestructorProfiles destructor_profiles;
 NativeRendererDestructorContext destructor;
 NativeRendererDefaultSurfacesContext defaults;

 DestructionGraph(NativeRendererConstructorContext& c,ActualNativeStringPoolStorage& strings,
  NativeStringRawPoolContext& raw,NativeResourceSupportRawContext& support,
  NativeRenderActualOwners& owners,const SingletonLifetimeCallbacks& validation,
  const NativeRenderResourceAccountingTables& accounting, GameNativeReadOnlyData& data,
  TypeIdCounterLifetime& counter, LightTypeBootstrapStorage shared_types,
  NativeRetainedMemoryOwnerContext& shared_retained)
  :profiles{data}, types(counter,shared_types),
   nodes(scenes,attachments,raw,&ActualSceneRuntime::node_type),
   surface{reinterpret_cast<NativeSurfaceRendererStorage* volatile&>(c.current_renderer_00f8d394),
    surface_pool,strings,support,support.actual_published_0108fedc,surface_tracking,c.live_one_00d7a24c},
   retained(shared_retained),
   notification(const_cast<const void* volatile&>(c.current_renderer_00f8d394),sync,strings,validation,accounting),
   texture{notification,retained,surface,texture_pool,serial,texture_tracking,profiles(0xd5f0a8),profiles(0xd619a0)},
   cube{notification,retained,game_native_graphics_pool_process().cube_texture_pool_0108db70(),serial,profiles(0xd5f0a8)},
   volume{notification,retained,volume_pool,profiles(0xd5f0a8)},
   texture_profiles{profiles(0xd61948),profiles(0xd61870),profiles(0xd618b0)},
   texture_binding{sync,texture,cube,volume,texture_profiles},
   physical{strings,support.actual_published_0108fedc,support,&game_native_graphics_pool_process().physical_index_pool_0108fda8(),&game_native_graphics_pool_process().physical_vertex_pool_0108fde0()},
   physical_profiles{profiles(0xd61e10),profiles(0xd61e34),profiles(0xd61e58),profiles(0xd61e7c)},
   vertex{const_cast<const void* volatile&>(c.current_renderer_00f8d394),sync,physical,physical_profiles,
    owners,&game_native_graphics_pool_process().logical_vertex_pool_0108fe18(),next_vertex,profiles(0xd61cc0),profiles(0xd5f0a8),profiles(0xd61d6c),recreation,0},
   vertex_binding{vertex},index_profiles{profiles(0xd61e10),profiles(0xd61e58)},
   index{const_cast<const void* volatile&>(c.current_renderer_00f8d394),sync,physical,index_profiles,&game_native_graphics_pool_process().logical_index_pool_0108fe50()},
   index_binding{index},
   hardware{c.current_renderer_00f8d394,profiles(0xd5f0a8),game_native_hardware_layout_tree_process().tree_0108d530(),game_native_hardware_layout_tree_process().invalid_parameters(),
    support.actual_published_0108fedc,support,profiles(0xd61d1c),profiles(0xd61cc0),&game_native_graphics_pool_process().vertex_declaration_pool_0108fd38(),game_native_graphics_pool_process().hardware_layout_pool_0108fe9c(),&owners},
   layout{sync,hardware,profiles(0xd62af4)},
   material{&sync,profiles(0xd61a2c),profiles(0xd61a34)},third{material,profiles(0xd61a3c)},
   frame{surface,profiles(0xd619a0)},cache{third,layout,index_binding,vertex_binding,texture_binding,frame,profiles(0xd5e600)},
   reset{vertex_binding,texture_binding,index_binding,layout,profiles(0xd5f0a8)},
   queries{c.current_renderer_00f8d394,sync,profiles(0xd62ad0),profiles(0xd5f0a8)},
   destructor_profiles{profiles(0xd5f1f0),profiles(0xd621ec),profiles(0xd62260),profiles(0xd626f4),
    profiles(0xd62a3c),profiles(0xd5e5a4),profiles(0xd5e5a8)},
   destructor{c.current_renderer_00f8d394,c.singleton_manager_01090aa0,c.string_pool_01090aa8,
    c.small_returns_disabled_01090aa4,c.state_definitions_0108fe90,c.system_constants_0108fe94,
    c.lua_owner_00f8d434,c.lua_bootstrap,reset,cache,queries,c.declaration_cleanup,c.texture_cleanup,
    c.effect_cleanup,nodes,destructor_profiles},
   defaults{surface,sync,*reinterpret_cast<const U(*)[2]>(profiles(0xd619a0))}
   {scenes.object_type_tokens={0,shared_types.node_0108ff90.own_id,shared_types.node_0108ff90.root_id};
   }
 ~DestructionGraph()=default; // canonical process CRT callbacks own pool cleanup
};
