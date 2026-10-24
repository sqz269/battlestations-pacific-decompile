#include "bsp/cc13_x87_model.hpp"
#include "bsp/sound_streaming_cc13.hpp"

#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/sound_dialog_logical.hpp"

#include <cstdio>
#include <cstring>

// Segment 74 sound routines. Evidence and ABIs: docs/CC13_SOUND_STREAMING.md.

namespace bsp::cc13_audio {
namespace {

template<class T> T read(const void* object, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(object) + offset, sizeof value);
    return value;
}
template<class T> void write(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(object) + offset, &value, sizeof value);
}
void* at(void* object, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(object) + offset;
}
const void* at(const void* object, std::size_t offset) noexcept {
    return static_cast<const unsigned char*>(object) + offset;
}

// An FLD m32 / FSTP m32 pair is exact except that a signalling NaN is quieted.
void x87_copy_float(void* destination, std::size_t to, const void* source, std::size_t from) noexcept {
    auto bits = read<std::uint32_t>(source, from);
    if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x007fffffu) != 0u) bits |= 0x00400000u;
    write(destination, to, bits);
}

void report_memory(FmodResult result, SoundSampleApplyHost& host) {
    if (static_cast<std::uint32_t>(result) == 0x2bu) host.on_out_of_sound_memory("Out of sounjd memory:");
}

SoundClassDescriptor& descriptor_at(SoundClassOwnership& classes, std::int32_t index) noexcept {
    // Native reads [table + index*4] and dereferences it without a null test.
    return *static_cast<SoundClassDescriptor*>(classes.manager().classes_98[static_cast<std::size_t>(index)]);
}

struct TemporaryName {
    NativeStringStorage& strings;
    NativeString value;
    TemporaryName(NativeStringStorage& storage, const char* text) : strings(storage) {
        value.assign_0041e870(strings, text);
    }
    ~TemporaryName() { destroy_native_string_header_0041dd20(&value, strings); }
    TemporaryName(const TemporaryName&) = delete;
    TemporaryName& operator=(const TemporaryName&) = delete;
};

const char* native_text(const void* actual_header) noexcept {
    // Native strings: +00 length, +04 data; null data reads the shared 00F8BBEE byte.
    const auto* data = read<const char*>(actual_header, 4);
    return data ? data : "";
}

} // namespace

void set_sound_class_duck_levels_00a7ac70(SoundClassOwnership& classes,
    std::uint32_t mask, float level) noexcept
{
    auto& slots = classes.manager().classes_98;
    std::uint32_t ordinal = 0;
    for (SoundClassLevel* slot : slots) {
        const std::uint32_t bit = sound_class_bit(ordinal);
        if ((mask & bit) != 0u) {
            auto& descriptor = *static_cast<SoundClassDescriptor*>(slot);
            // FUCOMIP + LAHF/TEST AH,44h/JNP: only an ordered equal compare
            // clears the pending bit; otherwise the new level is stored.
            if (descriptor.secondary_level_14 == level) mask ^= bit;
            else descriptor.secondary_level_14 = level;
        }
        ++ordinal;
    }
    if (mask != 0u) dirty_sound_classes_00a7a3f0(classes.manager(), mask);
}

void push_pop_sound_speech_duck_00a7d120(SoundClassOwnership& classes,
    std::int32_t& duck_depth_16c, bool push, SoundLevelNameHost& names,
    NativeStringStorage& strings)
{
    float level;
    if (push) {
        const std::int32_t previous = duck_depth_16c;
        duck_depth_16c = static_cast<std::int32_t>(static_cast<std::uint32_t>(previous) + 1u);
        if (previous != 0) return;
        level = 0.5f; // 00CE3800
    } else {
        duck_depth_16c = static_cast<std::int32_t>(static_cast<std::uint32_t>(duck_depth_16c) - 1u);
        if (duck_depth_16c != 0) return;
        level = 1.0f; // immediate 3F800000h
    }
    const TemporaryName warnings(strings, "Warnings");
    const TemporaryName speech(strings, "GUITestSpeech");
    const TemporaryName music(strings, "GUIMusic");
    const auto& manager = classes.manager();
    // Native lookup order: GUITestSpeech, Warnings, GUIMusic. -(1<<n) is the
    // native OR -1 / SHL form; a miss (-1) shifts by 31.
    const auto speech_index = static_cast<std::uint32_t>(find_sound_class_00a7acf0(manager, speech.value, names));
    const auto warnings_index = static_cast<std::uint32_t>(find_sound_class_00a7acf0(manager, warnings.value, names));
    const auto music_index = static_cast<std::uint32_t>(find_sound_class_00a7acf0(manager, music.value, names));
    const std::uint32_t mask = (0xffffffffu << (speech_index & 31u)) - sound_class_bit(warnings_index)
        - sound_class_bit(music_index) + 0xffffu;
    set_sound_class_duck_levels_00a7ac70(classes, mask, level);
}

void set_sound_class_paused_00a7f890(SoundClassOwnership& classes,
    std::int32_t index, std::uint8_t paused)
{
    const auto count = [&] { return static_cast<std::int32_t>(classes.manager().classes_98.size()); };
    if (index >= count()) classes.resize_00a7c2c0(index + 1);
    if (descriptor_at(classes, index).flags_0c == paused) return;
    if (index >= count()) classes.resize_00a7c2c0(index + 1);
    descriptor_at(classes, index).flags_0c = paused;
}

void set_sound_listener_environment_00a7b710(SoundConfigurationState& state,
    const char* name, SoundLevelNameHost& names)
{
    const std::int32_t found = find_sound_listener_00a7ae00(state, name, names);
    if (state.current_listener_10c == found) return;
    state.previous_listener_110 = state.current_listener_10c;
    state.current_listener_10c = found;
    select_sound_listener_00a7ae80(state, name, names);
}

void add_sound_class_00a7d9d0(SoundClassOwnership& classes,
    SoundClassDescriptor* by_value_reference)
{
    by_value_reference->class_index_08 = static_cast<std::uint32_t>(classes.manager().classes_98.size());
    classes.append_retained(*by_value_reference);
    by_value_reference->release(); // parameter destructor, EH state -1
}

void set_sound_stream_paused_00a85c40(void* stream, std::uint8_t paused) noexcept
{
    if (paused == read<std::uint8_t>(stream, 8)) return;
    // ADD EAX,-1 / CMP EAX,1 / JA: unsigned state-1 <= 1.
    if (read<std::uint32_t>(stream, 0x20) - 1u > 1u) return;
    write(stream, 8, paused);
}

void select_sound_stream_row_00a86710(void* stream, const void* actual_name_header) noexcept
{
    void* const table = read<void*>(stream, 0x3c);
    const std::int32_t selected = find_sound_dialog_record_00a865f0(table, actual_name_header);
    auto* row = static_cast<unsigned char*>(read<void*>(stream, 0x48));
    // The row count is reread from table+0Ch on every iteration.
    for (std::int32_t index = 0; index < read<std::int32_t>(table, 0xc); ++index, row += 12) {
        const bool chosen = index == selected;
        write(row, 0, chosen ? 1.0f : 0.0f);
        const double target = chosen ? 1.0 : 0.0;
        // FSUB/FDIV by the double 3.0 at 00D7A2B0, rounded once by FSTP m32.
        const float quotient = static_cast<float>((static_cast<double>(read<float>(row, 4)) - target) / 3.0);
        // FCOMIP ST0,0 / JBE: unordered or <= 0 takes the -0.0f - x branch.
        write(row, 8, quotient > 0.0f ? quotient : -0.0f - quotient);
    }
}

std::int32_t draw_sound_stream_debug_00a869b0(void* stream, std::int32_t x,
    std::int32_t y, SoundStreamDebugHost& host)
{
    static const char* const kStates[] = {"STOP", "STARTING", "PLAY", "OVER"}; // 00E12F00
    static const char* const kLayouts[] = {"undef", "mono", "stereo", "5.1"}; // 00E12F10
    if (read<std::int32_t>(stream, 0x20) != 2) return y;
    std::uint32_t ms = 0;
    if (static_cast<std::uint32_t>(host.channel_get_position_ms(read<void*>(stream, 0x44), &ms)) == 0x2bu)
        host.on_out_of_sound_memory("Out of sounjd memory:");
    const std::uint32_t saved = read<std::uint32_t>(stream, 0x18);
    const std::uint32_t position = saved > ms ? saved : ms; // CMP / JA, unsigned
    const float seconds = static_cast<float>(static_cast<double>(position) / 1000.0);
    char line[512];
    std::snprintf(line, sizeof line, "%s st:%s pos:%.2f", native_text(at(stream, 0x2c)),
        kStates[read<std::int32_t>(stream, 0x20)], static_cast<double>(seconds));
    host.draw_text(static_cast<float>(x), static_cast<float>(y), line);
    y += 12;
    const std::int32_t rows = read<std::int32_t>(stream, 0x4c);
    const float row_x = static_cast<float>(x + 32);
    for (std::int32_t index = 0; index < rows; ++index) {
        const void* record = static_cast<const unsigned char*>(read<void*>(read<void*>(stream, 0x3c), 8))
            + static_cast<std::uint32_t>(index) * 0x14u;
        const void* level = static_cast<const unsigned char*>(read<void*>(stream, 0x48))
            + static_cast<std::uint32_t>(index) * 12u;
        // Native indexes 00E12F10 without a bound; layouts beyond 3 read later data.
        std::snprintf(line, sizeof line, "%s(%s) vol:%.2f", native_text(record),
            kLayouts[read<std::int32_t>(record, 0x10)], static_cast<double>(read<float>(level, 4)));
        host.draw_text(row_x, static_cast<float>(y), line);
        y += 12;
    }
    return y;
}

void* copy_sound_sample_descriptor_00a82ae0(void* destination, const void* source,
    SoundSampleApplyHost& host)
{
    write(destination, 0, read<std::uint32_t>(source, 0));
    for (std::size_t offset : {std::size_t{4}, std::size_t{8}, std::size_t{0xc}, std::size_t{0x10}})
        x87_copy_float(destination, offset, source, offset);
    for (std::size_t offset = 0x14; offset <= 0x16; ++offset)
        write(destination, offset, read<std::uint8_t>(source, offset));
    x87_copy_float(destination, 0x18, source, 0x18);
    x87_copy_float(destination, 0x1c, source, 0x1c);
    for (std::size_t offset = 0x20; offset <= 0x22; ++offset)
        write(destination, offset, read<std::uint8_t>(source, offset));
    write(destination, 0x24, read<std::uint32_t>(source, 0x24));
    write(destination, 0x28, read<std::uint32_t>(source, 0x28));
    for (std::size_t offset : {std::size_t{0x2c}, std::size_t{0x30}, std::size_t{0x34}})
        x87_copy_float(destination, offset, source, offset);
    write(destination, 0x38, read<std::uint8_t>(source, 0x38));
    host.assign_parameters(at(destination, 0x3c), at(source, 0x3c));
    return destination;
}

std::string format_sound_sample_definition_00a83120(const void* sample,
    SoundSampleApplyHost& host)
{
    std::string text = read<std::int32_t>(sample, 8) == 1 ? "3D" : "2D"; // 00D5B0F0 / 00D5B0EC
    text += "\nVol ";
    text += host.format_float(read<float>(sample, 0xc));
    text += "\nPitch ";
    text += host.format_float(read<float>(sample, 0x10));
    text += read<std::uint8_t>(sample, 0x1d) ? "\nLoop" : "";
    text += "\nMinDistance ";
    text += host.format_float(read<float>(sample, 0x14));
    text += "\nMaxDistance ";
    text += host.format_float(read<float>(sample, 0x18));
    text += "\nDecompress ";
    text += host.format_int(read<std::uint8_t>(sample, 0x1c));
    text += read<std::uint8_t>(sample, 0x40) ? "\nAmbient" : "";
    text += read<std::uint8_t>(sample, 0x28) ? "\nLinear" : "";
    text += "\nFreqRndHz ";
    text += host.format_float(read<float>(sample, 0x38));
    text += "\nVolRnd ";
    text += host.format_float(read<float>(sample, 0x3c));
    if (read<std::uint8_t>(sample, 0x1e)) {
        text += "\nLoopPointsMillisec ";
        text += host.format_float(read<float>(sample, 0x20));
        text += " "; // 00CE3A90
        text += host.format_float(read<float>(sample, 0x24));
    }
    return text;
}

void apply_sound_sample_settings_00a83120(void* sample, const void* descriptor,
    SoundSampleApplyHost& host)
{
    copy_sound_sample_descriptor_00a82ae0(at(sample, 8), descriptor, host);
    auto sound = [&] { return read<void*>(read<void*>(sample, 0x78), 0x10); }; // reloaded per call
    report_memory(host.sound_set_3d_min_max_distance(sound(), read<float>(sample, 0x14),
        read<float>(sample, 0x18)), host);
    report_memory(host.sound_set_variations(sound(), read<float>(sample, 0x38),
        read<float>(sample, 0x3c), 0.0f), host);
    std::uint32_t mode = 0; // native local is not pre-initialized
    report_memory(host.sound_get_mode(sound(), &mode), host);
    mode = read<std::uint8_t>(sample, 0x28) ? (mode & 0xffefffffu) | 0x200000u
                                            : (mode & 0xffdfffffu) | 0x100000u;
    mode = read<std::uint8_t>(sample, 0x1c) ? (mode & 0xfffffdffu) | 0x100u
                                            : (mode & 0xfffffeffu) | 0x200u;
    report_memory(host.sound_set_mode(sound(), mode), host);
    const char* name = read<const char*>(sample, 0x54);
    // strstr(name, ".def") and then match - data != -1, which a hit always satisfies.
    if (name == nullptr || std::strstr(name, ".def") == nullptr) return;
    void* file = host.open_definition_file(at(sample, 0x50), 0x35);
    if (file == nullptr) return;
    host.write_and_delete_definition_file(file, format_sound_sample_definition_00a83120(sample, host));
}

void* sound_instance_default_handle_00a7be50() noexcept { return nullptr; }

float sound_instance_default_audibility_00a7be60() noexcept { return 1.0f; }

void* sound_instance_default_parameter_00a7bea0(void* out16, const void*) noexcept
{
    write<std::uint32_t>(out16, 0, 0u);
    write(out16, 4, 0.0f);
    write(out16, 8, 0.0f);
    write<std::uint32_t>(out16, 0xc, 0u);
    return out16;
}

void* sound_instance_default_vector_00a7bed0(void* out12) noexcept
{
    write(out12, 0, 0.0f);
    write(out12, 4, 0.0f);
    write(out12, 8, 0.0f);
    return out12;
}

bool sound_event_is_completed_00a89210(void* instance, SoundEventHost& host)
{
    if (read<std::uint8_t>(instance, 0x15) != 0) return true;
    void* const event = read<void*>(instance, 0x54);
    if (event == nullptr) return false;
    std::uint32_t state = 0;
    host.event_get_state(event, &state); // result ignored
    return ((~(state >> 3)) & 1u) != 0u;
}

void update_sound_event_00a89600(void* instance, float dt, SoundEventHost& host)
{
    if (read<std::uint8_t>(instance, 0x15) != 0) return;
    host.advance_fade(instance, dt);
    const float delay = read<float>(instance, 0x3c);
    if (delay > 0.0f) { // COMISS / JBE against 0.0f at 00D7A218
        const float remaining = delay - dt;
        write(instance, 0x3c, remaining);
        // FLDZ / FCOMIP / JB: only 0 < remaining keeps the hold; NaN clears nothing.
        if (!(0.0f < remaining) && !(remaining != remaining)) write<std::uint8_t>(instance, 0x38, 0);
    }
    if (read<void*>(instance, 0x54) == nullptr) host.create_and_start(instance);
    write(instance, 0x18, read<std::int32_t>(instance, 0x18) + 1);
    if (read<std::uint8_t>(instance, 0x14) != 0) {
        host.event_set_pitch(read<void*>(instance, 0x54), read<float>(instance, 0x20) - 1.0f, 0);
        host.event_set_volume(read<void*>(instance, 0x54), host.mixed_volume(instance));
        write<std::uint8_t>(instance, 0x14, 0);
    }
    host.update_parameters(instance);
    const bool paused = read<std::uint8_t>(instance, 0x1c) != 0
        || read<std::uint8_t>(read<void*>(instance, 0x44), 0xc) != 0;
    const auto hold = read<std::uint8_t>(instance, 0x38);
    host.event_set_paused(read<void*>(instance, 0x54), static_cast<std::uint8_t>(hold | (paused ? 1 : 0)));
}

void project_shadow_point_00a8ac90(float* inout_xyz, const void* actual_matrix_float16)
{
    const float source[4] = {inout_xyz[0], inout_xyz[1], inout_xyz[2], 1.0f}; // 1.0f from 00D7A24C
    float transformed[4];
    transform_native_vector4_00b62d10(source, transformed, actual_matrix_float16);
    // Double rounding through the 53-bit x87 is innocuous for one divide or
    // multiply of floats, so float arithmetic reproduces each FSTP.
    const float inverse_w = 1.0f / transformed[3];
    inout_xyz[0] = transformed[0] * inverse_w;
    inout_xyz[1] = transformed[1] * inverse_w;
    inout_xyz[2] = inverse_w * transformed[2];
}

void intersect_lines_2d_00a8b860(float* out, const float* p0, const float* p1,
    const float* q0, const float* q1) noexcept
{
    // cc13-fixture2 follow-up: the only caller (00A8B930 shadow trapezoid homography, itself
    // unreferenced) is render code, so the model follows the main thread's 24-bit precision
    // control: each FSUB/FMUL/FDIV rounds to 24 significand bits (cc13_x87_model.hpp).
    using namespace bsp::cc13x87;
    // 00A8B864..00A8B88C: det in one x87 expression, 1/det spilled to float.
    const double det = x87_sub24(x87_mul24(x87_sub24(p0[0], p1[0]), x87_sub24(q0[1], q1[1])),
                                 x87_mul24(x87_sub24(q0[0], q1[0]), x87_sub24(p0[1], p1[1])));
    const float inverse = static_cast<float>(x87_div24(1.0, det));
    out[1] = inverse;
    out[0] = inverse;
    // 00A8B899..00A8B8BF: reread after the stores; cross term spilled to float.
    const float p0x = p0[0], p1x = p1[0];
    const float p1y = p1[1], p0y = p0[1];
    const float cross_p = static_cast<float>(x87_sub24(x87_mul24(p0x, p1y), x87_mul24(p0y, p1x)));
    // 00A8B8C3..00A8B8E5
    const float q0x = q0[0], q1x = q1[0];
    const float cross_q = static_cast<float>(x87_sub24(x87_mul24(q0x, q1[1]), x87_mul24(q1x, q0[1])));
    // 00A8B8E9..00A8B909: x with the copied endpoints and the x87 inverse.
    out[0] = static_cast<float>(x87_mul24(
        x87_sub24(x87_mul24(x87_sub24(q0x, q1x), cross_p), x87_mul24(x87_sub24(p0x, p1x), cross_q)), inverse));
    // 00A8B90B..00A8B922: y rereads the inputs and out[1] after out[0].
    const double dqy = x87_sub24(q0[1], q1[1]);
    const double dpy = x87_sub24(p0[1], p1[1]);
    out[1] = static_cast<float>(x87_mul24(x87_sub24(x87_mul24(cross_p, dqy), x87_mul24(cross_q, dpy)), out[1]));
}

void submit_shadow_cascades_00a8ea20(void* owner, ShadowCascadeSubmitHost& host)
{
    write(owner, 0x390, 35.0f);   // 00CE4D90
    write(owner, 0x394, 100.0f);  // 00CE3D08
    write(owner, 0x398, 300.0f);  // 00CE3AE8
    write(owner, 0x39c, 1000.0f); // 00CE3804
    write(owner, 0x4ec, 1.5f);    // 00CE380C
    host.fit_cascade(owner, 5.0f, read<float>(owner, 0x390), 0); // near 00CE3850
    host.fit_cascade(owner, read<float>(owner, 0x390), read<float>(owner, 0x394), 1);
    host.fit_cascade(owner, read<float>(owner, 0x394), read<float>(owner, 0x398), 2);
    host.fit_cascade(owner, read<float>(owner, 0x398), read<float>(owner, 0x39c), 3);
    for (std::size_t cascade = 0; cascade < 4; ++cascade) {
        const std::size_t slot = 0x20 + cascade * 4;
        void* const render_camera = read<void*>(owner, 0x30);
        if ((read<std::uint8_t>(render_camera, 0x5c) & 2u) == 0u) host.refresh_world_matrix(render_camera);
        void* const cascade_camera = read<void*>(owner, slot);
        // COMISS -5.0, height / JBE: an unordered height clears the plane.
        if (-5.0f > read<float>(render_camera, 0x124)) {
            const float plane[4] = {0.0f, -1.0f, 0.0f, 2.0f}; // 00D7A260, 00CE3958
            host.set_extra_clip_plane(cascade_camera, plane);
        } else {
            host.clear_extra_clip_plane(cascade_camera);
        }
        void* const command = host.new_render_command(read<void*>(owner, 0x34),
            read<void*>(owner, slot), read<void*>(owner, 0x30), read<void*>(owner, slot + 0x4d0));
        const std::uint32_t metadata[3] = {0u, 0x62u, 0x1eu};
        host.set_metadata(command, metadata);
        host.set_diagnostic_name(command, "SHBU");
        host.append_shadow_command(command, 0u);
        host.submit_frame_job(command);
    }
    host.flush_frame_jobs();
}

namespace {
// Shared body of 00A8E860 and 00A8E180; `ground` selects the 00A8E860 tail.
void render_shadow_view(void* owner, const void* settings, std::uint32_t argument, void* camera,
    std::uint32_t a, std::uint32_t b, bool ground, ShadowSingleViewHost& host)
{
    if (read<std::uint8_t>(settings, 0xc) == 0 || read<std::uint8_t>(settings, 0xd) == 0) return;
    void* const previous = read<void*>(owner, 0x30);
    if (previous != camera) {
        write(owner, 0x30, camera);
        if (camera) host.retain(camera);
        if (previous) host.release(previous);
    }
    float viewport_matrix[16];
    host.build_viewport_matrix(owner, viewport_matrix, read<void*>(owner, 0x504));
    host.set_viewport(read<void*>(owner, 0x20), read<void*>(owner, 0x504));
    host.fit_single_view(owner, camera, a, b);
    host.multiply_into_shader_matrix(owner, static_cast<const float*>(at(owner, 0x44)), viewport_matrix);
    x87_copy_float(owner, 0x14c, owner, 0x4ac);
    x87_copy_float(owner, 0x15c, owner, 0x4b0);
    x87_copy_float(owner, 0x16c, owner, 0x4b4);
    x87_copy_float(owner, 0x17c, owner, 0x4b8);
    if (ground) {
        host.prepare_ground_casters(camera, 1.0f);
        host.queue_named_command(read<void*>(owner, 0x34), read<void*>(owner, 0x20),
            read<void*>(owner, 0x500), "SHBU"); // 00D5B5D0
    } else {
        host.queue_command(read<void*>(owner, 0x34), read<void*>(owner, 0x20),
            read<void*>(owner, 0x500), argument);
    }
    const std::uint32_t metadata[3] = {0u, 0x62u, 0x1eu};
    host.set_last_command_metadata(metadata);
    if (void* const retained = read<void*>(owner, 0x30)) {
        host.release(retained);
        write<void*>(owner, 0x30, nullptr);
    }
}
} // namespace

void render_shadow_ground_ortho_view_00a8e860(void* owner, const void* settings_00f8bbf0,
    void* camera, std::uint32_t a, std::uint32_t b, ShadowSingleViewHost& host)
{
    render_shadow_view(owner, settings_00f8bbf0, 0u, camera, a, b, true, host);
}

void render_shadow_single_view_00a8e180(void* owner, const void* settings_00f8bbf0,
    std::uint32_t argument, void* camera, std::uint32_t a, std::uint32_t b,
    ShadowSingleViewHost& host)
{
    render_shadow_view(owner, settings_00f8bbf0, argument, camera, a, b, false, host);
}

bool any_input_binding_forces_unit_scale_00a92050(const std::vector<InputActionBinding>& bindings) noexcept
{
    for (const auto& binding : bindings)
        if (binding.force_unit_scale) return true;
    return false;
}

bool input_actions_share_binding_00a92910(const std::vector<InputActionBinding>& first,
    const std::vector<InputActionBinding>& second) noexcept
{
    constexpr std::uint32_t kNone = 0xffffffffu;
    const std::size_t first_count = first.size();
    for (std::size_t i = 0; i < first_count; ++i) {
        const auto& a = first[i];
        if (a.input_code == kNone || static_cast<std::uint32_t>(a.device_class) == kNone) continue;
        for (std::size_t j = 0; j < second.size(); ++j) {
            const auto& b = second[j];
            if (b.input_code != kNone && static_cast<std::uint32_t>(b.device_class) != kNone
                && a.device_class == b.device_class && a.input_code == b.input_code)
                return true;
        }
    }
    return false;
}

GuiSoundSetState construct_gui_sound_set_00940dd0() noexcept
{
    return GuiSoundSetState{};
}

void init_gui_sound_set_00940ec0(GuiSoundSetState& set, GuiSoundSetHost& host)
{
    set.sound_class = host.find_sound_class(kGuiSoundClassName); // +24h
    // 00940F3B..00941060: assign each acquired sample to its RefPtr slot; the previous
    // reference is released after the new one is held.
    for (int i = 0; i < 5; ++i) {
        void* sample = host.acquire_sample(kGuiSoundSampleNames[i]);
        void* previous = set.samples[i];
        if (previous != sample) {
            set.samples[i] = sample;
            if (previous != nullptr) host.release_sample(previous);
        } else if (sample != nullptr) {
            host.release_sample(sample); // the temporary's reference
        }
    }
}

void play_gui_sound_00941070(const GuiSoundSetState& set, std::int32_t slot,
    GuiSoundSetHost& host)
{
    if (slot == 5) return; // 0094108E
    host.play_tracked_sound(set.samples[slot], set.sound_class, kGuiSoundTypeName, true);
}
} // namespace bsp::cc13_audio
