#pragma once
#include "bsp/mesh_resource.hpp"
#include "bsp/structured_hierarchy.hpp"
class AssetStreamProbe;
// Host diagnostic inputs retained after the installed reader/registry checks.
struct InstalledModelProbe {
    bsp::MeshResourcePayload mesh;
    bsp::HierarchyItem hierarchy;
};
bool probe_model_metadata(AssetStreamProbe&, InstalledModelProbe* output = nullptr);
