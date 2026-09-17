#pragma once
#include "bsp/native_string.hpp"
#include "bsp/native_mission_progress_owner.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
// Game+650..747 is the observed F8-byte profile slot. Neither entry clears
// opaque bytes. In particular construction does not initialize +64 before
// reset examines it; native game startup supplies its zeroed allocation.
struct alignas(4) NativePlayerProfileStorage final { std::byte bytes[0xf8]; };
static_assert(sizeof(NativePlayerProfileStorage)==0xf8);
static_assert(std::is_trivially_default_constructible_v<NativePlayerProfileStorage>);

// Known leaves have concrete existing source bindings. Other bodies are
// required services, with actual raw receivers; no projected profile or
// success fallback. Library allocation/collection calls remain contracts.
struct NativePlayerProfileCalls : NativeProfileCollectionCalls {
    virtual ~NativePlayerProfileCalls()=default;
    virtual void* call_004c3020();
    virtual void* call_007f82f0();
    virtual void* call_004c26b0();
    virtual void* call_007f8540();
    virtual void* call_005826b0();
    virtual void call_004cec60(void* tree,void* node,NativeStringStorage&);
    virtual void call_0058b520(void* tree,void* node,NativeStringStorage&);
    virtual void call_00bf6713();
    virtual void* call_004954f0(void* vector,void* output,void* first_owner,
        void* first,void* last_owner,void* last,NativeStringStorage&);
    virtual void call_007fa880(void* tree,void* node,NativeStringStorage&);
    virtual std::uint32_t* call_005070c0(void* map,const void* key_header)=0;
    virtual void call_004d05e0(void* list,NativeStringStorage&);
    virtual void* call_007f8390(void* next,void* previous,const std::uint8_t* gate);
    virtual std::uint32_t call_007fa3a0(void* list,std::uint32_t increment);
    virtual void call_008d4820(void* actual_settings_00f88980)=0;
    virtual void call_008d41c0(void* actual_settings_00f88980)=0;
};
struct NativePlayerProfileContext {
    NativeStringStorage& strings;
    NativePlayerProfileCalls& calls;
    void* const actual_settings_00f88980;
    const char* const empty_00ce3a0c;
    const char* const new_player_00cef794;
    const char* const rank_00cef15c;
};
struct NativePlayerProfileOperation final {
    enum class Phase { fresh,running,complete,failed,diagnostic_retired };
    Phase phase{Phase::fresh};
    void* owner{};
    NativePlayerProfileContext* context{};
    bool construction{};
    std::uint32_t constructor_site{};
    std::int32_t constructor_unwind_state{-1};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    NativeString temporary;
    void* current_allocation{};
    char* captured_rank_buffer{};
    std::uint32_t iterator_output[2];
    std::uint32_t captured_last{};
    std::uint32_t captured_first{};
    std::uint32_t lobby_index{};
    std::uint8_t gate;
    NativeMissionProgressOperation progress_destruction;
    NativeMissionProgressOperation progress_construction;
    NativePlayerProfileOperation() noexcept=default;
    ~NativePlayerProfileOperation();
    NativePlayerProfileOperation(const NativePlayerProfileOperation&)=delete;
    NativePlayerProfileOperation& operator=(const NativePlayerProfileOperation&)=delete;
    void acknowledge_diagnostic_cleanup() noexcept;
};

// 436710: ECX output header, EAX same header, RET. Literal is borrowed from
// the actual image; 41E870 performs the header initialization before strlen.
NativeString* construct_native_profile_empty_name_00436710(
    NativeString& output,NativeStringStorage&,const char* empty_literal);
// 7FDB20: ECX actual F8 profile, RET. Full normal parent reset, including
// retained captures across returning invalid-parameter handlers and map calls.
void reset_native_player_profile_007fdb20(void* actual_profile,
    NativePlayerProfileContext&,NativePlayerProfileOperation&);
// 7FEE20: ECX actual F8 profile, EAX same profile, RET. Full normal parent,
// constructs member storage then executes the raw reset above. Native FH3
// destruction of partial members is open; source failures retain both stages
// and graph/temporary storage and prohibit replay. No binary ABI claim.
void* construct_native_player_profile_007fee20(void* actual_profile,
    NativePlayerProfileContext&,NativePlayerProfileOperation&);
} // namespace bsp
