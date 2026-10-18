// cc13_seg62L: lobby layout bodies (packet cc13-seg62L, docs/CC13_SEG62L.md).
// Names are hypotheses; see the header for the hooks and layouts. No x87.
// The checked-iterator failures (00BF6713) are called at the image's points;
// the C++ continues after them as the image does.

#include "bsp/cc13_seg62L.hpp"

#include <cstring>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

constexpr std::uint32_t kSelectedName = 0x00E195BC;   // NativeString {length, data}
constexpr std::uint32_t kSelectedLayout = 0x00E195C4;  // NativeString {length, data}

// Rows of the lobby layout vector: count = (last - first) / 24h as a signed
// division (the IMUL 38E38E39h / SAR 3 sequence), compared unsigned.
std::uint32_t row_count(void* screen) {
    const std::int32_t bytes = static_cast<std::int32_t>(at<std::uint8_t*>(screen, 0x2E8) - at<std::uint8_t*>(screen, 0x2E4));
    return static_cast<std::uint32_t>(bytes / 0x24);
}

// The checked row access: first non-null and index < count, else 00BF6713.
void check_row(void* screen, std::uint32_t index, Cc13Seg62LHost& host) {
    if (at<void*>(screen, 0x2E4) == nullptr || index >= row_count(screen)) host.crt_invalid_parameter_noinfo_00bf6713();
}

// A NativeString vector {.., first +4h, last +8h} with 8-byte elements.
std::uint32_t string_count(void* vector) {
    // SUB, SAR 3, then an unsigned compare.
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(vector, 0x8) - at<std::uint8_t*>(vector, 0x4)) >> 3);
}

// The inlined NativeString assignment into an image global: unless the
// source is the global, 0041DD40(global, source length, 1), then when the
// source length is non-zero a memcpy of the (resized) global length.
void assign_global(std::uint32_t global, const std::uint8_t* source, Cc13Seg62LHost& host) {
    void* const dest = host.image_global(global);
    if (source == dest) return;
    const std::uint32_t length = *reinterpret_cast<const std::uint32_t*>(source);
    host.native_string_resize_0041dd40(dest, length, 1);
    if (length != 0) {
        host.crt_memcpy_00bf7680(at<char*>(dest, 0x4), *reinterpret_cast<char* const*>(source + 0x4),
                                 at<std::uint32_t>(dest, 0x0));
    }
}

// The emptied global: 0041DD40(global, 0, 0), then a memcpy of the empty
// text 00CE3A0C with the global's length when data is non-null.
void empty_global(std::uint32_t global, Cc13Seg62LHost& host) {
    void* const dest = host.image_global(global);
    host.native_string_resize_0041dd40(dest, 0, 0);
    if (char* const data = at<char*>(dest, 0x4); data != nullptr) {
        host.crt_memcpy_00bf7680(data, host.image_global(0x00CE3A0C), at<std::uint32_t>(dest, 0x0));
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 005D8B80 (lobby layout selection refresh)
// ABI: __thiscall(ECX = screen), RET; EAX = 00E195BC on every exit. No SEH.
// Evidence: the live listing 005D8B80..005D8FB5 (330 instructions; no export).
// Simplifications: the image keeps the row byte offset in a stack slot and
// re-reads +2E4h at every access; the C++ re-reads +2E4h at the same points.
// ---------------------------------------------------------------------------
std::uint32_t cc13_seg62L_lobby_refresh_selection_005d8b80(void* screen, Cc13Seg62LHost& host) {
    if (at<std::int32_t>(screen, 0x2F0) != -1 && at<std::int32_t>(screen, 0x2F4) != -1) {
        // 005D8B97..005D8C03: 00E195BC = record +14h vector [+2F4h].
        const std::uint32_t entry = at<std::uint32_t>(screen, 0x2F4);
        std::uint8_t* const names = static_cast<std::uint8_t*>(host.lobby_slot_names_005d5160(screen)) + 0x14;
        if (at<void*>(names, 0x4) == nullptr || entry >= string_count(names)) host.crt_invalid_parameter_noinfo_00bf6713();
        assign_global(kSelectedName, at<std::uint8_t*>(names, 0x4) + entry * 8, host);
        // 005D8C03..005D8C59: in a session of kind 2, the command-line text
        // is the +4h vector entry's text (00E19605 when null).
        if (at<std::int32_t>(*static_cast<void**>(host.image_global(0x00E188A8)), 0x1FE4) == 2) {
            const std::uint32_t index = at<std::uint32_t>(screen, 0x2F4);
            std::uint8_t* const texts = static_cast<std::uint8_t*>(host.lobby_slot_names_005d5160(screen)) + 0x4;
            if (at<void*>(texts, 0x4) == nullptr || index >= string_count(texts)) host.crt_invalid_parameter_noinfo_00bf6713();
            const char* text = at<const char*>(at<std::uint8_t*>(texts, 0x4) + index * 8, 0x4);
            if (text == nullptr) text = static_cast<const char*>(host.image_global(0x00E19605));
            host.game_set_command_line_text_004c8950(*static_cast<void**>(host.image_global(0x00E188A8)), text);
        }
        return kSelectedName;
    }
    // 005D8C6A..005D8CC9
    empty_global(kSelectedName, host);
    empty_global(kSelectedLayout, host);
    at<std::int32_t>(screen, 0x2F0) = -1;
    at<std::int32_t>(screen, 0x2F4) = -1;

    // 005D8CD3..005D8FA4: every row.
    for (std::uint32_t row = 0, offset = 0;
         at<void*>(screen, 0x2E4) != nullptr && row < row_count(screen); ++row, offset += 0x24) {
        void* const current_game = *static_cast<void**>(host.image_global(0x00E188A8));
        void* const mission = at<void*>(current_game, 0x5FC);
        if (mission == nullptr) continue;
        check_row(screen, row, host);
        if (at<std::int32_t>(at<std::uint8_t*>(screen, 0x2E4) + offset, 0x0) !=
            at<std::int32_t>(at<void*>(*static_cast<void**>(host.image_global(0x00E188A8)), 0x5FC), 0x1098)) {
            continue;
        }
        // 005D8D65..005D8E5D: the first +4h entry equal to the game's +600h
        // string (length, then _stricmp).
        for (std::uint32_t entry = 0;; ++entry) {
            check_row(screen, row, host);
            std::uint8_t* const texts = at<std::uint8_t*>(screen, 0x2E4) + offset + 0x4;
            if (at<void*>(texts, 0x4) == nullptr || entry >= string_count(texts)) break;
            check_row(screen, row, host);
            std::uint8_t* const texts2 = at<std::uint8_t*>(screen, 0x2E4) + offset + 0x4;
            if (at<void*>(texts2, 0x4) == nullptr || entry >= string_count(texts2)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            std::uint8_t* const candidate = at<std::uint8_t*>(texts2, 0x4) + entry * 8;
            void* const g = *static_cast<void**>(host.image_global(0x00E188A8));
            const std::uint32_t length = at<std::uint32_t>(candidate, 0x0);
            const std::uint32_t game_length = at<std::uint32_t>(g, 0x600);
            if (length != game_length) continue;
            std::int32_t compare;
            if (length == 0) {
                compare = game_length != 0 ? -1 : 0;
            } else if (game_length == 0) {
                compare = 1;
            } else {
                compare = host.crt_stricmp_00bf7fbf(at<const char*>(candidate, 0x4), at<const char*>(g, 0x604));
            }
            if (compare != 0) continue;

            // 005D8E62..005D8F98: select (row, entry) and copy both strings.
            check_row(screen, row, host);
            std::uint8_t* const names = at<std::uint8_t*>(screen, 0x2E4) + offset + 0x14;
            if (at<void*>(names, 0x4) == nullptr || entry >= string_count(names)) host.crt_invalid_parameter_noinfo_00bf6713();
            assign_global(kSelectedName, at<std::uint8_t*>(names, 0x4) + entry * 8, host);
            check_row(screen, row, host);
            std::uint8_t* const layouts = at<std::uint8_t*>(screen, 0x2E4) + offset + 0x4;
            if (at<void*>(layouts, 0x4) == nullptr || entry >= string_count(layouts)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            assign_global(kSelectedLayout, at<std::uint8_t*>(layouts, 0x4) + entry * 8, host);
            at<std::uint32_t>(screen, 0x2F0) = row;
            at<std::uint32_t>(screen, 0x2F4) = entry;
            break;  // 005D8F9E: on to the next row
        }
    }
    return kSelectedName;
}

// ---------------------------------------------------------------------------
// 005D8FC0 (lobby layout select default)
// ABI: __thiscall(ECX = screen), RET. No SEH.
// Evidence: the live listing 005D8FC0..005D92A3 (230 instructions; no export).
// ---------------------------------------------------------------------------
void cc13_seg62L_lobby_select_default_005d8fc0(void* screen, Cc13Seg62LHost& host) {
    auto game = [&host]() { return *static_cast<void**>(host.image_global(0x00E188A8)); };
    if (at<void*>(game(), 0x5FC) == nullptr) return;
    for (std::uint32_t row = 0, offset = 0;
         at<void*>(screen, 0x2E4) != nullptr && row < row_count(screen); ++row, offset += 0x24) {
        // 005D9012..005D90A9: this mission's row with a non-empty +14h vector.
        check_row(screen, row, host);
        if (at<std::int32_t>(at<std::uint8_t*>(screen, 0x2E4) + offset, 0x0) !=
            at<std::int32_t>(at<void*>(game(), 0x5FC), 0x1098)) {
            continue;
        }
        check_row(screen, row, host);
        {
            std::uint8_t* const names = at<std::uint8_t*>(screen, 0x2E4) + offset + 0x14;
            if (at<void*>(names, 0x4) == nullptr || string_count(names) == 0) continue;
        }
        // 005D90AF..005D91B2: entry 0 of both vectors into the globals.
        check_row(screen, row, host);
        std::uint8_t* const names = at<std::uint8_t*>(screen, 0x2E4) + offset + 0x14;
        if (at<void*>(names, 0x4) == nullptr || string_count(names) == 0) host.crt_invalid_parameter_noinfo_00bf6713();
        assign_global(kSelectedName, at<std::uint8_t*>(names, 0x4), host);
        check_row(screen, row, host);
        std::uint8_t* const layouts = at<std::uint8_t*>(screen, 0x2E4) + offset + 0x4;
        if (at<void*>(layouts, 0x4) == nullptr || string_count(layouts) == 0) host.crt_invalid_parameter_noinfo_00bf6713();
        assign_global(kSelectedLayout, at<std::uint8_t*>(layouts, 0x4), host);
        // 005D91B5..005D91E6
        at<std::uint32_t>(screen, 0x2F0) = row;
        at<std::uint32_t>(screen, 0x2F4) = 0;
        if (at<std::int32_t>(screen, 0x2F8) == -1 && at<std::int32_t>(screen, 0x2FC) == -1) {
            at<std::uint32_t>(screen, 0x2F8) = row;
            at<std::uint32_t>(screen, 0x2FC) = at<std::uint32_t>(screen, 0x2F4);
        }
        // 005D91EC..005D928A: the game's +600h string = entry 0 text of the
        // +4h vector (00E19605 when null): 0041DD40(strlen, 0), memcpy of the
        // new length (no terminator copied).
        check_row(screen, row, host);
        std::uint8_t* const texts = at<std::uint8_t*>(screen, 0x2E4) + offset + 0x4;
        if (at<void*>(texts, 0x4) == nullptr || string_count(texts) == 0) host.crt_invalid_parameter_noinfo_00bf6713();
        const char* text = at<const char*>(at<std::uint8_t*>(texts, 0x4), 0x4);
        if (text == nullptr) text = static_cast<const char*>(host.image_global(0x00E19605));
        std::uint8_t* const target = static_cast<std::uint8_t*>(game()) + 0x600;
        const std::uint32_t length = text != nullptr ? static_cast<std::uint32_t>(std::strlen(text)) : 0;
        host.native_string_resize_0041dd40(target, length, 0);
        if (char* const data = at<char*>(target, 0x4); data != nullptr) {
            host.crt_memcpy_00bf7680(data, text, at<std::uint32_t>(target, 0x0));
        }
    }
}

}  // namespace bsp
