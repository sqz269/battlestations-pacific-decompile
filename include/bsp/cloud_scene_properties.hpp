#pragma once

// The cloud entity's scene-property pass (00477710, cloud entity vtable 00CE5E68
// slot +A0h): the branch gate and the particle-cloud (CloudType 0) branch.
// Evidence: docs/CC13_SCENE_BROWSER.md ("Cloud property pass"). Names are
// hypotheses. The two mesh branches (CloudType 1 and 2) were added by cc13-p7.

#include <cstdint>
#include <map>
#include <memory>
#include <string>

namespace bsp {

// One scene property as 00477710 reads it: kind at +4 (0 integer, 3 boolean
// byte, otherwise float), value at +0Ch; a colour property's +0Ch is a nested
// bag holding R, G, B (and A for xBaseCol).
struct CloudSceneProperty {
    std::int32_t kind{};
    std::int32_t integer{};
    float real{};
    std::uint8_t byte{};
    std::shared_ptr<const std::map<std::string, CloudSceneProperty>> nested;

    // CVTSI2SS for kind 0, else MOVSS (00477793..004777B4).
    float as_float() const noexcept { return kind == 0 ? static_cast<float>(integer) : real; }
};
using CloudScenePropertyBag = std::map<std::string, CloudSceneProperty>;

enum class CloudPropertyBranch { None, Particle, ShadedMesh, DiffuseMesh };

// 00477737..00477780 and 00477C3C / 0047889A: after the base 0048DBD0, the
// pass runs only when the class CloudType (+34h of the class at +188h) or the
// particle instance (+1A4h) is non-zero, the property holder (+C0h) is present
// with kind 1 and byte +1A8h is clear; CloudType 0, 1 and 2 pick the branch.
// Every path ends with the entity's vtable+D8h (00478A2A), not modelled.
CloudPropertyBranch cloud_property_branch_00477710(std::int32_t cloud_type, bool has_instance,
                                                    bool has_holder, std::int32_t holder_kind,
                                                    bool suppressed) noexcept;

// The particle instance at entity+1A4h and the cloud manager
// [[00E188A8]+19E8h]+3Ch (00BBDDF0).
struct CloudParticleTarget {
    virtual ~CloudParticleTarget() = default;
    virtual void set_inside_color(const float rgba[4]) = 0;  // 00BBE540
    virtual void set_ambient(const float rgba[4]) = 0;       // 00BBE4B0
    virtual void set_diffuse(const float rgba[4]) = 0;       // 00BBE4E0
    virtual void set_specular(const float rgba[4]) = 0;      // 00BBE510
    virtual void set_density(float) = 0;                     // 00BBE570
    virtual void set_blur_radius(float) = 0;                 // 00BBE590
    virtual void set_scale(float) = 0;                       // 00BBE470
    virtual void set_alpha(float) = 0;                       // 00BBE490
    virtual void set_affected_by_wind(std::uint8_t) = 0;     // 00BBE5C0, MOVZX byte
    virtual void set_visibility_factor(float) = 0;           // 00B6DA70(instance, factor, 0)
    virtual void register_with_manager() = 0;                // manager->00BBD470(instance)
};

// Address: 00477710, CloudType-0 branch 00477786..00477C37. Original ABI of
// the whole routine: __thiscall, ECX = cloud entity, RET. `bag` is the holder's
// bag (+C0h, then +8). InsideColor, Ambient, Diffuse and Specular (R, G, B, with
// alpha 1.0 from 00D7A24C), Density and BlurRadius are read without a null check
// natively; the host throws std::out_of_range when one is missing. Scale, Alpha
// and AffectedByWind are optional (004772C0 test first); AffectedByWind passes the
// byte value of a kind-3 property, otherwise 0. The visibility factor is the byte at
// manager+98h converted to float.
void apply_cloud_particle_properties_00477710(const CloudScenePropertyBag& bag,
                                              std::uint8_t manager_visibility_byte,
                                              CloudParticleTarget& target);

// ---------------------------------------------------------------------------
// The two mesh branches (cc13-p7)
// ---------------------------------------------------------------------------

// The static shader-parameter block 00E186AC..00E186DB that CloudType 1 fills
// and registers by address, so the materials read it later.
struct CloudMeshShaderGlobals {
    float sky_col[3]{};    // 00E186AC, cSkyColor (float3)
    float sun_col[3]{};    // 00E186B8, cSunColor (float3)
    float base_col[4]{};   // 00E186C4, cBaseColor (float4 array, count 1)
    float rim_back[2]{};   // 00E186D4, cRim_Back (float2): xRimPower, xBackPower
};
static_assert(sizeof(CloudMeshShaderGlobals) == 0x30, "00E186AC..00E186DB");

// Entity fields the mesh branches write and register by address.
struct CloudMeshEntityFields {
    float brad_scale[2]{};       // +18Ch radius before scaling, +190h xScale: cBrad_Scale
    float diffuse_barany[4]{};   // +194h..+19Ch Diffuse R,G,B, +1A0h Scale: cDiffuseColor_BaranyScale
};
static_assert(sizeof(CloudMeshEntityFields) == 0x18, "entity +18Ch..+1A3h");

// One element's logical vertex stream, element+3Ch.
struct CloudVertexStream {
    virtual ~CloudVertexStream() = default;
    virtual std::uint32_t vertex_count() = 0;                    // vtable+20h (004782DE)
    virtual void lock(std::uint32_t count) = 0;                  // vtable+10h(count, 0, 0) (004782F2)
    virtual void unlock() = 0;                                   // vtable+14h (0047869B)
    // 00476180: the vertex's four colour bytes, in memory order.
    virtual void read_colour(std::uint32_t vertex, std::uint8_t out[4]) = 0;
    virtual void read_position(std::uint32_t vertex, float out[3]) = 0;  // 004768D0
    // +34h: the packed colour's byte offset in a vertex, negative when the
    // stream keeps float channels at +38h/+3Ch/+40h/+44h instead.
    virtual std::int32_t packed_colour_offset() = 0;
    // [+8h] + [+0Ch] * vertex + [+34h] (0047881D).
    virtual void write_packed_colour(std::uint32_t vertex, std::uint32_t bgra) = 0;
    // The four float channels, stored in this order (00478857..00478890).
    virtual void write_float_colour(std::uint32_t vertex, const float rgba[4]) = 0;
};

// The cloud mesh [[entity+178h]+160h]+0Ch and what the branches call on it.
struct CloudMeshTarget {
    virtual ~CloudMeshTarget() = default;
    virtual void get_local_bounds(float sphere[4]) = 0;         // mesh vtable+44h
    virtual void set_local_bounds(const float sphere[4]) = 0;   // 00B74390
    // 00B74640(mesh, 0) then 00B72B50 / 00B72B40 / 00B732C0 on the geometry.
    virtual void reset_geometry_bounds_cache() = 0;
    virtual std::uint32_t element_count() = 0;
    // The element's material (element+20h), 00B18AC0 / 00B18AE0 / 00B18B00:
    // the material keeps `values`, so they must outlive it.
    virtual void register_float4_array(std::uint32_t element, const char* name,
                                       const float* values, std::int32_t count) = 0;
    virtual void register_float3(std::uint32_t element, const char* name, const float* values) = 0;
    virtual void register_float2(std::uint32_t element, const char* name, const float* values) = 0;
    // 00B189F0(slot, texture) with [00F8D39C]'s 00B0D140 (slot 3) and 00B0D130 (slot 4).
    virtual void set_shadow_textures(std::uint32_t element) = 0;
    virtual CloudVertexStream& vertex_stream(std::uint32_t element) = 0;  // element+3Ch
    virtual void set_visibility_factor(float) = 0;              // 00B6DA70(mesh, factor, 0)
    virtual void register_with_manager() = 0;                   // manager->00BBD470(mesh)
};

// Address: 00477710, CloudType-1 branch 00477C45..00478726. Same routine and
// ABI as above. xBaseCol (R,G,B,A), xSkyCol and xSunCol (R,G,B) default to 1.0
// when absent, as do xScale, xRimPower and xBackPower; a present colour's
// channels are required. The mesh's bounding radius is scaled by xScale, the
// parameters are registered on every element's material, and each element's
// vertex colours are replaced by the averaged position of the vertices that
// share a colour (white vertices excluded), mapped into the unscaled bounding
// sphere. The colour records persist across elements and every element pass
// divides all of them again, as the listing does (0047854C..00478598).
void apply_cloud_shaded_mesh_properties_00477710(const CloudScenePropertyBag& bag,
                                                 std::uint8_t manager_visibility_byte,
                                                 CloudMeshShaderGlobals& globals,
                                                 CloudMeshEntityFields& entity,
                                                 CloudMeshTarget& mesh);

// Address: 00477710, CloudType-2 branch 0047889A..00478A1B. Diffuse (R,G,B) and
// Scale are read without a presence test (required here); the radius is scaled
// by Scale and element 0's material gets cDiffuseColor_BaranyScale.
void apply_cloud_diffuse_mesh_properties_00477710(const CloudScenePropertyBag& bag,
                                                  CloudMeshEntityFields& entity,
                                                  CloudMeshTarget& mesh);

// What the whole pass reads off the entity, and its two entity calls.
struct CloudEntityPassView {
    std::int32_t cloud_type{};              // [[entity+188h]+34h]
    bool has_particle_instance{};           // entity+1A4h != 0
    const CloudScenePropertyBag* holder_bag{};  // [entity+0C0h]+8, null when +0C0h is null
    std::int32_t holder_kind{};             // [entity+0C0h]+4
    bool suppressed{};                      // byte entity+1A8h
    std::uint8_t manager_visibility_byte{}; // [00BBDDF0()+98h]
};
struct CloudEntityPassHost {
    virtual ~CloudEntityPassHost() = default;
    virtual void base_scene_properties_0048dbd0() = 0;  // 00477737
    virtual void finish_vtable_d8() = 0;                // 00478A2A, entity vtable+D8h
};

// 00477710 whole: __thiscall(ECX = cloud entity), RET, body 00477710-00478A41.
// The base pass first; an entity with CloudType 0 and no particle instance
// returns at once (0047774F JZ 00478A2C, skipping vtable+D8h). Otherwise a
// missing holder, a holder kind other than 1 or a set +1A8h byte go straight to
// vtable+D8h; CloudType 0, 1 and 2 run their branch before it, any other type
// runs none (0047889D).
void cloud_entity_apply_scene_properties_00477710(const CloudEntityPassView& entity_view,
                                                  CloudParticleTarget& particle,
                                                  CloudMeshShaderGlobals& globals,
                                                  CloudMeshEntityFields& entity,
                                                  CloudMeshTarget& mesh,
                                                  CloudEntityPassHost& host);

} // namespace bsp
