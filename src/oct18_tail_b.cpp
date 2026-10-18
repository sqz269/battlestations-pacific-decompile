#include "bsp/oct18_tail_b.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace bsp::oct18_tail_b {
namespace {
static_assert(sizeof(void*) == 4, "Original storage uses Win32 pointers");
template<class T> T get(const void* p, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(p) + offset, sizeof value);
    return value;
}
template<class T> void put(void* p, std::size_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(p) + offset, &value, sizeof value);
}
void* at(void* p, std::size_t offset) {
    return static_cast<std::byte*>(p) + offset;
}
[[noreturn]] void invalid(Host& host) {
    host.invalid_iterator_00bf6713();
    throw std::logic_error("non-returning CRT provider returned");
}
}

// 007810F0 / 0079A380 / 0079D840: __thiscall, RET 4, EAX 0/1;
// vslot 5Ch. These expose the already catalogued class-test leaves over +C4h.
bool class60_is_kind_007810f0(const void* object, int query) {
    return query == 0x60 || query == 2 || query == 1 || query == 0 ||
           query == get<int>(object, 0xc4);
}
bool class54_is_kind_0079a380(const void* object, int query) {
    return query == 0x54 || query == 0x4c || query == 0 || query == get<int>(object, 0xc4);
}
bool class55_is_kind_0079d840(const void* object, int query) {
    return query == 0x55 || query == 0x4c || query == 0 || query == get<int>(object, 0xc4);
}
// 007B8A90: ECX=plane, RET; original two-instruction business body.
void plane_clear_turbo_007b8a90(void* plane) { put<std::uint8_t>(plane, 0x9f9, 0); }

// 008364B0 / 00836790: vslot 48h, unused ECX, stack category, RET 4.
bool director_allows_category_008364b0(int category) noexcept {
    return category == 2 || category == 0;
}
bool ship_director_allows_category_00836790(int category) noexcept {
    return category == 3 || category == 2 || category == 0;
}

// 00836EB0: __thiscall(class, target), RET 4, ST0 float. Assembly shows
// MOVSS/FLD only, with no x87 arithmetic or excess precision to reproduce.
float gun_class_value_for_target_00836eb0(const void* gun_class, void* target, Host& host) {
    return get<float>(gun_class, host.is_kind_vslot_5c(target, 8) ? 0x764 : 0x768);
}
// 00837DE0: ECX=settings, RET, EAX=selected interior pointer. The actual
// [00E188A8] global is supplied by the caller, without substituting host state.
void* ship_tuning_block_00837de0(void* settings, const void* game) {
    return at(settings, get<int>(game, 0x1fe4) != 0 ? 0xf0 : 0x80);
}
// 0084EB60: cdecl/no args, tail delegation to the Lua machine.
void reset_stationary_class_table_0084eb60(Host& host) {
    host.run_lua_006b8ad0("StationaryClass=nil\n", 0, 0, 2);
}

// 00797DA0: __thiscall(track,float), RET 4. Despite its old name it does
// not store time: rewinding before an active cue's +F0h clears +F8h and
// resets its optional +E4h object. FCOMIP/JBE skips unordered comparisons.
void movie_camera_track_set_time_00797da0(void* track, float time, Host& host) {
    void* node = get<void*>(get<void*>(track, 0x6c), 0);
    while (node != get<void*>(track, 0x6c)) {
        void* cue = get<void*>(node, 8);
        if (get<int>(cue, 0xf8) != 0 && time < get<float>(cue, 0xf0)) {
            put<int>(cue, 0xf8, 0);
            void* callback = get<void*>(cue, 0xe4);
            if (callback != nullptr) host.movie_reset_vslot_2c(callback);
        }
        // Re-read the sentinel after the callback, like the native iterator.
        if (node == get<void*>(track, 0x6c)) invalid(host);
        node = get<void*>(node, 0);
    }
}
// 00836680: ECX=destination, stack source, RET 4. EDI preserves destination
// across the base call. Force byte is captured before the source target query.
void director_copy_state_00836680(void* destination, void* source, Host& host) {
    host.copy_controller_00720450(destination, source);
    if (host.category_vslot_48(source, 2)) {
        const auto force = get<std::uint8_t>(source, 0x23c);
        if (force != 0) {
            void* target = host.fire_target_vslot_2c(source);
            host.store_fire_target_00836240(destination, target, force);
        }
    }
}
// 008367F0: __thiscall(director,visitor), RET 4. These are addresses of
// mutable fields, not copied values; tag 3 is a byte and tag 2 a float.
void director_visit_state_008367f0(void* director, void* visitor, Host& host) {
    host.visit_enables_00836510(director, visitor);
    struct Field { const char* name; std::size_t offset; int type; };
    constexpr Field fields[] = {
        {"torpedoAvoidance", 0x240, 3}, {"shipCollisionAvoidance", 0x241, 3},
        {"landCollisionAvoidance", 0x242, 3}, {"cruiseIsHeading", 0x243, 3},
        {"cruiseSteerOrHeading", 0x244, 2}, {"cruiseThrust", 0x248, 2}};
    for (const auto& field : fields)
        host.visit_reference_vslot_10(visitor, field.name, field.type, at(director, field.offset));
}
// 0084EFE0: ECX=stationary, RET, nullable part+160h query. The part link
// has no recovered descriptive type; this function claims only its offsets.
void* stationary_part_scene_0084efe0(const void* stationary) {
    void* part = get<void*>(stationary, 0x19c);
    return part != nullptr ? get<void*>(part, 0x160) : nullptr;
}
// 00784CD0: ECX=transport, RET, bool in AL. Deliberately clear the FIRST
// peer's flag when the count is one, even if a later peer supplied that flag.
bool consume_single_peer_flag_00784cd0(void* transport, Host& host) {
    std::uint32_t count = 0;
    for (void* node = get<void*>(get<void*>(transport, 0xc), 0);
         node != get<void*>(transport, 0xc); node = get<void*>(node, 0)) {
        void* peer = get<void*>(node, 8);
        if (peer != nullptr) {
            void* record = get<void*>(peer, 0xd50);
            if (record != nullptr && get<std::uint8_t>(record, 0x1a) != 0) ++count;
        }
    }
    if (count != 1) return false;
    void* first = get<void*>(get<void*>(transport, 0xc), 0);
    if (first == get<void*>(transport, 0xc)) invalid(host);
    put<std::uint8_t>(get<void*>(get<void*>(first, 8), 0xd50), 0x1a, 0);
    return true;
}
// 00968470: __thiscall(subscription,parameter-vector), RET 4, EAX 0/1.
// Keep lazy bounds checks: a rejecting earlier filter does not inspect later
// parameters. Re-read the vector and filter pointers after each virtual call.
bool recon_subscription_matches_00968470(const void* subscription,
                                         const void* parameters, Host& host) {
    for (std::uint32_t i = 0; i != 4; ++i) {
        const auto begin = get<std::uintptr_t>(parameters, 4);
        const auto end = get<std::uintptr_t>(parameters, 8);
        if (begin == 0 || ((end - begin) >> 2) < i + 1) invalid(host);
        const auto value = get<std::uint32_t>(reinterpret_cast<void*>(begin), i * 4);
        void* filter = get<void*>(subscription, 0xc + i * 0x10);
        if (!host.filter_vslot_00(filter, value)) return false;
    }
    return true;
}
} // namespace bsp::oct18_tail_b
