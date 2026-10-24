// Private ON-only ordinary command graph. Included after FrameGraph and the
// renderer's canonical device/camera/lifetime graphs. Native bodies remain in
// native_render_*; this file owns only their concrete retained composition.
struct MissionRenderGlobals {
 struct Bank { U header{}; std::array<U,256*4> values{}; };
 Bank vertex, pixel;
 void* volatile cached_effect{};
 std::array<U,16> parameters{};
 volatile U color_guard{};
 std::array<U,24> colors{};
 static MissionRenderGlobals& process() {
  // Intentionally outlives every app, device generation and native CRT drain.
  // These exact ranges are in the supported PE's loader-zero .data tail; see
  // OCT18B_RENDER_A2.md. Genuine B42350/B55550/B40860/B1DFF0 are the writers.
  static auto* const value=new MissionRenderGlobals;
  return *value;
 }
};
static_assert(offsetof(MissionRenderGlobals::Bank,values)==4);

struct MissionCommandGraph {
 struct Record;
 struct Owners final : NativeRenderActualOwners {
  MissionCommandGraph& graph;
  NativeRenderActualOwners& shared;
  Owners(MissionCommandGraph& g,NativeRenderActualOwners& s):graph(g),shared(s){}
  RenderCommandReference& resolve_actual(void* raw) override;
 } owners;
 struct Batches final : NativeRenderBatchReferences {
  MissionCommandGraph& graph;
  explicit Batches(MissionCommandGraph& g):graph(g){}
  NativeRenderBatchReference& resolve_actual_batch(void* raw) override;
 } batches;
 struct Associations final : NativeRenderCommandAssociations {
  MissionCommandGraph& graph;
  explicit Associations(MissionCommandGraph& g):graph(g){}
  NativeRenderContextReference& bind_initialized_context(NativeRenderContextStorage&) noexcept override;
  NativeRenderBatchReference& bind_acquired_batch(NativeRenderBatchStorage&) noexcept override;
 } associations;
 struct CollectingServices final : NativeInstanceCollectingRenderServices {
  NativeTracelineRenderServices& shared;
  CollectingServices(NativeInstanceCollectionAccess& c,NativeTracelineRenderServices& s)
   :NativeInstanceCollectingRenderServices(c),shared(s){}
  const volatile U* profile(const void* owner) override {return shared.profile(owner);}
  const void* call_virtual48(void* owner,U target) override {return shared.call_virtual48(owner,target);}
  U call_virtual58(void* owner,U target,void* camera,float lod) override {
   return shared.call_virtual58(owner,target,camera,lod);
  }
  void call_virtual20(void* owner,U target,void* context,float lod,float visibility,U flags) override {
   shared.call_virtual20(owner,target,context,lod,visibility,flags);
  }
 };
 struct PassRecord {
  NativeMaterialConstantBuildFrame constants;
  NativeMaterialPassExecutionFrame pass{constants};
 };
 struct BatchFrames {
  std::vector<std::unique_ptr<PassRecord>> records;
  std::vector<NativeMaterialPassExecutionFrame*> pointers;
  NativeMaterialEntryDispatchFrame frame;
  void prepare(U count) {
   records.reserve(count);pointers.reserve(count);
   for(U i=0;i<count;++i) {
    auto record=std::make_unique<PassRecord>();
    pointers.push_back(&record->pass);records.push_back(std::move(record));
   }
   frame={pointers.data(),count,0};
  }
 };
 struct Record {
  MissionCommandGraph& graph;
  void* context_identity{};
  std::optional<NativeRenderContextReference> context;
  bool context_retired{};
  void* batch_identity[2]{};
  std::optional<NativeRenderBatchReference> batch[2];
  bool batch_retired[2]{};
  U acquired_batches{};
  NativeRenderSceneSubmissionOperation submission;
  U scene_label[2]; // B41E870 initializes; B1F4D0 consumes exactly once.
  // Explicit initialized source preimage: copy the SAME current vertex bank,
  // never manufacture the original function's uninitialized stack bytes.
  // Native gather overwrites its established words and preserves the rest.
  std::array<U,0x4d0/4> prefix;
  NativeSystemConstantGatherFrame system{prefix.data(),{}};
  BatchFrames batch_frames[2];
  NativeRenderCommandExecutionFrame frame{&system,{&batch_frames[0].frame,&batch_frames[1].frame}};
  NativeRenderCommandExecutionFrame* frame_pointer{&frame};
  U entry_counts[2]{};
  bool ready{},executed{},failed{};
  explicit Record(MissionCommandGraph& g):graph(g) {
   std::memcpy(prefix.data(),g.globals.vertex.values.data(),sizeof prefix);
  }
  static void retire_context(void* opaque,NativeRenderContextReference&) noexcept {
   static_cast<Record*>(opaque)->context_retired=true;
  }
  static void retire_batch(void* opaque,NativeRenderBatchReference& ref,NativeRenderBatchRetirement result) noexcept {
   auto& r=*static_cast<Record*>(opaque);
   for(U i=0;i<2;++i) if(r.batch[i] && &*r.batch[i]==&ref) {
    r.batch_retired[i]=true;
    if(result!=NativeRenderBatchRetirement::returned_to_pool) r.failed=true;
    return;
   }
   std::terminate();
  }
  bool retired() const noexcept {
   return context_retired && acquired_batches==2 && batch_retired[0] && batch_retired[1];
  }
  static U frame_capacity(NativeRenderBatchStorage& batch) {
   std::uint64_t count=0;
   for(std::int32_t i=0;i<batch.count_10;++i) {
    const void* entry=batch.entries_0c[i];
    const void* model=get<void*>(entry,0x0c);
    const void* collection=get<void*>(model,0x170);
    const U lights=collection ? get<U>(collection,0x20) : 0;
    // B45360 reaches at most one pass per current light, or one unlit/
    // mode2 pass. The diagnostic excludes concurrent light-list mutation.
    count+=lights ? lights : 1;
    check(count<=std::numeric_limits<U>::max(),"mission pass frame count overflow");
   }
   return static_cast<U>(count);
  }
  void prepare() {
   check(submission.completed && submission.command,"mission command did not complete submission");
   for(U i=0;i<2;++i) {
    auto& actual=*submission.command->batches_0c[i];
    check(actual.count_10>=0,"mission batch has a negative entry count");
    entry_counts[i]=static_cast<U>(actual.count_10);
    batch_frames[i].prepare(frame_capacity(actual));
    if(graph.completed==0)for(auto& record:batch_frames[i].records) {
     record->pass.observation_context=&graph;
     record->pass.observe_before_draw=&MissionCommandGraph::observe_first_material_draw;
    }
   }
   check(graph.execution.actual_synchronization_0108d6dc.mode_00!=0 ||
     entry_counts[0]<=50 || entry_counts[1]<=50 ||
     (graph.execution.jobs && graph.execution.preparation_dispatch),
     "mission parallel batches require the canonical native preparation-job binding");
   ready=true;
  }
 };

 GameHostLog& log;
 FrameGraph& frames;
 CanonicalProfiles& profiles;
 GameNativeMissionCommandProviders providers;
 MissionRenderGlobals& globals;
 NativeRenderBatchLifetime& batch_lifetime;
 NativeRenderCommandActualEnvironment command_lifetime;
 NativeRenderCommandRawEnvironment raw_lifetime;
 NativeCameraFrustumContext frustum;
 NativeRendererCameraPrepareContext camera_prepare;
 NativeCameraFrameCommandContext camera_command;
 NativeCameraAxesContext axes;
 NativeLogicalBufferMappingContext mapping;
 NativeInstanceGeometryAcquired acquired_geometry;
 NativeInstanceCollectionAccess collection;
 CollectingServices services;
 const NativeCameraEnvironment& render_camera_environment;
 const NativeMatrixIndexedModelEnvironment& render_matrix_environment;
 std::unique_ptr<NativeRender20Profiles> render20;
 NativeTracelineRenderAccess render;
 NativeInstanceGroupUploadAccess upload;
 NativeRenderSceneSubmissionContext submission;
 NativeRendererIndexedDrawContext indexed;
 NativeMaterialConstantBuildContext constants;
 NativeMaterialPassExecutionContext passes;
 NativeMaterialEntryDispatchContext entries;
 NativeSystemConstantGatherContext system;
 NativeRenderCommandExecutionContext execution;
 NativeRenderQueueExecutionContext queue_execution;
 NativeRenderQueueExecutionFrame queue_frame;
 std::unique_ptr<Record> pending;
 Record* acquiring{};
 bool failed{};
 GameNativeMissionFrameJobs* frame_jobs{};
 bool material_observed{};
 unsigned long generation{};
 std::uint64_t submitted{},completed{};

 MissionCommandGraph(GameHostLog& output,FrameGraph& f,DestructionGraph& g,DeviceGraph& d,
  CameraGraph& cameras,NativeRendererConstructorContext& c,GameVfsHost& files,
  GameSingletonHost& host,NativeStringRawPoolContext& raw,ActualNativeStringPoolStorage& strings,
  void* volatile& service,void* const volatile& cache,GameNativeMissionCommandProviders borrowed,
  const NativeMatrixIndexedModelEnvironment& matrix_models,const NativeGroupEnvironment* gui_groups)
  :owners(*this,given_owners(borrowed)),batches(*this),associations(*this),
   log(output),frames(f),profiles(g.profiles),
   providers(borrowed),globals(MissionRenderGlobals::process()),batch_lifetime(files.native_owners().batches()),
   command_lifetime{batch_lifetime,batches,owners,providers.models,strings,
    reinterpret_cast<const char*>(g.profiles(0xce9a38))},
   raw_lifetime{batches,owners,providers.models,raw},
   frustum{*application_camera_axes_crt().dispatch_bypass_0109dd78,*g.profiles(0xd7a208)},
   camera_prepare{frustum,g.sync,*reinterpret_cast<const volatile double*>(g.profiles(0xce4b48)),
    *reinterpret_cast<const volatile U*>(0x109eea4)},
   camera_command{c.current_renderer_00f8d394,g.profiles(0xd5f0a8),camera_prepare,f.clear},
   axes{*application_camera_axes_crt().dispatch_bypass_0109dd78,*g.profiles(0xd7a24c),*g.profiles(0xce3800)},
   mapping{d.renderer,g.sync,d.lock,g.physical_profiles},
   collection{&frustum.length_access,reinterpret_cast<const volatile float*>(g.profiles(0xd7a24c)),
    reinterpret_cast<const volatile double*>(g.profiles(0xce4d70)),
    reinterpret_cast<const volatile double*>(g.profiles(0xceb690)),
    reinterpret_cast<const volatile float*>(g.profiles(0xce77dc)),&strings,&owners,&providers.geometry,
    &cache,&globals.color_guard,globals.colors.data(),
    reinterpret_cast<const char*>(g.profiles(0xd21d00)),
    reinterpret_cast<const char*>(g.profiles(0xd5e5e4)),&allocate,&acquired_geometry},
   services(collection,borrowed.render_services),
   render_camera_environment(cameras.environment),render_matrix_environment(matrix_models),
   render20(std::make_unique<NativeRender20Profiles>(cameras.environment,matrix_models.base,gui_groups,&matrix_models)),
   render{&frustum.length_access,&frustum,&mapping,nullptr,nullptr,&services,
    reinterpret_cast<const volatile float*>(g.profiles(0xd7a208)),
    reinterpret_cast<const volatile float*>(g.profiles(0xd7a218)),
    reinterpret_cast<const volatile float*>(g.profiles(0xd7a24c)),
    reinterpret_cast<const volatile float*>(g.profiles(0xd7a238)),
    reinterpret_cast<const volatile double*>(g.profiles(0xce4b48)),
    reinterpret_cast<const volatile double*>(g.profiles(0xd7a280)),&cache,nullptr,render20.get()},
   upload{render,g.scenes,*reinterpret_cast<const volatile float*>(g.profiles(0xce3800)),
    *reinterpret_cast<const volatile float*>(g.profiles(0xce3978))},
   submission{command_lifetime,associations,upload,raw},indexed{g.sync,g.profiles(0xd61d6c)},
   constants{globals.vertex.values.data(),globals.pixel.values.data(),c.current_renderer_00f8d394,service,
    &game_native_renderer_scalar_process().shadow_target_publication_00f8bbf0(),&g.texture_binding,
    collection,providers.animator_types_010900fc,*reinterpret_cast<const volatile float*>(g.profiles(0xd7a238)),
    float_cell(c.live_one_00d7a24c),*reinterpret_cast<const volatile float*>(g.profiles(0xce3978)),
    g.profiles(0xd5f0a8),g.profiles(0xd61d6c),g.profiles(0xd5b5d8),g.profiles(0xd62eb0),
    g.profiles(0xd64478),g.profiles(0xd644b4)},
   passes{c.current_renderer_00f8d394,service,globals.cached_effect,native_shader_register_limit_cell_00e13078(),
    globals.vertex.values.data(),globals.pixel.values.data(),*reinterpret_cast<const volatile float*>(g.profiles(0xd7a208)),
    *reinterpret_cast<const volatile float*>(g.profiles(0xd7a218)),float_cell(c.live_one_00d7a24c),g.sync,
    g.vertex_binding,g.index_binding,g.layout,g.texture_binding,indexed,g.material,f.queue_getter,raw,
    game_native_renderer_scalar_process().increment_iat_00ce221c(),constants,g.profiles(0xd5f0a8),
    g.profiles(0xd61d6c),g.profiles(0xd62a80),g.profiles(0xd61be8)},
   entries{c.current_renderer_00f8d394,globals.cached_effect,float_cell(c.live_one_00d7a24c),
    g.profiles(0xd5f0a8),g.profiles(0xd61a00),&passes,&render,nullptr,&g.types,nullptr,
    g.profiles(0xd62f58),g.profiles(0xd62fb0),g.profiles(0xd63008)},
   system{&service,&c.current_renderer_00f8d394,&providers.clock.actual_clock_01090ab0,
    &host.particle_clock_publication_00f8d420(),&host.manager_publication_01090aa0(),
    &providers.foliage_manager_00f8c274,&providers.foliage_00f8c210,globals.parameters.data(),
    &native_shader_register_limit_cell_00e13078(),g.profiles(0xcfad80),
    reinterpret_cast<const volatile float*>(g.profiles(0xce3978)),&axes,&g.sync,g.profiles(0xd68d50),
    &game_native_renderer_scalar_process().shadow_target_publication_00f8bbf0(),
    g.profiles(0xd5b5d8),g.profiles(0xd61948)},
   execution{c.current_renderer_00f8d394,service,g.sync,raw,g.profiles(0xd5f0a8),&d.targets,
    &camera_command,&system,nullptr,&f.queue_getter,nullptr,&entries},
   queue_execution{execution,owners,g.profiles(0xd5e5e0),&raw_lifetime} {
    check(&providers.geometry.models.nodes==&g.nodes &&
      &providers.geometry.models.retained_owners==&owners.shared,
      "mission model geometry must borrow the canonical renderer node/owner domain");
    frames.bind_ordinary_execution(&queue_execution,&queue_frame);
    log.note("native_mission_command graph=retained profile=D5E5E0 entry=B1D950 source_prefix=live_vertex_bank");
 }
 static NativeRenderActualOwners& given_owners(GameNativeMissionCommandProviders& p) {
  return p.geometry.models.retained_owners;
 }
 static const volatile float& float_cell(const volatile U& raw) noexcept {
  return reinterpret_cast<const volatile float&>(raw);
 }
 static void* __cdecl allocate(std::size_t n) {
  check(n<=std::numeric_limits<U>::max(),"mission native allocation size overflow");
  return singleton_lifetime_allocate({SingletonAllocationKind::object,static_cast<U>(n),n});
 }
 ~MissionCommandGraph() {
  if(failed || pending) std::terminate();
  frames.bind_ordinary_execution(nullptr,nullptr);
 }
 NativeTracelineRenderAccess& render_access() noexcept {return render;}
 bool requires_process_retention() const noexcept {return failed || pending!=nullptr;}
 static void observe_first_material_draw(void* context,void* pass,void* entry,void* renderer) {
  auto& self=*static_cast<MissionCommandGraph*>(context);
  if(self.material_observed)return;
  self.material_observed=true;
  auto& output=self.log;
  void* const section=get<void*>(entry,4);
  void* const material=get<void*>(section,0x20);
  const auto count=static_cast<std::int16_t>(get<std::uint16_t>(material,0x34));
  const U pair_count=get<U>(pass,0x28);
  const U enabled=get<U>(get<void*>(pass,0x74),0x84);
  auto* const pairs=get<unsigned char*>(pass,0x24);
  output.notef("native_mission_command first_draw pass=%p material=%p texture_count=%d pair_count=%u pixel_enabled=%08x",
      pass,material,static_cast<int>(count),pair_count,enabled);
  for(U i=0;i<5;++i) {
   void* const texture=static_cast<int>(i)<count?get<void*>(material,0x10+i*4):nullptr;
   if(!texture){output.notef("native_mission_command material_texture slot=%u owner=null",i);continue;}
   const U profile=get<U>(texture);
   if(profile!=0xd61948) {
    output.notef("native_mission_command material_texture slot=%u owner=%p profile=%08x unsupported_readback=1",i,texture,profile);
    continue;
   }
   const U length=get<U>(texture,8);
   const char* const name=get<const char*>(texture,0xc);
   const int shown=static_cast<int>(length<120?length:120);
   output.notef("native_mission_command material_texture slot=%u owner=%p profile=%08x COM=%p name_length=%u name=%.*s",
       i,texture,profile,get<void*>(texture,0x10),length,name?shown:0,name?name:"");
  }
  for(U i=0;i<pair_count && i<16;++i)
   output.notef("native_mission_command pass_pair ordinal=%u selector=%d vertex_sampler=%u pixel_enabled=%u",
       i,get<std::int32_t>(pairs,i*8),static_cast<U>(get<std::uint8_t>(pairs,i*8+4)),
       static_cast<U>((enabled>>(i&31))&1));
  auto* const device=get<IDirect3DDevice9*>(renderer,0x1a10);
  if(!device){output.note("native_mission_command first_draw device=null");return;}
  for(DWORD slot=0;slot<6;++slot) {
   IDirect3DBaseTexture9* texture=nullptr;
   const HRESULT result=device->GetTexture(slot,&texture);
   // GetTexture adds one COM credit even when the returned object is already
   // referenced by the renderer. Balance it on every diagnostic exit/unwind.
   struct TextureCredit {
    IDirect3DBaseTexture9* value;
    ~TextureCredit(){if(value)value->Release();}
   } credit{texture};
   DWORD srgb=0;
   const HRESULT sampler_result=device->GetSamplerState(slot,D3DSAMP_SRGBTEXTURE,&srgb);
   output.notef("native_mission_command device_sampler slot=%lu srgb_result=%08lx srgb_texture=%lu",
       slot,static_cast<unsigned long>(sampler_result),srgb);
   if(FAILED(result) || !texture) {
    output.notef("native_mission_command device_texture slot=%lu HRESULT=%08lx texture=%p",
        slot,static_cast<unsigned long>(result),texture);
    continue;
   }
   const auto type=texture->GetType();
   if(type==D3DRTYPE_TEXTURE) {
    D3DSURFACE_DESC desc;
    const HRESULT described=static_cast<IDirect3DTexture9*>(texture)->GetLevelDesc(0,&desc);
    if(SUCCEEDED(described))
     output.notef("native_mission_command device_texture slot=%lu HRESULT=%08lx texture=%p type=%u desc=%08lx width=%u height=%u format=%08x",
         slot,static_cast<unsigned long>(result),texture,static_cast<U>(type),
         static_cast<unsigned long>(described),desc.Width,desc.Height,static_cast<U>(desc.Format));
    else output.notef("native_mission_command device_texture slot=%lu HRESULT=%08lx texture=%p type=%u desc=%08lx",
         slot,static_cast<unsigned long>(result),texture,static_cast<U>(type),static_cast<unsigned long>(described));
   } else output.notef("native_mission_command device_texture slot=%lu HRESULT=%08lx texture=%p type=%u",
       slot,static_cast<unsigned long>(result),texture,static_cast<U>(type));
  }
 }
 void observe_first_device_constants() {
  if(completed!=0)return;
  // Read-only stock D3D9 observation AFTER native uploads/draws, BEFORE overlay.
  // Gather's supplied prefix is an input preimage, not its resulting stack;
  // the actual device answers whether system constants reached this draw.
  auto* const device=get<IDirect3DDevice9*>(execution.actual_renderer_00f8d394,0x1a10);
  if(!device){log.note("native_mission_command constants_readback device=null");return;}
  std::array<float,53*4> vertex,pixel;
  const HRESULT vs=device->GetVertexShaderConstantF(0,vertex.data(),53);
  const HRESULT ps=device->GetPixelShaderConstantF(0,pixel.data(),53);
  DWORD srgb=0;
  const HRESULT srgb_result=device->GetRenderState(D3DRS_SRGBWRITEENABLE,&srgb);
  log.notef("native_mission_command constants_readback VS=%08lx PS=%08lx srgb_result=%08lx srgb_write=%lu",
      static_cast<unsigned long>(vs),static_cast<unsigned long>(ps),
      static_cast<unsigned long>(srgb_result),static_cast<unsigned long>(srgb));
  const auto record=[&](const char* stage,const std::array<float,53*4>& values) {
   log.notef("native_mission_command device_%s fog=%g,%g,%g height=%g,%g,%g ambient=%g,%g,%g diffuse=%g,%g,%g cube0=%g,%g,%g sun=%g,%g,%g",
       stage,values[35*4],values[35*4+1],values[35*4+2],
       values[36*4],values[36*4+1],values[36*4+2],
       values[43*4],values[43*4+1],values[43*4+2],
       values[44*4],values[44*4+1],values[44*4+2],
       values[46*4],values[46*4+1],values[46*4+2],
       values[52*4],values[52*4+1],values[52*4+2]);
  };
  if(SUCCEEDED(vs))record("VS",vertex);
  if(SUCCEEDED(ps))record("PS",pixel);
 }
 void observe_failed_frame() {
  if(frame_jobs)frame_jobs->observe_interrupted();
  failed=true;
  if(!pending) {log.note("native_mission_command frame_failed retained=1 pending=0");return;}
  pending->failed=true;
  U indexed_calls=0,primitive_calls=0,returned=0,last_stage=0,last_constant=0;
  for(auto& batch:pending->batch_frames) for(const auto& record:batch.records) {
   if(record->pass.draw_terminal==0xb24010) ++indexed_calls;
   if(record->pass.draw_terminal==0xb21b40) ++primitive_calls;
   if(record->pass.draw_returned) ++returned;
   if(record->pass.stage) last_stage=record->pass.stage;
   if(record->constants.reached_callsite) last_constant=record->constants.reached_callsite;
  }
  log.notef("native_mission_command frame_failed retained=1 command_invocations=%u entries=%u,%u B24010=%u B21B40=%u returned=%u last_pass=%08x last_constant=%08x sampler_phase=%u",
   queue_frame.used,pending->entry_counts[0],pending->entry_counts[1],indexed_calls,primitive_calls,returned,
   last_stage,last_constant,static_cast<U>(pending->system.sampler.phase));
 }
 void before_device_generation(unsigned long current) {
  check(!failed && !pending,"mission command must drain before a device generation change");
  if(generation!=current) {
   generation=current;
   log.notef("native_mission_command device_generation=%lu queue=empty",generation);
  }
 }
 NativeRenderCommandQueueStorage& begin_submission() {
  check(!failed && !pending,"mission permits one retained ordinary command at a time");
  check(execution.actual_service_00f8d39c && providers.clock.actual_clock_01090ab0 &&
    providers.foliage_manager_00f8c274 && providers.foliage_00f8c210,
    "mission command requires live service/clock/foliage providers");
  check(native_shader_register_limit_00b343f0()<=0x4d,"mission system-prefix register count exceeds raw extent");
  auto* const queue=get_native_render_command_queue_004c11f0(frames.queue_getter);
  check(queue->commands_14.count_04==0 && queue->control_20==0 && !queue->context_30,
    "mission command requires the empty ordinary control0 queue");
  pending=std::make_unique<Record>(*this);
  queue_frame={&pending->frame_pointer,1,0};
  acquiring=pending.get();
  return *queue;
 }
 void finish_submission(NativeRenderCommandQueueStorage& queue,const char* native_entry) {
  acquiring=nullptr;
  pending->prepare();
  ++submitted;
  log.notef("native_mission_command submit=%s queue_count=%d entries=%u,%u indexed_groups=%d ordered_groups=%d",
   native_entry,queue.commands_14.count_04,pending->entry_counts[0],pending->entry_counts[1],
   pending->submission.command->indexed_groups_2c.count_04,pending->submission.command->ordered_groups_38.count_04);
 }
 void fail_submission() {
  acquiring=nullptr;failed=true;pending->failed=true;
  log.note("native_mission_command submission_failed retained=1");
 }
 void bind_preparation_jobs(GameNativeMissionFrameJobs& jobs) {
  check(!failed && !pending && submitted==0 && !frame_jobs && !execution.jobs && !execution.preparation_dispatch,
      "mission preparation jobs must bind once before native command submission");
  execution.jobs=&jobs.publication();
  execution.preparation_dispatch=&jobs.preparation_dispatch();
  frame_jobs=&jobs;
 }
 void bind_gui_groups(const NativeGroupEnvironment* groups) {
  check(!failed && !pending && submitted==0 && completed==0 && frames.queue &&
      frames.queue->commands_14.count_04==0 && frames.queue->control_20==0 && queue_frame.used==0,
      "GUI render profiles must bind before the first command submission");
  auto next=std::make_unique<NativeRender20Profiles>(render_camera_environment,
      render_matrix_environment.base,groups,&render_matrix_environment);
  // No native traversal or command can hold the old configuration. Publish
  // only after complete construction; a failed allocation preserves it.
  render.render20_profiles=next.get();
  render20=std::move(next);
 }
 void submit_drawable(void* scene,void* camera,void* targets,void* drawable) {
  auto& queue=begin_submission();
  try {
   submit_native_drawable_00b1f5e0(queue,scene,camera,targets,drawable,submission,pending->submission);
   finish_submission(queue,"B1F5E0");
  } catch(...) {
   fail_submission();throw;
  }
 }
 void submit_scene(void* scene,void* camera,void* targets) {
  check(scene!=nullptr,"mission scene submission requires an actual native scene");
  auto& queue=begin_submission();
  try {
   construct_native_string_header_0041e870(pending->scene_label,submission.strings,"Mission ship");
   // B1F4D0 owns this eight-byte argument on both normal and exceptional
   // paths. The consumed header stays unchanged and must not be destroyed.
   submit_native_scene_00b1f4d0(queue,scene,camera,targets,pending->scene_label,
     submission,pending->submission);
   finish_submission(queue,"B1F4D0");
  } catch(...) {
   fail_submission();throw;
  }
 }
 void submit_scene_with_label(void* scene,void* camera,void* targets,void* actual8b_label,
     bool& label_entered_native) {
  check(scene!=nullptr,"mission scene submission requires an actual native scene");
  check(actual8b_label!=nullptr,"mission scene submission requires its original eight-byte label header");
  auto& queue=begin_submission();
  try {
   // Borrow the ORIGINAL by-value argument header, using the same string pool
   // as submission.strings. B1F4D0 consumes its buffer on success or failure;
   // do not copy, relabel, zero or destroy that argument here or in the caller
   // after entry. A pre-entry check/acquisition failure has not consumed it.
   label_entered_native=true;
   submit_native_scene_00b1f4d0(queue,scene,camera,targets,actual8b_label,
     submission,pending->submission);
   finish_submission(queue,"B1F4D0");
  } catch(...) {
   fail_submission();throw;
  }
 }
 void execute_before_overlay() {
  check(pending && pending->ready && !pending->executed && !failed,
    "mission ordinary command execution is one-shot before overlay");
  check(frames.queue && frames.queue->commands_14.count_04==1 &&
    frames.queue->control_20==0 && !frames.queue->context_30 && queue_frame.used==0,
    "mission command requires its retained single-entry ordinary queue");
  try {
   // Original mission render executes its commands before finish/overlay.
   // Keep the existing BeginScene active; no Begin/End/Present is introduced.
   execute_native_render_queue_00b1ebe0(*frames.queue,&queue_execution,&queue_frame);
   pending->executed=true;
   check(frames.queue->commands_14.count_04==0 && !frames.queue->context_30 && pending->retired(),
     "mission queue execution did not retire native command owners");
   U indexed_calls=0,primitive_calls=0,returned=0;
   for(auto& batch:pending->batch_frames) for(const auto& record:batch.records) {
    if(record->pass.draw_terminal==0xb24010) ++indexed_calls;
    if(record->pass.draw_terminal==0xb21b40) ++primitive_calls;
    if(record->pass.draw_returned) ++returned;
   }
   log.notef("native_mission_command execute_before_overlay=B1EBE0/B1D950 command_invocations=%u entries=%u,%u B24010=%u B21B40=%u returned=%u queue_count=0 owners=retired",
    queue_frame.used,pending->entry_counts[0],pending->entry_counts[1],indexed_calls,primitive_calls,returned);
   observe_first_device_constants();
  } catch(...) {
   observe_failed_frame();throw;
  }
 }
 void complete_frame() {
  if(!pending) return;
  try {
   check(pending->ready && pending->executed && !failed && !pending->failed,
    "mission command did not execute before overlay");
   check(frames.queue && frames.queue->commands_14.count_04==0 && !frames.queue->context_30 && pending->retired(),
    "mission command did not complete native queue/context/batch retirement");
   U indexed_calls=0,primitive_calls=0,returned=0;
   for(auto& batch:pending->batch_frames) for(const auto& record:batch.records) {
    if(record->pass.draw_terminal==0xb24010) ++indexed_calls;
    if(record->pass.draw_terminal==0xb21b40) ++primitive_calls;
    if(record->pass.draw_returned) ++returned;
   }
   ++completed;
   log.notef("native_mission_command execute=B1EBE0/B1D950 frame=%llu entries=%u,%u B24010=%u B21B40=%u returned=%u queue_count=0 owners=retired",
    static_cast<unsigned long long>(completed),pending->entry_counts[0],pending->entry_counts[1],indexed_calls,primitive_calls,returned);
   queue_frame={};pending.reset();
  } catch(...) {
   failed=true;pending->failed=true;log.note("native_mission_command execution_failed retained=1");throw;
  }
 }
 // Complete only the successful command companion, not the device frame.
 // The same bound queue_frame object is reset for a later sequential command
 // inside the current BeginScene/Present bracket. Never create another graph.
 void retire_executed_command_before_overlay() {
  check(pending!=nullptr,"mission overlay handoff requires an executed pending command");
  complete_frame();
 }
};

RenderCommandReference& MissionCommandGraph::Owners::resolve_actual(void* raw) {
 if(graph.pending && graph.pending->context_identity==raw && !graph.pending->context_retired)
  return *graph.pending->context;
 return shared.resolve_actual(raw);
}
NativeRenderBatchReference& MissionCommandGraph::Batches::resolve_actual_batch(void* raw) {
 if(graph.pending) for(U i=0;i<graph.pending->acquired_batches;++i)
  if(graph.pending->batch_identity[i]==raw && !graph.pending->batch_retired[i]) return *graph.pending->batch[i];
 throw std::logic_error("mission batch has no canonical retained companion");
}
NativeRenderContextReference& MissionCommandGraph::Associations::bind_initialized_context(NativeRenderContextStorage& raw) noexcept {
 auto* const record=graph.acquiring;
 if(!record || record->context) std::terminate();
 record->context_identity=&raw;
 return record->context.emplace(raw,graph.owners,graph.profiles(0xd5e5c4),
   NativeRenderContextCompanionDisposal{record,&Record::retire_context});
}
NativeRenderBatchReference& MissionCommandGraph::Associations::bind_acquired_batch(NativeRenderBatchStorage& raw) noexcept {
 auto* const record=graph.acquiring;
 if(!record || record->acquired_batches>=2) std::terminate();
 const U index=record->acquired_batches++;
 record->batch_identity[index]=&raw;
 return record->batch[index].emplace(raw,graph.batch_lifetime,graph.profiles(0xd5e5ac),
   NativeRenderBatchCompanionDisposal{record,&Record::retire_batch});
}
