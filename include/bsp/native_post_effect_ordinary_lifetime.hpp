#pragma once

namespace bsp {
class NativePostEffectModelLifetimeContext;
class NativeMeshCanonicalDomain;
class NativeMaterialDomain;
class NativeMeshSectionLifetimeDomain;

// Immutable borrowed selection, prepared before the actual constructor.
// This selects Source transport; only real owning operations issue facts.
struct NativePostEffectOrdinaryLifetimeBinding final {
    NativePostEffectModelLifetimeContext& models;
    NativeMeshCanonicalDomain& meshes;
    NativeMaterialDomain* const materials{nullptr}; // cold Source selection only
    NativeMeshSectionLifetimeDomain* const sections{nullptr}; // SAME outer Source scope required
};
}
