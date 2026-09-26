// Stable prerequisites for genuine NativeChildren/Programs/loading contexts.
// Declare after CompilerOwnersGraph and before RenderResourcesGraph. Reverse
// destruction keeps compiler/state/pass and geometry domains alive until this
// domain's existing effect owner records have retired. Failed frames require
// caller retention; the existing owners destructor rejects surviving entries.
struct EffectOwnersGraph {
 NativeMaterialEffectConstructionAccess construction;
 NativeMaterialEffectDestructionAccess lifetime;
 NativeMaterialEffectLoadOwners owners;

 EffectOwnersGraph(CompilerOwnersGraph& compiler,TextureLoadingGraph& textures,
     CanonicalProfiles& profiles,ActualNativeStringPoolStorage& strings,
     void* const volatile& renderer)
  :construction{strings,renderer,game_native_material_process().effect_serial_00f8d3a8()},
   lifetime{strings,compiler.owners},
   owners(textures.geometry,lifetime,profiles(0xd61a00)) {}
 GameNativeMaterialEffectOwners borrowed() noexcept {
  return {construction,lifetime,owners};
 }
};
