#include "bsp/native_lua_dot_path.hpp"
#include "bsp/native_crt_strcspn.hpp"
#include "bsp/native_lua_reader_support.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <intrin.h>
#include <stdexcept>

extern "C" {
#include <lua.h>
}

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4 && sizeof(int) == 4);

Word word(const void* storage, std::size_t offset = 0) noexcept {
    Word result;
    _ReadWriteBarrier();
    std::memcpy(&result, static_cast<const unsigned char*>(storage) + offset, 4);
    _ReadWriteBarrier();
    return result;
}
void put(void* storage, std::size_t offset, Word value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(static_cast<unsigned char*>(storage) + offset, &value, 4);
    _ReadWriteBarrier();
}
void* address(Word value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
std::int32_t signed_word(Word bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof result);
    return result;
}
bool actual_span(const NativeLuaDotPathLiteralSpan& span,
    std::uintptr_t expected, NativeLuaDotPathLiteralRole role) noexcept {
    return span.original_address == expected && span.bytes == 2 &&
        span.role == role && reinterpret_cast<std::uintptr_t>(span.data) == expected;
}
bool current_table(const NativeLuaObjectStorage& object) {
    const auto kind = word(&object, 4);
    if (kind == 0) return false;
    if (kind != 2) return true;
    const auto* owner = address(word(&object));
    auto* const state = static_cast<lua_State*>(address(word(owner, 4)));
    const auto index = signed_word(word(&object, 8));
    return lua_type(state, index) == LUA_TTABLE;
}
bool current_nil(const NativeLuaObjectStorage& object) {
    const auto kind = word(&object, 4);
    if (kind != 2) return false;
    const auto* owner = address(word(&object));
    auto* const state = static_cast<lua_State*>(address(word(owner, 4)));
    const auto index = signed_word(word(&object, 8));
    return lua_type(state, index) == LUA_TNIL;
}
} // namespace

NativeLuaDotPathContext::NativeLuaDotPathContext(
    NativeLuaObjectStorage& root, void* output, const void* path,
    std::uint8_t* status, NativeStringRawPoolContext& strings,
    NativeLuaDotPathLiteralSpan dot, NativeLuaDotPathLiteralSpan zero,
    char* empty)
    : root_(root), output_(output), path_(path), status_(status), strings_(strings),
      dot_(dot), zero_(zero), empty_(empty) {
    // Binding metadata only: no literal, mutable-empty, header, cell or VM read.
    if (!output || !path || !status ||
        !actual_span(dot, 0x00ce3a70u, NativeLuaDotPathLiteralRole::dot) ||
        !actual_span(zero, 0x00ce43bcu, NativeLuaDotPathLiteralRole::zero) ||
        reinterpret_cast<std::uintptr_t>(empty) != 0x0108ff2cu)
        throw std::invalid_argument("raw Lua dot path requires actual storage and mapped roles");
}

NativeLuaDotPathFrame::NativeLuaDotPathFrame(
    const NativeLuaDotPathContext& context) noexcept : context_(context) {}

void NativeLuaDotPathFrame::fail() noexcept {
    failed_ = true;
    phase_ = Phase::failed;
    progress_.failed = true;
}
[[noreturn]] void NativeLuaDotPathFrame::reject(const char* text) {
    fail();
    throw std::logic_error(text);
}
void NativeLuaDotPathFrame::require_active() {
    if (phase_ != Phase::active || failed_)
        reject("raw Lua dot path invocation is not healthy and active");
}
void NativeLuaDotPathFrame::set_state(std::int32_t state) noexcept {
    state_ = state;
    progress_.selected_state = state;
}
void NativeLuaDotPathFrame::write_status(std::uint8_t value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(context_.status_, &value, 1);
    _ReadWriteBarrier();
    if (value != 0) progress_.status_one_written = true;
    else progress_.status_zero_written = true;
}
void NativeLuaDotPathFrame::resize(void* header, Word length) {
    ++progress_.resize_entered;
    resize_native_string_header_0041dd40(header, context_.strings_, length, true);
    ++progress_.resize_returned;
    require_active();
}
void NativeLuaDotPathFrame::assign_inline(void* destination, const void* source,
    Word& saved_destination_data) {
    const auto requested = word(source);
    resize(destination, requested);
    const auto current_source_length = word(source);
    saved_destination_data = word(destination, 4); // even when the guard is zero
    if (current_source_length != 0) {
        const auto copied = word(destination);
        const auto source_data = word(source, 4); // DEST data was captured first
        if (copied != 0)
            std::memmove(address(saved_destination_data), address(source_data), copied);
    }
    require_active();
}
void NativeLuaDotPathFrame::return_captured(Word data, Word bytes) {
    ++progress_.getter_entered;
    last_pool_ = native_string_pool_get_or_create_00419cc0(
        context_.strings_.actual_published_01090aa8,
        context_.strings_.actual_manager_publication_01090aa0);
    ++progress_.getter_returned;
    require_active(); // caught reentry cannot proceed from getter to block return
    ++progress_.block_return_entered;
    return_native_string_pool_00bd1510(last_pool_, address(data), bytes,
        context_.strings_.actual_small_returns_disabled_01090aa4);
    ++progress_.block_return_returned;
    // Caller performs immediate Native post-return writes BEFORE its next fence.
}
void NativeLuaDotPathFrame::normal_lua_release(NativeLuaObjectStorage& object) {
    require_active();
    if (word(&object, 4) != 0) {
        const auto index = signed_word(word(&object, 8));
        auto* const owner = static_cast<NativeLuaStateStorage*>(address(word(&object)));
        ++progress_.normal_lua_release_entered;
        release_native_lua_tracked_object_00b66de0(owner, object, index, 1);
        ++progress_.normal_lua_release_returned;
        // Deliberately NO B67700/current-kind clear on this normal inline edge.
        require_active();
    }
}
void NativeLuaDotPathFrame::unwind_selected() noexcept {
    try {
        while (state_ >= 0) {
            switch (state_) {
            case 4:
                set_state(2);
                ++progress_.selected_actions_entered;
                destroy_native_string_header_0041dd20(suffix_, context_.strings_);
                ++progress_.selected_actions_returned;
                break;
            case 3:
                set_state(2);
                ++progress_.selected_actions_entered;
                destroy_native_string_header_0041dd20(prefix_, context_.strings_);
                ++progress_.selected_actions_returned;
                break;
            case 2:
                set_state(1);
                ++progress_.selected_actions_entered;
                destroy_native_string_header_0041dd20(segment_, context_.strings_);
                ++progress_.selected_actions_returned;
                break;
            case 1:
                set_state(0);
                ++progress_.selected_actions_entered;
                destroy_native_string_header_0041dd20(rest_, context_.strings_);
                ++progress_.selected_actions_returned;
                break;
            default: // state0 -> -1, conditional output bit consumed first
                set_state(-1);
                if (output_action_armed_) {
                    output_action_armed_ = false;
                    ++progress_.selected_actions_entered;
                    destroy_native_lua_object_00b67700(*output_);
                    ++progress_.selected_actions_returned;
                }
                break;
            }
        }
    } catch (...) {
        std::terminate();
    }
}
void NativeLuaDotPathFrame::complete_and_transfer() {
    require_active();
    output_action_armed_ = false; // private transfer, not an Original-stack store
    normal_result_ = true;
    progress_.completed = true;
    phase_ = Phase::returned;
}

NativeLuaObjectStorage* resolve_native_lua_dot_path_00b68d70(
    const NativeLuaDotPathContext& c, NativeLuaDotPathFrame& f) {
    // Rejected inner entry cannot execute the admitted outer invocation's actions.
    if (&f.context_ != &c || f.phase_ != NativeLuaDotPathFrame::Phase::fresh)
        f.reject("raw Lua dot path Frame mismatch, reentry or replay");
    f.phase_ = NativeLuaDotPathFrame::Phase::active;
    f.progress_.attempted = true;
    try {
        f.progress_.output_copy_entered = true;
        f.output_ = copy_construct_native_lua_object_00b66fa0(c.output_, c.root_);
        f.progress_.output_copy_returned = true;
        f.output_action_armed_ = true;
        f.set_state(0);
        f.require_active();

        const bool initial_self = static_cast<const void*>(f.rest_) == c.path_;
        put(f.rest_, 0, 0);
        put(f.rest_, 4, 0);
        if (!initial_self) {
            f.assign_inline(f.rest_, c.path_, f.rest_data_);
            f.rest_length_ = word(f.rest_);
        }
        f.progress_.initial_rest_assigned = true;
        f.set_state(1);
        f.write_status(1);
        f.require_active();

        while (f.rest_length_ != 0) {
            ++f.progress_.iterations;
            f.segment_data_ = 0;
            put(f.segment_, 0, 0);
            put(f.segment_, 4, 0);
            f.set_state(2);
            bool split = false;
            if (f.rest_data_ != 0 && f.rest_length_ != 0) {
                ++f.progress_.scan_entered;
                f.split_index_ = count_native_crt_prefix_excluding_00bf9a50(
                    address(f.rest_data_), c.dot_.data);
                ++f.progress_.scan_returned;
                f.require_active();
                const auto current_rest_length = word(f.rest_);
                if (f.split_index_ != current_rest_length &&
                    (f.split_index_ & 0x80000000u) == 0)
                    split = true;
                else
                    f.rest_length_ = word(f.rest_); // Native no-split reload
            }
            if (split) {
                ++f.progress_.substring_entered;
                f.last_substring_ = construct_native_string_substring_00469840(
                    f.rest_, f.prefix_, 0, f.split_index_, c.strings_);
                ++f.progress_.substring_returned;
                const bool self = static_cast<void*>(f.segment_) == f.last_substring_;
                f.set_state(3);
                f.require_active();
                if (!self)
                    f.assign_inline(f.segment_, f.last_substring_, f.segment_data_);
                const auto prefix_data = word(f.prefix_, 4);
                f.set_state(2); // consume prefix BEFORE its normal getter/return
                if (prefix_data != 0) {
                    const auto bytes = word(f.prefix_) + 1u;
                    f.return_captured(prefix_data, bytes);
                    f.require_active();
                }

                ++f.progress_.substring_entered;
                f.last_substring_ = construct_native_string_substring_00469840(
                    f.rest_, f.suffix_, f.split_index_ + 1u, 0x7fffffffu, c.strings_);
                ++f.progress_.substring_returned;
                const bool suffix_self = static_cast<void*>(f.rest_) == f.last_substring_;
                f.set_state(4);
                f.require_active();
                if (!suffix_self)
                    f.assign_inline(f.rest_, f.last_substring_, f.rest_data_);
                const auto suffix_data = word(f.suffix_, 4);
                f.set_state(2);
                if (suffix_data != 0) {
                    const auto bytes = word(f.suffix_) + 1u;
                    f.return_captured(suffix_data, bytes);
                    f.require_active();
                }
                f.rest_length_ = word(f.rest_);
            } else {
                f.resize(f.segment_, f.rest_length_);
                if (f.rest_length_ != 0) {
                    const auto copied = word(f.segment_);
                    f.segment_data_ = word(f.segment_, 4);
                    if (copied != 0)
                        std::memmove(address(f.segment_data_), address(f.rest_data_), copied);
                    f.require_active();
                    if (f.rest_data_ != 0)
                        f.return_captured(f.rest_data_, f.rest_length_ + 1u);
                    // This rest edge stays state2 through the real normal return.
                    // Only then perform Native DATA0 followed by LENGTH0.
                    f.rest_data_ = 0;
                    put(f.rest_, 4, 0);
                    put(f.rest_, 0, 0);
                    f.require_active();
                    f.rest_length_ = word(f.rest_);
                } else {
                    // Native's reached memcpy(savedRestData,CE3A0C,0) is omitted;
                    // it has no payload read or replacement-literal authority.
                    f.segment_data_ = word(f.segment_, 4);
                }
            }

            if (word(f.segment_) == 0) {
                f.write_status(0);
                f.set_state(1);
                if (f.segment_data_ != 0) {
                    f.return_captured(f.segment_data_, 1); // EXACT empty-arm size
                    f.require_active();
                }
                break;
            }
            const bool table = current_table(*f.output_);
            f.require_active();
            if (!table) {
                f.write_status(0); // still execute the remaining-path loop
            } else {
                // B68100's actual raw normalization, not a NativeString cast:
                // CURRENT key DATA then null-only ACTUAL mutable0108FF2C.
                auto* key = static_cast<const char*>(address(word(f.segment_, 4)));
                if (!key) key = c.empty_;
                ++f.progress_.string_lookup_entered;
                f.lookup_ = native_lua_get_by_name_00b67800(*f.output_, f.lookup_bytes_, key);
                ++f.progress_.string_lookup_returned;
                f.require_active();
                const bool nil = current_nil(*f.lookup_);
                f.require_active();
                if (nil) {
                    auto* text = static_cast<const char*>(address(f.segment_data_));
                    if (!text) text = c.empty_;
                    const auto parsed = std::atol(text); // qualified CURRENT CRT
                    f.numeric_index_ = signed_word(static_cast<Word>(parsed));
                    f.require_active();
                    bool numeric = f.numeric_index_ != 0;
                    if (!numeric && f.segment_data_ != 0) {
                        const auto compared = ::_stricmp(
                            static_cast<const char*>(address(f.segment_data_)),
                            static_cast<const char*>(c.zero_.data));
                        numeric = compared == 0;
                        f.require_active();
                    }
                    if (numeric) {
                        ++f.progress_.index_lookup_entered;
                        f.index_ = native_lua_get_by_index_00b67720(
                            *f.output_, f.index_bytes_, f.numeric_index_);
                        ++f.progress_.index_lookup_returned;
                        f.require_active();
                        ++f.progress_.assignment_entered;
                        f.last_assignment_ = assign_native_lua_object_00b67690(*f.lookup_, *f.index_);
                        ++f.progress_.assignment_returned;
                        f.require_active();
                        f.normal_lua_release(*f.index_);
                    }
                }
                ++f.progress_.assignment_entered;
                f.last_assignment_ = assign_native_lua_object_00b67690(*f.output_, *f.lookup_);
                ++f.progress_.assignment_returned;
                f.require_active();
                f.normal_lua_release(*f.lookup_);
            }
            f.set_state(1);
            if (f.segment_data_ != 0) {
                const auto bytes = word(f.segment_) + 1u;
                f.return_captured(f.segment_data_, bytes);
                f.require_active();
            }
        }
        f.set_state(0);
        if (f.rest_data_ != 0) {
            f.return_captured(f.rest_data_, f.rest_length_ + 1u);
            f.require_active();
        }
        f.complete_and_transfer();
        return f.output_;
    } catch (...) {
        f.fail();
        f.unwind_selected();
        throw;
    }
}

} // namespace bsp
