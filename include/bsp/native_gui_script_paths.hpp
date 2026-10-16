#pragma once

#include <cstdint>

namespace bsp {

struct NativeStringRawPoolContext;

// Required borrowed identities from one actual string-pool/publication domain.
// Literals are the actual D5CB18 "interface/" and CFD2C8 ".lua" byte views,
// including their NULs. No default literals, semantic allocator or pool owner.
struct NativeGuiScriptPathContext {
    NativeStringRawPoolContext& strings;
    const void* actual_interface_prefix_00d5cb18;
    const void* actual_lua_suffix_00cfd2c8;
};

enum class NativeGuiScriptPathPhase { fresh, entered, complete, failed };
struct NativeGuiScriptPathObservation {
    NativeGuiScriptPathPhase phase;
    std::int32_t remaining_unwind_state;
    bool output_cleanup_armed;
    const void* output;
    const void* by_value_name;
    const void* suffix_header;
    const void* prefix_header;
    const void* joined_header;
    const void* cached_prefix_data;
};

// Caller-owned persistent Source frame, not the Original private stack layout.
// No implicit cleanup or destructor refusal: retention is a caller lifetime
// contract; an enclosing owner must guard its whole graph before unwinding.
// Retain the SAME frame and borrowed runtime domain after an entered failure,
// including any cleanup failure. Actions are consumed before
// their throwing getter; there is no retry/reset/resume or ownership-release API.
// Calls and raw-domain mutation must be externally serialized. Native-private
// frame aliases, FH3/double-exception behavior and stack-byte parity are excluded.
class NativeGuiScriptPathFrame final {
public:
    NativeGuiScriptPathFrame() noexcept = default;
    NativeGuiScriptPathFrame(const NativeGuiScriptPathFrame&) = delete;
    NativeGuiScriptPathFrame& operator=(const NativeGuiScriptPathFrame&) = delete;
    NativeGuiScriptPathObservation observation() const noexcept;
    bool requires_process_retention() const noexcept;
private:
    std::uint32_t suffix_[2]{};
    std::uint32_t prefix_[2]{};
    std::uint32_t joined_[2]{};
    void* output_{};
    void* by_value_name_{};
    void* cached_prefix_data_{};
    std::int32_t unwind_state_{-1};
    bool output_owned_{};
    NativeGuiScriptPathPhase phase_{NativeGuiScriptPathPhase::fresh};
    void unwind(NativeGuiScriptPathContext&);
    friend void* compose_native_gui_script_path_00ac5600(void*, void*,
        NativeGuiScriptPathContext&, NativeGuiScriptPathFrame&);
};

struct NativeGuiLayerScriptPathObservation {
    NativeGuiScriptPathPhase phase;
    bool name_transferred;
    const void* page;
    const void* output;
    const void* copied_name_header;
    NativeGuiScriptPathObservation compose;
};

// Owns AC5770's raw8h copy before AC5600 adopts that exact header. No invented
// pre-transfer wrapper EH cleanup. Keep this frame/domain on entered failure;
// after transfer only the nested callee's recorded cleanup owns that input.
// The same caller lifetime contract applies; destruction is not refused here.
class NativeGuiLayerScriptPathFrame final {
public:
    NativeGuiLayerScriptPathFrame() noexcept = default;
    NativeGuiLayerScriptPathFrame(const NativeGuiLayerScriptPathFrame&) = delete;
    NativeGuiLayerScriptPathFrame& operator=(const NativeGuiLayerScriptPathFrame&) = delete;
    NativeGuiLayerScriptPathObservation observation() const noexcept;
    bool requires_process_retention() const noexcept;
private:
    std::uint32_t copied_name_[2]{};
    NativeGuiScriptPathFrame compose_;
    const void* page_{};
    void* output_{};
    bool name_transferred_{};
    NativeGuiScriptPathPhase phase_{NativeGuiScriptPathPhase::fresh};
    friend void* compose_native_gui_layer_script_path_00ac5770(const void*, void*,
        NativeGuiScriptPathContext&, NativeGuiLayerScriptPathFrame&);
};

// Complete AC5600..AC576E body: Original ECX=actual OUT8h, stack BY-VALUE8h
// name, EAX=out, RET8. Fresh-frame rejection precedes adoption: the caller still
// owns its input on that Source preflight failure. Once admitted, consume the
// supplied SAME by-value header; do not pass a borrowed page/header owner.
// Output is armed only after concat2 returns. Success hands output to caller
// without modifying its header; existing output storage is abandoned by concat.
// C++ unwind consumes remaining map actions until a cleanup throws, where the
// newer exception escapes and remaining state stays retained, without retry.
// No private Native FH3/SEH or binary replacement ABI equivalence is claimed.
void* compose_native_gui_script_path_00ac5600(void* actual_output,
    void* actual_by_value_name, NativeGuiScriptPathContext&, NativeGuiScriptPathFrame&);

// Complete AC5770..AC57D2 body: ECX=actual page; stack out; EAX=out; RET4.
// Copies the current raw name at page+100h, then transfers its exact local8h
// copy to AC5600. Added context/frame/preflight and normal handoff metadata are
// Source contracts, not Original arguments, allocations or stack observations.
void* compose_native_gui_layer_script_path_00ac5770(const void* actual_page,
    void* actual_output, NativeGuiScriptPathContext&, NativeGuiLayerScriptPathFrame&);

}  // namespace bsp
