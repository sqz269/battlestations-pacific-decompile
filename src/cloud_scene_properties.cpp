#include "bsp/cloud_scene_properties.hpp"

#include <stdexcept>
#include <vector>

namespace bsp {

namespace {

const CloudSceneProperty& required(const CloudScenePropertyBag& bag, const char* key) {
    const auto it = bag.find(key);
    if (it == bag.end()) throw std::out_of_range(std::string("cloud property missing: ") + key);
    return it->second;
}

const CloudSceneProperty* optional(const CloudScenePropertyBag& bag, const char* key) {
    const auto it = bag.find(key);
    return it == bag.end() ? nullptr : &it->second;
}

// 004773A0(name), then R/G/B (00CE60EC/E8/E4) in its nested bag; alpha is 1.0.
void read_color(const CloudScenePropertyBag& bag, const char* key, float out[4]) {
    const auto& property = required(bag, key);
    if (!property.nested) throw std::out_of_range(std::string("cloud colour has no channels: ") + key);
    out[0] = required(*property.nested, "R").as_float();
    out[1] = required(*property.nested, "G").as_float();
    out[2] = required(*property.nested, "B").as_float();
    out[3] = 1.0f;  // 00D7A24C
}

} // namespace

CloudPropertyBranch cloud_property_branch_00477710(std::int32_t cloud_type, bool has_instance,
                                                    bool has_holder, std::int32_t holder_kind,
                                                    bool suppressed) noexcept {
    if (cloud_type == 0 && !has_instance) return CloudPropertyBranch::None;
    if (!has_holder || holder_kind != 1 || suppressed) return CloudPropertyBranch::None;
    switch (cloud_type) {
    case 0: return CloudPropertyBranch::Particle;
    case 1: return CloudPropertyBranch::ShadedMesh;
    case 2: return CloudPropertyBranch::DiffuseMesh;
    default: return CloudPropertyBranch::None;
    }
}

void apply_cloud_particle_properties_00477710(const CloudScenePropertyBag& bag,
                                              std::uint8_t manager_visibility_byte,
                                              CloudParticleTarget& target) {
    float color[4];
    read_color(bag, "InsideColor", color);   // 00477789..00477836
    target.set_inside_color(color);
    read_color(bag, "Ambient", color);       // 00477844..004778F1
    target.set_ambient(color);
    read_color(bag, "Diffuse", color);       // 004778FF..004779AC
    target.set_diffuse(color);
    read_color(bag, "Specular", color);      // 004779BA..00477A67
    target.set_specular(color);
    target.set_density(required(bag, "Density").as_float());         // 00477A75..00477AA5
    target.set_blur_radius(required(bag, "BlurRadius").as_float());  // 00477AB3..00477AE3
    if (const auto* p = optional(bag, "Scale")) target.set_scale(p->as_float());  // 00477AF1..00477B38
    if (const auto* p = optional(bag, "Alpha")) target.set_alpha(p->as_float());  // 00477B46..00477B8D
    if (const auto* p = optional(bag, "AffectedByWind"))                         // 00477B9B..00477BE4
        target.set_affected_by_wind(p->kind == 3 ? p->byte : std::uint8_t{0});
    target.set_visibility_factor(static_cast<float>(manager_visibility_byte));  // 00477BFA..00477C13
    target.register_with_manager();                                            // 00477C32
}

namespace {

// 00477C45..00477D22 and its two copies: an optional colour property whose
// channels default to 1.0 (00D7A24C) when the property is absent.
void read_optional_color(const CloudScenePropertyBag& bag, const char* key, float* out, int channels) {
    static const char* const kChannels[4] = {"R", "G", "B", "A"};  // 00CE60EC/E8/E4/8C
    const CloudSceneProperty* property = optional(bag, key);         // 004772C0
    if (property == nullptr) {
        for (int i = 0; i < channels; ++i) out[i] = 1.0f;
        return;
    }
    if (!property->nested) throw std::out_of_range(std::string("cloud colour has no channels: ") + key);
    for (int i = 0; i < channels; ++i) out[i] = required(*property->nested, kChannels[i]).as_float();  // 008F2260
}

float optional_float(const CloudScenePropertyBag& bag, const char* key) {
    const CloudSceneProperty* property = optional(bag, key);  // 004772C0, then 008F2260
    return property ? property->as_float() : 1.0f;            // 00D7A24C
}

// FISTP with the rounding control forced to chop (OR 0C00h): an out-of-range
// or NaN operand stores the integer indefinite 80000000h.
std::int32_t fistp_chop(float value) {
    const double v = static_cast<double>(value);
    if (!(v > -2147483649.0 && v < 2147483648.0)) return static_cast<std::int32_t>(0x80000000u);
    return static_cast<std::int32_t>(v);
}

// One colour group of the vertex pass: the colour as four floats, the summed
// (then averaged) positions and the vertex count. 32 bytes natively.
struct CloudColourRecord {
    float colour[4];
    float position[3];
    std::uint32_t count;
};

// 00478310..0047834B and 004785B0..00478613: BGRA bytes to floats in R, G, B, A
// order, each FILD / FDIV 255.0 (00CE4B48) rounded to float.
void colour_floats(const std::uint8_t bytes[4], float out[4]) {
    out[0] = static_cast<float>(bytes[2]) / 255.0f;
    out[1] = static_cast<float>(bytes[1]) / 255.0f;
    out[2] = static_cast<float>(bytes[0]) / 255.0f;
    out[3] = static_cast<float>(bytes[3]) / 255.0f;
}

bool same_colour(const float a[4], const float b[4]) {
    // FUCOMIP / UCOMISS with LAHF / TEST AH,44h / JP: equal only when ordered and equal.
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

} // namespace

void apply_cloud_shaded_mesh_properties_00477710(const CloudScenePropertyBag& bag,
                                                 std::uint8_t manager_visibility_byte,
                                                 CloudMeshShaderGlobals& globals,
                                                 CloudMeshEntityFields& entity,
                                                 CloudMeshTarget& mesh) {
    // Every float operation below is a single x87 operation at the main
    // thread's 24-bit precision on float operands, which float arithmetic
    // reproduces (one correctly rounded result per operation).
    read_optional_color(bag, "xBaseCol", globals.base_col, 4);  // 00477C45..00477D22
    read_optional_color(bag, "xSkyCol", globals.sky_col, 3);    // 00477D2A..00477DDF
    read_optional_color(bag, "xSunCol", globals.sun_col, 3);    // 00477DE7..00477E9C
    const float scale = optional_float(bag, "xScale");          // 00477EA4..00477EEA
    const float rim_power = optional_float(bag, "xRimPower");   // 00477EEA..00477F36
    const float back_power = optional_float(bag, "xBackPower"); // 00477F36..00477F82

    float sphere[4];
    mesh.get_local_bounds(sphere);                // 00477FA4, vtable+44h
    const float radius = sphere[3];               // 00477FB1, kept at [ESP+68h]
    sphere[3] = sphere[3] * scale;                // 00477FA6 FMUL, 00477FC7 FSTP
    mesh.set_local_bounds(sphere);                // 00477FE0
    mesh.reset_geometry_bounds_cache();           // 00477FF8 / 00477FFF
    globals.rim_back[0] = rim_power;              // 0047800A
    globals.rim_back[1] = back_power;             // 00478018
    entity.brad_scale[0] = radius;                // 00478026
    entity.brad_scale[1] = scale;                 // 00478034

    const std::uint32_t elements = mesh.element_count();  // 0047804F / 00478056
    for (std::uint32_t e = 0; e < elements; ++e) {        // 00478070..00478279
        mesh.register_float4_array(e, "cBaseColor", globals.base_col, 1);  // 004780BA
        mesh.register_float3(e, "cSkyColor", globals.sky_col);             // 00478110
        mesh.register_float3(e, "cSunColor", globals.sun_col);             // 00478166
        mesh.register_float2(e, "cRim_Back", globals.rim_back);            // 004781BC
        mesh.register_float2(e, "cBrad_Scale", entity.brad_scale);         // 00478217
        mesh.set_shadow_textures(e);                                       // 00478248..0047826D
    }

    // 0047827F: the colour records, an array at [ESP+3Ch] that lives across all
    // elements and is cleared through 00477020 at the end (00478721).
    std::vector<CloudColourRecord> records;
    for (std::uint32_t e = 0; e < elements; ++e) {        // 004782B0..004786AC
        CloudVertexStream& stream = mesh.vertex_stream(e);
        const std::uint32_t count = stream.vertex_count();  // 004782DE
        stream.lock(count);                                 // 004782F2

        // Pass one, 00478310..00478538: group the non-white vertices by colour.
        for (std::uint32_t k = 0; k < count; ++k) {
            std::uint8_t bytes[4];
            stream.read_colour(k, bytes);                   // 0047831F
            float colour[4];
            colour_floats(bytes, colour);
            float position[3];
            stream.read_position(k, position);              // 00478382
            const float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            if (same_colour(colour, white)) continue;       // 004783B1..004783F7 JNP 00478527
            CloudColourRecord* match = nullptr;
            for (CloudColourRecord& record : records) {     // 00478416..00478462
                if (same_colour(colour, record.colour)) { match = &record; break; }
            }
            if (match != nullptr) {                         // 004784FA..00478521
                match->position[0] = position[0] + match->position[0];
                match->position[1] = position[1] + match->position[1];
                match->position[2] = match->position[2] + position[2];
                match->count += 1;
            } else {
                // 0047846A..00478487 grows the capacity to max(2 * capacity, 1)
                // through 00476550 first; the growth has no other effect here.
                records.push_back(CloudColourRecord{{colour[0], colour[1], colour[2], colour[3]},
                                                    {position[0], position[1], position[2]}, 1u});
            }
        }

        // 0047853E..00478598: every record so far, including the ones earlier
        // elements already divided, is divided by its count again.
        for (CloudColourRecord& record : records) {
            // FILD count, + 2^32 (00CE3978) when negative, FSTP float.
            const float n = static_cast<float>(static_cast<double>(record.count));
            record.position[0] = record.position[0] / n;
            record.position[1] = record.position[1] / n;
            record.position[2] = record.position[2] / n;
        }

        // Pass two, 004785B0..0047868E: recolour every vertex whose colour has a record.
        for (std::uint32_t k = 0; k < count; ++k) {
            std::uint8_t bytes[4];
            stream.read_colour(k, bytes);                   // 004785BB
            float colour[4];
            colour_floats(bytes, colour);
            const CloudColourRecord* match = nullptr;
            for (const CloudColourRecord& record : records) {  // 00478629..00478677
                if (same_colour(colour, record.colour)) { match = &record; break; }
            }
            if (match == nullptr) continue;
            // 00478738..0047876F: the centroid mapped from [-r, r] to [0, 1].
            const float diameter = radius + radius;          // 0047875B
            const float x = (match->position[0] + radius) / diameter;
            const float y = (match->position[1] + radius) / diameter;
            const float z = (match->position[2] + radius) / diameter;
            // 00478797..00478806: times 255.0 and chopped, the low byte kept.
            const std::uint8_t red = static_cast<std::uint8_t>(fistp_chop(x * 255.0f));
            const std::uint8_t green = static_cast<std::uint8_t>(fistp_chop(y * 255.0f));
            const std::uint8_t blue = static_cast<std::uint8_t>(fistp_chop(z * 255.0f));
            if (stream.packed_colour_offset() >= 0) {        // 004787F8 / 0047880E JL
                // Bytes [ESP+38h..3Bh] = blue, green, red, 1 (0047872D).
                const std::uint32_t bgra = static_cast<std::uint32_t>(blue) |
                                           (static_cast<std::uint32_t>(green) << 8) |
                                           (static_cast<std::uint32_t>(red) << 16) | (1u << 24);
                stream.write_packed_colour(k, bgra);         // 0047881D
            } else {
                const float rgba[4] = {static_cast<float>(red) / 255.0f,    // 00478857
                                       static_cast<float>(green) / 255.0f,  // 00478872
                                       static_cast<float>(blue) / 255.0f,   // 0047888A
                                       0.003921568859368563f};              // 00CE601C, 1/255
                stream.write_float_colour(k, rgba);
            }
        }
        stream.unlock();                                    // 0047869B
    }

    mesh.set_visibility_factor(static_cast<float>(manager_visibility_byte));  // 004786B2..004786E6
    mesh.register_with_manager();                                            // 004786EB..0047870D
}

void apply_cloud_diffuse_mesh_properties_00477710(const CloudScenePropertyBag& bag,
                                                  CloudMeshEntityFields& entity,
                                                  CloudMeshTarget& mesh) {
    const CloudSceneProperty& diffuse = required(bag, "Diffuse");  // 004788AB, 004773A0
    if (!diffuse.nested) throw std::out_of_range("cloud colour has no channels: Diffuse");
    entity.diffuse_barany[0] = required(*diffuse.nested, "R").as_float();  // 004788DE
    entity.diffuse_barany[1] = required(*diffuse.nested, "G").as_float();  // 00478900
    entity.diffuse_barany[2] = required(*diffuse.nested, "B").as_float();  // 00478925
    entity.diffuse_barany[3] = required(bag, "Scale").as_float();          // 00478935, 00478952

    float sphere[4];
    mesh.get_local_bounds(sphere);                               // 0047896D
    sphere[3] = sphere[3] * entity.diffuse_barany[3];            // 0047896F FMUL, 00478984 FSTP
    mesh.set_local_bounds(sphere);                               // 00478993
    mesh.reset_geometry_bounds_cache();                          // 004789AB / 004789B2
    mesh.register_float4_array(0, "cDiffuseColor_BaranyScale", entity.diffuse_barany, 1);  // 00478A04
}

void cloud_entity_apply_scene_properties_00477710(const CloudEntityPassView& view,
                                                  CloudParticleTarget& particle,
                                                  CloudMeshShaderGlobals& globals,
                                                  CloudMeshEntityFields& entity,
                                                  CloudMeshTarget& mesh,
                                                  CloudEntityPassHost& host) {
    host.base_scene_properties_0048dbd0();                          // 00477737
    if (view.cloud_type == 0 && !view.has_particle_instance) return;  // 00477745..0047774F
    if (view.holder_bag != nullptr && view.holder_kind == 1 && !view.suppressed) {  // 0047775B..00477775
        switch (view.cloud_type) {
        case 0:
            apply_cloud_particle_properties_00477710(*view.holder_bag, view.manager_visibility_byte, particle);
            break;
        case 1:
            apply_cloud_shaded_mesh_properties_00477710(*view.holder_bag, view.manager_visibility_byte,
                                                        globals, entity, mesh);
            break;
        case 2:
            apply_cloud_diffuse_mesh_properties_00477710(*view.holder_bag, entity, mesh);
            break;
        default:
            break;
        }
    }
    host.finish_vtable_d8();                                        // 00478A20..00478A2A
}

} // namespace bsp
