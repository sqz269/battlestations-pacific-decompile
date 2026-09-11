// Reconstruction of the loading screen's element binding, hint rotation and
// render-worker callback. Addresses 0057C560, 0057C990, 0057C4C0, 0057C360 and
// 0057CA00; the text setter 00ABAED0 stays behind
// LoadingScreenElementHost::set_element_text because it belongs to the GUI text
// class this packet does not own.
//
// Evidence, calling conventions and the byte-checked constants are in
// docs/LOADING_SCREEN_ELEMENTS.md. Where the native body evaluates in x87
// extended precision and the C++ here does not, the comment says so.
#include "bsp/loading_screen_elements.hpp"

#include <cstdint>

namespace bsp {
namespace {

// FILD of a signed dword followed by the conditional 2^32 addend at 0057C9C9
// and 0057C433 is exactly an unsigned conversion.
double unsigned_to_double(std::int32_t value) noexcept {
    return static_cast<double>(static_cast<std::uint32_t>(value));
}

std::int32_t hint_count(const LoadingHintList& hints) noexcept {
    // 0057C9A7 and 0057C36F both test the vector's first pointer for null and
    // otherwise take (end - begin) >> 3 over 8-byte native strings.
    return static_cast<std::int32_t>(hints.keys.size());
}

} // namespace

float loading_hint_step_for_count(std::int32_t count) noexcept {
    if (count == 0) {
        // 0057C9D5: FLD qword [00CE4D70]; FSTP float, so 200.0 is narrowed once.
        return static_cast<float>(kLoadingHintStepWithoutHints);
    }
    // 0057C9CF: FLD1; FDIVRP. The native reciprocal is taken in x87 extended
    // precision and narrowed on store; the double here differs from that by at
    // most one ulp of the float result, and the value is only ever compared
    // against a progress delta.
    return static_cast<float>(1.0 / unsigned_to_double(count));
}

float loading_hint_step(const LoadingHintList& hints) noexcept {
    return loading_hint_step_for_count(hint_count(hints));
}

float loading_hint_hold_seconds(std::int32_t utf16_length) noexcept {
    // 0057C439: FMUL qword [00CEF290]. One sixteenth is a power of two, so the
    // product is exact for every length the resolver can report.
    return static_cast<float>(unsigned_to_double(utf16_length) * kLoadingHintSecondsPerCharacter);
}

LoadingPlateSize loading_plate_size(float caption_width, float plate_height) noexcept {
    // 0057CDD5..0057CDEA: FLD the caption width, FLD the 960.0 double, FDIV
    // ST(1),ST(0), then FSTP to a float local. The quotient is narrowed before
    // the comparison, so the ratio really is a float.
    const float ratio = static_cast<float>(static_cast<double>(caption_width)
        / kLoadingPlateReferenceWidth);
    // 0057CDF8: FCOMPI against the 4/15 double, JBE taking the short arm. NaN
    // compares unordered, which JBE also takes, and so does this test.
    if (static_cast<double>(ratio) > kLoadingPlatePadding) {
        // 0057CE00..0057CE13: the same quotient is recomputed and re-narrowed,
        // then the padding double is added and narrowed again.
        return LoadingPlateSize{
            static_cast<float>(static_cast<double>(ratio) + kLoadingPlatePadding),
            plate_height};
    }
    // 0057CE40: MOVSS of the 8/15 float, one padding unit of caption plus one
    // of padding, which is what the long arm produces at the threshold.
    return LoadingPlateSize{kLoadingPlateMinimumWidth, plate_height};
}

int loading_background_image_index(LoadingScreenMode mode, bool image_visible) noexcept {
    if (mode == LoadingScreenMode::MenuBackground) {
        return kMenuBackgroundImageIndex; // 0057CD3C pushes 2
    }
    // 0057CD93..0057CD9C: XOR EAX,EAX; CMP byte [00E08798],AL; SETNE AL. The
    // published visibility byte becomes the image index itself, not a flag.
    return image_visible ? 1 : 0;
}

float clock_sample_milliseconds(const ClockTimestamp& sample) noexcept {
    // 0057CAA7..0057CAC1 and 0057CB16..0057CB2F: FILD/FILD/FDIVP narrowed to a
    // float, then multiplied by the 1000.0 double and narrowed again.
    return static_cast<float>(static_cast<double>(timestamp_seconds_x87(sample))
        * kMillisecondsPerSecond);
}

float worker_tick_delta_seconds(float now_ms, float last_ms) noexcept {
    // 0057CACB: FSUB of two floats in extended precision, narrowed on store.
    // The exact difference of two floats always rounds the same way as the
    // float subtraction, so this needs no wider intermediate.
    const float delta = now_ms - last_ms;
    // 0057CAD5..0057CAF1: FLDZ, FCOMIP, JBE. The non-positive arm computes
    // -0.0f minus the delta, so a zero delta comes back as -0.0f and a NaN
    // delta takes the same arm as a negative one.
    const float magnitude = (delta > 0.0f) ? delta : (kNegativeZero - delta);
    // 0057CAFD: FDIVR against the 1000.0 double left on the x87 stack.
    return static_cast<float>(static_cast<double>(magnitude) / kMillisecondsPerSecond);
}

void bind_loading_screen_elements_0057c560(LoadingScreenElements& elements,
    LoadingScreenElementHost& host) {
    // 0057C566. The screen publishes itself in the front-end registry before it
    // owns any GUI object.
    host.register_front_end_screen(kLoadingScreenId);
    // 0057C5C2..0057C5D5: the GUI manager singleton, then the page factory with
    // both extra arguments zero. Stored at +0Ch, which is also the descriptor
    // 0057CED3 hands to the render worker.
    elements.page = host.create_gui_page(kLoadingPageName);
    // Every lookup below is 00AA7E00(&name, 1) with ECX = the page (0057C631
    // reloads it from +0Ch each time), so all eight widgets are searched for
    // recursively from the page root.
    elements.hint_text = host.find_element(elements.page, "hint_Text"); // +18h
    elements.radar_group = host.find_element(elements.page, "radar_Group"); // +10h
    elements.wave_icon = host.find_element(elements.page, "wave_Icon"); // +14h
    // 0057C609: the result of this one is discarded. The group is looked up and
    // dropped, so nothing in the screen ever refers to it again.
    static_cast<void>(host.find_element(elements.page, "bg_Group"));
    elements.title_logo_icon = host.find_element(elements.page, "titleLogo_Icon"); // +20h
    elements.frame_flag_icon = host.find_element(elements.page, "frameFlag_Icon"); // +1Ch
    elements.title_text = host.find_element(elements.page, "title_Text"); // +24h
    elements.logo_frame_box = host.find_element(elements.page, kLoadingPlateName); // +28h
    // The native body neither null-checks a lookup nor clears a field when one
    // fails; 00AA7E00 returning null simply leaves null in the field.
}

bool advance_loading_hint_0057c360(LoadingScreenElementState& screen,
    LoadingHintList& hints, LoadingScreenElementHost& host) {
    const std::int32_t count = hint_count(hints);
    // 0057C37E: signed CMP against count - 1. With an empty list this is
    // -1 < -1, so the rotation never starts and no clock sample is taken.
    if (hints.current >= count - 1) {
        return false;
    }
    ++hints.current; // 0057C386, the global cursor at 00E0877C
    // 0057C393..0057C3A9: clock vtable +20h into a local, then sixteen bytes
    // copied into +38h..+44h as four dwords.
    screen.rotation.shown_at = host.sample_clock();
    // 0057C3FA..0057C422 and again at 0057C447..0057C473: the native re-reads
    // the vector and range-checks the cursor before each use, trapping through
    // the CRT invalid-argument helper 00BF6713 when it is out of range. The
    // bound here is the container itself.
    const std::string& key = hints.keys[static_cast<std::size_t>(hints.current)];
    // 0057C422: the resolver reports the UTF-16 length of the localised string;
    // the buffer it also returns is released without being read.
    screen.rotation.hold_seconds
        = loading_hint_hold_seconds(host.resolve_localised_length(key));
    // 0057C46A: ECX = +18h, so the hint line, not the caption at +24h. The
    // trailing 1 routes the key through the localisation resolver.
    host.set_element_text(screen.elements.hint_text, key, true);
    return true;
}

void tick_loading_hint_0057c4c0(LoadingScreenElementState& screen,
    LoadingHintList& hints, LoadingScreenElementHost& host) {
    LoadingHintRotation& rotation = screen.rotation;
    // 0057C4C6..0057C4D5: the difference is formed in extended precision and
    // compared against the step without being narrowed, so the doubles here are
    // the faithful model rather than a float subtraction.
    const double travelled = static_cast<double>(rotation.progress)
        - static_cast<double>(rotation.advanced_at_progress);
    if (!(travelled > static_cast<double>(rotation.progress_step))) {
        return; // JBE at 0057C4D5
    }
    // 0057C4D7..0057C4F4: sample the clock, then subtract the timestamp stored
    // at +38h from it. ECX is the fresh sample and the stored one is the right
    // operand, so the result is the age of the current hint.
    const ClockTimestamp now = host.sample_clock();
    ClockTimestamp elapsed{};
    subtract_timestamp_00530890(elapsed, now, rotation.shown_at);
    const float seconds = timestamp_seconds_x87(elapsed); // 0057C4F9
    // 0057C50B: JBE, so the hold time is a strict lower bound the age must pass.
    if (!(seconds > rotation.hold_seconds)) {
        return;
    }
    advance_loading_hint_0057c360(screen, hints, host); // 0057C513
    // 0057C518: the mark is the progress that was read at the top, so a hint
    // that is held back by its display time does not lose the travelled steps.
    rotation.advanced_at_progress = rotation.progress;
}

void reset_loading_hints_0057c990(LoadingScreenElementState& screen,
    LoadingHintList& hints, LoadingScreenElementHost& host) {
    LoadingHintRotation& rotation = screen.rotation;
    rotation.progress = 0.0f; // 0057C99A
    rotation.progress_units = -1; // 0057C99F; not a clock sample
    rotation.advanced_at_progress = 0.0f; // 0057C9A2
    rotation.progress_step = loading_hint_step(hints); // 0057C9DB
    hints.current = -1; // 0057C9E0, the cursor at 00E0877C
    advance_loading_hint_0057c360(screen, hints, host); // 0057C9E6, shows hint 0
    // 0057C9F1 is a tail jump, not a call: the reset ends inside the tick, so a
    // list of one key still gets its single hint and then stops.
    tick_loading_hint_0057c4c0(screen, hints, host);
}

void loading_screen_worker_tick_0057ca00(LoadingScreenWorkerContext& context,
    LoadingHintList& hints, LoadingScreenElementHost& host) {
    // 0057CA1B..0057CA67: the callback builds the name in a pooled string and
    // resolves it inside its own context on every tick, even though 0057C560
    // already cached the same widget at screen+14h.
    void* wave_icon = host.find_element(context.radar_group, "wave_Icon");
    // 0057CAA5: one clock sample serves both the delta and the new mark.
    const ClockTimestamp sample = host.sample_clock();
    const float now_ms = clock_sample_milliseconds(sample);
    // 0057CB01..0057CB14: widget vtable +40h with the elapsed seconds. The
    // native dereferences the lookup result without a null check.
    host.update_element(wave_icon, worker_tick_delta_seconds(now_ms, context.last_sample_ms));
    // 0057CB16..0057CB35: the mark is recomputed from the same buffered sample
    // after the widget call, so it is the same value the delta was taken from.
    context.last_sample_ms = now_ms;
    // 0057CB1E: the singleton is null between screens, and only then is the
    // rotation skipped. The callback keeps running until the worker is stopped.
    if (context.screen != nullptr) {
        tick_loading_hint_0057c4c0(*context.screen, hints, host);
    }
    host.update_online_stats(); // 0057CB48, ECX = the game object at 00E188A8
}

} // namespace bsp
