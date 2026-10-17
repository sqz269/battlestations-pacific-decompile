#include "bsp/shadow_cascades_cc13.hpp"

#include "bsp/camera_affine.hpp"
#include "bsp/camera_multiply.hpp"
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/native_camera_projection.hpp"
#include "bsp/sound_streaming_cc13.hpp"

#include <array>
#include <cmath>
#include <cstring>

// Directional shadow view fitting. Evidence and ABIs: docs/CC13_SHADOW_CASCADES.md.

namespace bsp::cc13_shadow {
namespace {

using D = double;
using Vec3 = std::array<float, 3>;

template<class T> T read(const void* object, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(object) + offset, sizeof value);
    return value;
}
template<class T> void write(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(object) + offset, &value, sizeof value);
}
float* field(void* object, std::size_t offset) noexcept {
    return reinterpret_cast<float*>(static_cast<unsigned char*>(object) + offset);
}
CameraMatrix read_matrix(const void* object, std::size_t offset) noexcept {
    return read<CameraMatrix>(object, offset);
}
void write_matrix(void* object, std::size_t offset, const CameraMatrix& m) noexcept {
    write(object, offset, m);
}
CameraMatrix identity() noexcept {
    return {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}; // 1.0f from 00D7A24C
}
CameraMatrix multiply(const CameraMatrix& left, const CameraMatrix& right) {
    CameraMatrix out;
    multiply_camera_matrices_00413920(out, left, right);
    return out;
}
float tangent(float argument) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &argument, sizeof bits);
    return native_projection_tangent_00412e20(bits);
}
void refresh_unless_valid(void* camera, ShadowFitHost& host) {
    if ((read<std::uint8_t>(camera, 0x5c) & 2u) == 0) host.refresh_world_matrix(camera);
}

constexpr std::uint32_t kShadowCommandMetadata[3] = {0, 0x62, 0x1e};

using Vec4 = std::array<float, 4>;

Vec4 transform4(const Vec4& source, const CameraMatrix& matrix) noexcept {
    Vec4 out;
    transform_native_vector4_00b62d10(source.data(), out.data(), matrix.data());
    return out;
}
// Native magnitude: FCOMIP/COMISS against 0 then JBE to the -0.0f SUBSS, so
// zero and NaN take the negated copy.
float magnitude(float v) noexcept {
    return v > 0.0f ? v : -v;
}
// Light look-at up vector (00A8C15F..00A8C1B3, 00A8CA38..00A8CA9F).
Vec3 pick_up(float x, float y) noexcept {
    return magnitude(y) > magnitude(x) ? Vec3{0.0f, 0.0f, 1.0f} : Vec3{0.0f, 1.0f, 0.0f};
}
// Bound updates of the BD20 loops: FCOMI(P) then JBE/JA, so a NaN replaces
// the bound.
void lower(float& bound, float v) noexcept { if (!(v > bound)) bound = v; }
void raise(float& bound, float v) noexcept { if (!(bound > v)) bound = v; }
// One x87 arithmetic operation of 00A8BD20 under the main thread's 24-bit
// precision control (docs/CC13_FPU_PRECISION.md): the exact result rounded to
// float. Float and float-exact double operands make the double step exact or
// an innocuous double rounding.
float pc24(D value) noexcept {
    return static_cast<float>(value);
}
// Square sum of 00A8C82B / 00A8D404: each square spilled to float, the first
// two added on the x87, the result spilled. 24-bit PC: the unspilled first
// sum rounds to float too.
float square_length_sum(float x, float y, float z) noexcept {
    const float yy = static_cast<float>(D(y) * y);
    const float xx = static_cast<float>(D(x) * x);
    const float zz = static_cast<float>(D(z) * z);
    return pc24(D(pc24(D(yy) + xx)) + zz);
}

} // namespace

void build_trapezoid_homography_00a8b930(float* m, const float* p, const float* a,
    const float* b, const float* c) noexcept
{
    // 00A8B93E..00A8B9BF: rotation rows from u = (b - c) * 0.5 (00D7A280).
    const float ux = static_cast<float>((D(b[0]) - c[0]) * 0.5);
    const float nux = -ux; // MOVSS 00D7A208 (-0.0f) SUBSS: exact negation
    m[4] = nux;
    const float uy = static_cast<float>(0.5 * (D(b[1]) - c[1]));
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = 1.0f; // 00D7A24C
    m[0] = ux;
    m[1] = uy;
    m[2] = static_cast<float>(D(uy) * uy + D(ux) * ux); // dead: rewritten below
    m[3] = uy;
    const D cross = D(ux) * uy;
    m[5] = static_cast<float>(cross - cross); // FSUBR ST0,ST0: 0, or NaN for inf/NaN; dead
    // 00A8B9C2: i = line(p, c) x line(a, b); p stays in EDX across the call.
    float i[2];
    bsp::cc13_audio::intersect_lines_2d_00a8b860(i, p, c, a, b);
    const float m2 = static_cast<float>(-(D(i[0]) * ux + D(i[1]) * uy));
    m[2] = m2;
    const float m5 = static_cast<float>(-(D(i[0]) * uy + D(i[1]) * nux));
    m[5] = m5;
    // 00A8BA13..00A8BA9F: shear factor from the transformed b and c.
    const float cy = c[1], cx = c[0], by = b[1], bx = b[0];
    const float ta = static_cast<float>((D(cy) * uy + D(cx) * ux) + m2);
    const float tb = static_cast<float>(m2 + (D(ux) * bx + D(by) * uy));
    const float tc = static_cast<float>((D(cx) * uy + D(nux) * cy) + m5);
    const float td = static_cast<float>(m5 + (D(nux) * by + D(bx) * uy));
    const float shear = static_cast<float>(-((D(ta) + tb) / (D(tc) + td)));
    const float r0 = static_cast<float>(D(uy) * shear + ux);
    m[0] = r0;
    const float r1 = static_cast<float>(D(shear) * nux + uy);
    m[1] = r1;
    const float r2 = static_cast<float>(D(shear) * m5 + m2);
    m[2] = r2;
    // 00A8BAE6..00A8BB9B: scale rows 1 and 2 by b, then the projective step.
    const float s1 = static_cast<float>(1.0 / (D(r2) + (D(r0) * b[0] + D(r1) * b[1])));
    const float s2 = static_cast<float>(1.0 / ((D(uy) * b[0] + D(b[1]) * nux) + m5));
    m[0] = static_cast<float>(D(r0) * s1);
    m[1] = static_cast<float>(D(r1) * s1);
    m[2] = static_cast<float>(D(s1) * r2);
    const float q3 = static_cast<float>(D(uy) * s2);
    m[3] = q3;
    const float q4 = static_cast<float>(D(s2) * nux);
    m[4] = q4;
    const float q8 = static_cast<float>(D(s2) * m5);
    m[6] = q3;
    m[7] = q4;
    m[8] = q8;
    const float q5 = static_cast<float>(1.0 + q8);
    m[5] = q5;
    // 00A8BBA6..00A8BC9C: translate so b and p are centred in y/w.
    const float py = p[1], px = p[0];
    const float e = static_cast<float>((D(b[1]) * q4 + D(b[0]) * q3) + q5);
    const float f = static_cast<float>((D(b[0]) * q3 + D(b[1]) * q4) + q8);
    const float g = static_cast<float>(q5 + (D(py) * q4 + D(px) * q3));
    const float h = static_cast<float>((D(q3) * px + D(q4) * py) + q8);
    const float t = static_cast<float>(((D(g) / h) + (D(e) / f)) * -0.5); // 00CEC9E0
    const float n3 = static_cast<float>(D(q3) * t + q3);
    m[3] = n3;
    const float n4 = static_cast<float>(D(q4) * t + q4);
    m[4] = n4;
    const float n5 = static_cast<float>(D(t) * q8 + q5);
    m[5] = n5;
    // 00A8BCA2..00A8BD13: scale row 2 so p lands on y/w = -1; p is reread.
    const float py2 = p[1], px2 = p[0];
    const float wi = static_cast<float>(q8 + (D(q3) * px2 + D(q4) * py2));
    const float wj = static_cast<float>((D(px2) * n3 + D(n4) * py2) + n5);
    const float w = static_cast<float>(D(-wi) / wj);
    m[3] = static_cast<float>(D(n3) * w);
    m[4] = static_cast<float>(D(n4) * w);
    m[5] = static_cast<float>(D(n5) * w);
}

void fit_shadow_box_view_00a8aeb0(void* owner, const float* box_min, const float* box_max,
    ShadowFitHost& host)
{
    write(owner, 0x390, 100.0f); // 00CE3D08
    write(owner, 0x394, 200.0f); // 00CE386C
    write(owner, 0x398, 300.0f); // 00CE3AE8
    write(owner, 0x39c, 400.0f); // 00CFD710
    write_matrix(owner, 0x44, identity());  // 004134F0, ECX = owner+44h
    write_matrix(owner, 0x144, identity()); // 004134F0, ECX = owner+144h
    // 00A8AFF3..00A8B0FB: light direction.
    void* const light = read<void*>(owner, 0x0c);
    if (host.light_query_0c(light, host.override_context_word())) {
        host.normalize(field(owner, 0x38), field(light, 0x1e0)); // FLD/FSTP copies
        const float y = read<float>(owner, 0x3c);
        const float bound_angle = 0.436332f; // 00D5B564 (3EDF66F4h)
        float clamped;
        if (y >= 0.0f) { // COMISS 00D7A218 / JB: NaN takes the other branch
            const float t = tangent(bound_angle);
            clamped = y > t ? y : t; // FCOMIP / JBE: unordered keeps t
        } else {
            const float t = -tangent(bound_angle); // FCHS then FSTP32
            clamped = t > y ? y : t;
        }
        const float tilted[3] = {read<float>(owner, 0x38), clamped, read<float>(owner, 0x40)};
        host.normalize(field(owner, 0x38), tilted);
    } else {
        write(owner, 0x38, 0.0f);
        write(owner, 0x3c, 1.0f);
        write(owner, 0x40, 0.0f);
    }
    // 00A8B100..00A8B1CB: look from position + 10000*dir back at the position.
    void* const camera = read<void*>(owner, 0x30);
    refresh_unless_valid(camera, host);
    const float position[3] = {read<float>(camera, 0x120), read<float>(camera, 0x124),
                               read<float>(camera, 0x128)};
    const D reach = 10000.0; // 00CE4BD8 (double)
    const float ox = static_cast<float>(read<float>(owner, 0x38) * reach);
    const float oy = static_cast<float>(read<float>(owner, 0x3c) * reach);
    const float oz = static_cast<float>(reach * read<float>(owner, 0x40));
    const float eye[3] = {static_cast<float>(D(ox) + position[0]),
                          static_cast<float>(D(oy) + position[1]),
                          static_cast<float>(D(oz) + position[2])};
    CameraMatrix view;
    host.look_at(view, eye, position, 0.0f, 1.0f, 0.0f);
    void* const shadow_camera = read<void*>(owner, 0x20);
    host.set_view_matrix(shadow_camera, view);
    // 00A8B1D0..00A8B30D: the eight box corners into light space.
    const float ax = box_min[0], ay = box_min[1], az = box_min[2];
    const float bz = box_max[2], bx = box_max[0], by = box_max[1];
    Vec3 corners[8] = {{ax, ay, az}, {ax, ay, bz}, {bx, ay, bz}, {bx, ay, az},
                       {ax, by, az}, {ax, by, bz}, {bx, by, bz}, {bx, by, az}};
    for (Vec3& corner : corners) {
        Vec3 out;
        transform_point_004142e0(corner, view, out);
        corner = out;
    }
    // 00A8B30F..00A8B3F8: bounds; a NaN component never replaces a bound.
    Vec3 lo = corners[0], hi = corners[0];
    for (int k = 1; k < 8; ++k) {
        for (int axis = 0; axis < 3; ++axis) {
            const float v = corners[k][axis];
            if (lo[axis] > v) lo[axis] = v;
            if (v > hi[axis]) hi[axis] = v;
        }
    }
    // 00A8B3FE..00A8B4D2: projection = ortho(bounds) * diag(-1, 1, 1, 1).
    CameraMatrix ortho;
    host.ortho_off_center(ortho, lo[0], hi[0], lo[1], hi[1], lo[2], hi[2]);
    CameraMatrix mirror = identity();
    mirror[0] = -1.0f; // 00D7A260
    host.set_projection_matrix(shadow_camera, multiply(ortho, mirror));
    // 00A8B4D7..00A8B56C: depth remap of the box to [0, 1].
    CameraMatrix depth = identity();
    depth[10] = static_cast<float>(1.0 / (D(hi[2]) - lo[2]));
    depth[14] = static_cast<float>(D(lo[2]) / (D(lo[2]) - hi[2]));
    const CameraMatrix light_view = multiply(view, depth);
    // 00A8B571..00A8B654: cascade matrix and shader matrix.
    write_matrix(owner, 0x44, host.view_projection_matrix(shadow_camera));
    write_matrix(owner, 0x144, multiply(read_matrix(owner, 0x44), read_matrix(owner, 0x244)));
    // 00A8B657..00A8B7AF
    write_matrix(owner, 0x3ac, light_view);
    write(owner, 0x4ac, light_view[2]);
    write(owner, 0x4b0, light_view[6]);
    write(owner, 0x4b4, light_view[10]);
    write(owner, 0x4b8, light_view[14]);
    write(owner, 0x14c, read<float>(owner, 0x4ac));
    write(owner, 0x15c, read<float>(owner, 0x4b0));
    write(owner, 0x16c, read<float>(owner, 0x4b4));
    write(owner, 0x17c, read<float>(owner, 0x4b8));
    host.set_depth_scale_pointer(shadow_camera, field(owner, 0x4ac));
}

void fit_and_submit_shadow_slice_00a8ecb0(void* owner, float near_distance, float far_distance,
    std::int32_t cascade, ShadowFitHost& host)
{
    // 00A8ECD1..00A8ED24: half-angle tangents; 00B6FC30 is FLD aspect, FMUL fov, FSTP32.
    void* camera = read<void*>(owner, 0x30);
    const float fov_aspect = static_cast<float>(D(read<float>(camera, 0x1c8)) * read<float>(camera, 0x1c4));
    const float fov = read<float>(camera, 0x1c4);
    const float tx = tangent(static_cast<float>(fov_aspect * 0.5)); // 00D7A280
    const float ty = tangent(static_cast<float>(fov * 0.5));
    // 00A8ED28..00A8EE9D: the slice corners in camera space.
    const float xn = static_cast<float>(D(tx) * near_distance);
    const float xf = static_cast<float>(D(tx) * far_distance);
    const float yn = static_cast<float>(D(ty) * near_distance);
    const float yf = static_cast<float>(D(ty) * far_distance);
    const float n = near_distance, f = far_distance;
    float corners[8][3] = {{-xn, -yn, n}, {xn, -yn, n}, {xn, yn, n}, {-xn, yn, n},
                           {-xf, -yf, f}, {xf, -yf, f}, {xf, yf, f}, {-xf, yf, f}};
    // 00A8EE73..00A8EF21: unit-scale perspective for the slice.
    CameraMatrix perspective = identity();
    const float q = static_cast<float>(D(f) / (D(f) - n));
    perspective[10] = q;
    perspective[11] = 1.0f;
    perspective[14] = static_cast<float>(D(-n) * q);
    perspective[15] = 0.0f;
    const CameraMatrix view_perspective = multiply(host.view_matrix(camera), perspective);
    // 00A8EF34..00A8EFDB: horizontal forward from world row 2 (+110h, +118h).
    void* const camera_a = read<void*>(owner, 0x30);
    refresh_unless_valid(camera_a, host);
    void* const camera_b = read<void*>(owner, 0x30);
    refresh_unless_valid(camera_b, host);
    const float fx = read<float>(camera_b, 0x110);
    const float fz = read<float>(camera_a, 0x118);
    const float flat[3] = {fx, 0.0f, fz};
    const float len = host.length(flat);
    const float inv = len > 0.0f ? static_cast<float>(1.0 / len) : 0.0f; // FCOMI / JBE
    const float hx = static_cast<float>(D(fx) * inv);
    const float hy = static_cast<float>(0.0 * inv); // 00D7A258 (double 0)
    const float hz = static_cast<float>(D(inv) * fz);
    // 00A8EFB7..00A8F082: look along it from the ground point below the camera.
    camera = read<void*>(owner, 0x30);
    refresh_unless_valid(camera, host);
    const float ground[3] = {read<float>(camera, 0x120), 0.0f, read<float>(camera, 0x128)};
    const float ahead[3] = {static_cast<float>(D(hx) + ground[0]),
                            static_cast<float>(D(hy) + ground[1]),
                            static_cast<float>(D(ground[2]) + hz)};
    CameraMatrix horizontal;
    host.look_at(horizontal, ground, ahead, 0.0f, 1.0f, 0.0f);
    // 00A8F087..00A8F0B4: results are overwritten by the products below.
    for (auto& corner : corners) bsp::cc13_audio::project_shadow_point_00a8ac90(corner, view_perspective.data());
    // 00A8F0B6..00A8F150
    CameraMatrix ortho;
    host.ortho_off_center(ortho, -1.0f, 1.0f, 0.0f, 1.0f, -100.0f, 100.0f); // 00D7A260, 00CE65D8, 00CE3D08
    const float origin[3] = {0.0f, 0.0f, 0.0f};
    const float down[3] = {0.0f, -1.0f, 0.0f};
    CameraMatrix top_down;
    host.look_at(top_down, origin, down, 0.0f, 0.0f, 1.0f);
    // 00A8F155..00A8F2B1
    const std::size_t slot = static_cast<std::size_t>(cascade) * 0x40;
    write_matrix(owner, 0x44 + slot, multiply(multiply(multiply(horizontal, perspective), top_down), ortho));
    write_matrix(owner, 0x144 + slot, multiply(read_matrix(owner, 0x44 + slot), read_matrix(owner, 0x244 + slot)));
    // 00A8F2B7..00A8F2F6
    void* const cascade_camera = read<void*>(owner, 0x20 + static_cast<std::size_t>(cascade) * 4);
    host.set_view_matrix(cascade_camera, horizontal);
    host.set_projection_matrix(read<void*>(owner, 0x20 + static_cast<std::size_t>(cascade) * 4),
        multiply(multiply(perspective, top_down), ortho));
    // 00A8F2FB..00A8F385: EBX is zero after the corner loop.
    write<std::uint8_t>(owner, 0x3a0 + static_cast<std::size_t>(cascade), 0);
    host.queue_named_command(read<void*>(owner, 0x34),
        read<void*>(owner, 0x20 + static_cast<std::size_t>(cascade) * 4),
        read<void*>(owner, 0x4f0 + static_cast<std::size_t>(cascade) * 4), "SHBU"); // 00D5B5D0
    host.set_last_command_metadata(kShadowCommandMetadata);
}

void fit_shadow_cascade_00a8bd20(void* owner, float near_distance, float far_distance,
    std::int32_t cascade, ShadowFitHost& host)
{
    const float n = near_distance, f = far_distance;
    const std::size_t c = static_cast<std::size_t>(cascade);
    // 00A8BD2B..00A8BEE3: slice corners C in camera space, as in 00A8ECB0.
    void* camera = read<void*>(owner, 0x30);
    const float fov_aspect = static_cast<float>(D(read<float>(camera, 0x1c8)) * read<float>(camera, 0x1c4));
    const float fov = read<float>(read<void*>(owner, 0x30), 0x1c4);
    const float tx = tangent(static_cast<float>(fov_aspect * 0.5)); // 00D7A280
    const float ty = tangent(static_cast<float>(fov * 0.5));
    const float xn = static_cast<float>(D(tx) * n);
    const float xf = static_cast<float>(D(tx) * f);
    const float yn = static_cast<float>(D(n) * ty);
    const float yf = static_cast<float>(D(f) * ty);
    const Vec3 corners[8] = {{-xn, -yn, n}, {xn, -yn, n}, {xn, yn, n}, {-xn, yn, n},
                             {-xf, -yf, f}, {xf, -yf, f}, {xf, yf, f}, {-xf, yf, f}};
    // 00A8BEC1..00A8BF4C: W = C through the camera world matrix (+F0h, REP MOVSD copy).
    camera = read<void*>(owner, 0x30);
    refresh_unless_valid(camera, host);
    const CameraMatrix world = read_matrix(camera, 0xf0);
    Vec3 points[8]; // W, later the shifted slice (00A8C636 overwrites the same slots)
    for (int i = 0; i < 8; ++i) transform_point_004142e0(corners[i], world, points[i]);
    // 00A8BF4E..00A8C10D: far-plane centre, near-plane centre, slice middle.
    auto quarter_sum = [&](int first, int second, int third, int fourth, int axis) {
        const float ab = static_cast<float>(D(points[first][axis]) + points[second][axis]);
        const float abc = static_cast<float>(D(ab) + points[third][axis]);
        const float abcd = static_cast<float>(D(abc) + points[fourth][axis]);
        return static_cast<float>(D(abcd) * 0.25); // 00D7A348
    };
    Vec3 far_centre, near_centre, middle;
    for (int axis = 0; axis < 3; ++axis) far_centre[axis] = quarter_sum(4, 5, 6, 7, axis);
    for (int axis = 0; axis < 3; ++axis) near_centre[axis] = quarter_sum(0, 1, 2, 3, axis);
    for (int axis = 0; axis < 3; ++axis) {
        const float sum = static_cast<float>(D(near_centre[axis]) + far_centre[axis]);
        middle[axis] = static_cast<float>(D(sum) * 0.5);
    }
    // 00A8C111..00A8C1D9: light look-at from middle + 10000*dir.
    const float lx = static_cast<float>(read<float>(owner, 0x38) * 10000.0); // 00CE4BD8
    const float ly = static_cast<float>(read<float>(owner, 0x3c) * 10000.0);
    const float lz = static_cast<float>(10000.0 * read<float>(owner, 0x40));
    const float light_eye[3] = {static_cast<float>(D(lx) + middle[0]),
                                static_cast<float>(D(ly) + middle[1]),
                                static_cast<float>(D(lz) + middle[2])};
    Vec3 up = pick_up(read<float>(owner, 0x38), read<float>(owner, 0x3c));
    CameraMatrix light_view;
    host.look_at(light_view, light_eye, middle.data(), up[0], up[1], up[2]);
    // 00A8C1DE..00A8C254: light-view depth range of W.
    float zmin = 1.0e10f, zmax = -1.0e10f; // 00CE4970, 00CE4ADC
    for (const Vec3& w : points) {
        Vec3 v;
        transform_point_004142e0(w, light_view, v);
        lower(zmin, v[2]);
        raise(zmax, v[2]);
    }
    // 00A8C256..00A8C46D: light view with depth remapped to [0, 1].
    CameraMatrix depth = identity();
    depth[10] = pc24(1.0 / D(pc24(D(zmax) - zmin)));
    depth[14] = pc24(D(zmin) / D(pc24(D(zmin) - zmax)));
    const CameraMatrix depth_view = multiply(light_view, depth);
    write_matrix(owner, 0x3ac + c * 0x40, depth_view);
    write(owner, 0x4ac + c * 0x10, depth_view[2]);
    write(owner, 0x4b0 + c * 0x10, depth_view[6]);
    write(owner, 0x4b4 + c * 0x10, depth_view[10]);
    write(owner, 0x4b8 + c * 0x10, depth_view[14]);
    host.set_depth_scale_pointer(read<void*>(owner, 0x20 + c * 4), field(owner, 0x4ac + c * 0x10));
    // 00A8C472..00A8C52B: sinGamma and the optimal near distance.
    const float to_light[3] = {-read<float>(owner, 0x38), -read<float>(owner, 0x3c),
                               -read<float>(owner, 0x40)}; // -0.0f SUBSS
    camera = read<void*>(owner, 0x30);
    refresh_unless_valid(camera, host);
    const float cos_gamma = pc24(D(pc24(D(pc24(D(read<float>(camera, 0x114)) * to_light[1]))
        + pc24(D(to_light[0]) * read<float>(camera, 0x110)))) + pc24(D(read<float>(camera, 0x118)) * to_light[2]));
    const D n_factor = read<float>(owner, 0x4ec); // FSTP64 copy
    const float sin_squared = pc24(1.0 - D(pc24(D(cos_gamma) * cos_gamma)));
    const float sin_gamma = static_cast<float>(host.crt_sqrt(sin_squared));
    const float n_opt = static_cast<float>(n_factor / sin_gamma);
    // 00A8C52F..00A8C58B: push the slice back when n_opt reaches far/(far-near).
    const float f_minus_n = pc24(D(f) - n);   // 00A8C541, kept on the stack for the shift
    const float q = pc24(D(f) / f_minus_n);
    float shift = 0.0f, near_shifted = n, far_shifted = f;
    if (n_opt >= q) { // FCOMI / JB: NaN keeps the slice
        shift = pc24(D(pc24(D(pc24(D(f_minus_n) * n_opt)) - f)) + D(1.0e-4f)); // 00D7A268
        far_shifted = static_cast<float>(D(f) + shift);
        near_shifted = static_cast<float>(D(shift) + n);
    }
    CameraMatrix translate = identity();
    translate[14] = shift;
    // 00A8C636..00A8C6B8: inlined affine transform of C by translate, with the
    // zero terms of the folded matrix still on the x87 stack. Every step but the
    // final + shift adds a signed zero to a float, which is exact, so the 24-bit
    // per-operation rounding only matters at that last add, done once here.
    for (int i = 0; i < 8; ++i) {
        const float x = corners[i][0], y = corners[i][1], z = corners[i][2];
        const D y0 = D(y) * 0.0, z0 = D(z) * 0.0, x0 = D(x) * 0.0;
        points[i][0] = static_cast<float>(((D(x) + y0) + z0) + 0.0);
        points[i][1] = static_cast<float>((z0 + (D(y) + x0)) + 0.0);
        points[i][2] = static_cast<float>((D(z) + (y0 + x0)) + shift);
    }
    // 00A8C6C0..00A8C7AD: perspective over the shifted near plane.
    CameraMatrix perspective = {};
    const D twice_near = D(near_shifted) + near_shifted;
    const float width = static_cast<float>(D(points[1][0]) - points[0][0]);
    perspective[0] = static_cast<float>(twice_near / width);
    const float height = static_cast<float>(D(points[2][1]) - points[1][1]);
    perspective[5] = static_cast<float>(twice_near / height);
    perspective[10] = pc24(D(far_shifted) / D(pc24(D(far_shifted) - near_shifted)));
    perspective[11] = 1.0f;
    perspective[14] = pc24(D(pc24(D(near_shifted) * far_shifted)) / D(pc24(D(near_shifted) - far_shifted)));
    const CameraMatrix shifted_perspective = multiply(translate, perspective);
    // 00A8C7B2..00A8C8DC: light direction in camera view space, normalized.
    const CameraMatrix view = host.view_matrix(read<void*>(owner, 0x30));
    const float dx = read<float>(owner, 0x38), dy = read<float>(owner, 0x3c), dz = read<float>(owner, 0x40);
    auto dot3 = [](float a0, float b0, float a1, float b1, float a2, float b2) {
        return pc24(D(pc24(D(pc24(D(a0) * b0)) + pc24(D(a1) * b1))) + pc24(D(a2) * b2));
    };
    const float vx = dot3(view[0], dx, view[4], dy, view[8], dz);
    const float vy = dot3(view[1], dx, view[5], dy, view[9], dz);
    const float vz = dot3(dx, view[2], dy, view[6], dz, view[10]);
    const float view_length = static_cast<float>(host.crt_sqrt(square_length_sum(vx, vy, vz)));
    const float view_inverse = view_length > 0.0f ? static_cast<float>(1.0 / view_length) : 0.0f;
    const Vec4 view_dir = {static_cast<float>(D(vx) * view_inverse), static_cast<float>(D(view_inverse) * vy),
                           static_cast<float>(D(view_inverse) * vz), 0.0f};
    // 00A8C8E0..00A8C9ED: post-perspective light point and the w < 0 flag.
    const Vec4 light_pps = transform4(view_dir, perspective);
    const bool behind = 0.0f > light_pps[3]; // COMISS / JBE
    write<std::uint8_t>(owner, 0x3a0 + c, behind ? 1 : 0);
    const float abs_w = std::fabs(light_pps[3]); // FABS, FSTP32
    CameraMatrix mirror = identity();
    const float inverse_w = static_cast<float>(1.0 / light_pps[3]);
    const float point[3] = {static_cast<float>(D(inverse_w) * light_pps[0]),
                            static_cast<float>(D(inverse_w) * light_pps[1]),
                            static_cast<float>(D(inverse_w) * light_pps[2])};
    CameraMatrix light_look, fit;
    if (abs_w <= 0.001f) { // 00D7A23C; FCOMIP / JB sends NaN to the other branch
        // 00A8C9F3..00A8CC03: look from the point towards (0, 0, 0.5).
        const float target[3] = {0.0f, 0.0f, 0.5f}; // 00CE3800
        up = pick_up(point[0], point[1]);
        host.look_at(light_look, point, target, up[0], up[1], up[2]);
        const CameraMatrix pps_light = multiply(shifted_perspective, light_look);
        // 00A8CC08..00A8CDFB: bounds of the projected slice corners.
        Vec3 lo = {1.0e10f, 1.0e10f, 1.0e10f}, hi = {-1.0e10f, -1.0e10f, -1.0e10f};
        for (const Vec3& corner : corners) {
            const Vec4 v = transform4({corner[0], corner[1], corner[2], 1.0f}, pps_light);
            const float inverse = static_cast<float>(1.0 / v[3]);
            const float p[3] = {static_cast<float>(D(v[0]) * inverse), static_cast<float>(D(v[1]) * inverse),
                                static_cast<float>(D(inverse) * v[2])};
            for (int axis = 2; axis >= 0; --axis) raise(hi[axis], p[axis]);
            for (int axis = 2; axis >= 0; --axis) lower(lo[axis], p[axis]);
        }
        // 00A8CE01..00A8CF40: ortho box with the depth range padded by 30 (00CE7630).
        host.ortho_off_center(fit, lo[0], hi[0], lo[1], hi[1],
            static_cast<float>(D(lo[2]) - 30.0), static_cast<float>(D(hi[2]) + 30.0));
    } else {
        // 00A8CFEE..00A8D227: bounds of the slice corners in post-perspective space.
        Vec3 pps[8];
        Vec3 lo = {1.0e10f, 1.0e10f, 1.0e10f}, hi = {-1.0e10f, -1.0e10f, -1.0e10f};
        for (int i = 0; i < 8; ++i) {
            const Vec4 v = transform4({corners[i][0], corners[i][1], corners[i][2], 1.0f}, shifted_perspective);
            const float inverse = static_cast<float>(1.0 / v[3]);
            pps[i] = {static_cast<float>(D(inverse) * v[0]), static_cast<float>(D(inverse) * v[1]),
                      static_cast<float>(D(inverse) * v[2])};
            for (int axis = 2; axis >= 0; --axis) lower(lo[axis], pps[i][axis]);
            for (int axis = 2; axis >= 0; --axis) raise(hi[axis], pps[i][axis]);
        }
        // 00A8D22D..00A8D277: box centre.
        float centre[3];
        for (int axis = 0; axis < 3; ++axis) {
            const float sum = static_cast<float>(D(hi[axis]) + lo[axis]);
            centre[axis] = static_cast<float>(D(sum) * 0.5);
        }
        // 00A8D27E..00A8D4A4: shifted slice axis (far centre - near centre), normalized.
        Vec3 axis_dir;
        for (int axis = 0; axis < 3; ++axis) {
            axis_dir[axis] = static_cast<float>(D(quarter_sum(4, 5, 6, 7, axis)) - quarter_sum(0, 1, 2, 3, axis));
        }
        const float axis_length = static_cast<float>(host.crt_sqrt(
            square_length_sum(axis_dir[0], axis_dir[1], axis_dir[2])));
        const float axis_inverse = axis_length > 0.0f ? static_cast<float>(1.0 / axis_length) : 0.0f;
        // 00A8D4C4: look from the light point at the box centre, slice axis up.
        host.look_at(light_look, point, centre, static_cast<float>(D(axis_dir[0]) * axis_inverse),
            static_cast<float>(D(axis_dir[1]) * axis_inverse), static_cast<float>(D(axis_inverse) * axis_dir[2]));
        // 00A8D5A8..00A8D802: bounds in that view, and the largest |x/z| and |y/z|.
        lo = {1.0e10f, 1.0e10f, 1.0e10f};
        hi = {-1.0e10f, -1.0e10f, -1.0e10f};
        float max_x_slope = -1.0e10f, max_y_slope = -1.0e10f;
        for (Vec3& p : pps) {
            const Vec4 v = transform4({p[0], p[1], p[2], 1.0f}, light_look);
            const float inverse = static_cast<float>(1.0 / v[3]);
            p = {static_cast<float>(D(v[0]) * inverse), static_cast<float>(D(v[1]) * inverse),
                 static_cast<float>(D(inverse) * v[2])};
            for (int axis = 2; axis >= 0; --axis) lower(lo[axis], p[axis]);
            for (int axis = 2; axis >= 0; --axis) raise(hi[axis], p[axis]);
            raise(max_x_slope, magnitude(static_cast<float>(D(p[0]) / p[2])));
            raise(max_y_slope, magnitude(static_cast<float>(D(p[1]) / p[2])));
        }
        const float scale_x = static_cast<float>(1.0 / max_x_slope);
        const float scale_y = static_cast<float>(1.0 / max_y_slope);
        // 00A8D808..00A8DA00: perspective with near/far from the z range.
        float far_z, near_z;
        if (behind) {
            const float limit = D(0.001f) > lo[2] ? 0.001f : lo[2]; // 00D7A318 / 00D7A23C
            near_z = -limit;
            far_z = lo[2];
        } else {
            const float scaled = static_cast<float>(D(lo[2]) * D(0.6f)); // 00CEFF98
            near_z = D(0.001f) > scaled ? 0.001f : scaled;
            far_z = hi[2];
        }
        fit = {};
        fit[0] = scale_x;
        fit[5] = scale_y;
        const float depth_scale = pc24(D(far_z) / D(pc24(D(far_z) - near_z)));
        fit[10] = depth_scale;
        fit[11] = 1.0f;
        fit[14] = static_cast<float>(D(-near_z) * depth_scale);
    }
    // 00A8CF46 / 00A8D859 / 00A8D8AA: mirror x unless the light point has w < 0.
    if (!behind) mirror[0] = -1.0f; // 00D7A260
    // 00A8DA09..00A8DCA1: cascade and shader matrices.
    const CameraMatrix fit_mirrored = multiply(fit, mirror);
    write_matrix(owner, 0x44 + c * 0x40, multiply(multiply(multiply(multiply(
        host.view_matrix(read<void*>(owner, 0x30)), translate), perspective), light_look), fit_mirrored));
    write_matrix(owner, 0x144 + c * 0x40, multiply(read_matrix(owner, 0x44 + c * 0x40), read_matrix(owner, 0x244 + c * 0x40)));
    write(owner, 0x14c + c * 0x40, read<float>(owner, 0x4ac + c * 0x10));
    write(owner, 0x15c + c * 0x40, read<float>(owner, 0x4b0 + c * 0x10));
    write(owner, 0x16c + c * 0x40, read<float>(owner, 0x4b4 + c * 0x10));
    write(owner, 0x17c + c * 0x40, read<float>(owner, 0x4b8 + c * 0x10));
    // 00A8DCA7..00A8DD15
    host.set_view_matrix(read<void*>(owner, 0x20 + c * 4), host.view_matrix(read<void*>(owner, 0x30)));
    host.set_projection_matrix(read<void*>(owner, 0x20 + c * 4),
        multiply(multiply(multiply(translate, perspective), light_look), fit_mirrored));
    write(read<void*>(owner, 0x20 + c * 4), 0x178, read<float>(owner, 0x08));
}

namespace {

// Checked walk of a native pointer vector whose descriptor sits at foliage+base
// (begin +4, end +8), as 00AD7360/00AD72A0 inline it.
template<class Visit>
void walk_checked(void* foliage, std::size_t base, FoliageRenderHost& host, Visit visit) {
    using Slot = void* const*;
    Slot cursor = read<Slot>(foliage, base + 4);
    if (cursor > read<Slot>(foliage, base + 8)) host.invalid_parameter();
    for (;;) {
        const Slot end = read<Slot>(foliage, base + 8);
        if (read<Slot>(foliage, base + 4) > end) host.invalid_parameter();
        if (cursor == end) return;
        if (cursor >= read<Slot>(foliage, base + 8)) host.invalid_parameter();
        visit(*cursor);
        if (cursor >= read<Slot>(foliage, base + 8)) host.invalid_parameter();
        ++cursor;
    }
}

} // namespace

void render_foliage_groups_00ad7360(void* foliage, void* camera, FoliageRenderHost& host)
{
    const auto pass = read<std::int32_t>(camera, 0x198);
    if (pass != 2 && pass != 7) {
        walk_checked(foliage, 0x64, host, [&](void* group) { host.render_bush_group(group, camera); });
    }
    walk_checked(foliage, 0x54, host, [&](void* entry) { host.clear_entry_nodes(entry); });
}

void debug_draw_foliage_00ad72a0(void* foliage, FoliageRenderHost& host)
{
    walk_checked(foliage, 0x64, host, [&](void* group) { host.debug_draw_bush_group(group); });
    walk_checked(foliage, 0x54, host, [&](void* entry) { host.debug_draw_entry_group(entry); });
}

void render_foliage_for_camera_00ad7a30(void* foliage, const void* foliage_global_00f8c210,
    void* camera, FoliageRenderHost& host)
{
    if (camera == nullptr) return;
    void* const effect = read<void*>(foliage, 0xa4);
    if (effect == nullptr) {
        write(foliage, 0xa4, host.load_effect("bush.mshd")); // 00D5D224
    } else {
        // FLD32/FSTP32 copies; the third value is FMUL64 0.5 (00D7A280) then a float spill.
        host.set_effect_float(effect, 0, read<float>(foliage_global_00f8c210, 0x08));
        host.set_effect_float(read<void*>(foliage, 0xa4), 6, read<float>(foliage_global_00f8c210, 0x08));
        host.set_effect_float(read<void*>(foliage, 0xa4), 2,
            static_cast<float>(D(read<float>(foliage_global_00f8c210, 0x08)) * 0.5));
        host.set_effect_byte_13c(read<void*>(foliage, 0xa4), 1);
    }
    if (read<std::uint8_t>(foliage, 0x06) == 0) {
        host.dispatch_entry_pass(foliage);
    } else {
        render_foliage_groups_00ad7360(foliage, camera, host);
    }
    if (read<std::uint8_t>(foliage, 0x05) != 0) debug_draw_foliage_00ad72a0(foliage, host);
}

void draw_node_box_00ae0a50(const void* node, std::uint32_t colour, float y_offset, FoliageQuadHost& host)
{
    // 00AE0A81..00AE0B27: each end spilled to float.
    const float x0 = static_cast<float>(D(read<float>(node, 0x00)) - read<float>(node, 0x0c));
    const float z0 = static_cast<float>(D(read<float>(node, 0x08)) - read<float>(node, 0x14));
    const float x1 = static_cast<float>(D(read<float>(node, 0x00)) + read<float>(node, 0x0c));
    const float z1 = static_cast<float>(D(read<float>(node, 0x08)) + read<float>(node, 0x14));
    const float ylo = static_cast<float>(D(read<float>(node, 0x04)) - read<float>(node, 0x10));
    const float yhi = static_cast<float>(D(read<float>(node, 0x04)) + read<float>(node, 0x10));
    const float y1 = static_cast<float>(D(yhi) + y_offset);
    const float y0 = static_cast<float>(D(y_offset) + ylo);
    auto line = [&](float ax, float ay, float az, float bx, float by, float bz) {
        const float a[3] = {ax, ay, az}, b[3] = {bx, by, bz};
        host.draw_line(a, b, colour, colour);
    };
    line(x0, y0, z0, x0, y1, z0);
    line(x0, y0, z1, x0, y1, z1);
    line(x1, y0, z0, x1, y1, z0);
    line(x1, y0, z1, x1, y1, z1);
    line(x0, y1, z0, x1, y1, z0);
    line(x1, y1, z0, x1, y1, z1);
    line(x1, y1, z1, x0, y1, z1);
    line(x0, y1, z1, x0, y1, z0);
    line(x0, y0, z0, x1, y0, z0);
    line(x1, y0, z0, x1, y0, z1);
    line(x1, y0, z1, x0, y0, z1);
    line(x0, y0, z1, x0, y0, z0);
}

void debug_draw_entry_node_00ae15e0(void* node, void* model, FoliageQuadHost& host)
{
    void* const transform = host.model_transform(model);
    if ((read<std::uint8_t>(transform, 0x5c) & 2u) == 0) host.refresh_world_matrix(transform);
    draw_node_box_00ae0a50(node, 0xffffffffu, 20.0f, host); // four 0FFh bytes; 00CE3930
    if (read<std::uint32_t>(node, 0x34) == 0) return;
    for (std::size_t slot = 0x1c; slot <= 0x28; slot += 4) {
        void* const child = read<void*>(node, slot);
        if (child != nullptr) debug_draw_entry_node_00ae15e0(child, model, host);
    }
}

void debug_draw_entry_group_00ae1670(void* entry, FoliageQuadHost& host)
{
    debug_draw_entry_node_00ae15e0(read<void*>(entry, 0x18), read<void*>(entry, 0x00), host);
}

namespace {

// Emission shared by the sum-6 (00AE4897) and leaf (00AE49FD) paths.
std::int32_t emit_foliage_section(void* cell, bool static_slot, void* source, void* stream,
    void* layout, void* geometry, void* material, std::int32_t count, std::uint32_t section10,
    std::uint32_t section18, FoliageQuadHost& host)
{
    const std::int32_t instances = read<std::int32_t>(cell, 0x4c) - read<std::int32_t>(cell, 0x48) + 1;
    void* const section = static_slot ? host.allocate_draw_section() : host.create_draw_section();
    host.set_vertex_layout(section, layout);
    write<std::uint32_t>(section, 0x08, 4);
    host.set_material(section, material);
    const std::uint32_t s20 = host.source_vslot_20(source);
    const std::uint32_t s24 = host.source_vslot_24(source);
    void* const memory = host.allocate_stream_slot();
    void* view = nullptr;
    if (memory != nullptr) {
        view = host.construct_stream_view(memory, s20, s24, read<void*>(source, 0x58), // 00B48D00
            static_cast<std::uint32_t>(read<std::int32_t>(cell, 0x48)) << 5);
    }
    write(cell, 0x54, view);
    // Native stores 80000000h to view+54h unconditionally; a null view faults there.
    if (view != nullptr) write<std::uint32_t>(view, 0x54, 0x80000000u);
    host.append_vertex_stream(section, stream);
    host.append_vertex_stream(section, read<void*>(cell, 0x54));
    write<std::uint32_t>(section, 0x18, section18);
    write<std::uint32_t>(section, 0x0c, 0);
    write<std::uint32_t>(section, 0x14, 0);
    write<std::uint32_t>(section, 0x10, section10);
    host.set_instance_count(section, instances);
    host.append_draw_section(geometry, section);
    host.release(section);
    if (void* const own = read<void*>(cell, 0x54)) {
        host.release(own);
        write<void*>(cell, 0x54, nullptr);
    }
    return instances + count;
}

} // namespace

std::int32_t cull_and_emit_foliage_cell_00ae47e0(void* cell, void* camera, void* model,
    void* source, void* stream, void* layout, void* geometry, void* material,
    std::int32_t count, std::uint32_t section10, std::uint32_t section18, FoliageQuadHost& host)
{
    std::int32_t sum = 0;
    for (std::int32_t i = host.frustum_plane_count(camera) - 1; i >= 0; --i) {
        const float* const p = host.frustum_plane(camera, i);
        const float plane[4] = {p[0], p[1], p[2], p[3]}; // FLD/FSTP copy to the stack
        const std::int32_t side = host.classify_plane(cell, plane, model);
        if (side == 0) return count;
        sum += side;
    }
    void* const transform = host.model_transform(model);
    if ((read<std::uint8_t>(transform, 0x5c) & 2u) == 0) host.refresh_world_matrix(transform);
    if (sum == 6) {
        return emit_foliage_section(cell, true, source, stream, layout, geometry, material, count,
            section10, section18, host);
    }
    if (read<std::uint32_t>(cell, 0x24) == 0) {
        return emit_foliage_section(cell, false, source, stream, layout, geometry, material, count,
            section10, section18, host);
    }
    for (std::size_t slot = 0x2c; slot <= 0x38; slot += 4) {
        if (void* const child = read<void*>(cell, slot)) {
            count = cull_and_emit_foliage_cell_00ae47e0(child, camera, model, source, stream, layout,
                geometry, material, count, section10, section18, host);
        }
    }
    return count;
}

void debug_draw_quad_node_00ae37f0(void* node, void* model, FoliageQuadHost& host)
{
    void* const transform = host.model_transform(model);
    if ((read<std::uint8_t>(transform, 0x5c) & 2u) == 0) host.refresh_world_matrix(transform);
    const float wx = read<float>(transform, 0x120), wz = read<float>(transform, 0x128);
    const float wy = 0.0f; // XORPS
    auto line = [&](float ax, float ay, float az, float bx, float by, float bz, std::uint32_t c) {
        const float a[3] = {ax, ay, az}, b[3] = {bx, by, bz};
        host.draw_line(a, b, c, c);
    };
    auto sum = [](float a, float b) { return static_cast<float>(D(a) + b); };
    using Slots = void* const*;
    const Slots begin = read<Slots>(node, 0x14);
    const bool leaf = read<std::uint32_t>(node, 0x24) == 0;
    if (leaf && (begin == nullptr || ((read<Slots>(node, 0x18) - begin) == 0))) {
        // 00AE3866..00AE39D2: rect at y 0, white.
        const float x0 = sum(wx, read<float>(node, 0x00)), x1 = sum(read<float>(node, 0x08), wx);
        const float z0 = sum(read<float>(node, 0x04), wz), z1 = sum(wz, read<float>(node, 0x0c));
        line(x0, wy, z0, x1, wy, z0, 0xffffffffu);
        line(x1, wy, z0, x1, wy, z1, 0xffffffffu);
        line(x1, wy, z1, x0, wy, z1, 0xffffffffu);
        line(x0, wy, z1, x0, wy, z0, 0xffffffffu);
    } else {
        // 00AE39D9..00AE3EE1: box, then the children.
        const std::uint32_t colour = read<std::uint8_t>(node, 0x28) != 0 ? 0xffff0000u : 0xff00ffffu;
        const float x0 = sum(read<float>(node, 0x00), wx), x1 = sum(read<float>(node, 0x08), wx);
        const float z0 = sum(read<float>(node, 0x04), wz), z1 = sum(wz, read<float>(node, 0x0c));
        const float y0 = sum(wy, read<float>(node, 0x40)), y1 = sum(read<float>(node, 0x44), wy);
        line(x0, y0, z0, x0, y1, z0, colour);
        line(x0, y0, z1, x0, y1, z1, colour);
        line(x1, y0, z0, x1, y1, z0, colour);
        line(x1, y0, z1, x1, y1, z1, colour);
        line(x0, y1, z0, x1, y1, z0, colour);
        line(x1, y1, z0, x1, y1, z1, colour);
        line(x1, y1, z1, x0, y1, z1, colour);
        line(x0, y1, z1, x0, y1, z0, colour);
        line(x0, y0, z0, x1, y0, z0, colour);
        line(x1, y0, z0, x1, y0, z1, colour);
        line(x1, y0, z1, x0, y0, z1, colour);
        line(x0, y0, z1, x0, y0, z0, colour);
        if (!leaf) {
            for (std::size_t slot = 0x2c; slot <= 0x38; slot += 4) {
                if (void* const child = read<void*>(node, slot)) debug_draw_quad_node_00ae37f0(child, model, host);
            }
        }
    }
    // 00AE3EE6..00AE4026: instance markers; size is reread every step.
    auto size = [&]() -> std::uint32_t {
        return static_cast<std::uint32_t>((read<Slots>(node, 0x18) - read<Slots>(node, 0x14)));
    };
    if (read<Slots>(node, 0x14) == nullptr || static_cast<std::int32_t>(size()) <= 0) return;
    for (std::uint32_t i = 0;; ++i) {
        if (read<Slots>(node, 0x14) == nullptr || i >= size()) return;
        auto check = [&] { if (read<Slots>(node, 0x14) == nullptr || i >= size()) host.invalid_parameter(); };
        check();
        const float tz = read<float>(read<Slots>(node, 0x14)[i], 0x08);
        check();
        const float top[3] = {sum(read<float>(read<Slots>(node, 0x14)[i], 0x00), wx), 20.0f, sum(tz, wz)};
        check();
        const float bz = read<float>(read<Slots>(node, 0x14)[i], 0x08);
        check();
        const float bottom[3] = {sum(read<float>(read<Slots>(node, 0x14)[i], 0x00), wx), 0.0f, sum(bz, wz)};
        host.draw_line(bottom, top, 0xffff0000u, 0xffff0000u);
    }
}

void debug_draw_bush_group_00ae4040(void* group, FoliageQuadHost& host)
{
    debug_draw_quad_node_00ae37f0(read<void*>(group, 0x10), read<void*>(group, 0x00), host);
}

void render_bush_group_00ae4cf0(void* group, void* camera, FoliageQuadHost& host)
{
    host.clear_draw_section_owners(read<void*>(group, 0x28));
    const float saved_far = read<float>(camera, 0x1d8);
    const float saved_near = read<float>(camera, 0x1d4);
    auto walk_mode = [&] {
        const auto mode = read<std::int32_t>(camera, 0x198);
        return mode != 2 && mode != 7;
    };
    if (walk_mode()) {
        host.set_far_plane(camera, 700.0f); // 00D1F980
        host.set_near_plane(camera, 0.01f); // 00D7A238
    }
    void* const header = read<void*>(group, 0x04);
    write(group, 0x30, cull_and_emit_foliage_cell_00ae47e0(read<void*>(group, 0x10), camera,
        read<void*>(group, 0x00), read<void*>(group, 0x1c), read<void*>(group, 0x20),
        read<void*>(group, 0x24), read<void*>(group, 0x28), read<void*>(group, 0x34), 0,
        read<std::uint32_t>(header, 0x0c), read<std::uint32_t>(header, 0x10), host));
    if (walk_mode()) {
        host.set_far_plane(camera, saved_far);
        host.set_near_plane(camera, saved_near);
    }
    if (read<std::uint8_t>(group, 0x38) == 0) host.clear_draw_section_owners(read<void*>(group, 0x28));
}

} // namespace bsp::cc13_shadow
