#include "bsp/vertex_format.hpp"
#include <limits>
#include <utility>

namespace bsp {
namespace {
struct Alias { std::string_view name, encoded; };
// Literal pairings from00b2dc32..00b2e2b8, including the three direct stricmp
// terrain rewrites. Preserve the native spellings, including "skined".
constexpr Alias aliases[] = {
    {"simple.mvfm", "pf43nf43uf42.mvfm"},
    {"simpleindexed.mvfm", "pf43nf43uf42if41.mvfm"},
    {"color.mvfm", "pf43cc.mvfm"},
    {"position.mvfm", "pf43.mvfm"},
    {"oceanheightmapgen.mvfm", "pf44uf42.mvfm"},
    {"oceannormalmapgen.mvfm", "pf44uf42uf42uf42uf42uf42.mvfm"},
    {"oceanheightmap.mvfm", "nf24.mvfm"},
    {"simplecolor.mvfm", "pf43ccuf42.mvfm"},
    {"simplecolor2.mvfm", "pf44ccccuf42uf42.mvfm"},
    {"ship.mvfm", "pf43nf43uf42uf42tf43bf43.mvfm"},
    {"shipvc.mvfm", "pf43nf43uf42uf42tf43bf43cc.mvfm"},
    {"gun.mvfm", "pf43nf43uf42.mvfm"},
    {"gunvc.mvfm", "pf43nf43uf42cc.mvfm"},
    {"gunvcindexed.mvfm", "pf43nf43uf42ccif41.mvfm"},
    {"ocean.mvfm", "pf43nf43uf43.mvfm"},
    {"projocean.mvfm", "pf42.mvfm"},
    {"projoceanfull.mvfm", "pf42nc.mvfm"},
    {"terraindx9ati.mvfm", "pcpc.mvfm"},
    {"shadowmap.mvfm", "pf43nf43uf42.mvfm"},
    {"airplane.mvfm", "pf43nf43uf42tf43bf43.mvfm"},
    {"airplaneindexed.mvfm", "pf43nf43uf42tf43bf43if41.mvfm"},
    {"transformpps.mvfm", "pf43.mvfm"},
    {"particlesprite.mvfm", "pf43ccuf41uf41uf42uf42.mvfm"},
    {"particleaxial.mvfm", "pf43ccuf43uf42uf42.mvfm"},
    {"particlefloating.mvfm", "pf44ccuf24uf42.mvfm"},
    {"particleaxialsprite.mvfm", "pf44ccuf44uf24uf42uf22.mvfm"},
    {"coast.mvfm", "pf43.mvfm"},
    {"beam.mvfm", "pf43uf43cc.mvfm"},
    {"waterparticle.mvfm", "cc.mvfm"},
    {"watertracer.mvfm", "pf43ccuf42.mvfm"},
    {"choppy.mvfm", "pf43nf43cc.mvfm"},
    {"cloud.mvfm", "pf43ccuf42uf42.mvfm"},
    {"impostor.mvfm", "pf43uf42.mvfm"},
    {"skined.mvfm", "pf43nf43uf42wf44if44.mvfm"},
    {"skinedUW.mvfm", "pf43nf43tf43bf43uf42wf44if44.mvfm"},
    {"FoliageNoInst.mvfm", "pf43nf43uf42uf41cf43.mvfm"},
    {"rope.mvfm", "pf43nf43uf42uf41.mvfm"},
    {"FullscreenQuad.mvfm", "pf44uf42uf42uf42uf42.mvfm"},
    {"lightning.mvfm", "pf43uf41uf42.mvfm"},
    {"foliagesprite.mvfm", "pf43uf42ccuf43.mvfm"},
    {"traceline.mvfm", "pf43uf43ccuf44uf41.mvfm"},
    {"airfield.mvfm", "pf43nf43uf42uf42.mvfm"},
    {"watertracerskined.mvfm", "pf44if43wf43uf44uf42.mvfm"},
    {"watertracerskinedstatic.mvfm", "pf44uf42uf42.mvfm"},
    {"waterspray.mvfm", "pf43uf42uf44uf43.mvfm"},
    {"bterrain2.mvfm", "pf43nf43uf42uf42.mvfm"},
    {"bterrain3.mvfm", "pf43nf43uf42uf42uf42.mvfm"},
    {"shore.mvfm", "pf43uf42cc.mvfm"},
    {"river.mvfm", "pf43nf43tf43bf43uf42cc.mvfm"},
    {"foliagevc.mvfm", "pf43nf43uf42cc.mvfm"},
    {"decal.mvfm", "pf43nf43uf44uf42.mvfm"},
    {"bterrain4.mvfm", "pf43nf43uf42uf42uf42uf42.mvfm"},
    {"uterrain3.mvfm", "pf43nf43uf42uf42uf42cc.mvfm"},
    {"uterrain4.mvfm", "pf43nf43uf42uf42uf42uf42cc.mvfm"},
};
struct Usage { char token; std::uint32_t value; };
constexpr Usage usages[] = {{'p',0}, {'w',1}, {'i',2}, {'n',3},
    {'u',5}, {'t',6}, {'b',7}, {'c',10}};
// Native token-search order at0108d5a8. Array index is the element type enum.
constexpr std::string_view types[] = {"f41", "f42", "f43", "f44", "c",
    "ub4", "ss2", "ss4", "ubn4", "ssn2", "ssn4", "usn2", "usn4",
    "ud3", "sdn3", "f22", "f24"};
char lower_ascii(char value) {
    return value >= 'A' && value <= 'Z' ? value + ('a' - 'A') : value;
}
bool equal_ascii(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i != a.size(); ++i)
        if (lower_ascii(a[i]) != lower_ascii(b[i])) return false;
    return true;
}
bool fail(std::string& error, const char* reason) {
    error = reason;
    return false;
}
}

bool decode_vertex_format_00b2dbd0(std::string_view name,
    VertexDeclaration& output, std::string& error) {
    error.clear();
    if (name.find('\0') != std::string_view::npos)
        return fail(error, "Vertex format name contains an embedded NUL");
    // Native arithmetic is unchecked. Keep packed byte offsets representable
    // in this valid-input host projection (16 is the largest element size).
    if (name.size() > std::numeric_limits<std::uint32_t>::max() / 16)
        return fail(error, "Vertex format name exceeds the host length domain");
    for (const auto& alias : aliases) {
        if (equal_ascii(name, alias.name)) {
            name = alias.encoded;
            break;
        }
    }
    VertexDeclaration parsed;
    std::size_t cursor = 0;
    while (cursor < name.size() && name[cursor] != '.') {
        const Usage* usage = nullptr;
        for (const auto& entry : usages) {
            if (lower_ascii(name[cursor]) == entry.token) {
                usage = &entry;
                break;
            }
        }
        if (!usage) return fail(error, "Unrecognized vertex usage token");
        ++cursor;
        std::uint32_t type = 0;
        for (; type != 17; ++type) {
            const auto token = types[type];
            if (equal_ascii(name.substr(cursor, token.size()), token)) break;
        }
        if (type == 17) return fail(error, "Unrecognized vertex type token");
        cursor += types[type].size();
        parsed.append_00b48330(type, usage->value, -1);
    }
    if (!equal_ascii(name.substr(cursor), ".mvfm") || parsed.elements().empty())
        return fail(error, "Vertex format requires a .mvfm suffix and an element");
    output = std::move(parsed);
    return true;
}

bool resolve_mesh_vertex_format_layout_00b2dbd0(const std::string& name,
    MeshVertexFormatLayout& output, std::string& error) {
    VertexDeclaration declaration;
    if (!decode_vertex_format_00b2dbd0(name, declaration, error)) return false;
    output = {declaration.stride,
        static_cast<std::uint32_t>(declaration.elements().size())};
    return true;
}
}
