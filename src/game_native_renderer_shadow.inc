// Retained after the genuine runtime/surface/terminal graph, with one canonical
// process publication/import set. Composition performs no native owner work.
struct ShadowTargetGraph {
 GameNativeRendererScalarProcess& process;
 NativeShadowDepthTargetContext context;
 ShadowTargetGraph(DestructionGraph& g,RenderResourcesGraph& resources,
     GameSingletonHost& host,void* const volatile& renderer)
  :process(game_native_renderer_scalar_process()),
   context({host.manager_publication_01090aa0(),process.shadow_target_publication_00f8bbf0(),
       renderer,resources.runtime_textures,resources.texture_surfaces,resources.lifetime,
       resources.direct_terminals,g.profiles(0xd5f0a8),g.profiles(0xd61948),
       process.enter_iat_00ce2218(),process.leave_iat_00ce2210()}) {}
 bool quiescent() const noexcept {
  for(std::size_t i=0;i<context.operation_count();++i)
   if(context.operation(i)->phase!=NativeShadowDepthTargetOperation::Phase::complete)return false;
  return true;
 }
 void after_native_drain() const {
  check(quiescent(),"incomplete shadow target operation requires process retention");
  check(!process.shadow_target_publication_00f8bbf0(),"shadow target survived native drain");
 }
 // No destructor accesses GameSingletonHost: startup deletes that host before
 // this graph. Completed operation records are metadata, never cleanup owners.
};
