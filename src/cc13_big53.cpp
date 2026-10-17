// Segments 52/53, large or skipped bodies (packet cc13-big53). See include/bsp/cc13_big53.hpp and
// docs/CC13_BIG53.md. Marker: cc13_big53.

#include "bsp/cc13_big53.hpp"

#include <cmath>
#include <cstring>

namespace bsp::cc13_big53 {

namespace {

template <class T>
T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }
void* add(void* base, std::size_t off) noexcept { return static_cast<std::uint8_t*>(base) + off; }
float* fptr(void* base, std::size_t off) noexcept { return static_cast<float*>(add(base, off)); }

ArchiveArg key(const char* name) noexcept { return ArchiveArg{0u, reinterpret_cast<std::uintptr_t>(name)}; }
ArchiveArg arg(std::uint32_t kind, std::uintptr_t value) noexcept { return ArchiveArg{kind, value}; }
ArchiveArg arg(std::uint32_t kind, void* address) noexcept {
    return ArchiveArg{kind, reinterpret_cast<std::uintptr_t>(address)};
}

// The VC8 std::string temporary (1Ch bytes: proxy +0, buffer +4, size +14h, capacity +18h).
struct TempString {
    std::uint8_t bytes[0x1C];
};
void temp_string_init(TempString& s) noexcept {
    at<std::uint32_t>(s.bytes, 0x18) = 0xF;
    at<std::uint32_t>(s.bytes, 0x14) = 0;
    s.bytes[4] = 0;
}
// The inlined tidy: free the heap buffer when capacity >= 10h, then the empty-state stores.
void temp_string_destroy(Big53Host& host, TempString& s) {
    if (at<std::uint32_t>(s.bytes, 0x18) >= 0x10) host.free_00bf65ac(ptr(s.bytes, 4));
    temp_string_init(s);
}

// The inlined construct / 00718870 / destroy sequence each named lookup uses.
void* lookup_point_group(Big53Host& host, void* cls, const char* name, std::uint32_t length,
                         std::uint32_t index) {
    TempString s;
    temp_string_init(s);
    host.std_string_assign_00408720(s.bytes, name, length);
    void* node = host.model_lookup_point_group_00718870(ptr(cls, 0x50), s.bytes, index);
    temp_string_destroy(host, s);
    return node;
}

// size() of a std::vector with 12-byte elements whose header is at `vec` (first +4, last +8):
// 0 for a null first, else the signed (last - first) / 12 (the 2AAAAAABh multiply).
std::int32_t count12(void* vec) noexcept {
    const std::uintptr_t first = at<std::uintptr_t>(vec, 4);
    if (first == 0) return 0;
    return static_cast<std::int32_t>(at<std::uintptr_t>(vec, 8) - first) / 12;
}

// The inlined checked operator[]: 00BF6713 when first is null or index >= size (unsigned), then
// a fresh load of first.
float* checked_point(Big53Host& host, void* vec, std::uint32_t index) {
    if (at<std::uintptr_t>(vec, 4) == 0 ||
        static_cast<std::uint32_t>(count12(vec)) <= index) {
        host.invalid_parameter_00bf6713();
    }
    return reinterpret_cast<float*>(at<std::uintptr_t>(vec, 4) + index * 12u);
}

void copy3(float* dst, const float* src) noexcept {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}

void store_identity(float* m) noexcept {
    for (int i = 0; i < 16; ++i) m[i] = (i % 5 == 0) ? kOne_00d7a24c : 0.0f;
}

// One orrhullam bow frame (00830285..00830551 for node 1 / class+5B0, 0083055E..0083082E for
// node 2 / class+5F0). Every point read is its own checked access on node+44h. The x87
// FLD/FSUB/FSTP of float operands rounds once to float under either precision control.
void bind_bow_frame(Big53Host& host, void* node, float* m) {
    void* pts = add(node, 0x44);
    copy3(m + 12, checked_point(host, pts, 0));  // translation row = p0
    float p0[3];
    copy3(p0, checked_point(host, pts, 0));
    const float* p2 = checked_point(host, pts, 2);
    m[4] = p2[0] - p0[0];  // row +10h = p2 - p0
    m[5] = p2[1] - p0[1];
    m[6] = p2[2] - p0[2];
    copy3(p0, checked_point(host, pts, 0));
    const float* p1 = checked_point(host, pts, 1);
    float e[3] = {p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]};  // p1 - p0 (local +60h)
    copy3(p0, checked_point(host, pts, 0));
    p2 = checked_point(host, pts, 2);
    float d[3] = {p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]};
    float out[3];
    copy3(m + 8, host.vector3_cross_004f9b30(out, d, e));  // row +20h = (p2-p0) x (p1-p0)
    host.matrix_orthonormalize_rows_0085dc80(m);
}

// The farviz / explosion append (engine array {first, count, capacity}, 12-byte entries).
void append_point12(Big53Host& host, void* array, const float* p) {
    if (at<std::int32_t>(array, 4) == at<std::int32_t>(array, 8)) {
        std::int32_t capacity = at<std::int32_t>(array, 8) * 2;
        if (capacity <= 1) capacity = 1;
        host.points_array_reserve_0074d190(array, capacity);
    }
    float* slot = reinterpret_cast<float*>(at<std::uintptr_t>(array, 0) +
                                           static_cast<std::uintptr_t>(at<std::int32_t>(array, 4)) * 12u);
    if (slot != nullptr) copy3(slot, p);
    at<std::int32_t>(array, 4) += 1;
}

// Loops "farviz" (class+650h) and "explosion" (class+684h): index 1.. until the lookup misses;
// each appends the group's checked point 0.
void append_indexed_family12(Big53Host& host, void* cls, const char* name, std::uint32_t length,
                             std::size_t array_off) {
    for (std::uint32_t i = 1;; ++i) {
        void* node = lookup_point_group(host, cls, name, length, i);
        if (node == nullptr) return;
        float p[3];
        copy3(p, checked_point(host, add(node, 0x44), 0));
        append_point12(host, add(cls, array_off), p);
    }
}

}  // namespace

// 0082FE30, __fastcall(ECX ship class), plain RET, frame 128h below entry; SEH FuncInfo 00DC377C
// (13 states, all string-temporary cleanup). The x87 work is float subtraction and multiplication
// by 0.5 of float operands, so the precision control (main thread, 24-bit) does not change a
// result. Simplifications, all on dead or stale-stack values: the 0Ch-byte 004D1510 prototype and
// word +0 of the 10h-byte 0082E990 prototype are uninitialised stack in the image (zero here); the
// matrix 0082C560 receives is the last idle frame built, or stale stack when the idle loop built
// none (zero here); 0082C560 is reached only to shrink, so it never reads it.
void ship_class_bind_model_data_0082fe30(Big53Host& host, void* cls) {
    host.vehicle_class_bind_model_data_0095f500(cls);
    host.ship_class_append_model_type54_pointers_0082d700(cls);

    // 0082FE5D..0082FF31: deckline / bottomline -> 0082D040(bottom+44h, deck+44h, [class+71Ch]).
    {
        TempString deck;
        TempString bottom;
        temp_string_init(deck);
        host.std_string_assign_00408720(deck.bytes, kDecklineName, 8);
        temp_string_init(bottom);
        host.std_string_assign_00408720(bottom.bytes, kBottomlineName, 0xA);
        void* deck_node = host.model_find_last_point_group_00718000(ptr(cls, 0x50), deck.bytes, 0);
        void* bottom_node = host.model_find_last_point_group_00718000(ptr(cls, 0x50), bottom.bytes, 0);
        host.ship_build_buoyancy_elements_0082d040(cls, add(bottom_node, 0x44), add(deck_node, 0x44),
                                                   at<float>(cls, 0x71C));
        temp_string_destroy(host, bottom);
        temp_string_destroy(host, deck);
    }

    // 0082FF34..008300AB: debarkation points into the vector class+584h.
    void* debark_vec = add(cls, 0x584);
    void* debark = lookup_point_group(host, cls, kDebarkationName, 0xB, 0);
    if (debark != nullptr) {
        const std::int32_t count = count12(add(debark, 0x44));
        const std::uint8_t prototype[12] = {};
        host.vector12_resize_004d1510(debark_vec, static_cast<std::uint32_t>(count), prototype);
        for (std::int32_t i = 0; i < count; ++i) {
            float p[3];
            copy3(p, checked_point(host, add(debark, 0x44), static_cast<std::uint32_t>(i)));
            copy3(checked_point(host, debark_vec, static_cast<std::uint32_t>(i)), p);
        }
    } else {
        // 008309F1..00830DD2: twelve box corners from the half extents; n = -0.0 - v (SUBSS).
        const std::uint8_t prototype[12] = {};
        host.vector12_resize_004d1510(debark_vec, 12u, prototype);
        const float x = static_cast<float>(static_cast<double>(at<float>(cls, 0xA4)) * kHalf_00d7a280);
        const float y = static_cast<float>(static_cast<double>(at<float>(cls, 0xA8)) * kHalf_00d7a280);
        const float z = static_cast<float>(static_cast<double>(at<float>(cls, 0xA0)) * kHalf_00d7a280);
        const float nx = kNegZero_00d7a208 - x;
        const float ny = kNegZero_00d7a208 - y;
        const float nz = kNegZero_00d7a208 - z;
        const float corners[12][3] = {
            {nx, ny, nz}, {x, ny, nz}, {nx, y, nz}, {x, y, nz},
            {nx, ny, z},  {x, ny, z},  {nx, y, z},  {x, y, z},
            {nx, ny, 0.0f}, {x, ny, 0.0f}, {nx, y, 0.0f}, {x, y, 0.0f},
        };
        for (std::uint32_t i = 0; i < 12; ++i) copy3(checked_point(host, debark_vec, i), corners[i]);
    }

    // 008300B3..0083082E: the two orrhullam bow frames; both matrices are reset to identity
    // before either node is tested.
    void* hull1 = lookup_point_group(host, cls, kOrrhullamName, 9, 1);
    void* hull2 = lookup_point_group(host, cls, kOrrhullamName, 9, 2);
    store_identity(fptr(cls, 0x5B0));
    store_identity(fptr(cls, 0x5F0));
    if (hull1 != nullptr) bind_bow_frame(host, hull1, fptr(cls, 0x5B0));
    if (hull2 != nullptr) bind_bow_frame(host, hull2, fptr(cls, 0x5F0));

    // 00830833..0083093F: wave p0 -> +594h, p1 -> +5A0h. No null test on the node.
    void* wave = lookup_point_group(host, cls, kWaveName, 4, 0);
    copy3(fptr(cls, 0x594), checked_point(host, add(wave, 0x44), 0));
    copy3(fptr(cls, 0x5A0), checked_point(host, add(wave, 0x44), 1));

    // 00830924..00830DF5: shipcenter p0 -> +578h, or zero.
    void* center = lookup_point_group(host, cls, kShipcenterName, 0xA, 0);
    if (center != nullptr) {
        copy3(fptr(cls, 0x578), checked_point(host, add(center, 0x44), 0));
    } else {
        at<float>(cls, 0x57C) = 0.0f;
        at<float>(cls, 0x580) = 0.0f;
        at<float>(cls, 0x578) = 0.0f;
    }

    // 00830DEA..00830F27: wave_stern p0 -> +638h, p1 -> +644h, or zero.
    void* stern = lookup_point_group(host, cls, kWaveSternName, 0xA, 0);
    if (stern != nullptr) {
        copy3(fptr(cls, 0x638), checked_point(host, add(stern, 0x44), 0));
        copy3(fptr(cls, 0x644), checked_point(host, add(stern, 0x44), 1));
    } else {
        for (std::size_t off = 0x638; off <= 0x64C; off += 4) at<float>(cls, off) = 0.0f;
    }

    // 00830F22..0083104C: kemeny 1.. -> class+678h {first, count, capacity}, 10h-byte entries
    // {point, index}.
    for (std::uint32_t i = 1;; ++i) {
        void* node = lookup_point_group(host, cls, kKemenyName, 6, i);
        if (node == nullptr) break;
        float p[3];
        copy3(p, checked_point(host, add(node, 0x44), 0));
        void* array = add(cls, 0x678);
        if (at<std::int32_t>(array, 4) == at<std::int32_t>(array, 8)) {
            std::int32_t capacity = at<std::int32_t>(array, 8) * 2;
            if (capacity <= 1) capacity = 1;
            host.indexed_points_reserve_00829180(array, capacity);
        }
        std::uint8_t* slot = reinterpret_cast<std::uint8_t*>(
            at<std::uintptr_t>(array, 0) + (static_cast<std::uintptr_t>(at<std::int32_t>(array, 4)) << 4));
        if (slot != nullptr) {
            copy3(reinterpret_cast<float*>(slot), p);
            at<std::uint32_t>(slot, 0xC) = i;
        }
        at<std::int32_t>(array, 4) += 1;
    }

    // 00831051..0083129A: farviz -> class+650h, explosion -> class+684h.
    append_indexed_family12(host, cls, kFarvizName, 6, 0x650);
    append_indexed_family12(host, cls, kExplosionName, 9, 0x684);

    // 0083129F..008313E2: path 1.. while the group exists with at least two points: resize the
    // path list class+6CCh to i, then append every point of the group to its last element.
    std::int32_t paths = 0;
    for (std::uint32_t i = 1;; ++i) {
        void* node = lookup_point_group(host, cls, kPathName, 4, i);
        if (node == nullptr || at<std::uintptr_t>(node, 0x48) == 0 || count12(add(node, 0x44)) < 2) break;
        const std::uint8_t prototype[16] = {};
        void* list = add(cls, 0x6CC);
        host.path_vectors_resize_0082e990(list, i, prototype);
        // The checked --end() and dereference.
        const std::uintptr_t last = at<std::uintptr_t>(list, 8);
        if (at<std::uintptr_t>(list, 4) > last) host.invalid_parameter_00bf6713();
        const std::uintptr_t back = last - 0x10;
        if (back > at<std::uintptr_t>(list, 8) || back < at<std::uintptr_t>(list, 4)) {
            host.invalid_parameter_00bf6713();
        }
        if (back >= at<std::uintptr_t>(list, 8)) host.invalid_parameter_00bf6713();
        void* path = reinterpret_cast<void*>(back);
        for (std::int32_t j = 0; j < host.point_group_count_points_004fba10(node); ++j) {
            float tmp[3];
            host.vector12_push_back_004215d0(path, host.point_group_point_00484270(node, tmp,
                                                                                   static_cast<std::uint32_t>(j)));
        }
        paths += 1;
    }
    // 008313E7..00831423: no path at all erases [first, last) of the vector class+6DCh.
    if (paths == 0) {
        void* v = add(cls, 0x6DC);
        const std::uintptr_t last = at<std::uintptr_t>(cls, 0x6E4);
        if (at<std::uintptr_t>(cls, 0x6E0) > last) host.invalid_parameter_00bf6713();
        const std::uintptr_t first = at<std::uintptr_t>(v, 4);
        if (first > at<std::uintptr_t>(v, 8)) host.invalid_parameter_00bf6713();
        std::uint8_t result[8];
        host.vector_erase_range_0082f4e0(v, result, v, first, v, last);
    }

    // 00831425..008316FD: idle 1.. while the group exists with at least two points: a frame with
    // translation p0, row +20h = p2 - p0, row +10h = (p2 - p0) x (p1 - p0), orthonormalised and
    // appended to class+6ECh. The points after p0 come through 00484270.
    float m[16] = {};
    for (std::uint32_t i = 1;; ++i) {
        void* node = lookup_point_group(host, cls, kIdleName, 4, i);
        if (node == nullptr || at<std::uintptr_t>(node, 0x48) == 0 || count12(add(node, 0x44)) < 2) break;
        store_identity(m);
        copy3(m + 12, checked_point(host, add(node, 0x44), 0));
        float t[6][3];
        const float* a = host.point_group_point_00484270(node, t[0], 0);
        const float* b = host.point_group_point_00484270(node, t[1], 2);
        m[8] = b[0] - a[0];
        m[9] = b[1] - a[1];
        m[10] = b[2] - a[2];
        a = host.point_group_point_00484270(node, t[2], 0);
        b = host.point_group_point_00484270(node, t[3], 1);
        float e[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
        a = host.point_group_point_00484270(node, t[4], 0);
        b = host.point_group_point_00484270(node, t[5], 2);
        float d[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
        float out[3];
        copy3(m + 4, host.vector3_cross_004f9b30(out, d, e));
        host.matrix_orthonormalize_rows_0085dc80(m);
        host.matrices_push_back_0082ca40(add(cls, 0x6EC), m);
    }

    // 00831702..008317E4: reconcile the idle frames (class+6ECh, 40h-byte elements) with the idle
    // records (class+6FCh, 1Ch-byte elements) by shrinking the larger to the smaller count.
    void* frames = add(cls, 0x6EC);
    void* records = add(cls, 0x6FC);
    auto frame_count = [&]() -> std::uint32_t {
        const std::uintptr_t first = at<std::uintptr_t>(frames, 4);
        return first == 0 ? 0u
                          : static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uintptr_t>(frames, 8) - first) >> 6);
    };
    auto record_count = [&]() -> std::uint32_t {
        const std::uintptr_t first = at<std::uintptr_t>(records, 4);
        return first == 0 ? 0u
                          : static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uintptr_t>(records, 8) - first) / 28);
    };
    const std::uint32_t frames_now = frame_count();
    if (at<std::uintptr_t>(records, 4) != 0 && frames_now < record_count()) {
        const std::uint32_t target = frame_count();
        std::uint8_t prototype[0x1C];
        host.idle_record_construct_0082e1f0(prototype);
        host.idle_records_resize_0082f8e0(records, target, prototype);
        return;
    }
    const std::uint32_t records_now = record_count();
    if (at<std::uintptr_t>(frames, 4) == 0) return;
    if (records_now < frame_count()) {
        host.matrices_resize_0082c560(frames, record_count(), m);
    }
}

// 00863130, __thiscall(ECX gunnery AI, archive), RET 4. Section "_bot": 008FE660, close; a new
// 24h-byte visibility cache at +68h {+0 owner [ai+50h], +4/+8/+0Ch 0, +10h -1.0f, byte +20h 0}
// (the other bytes are left as allocated; a null allocation stores null); "thinkTime" as a float
// by address into +6Ch; section "fire": twelve bytes (kind 3) keyed by integer index (kind 1) into
// +70h+i; section "enable": twelve dwords (kind 1) into +80h+4i.
void unit_gunnery_ai_load_properties_00863130(Big53Host& host, void* ai, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    host.gun_bot_read_base_properties_008fe660(ai, archive);
    host.archive_close_section_vt08(archive);
    void* cache = host.operator_new_00bf681b(0x24);
    if (cache != nullptr) {
        at<void*>(cache, 0x0) = ptr(ai, 0x50);
        at<std::uint32_t>(cache, 0x4) = 0;
        at<std::uint32_t>(cache, 0x8) = 0;
        at<std::uint32_t>(cache, 0xC) = 0;
        at<std::uint8_t>(cache, 0x20) = 0;
        at<float>(cache, 0x10) = kMinusOne_00d7a260;
    }
    at<void*>(ai, 0x68) = cache;
    host.archive_read_field_vt10(archive, key(kThinkTimeKey), arg(2, add(ai, 0x6C)));
    host.archive_open_section_vt04(archive, key(kFireSectionKey));
    for (std::uint32_t i = 0; i < 12; ++i) {
        host.archive_read_field_vt10(archive, arg(1, static_cast<std::uintptr_t>(i)), arg(3, add(ai, 0x70 + i)));
    }
    host.archive_close_section_vt08(archive);
    host.archive_open_section_vt04(archive, key(kEnableSectionKey));
    for (std::uint32_t i = 0; i < 12; ++i) {
        host.archive_read_field_vt10(archive, arg(1, static_cast<std::uintptr_t>(i)), arg(1, add(ai, 0x80 + 4 * i)));
    }
    host.archive_close_section_vt08(archive);
}

// ===========================================================================================
// Seg 53 debug geometry. Main thread, 24-bit precision control: every x87 FADD/FSUB/FMUL/FDIV
// rounds to float. All operands here are floats (or the float-representable double constants
// 0.1 [00D7A3A0], pi [00CE3D28], 2pi [00CE3828], 1/32, 0.5, 4.0, 255.0), so each step is one
// correctly rounded float operation, written as a float expression or float(double product).
// FSIN/FCOS are not affected by precision control; they are modelled as float(std::sin(double))
// (the x87 transcendental may differ from the CRT in the last bit of the double result).
// ===========================================================================================

namespace {

inline constexpr double kTwoPi_00ce3828 = 6.2831854820251465;      // 401921FB60000000h
inline constexpr double kPi_00ce3d28 = 3.1415927410125732;         // 400921FB60000000h
inline constexpr double kOneThirtySecond_00cf6108 = 0.03125;       // 3FA0000000000000h
inline constexpr double kZero_00d7a258 = 0.0;                      // 00D7A258, double
inline constexpr double kEps_00ce3820 = 1e-10;                     // 3DDB7CDFD9D7BDBBh
inline constexpr double kEps_00d7a268 = 9.99999974737875163555e-05;  // 3F1A36E2E0000000h
inline constexpr double kTenth_00d7a3a0 = 0.10000000149011612;     // 3FB99999A0000000h
inline constexpr float kHundredth_00d7a238 = 0.01f;                // 3C23D70Ah
inline constexpr float kHalf_00ce3800 = 0.5f;                      // 3F000000h
inline constexpr double kFour_00d7a328 = 4.0;
inline constexpr double kScale255_00ce4b48 = 255.0;

float fsin(float a) { return static_cast<float>(std::sin(static_cast<double>(a))); }
float fcos(float a) { return static_cast<float>(std::cos(static_cast<double>(a))); }
// 00BF7030, the CRT sqrt on ST0: one rounding to float of a float input.
float fsqrt(float v) { return static_cast<float>(std::sqrt(static_cast<double>(v))); }

// FISTP under a chop control word of float(channel * 255.0); the low byte. An out-of-range
// product is the integer indefinite 80000000h (byte 0).
std::uint32_t colour_byte24(float channel) {
    const float scaled = static_cast<float>(static_cast<double>(channel) * kScale255_00ce4b48);
    const std::int32_t value = (scaled >= -2147483648.0f && scaled < 2147483648.0f)
                                   ? static_cast<std::int32_t>(scaled)
                                   : static_cast<std::int32_t>(0x80000000u);
    return static_cast<std::uint32_t>(value) & 0xFFu;
}
std::uint32_t argb24(float r, float g, float b, float a) {
    return (colour_byte24(a) << 24) | (colour_byte24(r) << 16) | (colour_byte24(g) << 8) | colour_byte24(b);
}

void line_one_colour(cc13_seg53::Seg53Host& draw, const float* p0, const float* p1, const float* rgba) {
    cc13_seg53::debug_draw_line_rgba_floats_0085f990(draw, p0, p1, rgba[0], rgba[1], rgba[2], rgba[3],
                                                     rgba[0], rgba[1], rgba[2], rgba[3]);
}

// Squared length as the image sums it: (y*y + x*x) + z*z, one float rounding per op.
float length_sq(const float* v) {
    const float xx = v[0] * v[0];
    const float yy = v[1] * v[1];
    const float xy = yy + xx;
    const float zz = v[2] * v[2];
    return xy + zz;
}

}  // namespace

// 0085C5D0. The start point (origin, 1) and the end point (origin + delta, 1) go through 00B62D10
// and are divided by w (iw = 1/w, then x*iw ...). c = |a|^2 - 1; without allow_inside_start a start
// with c <= 0 (or NaN) returns false (COMISS/JBE). v = b - a, A = |v|^2, B2 = 2(a.v),
// disc = B2*B2 - (A*4.0)*c; disc < 0 or NaN returns false (FCOMI/JC); else r = sqrt(disc) (00BF7030),
// *t_plus = (r - B2)/(A+A), *t_minus = (-B2 - r)/(A+A), true.
bool ellipsoid_intersect_segment_0085c5d0(Big53Host& host, const float* m, const float* origin,
                                          const float* delta, float* t_plus, float* t_minus,
                                          bool allow_inside_start) {
    float h[4] = {origin[0], origin[1], origin[2], kOne_00d7a24c};
    float out[4];
    const float* q = host.vector4_transform_00b62d10(h, out, m);
    float t[4] = {q[0], q[1], q[2], q[3]};
    const float iw = 1.0f / t[3];
    const float a[3] = {t[0] * iw, t[1] * iw, t[2] * iw};
    const float c = length_sq(a) - 1.0f;
    if (!allow_inside_start && !(c > 0.0f)) return false;

    float e[4] = {origin[0] + delta[0], delta[1] + origin[1], delta[2] + origin[2], kOne_00d7a24c};
    float out2[4];
    q = host.vector4_transform_00b62d10(e, out2, m);
    t[0] = q[0];
    t[1] = q[1];
    t[2] = q[2];
    t[3] = q[3];
    const float iw2 = 1.0f / t[3];
    const float b[3] = {t[0] * iw2, t[1] * iw2, t[2] * iw2};
    const float v[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
    const float big_a = length_sq(v);
    const float ax = a[0] * v[0];
    const float ay = v[1] * a[1];
    const float s1 = ay + ax;
    const float az = a[2] * v[2];
    const float dot = az + s1;
    const float b2 = dot + dot;
    const float b2sq = b2 * b2;
    const float a4 = static_cast<float>(static_cast<double>(big_a) * kFour_00d7a328);
    const float a4c = a4 * c;
    const float disc = b2sq - a4c;
    if (!(disc >= 0.0f)) return false;
    const float r = fsqrt(disc);
    const float a2 = big_a + big_a;
    *t_plus = (r - b2) / a2;
    *t_minus = (-b2 - r) / a2;
    return true;
}

// 0085FB70. Line from -> to in (c0, c1) through renderer slot B4h (the 0085F990 body inlined;
// channels chopped from float(channel*255.0)). Then n = (to - from) / len with len = sqrt(|d|^2)
// when |d|^2 > 1e-10 [00CE3820] else 0 (n is then inf/NaN, as in the image); the camera
// [[00E188A8]+19FCh] is refreshed (00B6DB70) unless byte +5Ch bit 1 is set; w = cross(cam+110h, n)
// (004F9B30); with |w|^2 > 1e-10 and sqrt(|w|^2) > 0.01f, w is normalised (0042B260), h = len*0.1,
// hw = 0.1*h, s = w*hw, q = to - n*h, and the barbs q+s -> to and q-s -> to are drawn in (c1, c1).
void debug_draw_arrow_rgba_floats_0085fb70(cc13_seg53::Seg53Host& draw, Big53Host& host,
                                           const float* from, const float* to, float r0, float g0,
                                           float b0, float a0, float r1, float g1, float b1,
                                           float a1) {
    const std::uint32_t c0 = argb24(r0, g0, b0, a0);
    const std::uint32_t c1 = argb24(r1, g1, b1, a1);
    draw.debug_renderer_line_vslot_b4(from, to, c0, c1);

    float n[3] = {to[0] - from[0], to[1] - from[1], to[2] - from[2]};
    const float dsq = length_sq(n);
    const float len = (static_cast<double>(dsq) > kEps_00ce3820) ? fsqrt(dsq) : 0.0f;
    const float inv = 1.0f / len;
    void* camera = at<void*>(host.game_00e188a8(), 0x19FC);
    n[0] = n[0] * inv;
    n[1] = n[1] * inv;
    n[2] = n[2] * inv;
    if ((at<std::uint8_t>(camera, 0x5C) & 2u) == 0) host.transform_refresh_world_matrix_00b6db70(camera);
    float cross_out[3];
    const float* cr = host.vector3_cross_004f9b30(cross_out, fptr(camera, 0x110), n);
    float w[3] = {cr[0], cr[1], cr[2]};
    const float wsq = length_sq(w);
    if (!(static_cast<double>(wsq) > kEps_00ce3820)) return;
    if (!(fsqrt(wsq) > kHundredth_00d7a238)) return;
    host.normalize_vector_with_floor_0042b260(w);
    const float h = static_cast<float>(static_cast<double>(len) * kTenth_00d7a3a0);
    const float hw = static_cast<float>(kTenth_00d7a3a0 * static_cast<double>(h));
    const float s[3] = {w[0] * hw, w[1] * hw, hw * w[2]};
    const float back[3] = {n[0] * h, n[1] * h, h * n[2]};
    const float q[3] = {to[0] - back[0], to[1] - back[1], to[2] - back[2]};
    const float left[3] = {q[0] + s[0], q[1] + s[1], q[2] + s[2]};
    const float q2[3] = {to[0] - back[0], to[1] - back[1], to[2] - back[2]};
    const float right[3] = {q2[0] - s[0], q2[1] - s[1], q2[2] - s[2]};
    draw.debug_renderer_line_vslot_b4(left, to, c1, c1);
    draw.debug_renderer_line_vslot_b4(right, to, c1, c1);
}

// 0085FFF0: ECX/EDX pass through; the four colour dwords are pushed twice.
void debug_draw_arrow_rgba_floats_one_colour_0085fff0(cc13_seg53::Seg53Host& draw, Big53Host& host,
                                                      const float* from, const float* to,
                                                      const float* rgba) {
    debug_draw_arrow_rgba_floats_0085fb70(draw, host, from, to, rgba[0], rgba[1], rgba[2], rgba[3],
                                          rgba[0], rgba[1], rgba[2], rgba[3]);
}

// 00860140. n = _ftol2(float(float(|sweep| / 2pi) * segments)) (00BF7420, truncation), at least
// 1; step = sweep / n; z = radius * 0.0. For i in [0, n): a0 = float(i)*step + start,
// a1 = float(i+1)*step + start; the points (radius*sin, radius*cos, z) are transformed by the
// frame (004142E0, the end point first) and joined with 0085F990 in the one colour (rgba is
// read through EDX as four floats). The int products are exact for |segments| < 2^24.
void debug_draw_arc_in_frame_00860140(cc13_seg53::Seg53Host& draw, Big53Host& host,
                                      const float* frame, const float* rgba, float radius,
                                      float start, float sweep, std::int32_t segments) {
    const float turns = static_cast<float>(static_cast<double>(std::fabs(sweep)) / kTwoPi_00ce3828);
    const float scaled = static_cast<float>(static_cast<double>(turns) * static_cast<double>(segments));
    std::int32_t n = static_cast<std::int32_t>(scaled);
    if (n < 1) n = 1;
    const float step = static_cast<float>(static_cast<double>(sweep) / static_cast<double>(n));
    const float z = static_cast<float>(static_cast<double>(radius) * kZero_00d7a258);
    for (std::int32_t i = 0; i < n; ++i) {
        const float a0 = static_cast<float>(static_cast<double>(i) * step) + start;
        const float a1 = static_cast<float>(static_cast<double>(i + 1) * step) + start;
        const float s0 = fsin(a0);
        const float k0 = fcos(a0);
        const float v0[3] = {radius * s0, radius * k0, z};
        const float s1 = fsin(a1);
        const float k1 = fcos(a1);
        const float v1[3] = {radius * s1, radius * k1, z};
        float w1[3];
        float w0[3];
        host.vector3_transform_affine_point_004142e0(v1, w1, frame);
        host.vector3_transform_affine_point_004142e0(v0, w0, frame);
        line_one_colour(draw, w0, w1, rgba);
    }
}

// 008605C0. For each axis a (0..2) with c1 = (a+1)%3, c2 = (a+2)%3, and j, m in {0, 1}
// (0 = max, 1 = min): p0[a] = min[a], p1[a] = max[a], p0[c1] = p1[c1] = (j ? min : max)[c1],
// p0[c2] = p1[c2] = (m ? min : max)[c2], 0085F990(p0, p1, rgba, rgba). The two endpoint
// buffers persist across the twelve calls.
void debug_draw_aabb_wireframe_008605c0(cc13_seg53::Seg53Host& draw, const float* box,
                                        const float* rgba) {
    float p0[3];
    float p1[3];
    for (int a = 0; a < 3; ++a) {
        const int c1 = (a + 1) % 3;
        const int c2 = (a + 2) % 3;
        for (int j = 0; j < 2; ++j) {
            for (int m = 0; m < 2; ++m) {
                p0[a] = box[a];
                p1[a] = box[a + 3];
                const float v1 = j != 0 ? box[c1] : box[c1 + 3];
                p1[c1] = v1;
                p0[c1] = v1;
                const float v2 = m != 0 ? box[c2] : box[c2 + 3];
                p1[c2] = v2;
                p0[c2] = v2;
                line_one_colour(draw, p0, p1, rgba);
            }
        }
    }
}

// 00860710. y = float(radius * 0.0); angle(i) = float(i * pi) * (1/32) (both products exact
// or singly rounded); for i in [0, 64): 0085F990(centre + (r sin a_i, y, r cos a_i),
// centre + (r sin a_i+1, y, r cos a_i+1), rgba, rgba).
void debug_draw_circle64_00860710(cc13_seg53::Seg53Host& draw, const float* centre,
                                  const float* rgba, float radius) {
    const float y = static_cast<float>(static_cast<double>(radius) * kZero_00d7a258);
    auto angle = [](std::int32_t i) {
        const float t = static_cast<float>(static_cast<double>(i) * kPi_00ce3d28);
        return static_cast<float>(static_cast<double>(t) * kOneThirtySecond_00cf6108);
    };
    for (std::int32_t i = 0; i < 64; ++i) {
        const float a0 = angle(i);
        const float s0 = fsin(a0);
        const float k0 = fcos(a0);
        const float va[3] = {radius * s0, y, radius * k0};
        const float a1 = angle(i + 1);
        const float s1 = fsin(a1);
        const float k1 = fcos(a1);
        const float vb[3] = {radius * s1, y, radius * k1};
        const float pb[3] = {centre[0] + vb[0], centre[1] + vb[1], centre[2] + vb[2]};
        const float pa[3] = {centre[0] + va[0], centre[1] + va[1], centre[2] + va[2]};
        line_one_colour(draw, pa, pb, rgba);
    }
}

// 008608B0. The function-static colour (1, 0.5, 0, 1) is initialised under guard bit 0 of
// 00F875B8 (the inlined init appears three times; the later two find the bit set). Circle64
// (00860710) in that colour; d = (cx - px, 0, cz - pz); unless |dx| < eps && 0 < eps &&
// |dz| < eps (eps = 1e-4 [00D7A268], compared in double; a NaN takes the draw path), d is
// normalised (0042B260), scaled by radius, and 0085F990(centre, centre - d, colour, colour).
void debug_draw_circle_with_heading_008608b0(cc13_seg53::Seg53Host& draw, Big53Host& host,
                                             const float* centre, float radius, float px,
                                             float py, float pz) {
    (void)py;  // the image never reads the y argument
    std::uint8_t* stat = host.heading_colour_static_00f875a8();
    auto init_colour = [&]() {
        if ((at<std::uint32_t>(stat, 0x10) & 1u) == 0) {
            at<std::uint32_t>(stat, 0x10) |= 1u;
            at<float>(stat, 0x4) = kHalf_00ce3800;
            at<float>(stat, 0x0) = kOne_00d7a24c;
            at<float>(stat, 0x8) = 0.0f;
            at<float>(stat, 0xC) = kOne_00d7a24c;
        }
    };
    init_colour();
    debug_draw_circle64_00860710(draw, centre, reinterpret_cast<const float*>(stat), radius);
    float d[3] = {centre[0] - px, 0.0f, centre[2] - pz};
    if (kEps_00d7a268 > static_cast<double>(std::fabs(d[0])) && kEps_00d7a268 > 0.0 &&
        kEps_00d7a268 > static_cast<double>(std::fabs(d[2]))) {
        return;
    }
    host.normalize_vector_with_floor_0042b260(d);
    d[0] = d[0] * radius;
    d[1] = d[1] * radius;
    d[2] = radius * d[2];
    init_colour();
    const float colour[4] = {at<float>(stat, 0x0), at<float>(stat, 0x4), at<float>(stat, 0x8),
                             at<float>(stat, 0xC)};
    const float tip[3] = {centre[0] - d[0], centre[1] - d[1], centre[2] - d[2]};
    line_one_colour(draw, centre, tip, colour);
}

// 00861260. When 0043A870(world, &xy, 2, 1) is non-zero: 004C7070([00E188A8], x, y, text) with
// text = [obj+4], or the empty string at 00F875BC when that is null (passed as the format).
void debug_draw_text_at_world_point_00861260(Big53Host& host, const float* world,
                                             const void* text) {
    float xy[2];
    if (host.camera_project_0043a870(world, xy, 2, 1) == 0) return;
    const char* s = *reinterpret_cast<const char* const*>(static_cast<const std::uint8_t*>(text) + 4);
    if (s == nullptr) s = "";  // 00F875BC (a zero byte in .bss)
    host.diagnostic_printf_004c7070(host.game_00e188a8(), xy[0], xy[1], s);
}

}  // namespace bsp::cc13_big53
