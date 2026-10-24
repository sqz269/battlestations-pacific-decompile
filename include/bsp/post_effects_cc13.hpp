#pragma once
// Post-effect math, texture alpha fix-ups, the .mshd material-shader description
// tree, and render-resource helpers from segments 86 (00B4B490-00B65AC0) and 91
// (00B8A2B0-00B999A0). Packet cc13_render3_post_effects,
// docs/CC13_POST_EFFECTS_AND_RESOURCES.md. Read from the listings; every name is
// a hypothesis, not a recovered symbol.
//
// Status: reconstructed, build-tested. These are new C++ interfaces, not drop-in
// binary replacements: the original ABI of each routine is recorded beside it.
// x87 chains are evaluated in double and rounded to float wherever the image
// stores to a float slot (FSTP float / MOVSS); FSIN/FCOS/exp/sqrt are std::
// calls, so the last bit of a transcendental result can differ from the image.
//
// RNG: nothing in this file draws randomness. The image's post-effect code read
// here (sample offsets, full-screen primitives, luminance/bloom/distortion
// passes) has no call to the shared generator 00BD2F10.

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bsp::cc13fx {

struct Float4 {
    float x{}, y{}, z{}, w{};
};

// ---------------------------------------------------------------------------
// Texture alpha fix-ups applied by the particle texture loader 00B00950 after
// a load: format 15h (D3DFMT_A8R8G8B8) -> premultiply, 16h (D3DFMT_X8R8G8B8)
// -> alpha from non-black, DXT1 and every other format untouched.

// 00B4C490, __fastcall(ECX surface owner), RET. Locks the whole surface
// (vt+2Ch(&D3DLOCKED_RECT, 0, 0)), then for y < vt+20h height, x < vt+1Ch width
// scales B, G, R by A: c = (c * a) / 255 (signed IMUL by 80808081h, exact for
// 0..255*255). Pixel index is width*y + x: the lock pitch is ignored, so the
// image assumes pitch == width*4. Unlock vt+30h only when the lock succeeded.
void premultiply_alpha_bgra(std::uint8_t* pixels, std::uint32_t width, std::uint32_t height);

// 00B4C5D0, same ABI and walk. A = (B + G + R != 0) ? FFh : 0 (NEG of the
// boolean). Colour bytes are not changed.
void alpha_from_non_black_bgra(std::uint8_t* pixels, std::uint32_t width, std::uint32_t height);

// ---------------------------------------------------------------------------
// Post-effect sample tables (the cSampleOffsets / cSampleWeights shader
// constants). The routines are the DirectX SDK HDRLighting helpers with the
// game's own layouts; constants read from .rdata.

// 00B4CC50, __stdcall(int width, int height, Float4* out16), RET 0Ch.
// Downscale4x4: out[row*4 + col].xy = ((col - 1.5) / width, (row - 1.5) / height)
// with col offsets {-1.5,-0.5,0.5,1.5} (doubles CE3D78/CEC9E0/D7A280). z and w
// are not written. Widths are converted as unsigned (FILD + 2^32 when negative).
void sample_offsets_downscale4x4(std::uint32_t width, std::uint32_t height, Float4 out[16]);

// 00B4CDE0, __stdcall(int width, int height, Float4* out9), RET 0Ch.
// SampleAvgLum 3x3: tu = 1/(width*3), tv = 1/(height*3) (double D7A2B0 = 3);
// for i in -1..1: out[3*(i+1) + j] = (i*tu, {-1,0,1}[j]*tv) where the y factors
// are doubles D7A250 = -1, D7A258 = 0 and the plain tv. z and w are not written.
void sample_offsets_avg_lum3x3(std::uint32_t width, std::uint32_t height, Float4 out[9]);

// 00B4CEA0, __stdcall(float x, float y, float rho) -> ST0, RET 0Ch.
// GaussianDistribution: exp(-(x^2 + y^2) / (2*rho*rho)) / sqrt(rho*rho*2pi),
// the exponent and the root each rounded to float; 2pi is the double CE3828
// = (double)(float)6.2831853. exp is the CRT _CIexp at 00BFEEEC (provisional:
// SSE2-dispatching CRT stub matched by role only).
float gaussian_distribution(float x, float y, float rho);

// 00B4D500, __stdcall(int width, int height, Float4* offsets15, Float4* weights15,
// float deviation, float multiplier), RET 18h. Bloom: weights[0] = G(0,0,dev)*mult
// (rgb), alpha 1.0; offsets[0] = 0. For i = 1..7: offsets[i] = (i/width,
// i/height, 0, 0), weights[i] = G(i,0,dev)*mult with alpha 1.0 (G inlined; the
// x^2 and 2*rho*rho terms are kept as doubles before the float exponent).
// Mirror: offsets[i+7] = -0.0f - offsets[i] (all four components, float
// D7A208 = -0.0), weights[i+7] = weights[i].
void sample_offsets_bloom15(std::uint32_t width, std::uint32_t height, Float4 offsets[15],
    Float4 weights[15], float deviation, float multiplier);

// 00B4D980, __stdcall(int width, int height, Float4* offsets13, Float4* weights13,
// float multiplier), RET 14h. GaussBlur5x5 with rho = 1: for x in -2..2 (outer),
// y in -2..2 (inner), |x|+|y| <= 2: offsets[n] = (x/width, y/height, 0, 0),
// weights[n] = exp(-(x^2+y^2)*0.5)/sqrt(2pi) in all four components;
// total += weights[n].x (float). Then every component is divided by total and
// multiplied by multiplier, in that order. Returns the tap count (13); the
// image returns nothing.
int sample_offsets_gauss_blur5x5(std::uint32_t width, std::uint32_t height, Float4 offsets[13],
    Float4 weights[13], float multiplier);

// ---------------------------------------------------------------------------
// Full-screen primitive written by 00B4DCC0 (ECX post-effect pass, one unused
// stack word, RET 4) into the generated model's vertex stream 0 before the
// pass's +14h material draws it. Record count is pass+1Ch: 3 = one oversized
// triangle, otherwise 4 = a quad. Each vertex is seven floats (28 bytes):
// clip x, y, z = 0.5 (CE3800), w = 1.0, texture u, v, and the vertex index.
// Constants: D7A260 -1, CE3D50 -3, CE3850 5, CE3854 3, CE7D7C -2, CE3958 2.
struct FullscreenVertex {
    float x, y, z, w, u, v, index;
};
// Returns the number of vertices written (3 or 4).
int fill_fullscreen_primitive(int vertex_count, FullscreenVertex out[4]);

// ---------------------------------------------------------------------------
// Small matrix helpers in the camera block 00B62DD0-00B65710.

// 00B62DD0, __fastcall(float m[4]) in place, RET. Row-major 2x2 inverse:
// det = m0*m3 - m1*m2; (m3, -m1, -m2, m0) / det. No zero test. No static caller.
void invert2x2(float m[4]);

// 00B62F20, __fastcall(float m[16]) in place, RET. Rows 0..2 of a row-major
// 4x4 (stride 4 floats): normalise r0; r1 -= dot(r0,r1)*r0 and normalise;
// r2 = cross(r0, r1) (004F9B30) and normalise. Each normalise multiplies by
// 1/len when len > 0, else by 0 (length via 00419440). Called at 00B64CB9.
void orthonormalize_rows(float m[16]);

// 00B64470, __fastcall(float out[16] ECX, const float t[3] EDX,
// const float euler[3], const float scale[3]), RET 8. Callers 005ADDA0,
// 00ACBEC0. With s_i = sin(euler[i]), c_i = cos(euler[i]) (FSIN/FCOS, stored
// to float):
//   row0 = scale.x * (c2*c0 - s1*s2*s0,  c1*s2,  -s1*s2*c0 - s0*c2, 0)
//   row1 = scale.y * (-s1*s0*c2 - s2*c0, c1*c2,  s2*s0 - s1*c2*c0,  0)
//   row2 = scale.z * (c1*s0,             s1,     c1*c0,             0)
//   row3 = (t.x, t.y, t.z, 1)
void compose_euler_scale_translation(float out[16], const float t[3], const float euler[3],
    const float scale[3]);

// 00B65560, __thiscall(float planes[24], float vertical, float horizontal,
// float near_d, float far_d), RET 10h. No static caller. Six view-space planes
// (a, b, c, d) for a symmetric frustum with half-angles measured from the
// view axis:
//   ( cos h, 0, sin h, 0), (-cos h, 0, sin h, 0),
//   (0, -cos v, sin v, 0), (0,  cos v, sin v, 0),
//   (0, 0, 1, near_d),     (0, 0, -1, -0.0f - far_d)
void frustum_planes(float planes[24], float vertical, float horizontal, float near_d, float far_d);

// 00B62A10, __thiscall(float out[16], l, r, b, t, zn, zf), RET 18h. The
// D3DXMatrixOrthoOffCenterLH layout (2 is the double D7A308): m00 = 2/(r-l),
// m11 = 2/(t-b), m22 = 1/(zf-zn), m30 = (l+r)/(l-r), m31 = (b+t)/(b-t),
// m32 = zn/(zn-zf), m33 = 1, all else 0. Used by the GUI order pass
// (include/bsp/gui_render_order.hpp).
// The image runs it on the main thread at 24-bit precision control:
// ortho_off_center_lh_pc24 models that per operation and is fixture-tested. The live
// ortho_off_center_lh keeps the double model while this switch is OFF (cc9 flips it).
inline constexpr bool kOrthoOffCenterPc24Cc13Bound = false;
void ortho_off_center_lh(float out[16], float l, float r, float b, float t, float zn, float zf);
void ortho_off_center_lh_pc24(float out[16], float l, float r, float b, float t, float zn, float zf);

// Plane-set classifiers over the 20-byte rows {a, b, c, d, flags} with the
// signed count at +140h (BSP_PlaneSet_GetCount 00B65080). Both return 0AAAh or
// 555h like BSP_NativePlaneSet_ClassifySphere 00B651E0 (AAA = outside there).
struct PlaneRow {
    float a, b, c, d;
    std::uint32_t flags;
};
constexpr std::uint32_t kPlaneSetOutside = 0xaaa;
constexpr std::uint32_t kPlaneSetInside = 0x555;
// 00B65140, __thiscall(set, const float p[3]), RET 4. AAA as soon as a plane
// gives (c*p.z + b*p.y + a*p.x) - d > 0. No static caller. Note the sign: the
// box test below rejects on the negative side, so the two disagree.
std::uint32_t plane_set_classify_point(const PlaneRow* rows, int count, const float p[3]);
// 00B65710, __thiscall(set, const float box[6] {min.xyz, max.xyz}), RET 4.
// With half = 0.5 (double D7A280) and |n| as (n <= 0 ? -0.0f - n : n):
// AAA when dot(n, (min+max)*half) - d < -dot(|n|, (max-min)*half).
std::uint32_t plane_set_classify_aabb(const PlaneRow* rows, int count, const float box[6]);

// ---------------------------------------------------------------------------
// D3DX constant-table census 00B60D30, __fastcall(ID3DXConstantTable* ECX,
// int* out EDX), RET: GetDesc (vt+14h), then for each i < Constants:
// GetConstant(NULL, i) (vt+20h) and GetConstantDesc(handle, descs, &count=100h)
// (vt+18h); sums RegisterCount over descriptors with RegisterSet == 2
// (D3DXRS_FLOAT4). This takes the already-fetched descriptors.
struct ConstantDescRegisters {
    int register_set;    // D3DXCONSTANT_DESC+4
    int register_count;  // D3DXCONSTANT_DESC+0Ch
};
int count_float4_registers(const std::vector<std::vector<ConstantDescRegisters>>& per_constant);

// ---------------------------------------------------------------------------
// .mshd material-shader description tree (Lua; shaderfx/*.mshd run after
// DoFile("fshaders/dx9_lua.inc")). Global `Shader` holds LOD phases, each holds
// versions (DX8/DX9 ...), each holds render modes (RM_* from dx9_lua.inc), each
// holds layers (passes). Integer-keyed children are visited in lua_next order;
// non-integer keys are named fields. The node structs record the original
// object layout as field comments; they are not layout-compatible.
//
// The source is abstracted as MshdTable so this code needs no Lua runtime.

class MshdTable {
public:
    virtual ~MshdTable() = default;
    // Named field access (BSP_LuaObject_GetByName 00B67800 then a type test).
    virtual bool is_nil(const char* key) const = 0;
    virtual bool is_integer(const char* key) const = 0;   // 00B66A60: number with integral value
    virtual bool is_string(const char* key) const = 0;    // 00B660A0
    virtual bool is_boolean(const char* key) const = 0;   // 00B66000: Lua boolean only
    virtual bool is_table(const char* key) const = 0;     // 00B661B0
    // Readers are only called on present keys of the right type, except where
    // the image reads unconditionally (documented per field). get_string on a
    // missing key must throw: the image would strlen a null lua_tostring.
    virtual std::string get_string(const char* key) const = 0;   // 00B662B0
    virtual int get_integer(const char* key) const = 0;          // 00B66290 (truncating __ftol)
    virtual double get_number(const char* key) const = 0;
    virtual bool get_boolean(const char* key) const = 0;
    virtual std::unique_ptr<MshdTable> get_table(const char* key) const = 0;
    // The integer-keyed array part in lua_next order; entries whose key is an
    // integer, whatever the value type. A null child is a non-table value.
    struct Child {
        std::unique_ptr<MshdTable> table;  // null when the value is not a table
        bool value_is_string{};
        bool value_is_integer{};
        std::string string_value;
        int integer_value{};
    };
    // Every entry in lua_next order with is_integer_key telling the key kind.
    struct Entry {
        bool is_integer_key{};
        Child value;
    };
    virtual std::vector<Entry> entries() const = 0;
};

// Shader-constant definition lookup used by the constants reader
// (BSP_ShaderConstants_FindByName). Fields read through 00B5B840 (+8),
// 00B5B850 (+0Ch) and 00B5B860 (+10h).
struct ShaderConstantDefinition {
    int dimension_8{};   // +08, BSP_ShaderConstant_GetSecondDimension
    int dimension_c{};   // +0C, BSP_ShaderConstant_GetFirstDimension
    int array_count{};   // +10, BSP_ShaderConstant_GetArrayCount
};
using ShaderConstantLookup = std::function<const ShaderConstantDefinition*(const std::string&)>;

// One VS_/PS_Constants row, 1Ch record built by 00B56E30.
struct ShaderConstantRow {
    const ShaderConstantDefinition* definition{};  // +00 lookup result or null
    std::string name;                              // +04 native string
    int dimension_8{};                             // +0C
    int dimension_c{};                             // +10
    int array_count{1};                            // +14
    int register_index{};                          // +18 running register at insert
};
// 10h list from 00B58400: {rows data, count, capacity, next_register(+0C)}.
struct ShaderConstantList {
    std::vector<ShaderConstantRow> rows;
    int next_register{};  // +0C; 00B58160 adds dimension_8 * array_count per row
};

// 00B55F00 Caps reader; the Version object embeds it at +0Ch (44h bytes). The
// version allocator 00B5AFF0 does NOT initialise this block, so without a
// `Caps` table every field below is uninitialised heap in the image; here they
// are std::nullopt.
struct MshdCaps {
    std::optional<int> max_samplers;            // caps+10h (version+1Ch)
    std::optional<int> max_texture_registers;   // caps+14h (version+20h)
    std::optional<int> min_pixel_shader;        // caps+28h, 101h (1.1) when Caps exists
    std::optional<int> min_vertex_shader;       // caps+2Ch, 101h (1.1) when Caps exists
};

// Opaque results of the existing state/sampler readers, which the layer
// reader calls but this packet does not reconstruct.
struct MshdLayerHooks {
    // BSP_ShaderDescriptor_ReadRenderStates (layer+0Ch list) on `RenderStates`.
    std::function<void(const MshdTable&)> read_render_states;
    // BSP_ShaderStateDefinitions_ReadSampler on each integer child of `Samplers`.
    std::function<void(const MshdTable&)> read_sampler;
    ShaderConstantLookup find_constant;
};

// Layer, 78h object, vtable 00D621F8, constructor 00B56B90, reader 00B5A1F0.
struct MshdLayer {
    std::string name;                       // +04 `Name` (unconditional string)
    bool has_render_states{};               // +0C list (empty when no table)
    int fog{0};                             // +10 `Fog` (FM_*)
    int fog_type{1};                        // +14 `FogType` (FT_*, default FT_NORMAL)
    bool receive_shadows{};                 // +18 `ReceiveShadows` (boolean only)
    bool directional_lighting{};            // +19 `DirectionalLightning` (boolean only)
    bool point_lighting{};                  // +1A `PointLightning` (boolean only)
    int diffuse_light{0};                   // +1C `DiffuseLight` (LIT_*)
    int specular_light{0};                  // +20 `SpecularLight` (LIT_*)
    int sampler_count{};                    // +24/+28/+2C sampler pointer vector
    std::string vs_target;                  // +30 `VS_Target` (unconditional)
    std::string vs_body;                    // +38 `VS_Body` (unconditional)
    std::string ps_target;                  // +40 `PS_Target` (unconditional)
    std::string ps_body;                    // +48 `PS_Body` (unconditional)
                                            // +50 parent render mode
    int render_queue{};                     // +54 `RenderQueue` (unconditional integer, RPID_*)
    bool per_pixel_normalize{};             // +58 `PerPixelNormalize` (boolean only)
    int reserved_color_registers{0};        // +5C
    int reserved_texture_registers{0};      // +60
    std::optional<ShaderConstantList> vs_constants;  // +64
    std::optional<ShaderConstantList> ps_constants;  // +68
    int ps_code{1};                         // +6C `PS_Code`
    std::string vertex_format;              // +70 `VertexFormat` (.mvfm name, string only)
};

// RenderMode, 20h object, vtable 00D621FC, reader 00B5ADF0.
struct MshdRenderMode {
    std::string name;                // +04 `Name`
    int render_mode{};               // +0C `RenderMode` (RM_*, unconditional)
    std::vector<MshdLayer> layers;   // +10/+14/+18; each layer+50h = this
                                     // +1C parent version
};

// Version, 78h object, vtable 00D62264, reader 00B5AFF0.
struct MshdVersion {
    std::string name;                         // +04 `Name`
    MshdCaps caps;                            // +0C..+4F `Caps`
                                              // +50..+64 zeroed, not read here
    std::vector<MshdRenderMode> render_modes; // +68/+6C/+70; each mode+1Ch = this
                                              // +74 parent LOD phase
};

// LOD phase, 1Ch object, vtable 00D621F0, reader 00B5B240.
struct MshdLodPhase {
    int index{};                        // +04 ordinal among integer children (root)
    float lod_value{};                  // +08 `LodValue` (0 when nil)
    std::vector<MshdVersion> versions;  // +0C/+10/+14; each version+74h = this
                                        // +18 parent shader
};

// Shader root, 1Ch refcounted object, vtable 00D62200, constructor 00B571E0
// (name at +08), built by 00B5B3F0 from global `Shader`.
struct MshdShader {
    std::string name;                     // +08
    std::vector<MshdLodPhase> lod_phases; // +10/+14/+18
};

MshdCaps read_mshd_caps(const MshdTable& caps);                                       // 00B55F00
ShaderConstantList read_mshd_constants(const MshdTable& list, const ShaderConstantLookup&);  // 00B59E70
MshdLayer read_mshd_layer(const MshdTable& layer, const MshdLayerHooks&);              // 00B5A1F0
MshdRenderMode read_mshd_render_mode(const MshdTable& mode, const MshdLayerHooks&);    // 00B5ADF0
MshdVersion read_mshd_version(const MshdTable& version, const MshdLayerHooks&);        // 00B5AFF0
MshdLodPhase read_mshd_lod_phase(const MshdTable& phase, const MshdLayerHooks&);       // 00B5B240
// 00B5B3F0 minus the Lua state: `shader_global` is the value of global `Shader`.
MshdShader read_mshd_shader(const std::string& name, const MshdTable& shader_global,
    const MshdLayerHooks&);

// 00B5B8E0, __thiscall(table ECX, const NativeString* name), RET 4. Linear scan
// of a 20h-stride record array (data at +4, count at +8): record+14h holds a
// native string {size, chars}; equal size and (both empty or _stricmp == 0)
// selects record+1Ch. Miss returns 37h. No static caller.
struct NamedIndexRecord {
    std::string name;  // +14
    int value{};       // +1C
};
int find_named_index(const std::vector<NamedIndexRecord>& records, const std::string& name);

// ---------------------------------------------------------------------------
// Segment 91: Mit::allocator block pools (RTTI Mit::allocator_list_elem).
// Each owner links into the allocator list 00E188B4, holds a CRITICAL_SECTION
// at +0Ch with a recursion counter at +24h, a block pointer vector
// {+28h data, +2Ch count, +30h capacity} and the current block index +34h
// (-1 = none). A block is 32 slots of `stride` bytes, the slot's last dword
// holding its block index, then a uint16 free-index stack [32] and a uint16
// free count. Three instantiations:
//   stride 1E4h (flare, global 010902A8): ctor 00B8D580, block init 00B8B6D0,
//     allocate 00B8D700, free 00B8C3A0, trim 00B8D660 (vtable D63410 slot 0)
//   stride 198h (quadtree node): block init 00B8FA80, allocate 00B90800,
//     free 00B90020, trim 00B90760
//   stride 194h (bone model): block init 00B8FB50, allocate 00B90AC0,
//     free 00B90150, trim 00B90A20
// This models the slot order only (which block/slot a call returns), not the
// memory; the lock is the caller's.
class MitBlockPool {
public:
    static constexpr int kSlots = 32;
    struct Slot {
        int block{-1};
        int slot{-1};
    };
    // allocate: when current == -1 a block is appended (stack[i] = 31 - i,
    // count 32) and becomes current. Pops stack[--count] of the current block;
    // when that empties the block, current becomes the first later block with
    // a free slot, else -1. Slot 0 of a fresh block is handed out first.
    Slot allocate();
    // free: pushes the slot index at stack[count++]; current = min(current,
    // block) compared unsigned, so -1 always takes the block.
    void free(Slot s);
    // trim: every block whose count is 32 is released and replaced by the last
    // block (whose slots are renumbered), re-examining the same index; then
    // current = the first block with a free slot, else -1. In 00B8D660 the
    // body after CALL _free (00B8D686..00B8D6BF) is missing from the Ghidra
    // listing because the call is treated as no-return.
    void trim();
    int current() const { return current_; }
    int block_count() const { return static_cast<int>(blocks_.size()); }
    int free_count(int block) const { return blocks_.at(block).count; }

private:
    struct Block {
        std::array<std::uint16_t, kSlots> stack{};
        std::uint16_t count{};
        int id{};  // stands in for the block address
    };
    std::vector<Block> blocks_;
    int current_{-1};
    int next_id_{};
};

// ---------------------------------------------------------------------------
// Flare object (1E0h payload in the 1E4h pool; vtable D63420, scalar delete
// 00B8D9A0, constructor 00B8D9C0 "flare"). Fields used below: +1A4h/+1A8h the
// projected position (clip x, y), +1B4h visibility, +1B8h, +1C8h flag,
// +1C0h/+1C4h a pair compared by 00B8B670, +1DCh the pow base of 00B8B8C0.

// 00B8B350, __fastcall(ECX flare) -> ST0, RET. 0 unless visibility > 0; else
// (1.4142136 - sqrt(x*x + y*y)) * 0.2 + 0.05 with the sum and the root
// rounded to float (doubles D633E0, CE3D10, D7A270). Larger toward the centre.
float flare_centre_weight(float x, float y, float visibility);

// 00B8B3B0, __fastcall(ECX flare) -> ST0, RET. Screen-edge fade. 0 unless
// visibility > 0 and (visibility <= limit(+1B8h) or flag(+1C8h) == 0), and
// -1 < x < 1 and -1 < y < 1. f starts at 1; x < -0.94 sets f = (x + 0.94)/0.06
// + 1, y < -0.94 multiplies by (y + 0.94)/0.06 + 1, x > 0.94 and then y > 0.94
// multiply by 1 - (v - 0.94)/0.06 (doubles D633F8 -0.94,
// D633F0 0.94, D633E8 0.06). f is stored to float after every step.
float flare_edge_fade(float x, float y, float visibility, float limit, bool limit_flag);

// 00B8B670, __fastcall(ECX flare) -> AL, RET. false when +1C0h == 0.0
// exactly; otherwise +1C4h >= +1C0h (false when either is NaN).
bool flare_pair_test(float a_1c0, float b_1c4);

// 00B8B8C0, __thiscall(ECX flare owner, unused, element*), RET 8.
// k = pow-base via the CRT stub 00BFA910 (log, provisional) of owner+1DCh;
// e = exp(k * element+44h) (00BFEEEC); element+34h/+38h = element+8h/+0Ch +
// e * element+3Ch/+40h. I.e. base + dir * owner^t.
void flare_place_element(float owner_base, const float origin[2], const float dir[2], float t,
    float out[2]);

// 00B8C520, __thiscall(const uint8 rgba[4] ECX, uint8 out[4], float scale),
// RET 8. Each byte times scale, clamped to [0, 255] (double CE4B48 test,
// float CEDD44 value) and rounded with FISTP (round-half-even in the default
// control word); output byte order equals input order.
void scale_color32(const std::uint8_t in[4], std::uint8_t out[4], float scale);

// Index pattern of the disc fan written by 00B8B980 (centre vertex 0, rim
// vertices 1..n): triangle i = (0, i+1, (i+1) % n + 1), 16-bit indices.
void disc_fan_indices(std::uint32_t rim_count, std::vector<std::uint16_t>& out);

// 00B8B980 whole body (cc13-P4), __stdcall(geometry, const float radius_xy[2],
// uint32 rim_count), RET 0Ch; ECX is not read. The angle and step are locals,
// not register inputs: angle starts at 0.0f (00B8B999) and the step is
// 2pi / rim_count (00B8B995..00B8B9B3: FILD, +2^32 when negative, FDIVR by the
// double at 00CE3828, which is 2pi rounded to float). The host owns the two
// D3D buffers: 00B73260(geometry, 0) then its vtable+10h(0,0,0) lock and +14h
// unlock for vertices; [geometry+60h] vtable+0Ch(0,0,0) lock and +10h unlock
// for indices.
struct FlareDiscFanHost {
    virtual ~FlareDiscFanHost() = default;
    virtual unsigned char* lock_vertex_stream0_00b73260() = 0;
    virtual void unlock_vertex_stream0() = 0;
    virtual std::uint16_t* lock_index_buffer_60() = 0;
    virtual void unlock_index_buffer_60() = 0;
};
// 28h-byte vertices: centre {0,0,0, 1.0f, 00FFFFFFh}; rim k {r.x*cos a,
// r.y*sin a, 0.999f (00D63400), 1.0f, 00FFFFFFh, +18h/+1Ch 0.5f}. The image
// leaves +14h, and +18h..+27h of the centre, unwritten. Then the indices of
// disc_fan_indices. Float math is modelled at the 24-bit precision control.
void write_disc_fan_00b8b980(FlareDiscFanHost& host, const float radius_xy[2],
    std::uint32_t rim_count);

} // namespace bsp::cc13fx
