// Curve, Line, Scrollbar and Progbar widget kernels. Evidence and the
// native ABI of each routine are in docs/CC13_GUI_RENDER.md; the header lists
// the addresses. Names are hypotheses, not recovered symbols.
#include "bsp/gui_render.hpp"

#include <cmath>

namespace bsp {
namespace {

// 00419260: 1/|v|, or 0 when the length compares equal to zero
// (FUCOMIP/LAHF/TEST AH,44h/JP selection).
float reciprocal_length_or_zero(float x, float y) noexcept
{
    const float length = std::sqrt(x * x + y * y);
    return length == 0.0f ? 0.0f : 1.0f / length;
}

float to_float_unsigned(std::uint32_t value) noexcept
{
    // FILD of the dword plus 00CE3978 (2^32) when the sign bit is set.
    return static_cast<float>(static_cast<double>(value));
}

} // namespace

// ---------------------------------------------------------------------------
// Curve
// ---------------------------------------------------------------------------

std::vector<GuiVec2f> gui_curve_control_points_00abca10(
    const std::vector<GuiVec2f>& authored)
{
    std::vector<GuiVec2f> control;
    // 00ABCA2D..00ABCA3D empties +ECh; the build runs only when +110h > 1.
    if (authored.size() < 2) {
        return control;
    }
    control.reserve(authored.size() + 2);
    const GuiVec2f& p0 = authored[0];
    const GuiVec2f& p1 = authored[1];
    control.push_back({p0.x - (p1.x - p0.x), p0.y - (p1.y - p0.y)});
    control.insert(control.end(), authored.begin(), authored.end());
    const GuiVec2f& last = authored[authored.size() - 1];
    const GuiVec2f& before = authored[authored.size() - 2];
    control.push_back({(last.x - before.x) + last.x, last.y + (last.y - before.y)});
    return control;
}

GuiVec2f gui_curve_catmull_rom_00abced0(const GuiVec2f& p0, const GuiVec2f& p1,
    const GuiVec2f& p2, const GuiVec2f& p3, float t) noexcept
{
    // Coefficients from 00D7A2B0 (3.0), 00D7A328 (4.0), 00D7A370 (5.0),
    // 00D7A308 (2.0) and 00D7A280 (0.5), all doubles narrowed to float.
    auto lane = [t](float a, float b, float c, float d) {
        const float cubic = ((-a + b * 3.0f) - c * 3.0f) + d;
        const float quadratic = ((a * 2.0f - b * 5.0f) + c * 4.0f) - d;
        const float linear = -a + c;
        const float sum = ((b * 2.0f + linear * t) + (quadratic * t) * t)
            + ((cubic * t) * t) * t;
        return sum * 0.5f;
    };
    return {lane(p0.x, p1.x, p2.x, p3.x), lane(p0.y, p1.y, p2.y, p3.y)};
}

GuiCurveMesh gui_curve_tessellate_00abced0(const std::vector<GuiVec2f>& control,
    const std::vector<float>& widths, const GuiUvRect& uv, float stale_arc_weight)
{
    GuiCurveMesh mesh;
    const std::size_t count = control.size();
    // 00ABCF00: JL past everything unless +F0h > 3.
    if (count <= 3 || widths.size() + 2 < count) {
        return mesh;
    }
    const bool one_span = count == 4;
    const std::uint32_t samples = one_span ? kGuiCurveShortSamplesPerSpan : kGuiCurveSamplesPerSpan;
    const std::uint32_t cross = one_span ? kGuiCurveShortCrossVertices : kGuiCurveCrossVertices;
    const std::uint32_t spans = static_cast<std::uint32_t>(count - 3);
    mesh.samples_per_span = samples;
    mesh.cross_vertices = cross;
    mesh.rows = spans * samples + 1;

    // 00ABD095..00ABD118: one zig-zag strip per row pair, emitted as a list,
    // with the last two indices swapped on odd triangles.
    const std::uint32_t per_strip = cross * 2 - 2;
    mesh.indices.reserve(static_cast<std::size_t>(cross - 1) * (mesh.rows - 1) * 6);
    std::uint32_t base = 1;
    for (std::uint32_t strip = 0; strip + 1 < mesh.rows; ++strip) {
        std::uint32_t a = base - 1;
        std::uint32_t b = cross + base - 1;
        std::uint32_t c = base;
        for (std::uint32_t k = 0; k < per_strip; ++k) {
            mesh.indices.push_back(static_cast<std::uint16_t>(a));
            if (k & 1u) {
                mesh.indices.push_back(static_cast<std::uint16_t>(c));
                mesh.indices.push_back(static_cast<std::uint16_t>(b));
            } else {
                mesh.indices.push_back(static_cast<std::uint16_t>(b));
                mesh.indices.push_back(static_cast<std::uint16_t>(c));
            }
            const std::uint32_t next = b + 1;
            a = b;
            b = c;
            c = next;
        }
        mesh.triangle_count += per_strip;
        base += cross;
    }

    // 00ABD197..00ABD1C8: dt = 1 / samples.
    const float dt = 1.0f / to_float_unsigned(samples);
    float arc = 0.0f; // [ESP+1Ch]
    float weight = stale_arc_weight; // [ESP+18h]
    mesh.vertices.reserve(static_cast<std::size_t>(mesh.rows) * cross);

    for (std::uint32_t i = 1; i < count - 2; ++i) {
        const std::uint32_t span_samples = i == count - 3 ? samples + 1 : samples;
        // 00ABD1F7..00ABD250: the chord from control[0] to control[3],
        // recomputed for every span although it never changes.
        const float cx = control[0].x - control[3].x;
        const float cy = control[0].y - control[3].y;
        const float chord = std::sqrt(cx * cx + cy * cy);
        float t = 0.0f;
        for (std::uint32_t k = 0; k < span_samples; ++k) {
            if (samples == kGuiCurveShortSamplesPerSpan) {
                // 00ABD2D1..00ABD302, clamped below at zero.
                const float scaled = static_cast<float>(chord / kGuiCurveShortArcChordScale);
                weight = scaled * (kGuiCurveShortArcWeights[k] - 1.0f) + 1.0f;
                if (0.0f > weight) {
                    weight = 0.0f;
                }
            }
            const GuiVec2f here = gui_curve_catmull_rom_00abced0(
                control[i - 1], control[i], control[i + 1], control[i + 2], t);
            const float next_t = dt + t;
            const GuiVec2f ahead = gui_curve_catmull_rom_00abced0(
                control[i - 1], control[i], control[i + 1], control[i + 2], next_t);
            const float dx = ahead.x - here.x;
            const float dy = ahead.y - here.y;
            const float inverse = reciprocal_length_or_zero(dx, dy);
            const float ux = inverse * dx;
            const float uy = inverse * dy;
            const float width = widths[i - 1] + t * (widths[i] - widths[i - 1]);
            const float nx = width * -uy;
            const float ny = width * ux;
            const float last_cross = to_float_unsigned(cross - 1);
            for (std::uint32_t j = 0; j < cross; ++j) {
                const float s = to_float_unsigned(j) / last_cross;
                const float side = (s - 0.5f) * 2.0f;
                GuiQuadVertex vertex;
                vertex.x = side * nx + here.x;
                vertex.y = static_cast<float>((side * ny + here.y) * kGuiCurveYScale);
                vertex.z = kGuiCurveVertexZ;
                vertex.u = arc; // rescaled below
                vertex.v = s;
                mesh.vertices.push_back(vertex);
            }
            // 00ABDC0C..00ABDC7B: the step to the next sample is added unless
            // this is the span's last sample.
            if (k != span_samples - 1) {
                arc = std::sqrt(dx * dx + dy * dy) * weight + arc;
            }
            t = next_t;
        }
    }

    // 00ABDCB8..00ABDD41: U normalised by the total arc length and both
    // coordinates mapped into the atlas rectangle.
    mesh.arc_length = arc;
    for (GuiQuadVertex& vertex : mesh.vertices) {
        vertex.u = (uv.right - uv.left) * (vertex.u / arc) + uv.left;
        vertex.v = uv.top + vertex.v * (uv.bottom - uv.top);
    }
    return mesh;
}

// ---------------------------------------------------------------------------
// Line
// ---------------------------------------------------------------------------

GuiLineFlush gui_line_flush_00ac29d0(const std::vector<GuiVec2f>& pending,
    const GuiVec2f& pivot, std::uint32_t committed)
{
    GuiLineFlush flush;
    flush.first_vertex = committed;
    flush.committed_after = committed;
    const std::int32_t n = static_cast<std::int32_t>(pending.size());
    if (n == 0) {
        return flush; // 00AC29DE / 00AC29F4
    }
    flush.element_vertex_count = static_cast<std::uint32_t>(n);
    flush.element_primitive_count = static_cast<std::uint32_t>(n - 1);
    const std::int32_t appended = n - static_cast<std::int32_t>(committed);
    for (std::int32_t k = 0; k < appended; ++k) {
        const std::int32_t vertex_index = static_cast<std::int32_t>(committed) + k;
        if (vertex_index >= 1) {
            flush.indices.push_back(static_cast<std::uint16_t>(vertex_index - 1));
            flush.indices.push_back(static_cast<std::uint16_t>(vertex_index));
        }
        GuiQuadVertex vertex;
        vertex.x = pending[static_cast<std::size_t>(k)].x - pivot.x;
        vertex.y = pending[static_cast<std::size_t>(k)].y * kGuiLineYScale - pivot.y;
        flush.vertices.push_back(vertex);
    }
    flush.committed_after = static_cast<std::uint32_t>(n); // 00AC2C1B
    return flush;
}

std::int32_t gui_line_capacity_00ac2690(std::int32_t requested) noexcept
{
    return requested < 5 ? kGuiLineMinimumCapacity : requested;
}

// ---------------------------------------------------------------------------
// Scrollbar
// ---------------------------------------------------------------------------

void gui_scrollbar_set_value_00ac7d00(GuiScrollbarState& state, float value) noexcept
{
    if (value < state.minimum) {
        state.value = state.minimum;
    } else if (state.maximum < value) {
        state.value = state.maximum;
    } else {
        state.value = value;
    }
}

void gui_scrollbar_set_minimum_00ac7d80(GuiScrollbarState& state, float minimum) noexcept
{
    state.minimum = minimum;
    if (state.maximum < minimum) {
        state.maximum = minimum;
    }
    if (state.value < minimum) {
        gui_scrollbar_set_value_00ac7d00(state, minimum);
    }
}

void gui_scrollbar_set_maximum_00ac7de0(GuiScrollbarState& state, float maximum) noexcept
{
    state.maximum = maximum;
    if (maximum < state.minimum) {
        state.minimum = maximum;
    }
    if (maximum < state.value) {
        gui_scrollbar_set_value_00ac7d00(state, maximum);
    }
}

void gui_scrollbar_apply_properties_00ac85a0(GuiScrollbarState& state,
    bool alignment_is_vertical, float min_value, float max_value) noexcept
{
    state.horizontal = !alignment_is_vertical; // 00AC8634 / 00AC863C
    state.minimum = min_value;
    state.maximum = max_value;
    gui_scrollbar_set_minimum_00ac7d80(state, state.minimum); // 00AC86E7
    const float maximum = state.maximum;
    if (state.minimum > maximum) {
        state.minimum = maximum;
    }
    if (state.value > maximum) {
        gui_scrollbar_set_value_00ac7d00(state, maximum); // 00AC8718
    }
}

GuiScrollbarLayout gui_scrollbar_layout_00ac8a80(const GuiScrollbarState& state,
    const GuiVec2f& size, const GuiScrollbarChildren& children) noexcept
{
    GuiScrollbarLayout layout;
    auto along = [&](const GuiVec2f& v) { return state.horizontal ? v.x : v.y; };
    auto place = [&](float offset) {
        return state.horizontal ? GuiVec2f{offset, 0.0f} : GuiVec2f{0.0f, offset};
    };
    const double extent = along(size);
    const float back = along(children.back_arrow_size);
    layout.back_arrow = place(0.0f);
    layout.forward_arrow = place(static_cast<float>(extent - along(children.forward_arrow_size)));
    if (children.has_background) {
        layout.background = place(back);
    }
    if (children.has_slider) {
        float offset = back;
        if (!(state.maximum == state.minimum)) {
            const double doubled_back = static_cast<double>(back) + back;
            const double ratio = (state.value - state.minimum) / (state.maximum - state.minimum);
            const double travel = (extent - (along(children.slider_size) + doubled_back)) * ratio;
            offset = static_cast<float>(back + travel);
        }
        layout.slider = place(offset);
    }
    return layout;
}

// ---------------------------------------------------------------------------
// Progbar
// ---------------------------------------------------------------------------

bool gui_progbar_set_value_00ac7870(float width, float requested, float& current) noexcept
{
    const float stored = static_cast<float>((static_cast<double>(width) * requested) / width);
    if (stored == current) {
        return false;
    }
    current = stored;
    return true;
}

std::array<GuiQuadVertex, 4> gui_progbar_quad_00ac7930(const GuiUvRect& uv,
    float width, float height, float value) noexcept
{
    const float filled = width * value;
    const float bottom = kGuiProgbarYScale * height;
    const float u = uv.left + value * (uv.right - uv.left);
    std::array<GuiQuadVertex, 4> quad{};
    quad[0] = {0.0f, 0.0f, 0.0f, uv.left, uv.top};
    quad[1] = {filled, 0.0f, 0.0f, u, uv.top};
    quad[2] = {0.0f, bottom, 0.0f, uv.left, uv.bottom};
    quad[3] = {filled, bottom, 0.0f, u, uv.bottom};
    return quad;
}

// ---------------------------------------------------------------------------
// Icon delayed texture loads
// ---------------------------------------------------------------------------

bool gui_native_name_equal_00ab5f00(const std::string& a, const std::string& b) noexcept
{
    // The scans in 00AB5F00, 00AB51B0 and 00AB3840: lengths first; two empty
    // names compare equal; otherwise __stricmp, folded here as in the C locale.
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        unsigned char x = static_cast<unsigned char>(a[i]);
        unsigned char y = static_cast<unsigned char>(b[i]);
        if (x >= 'A' && x <= 'Z') {
            x = static_cast<unsigned char>(x - 'A' + 'a');
        }
        if (y >= 'A' && y <= 'Z') {
            y = static_cast<unsigned char>(y - 'A' + 'a');
        }
        if (x != y) {
            return false;
        }
    }
    return true;
}

bool GuiIconDelayedLoads::request_00ab5f00(const std::string& name, const void* owner,
    bool request_accepted)
{
    for (const Entry& entry : entries_) {
        if (gui_native_name_equal_00ab5f00(entry.name, name) && entry.owner == owner) {
            return false;
        }
    }
    if (!request_accepted) {
        return false;
    }
    entries_.push_back({name, owner});
    return true;
}

void GuiIconDelayedLoads::remove_at(std::size_t index)
{
    // Copy the last entry over the removed one, then drop the last.
    if (index != entries_.size() - 1) {
        entries_[index] = entries_.back();
    }
    entries_.pop_back();
}

std::vector<const void*> GuiIconDelayedLoads::deliver_00ab51b0(const std::string& name)
{
    std::vector<const void*> owners;
    std::size_t i = 0;
    while (i < entries_.size()) {
        if (gui_native_name_equal_00ab5f00(entries_[i].name, name)) {
            owners.push_back(entries_[i].owner);
            remove_at(i);
        } else {
            ++i;
        }
    }
    return owners;
}

void GuiIconDelayedLoads::cancel_owner_00ab50e0(const void* owner)
{
    std::size_t i = 0;
    while (i < entries_.size()) {
        if (entries_[i].owner == owner) {
            remove_at(i);
        } else {
            ++i;
        }
    }
}

} // namespace bsp
