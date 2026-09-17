// Private application frame graph. Every context borrows the existing owner,
// pool, manager, publication and synchronization domains through native drain.
struct FrameGraph {
 NativeRenderCommandQueueStorage* volatile queue{};
 void* volatile service{}; // Actual F8D39C service is not constructed yet.
 volatile std::uint8_t in_end_frame=0,clear_request=1; // PE 108D4CC / E1306C.
 NativeRendererStopPipelineContext stop;
 NativeRenderQueueConstructorContext queue_constructor;
 NativeRenderBatchPreparationContext queue_getter;
 NativeRenderQueueDestructionContext queue_destruction;
 NativeRendererBeginFrameContext begin;
 NativeD3dx9SurfaceLoadImport surface_load;
 NativeRendererSurfaceSavePublishContext surface_save;
 NativeRendererViewportClearContext clear;
 NativePhysicalBufferRewindContext rewind;
 NativeXLiveRenderImport xlive;
 NativeRendererPresentObservation present;
 NativeRendererEndFrameContext end;
 NativeRendererEndFrameContext worker_end;

 FrameGraph(DestructionGraph& g,DeviceGraph& d,NativeRendererConstructorContext& c,
  GameSingletonHost& host,ActualNativeStringPoolStorage& strings,NativeStringRawPoolContext& raw,
  void* const volatile& counter,const XLiveLibrary& library)
  :stop{g.reset,g.cache,g.profiles(0xd6821c)},
   queue_constructor{d.renderer,queue,stop,strings},
   queue_getter{host.manager_publication_01090aa0(),queue_constructor,g.profiles(0xd5e5ac),g.profiles(0xd62064)},
   queue_destruction{queue,strings,nullptr,nullptr},
   begin{d.reset,service,raw},surface_load(d.d3dx),
   surface_save{&strings,&surface_load,g.profiles(0xd5f0a8)},
   clear{g.sync,c.live_one_00d7a24c},
   rewind{c.current_renderer_00f8d394,g.sync,g.profiles(0xd61d6c),g.profiles(0xd61de0)},
   xlive(library),
   end{queue_getter,g.reset,g.cache,surface_save,clear,rewind,xlive,nullptr,
       in_end_frame,clear_request,c.device_lost_0108d4b9,counter,g.profiles(0xd62ad0),nullptr},
   worker_end(end) {
    // Current frontend work does not produce native commands or debug records.
    // Null providers are admitted only by their actual empty-count branches;
    // future nonempty paths require complete persistent execution domains.
    end.present_observation=&present;
 }
 static void worker_finish(void* renderer,void* context) {
  end_native_renderer_frame_default_00b2f4a0(renderer,
      static_cast<NativeRendererEndFrameContext*>(context));
 }
};
