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
  // Explicit initialized source preimage: copy the SAME current vertex bank,
  // never manufacture the original function's uninitialized stack bytes.
  // Native gather overwrites its established words and preserves the rest.
  std::array<U,0x4d0/4> prefix;
  NativeSystemConstantGatherFrame system{prefix.data(),{}};
  BatchFrames batch_frames[2];
  NativeRenderCommandExecutionFrame frame{&system,{&batch_frames[0].frame,&batch_frames[1].frame}};
  NativeRenderCommandExecutionFrame* frame_pointer{&frame};
  U entry_counts[2]{};
  bool ready{},failed{};
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
   }
   check(graph.execution.actual_synchronization_0108d6dc.mode_00!=0 ||
     entry_counts[0]<=50 || entry_counts[1]<=50,
     "mission single-model batches require the established serial preparation branch");
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
 NativeRender20Profiles render20;
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
 unsigned long generation{};
 std::uint64_t submitted{},completed{};

 MissionCommandGraph(GameHostLog& output,FrameGraph& f,DestructionGraph& g,DeviceGraph& d,
  CameraGraph& cameras,NativeRendererConstructorContext& c,GameVfsHost& files,
  GameSingletonHost& host,NativeStringRawPoolContext& raw,ActualNativeStringPoolStorage& strings,
  void* volatile& service,void* const volatile& cache,GameNativeMissionCommandProviders borrowed)
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
   render20{cameras.environment,providers.geometry.models},
   render{&frustum.length_access,&frustum,&mapping,nullptr,nullptr,&services,
    reinterpret_cast<const volatile float*>(g.profiles(0xd7a208)),
    reinterpret_cast<const volatile float*>(g.profiles(0xd7a218)),
    reinterpret_cast<const volatile float*>(g.profiles(0xd7a24c)),
    reinterpret_cast<const volatile float*>(g.profiles(0xd7a238)),
    reinterpret_cast<const volatile double*>(g.profiles(0xce4b48)),
    reinterpret_cast<const volatile double*>(g.profiles(0xd7a280)),&cache,nullptr,&render20},
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
 void observe_failed_frame() {
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
 void submit_drawable(void* scene,void* camera,void* targets,void* drawable) {
  check(!failed && !pending,"mission permits one retained ordinary command per frame");
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
  try {
   submit_native_drawable_00b1f5e0(*queue,scene,camera,targets,drawable,submission,pending->submission);
   acquiring=nullptr;
   pending->prepare();
   ++submitted;
   log.notef("native_mission_command submit=B1F5E0 queue_count=%d entries=%u,%u indexed_groups=%d ordered_groups=%d",
    queue->commands_14.count_04,pending->entry_counts[0],pending->entry_counts[1],
    pending->submission.command->indexed_groups_2c.count_04,pending->submission.command->ordered_groups_38.count_04);
  } catch(...) {
   acquiring=nullptr;failed=true;pending->failed=true;
   log.note("native_mission_command submission_failed retained=1");throw;
  }
 }
 void complete_frame() {
  if(!pending) return;
  try {
   check(pending->ready && !failed && !pending->failed,"mission command frame is not executable");
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
