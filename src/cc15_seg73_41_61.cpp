// cc15_seg73_41_61: the game-side MetricsSystem TU (00750880-00758090), packet
// cc15-rB1. See include/bsp/cc15_seg73_41_61.hpp and docs/CC15_SEG73_41_61.md.
// Hypothesis names, build-tested only, nothing wired.

#include "bsp/cc15_seg73_41_61.hpp"

#include <cstring>

namespace bsp {
namespace cc15_metrics {
namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

template <class T>
const T& at(const void* base, std::size_t offset) {
    return *reinterpret_cast<const T*>(static_cast<const std::uint8_t*>(base) + offset);
}

void* ptr_at(const void* base, std::size_t offset) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(at<std::uint32_t>(base, offset)));
}

std::uint32_t addr(const void* p) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
}

// The data pointer of a native string, or 00E1AED8 when it is null (the inline
// "data ? data : empty" every reader repeats).
const char* text_or_empty(std::uint32_t data, Cc15MetricsHost& host) {
    return data != 0 ? reinterpret_cast<const char*>(static_cast<std::uintptr_t>(data)) : host.empty_string_00e1aed8();
}

// Inline native-string release: when data is set, return a length+1 block.
void release(const NativeStringSlot& s, Cc15MetricsHost& host) {
    if (s.data != 0) host.pool_return_block_00bd1510(s.data, s.length + 1);
}

// Inline native-string assign (the image's "if (&dst != &src) { 0041DD40(dst,
// src.length, 1); if (src.length) memcpy(dst.data, src.data, dst.length); }").
void assign(NativeStringSlot& dst, const NativeStringSlot& src, Cc15MetricsHost& host) {
    if (&dst == &src) return;
    host.native_string_resize_0041dd40(&dst, src.length, 1);
    if (src.length != 0) {
        host.memcpy_00bf7680(reinterpret_cast<void*>(static_cast<std::uintptr_t>(dst.data)),
                             reinterpret_cast<const void*>(static_cast<std::uintptr_t>(src.data)), dst.length);
    }
}

// Inline one-character native string from a literal (0041DD40(s, 1, 1), then
// memcpy(data, literal, length + 1)).
void literal1(NativeStringSlot& s, const char* text, Cc15MetricsHost& host) {
    s.length = 0;
    s.data = 0;
    host.native_string_resize_0041dd40(&s, 1, 1);
    if (s.data != 0) host.memcpy_00bf7680(reinterpret_cast<void*>(static_cast<std::uintptr_t>(s.data)), text, s.length + 1);
}

// The "ZString key(literal); ZString value(...); 007520A0(key, value); ~value; ~key"
// sequence the report functions repeat.
void add_string_row(void* metric, const char* key_text, const char* value_text, Cc15MetricsHost& host) {
    ZStringSlot key;
    ZStringSlot value;
    host.zstring_from_cstr_00a52560(&key, key_text);
    void* v = host.zstring_from_cstr_00a52560(&value, value_text);
    metric_add_string_007520a0(metric, &key, v, host);
    host.zstring_destroy_00a53210(&value);
    host.zstring_destroy_00a53210(&key);
}

void add_int_text_row(void* metric, const char* key_text, std::int32_t value_int, Cc15MetricsHost& host) {
    ZStringSlot key;
    ZStringSlot value;
    host.zstring_from_cstr_00a52560(&key, key_text);
    void* v = host.zstring_from_int_00a52820(&value, value_int);
    metric_add_string_007520a0(metric, &key, v, host);
    host.zstring_destroy_00a53210(&value);
    host.zstring_destroy_00a53210(&key);
}

// The local player name the session id is seeded from: game +6A0h when its
// length is nonzero (unsigned JA), else +68Ch.
const NativeStringSlot& seed_name(void* game) {
    std::uint8_t* g = static_cast<std::uint8_t*>(game);
    return at<std::uint32_t>(g, 0x6A0) != 0 ? at<NativeStringSlot>(g, 0x6A0) : at<NativeStringSlot>(g, 0x68C);
}

// The 3-instruction signed modulo the image uses for x % 64 (AND 8000003Fh,
// JNS, DEC/OR FFFFFFC0h/INC): truncating, like C++ %.
std::int32_t mod64(std::int32_t x) { return x % 64; }

}  // namespace

std::uint8_t metrics_encode_char_00750880(std::uint32_t v) {
    if (v <= 9) return static_cast<std::uint8_t>(v + 0x30);
    if (v - 10 <= 0x1A) return static_cast<std::uint8_t>(v + 0x37);
    if (v - 0x24 <= 0x10) return static_cast<std::uint8_t>(v + 0x1D);
    switch (v) {
    case 0x35: return 0x2B;
    case 0x36: return 0x2D;
    case 0x37: return 0x5B;
    case 0x38: return 0x5D;
    case 0x39: return 0x7B;
    case 0x3A: return 0x7D;
    case 0x3B: return 0x5F;
    case 0x3C: return 0x21;
    case 0x3D: return 0x2E;
    case 0x3E: return 0x2C;
    case 0x3F: return 0x28;
    default: break;
    }
    // SETNE AL; SUB AL,1; AND AL,E9h; ADD AL,40h: 64 -> 29h ')', else 40h '@'.
    const std::uint8_t ne = (v != 0x40) ? 1 : 0;
    return static_cast<std::uint8_t>(((ne - 1) & 0xE9) + 0x40);
}

void metrics_generate_session_id_00751bd0(std::uint8_t* metrics, Cc15MetricsHost& host) {
    void* game = host.game_00e188a8();
    const NativeStringSlot& first = seed_name(game);
    const char* name = text_or_empty(first.data, host);  // read before GetSystemTime, as in the image
    std::uint16_t st[8];  // SYSTEMTIME: year, month, day-of-week, day, hour, minute, second, ms
    host.get_system_time(st);
    metrics[0x1CE] = 0x20;
    metrics[0x1CF] = metrics_encode_char_00750880(static_cast<std::uint32_t>(st[0]) - 0x7D8);
    metrics[0x1D0] = metrics_encode_char_00750880(st[1]);
    metrics[0x1D1] = metrics_encode_char_00750880(st[3]);
    metrics[0x1D2] = metrics_encode_char_00750880(st[4]);
    metrics[0x1D3] = metrics_encode_char_00750880(st[5]);
    metrics[0x1D4] = metrics_encode_char_00750880(st[6]);
    const std::uint16_t ms = st[7];
    metrics[0x1D5] = metrics_encode_char_00750880(static_cast<std::uint32_t>(mod64(ms)));
    metrics[0x1D6] = ms > 0x40 ? metrics_encode_char_00750880(static_cast<std::uint32_t>(mod64(ms - 0x40))) : 0x30;
    const std::int32_t length = static_cast<std::int32_t>(seed_name(game).length);  // re-read at 00751CCC
    std::uint8_t* out = metrics + 0x1D7;
    std::int32_t i = 1;
    do {
        std::uint32_t v;
        if (i < length) {
            const std::int32_t a = static_cast<signed char>(name[i]);
            const std::int32_t b = static_cast<signed char>(name[i - 1]);
            v = static_cast<std::uint32_t>(mod64(a + b - 0x40));
        } else {
            v = host.random_threads_next_u32_00bd2fc0(0) & 0x3F;
        }
        *out++ = metrics_encode_char_00750880(v);
        i += 2;
    } while (out - (metrics + 0x1CE) < 0x17);
    metrics[0x1E5] = 0;
    // 004254B0("Generated Session id %s", metrics + 1CEh): trivial logger, omitted.
}

std::uint8_t* metrics_static_pool_construct_00751080(std::uint8_t* pool, Cc15MetricsHost& host) {
    host.static_pool_construct_00a54010(pool, pool + 0x40, kStaticPoolBytes);
    at<std::uint32_t>(pool, 0) = kStaticPoolVtable_00d0127c;
    return pool;
}

void metrics_static_pool_destroy_007510a0(std::uint8_t* pool, Cc15MetricsHost& host) {
    at<std::uint32_t>(pool, 0) = kStaticPoolVtable_00d0127c;
    host.static_pool_destroy_00a54040(pool);
}

std::uint8_t* metrics_static_pool_deleting_dtor_007510b0(std::uint8_t* pool, std::uint32_t flags, Cc15MetricsHost& host) {
    at<std::uint32_t>(pool, 0) = kStaticPoolVtable_00d0127c;
    host.static_pool_destroy_00a54040(pool);
    if ((flags & 1) != 0) host.free_00bf65ac(pool);
    return pool;
}

std::uint8_t* metric_string_data_construct_00751800(std::uint8_t* data, const void* src, Cc15MetricsHost& host) {
    at<std::uint32_t>(data, 0) = kStringDataVtable_00d01290;
    host.zstring_copy_00a529d0(data + 4, src);
    return data;
}

void metric_string_data_destroy_00751870(std::uint8_t* data, Cc15MetricsHost& host) {
    at<std::uint32_t>(data, 0) = kStringDataVtable_00d01290;
    host.zstring_destroy_00a53210(data + 4);
    at<std::uint32_t>(data, 0) = kPooledObjectVtable_00d24f84;
}

std::uint8_t* metric_string_data_deleting_dtor_00751960(std::uint8_t* data, std::uint32_t flags, Cc15MetricsHost& host) {
    metric_string_data_destroy_00751870(data, host);  // inlined in the image
    if ((flags & 1) != 0) host.pool_free_00a51e70(data);
    return data;
}

std::uint8_t* metric_int_data_construct_007518c0(std::uint8_t* data, const std::uint32_t* src) {
    at<std::uint32_t>(data, 0) = kIntDataVtable_00d01298;
    at<std::uint32_t>(data, 4) = *src;
    return data;
}

std::uint8_t* metric_bool_data_construct_00751910(std::uint8_t* data, const std::uint8_t* src) {
    at<std::uint32_t>(data, 0) = kBoolDataVtable_00d012a0;
    at<std::uint8_t>(data, 4) = *src;
    return data;
}

void metric_int_data_write_007518e0(const std::uint8_t* data, std::uint32_t stream, Cc15MetricsHost& host) {
    host.stream_write_u32_00a4d670(stream, at<std::uint32_t>(data, 4));
}

void metric_bool_data_write_00751930(const std::uint8_t* data, std::uint32_t stream, Cc15MetricsHost& host) {
    host.stream_write_u32_00a4d670(stream, at<std::uint8_t>(data, 4));  // MOVZX
}

void metric_int_data_destroy_00751900(std::uint8_t* data) { at<std::uint32_t>(data, 0) = kPooledObjectVtable_00d24f84; }

void metric_bool_data_destroy_00751950(std::uint8_t* data) { at<std::uint32_t>(data, 0) = kPooledObjectVtable_00d24f84; }

std::uint8_t* metric_int_data_deleting_dtor_007519d0(std::uint8_t* data, std::uint32_t flags, Cc15MetricsHost& host) {
    at<std::uint32_t>(data, 0) = kPooledObjectVtable_00d24f84;
    if ((flags & 1) != 0) host.pool_free_00a51e70(data);
    return data;
}

std::uint8_t* metric_bool_data_deleting_dtor_007519f0(std::uint8_t* data, std::uint32_t flags, Cc15MetricsHost& host) {
    at<std::uint32_t>(data, 0) = kPooledObjectVtable_00d24f84;
    if ((flags & 1) != 0) host.pool_free_00a51e70(data);
    return data;
}

std::uint32_t* metrics_singleton_register_00751d70(std::uint32_t* self, void*& global_00e1aed4, SingletonLifetimeHost& host) {
    return cc13_seg41::singleton_register(self, kMetricsSingletonVtable_00d012c0, global_00e1aed4, host);
}

void metrics_singleton_unregister_00752410(std::uint32_t* self, void*& global_00e1aed4, SingletonLifetimeHost& host) {
    cc13_seg41::singleton_unregister(self, kMetricsSingletonVtable_00d012c0, global_00e1aed4, host);
}

bool metrics_library_startup_00752020(const MetricsStaticArena& arena, Cc15MetricsHost& host) {
    if ((*arena.guard_00f1af20 & 1) == 0) {
        *arena.guard_00f1af20 |= 1;
        host.static_pool_construct_00a54010(arena.object_00e1aee0, arena.buffer_00e1af20, kStaticPoolBytes);
        at<std::uint32_t>(arena.object_00e1aee0, 0) = kStaticPoolVtable_00d0127c;
        host.atexit_00bf6ff5(0x00CDD6E0u);  // vtable store + JMP 00A54040 on the arena
    }
    host.set_allocator_00a51ee0(arena.object_00e1aee0);
    host.library_startup_00a51d40();
    return true;  // MOV AL, 1
}

void metric_add_string_007520a0(void* metric, const void* key, const void* value, Cc15MetricsHost& host) {
    void* block = host.pool_alloc_00a51e50(0x10);
    std::uint32_t record = 0;
    if (block != nullptr) record = addr(metric_string_data_construct_00751800(static_cast<std::uint8_t*>(block), value, host));
    host.metric_add_vslot4(static_cast<std::uint8_t*>(metric) + 8, key, &record);
}

void metric_add_int_00752120(void* metric, const void* key, const std::uint32_t* value, Cc15MetricsHost& host) {
    void* block = host.pool_alloc_00a51e50(8);
    std::uint32_t record = 0;
    if (block != nullptr) record = addr(metric_int_data_construct_007518c0(static_cast<std::uint8_t*>(block), value));
    host.metric_add_vslot4(static_cast<std::uint8_t*>(metric) + 8, key, &record);
}

void metric_add_bool_00752170(void* metric, const void* key, const std::uint8_t* value, Cc15MetricsHost& host) {
    void* block = host.pool_alloc_00a51e50(8);
    std::uint32_t record = 0;
    if (block != nullptr) record = addr(metric_bool_data_construct_00751910(static_cast<std::uint8_t*>(block), value));
    host.metric_add_vslot4(static_cast<std::uint8_t*>(metric) + 8, key, &record);
}

void metrics_system_init_007524d0(std::uint8_t* metrics, Cc15MetricsHost& host) {
    for (int i = 0; i < 19; ++i) metrics[0x20 + i] = kUnitTypeTags_00e0a570[i][0] != '\0' ? 1 : 0;  // strlen > 0
    metrics[0x33] = 0;  // DL, the last terminator read
    for (int row = 0; row < 19; ++row) {
        for (int col = 0; col < 19; ++col) {
            metrics[0x34 + 0x14 * row + col] = kUnitPairTags_00e0a5c0[19 * row + col][0] != '\0' ? 1 : 0;
        }
    }
    for (int i = 0; i <= 0x13; ++i) {
        metrics[0x47 + 0x14 * i] = 0;
        metrics[0x1B0 + i] = 0;
    }
    if (metrics[0x238] == 0 && metrics_library_startup_00752020(host.static_arena(), host)) {
        void* url = host.operator_new_00bf55be(0x96);
        at<std::uint32_t>(metrics, 0x10) = addr(url);
        std::memcpy(url, kMetricsUrl_00d012e8, sizeof kMetricsUrl_00d012e8);  // REP MOVSD, 0Fh dwords
        void* block = host.pool_alloc_00a51e50(100);
        void* collector = block != nullptr ? host.collector_construct_00a4f7c0(block) : nullptr;
        at<std::uint32_t>(metrics, 0x230) = addr(collector);
        void* tblock = host.pool_alloc_00a51e50(0x9C);
        void* transmitter = nullptr;
        bool url_built = false;
        alignas(4) std::uint8_t url_object[0x4C];  // the ZUrl temporary at [ESP+18h] (frame room 4Ch)
        if (tblock != nullptr) {
            void* u = host.url_construct_00a57210(url_object, static_cast<const char*>(ptr_at(metrics, 0x10)));
            url_built = true;
            transmitter = host.transmitter_construct_00a50210(tblock, u);
        }
        at<std::uint32_t>(metrics, 0x234) = addr(transmitter);
        if (url_built) host.url_destroy_00a56400(url_object);
        void* t = ptr_at(metrics, 0x234);
        host.set_transmitter_00a4f040(t != nullptr ? static_cast<std::uint8_t*>(t) + 0x10 : nullptr);
        metrics[0x238] = 1;
        // 004254B0("MetricsSystem ... Initialised ..."): trivial logger, omitted.
    }
    at<std::uint32_t>(metrics, 0x14) = 0;
    const std::int32_t geo = host.get_user_geo_id_nation();
    if (geo != -1) at<std::int32_t>(metrics, 0x14) = geo;
    metrics[0x1CE] = 'N';
    metrics[0x1CF] = 'U';
    metrics[0x1D0] = 'L';
    metrics[0x1D1] = 'L';
    metrics[0x1D2] = 0;
    at<std::uint32_t>(metrics, 0x22C) = kNoUpload;
}

void metrics_session_message_destroy_00752b90(std::uint8_t* message, Cc15MetricsHost& host) {
    at<std::uint32_t>(message, 0) = kMetricsSessionVtable_00d01324;
    release(at<NativeStringSlot>(message, 0x18), host);
    at<std::uint32_t>(message, 0) = kSessionMessageBaseVtable_00ce4974;
}

void metrics_session_message_write_00752c00(const std::uint8_t* message, void* cursor, Cc15MetricsHost& host) {
    host.bit_cursor_write_bits_00428ff0(cursor, at<std::uint8_t>(message, 0x10), 8);
    host.bit_cursor_write_owned_string_00429ac0(cursor, &at<NativeStringSlot>(message, 0x18));
}

std::uint8_t* metrics_session_message_construct_00752d50(std::uint8_t* message, NativeStringSlot id, Cc15MetricsHost& host) {
    host.session_message_construct_0075b430(message, kMetricsSessionType);
    at<std::uint32_t>(message, 4) = 1;
    at<std::uint32_t>(message, 0) = kMetricsSessionVtable_00d01324;
    NativeStringSlot& s = at<NativeStringSlot>(message, 0x18);
    s.length = 0;
    s.data = 0;
    // 00752DAA: resize to the argument's length, memcpy only when it is nonzero
    // (the argument's own length, not s.length; they agree after the resize).
    host.native_string_resize_0041dd40(&s, id.length, 1);
    if (id.length != 0) {
        host.memcpy_00bf7680(reinterpret_cast<void*>(static_cast<std::uintptr_t>(s.data)),
                             reinterpret_cast<const void*>(static_cast<std::uintptr_t>(id.data)), s.length);
    }
    release(id, host);  // the by-value argument
    return message;
}

void metrics_build_timestamp_00752e00(std::uint8_t* metrics, Cc15MetricsHost& host) {
    std::uint16_t st[8];
    host.get_system_time(st);
    NativeStringSlot colon_minute, colon_hour, space, dash_day, dash_month;  // built in this order
    literal1(colon_minute, ":", host);  // 00CF00B0
    literal1(colon_hour, ":", host);    // 00CF00B0
    literal1(space, " ", host);         // 00CE3A90
    literal1(dash_day, "-", host);      // 00CEED40
    literal1(dash_month, "-", host);    // 00CEED40
    NativeStringSlot t[11];
    NativeStringSlot* s = host.native_string_from_int_004260b0(&t[0], st[0]);
    s = host.native_string_concat_004261a0(s, &t[1], &dash_month);
    s = host.native_string_concat_int_004263b0(s, &t[2], st[1]);
    s = host.native_string_concat_004261a0(s, &t[3], &dash_day);
    s = host.native_string_concat_int_004263b0(s, &t[4], st[3]);
    s = host.native_string_concat_004261a0(s, &t[5], &space);
    s = host.native_string_concat_int_004263b0(s, &t[6], st[4]);
    s = host.native_string_concat_004261a0(s, &t[7], &colon_hour);
    s = host.native_string_concat_int_004263b0(s, &t[8], st[5]);
    s = host.native_string_concat_004261a0(s, &t[9], &colon_minute);
    s = host.native_string_concat_int_004263b0(s, &t[10], st[6]);
    assign(at<NativeStringSlot>(metrics, 0x18), *s, host);
    for (int i = 10; i >= 0; --i) release(t[i], host);  // reverse construction order
    release(dash_month, host);
    release(dash_day, host);
    release(space, host);
    release(colon_hour, host);
    release(colon_minute, host);
}

void metrics_set_session_id_00753350(std::uint8_t* metrics, NativeStringSlot id, Cc15MetricsHost& host) {
    if (id.length > 0x18) {
        NativeStringSlot cut;
        NativeStringSlot* c = host.native_string_substring_00469840(&id, &cut, 0, 0x17);
        assign(id, *c, host);
        release(cut, host);
    }
    const char* src = text_or_empty(id.data, host);
    char* dst = reinterpret_cast<char*>(metrics + 0x1CE);
    while ((*dst++ = *src++) != '\0') {
    }
    release(id, host);  // the by-value argument
}

namespace {

// The gate 00753570 and 00753810 share: a selected Live user, sign-in state 2
// or 1 (state read twice, as in the image), a Live-enabled account.
bool live_user_ready(Cc15MetricsHost& host) {
    void* x = host.xenon_00f8abe8();
    if (!host.xenon_has_selected_user_00a3e510(x)) return false;
    if (host.xenon_sign_in_state_00a3ead0(x) != 2 && host.xenon_sign_in_state_00a3ead0(x) != 1) return false;
    return host.xenon_is_live_enabled_00a3e520(x);
}

void* collector(const std::uint8_t* metrics) { return ptr_at(metrics, 0x230); }

}  // namespace

void metrics_send_hardware_00753570(std::uint8_t* metrics, const std::uint8_t* rows, Cc15MetricsHost& host) {
    if (metrics[0x238] == 0) return;
    if (!live_user_ready(host)) return;
    host.collector_enable_00a4efe0(collector(metrics), 1);
    void* metric = host.collector_create_metric_vslotC(collector(metrics), "Hardwares");
    if (metric == nullptr) return;
    // Checked vector walk over 10h-byte {key, value} native-string pairs; its
    // _invalid_parameter_noinfo hooks test first <= last and the element index,
    // which this walk cannot break.
    const std::uint32_t last = at<std::uint32_t>(rows, 8);
    for (std::uint32_t e = at<std::uint32_t>(rows, 4); e != last; e += 0x10) {
        const std::uint8_t* row = reinterpret_cast<const std::uint8_t*>(static_cast<std::uintptr_t>(e));
        ZStringSlot value;
        ZStringSlot key;
        void* v = host.zstring_from_cstr_00a52560(&value, text_or_empty(at<std::uint32_t>(row, 0xC), host));
        void* k = host.zstring_from_cstr_00a52560(&key, text_or_empty(at<std::uint32_t>(row, 4), host));
        metric_add_string_007520a0(metric, k, v, host);
        host.zstring_destroy_00a53210(&key);
        host.zstring_destroy_00a53210(&value);
        // 004254B0("Hardwares %s %s", key, value): trivial logger, omitted.
    }
    ZStringSlot location;
    host.zstring_from_cstr_00a52560(&location, "Location");
    // 00752120 inlined: an 8-byte int record of +14h.
    void* block = host.pool_alloc_00a51e50(8);
    std::uint32_t record = 0;
    if (block != nullptr) {
        record = addr(metric_int_data_construct_007518c0(static_cast<std::uint8_t*>(block), &at<std::uint32_t>(metrics, 0x14)));
    }
    host.metric_add_vslot4(static_cast<std::uint8_t*>(metric) + 8, &location, &record);
    host.zstring_destroy_00a53210(&location);
    metrics_build_timestamp_00752e00(metrics, host);
    add_string_row(metric, "TimeStamp", text_or_empty(at<std::uint32_t>(metrics, 0x1C), host), host);
    host.collector_submit_00a4f4d0(collector(metrics), metric);  // result unused here
}

namespace {

// Lua lookup of VehicleClass[id].Name into `out` (00753E0F / 00753F01).
void vehicle_class_name(void* game, std::int32_t id, NativeStringSlot& out, Cc15MetricsHost& host) {
    alignas(4) std::uint8_t globals[0x14], vehicle_class[0x14], entry[0x14], name[0x14];
    void* g = host.lua_globals_00b67980(static_cast<std::uint8_t*>(game) + 0x1A0C, globals);
    host.lua_get_field_00b67800(g, vehicle_class, "VehicleClass");
    host.lua_destroy_00b67700(globals);
    host.lua_get_index_00b67720(vehicle_class, entry, id);
    void* n = host.lua_get_field_00b67800(entry, name, "Name");
    host.native_string_assign_cstr_0041e350(&out, host.lua_to_string_00b662b0(n));
    host.lua_destroy_00b67700(name);
    host.lua_destroy_00b67700(entry);
    host.lua_destroy_00b67700(vehicle_class);
}

}  // namespace

void metrics_mission_start_00753810(std::uint8_t* metrics, std::int32_t event, std::uint8_t checkpoint,
                                    Cc15MetricsHost& host) {
    if (!live_user_ready(host)) return;
    // 004254B0("MMetricsSystem MissionStart Call"): trivial logger, omitted.
    std::uint8_t* game = static_cast<std::uint8_t*>(host.game_00e188a8());
    const std::int32_t net_mode = at<std::int32_t>(game, 0x1FE4);
    if (net_mode == 0 || net_mode == 1) metrics_generate_session_id_00751bd0(metrics, host);
    if (at<std::int32_t>(host.game_00e188a8(), 0x1FE4) == 1) {
        // Host: broadcast the id. The argument string is built in the outgoing
        // argument area (0041E870), consumed by 00752D50, and the message is
        // sent through 0076A9F0(ECX game+1EF0h, message, 0) and destroyed.
        NativeStringSlot id;
        host.native_string_construct_cstr_0041e870(&id, reinterpret_cast<const char*>(metrics + 0x1CE));
        alignas(4) std::uint8_t message[0x20];
        metrics_session_message_construct_00752d50(message, id, host);
        host.session_send_0076a9f0(static_cast<std::uint8_t*>(host.game_00e188a8()) + 0x1EF0, message, 0);
        metrics_session_message_destroy_00752b90(message, host);
    }
    metrics[0x239] = checkpoint;
    if (metrics[0x238] == 0) return;
    host.collector_enable_00a4efe0(collector(metrics), 1);
    void* metric = host.collector_create_metric_vslotC(collector(metrics), "MissionStart");
    if (metric == nullptr) return;
    std::uint8_t* sid = metrics + 0x1CE;
    *sid = 'A';
    {
        const std::uint32_t map = at<std::uint32_t>(host.game_00e188a8(), 0x219C);
        host.sprintf_s_00bf7a6a(reinterpret_cast<char*>(metrics + 0x1F6), text_or_empty(map, host));
    }
    if (event == 1) {
        add_string_row(metric, "ModString", "Single", host);
        *sid = 'S';
    } else if (event == 5) {
        add_string_row(metric, "ModString", "TranindGround", host);  // sic
        *sid = 'T';
    } else if (event == 3) {
        add_string_row(metric, "ModString", "Skirmish", host);
        *sid = 'K';
    } else if (event == 9) {
        add_string_row(metric, "ModString", "Multi", host);
        // Jump table 007541F8 on 004BCA50(game): 0-3 'I', 4 'D', 5 'E', 6 'G', 7 'C', else 'X'.
        const std::uint32_t mode = static_cast<std::uint32_t>(host.game_effective_mode_004bca50(host.game_00e188a8()));
        static constexpr char kModeChar[8] = {'I', 'I', 'I', 'I', 'D', 'E', 'G', 'C'};
        *sid = mode <= 7 ? static_cast<std::uint8_t>(kModeChar[mode]) : 'X';
    }
    add_string_row(metric, "SiD", reinterpret_cast<const char*>(sid), host);
    add_int_text_row(metric, "Mod", static_cast<signed char>(*sid), host);  // MOVSX: the character code as a number
    add_string_row(metric, "MapName", reinterpret_cast<const char*>(metrics + 0x1F6), host);
    add_string_row(metric, "Version", host.version_string_00e0a540(), host);
    NativeStringSlot player;
    host.native_string_construct_cstr_0041e870(&player, "none");
    {
        void* x = host.xenon_00f8abe8();
        if (host.xenon_has_selected_user_00a3e510(x)) {
            char buffer[0x50];  // [ESP+84h]
            buffer[0] = 0;
            host.sprintf_llx_00bf7a6a(buffer, host.xenon_selected_xuid_00a3eb00(host.xenon_00f8abe8()));
            NativeStringSlot hex;
            host.native_string_construct_cstr_0041e870(&hex, buffer);
            host.native_string_assign_00425f40(&player, &hex);
            release(hex, host);
        }
    }
    add_string_row(metric, "PlayerName", text_or_empty(player.data, host), host);
    NativeStringSlot unit;
    host.native_string_construct_cstr_0041e870(&unit, "none");
    {
        std::uint8_t* g = static_cast<std::uint8_t*>(host.game_00e188a8());
        bool path_a = false;
        if (event == 9 || event == 3) {
            const std::uint8_t* player_unit = static_cast<const std::uint8_t*>(
                ptr_at(g, 0x18CC + 4 * at<std::uint32_t>(g, 0x18EC)));
            if (at<std::int32_t>(player_unit, 0x110) > 0) {
                path_a = true;
                vehicle_class_name(g, at<std::int32_t>(player_unit, 0x110), unit, host);
            }
        }
        if (!path_a && event == 1) {
            const std::uint8_t* manager = static_cast<const std::uint8_t*>(host.manager_00e198ac());
            if (manager != nullptr && at<std::uint32_t>(manager, 0x70) != 0) {
                const std::uint8_t* owner = static_cast<const std::uint8_t*>(ptr_at(manager, 0x70));
                if (at<std::int32_t>(owner, 0x134) > 0 && checkpoint == 0) {
                    vehicle_class_name(g, at<std::int32_t>(owner, 0x134), unit, host);
                }
            }
        }
    }
    add_string_row(metric, "Unit", text_or_empty(unit.data, host), host);
    add_int_text_row(metric, "Diff", at<std::int32_t>(host.game_00e188a8(), 0x6AC), host);
    {
        ZStringSlot key;
        host.zstring_from_cstr_00a52560(&key, "CheckPoint");
        metric_add_bool_00752170(metric, &key, &metrics[0x239], host);
        host.zstring_destroy_00a53210(&key);
    }
    {
        ZStringSlot key;
        host.zstring_from_cstr_00a52560(&key, "Location");
        metric_add_int_00752120(metric, &key, &at<std::uint32_t>(metrics, 0x14), host);
        host.zstring_destroy_00a53210(&key);
    }
    metrics_build_timestamp_00752e00(metrics, host);
    add_string_row(metric, "TimeStamp", text_or_empty(at<std::uint32_t>(metrics, 0x1C), host), host);
    if (!host.collector_submit_00a4f4d0(collector(metrics), metric)) {
        // 004254B0(00D0136C, [+22Ch]): trivial logger, omitted.
        host.report_error_00a52320(0xF);
    }
    metrics[0x23A] = 1;
    release(unit, host);
    release(player, host);
}

namespace {

// How a row's pair temporary is destroyed: through 00750910 (category 0 and
// the common rows of category 1) or inline as ~value (+0Ch), ~key (+0) with
// 00A53210 (every later row).
enum class PairDtor { call_00750910, inline_00a53210 };

// One "rows.push_back(pair(ZString(key), ZString(value)))": the value is built
// first (it is the higher argument block), 00751310 consumes both.
void push_pair(ZPairVectorSlot& rows, ZStringPairArgs& args, PairDtor dtor, Cc15MetricsHost& host) {
    ZPairSlot pair;
    void* p = host.zpair_construct_00751310(&pair, &args);
    host.zpairs_push_back_00754ad0(&rows, p);
    if (dtor == PairDtor::call_00750910) {
        host.zpair_destroy_00750910(&pair);
    } else {
        host.zstring_destroy_00a53210(pair.bytes + 0xC);
        host.zstring_destroy_00a53210(pair.bytes);
    }
}

void push_text_row(ZPairVectorSlot& rows, const char* key, const char* value, PairDtor dtor, Cc15MetricsHost& host) {
    ZStringPairArgs args;
    host.zstring_from_cstr_00a52560(&args.value, value);
    host.zstring_from_cstr_00a52560(&args.key, key);
    push_pair(rows, args, dtor, host);
}

void push_int_row(ZPairVectorSlot& rows, const char* key, std::int32_t value, PairDtor dtor, Cc15MetricsHost& host) {
    ZStringPairArgs args;
    host.zstring_from_int_00a52820(&args.value, value);
    host.zstring_from_cstr_00a52560(&args.key, key);
    push_pair(rows, args, dtor, host);
}

// The SiD / Mod / Version rows every category 1-8 starts with. Mod is the
// first session-id character, sign-extended (MOVSX).
void push_common_rows(ZPairVectorSlot& rows, const std::uint8_t* metrics, PairDtor dtor, Cc15MetricsHost& host) {
    const char* sid = reinterpret_cast<const char*>(metrics + 0x1CE);
    push_text_row(rows, "SiD", sid, dtor, host);
    push_int_row(rows, "Mod", static_cast<signed char>(sid[0]), dtor, host);
    push_text_row(rows, "Version", host.version_string_00e0a540(), dtor, host);
}

// "map[key] = rows" through 00755520 and 007547F0, then the has-data flag.
void store_category(std::uint8_t* metrics, std::int32_t key, const ZPairVectorSlot& rows, Cc15MetricsHost& host) {
    std::int32_t k = key;
    host.zpairs_assign_007547f0(host.category_rows_00755520(metrics + 4, &k), &rows);
    metrics[0x1C4 + key] = 1;
}

// ~vector: when first is set, 00752900(first, last) and free(first).
void destroy_rows(ZPairVectorSlot& rows, Cc15MetricsHost& host) {
    if (rows.first == 0) return;
    host.zpair_destroy_range_00752900(rows.first, rows.last, &rows);
    host.free_00bf65ac(reinterpret_cast<void*>(static_cast<std::uintptr_t>(rows.first)));
}

// The image's checked-iterator loop over a native std::map object: begin is
// head->left, the head is re-read for every end test, ++ is the bound
// instantiation. Its _invalid_parameter_noinfo (00BF6713) checks test that the
// iterator belongs to this map and is not end, which these walks cannot break,
// and are omitted.
template <class F>
void walk_map(const std::uint8_t* map, void (Cc15MetricsHost::*inc)(NativeTreeIter*), Cc15MetricsHost& host, F visit) {
    NativeTreeIter it{addr(map), at<std::uint32_t>(ptr_at(map, 4), 0)};
    while (it.node != at<std::uint32_t>(map, 4)) {
        visit(static_cast<const std::uint8_t*>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(it.node))));
        (host.*inc)(&it);
    }
}

// map<int unit_type, int value> rows: one int row per unit type whose
// +20h+type flag is set, tagged from 00E0A570.
void push_unit_rows(ZPairVectorSlot& rows, const std::uint8_t* metrics, const std::uint8_t* map,
                    void (Cc15MetricsHost::*inc)(NativeTreeIter*), Cc15MetricsHost& host) {
    walk_map(map, inc, host, [&](const std::uint8_t* node) {
        const std::int32_t type = at<std::int32_t>(node, 0xC);
        if (metrics[0x20 + type] == 0) return;
        push_int_row(rows, kUnitTypeTags_00e0a570[type], at<std::int32_t>(node, 0x10), PairDtor::inline_00a53210, host);
    });
}

// map<int target, int value> rows for one attacker: the +34h pair flag
// [attacker*14h + target], tagged from 00E0A5C0[attacker*19 + target].
void push_pair_rows(ZPairVectorSlot& rows, const std::uint8_t* metrics, std::int32_t attacker, const std::uint8_t* map,
                    Cc15MetricsHost& host) {
    walk_map(map, &Cc15MetricsHost::tree_iter_inc_0057f4a0, host, [&](const std::uint8_t* node) {
        const std::int32_t target = at<std::int32_t>(node, 0xC);
        if (metrics[0x34 + attacker * 0x14 + target] == 0) return;
        push_int_row(rows, kUnitPairTags_00e0a5c0[attacker * 19 + target], at<std::int32_t>(node, 0x10),
                     PairDtor::inline_00a53210, host);
    });
}

}  // namespace

// The float fields convert with CVTTSS2SI (truncation; SSE, no x87 in this
// body), which is what static_cast<int32_t> compiles to.
void metrics_report_mission_end_007556a0(std::uint8_t* metrics, Cc15MetricsHost& host) {
    // 004254B0("Metrics_System MissionEnd Called"): trivial logger, omitted.
    if (metrics[0x238] == 0) return;
    if (!live_user_ready(host)) return;
    if (metrics[0x23A] == 0) metrics_generate_session_id_00751bd0(metrics, host);
    // vector::clear on each category: erase(begin, end) inlined as
    // 007521C0(last, last, first) then 00752900(new last, old last).
    for (std::int32_t category = 0; category < 10; ++category) {
        metrics[0x1C4 + category] = 0;
        std::int32_t key = category;
        std::uint8_t* v = static_cast<std::uint8_t*>(host.category_rows_00755520(metrics + 4, &key));
        const std::uint32_t last = at<std::uint32_t>(v, 8);
        if (at<std::uint32_t>(v, 4) != last) {
            const std::uint32_t new_last = host.zpair_copy_007521c0(last, last, at<std::uint32_t>(v, 4));
            host.zpair_destroy_range_00752900(new_last, at<std::uint32_t>(v, 8), v);
            at<std::uint32_t>(v, 8) = new_last;
        }
    }
    at<std::uint32_t>(metrics, 0x22C) = 0;
    // The local player's 284h-byte score record: game +21A0h -> base,
    // base + [base+1424h] * 284h + 4.
    const std::uint8_t* players = static_cast<const std::uint8_t*>(ptr_at(host.game_00e188a8(), 0x21A0));
    const std::uint8_t* rec = players + at<std::int32_t>(players, 0x1424) * 0x284 + 4;
    const PairDtor call = PairDtor::call_00750910;
    const PairDtor inl = PairDtor::inline_00a53210;

    // Category 0: the mission summary.
    {
        ZPairVectorSlot rows{0, 0, 0, 0};  // the image zeroes +4..+0Ch only
        push_common_rows(rows, metrics, call, host);
        push_text_row(rows, "MapName", "OMMBIE", call, host);  // 00D01544, a literal
        NativeStringSlot player;
        host.native_string_construct_cstr_0041e870(&player, "none");
        if (host.xenon_has_selected_user_00a3e510(host.xenon_00f8abe8())) {
            char buffer[0x6C];  // [ESP+78h], to the end of the locals
            buffer[0] = 0;
            host.sprintf_llx_00bf7a6a(buffer, host.xenon_selected_xuid_00a3eb00(host.xenon_00f8abe8()));
            NativeStringSlot hex;
            host.native_string_construct_cstr_0041e870(&hex, buffer);
            assign(player, hex, host);
            release(hex, host);
        }
        push_text_row(rows, "PlayerName", text_or_empty(player.data, host), call, host);
        push_text_row(rows, "Mode",
                      host.game_mode_name_004b4b40(host.game_effective_mode_004bca50(host.game_00e188a8())), call, host);
        push_text_row(rows, "Unit", "ladik", call, host);
        push_int_row(rows, "Bandwidth_up", 0, call, host);
        push_int_row(rows, "Bandwidth_down", 0, call, host);
        push_text_row(rows, "End_State", "Normal", call, host);
        push_int_row(rows, "play_time", static_cast<std::int32_t>(at<float>(rec, 0xC)), call, host);
        push_int_row(rows, "mission_completed", at<std::int32_t>(rec, 0), call, host);
        push_int_row(rows, "ranking", at<std::int32_t>(rec, 4), call, host);
        push_int_row(rows, "completion_time", static_cast<std::int32_t>(at<float>(rec, 0x10)), call, host);
        push_int_row(rows, "slot", at<std::int32_t>(rec, 0x250), call, host);
        push_int_row(rows, "map_usage", static_cast<std::int32_t>(at<float>(rec, 0x14)), call, host);
        push_int_row(rows, "total_action_score", at<std::int32_t>(rec, 0x1CC), call, host);
        push_int_row(rows, "total_ship_score", at<std::int32_t>(rec, 0x1D0), call, host);
        push_int_row(rows, "total_plane_score", at<std::int32_t>(rec, 0x1D4), call, host);
        push_int_row(rows, "total_command_score", at<std::int32_t>(rec, 0x1D8), call, host);
        push_int_row(rows, "total_badge_score", at<std::int32_t>(rec, 0x1DC), call, host);
        push_int_row(rows, "total_score", at<std::int32_t>(rec, 0x1E0), call, host);
        push_int_row(rows, "CheckPoint", metrics[0x239], call, host);
        push_int_row(rows, "Location", at<std::int32_t>(metrics, 0x14), call, host);
        metrics_build_timestamp_00752e00(metrics, host);
        push_text_row(rows, "TimeStamp", text_or_empty(at<std::uint32_t>(metrics, 0x1C), host), call, host);
        store_category(metrics, 0, rows, host);
        release(player, host);
        destroy_rows(rows, host);
    }
    // Categories 1, 2, 3 and 6: map<int, int> at +F0h, +108h, +E4h, +D8h.
    struct UnitCategory {
        std::int32_t key;
        std::size_t map;
        PairDtor common;
    };
    const auto unit_category = [&](const UnitCategory& c) {
        if (at<std::uint32_t>(rec, c.map + 8) == 0) return;
        ZPairVectorSlot rows{0, 0, 0, 0};
        push_common_rows(rows, metrics, c.common, host);
        push_unit_rows(rows, metrics, rec + c.map, &Cc15MetricsHost::tree_iter_inc_0057f4a0, host);
        store_category(metrics, c.key, rows, host);
        destroy_rows(rows, host);
    };
    unit_category({1, 0xF0, call});
    unit_category({2, 0x108, inl});
    unit_category({3, 0xE4, inl});
    // Category 4: map<int, map<int, int>> at +CCh, only the inner map under key 0.
    if (at<std::uint32_t>(rec, 0xD4) != 0) {
        ZPairVectorSlot rows{0, 0, 0, 0};
        push_common_rows(rows, metrics, inl, host);
        walk_map(rec + 0xCC, &Cc15MetricsHost::tree_iter_inc_0057f430, host, [&](const std::uint8_t* node) {
            if (at<std::int32_t>(node, 0xC) != 0) return;
            push_unit_rows(rows, metrics, node + 0x10, &Cc15MetricsHost::tree_iter_inc_0057f4a0, host);
        });
        store_category(metrics, 4, rows, host);
        destroy_rows(rows, host);
    }
    // Category 5: map<int, float> at +114h; rows only above 1.0f (00D7A24C;
    // COMISS/JBE, so NaN is skipped), truncated to int.
    if (at<std::uint32_t>(rec, 0x11C) != 0) {
        ZPairVectorSlot rows{0, 0, 0, 0};
        push_common_rows(rows, metrics, inl, host);
        walk_map(rec + 0x114, &Cc15MetricsHost::tree_iter_inc_0057f510, host, [&](const std::uint8_t* node) {
            if (!(at<float>(node, 0x10) > 1.0f)) return;
            const std::int32_t type = at<std::int32_t>(node, 0xC);
            if (metrics[0x20 + type] == 0) return;
            push_int_row(rows, kUnitTypeTags_00e0a570[type], static_cast<std::int32_t>(at<float>(node, 0x10)), inl,
                         host);
        });
        store_category(metrics, 5, rows, host);
        destroy_rows(rows, host);
    }
    unit_category({6, 0xD8, inl});
    // Category 7: map<int, map<int attacker, map<int target, int>>> at +B4h,
    // only the middle map under key 1.
    if (at<std::uint32_t>(rec, 0xBC) != 0) {
        ZPairVectorSlot rows{0, 0, 0, 0};  // [ESP+60h] in the image
        push_common_rows(rows, metrics, inl, host);
        walk_map(rec + 0xB4, &Cc15MetricsHost::tree_iter_inc_0057f3c0, host, [&](const std::uint8_t* outer) {
            if (at<std::int32_t>(outer, 0xC) != 1) return;
            walk_map(outer + 0x10, &Cc15MetricsHost::tree_iter_inc_0057f920, host, [&](const std::uint8_t* middle) {
                push_pair_rows(rows, metrics, at<std::int32_t>(middle, 0xC), middle + 0x10, host);
            });
        });
        store_category(metrics, 7, rows, host);
        destroy_rows(rows, host);
    }
    // Category 8: map<int attacker, map<int, map<int target, int>>> at +9Ch,
    // only the middle map entries under key 1.
    if (at<std::uint32_t>(rec, 0xA4) != 0) {
        ZPairVectorSlot rows{0, 0, 0, 0};  // [ESP+30h] in the image
        push_common_rows(rows, metrics, inl, host);
        walk_map(rec + 0x9C, &Cc15MetricsHost::tree_iter_inc_0057f350, host, [&](const std::uint8_t* outer) {
            const std::int32_t attacker = at<std::int32_t>(outer, 0xC);
            walk_map(outer + 0x10, &Cc15MetricsHost::tree_iter_inc_0057f430, host, [&](const std::uint8_t* middle) {
                if (at<std::int32_t>(middle, 0xC) != 1) return;
                push_pair_rows(rows, metrics, attacker, middle + 0x10, host);
            });
        });
        store_category(metrics, 8, rows, host);
        destroy_rows(rows, host);
    }
    // 004254B0("Metrics_System MissionEnd Call End"): trivial logger, omitted.
    at<std::uint32_t>(metrics, 0x22C) = 0;
}

void metrics_upload_tick_00757ce0(std::uint8_t* metrics, Cc15MetricsHost& host) {
    if (metrics[0x238] == 0) return;
    std::int32_t& index = at<std::int32_t>(metrics, 0x22C);
    if (index == static_cast<std::int32_t>(kNoUpload)) return;
    // 004254B0("Metrics_System UpLoading %s Metrics", category): trivial logger, omitted.
    if (!host.collector_ready_vslot4(collector(metrics))) return;
    if (metrics[0x1C4 + index] == 0) {
        index += 1;  // nothing recorded for this category
        return;
    }
    void* metric = host.collector_create_metric_vslotC(collector(metrics), kScoreCategories_00e0a544[index]);
    if (metric == nullptr) return;
    // Checked walk of map[index] (18h-byte pair<ZString, ZString>); operator[]
    // is re-evaluated for the end test each pass, as in the image. The hooks
    // (container identity, first <= last, element in range) cannot fail here.
    const std::uint8_t* rows = static_cast<const std::uint8_t*>(host.category_rows_00755520(metrics + 4, &index));
    std::uint32_t e = at<std::uint32_t>(rows, 4);  // 007526D0 begin()
    for (;;) {
        rows = static_cast<const std::uint8_t*>(host.category_rows_00755520(metrics + 4, &index));
        if (e == at<std::uint32_t>(rows, 8)) break;
        const std::uint8_t* pair = reinterpret_cast<const std::uint8_t*>(static_cast<std::uintptr_t>(e));
        metric_add_string_007520a0(metric, pair, pair + 0xC, host);
        e += 0x18;
    }
    if (host.collector_submit_00a4f4d0(collector(metrics), metric)) {
        index += 1;
        return;
    }
    // 004254B0("Metrics_System Error Add %s Metrics to Q", category): trivial logger, omitted.
    host.report_error_00a52320(0xF);
}

void metrics_system_destroy_00757ea0(std::uint8_t* metrics, void*& global_00e1aed4, Cc15MetricsHost& host) {
    at<std::uint32_t>(metrics, 0) = kMetricsSystemVtable_00d015c0;
    if (metrics[0x238] != 0) {
        host.collector_shutdown_00a4f300(ptr_at(metrics, 0x230));
        host.transmitter_stop_vslot8(static_cast<std::uint8_t*>(ptr_at(metrics, 0x234)) + 0x10);
        if (void* c = ptr_at(metrics, 0x230)) host.delete_vslot0(c);
        if (void* t = ptr_at(metrics, 0x234)) host.delete_vslot0(t);
        host.library_shutdown_00a52390();
        host.library_shutdown_00a51a30();
        host.library_shutdown_00a52170();
    }
    host.free_00bf6989(ptr_at(metrics, 0x10));
    release(at<NativeStringSlot>(metrics, 0x18), host);
    std::uint8_t* map = metrics + 4;
    const std::uint32_t head = at<std::uint32_t>(map, 4);
    host.map_erase_range_00755250(map, at<std::uint32_t>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(head)), 0), head);
    host.free_00bf65ac(ptr_at(map, 4));
    at<std::uint32_t>(map, 4) = 0;
    at<std::uint32_t>(map, 8) = 0;
    metrics_singleton_unregister_00752410(reinterpret_cast<std::uint32_t*>(metrics), global_00e1aed4, host.singletons());
}

std::uint8_t* metrics_system_construct_00757fe0(std::uint8_t* metrics, void*& global_00e1aed4, Cc15MetricsHost& host) {
    metrics_singleton_register_00751d70(reinterpret_cast<std::uint32_t*>(metrics), global_00e1aed4, host.singletons());
    std::uint8_t* map = metrics + 4;
    at<std::uint32_t>(metrics, 0) = kMetricsSystemVtable_00d015c0;
    std::uint8_t* head = static_cast<std::uint8_t*>(host.map_buy_head_node_00751fb0(map));
    at<std::uint32_t>(map, 4) = addr(head);
    head[0x21] = 1;  // _Isnil
    at<std::uint32_t>(head, 4) = addr(head);  // parent
    at<std::uint32_t>(head, 0) = addr(head);  // left
    at<std::uint32_t>(head, 8) = addr(head);  // right
    at<std::uint32_t>(map, 8) = 0;            // size
    at<std::uint32_t>(metrics, 0x18) = 0;     // timestamp {length, data}
    at<std::uint32_t>(metrics, 0x1C) = 0;
    metrics[0x238] = 0;
    metrics_system_init_007524d0(metrics, host);
    return metrics;
}

// Generated by local/rb_gen_tables.py from the on-disk image (pointer tables in .data).
const char* const kScoreCategories_00e0a544[10] = {
    "Score_MissionEnd", "Score_Spawns", "Score_AISpawns", "Score_UnitSells", "Score_RemainingUnits", "Score_UnitUsages", "Score_SuicideUnits", "Score_PlayerKills", "Score_PlayerDamages", "",
};
const char* const kUnitTypeTags_00e0a570[19] = {
    "cv", "dd", "pt", "bb", "cacl", "ak", "landing", "lbomb", "dbomb", "tbomb", "fighter", "recon", "kami", "sub", "lvehicle", "lfort", "af", "sy", "",
};
const char* const kUnitPairTags_00e0a5c0[19 * 19] = {
    // row 0
    "cv-cv", "cv-dd", "cv-pt", "cv-bb", "cv-cacl", "cv-ak", "cv-landing", "cv-lbomb", "cv-dbomb", "cv-tbomb",
    "cv-fighter", "cv-recon", "cv-kami", "cv-sub", "cv-lvehicle", "cv-lfort", "cv-af", "cv-sy", "",
    // row 1
    "dd-cv", "dd-dd", "dd-pt", "dd-bb", "dd-cacl", "dd-ak", "dd-landing", "dd-lbomb", "dd-dbomb", "dd-tbomb",
    "dd-fighter", "dd-recon", "dd-kami", "dd-sub", "dd-lvehicle", "dd-lfort", "dd-af", "dd-sy", "",
    // row 2
    "pt-cv", "pt-dd", "pt-pt", "pt-bb", "pt-cacl", "pt-ak", "pt-landing", "pt-lbomb", "pt-dbomb", "pt-tbomb",
    "pt-fighter", "pt-recon", "pt-kami", "pt-sub", "pt-lvehicle", "pt-lfort", "pt-af", "pt-sy", "",
    // row 3
    "bb-cv", "bb-dd", "bb-pt", "bb-bb", "bb-cacl", "bb-ak", "bb-landing", "bb-lbomb", "bb-dbomb", "bb-tbomb",
    "bb-fighter", "bb-recon", "bb-kami", "bb-sub", "bb-lvehicle", "bb-lfort", "bb-af", "bb-sy", "",
    // row 4
    "cacl-cv", "cacl-dd", "cacl-pt", "cacl-bb", "cacl-cacl", "cacl-ak", "cacl-landing", "cacl-lbomb",
    "cacl-dbomb", "cacl-tbomb", "cacl-fighter", "cacl-recon", "cacl-kami", "cacl-sub", "cacl-lvehicle",
    "cacl-lfort", "cacl-af", "cacl-sy", "",
    // row 5
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    // row 6
    "landing-cv", "landing-dd", "landing-pt", "", "landing-cacl", "landing-ak", "landing-landing",
    "landing-lbomb", "landing-dbomb", "landing-tbomb", "landing-fighter", "landing-recon", "landing-kami",
    "landing-sub", "landing-lvehicle", "landing-lfort", "landing-af", "landing-sy", "",
    // row 7
    "lbomb-cv", "lbomb-dd", "lbomb-pt", "lbomb-bb", "lbomb-cacl", "lbomb-ak", "lbomb-landing", "lbomb-lbomb",
    "lbomb-dbomb", "lbomb-tbomb", "lbomb-fighter", "lbomb-recon", "lbomb-kami", "lbomb-sub", "lbomb-lvehicle",
    "lbomb-lfort", "lbomb-af", "lbomb-sy", "",
    // row 8
    "dbomb-cv", "dbomb-dd", "dbomb-pt", "dbomb-bb", "dbomb-cacl", "dbomb-ak", "dbomb-landing", "dbomb-lbomb",
    "dbomb-dbomb", "dbomb-tbomb", "dbomb-fighter", "dbomb-recon", "dbomb-kami", "dbomb-sub", "dbomb-lvehicle",
    "dbomb-lfort", "dbomb-af", "dbomb-sy", "",
    // row 9
    "tbomb-cv", "tbomb-dd", "tbomb-pt", "tbomb-bb", "tbomb-cacl", "tbomb-ak", "tbomb-landing", "tbomb-lbomb",
    "tbomb-dbomb", "tbomb-tbomb", "tbomb-fighter", "tbomb-recon", "tbomb-kami", "tbomb-sub", "tbomb-lvehicle",
    "tbomb-lfort", "tbomb-af", "tbomb-sy", "",
    // row 10
    "fighter-cv", "fighter-dd", "fighter-pt", "fighter-bb", "fighter-cacl", "fighter-ak", "fighter-landing",
    "fighter-lbomb", "fighter-dbomb", "fighter-tbomb", "fighter-fighter", "fighter-recon", "fighter-kami",
    "fighter-sub", "fighter-lvehicle", "fighter-lfort", "fighter-af", "fighter-sy", "",
    // row 11
    "recon-cv", "recon-dd", "recon-pt", "recon-bb", "recon-cacl", "recon-ak", "recon-landing", "recon-lbomb",
    "recon-dbomb", "recon-tbomb", "recon-fighter", "recon-recon", "recon-kami", "recon-sub", "recon-lvehicle",
    "recon-lfort", "recon-af", "recon-sy", "",
    // row 12
    "kami-cv", "kami-dd", "kami-pt", "kami-bb", "kami-cacl", "kami-ak", "kami-landing", "kami-lbomb",
    "kami-dbomb", "kami-tbomb", "kami-fighter", "kami-recon", "kami-kami", "kami-sub", "kami-lvehicle",
    "kami-lfort", "kami-af", "kami-sy", "",
    // row 13
    "sub-cv", "sub-dd", "sub-pt", "sub-bb", "sub-cacl", "sub-ak", "sub-landing", "sub-lbomb", "sub-dbomb",
    "sub-tbomb", "sub-fighter", "sub-recon", "sub-kami", "sub-sub", "sub-lvehicle", "sub-lfort", "sub-af",
    "sub-sy", "",
    // row 14
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    // row 15
    "lfort-cv", "lfort-dd", "lfort-pt", "lfort-bb", "lfort-cacl", "lfort-ak", "lfort-landing", "lfort-lbomb",
    "lfort-dbomb", "lfort-tbomb", "lfort-fighter", "lfort-recon", "lfort-kami", "lfort-sub", "lfort-lvehicle",
    "lfort-lfort", "lfort-af", "lfort-sy", "",
    // row 16
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    // row 17
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    // row 18
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
};

}  // namespace cc15_metrics
}  // namespace bsp
