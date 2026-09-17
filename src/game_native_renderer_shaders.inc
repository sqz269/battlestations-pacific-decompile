// Shared compiler process state and the existing application's VFS/raw pool.
// Native73BF80 determines when the cache is created/released; no eager I/O.
struct ShaderGraph {
 GameNativeShaderProcess& process;
 NativeResourceStreamReadContext reads;
 NativeShaderBinaryCacheContext cache;
 std::unique_ptr<NativeShaderBinaryCacheOperation> operation;
 bool released{};
 ShaderGraph(GameNativeVfsRawServices& vfs,NativeStringRawPoolContext& raw,CameraGraph& cameras,
     CanonicalProfiles& profiles)
  :process(game_native_shader_process()),reads{raw,vfs.bindings,process.stream_empty_0109db64()},
   cache{reads,vfs.bindings,vfs.actual_vfs_publication_0109ceec,
       process.modes().load_variants_0108d6f0,process.modes().source_mode_0108d6f1,
       process.cache_0108d6ec(),reinterpret_cast<const char*>(profiles(0xd60d30)),
       profiles(0xd691b0),cameras.decrement,&raw_allocate,&raw_allocate,&raw_free,&raw_free} {}
 void create() {
  check(!operation,"application shader cache creation is one-shot");
  operation=std::make_unique<NativeShaderBinaryCacheOperation>();
  create_native_shader_binary_cache_00b3a600(cache,*operation);
 }
 void release() {
  check(operation && operation->phase==NativeShaderBinaryCacheOperation::Phase::complete && !released,
      "application shader cache requires a completed unretired creation");
  release_native_shader_binary_cache_00b3b140(cache);released=true;
 }
 bool quiescent() const noexcept {return (!operation || released) && !process.cache_0108d6ec();}
};
