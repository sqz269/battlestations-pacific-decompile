#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual MenuPrompt construction requires MSVC Win32.
#endif

namespace bsp::game { class GameNativeReadOnlyData; }
namespace bsp {
struct NativeStringRawPoolContext;

// Pointer metadata borrowed from the existing authenticated original-VA RO
// supplier. No copied literal bank or default/null supplier can be constructed.
// Prepare this outside native entry; retain the supplier and these refs through
// construction, callbacks and unwind. The factory checks all fourteen exact
// original address/length/content fingerprints against the verified PE mapping.
class NativeMenuPromptConstructionLiterals final {
public:
    const char* label(std::size_t index) const noexcept { return labels_[index]; }
private:
    NativeMenuPromptConstructionLiterals(const game::GameNativeReadOnlyData& data,
        const std::array<const char*, 14>& labels) noexcept : data_(data), labels_(labels) {}
    friend NativeMenuPromptConstructionLiterals borrow_native_menu_prompt_literals(
        const game::GameNativeReadOnlyData&);
    const game::GameNativeReadOnlyData& data_;
    std::array<const char*, 14> labels_;
};
NativeMenuPromptConstructionLiterals borrow_native_menu_prompt_literals(
    const game::GameNativeReadOnlyData&);

// Borrow genuine stable cells from the one retained application's authority.
// screen_words identifies exactly95 actual DWORD cells; the sentinel is not
// part of it. prompt_publication is E18E6C, not E1930C. strings carries the SAME
// actual manager publication used by the getter and actual pooled-string owner.
struct NativeMenuPromptConstructionContext {
    NativeStringRawPoolContext& strings;
    const NativeMenuPromptConstructionLiterals& literals;
    void* volatile& actual_prompt_publication_00e18e6c;
    volatile std::uint32_t* actual_screen_words_00e18b60;
    const volatile std::uint32_t& actual_feature_word_0109eea4;
};

// Preprepared, nonowning per-invocation metadata. No allocations, native stores
// or cleanup occur in its construction/destruction. A fresh frame is required
// for every invocation/reentry. Failed frames are diagnostic, never replayed;
// the application retains frames/providers and any still-live backing until
// resolved. Getter-freed allocation addresses are diagnostic only, never read.
struct NativeMenuPromptConstructionOperation final {
    enum class Phase { fresh, running, complete, failed };
    Phase phase{Phase::fresh};
    void* captured_receiver{};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    std::uint32_t failure_native_site{};
    std::int32_t failure_unwind_state{-1};
    std::uint32_t completed_vectors{};
    std::uint32_t completed_records{};
    std::uint32_t temporary_header[2]; // Initialized only at each native temp epoch.
    void* captured_temporary_data{};
    void* cleanup_list{};
    void* cleanup_list_head{};
    void* cleanup_list_node{};
    void* cleanup_list_next{};
    void* cleanup_record{};
    std::int32_t cleanup_record_state{-1};
    void* cleanup_vector{};
    std::uint32_t cleanup_records_remaining{};
    std::uint32_t cleanup_vectors_remaining{};
    bool unwind_complete{};
    bool unwind_failed{};
    NativeMenuPromptConstructionOperation() noexcept = default;
    NativeMenuPromptConstructionOperation(const NativeMenuPromptConstructionOperation&) = delete;
    NativeMenuPromptConstructionOperation& operator=(const NativeMenuPromptConstructionOperation&) = delete;
};

// Whole00533120..0053378D,1646B/513ops. Native ECX genuine290h raw receiver,
// EAX same receiver, bareRET. No stacked semantic input or incomingEDX is used.
// Compose the admitted actual4F7180/530F40, real raw strings/vector append and
// cleanup, exact list allocation/link stores and native state2/3/5/6/7..20
// epochs. Normal temp returns use captured data; unwind uses CURRENT header.
// +258..264 and other native-untouched bytes are not initialized here.
void* construct_native_menu_prompt_screen_00533120(void* actual_receiver,
    NativeMenuPromptConstructionContext&, NativeMenuPromptConstructionOperation&);

// New ordinary Source ABI/private scratch and C++ catch transport. No original
// FH3/SEH, hardware-fault cleanup, secondary-unwind/CRT identity or arbitrary
// private-frame alias proof. Valid raw backing/cells/providers are caller proof.
// No raw screen registration, GUI Page work or game activation is added.
} // namespace bsp
