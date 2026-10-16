#include "bsp/native_gui_script_paths.hpp"

#if !defined(_MSC_VER) || !defined(_WIN32) || !defined(_M_IX86)
#error Native GUI script paths require the MSVC Win32 x86 target
#endif
static_assert(sizeof(void*) == 4, "Native GUI script paths require actual8h Win32 headers");

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
std::uint32_t word(const void* header) noexcept {
    std::uint32_t value;
    std::memcpy(&value, header, sizeof(value));
    return value;
}
void* data(const void* header) noexcept {
    void* value;
    std::memcpy(&value, static_cast<const unsigned char*>(header) + 4, sizeof(value));
    return value;
}
void clear(void* header) noexcept {
    const std::uint32_t zero = 0;
    std::memcpy(header, &zero, 4);
    std::memcpy(static_cast<unsigned char*>(header) + 4, &zero, 4);
}
void return_captured(void* block, std::uint32_t size, NativeStringRawPoolContext& strings) {
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, block, size,
        strings.actual_small_returns_disabled_01090aa4);
}
void copy_literal(void* header, const void* literal) noexcept {
    auto* const destination = data(header);
    if (destination != nullptr) {
        const auto count = word(header) + 1u;
        // Preserve BF7680 overlap behavior and the existing zero-byte omission.
        if (count != 0) std::memmove(destination, literal, count);
    }
}
}  // namespace

NativeGuiScriptPathObservation NativeGuiScriptPathFrame::observation() const noexcept {
    return {phase_, unwind_state_, output_owned_, output_, by_value_name_,
        suffix_, prefix_, joined_, cached_prefix_data_};
}
bool NativeGuiScriptPathFrame::requires_process_retention() const noexcept {
    return phase_ == NativeGuiScriptPathPhase::entered || phase_ == NativeGuiScriptPathPhase::failed;
}
void NativeGuiScriptPathFrame::unwind(NativeGuiScriptPathContext& context) {
    while (unwind_state_ >= 0) {
        const auto action = unwind_state_;
        --unwind_state_; // DEF860: consume before the potentially throwing getter.
        switch (action) {
        case 4: destroy_native_string_header_0041dd20(joined_, context.strings); break;
        case 3: destroy_native_string_header_0041dd20(prefix_, context.strings); break;
        case 2: destroy_native_string_header_0041dd20(suffix_, context.strings); break;
        case 1: destroy_native_string_header_0041dd20(by_value_name_, context.strings); break;
        case 0:
            if (output_owned_) {
                output_owned_ = false; // CB888C, BEFORE loading the output header.
                auto* const output = output_;
                destroy_native_string_header_0041dd20(output, context.strings);
            }
            break;
        }
    }
}

void* compose_native_gui_script_path_00ac5600(void* actual_output,
    void* actual_by_value_name, NativeGuiScriptPathContext& context, NativeGuiScriptPathFrame& frame) {
    if (frame.phase_ != NativeGuiScriptPathPhase::fresh)
        throw std::logic_error("Native GUI script-path frame is not fresh");
    frame.output_ = actual_output;
    frame.by_value_name_ = actual_by_value_name;
    frame.phase_ = NativeGuiScriptPathPhase::entered;
    frame.unwind_state_ = 1; // AC562E: own the by-value input before suffix resize.
    try {
        clear(frame.suffix_);
        resize_native_string_header_0041dd40(frame.suffix_, context.strings, 4, true);
        copy_literal(frame.suffix_, context.actual_lua_suffix_00cfd2c8);

        frame.unwind_state_ = 2;
        clear(frame.prefix_);
        resize_native_string_header_0041dd40(frame.prefix_, context.strings, 10, true);
        frame.cached_prefix_data_ = data(frame.prefix_); // AC567C ESI, before copy/concat.
        if (frame.cached_prefix_data_ != nullptr) {
            const auto count = word(frame.prefix_) + 1u;
            if (count != 0) std::memmove(frame.cached_prefix_data_,
                context.actual_interface_prefix_00d5cb18, count);
        }

        frame.unwind_state_ = 3;
        auto* const joined = concatenate_native_string_headers_004261a0(
            frame.prefix_, frame.joined_, actual_by_value_name, context.strings);
        frame.unwind_state_ = 4;
        concatenate_native_string_headers_004261a0(joined, actual_output, frame.suffix_, context.strings);

        auto* const joined_data = data(frame.joined_); // AC56C4, before output bit.
        frame.output_owned_ = true; // AC56CA: ONLY after second concat returns.
        frame.unwind_state_ = 3;
        if (joined_data != nullptr)
            return_captured(joined_data, word(frame.joined_) + 1u, context.strings);

        frame.unwind_state_ = 2;
        if (frame.cached_prefix_data_ != nullptr)
            return_captured(frame.cached_prefix_data_, word(frame.prefix_) + 1u, context.strings);

        auto* const suffix_data = data(frame.suffix_); // Current, after prefix return.
        frame.unwind_state_ = 1;
        if (suffix_data != nullptr)
            return_captured(suffix_data, word(frame.suffix_) + 1u, context.strings);

        auto* const input_data = data(actual_by_value_name); // Current, after suffix return.
        frame.unwind_state_ = 0;
        if (input_data != nullptr)
            return_captured(input_data, word(actual_by_value_name) + 1u, context.strings);

        // Source handoff metadata only; Original returns with its private bit set.
        frame.output_owned_ = false;
        frame.unwind_state_ = -1;
        frame.phase_ = NativeGuiScriptPathPhase::complete;
        return actual_output;
    } catch (...) {
        frame.phase_ = NativeGuiScriptPathPhase::failed;
        // A cleanup exception replaces the current exception; its action was
        // already consumed and lower actions remain recorded in the SAME frame.
        frame.unwind(context);
        throw;
    }
}

NativeGuiLayerScriptPathObservation NativeGuiLayerScriptPathFrame::observation() const noexcept {
    return {phase_, name_transferred_, page_, output_, copied_name_, compose_.observation()};
}
bool NativeGuiLayerScriptPathFrame::requires_process_retention() const noexcept {
    return phase_ == NativeGuiScriptPathPhase::entered || phase_ == NativeGuiScriptPathPhase::failed;
}
void* compose_native_gui_layer_script_path_00ac5770(const void* actual_page,
    void* actual_output, NativeGuiScriptPathContext& context, NativeGuiLayerScriptPathFrame& frame) {
    if (frame.phase_ != NativeGuiScriptPathPhase::fresh)
        throw std::logic_error("Native GUI layer script-path frame is not fresh");
    frame.page_ = actual_page;
    frame.output_ = actual_output;
    frame.phase_ = NativeGuiScriptPathPhase::entered;
    try {
        const auto* const source = static_cast<const unsigned char*>(actual_page) + 0x100;
        const bool identical = static_cast<const void*>(frame.copied_name_) == source;
        clear(frame.copied_name_);
        if (!identical) {
            resize_native_string_header_0041dd40(frame.copied_name_, context.strings, word(source), true);
            if (word(source) != 0) { // AC57A7: current source length, after resize.
                const auto count = word(frame.copied_name_); // AC57AC
                const auto* const input = data(source); // AC57AE BEFORE output.data.
                auto* const output = data(frame.copied_name_); // AC57B1
                if (count != 0) std::memmove(output, input, count);
            }
        }
        // Nested frame is necessarily fresh; wrapper has no pre-transfer EH arm.
        frame.name_transferred_ = true;
        compose_native_gui_script_path_00ac5600(actual_output, frame.copied_name_, context, frame.compose_);
        frame.phase_ = NativeGuiScriptPathPhase::complete;
        return actual_output;
    } catch (...) {
        frame.phase_ = NativeGuiScriptPathPhase::failed;
        throw; // No guessed wrapper cleanup, before or after callee adoption.
    }
}

}  // namespace bsp
