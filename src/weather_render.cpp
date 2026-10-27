// Weather and sky effects of segment 94. Evidence: docs/CC13_WEATHER_RENDER.md.
// Each function names the native entry it reconstructs; register-level ABI is
// not preserved (see the header). Every FSTP dword spill is an f32() below.

#include "bsp/weather_render.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <cmath>

namespace bsp::weather {
namespace {

inline float f32(double v) noexcept { return static_cast<float>(v); }

// FILD of a dword the code treats as unsigned (FADD 2^32 when negative, 00CE3978).
inline double as_unsigned(std::int32_t v) noexcept {
    return static_cast<double>(static_cast<std::uint32_t>(v));
}

constexpr double kDegToRad = 0.01745277777777778;  // 00D646E0
constexpr float kFlashLightRadius = 20000.0f;        // 00CE3CC0
constexpr double kLightHeight = 500.0;               // 00CE3840
constexpr float kDarkAge = -1000.0f;                 // 00D7A240
constexpr float kFlashHold = 1000.0f;                // 00CE3804

}  // namespace

// ---------------------------------------------------------------------------
// Lightning

void lightning_storm_init_00bc0d40(LightningStormState& s, WeatherMath& math) {
    // 00BC1102..00BC112E: after the six bolt meshes are loaded and hidden.
    s.timer = math.random_uniform(RandomStream::primary, 0.0f, s.max_strike_interval);
    s.light_active = false;
    s.flashes_left = 0;
    s.flash_age = kDarkAge;
}

bool lightning_storm_update_00bc0510(LightningStormState& s, WeatherMath& math, LightningHost& host,
                                     float delta, Vec3& thunder_position, float& shader_lifetime) {
    // 00BC0535..00BC0582. COMISS with JB: a NaN age is treated as dark.
    s.timer = f32(static_cast<double>(s.timer) - delta);
    if (s.flash_age >= 0.0f) {
        s.flash_age = f32(static_cast<double>(delta) + s.flash_age);
        shader_lifetime = f32(1.0 - (6.0 - as_unsigned(s.flashes_left)) * 0.14000000059604645);
    }

    bool started = false;
    if (s.timer <= 0.0f) {
        if (s.flashes_left == 0) {
            // A new strike. Stream 1 picks the bolt, stream 0 the bearing and range.
            s.bolt = math.random_u32(RandomStream::secondary) % 6u;
            const float angle = f32(static_cast<double>(math.random_uniform(
                                        RandomStream::primary, -80.0f, 80.0f)) * kDegToRad);
            const Vec3 forward = host.camera_forward();
            const Vec3 fh = math.normalize({forward.x, 0.0f, forward.z});
            const float c = f32(std::cos(static_cast<double>(angle)));
            const Vec3 along{f32(static_cast<double>(fh.x) * c), f32(static_cast<double>(fh.y) * c),
                             f32(static_cast<double>(c) * fh.z)};
            const Vec3 right = host.camera_projected_x_axis();
            const Vec3 rh = math.normalize({right.x, 0.0f, right.z});
            const float sn = f32(std::sin(static_cast<double>(angle)));
            const Vec3 across{f32(static_cast<double>(rh.x) * sn), f32(static_cast<double>(rh.y) * sn),
                              f32(static_cast<double>(rh.z) * sn)};
            const Vec3 dir = math.normalize({f32(static_cast<double>(across.x) + along.x),
                                             f32(static_cast<double>(across.y) + along.y),
                                             f32(static_cast<double>(across.z) + along.z)});

            // 00BC0713..00BC0784: range 500..2000, thunder source 200..1500.
            const float range = math.random_uniform(RandomStream::primary, 500.0f, 2000.0f);
            const float sound = f32((static_cast<double>(range) - 500.0) * 0.8666666666666667 + 200.0);
            const Vec3 offset{f32(static_cast<double>(dir.x) * range), f32(static_cast<double>(dir.y) * range),
                              f32(static_cast<double>(dir.z) * range)};
            const Vec3 cam = host.camera_position();
            host.set_bolt_position(s.bolt, {f32(static_cast<double>(cam.x) + offset.x),
                                            f32(static_cast<double>(cam.y) + offset.y),
                                            f32(static_cast<double>(cam.z) + offset.z)});
            const Vec3 cam2 = host.camera_position();
            const Vec3 to_sound{f32(static_cast<double>(dir.x) * sound), f32(static_cast<double>(dir.y) * sound),
                                f32(static_cast<double>(sound) * dir.z)};
            thunder_position = {f32(static_cast<double>(to_sound.x) + cam2.x),
                                f32(static_cast<double>(to_sound.y) + cam2.y),
                                f32(static_cast<double>(to_sound.z) + cam2.z)};
            host.set_bolt_visibility(s.bolt, 1.0f);

            s.flash_age = 0.0f;
            shader_lifetime = 0.0f;
            s.timer = kFlashHold;
            // 00BC0952..00BC0A7A: light 500 above the bolt base, diffuse scale 4.
            const Vec3 cam3 = host.camera_position();
            host.create_flash_light({f32(static_cast<double>(f32(static_cast<double>(cam3.x) + offset.x)) + 0.0),
                                     f32(static_cast<double>(f32(static_cast<double>(cam3.y) + offset.y)) + kLightHeight),
                                     f32(0.0 + f32(static_cast<double>(cam3.z) + offset.z))},
                                    kFlashLightRadius, 4.0f);
            s.light_active = true;
            // 00BC0A7F..00BC0AB6: FISTP qword with the control word forced to chop.
            const float flashes = math.random_uniform(RandomStream::primary, 1.0f, 5.0f);
            s.flashes_left = static_cast<std::int32_t>(static_cast<std::int64_t>(flashes));
            started = true;
        } else {
            // A repeat flash on the same bolt, dimmer by 0.5 per flash already shown.
            host.set_bolt_visibility(s.bolt, 1.0f);
            s.flash_age = 0.0f;
            shader_lifetime = 0.0f;
            s.timer = kFlashHold;
            const Vec3 base = host.bolt_position(s.bolt);
            const float scale = f32(4.0 - as_unsigned(6 - s.flashes_left) * 0.5);
            host.create_flash_light({f32(static_cast<double>(base.x) + 0.0),
                                     f32(static_cast<double>(base.y) + kLightHeight),
                                     f32(static_cast<double>(base.z) + 0.0)},
                                    kFlashLightRadius, scale);
            s.light_active = true;
            s.flashes_left -= 1;
        }
    }

    // 00BC0CAC..00BC0D23: the flash ends after flash_duration seconds.
    if (s.flash_age > s.flash_duration) {
        host.set_bolt_visibility(s.bolt, 0.0f);
        const float hi = s.flashes_left != 0 ? 0.07f : s.max_strike_interval;  // 00CED070
        s.timer = math.random_uniform(RandomStream::primary, 0.0f, hi);
        s.flash_age = kDarkAge;
        shader_lifetime = 0.0f;
        if (s.light_active) {
            host.release_flash_light();
        }
        s.light_active = false;
    }
    return started;
}

// ---------------------------------------------------------------------------
// Water drops

void waterdrops_set_texture_cells_00bc1990(WaterdropsSystem& w, std::uint32_t v3c, std::uint32_t v40) {
    // DIV of the two virtual results; the image divides by zero when +40h is 0.
    w.variants = v3c / v40;
    w.variant_step = f32(1.0 / static_cast<double>(w.variants));
}

void waterdrops_spawn_00bc1a20(WaterdropsSystem& w, WaterdropsBudget& budget, WeatherMath& math,
                               WaterdropsHost& host, const WaterdropsCamera& camera) {
    w.last_camera = camera.position;
    host.attach_and_hide();

    // 00BC1AB5..00BC1AE8: unsigned compare against the 3000-drop budget.
    if (static_cast<std::uint32_t>(w.requested) + budget.total > 3000u) {
        w.requested = static_cast<std::int32_t>(3000u - budget.total);
    }
    budget.total += static_cast<std::uint32_t>(w.requested);
    w.drops.resize(static_cast<std::size_t>(w.requested < 0 ? 0 : w.requested));

    constexpr RandomStream p = RandomStream::primary;
    std::uint32_t index = 1;
    for (Waterdrop& d : w.drops) {
        float x;
        float y;
        if (w.apply_at_bottom_only) {
            // 00BC1B52..00BC1C25: a disc segment rotated by the camera roll.
            const float a = math.random_uniform(p, -1.0f, 1.0f);
            const float b = math.random_uniform(p, 0.0f, 1.0f);
            const float along = f32(static_cast<double>(a) * 1.414199948310852);
            const float across = f32(1.414199948310852 * f32(1.0 - static_cast<double>(b) * b));
            float angle = f32(-static_cast<double>(math.asin_clamped(camera.matrix_01)));
            if (0.0f > camera.matrix_11) {
                angle = f32(3.140000104904175 - angle);
            }
            const float c1 = f32(std::cos(static_cast<double>(angle)));
            const float s1 = f32(std::sin(static_cast<double>(angle)));
            x = f32(static_cast<double>(c1) * along + static_cast<double>(s1) * across);
            const float s2 = f32(std::sin(static_cast<double>(angle)));
            const float c2 = f32(std::cos(static_cast<double>(angle)));
            y = f32(static_cast<double>(s2) * along - static_cast<double>(c2) * across);
        } else if (w.start_top_off_screen) {
            x = math.random_uniform(p, -1.0f, 1.0f);
            y = math.random_uniform(p, 1.0f, 1.1f);
        } else if (w.apply_at_top_only) {
            x = math.random_uniform(p, -1.0f, 1.0f);
            y = math.random_uniform(p, 0.5f, 0.8f);
        } else {
            x = math.random_uniform(p, -1.0f, 1.0f);
            y = math.random_uniform(p, -1.0f, 1.0f);
        }
        d.x = x;
        d.y = y;

        // 00BC1CD7..00BC1D84
        const float size = math.random_uniform(p, w.min_size, w.max_size);
        d.size = size;
        const float height = f32(static_cast<double>(size) * camera.aspect);
        d.height = height;
        const float u = math.random_uniform(p, 0.5f, 1.0f);
        const float sq = f32(static_cast<double>(f32(static_cast<double>(size) * size)) +
                             f32(static_cast<double>(height) * height));
        d.slide = f32(static_cast<double>(math.sqrt(sq)) * u);

        const float life = math.random_uniform(p, w.min_life, w.max_life);
        d.life_left = life;
        d.life_total = life;

        const float ox = f32(static_cast<double>(d.x) - 0.009999999776482582);
        const float oy = f32(static_cast<double>(d.y) - 0.009999999776482582);
        const float inv = math.reciprocal_length2(ox, oy);
        d.dir_x = f32(static_cast<double>(ox) * inv);
        d.dir_y = f32(static_cast<double>(inv) * oy);
        d.field_24 = 0.0f;
        d.field_28 = 0.0f;
        d.stick = 1.0f;

        // 00BC1E01..00BC1E6A: the first requested*StickyPercent/100 drops stick
        // (index is 1-based) and live twice as long.
        if (as_unsigned(w.requested) * w.sticky_percent / 100.0 > static_cast<double>(index)) {
            const float twice = f32(static_cast<double>(d.life_total) + d.life_total);
            d.stick = 1000.0f;
            d.life_total = twice;
            d.life_left = twice;
        }

        // 00BC1E6D..00BC1EAB: _ftol2 truncation, masked by cells-1, negatives to 0.
        const float pick = math.random_uniform(p, 0.0f, 1000.0f);
        const std::int32_t masked = static_cast<std::int32_t>(pick) & static_cast<std::int32_t>(w.variants - 1u);
        d.variant = static_cast<std::uint32_t>(masked > 0 ? masked : 0);
        ++index;
    }
    w.live = w.requested;
}

void waterdrops_update_00bc12f0(WaterdropsSystem& w, WeatherMath& math, WaterdropsHost& host,
                                const Vec3& camera_position, float delta) {
    // 00BC12F6..00BC1398: SUBSS from -0.0 negates the float step.
    const float dx = f32(static_cast<double>(camera_position.x) - w.last_camera.x);
    const float dy = f32(static_cast<double>(camera_position.y) - w.last_camera.y);
    const float dz = f32(static_cast<double>(camera_position.z) - w.last_camera.z);
    w.camera_step = {-dx, -dy, -dz};
    w.last_camera = camera_position;
    const Vec3 view_step = host.to_view_direction(w.camera_step);
    const Vec3 view_down = host.to_view_direction({0.0f, -1.0f, 0.0f});

    host.set_visibility(w.live != 0 ? 1.0f : 0.0f);
    w.live = 0;

    constexpr RandomStream p = RandomStream::primary;
    for (Waterdrop& d : w.drops) {
        const float left = f32(static_cast<double>(d.life_left) - delta);
        d.life_left = 0.0f > left ? 0.0f : left;
        if (d.life_left == 0.0f) {
            continue;
        }
        ++w.live;
        const float mx = f32(static_cast<double>(d.dir_x) + view_step.x);
        const float my = f32(static_cast<double>(d.dir_y) + view_step.y);
        const float stick = d.stick;

        if (stick <= 1.0f && !w.no_camera_move) {
            // 00BC14B4..00BC1664: jitter plus streaking along the camera motion.
            const float rx = math.random_uniform(p, -1.0f, 1.0f);
            const float ry = math.random_uniform(p, -1.0f, 1.0f);
            const float inv = math.reciprocal_length2(rx, ry);
            const float nx = f32(static_cast<double>(inv) * rx);
            const float ny = f32(static_cast<double>(inv) * ry);
            const float jitter = math.random_uniform(p, 0.001f, 0.005f);
            const float jx = f32(static_cast<double>(nx) * jitter);
            const float jy = f32(static_cast<double>(jitter) * ny);
            const float len1 = math.length(w.camera_step);
            const float px = f32(static_cast<double>(jx) * len1);
            const float py = f32(static_cast<double>(len1) * jy);
            const float qx = f32(static_cast<double>(px) * stick);
            const float qy = f32(static_cast<double>(stick) * py);
            const float x1 = f32(static_cast<double>(d.x) + qx);
            d.x = x1;
            const float y1 = f32(static_cast<double>(d.y) + qy);
            d.y = y1;
            const float len2 = math.length(w.camera_step);
            const float wx = f32(static_cast<double>(len2) * mx);
            const float wy = f32(static_cast<double>(len2) * my);
            const float zx = f32(static_cast<double>(wx) * d.slide);
            const float zy = f32(static_cast<double>(d.slide) * wy);
            const float tx = f32(static_cast<double>(zx) * delta);
            const float ty = f32(static_cast<double>(zy) * delta);
            d.x = f32(static_cast<double>(x1) + tx);
            d.y = f32(static_cast<double>(y1) + ty);
        }

        // 00BC1667..00BC16BA: stickiness decays 0.2 per second, floored at 0.
        const float decayed = f32(static_cast<double>(stick) - static_cast<double>(delta) * 0.20000000298023224);
        d.stick = decayed > 0.0f ? decayed : 0.0f;
        d.field_24 = 0.0f;
        d.field_28 = -1.0f;

        // 00BC16BF..00BC1817: gravity rotated by up to pi/5 either way.
        const float angle = math.random_uniform(p, -0.6283185482025146f, 0.6283185482025146f);
        const float c1 = f32(std::cos(static_cast<double>(angle)));
        const float s1 = f32(std::sin(static_cast<double>(angle)));
        const float gx = f32(static_cast<double>(c1) * view_down.x - static_cast<double>(s1) * view_down.y);
        const float s2 = f32(std::sin(static_cast<double>(angle)));
        const float c2 = f32(std::cos(static_cast<double>(angle)));
        const float gy = f32(static_cast<double>(c2) * view_down.y + static_cast<double>(s2) * view_down.x);
        const float free = f32(1.0 - d.stick);
        const float weight = 0.0f > free ? 0.0f : (free > 1.0f ? 1.0f : free);
        const float vx = f32(static_cast<double>(gx) * delta);
        const float vy = f32(static_cast<double>(gy) * delta);
        const float ax = f32(static_cast<double>(vx) * w.gravity);
        const float ay = f32(static_cast<double>(w.gravity) * vy);
        const float bx = f32(static_cast<double>(ax) * d.slide);
        const float by = f32(static_cast<double>(d.slide) * ay);
        const float cx = f32(static_cast<double>(bx) * weight);
        const float cy = f32(static_cast<double>(weight) * by);
        d.x = f32(static_cast<double>(cx) + d.x);
        d.y = f32(static_cast<double>(d.y) + cy);
    }
}

void waterdrops_release_budget_00bc1f30(const WaterdropsSystem& w, WaterdropsBudget& budget) noexcept {
    const std::int32_t left = static_cast<std::int32_t>(budget.total - static_cast<std::uint32_t>(w.requested));
    budget.total = left < 1 ? 0u : static_cast<std::uint32_t>(left);
}

// ---------------------------------------------------------------------------
// Particle cloud class

void cloud_read_spheres_00bc42e0(CloudClassGeometry& c, CloudSphereReader& reader) {
    std::uint32_t count = reader.read_u32();
    c.max_sphere_radius = 0.0f;
    while (count != 0) {
        CloudSphere s;
        s.center.x = reader.read_float();
        s.center.y = reader.read_float();
        s.center.z = reader.read_float();
        s.radius = reader.read_float();
        c.max_sphere_radius = s.radius < c.max_sphere_radius ? c.max_sphere_radius : s.radius;
        c.spheres.push_back(s);  // 00BC31E0 grows to max(2 * capacity, 1)
        --count;
    }
}

bool cloud_offset_within_00bc32f0(const Vec3& offset, float radius) noexcept {
    // |v| is v when v > 0, else -0.0 - v; any component beyond radius rejects.
    const auto mag = [](float v) { return v > 0.0f ? v : f32(-0.0 - static_cast<double>(v)); };
    if (mag(offset.x) > radius || mag(offset.y) > radius || mag(offset.z) > radius) {
        return false;
    }
    // 004193E0 squared length (x^2 and y^2 spilled, z^2 spilled, one final spill)
    // against the float-stored radius^2.
    const float length_sq = f32(static_cast<double>(f32(static_cast<double>(offset.x) * offset.x)) +
                                f32(static_cast<double>(offset.y) * offset.y) +
                                f32(static_cast<double>(offset.z) * offset.z));
    const float radius_sq = f32(static_cast<double>(radius) * radius);
    return !(radius_sq < length_sq);
}

void cloud_add_puff_00bc3ef0(CloudClassGeometry& c, WeatherMath& math, CloudPuffFactory& factory,
                             const Vec3& position, float angle, float size) {
    const float half = f32(static_cast<double>(size) * 0.5);
    for (const CloudPuff& existing : c.puffs) {
        const Vec3 offset{f32(static_cast<double>(position.x) - existing.position.x),
                          f32(static_cast<double>(position.y) - existing.position.y),
                          f32(static_cast<double>(position.z) - existing.position.z)};
        const float reach = f32(static_cast<double>(existing.size) * 0.44999998807907104 + half);
        if (cloud_offset_within_00bc32f0(offset, reach)) {
            return;
        }
    }
    const std::optional<CloudPuff> made =
        factory.create(static_cast<std::int32_t>(c.puffs.size()), position, angle, size);
    if (!made) {
        return;
    }
    c.puffs.push_back(*made);  // 00BC3180 grows to max(2 * capacity, 1)

    // 00BC3FD5..00BC4063: bounding radius = max(|position| + size).
    const float xx = f32(static_cast<double>(position.x) * position.x);
    const float yy = f32(static_cast<double>(position.y) * position.y);
    const float sum = f32(static_cast<double>(yy) + xx + f32(static_cast<double>(position.z) * position.z));
    const float reach = f32(static_cast<double>(math.sqrt(sum)) + size);
    c.bounding_radius = c.bounding_radius > reach ? c.bounding_radius : reach;

    // 00481C90 / 00481D40 keep the candidate unless it is strictly outside (FCOMIP/JBE,
    // so an unordered compare also keeps the candidate).
    const auto keep_min = [](float cur, float cand) { return !(cand > cur) ? cand : cur; };
    const auto keep_max = [](float cur, float cand) { return !(cur > cand) ? cand : cur; };
    c.bounds_min = {keep_min(c.bounds_min.x, f32(static_cast<double>(position.x) - size)),
                    keep_min(c.bounds_min.y, f32(static_cast<double>(position.y) - size)),
                    keep_min(c.bounds_min.z, f32(static_cast<double>(position.z) - size))};
    c.bounds_max = {keep_max(c.bounds_max.x, f32(static_cast<double>(position.x) + size)),
                    keep_max(c.bounds_max.y, f32(static_cast<double>(position.y) + size)),
                    keep_max(c.bounds_max.z, f32(static_cast<double>(position.z) + size))};
}

void cloud_scatter_puffs_00bc40f0(CloudClassGeometry& c, WeatherMath& math, CloudPuffFactory& factory,
                                  const Vec3& center, float rmin, float rmax, float count, float size) {
    constexpr RandomStream p = RandomStream::primary;
    const double limit = count;
    if (!(limit > 0.0)) {
        return;
    }
    std::uint32_t made = 0;
    do {
        // A cube-uniform direction (not sphere-uniform), normalised by hand.
        const float a = math.random_uniform(p, -100.0f, 100.0f);
        const float b = math.random_uniform(p, -100.0f, 100.0f);
        const float d = math.random_uniform(p, -100.0f, 100.0f);
        const float aa = f32(static_cast<double>(a) * a);
        const float bb = f32(static_cast<double>(b) * b);
        const float dd = f32(static_cast<double>(d) * d);
        const float len = math.sqrt(f32(static_cast<double>(aa) + bb + dd));
        const float inv = len > 0.0f ? f32(1.0 / static_cast<double>(len)) : 0.0f;
        const Vec3 dir{f32(static_cast<double>(inv) * a), f32(static_cast<double>(b) * inv),
                       f32(static_cast<double>(inv) * d)};
        const float r = math.random_uniform(p, rmin, rmax);
        const Vec3 at{f32(static_cast<double>(center.x) + f32(static_cast<double>(dir.x) * r)),
                      f32(static_cast<double>(f32(static_cast<double>(dir.y) * r)) + center.y),
                      f32(static_cast<double>(center.z) + f32(static_cast<double>(dir.z) * r))};
        const float spin = math.random_uniform(p, 0.0f, 6.2831854820251465f);
        cloud_add_puff_00bc3ef0(c, math, factory, at, spin, size);
        ++made;
    } while (limit > static_cast<double>(made));
}

void cloud_build_geometry_00bc47d0(CloudClassGeometry& c, WeatherMath& math, CloudPuffFactory& factory) {
    c.bounds_min = {1e10f, 1e10f, 1e10f};
    c.bounds_max = {-1e10f, -1e10f, -1e10f};
    c.bounding_radius = 0.0f;
    // Spheres are read here by 00BC4530 (cloud_read_spheres_00bc42e0).
    const float puff_size = f32(static_cast<double>(c.max_sphere_radius) / 2.700000047683716);
    const float per_sphere = f32(static_cast<double>(c.density) * 1000.0);
    for (std::size_t i = 0; i < c.spheres.size(); ++i) {
        const CloudSphere s = c.spheres[i];
        cloud_scatter_puffs_00bc40f0(c, math, factory, s.center, 0.0f, s.radius, per_sphere, puff_size);
    }
    // 00BC4937..00BC49FF
    c.center = {f32(static_cast<double>(f32(static_cast<double>(c.bounds_min.x) + c.bounds_max.x)) * 0.5),
                f32(static_cast<double>(f32(static_cast<double>(c.bounds_min.y) + c.bounds_max.y)) * 0.5),
                f32(static_cast<double>(f32(static_cast<double>(c.bounds_min.z) + c.bounds_max.z)) * 0.5)};
    c.extent = {f32(static_cast<double>(c.bounds_max.x) - c.center.x),
                f32(static_cast<double>(c.bounds_max.y) - c.center.y),
                f32(static_cast<double>(c.bounds_max.z) - c.center.z)};
    c.inv_extent = {f32(1.0 / c.extent.x), f32(1.0 / c.extent.y), f32(1.0 / c.extent.z)};
}

// ---------------------------------------------------------------------------
// Cloud instance

float cloud_instance_fade_00bbe8b0(float reach) noexcept {
    const float r = 0.0f > reach ? 0.0f : reach;
    const float t = f32((static_cast<double>(r) - 2000.0) / 600.0);
    if (0.0f > t) {
        return 0.0f;
    }
    return t > 1.0f ? 1.0f : t;
}

bool cloud_instance_home_out_of_range_00bbe8b0(float home_reach) noexcept {
    const float r = 0.0f > home_reach ? 0.0f : home_reach;
    const float u = f32(1.0 - (static_cast<double>(r) - 2000.0) * 0.0020000000949949026);
    if (0.0f > u) {
        return true;
    }
    return !(u > 1.0f) && u == 0.0f;
}

CloudInstanceStep cloud_instance_step_00bbe8b0(CloudInstanceState& s, WeatherMath& math,
                                               CloudInstanceHost& host, float delta) {
    const auto reach_from = [&](const Vec3& at) {
        const Vec3 cam = host.camera_position();
        const float len = math.length({f32(static_cast<double>(cam.x) - at.x),
                                       f32(static_cast<double>(cam.y) - at.y),
                                       f32(static_cast<double>(cam.z) - at.z)});
        return f32(static_cast<double>(len) - host.class_radius());
    };

    host.set_visibility(1.0f);
    const Vec3 pos = host.position();
    const float t = cloud_instance_fade_00bbe8b0(reach_from(pos));
    const float shown = f32(1.0 - t);
    host.set_visibility(f32(static_cast<double>(s.visibility_1c4) * shown));  // 00BBEA2A: +1C4, not the SetAlpha field

    if (shown == 0.0f) {
        if (math.length(s.scroll) == 0.0f) {
            return CloudInstanceStep::Hidden;
        }
        const Vec3 at = host.position();
        const Vec3 home{f32(static_cast<double>(at.x) - s.scroll.x), f32(static_cast<double>(at.y) - s.scroll.y),
                        f32(static_cast<double>(at.z) - s.scroll.z)};
        if (!cloud_instance_home_out_of_range_00bbe8b0(reach_from(home))) {
            return CloudInstanceStep::Hidden;
        }
        host.set_position(home);
        s.scroll = {};
        return CloudInstanceStep::Recycled;
    }

    if (host.wind_enabled() && s.affected_by_wind) {
        // 00BBEC06..00BBECA7: drift = direction * delta * speed * 30 (00CE7630).
        const Vec3 dir = host.wind_direction();
        const float ax = f32(static_cast<double>(dir.x) * delta);
        const float ay = f32(static_cast<double>(dir.y) * delta);
        const float az = f32(static_cast<double>(delta) * dir.z);
        const float speed = host.wind_speed();
        const float bx = f32(static_cast<double>(ax) * speed);
        const float by = f32(static_cast<double>(ay) * speed);
        const float bz = f32(static_cast<double>(speed) * az);
        const Vec3 step{f32(static_cast<double>(bx) * 30.0), f32(static_cast<double>(by) * 30.0),
                        f32(30.0 * static_cast<double>(bz))};
        s.scroll = {f32(static_cast<double>(step.x) + s.scroll.x), f32(static_cast<double>(s.scroll.y) + step.y),
                    f32(static_cast<double>(s.scroll.z) + step.z)};
        const Vec3 now = host.position();
        host.set_position({f32(static_cast<double>(now.x) + step.x), f32(static_cast<double>(now.y) + step.y),
                           f32(static_cast<double>(now.z) + step.z)});
    }
    return CloudInstanceStep::Visible;
}

// ---------------------------------------------------------------------------
// Sky colour gradients

// cc13-fixture2 follow-up: the callers (00BBF440 sky gradient, 00BBF590 fog colour) render on
// the main thread at 24-bit precision control: each FSUB/FMUL/FADD rounds to 24 significand
// bits before its FSTP, which differs from a float operation only for denormal results.
Rgba sky_lerp_00bbf280(const Rgba& a, const Rgba& b, float t) noexcept {
    const auto one = [t](float lo, float hi) {
        const float d = f32(cc13x87::x87_sub24(hi, lo));
        return f32(cc13x87::x87_add24(lo, f32(cc13x87::x87_mul24(d, t))));
    };
    return {one(a.r, b.r), one(a.g, b.g), one(a.b, b.b), one(a.a, b.a)};
}

Rgba sky_gradient_sample_00bbf440(const SkyGradient& strip, float t, bool wrap) noexcept {
    const float u = f32(static_cast<double>(strip.count) * t - 0.5);
    const std::int32_t lo = static_cast<std::int32_t>(u);  // CVTTSS2SI
    const float frac = f32(static_cast<double>(u) - lo);   // FISUB
    std::int32_t hi = lo + 1;
    if (wrap) {
        hi %= strip.count;  // IDIV: the image faults on an empty strip
    } else if (hi < 0) {
        hi = 0;
    } else if (hi > strip.count - 1) {
        hi = strip.count - 1;
    }
    const auto texel = [&](std::int32_t i) {
        const std::uint8_t* p = strip.texels + static_cast<std::ptrdiff_t>(i) * 4;
        return Rgba{f32(p[2] / 255.0), f32(p[1] / 255.0), f32(p[0] / 255.0), f32(p[3] / 255.0)};
    };
    return sky_lerp_00bbf280(texel(lo), texel(hi), frac);
}

Rgba sky_fog_colour_00bbf590(const SkyGradient& by_time, const SkyGradient& by_blend, float time_of_day,
                             float blend) noexcept {
    const Rgba c1 = sky_gradient_sample_00bbf440(by_time, time_of_day, true);
    const Rgba c2 = sky_gradient_sample_00bbf440(by_blend, blend, false);
    return sky_lerp_00bbf280(c1, c2, c1.a);
}

// ---------------------------------------------------------------------------
// Particle cloud lighting

namespace {

// The spill pattern shared by 00BC3430, 00BC3930 and 00BC3EF0: y^2 and x^2 each
// stored to float, their sum kept on the x87 stack, z^2 stored, one final spill.
inline float sum_sq_spilled(float x, float y, float z) noexcept {
    return f32(static_cast<double>(f32(static_cast<double>(y) * y)) + f32(static_cast<double>(x) * x) +
               f32(static_cast<double>(z) * z));
}

struct CloudSortEntry {
    CloudPuffShading shading;  // first, so the comparator can read sort_key
    CloudPuff puff;
};

}  // namespace

int cloud_puff_compare_00bc2fe0(const void* a, const void* b) noexcept {
    const double ka = static_cast<const CloudPuffShading*>(a)->sort_key;
    const double kb = static_cast<const CloudPuffShading*>(b)->sort_key;
    return static_cast<int>(kb - ka);  // _ftol2 truncation (00BF7420)
}

void cloud_sort_puffs_00bc3430(CloudClassGeometry& c, std::vector<CloudPuffShading>& shading,
                               WeatherMath& math, CloudLightView& view, const Vec3& toward) {
    const float len = math.length(toward);
    const float inv = len > 0.0f ? f32(1.0 / static_cast<double>(len)) : 0.0f;
    const float nx = f32(static_cast<double>(toward.x) * inv);
    const float ny = f32(static_cast<double>(toward.y) * inv);
    const float nz = f32(static_cast<double>(inv) * toward.z);
    const float radius = c.bounding_radius;
    const Vec3 eye{f32(static_cast<double>(nx) * radius), f32(static_cast<double>(ny) * radius),
                   f32(static_cast<double>(radius) * nz)};
    std::vector<CloudSortEntry> entries(c.puffs.size());
    for (std::size_t i = 0; i < c.puffs.size(); ++i) {
        const Vec3& p = c.puffs[i].position;
        const float dx = f32(static_cast<double>(eye.x) - p.x);
        const float dy = f32(static_cast<double>(eye.y) - p.y);
        const float dz = f32(static_cast<double>(eye.z) - p.z);
        shading[i].sort_key = math.sqrt(sum_sq_spilled(dx, dy, dz));  // 00BC7A90
        entries[i] = {shading[i], c.puffs[i]};
    }
    view.qsort(entries.data(), entries.size(), sizeof(CloudSortEntry), cloud_puff_compare_00bc2fe0);
    for (std::size_t i = 0; i < entries.size(); ++i) {
        shading[i] = entries[i].shading;
        c.puffs[i] = entries[i].puff;
    }
}

float cloud_puff_transmittance_00bc3730(const CloudClassGeometry& c, WeatherMath& math, CloudLightView& view,
                                        std::uint32_t index, float density) {
    const Vec3 vi = view.transform_point(c.puffs[index].position);
    const float ri = c.puffs[index].size;
    float through = 1.0f;
    for (std::uint32_t j = 0; j < index; ++j) {
        const Vec3 vj = view.transform_point(c.puffs[j].position);
        const float dx = f32(static_cast<double>(vi.x) - vj.x);
        const float dy = f32(static_cast<double>(vi.y) - vj.y);
        const float dist = math.sqrt(f32(static_cast<double>(f32(static_cast<double>(dy) * dy)) +
                                         f32(static_cast<double>(dx) * dx)));
        const float f = f32(static_cast<double>(dist) / (static_cast<double>(c.puffs[j].size) + ri) + density);
        const float clamped = 0.0f > f ? 0.0f : (f > 1.0f ? 1.0f : f);
        through = f32(static_cast<double>(clamped) * through);
    }
    return through;
}

void cloud_compute_lighting_00bc3930(CloudClassGeometry& c, std::vector<CloudPuffShading>& shading,
                                     WeatherMath& math, CloudLightView& view, std::int32_t light,
                                     const Vec3& dir, float density, float blur, float& mean) {
    mean = 0.0f;
    shading.resize(c.puffs.size());
    cloud_sort_puffs_00bc3430(c, shading, math, view,
                              {f32(-0.0 - dir.x), f32(-0.0 - dir.y), f32(-0.0 - dir.z)});

    // 00BC3986..00BC3A19: look-at from 1000 * dir towards the origin; the up axis
    // switches to +Z when the light is within about 26 degrees of vertical.
    const float dot = f32(static_cast<double>(dir.x) * 0.0 + dir.y + 0.0 * static_cast<double>(dir.z));
    const Vec3 up = dot > 0.8999999761581421 ? Vec3{0.0f, 0.0f, 1.0f} : Vec3{0.0f, 1.0f, 0.0f};
    view.build_look_at({f32(static_cast<double>(dir.x) * 1000.0), f32(static_cast<double>(dir.y) * 1000.0),
                        f32(1000.0 * static_cast<double>(dir.z))},
                       {0.0f, 0.0f, 0.0f}, up);

    const std::size_t n = c.puffs.size();
    for (std::size_t i = 0; i < n; ++i) {
        const float t = cloud_puff_transmittance_00bc3730(c, math, view, static_cast<std::uint32_t>(i), density);
        mean = f32(static_cast<double>(mean) + t);
        shading[i].light[light] = t;
        // 00BC7B10: FISTP under a chop control word, low byte kept.
        const float a = math.random_uniform(RandomStream::primary, 0.6f, 0.75f);
        shading[i].alpha = static_cast<std::uint8_t>(static_cast<std::int32_t>(static_cast<double>(a) * 255.0));
    }
    mean = f32(static_cast<double>(mean) / static_cast<double>(static_cast<std::int32_t>(n)));

    // 00BC3AD4..00BC3C70: blur over puffs within size_i * blur (the puff itself is
    // counted twice: once as the seed, once at distance 0).
    for (std::size_t i = 0; i < n; ++i) {
        float sum = shading[i].light[light];
        float count = 1.0f;
        const Vec3 pi = c.puffs[i].position;
        for (std::size_t j = 0; j < n; ++j) {
            const Vec3 pj = c.puffs[j].position;
            const float dist = math.sqrt(sum_sq_spilled(f32(static_cast<double>(pj.x) - pi.x),
                                                        f32(static_cast<double>(pj.y) - pi.y),
                                                        f32(static_cast<double>(pj.z) - pi.z)));
            if (static_cast<double>(c.puffs[i].size) * blur > dist) {
                sum = f32(static_cast<double>(shading[j].light[light]) + sum);
                count = f32(static_cast<double>(count) + 1.0);
            }
        }
        shading[i].shade[light] = f32(static_cast<double>(sum) / count);
    }

    // 00BC3C76..00BC3DCC: darken the lower half; runs on every call, so after the
    // light-1 call shade[0] carries the factor twice and shade[1] holds only g.
    const float lo = c.bounds_min.y;
    const float half = f32((static_cast<double>(c.bounds_max.y) - lo) * 0.5);
    const float mid = f32(static_cast<double>(lo) + half);
    for (std::size_t i = 0; i < n; ++i) {
        const float u = f32(static_cast<double>(c.puffs[i].position.y) - mid);
        const float below = 0.0f > u ? f32(-0.0 - u) : -0.0f;
        const float t = f32(static_cast<double>(below) / half);
        const float g = 0.0f > t ? 0.0f : (t > 1.0f ? 1.0f : t);
        shading[i].shade[0] = f32(static_cast<double>(shading[i].shade[0]) * (1.0 - g));
        shading[i].shade[1] = g;
    }
}

}  // namespace bsp::weather
