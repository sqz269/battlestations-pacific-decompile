// Descriptor parsing borrows the application's existing Lua/VFS/string/state
// definitions and canonical state-list pool. No dummy compiler or descriptor.
struct DescriptorGraph {
 NativeShaderSamplerClassBinding samplers;
 // Same hardware-derived conversion input used by the existing application
 // Lua consumers. This is a retained source mode, not original CRT data mapping.
 const bool numeric_mode{IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)!=FALSE};
 const NativeShaderCombinerStackInputs combiner_stack{nullptr,nullptr};
 const NativeShaderFieldStackInputs field_stack{nullptr,nullptr};
 NativeShaderDescriptorReadContext reader;
 DescriptorGraph(NativeStringStorage& strings,GameNativeLuaServices& lua,
     NativeShaderStateDefinitionsStorage* volatile& definitions)
  :samplers(game_native_shader_process().state_list_pool_0108fee4(),strings),
   reader{strings,lua.bootstrap(),lua.binding().files(),definitions,samplers,
       numeric_mode,combiner_stack,field_stack} {}
};
