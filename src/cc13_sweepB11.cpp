// cc13_sweepB11: eleventh pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB11, docs/CC13_SWEEPB11.md). Names are hypotheses; see the header
// for the hooks.
//
// No x87 in this pass. EH frames are noted, not modelled. The checked-iterator
// failures (00BF6713) are called at the image's points; the image treats the
// call as returning, so the C++ continues after it as the image does.

#include "bsp/cc13_sweepB11.hpp"

#include <cstring>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

// The inlined native-string release: when data is non-null, the data goes
// back to the sized pool with length + 1 bytes.
void release_string(Cc13SweepBNativeString& text, Cc13SweepB11Host& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

// The inlined wide-string release: length * 2 + 2 bytes.
void release_wide(Cc13SweepB11WideString& text, Cc13SweepB11Host& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length * 2 + 2, 1);
    }
}

// The inlined temporary native string of a C string: {0, 0}, 0041DD40(strlen,
// 1), memcpy of length + 1 bytes when data is non-null.
void make_c_string(Cc13SweepBNativeString& text, const char* source, Cc13SweepB11Host& host) {
    text = {0, nullptr};
    host.native_string_resize_0041dd40(&text, static_cast<std::uint32_t>(std::strlen(source)), 1);
    if (text.data != nullptr) host.crt_memcpy_00bf7680(text.data, source, text.length + 1);
}

// The inlined NativeString assignment: unless dest is source, 0041DD40(dest,
// source length, 1), then when the source length is non-zero a memcpy of the
// (resized) dest length.
void assign_string(void* dest, const void* source, Cc13SweepB11Host& host) {
    if (dest == source) return;
    const std::uint32_t length = *static_cast<const std::uint32_t*>(source);
    host.native_string_resize_0041dd40(dest, length, 1);
    if (length != 0) {
        host.crt_memcpy_00bf7680(at<char*>(dest, 0x4), *reinterpret_cast<char* const*>(
                                     static_cast<const std::uint8_t*>(source) + 0x4),
                                 at<std::uint32_t>(dest, 0x0));
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 00776930 BSP_SessionHostInfo_CheckDlc
// ABI: __thiscall(ECX = host_info; client, unused, WideString* out), RET 0Ch;
// SEH frame (handler 00C89CB0), 598h-byte frame. arg 2 is never read.
// Evidence: the full listing 00776930..0077780F (1125 instructions). Frame
// offsets below are ESP-relative after the PUSH EBX / PUSH EDI at
// 00776A9B / 00776AA0, the frame most of the body runs in.
// Simplifications: the frame objects (LuaStateOwner 4CCh bytes at +E0h,
// LuaObjects 14h bytes, lists 0Ch bytes) are byte blocks of those sizes; the
// compiler helpers __aullshr (00BFAED0) and __allshl (00BF9640) are the C++
// 64-bit shifts (counts 0..63 only); the empty C string 00F1B03C is "".
// ---------------------------------------------------------------------------
void cc13_sweepB11_session_check_dlc_00776930(void* host_info, void* client, std::uint32_t /*unused*/,
                                              Cc13SweepB11WideString* out, Cc13SweepB11Host& host) {
    auto check = [&host]() { host.crt_invalid_parameter_noinfo_00bf6713(); };

    // 0077694B..00776979: the packs the host has and the client lacks, per
    // 32-bit half.
    const std::uint32_t missing_lo = ~at<std::uint32_t>(client, 0x48) & at<std::uint32_t>(host_info, 0x170);
    const std::uint32_t missing_hi = ~at<std::uint32_t>(client, 0x4C) & at<std::uint32_t>(host_info, 0x174);
    const std::uint64_t missing = (static_cast<std::uint64_t>(missing_hi) << 32) | missing_lo;

    // 0077697D..00776A08: a private Lua state that runs the DLC table script.
    alignas(4) std::uint8_t owner[0x4CC];
    host.lua_state_owner_construct_00b66bd0(owner);
    host.lua_state_owner_open_00b6a020(owner, 4);
    {
        Cc13SweepBNativeString path;
        make_c_string(path, "SCRIPTS/datatables/DLC.lua", host);
        host.lua_state_owner_run_script_00b69d40(owner, &path, 0);
        release_string(path, host);
    }

    // 00776A0D..00776A75: table = globals.DLCTable; key and value objects.
    alignas(4) std::uint8_t globals[0x14];
    alignas(4) std::uint8_t table[0x14];
    alignas(4) std::uint8_t key[0x14];
    alignas(4) std::uint8_t value[0x14];
    void* const globals_object = host.lua_globals_00b67980(owner, globals);
    host.lua_object_get_field_00b67800(globals_object, table, "DLCTable");
    host.lua_object_destroy_00b67700(globals);
    host.lua_object_construct_00b65f50(key);
    host.lua_object_construct_00b65f50(value);

    // 00776A79..00776A96: the group list (+5Ch), built and cleared.
    alignas(4) std::uint8_t groups[0xC];
    at<void*>(groups, 0x4) = host.dlc_group_list_buy_head_0076e970(groups);
    at<std::uint32_t>(groups, 0x8) = 0;
    host.dlc_group_list_clear_00776540(groups);

    // 00776AA1..00776D3E: one group per missing bit (signed index 0..3Fh).
    for (std::int32_t bit = 0; bit < 0x40; ++bit) {
        if (((missing >> bit) & 1) == 0) continue;  // __aullshr(CL = bit), AND 1

        // 00776ABF..00776B31: push_back of a copy of an empty record list,
        // then the temporary is cleared and its head freed.
        alignas(4) std::uint8_t fresh[0xC];
        at<void*>(fresh, 0x4) = host.dlc_record_list_buy_head_0076e990(fresh);
        at<std::uint32_t>(fresh, 0x8) = 0;
        {
            void* const head = at<void*>(groups, 0x4);
            void* const node = host.dlc_group_list_buy_node_007767f0(groups, head, at<void*>(head, 0x4), fresh);
            host.dlc_group_list_incsize_007736a0(groups, 1);
            at<void*>(head, 0x4) = node;
            at<void*>(at<void*>(node, 0x4), 0x0) = node;
        }
        host.dlc_record_list_clear_00774050(fresh);
        host.crt_operator_delete_00bf65ac(at<void*>(fresh, 0x4));
        at<void*>(fresh, 0x4) = nullptr;

        // 00776B21..00776B3E: back() of the groups (checked twice when it is
        // the head).
        void* const group = at<void*>(at<void*>(groups, 0x4), 0x4);
        if (group == at<void*>(groups, 0x4)) {
            check();
            if (group == at<void*>(groups, 0x4)) check();
        }
        void* const records = static_cast<std::uint8_t*>(group) + 0x8;  // EBX

        // 00776B43..00776D2A: every DLCTable entry.
        host.lua_object_iterate_first_00b67080(table, key, value);
        while (!host.lua_object_is_unbound_00b66420(table, key)) {
            const char* const bits = host.lua_object_get_string_00b662b0(value);
            if (bits[0x3F - bit] == '1') {
                // 00776B9E..00776BF9: push_back of a zeroed record; ESI = back().
                alignas(4) std::uint8_t blank[0x10] = {};
                {
                    void* const head = at<void*>(records, 0x4);
                    void* const node = host.dlc_record_list_buy_node_007740b0(records, head, at<void*>(head, 0x4), blank);
                    host.dlc_record_list_incsize_00773740(records, 1);
                    at<void*>(head, 0x4) = node;
                    at<void*>(at<void*>(node, 0x4), 0x0) = node;
                }
                void* const record = at<void*>(at<void*>(records, 0x4), 0x4);
                if (record == at<void*>(records, 0x4)) check();
                if (record == at<void*>(records, 0x4)) check();

                // 00776BFE..00776C39: the bit string read MSB first.
                std::uint64_t mask = 0;
                std::int32_t index = 0;
                std::int32_t shift = 0x3F;
                do {
                    if (bits[index] == '1') mask += static_cast<std::uint64_t>(1) << shift;  // __allshl
                    --shift;
                    ++index;
                } while (shift > -1);

                // 00776C3B..00776CE5: record name = the key string (inlined
                // assignment from a C string; a null string has length 0).
                const char* const name = host.lua_object_get_string_00b662b0(key);
                const std::uint32_t length = name != nullptr ? static_cast<std::uint32_t>(std::strlen(name)) : 0;
                std::uint32_t& record_length = at<std::uint32_t>(record, 0x8);
                char*& record_data = at<char*>(record, 0xC);
                if (length != record_length) {
                    if (length == 0) {
                        if (record_data != nullptr) {
                            host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), record_data,
                                                                  record_length + 1, 1);
                        }
                        record_data = nullptr;
                        record_length = 0;
                    } else {
                        char* const block = static_cast<char*>(
                            host.sized_pool_allocate_block_00bd1120(host.sized_pool_get_00419cc0(), length + 1, 1));
                        if (record_data != nullptr) {
                            host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), record_data,
                                                                  record_length + 1, 1);
                        }
                        record_data = block;
                        record_length = length;
                        block[length] = 0;
                    }
                }
                if (record_data != nullptr) host.crt_memcpy_00bf7680(record_data, name, record_length);
                // 00776CE8..00776CF3
                at<std::uint32_t>(record, 0x10) = static_cast<std::uint32_t>(mask);
                at<std::uint32_t>(record, 0x14) = static_cast<std::uint32_t>(mask >> 32);
            }
            host.lua_object_iterate_next_00b67190(table, key, value);
        }
    }

    // 00776D44..0077702D: one record per group into the result list (+38h).
    alignas(4) std::uint8_t result[0xC];
    at<void*>(result, 0x4) = host.dlc_record_list_buy_head_0076e990(result);
    at<std::uint32_t>(result, 0x8) = 0;
    auto push_result = [&](const void* value_record) {
        void* const head = at<void*>(result, 0x4);
        void* const node = host.dlc_record_list_buy_node_007740b0(result, head, at<void*>(head, 0x4), value_record);
        host.dlc_record_list_incsize_00773740(result, 1);
        at<void*>(head, 0x4) = node;
        at<void*>(at<void*>(node, 0x4), 0x0) = node;
    };
    void* group = at<void*>(at<void*>(groups, 0x4), 0x0);
    while (group != at<void*>(groups, 0x4)) {
        if (at<std::uint32_t>(group, 0x10) <= 1) {
            // 00776FCA..0077700C: fewer than two records: copy the first.
            if (group == at<void*>(groups, 0x4)) check();
            void* const first = at<void*>(at<void*>(group, 0xC), 0x0);
            if (first == at<void*>(group, 0xC)) check();
            push_result(static_cast<std::uint8_t*>(first) + 0x8);
        } else {
            // 00776D79..00776E3D: copies of the first two records.
            Cc13SweepBNativeString first_name{0, nullptr};   // +20h
            Cc13SweepBNativeString second_name{0, nullptr};  // +18h
            if (group == at<void*>(groups, 0x4)) check();
            void* const first = at<void*>(at<void*>(group, 0xC), 0x0);
            if (first == at<void*>(group, 0xC)) check();
            assign_string(&first_name, static_cast<std::uint8_t*>(first) + 0x8, host);
            if (first == at<void*>(group, 0xC)) check();
            const std::uint32_t first_lo = at<std::uint32_t>(first, 0x10);
            const std::uint32_t first_hi = at<std::uint32_t>(first, 0x14);
            if (first == at<void*>(group, 0xC)) check();
            void* const second = at<void*>(first, 0x0);
            if (second == at<void*>(group, 0xC)) check();
            assign_string(&second_name, static_cast<std::uint8_t*>(second) + 0x8, host);
            if (second == at<void*>(group, 0xC)) check();
            const std::uint32_t second_lo = at<std::uint32_t>(second, 0x10);
            const std::uint32_t second_hi = at<std::uint32_t>(second, 0x14);

            // 00776E42..00776E74: |first - second| as unsigned 64-bit (first
            // <= second gives second - first).
            const std::uint64_t a = (static_cast<std::uint64_t>(first_hi) << 32) | first_lo;
            const std::uint64_t b = (static_cast<std::uint64_t>(second_hi) << 32) | second_lo;
            const std::uint64_t diff = a <= b ? b - a : a - b;
            const std::uint32_t diff_lo = static_cast<std::uint32_t>(diff);
            const std::uint32_t diff_hi = static_cast<std::uint32_t>(diff >> 32);

            // 00776E76..00776EDC: push_back of a zeroed record; EBP = back().
            alignas(4) std::uint8_t blank[0x10] = {};
            push_result(blank);
            void* const last = at<void*>(at<void*>(result, 0x4), 0x4);
            if (last == at<void*>(result, 0x4)) {
                check();
                if (last == at<void*>(result, 0x4)) check();
            }
            std::uint8_t* const dest = static_cast<std::uint8_t*>(last) + 0x8;

            // 00776EE1..00776F77: the first record when it agrees with the
            // missing mask on every differing bit, else the second.
            if ((diff_lo & first_lo) == (diff_lo & missing_lo) && (diff_hi & first_hi) == (diff_hi & missing_hi)) {
                assign_string(dest, &first_name, host);
                at<std::uint32_t>(dest, 0x8) = first_lo;
                at<std::uint32_t>(dest, 0xC) = first_hi;
            } else {
                assign_string(dest, &second_name, host);
                at<std::uint32_t>(dest, 0x8) = second_lo;
                at<std::uint32_t>(dest, 0xC) = second_hi;
            }
            // 00776F7A..00776FC3
            release_string(second_name, host);
            release_string(first_name, host);
        }
        // 0077700E..0077702D: ++iterator.
        if (group == at<void*>(groups, 0x4)) check();
        group = at<void*>(group, 0x0);
    }

    // 00777032..0077703F
    host.dlc_record_list_sort_007765a0(result);
    host.dlc_record_list_unique_00774280(result);

    // 00777044..0077708A: the key list (+18h) and the resolved text (+68h).
    Cc13SweepBNativeString list_text;
    make_c_string(list_text, "", host);  // 00CE3A0C
    Cc13SweepB11WideString resolved{0, nullptr};
    if (at<std::uint32_t>(result, 0x8) != 0) {
        // 00777090..00777214: key = "globals." + name + "_" + language, for
        // the FIRST record only (no loop over the list).
        void* const first = at<void*>(at<void*>(result, 0x4), 0x0);  // +84h
        Cc13SweepBNativeString language;  // +20h
        make_c_string(language, static_cast<const char*>(client) + 0x32, host);
        Cc13SweepBNativeString separator;  // +10h
        make_c_string(separator, "_", host);  // 00CE7890
        if (first == at<void*>(result, 0x4)) check();
        Cc13SweepBNativeString prefixed;  // +70h
        Cc13SweepBNativeString joined;    // +88h
        Cc13SweepBNativeString dlc_key;   // +54h
        void* text = host.native_string_prefixed_0043c130(&prefixed, "globals.", static_cast<std::uint8_t*>(first) + 0x8);
        text = host.native_string_concat_004261a0(text, &joined, &separator);
        host.native_string_concat_004261a0(text, &dlc_key, &language);
        release_string(joined, host);
        release_string(prefixed, host);
        release_string(separator, host);
        release_string(language, host);

        // 00777214..00777275: resolve the key; unresolved when the text
        // starts with the key itself.
        host.localization_resolve_key_list_00a9fad0(*static_cast<void**>(host.image_global(0x00F8BC4C)), &resolved,
                                                    &dlc_key);
        Cc13SweepB11WideString key_wide;  // +30h
        host.wide_string_construct_004c5e60(&key_wide, dlc_key.data != nullptr ? dlc_key.data : "");
        const bool unresolved = host.wide_string_matches_at_0076e7f0(&resolved, &key_wide, 0);
        release_wide(key_wide, host);

        if (unresolved) {
            // 0077727B..0077743F: list += "globals." + name + "_english|.,|".
            Cc13SweepBNativeString english;  // +10h
            make_c_string(english, "_english|.,|", host);
            Cc13SweepBNativeString globals_prefix;  // +20h
            make_c_string(globals_prefix, "globals.", host);
            Cc13SweepBNativeString step1;  // +44h
            Cc13SweepBNativeString step2;  // +28h
            Cc13SweepBNativeString step3;  // +30h
            void* joined_text = host.native_string_concat_004261a0(&list_text, &step1, &globals_prefix);
            if (first == at<void*>(result, 0x4)) check();
            joined_text = host.native_string_concat_004261a0(joined_text, &step2, static_cast<std::uint8_t*>(first) + 0x8);
            joined_text = host.native_string_concat_004261a0(joined_text, &step3, &english);
            assign_string(&list_text, joined_text, host);
            release_string(step3, host);
            release_string(step2, host);
            release_string(step1, host);
            release_string(globals_prefix, host);
            release_string(english, host);
        } else {
            // 00777444..0077754B: list += key + "|.,|".
            Cc13SweepBNativeString separator_bar;  // +28h
            make_c_string(separator_bar, "|.,|", host);  // 00D03D48
            Cc13SweepBNativeString step1;  // +30h
            Cc13SweepBNativeString step2;  // +44h
            void* joined_text = host.native_string_concat_004261a0(&list_text, &step1, &dlc_key);
            joined_text = host.native_string_concat_004261a0(joined_text, &step2, &separator_bar);
            assign_string(&list_text, joined_text, host);
            release_string(step2, host);
            release_string(step1, host);
            release_string(separator_bar, host);
        }
        // 00777550..00777584
        if (first == at<void*>(result, 0x4)) check();
        release_string(dlc_key, host);
    }

    // 00777589..007775DE: resolve the list without its trailing 4 characters
    // into out (count = length - 4, unsigned wrap when the list is empty).
    {
        Cc13SweepBNativeString keys;  // +44h
        void* const keys_text = host.native_string_substring_00469840(&list_text, &keys, 0,
                                                                      static_cast<std::int32_t>(list_text.length - 4));
        host.localization_resolve_key_list_00a9fad0(*static_cast<void**>(host.image_global(0x00F8BC4C)), out,
                                                    keys_text);
        release_string(keys, host);
    }

    // 007775E3..0077770C: longer than 100 units: the first 90 plus "...".
    if (out->length > 100) {
        Cc13SweepB11WideString dots{0, nullptr};  // +10h
        const char* const dots_text = "...";      // 00CF3F98
        host.wide_string_resize_004c53e0(&dots, static_cast<std::uint32_t>(std::strlen(dots_text)), 1);
        if (dots.data != nullptr) {
            std::uint16_t* unit = dots.data;
            const unsigned char* source = reinterpret_cast<const unsigned char*>(dots_text);
            std::uint16_t c;
            do {
                c = *source++;
                *unit++ = c;
            } while (c != 0);
        }
        Cc13SweepB11WideString head_part;  // +70h
        Cc13SweepB11WideString joined;     // +44h
        void* const part = host.wide_string_substring_00770e50(out, &head_part, 0, 0x5A);
        auto* const whole = static_cast<Cc13SweepB11WideString*>(host.wide_string_concat_005cf570(part, &joined, &dots));
        if (out != whole) {
            host.wide_string_resize_004c53e0(out, whole->length, 1);
            if (whole->length != 0) {
                const std::uint16_t* source = whole->data;
                std::uint16_t* unit = out->data;
                std::uint16_t c;
                do {
                    c = *source++;
                    *unit++ = c;
                } while (c != 0);
            }
        }
        release_wide(joined, host);
        release_wide(head_part, host);
        release_wide(dots, host);
    }

    // 00777711..007777F4: teardown.
    release_wide(resolved, host);
    release_string(list_text, host);
    host.dlc_record_list_clear_00774050(result);
    host.crt_operator_delete_00bf65ac(at<void*>(result, 0x4));
    at<void*>(result, 0x4) = nullptr;
    host.dlc_group_list_clear_00776540(groups);
    host.crt_operator_delete_00bf65ac(at<void*>(groups, 0x4));
    at<void*>(groups, 0x4) = nullptr;
    host.lua_object_destroy_00b67700(value);
    host.lua_object_destroy_00b67700(key);
    host.lua_object_destroy_00b67700(table);
    host.lua_state_owner_close_00b669a0(owner);
}

}  // namespace bsp
