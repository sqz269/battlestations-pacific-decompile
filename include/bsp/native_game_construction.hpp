#pragma once
#include "bsp/dyn_world_settings.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_player_profile_owner.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
// 73E150 allocates71A0, 73E163 clears the entire allocation, then 73E1A1
// invokes4DDB90. The constructor itself preserves every unlisted byte. This
// type supplies the observed allocation extent, not an invented game layout.
struct alignas(4) NativeGameStorage final { std::byte bytes[0x71a0]; };
static_assert(sizeof(NativeGameStorage)==0x71a0);
static_assert(std::is_trivially_default_constructible_v<NativeGameStorage>);

// Required external bodies, identified by native address. No default, no-op,
// projected owner or success fallback is supplied. Container allocation and
// CRT array iteration remain library contracts, not new STL/CRT ports.
// A concrete application must bind ALL reached calls to their real services.
struct NativeGameConstructionCalls {
    virtual ~NativeGameConstructionCalls()=default;
    virtual void array_construct_00bf7cd1(void* base,std::uint32_t stride,
        std::uint32_t count,std::uint32_t constructor,std::uint32_t destructor)=0;
    virtual void* call_004c2700()=0; // allocate14h tree node, flags10/11
    virtual void* call_004c2750()=0; // allocate28h tree node, flags24/25
    virtual void* call_004c27a0()=0; // allocate18h tree node, flags14/15
    virtual void* call_004c2830()=0; // allocate18h tree node, flags14/15
    virtual void* call_004c1950()=0; // allocate24h list head, links0/4=self
    virtual void* call_004c26b0()=0; // existing raw18h tree leaf
    virtual void call_007ff9d0(void* race_record)=0;
    virtual void call_0076ede0(void* embedded)=0;
    virtual void* call_004c1a40()=0; // allocate0Ch list head, links0/4=self
    virtual void call_008d9150()=0;
    virtual void* call_00432650()=0;
    virtual void call_0087d7b0(void* captured_configuration)=0;
    virtual void call_00717e80()=0;
    virtual void* allocate_00bf681b(std::uint32_t bytes)=0;
    virtual void* call_0070bd70(void* allocation,float argument)=0;
    virtual void call_00727bd0()=0;
    virtual void* call_008882d0(void* allocation)=0;
    virtual std::uint32_t call_00be4800()=0;
    virtual void* call_00c55f50(const std::uint32_t* worker_count)=0;
    virtual void* call_00c420e0(void* captured_engine,const DynWorldDescriptor&)=0;
    virtual void call_00c31a40(void* captured_world,void* observer)=0;
    virtual void* call_00bd1860()=0;
};

struct NativeGameConstructionConstants {
    const volatile float& argument_00ce7d20;
    const volatile float& argument_00ce7d1c;
    // SSE MOVSS copies retain these exact32-bit representations and read order.
    const volatile std::uint32_t& bits_00ce7480;
    const volatile std::uint32_t& bits_00d7a24c;
    const volatile std::uint32_t& bits_00ce746c;
    const volatile std::uint32_t& bits_00ce7638;
    const volatile std::uint32_t& bits_00d7a2f0;
    const volatile std::uint32_t& bits_00ce3800;
    const volatile std::uint32_t& bits_00ce6848;
};
struct NativeGameConstructionContext {
    void* volatile& actual_game_00e188a8;
    void* volatile& actual_00e19b0c;
    void* volatile& actual_00e19b08;
    void* volatile& actual_00e19b04;
    void* volatile& actual_movie_00e18d48;
    NativeStringStorage& strings;
    NativeGameConstructionConstants constants;
    NativePlayerProfileContext& profile;
    NativeGameConstructionCalls& calls;
};
struct NativeGameConstructionOperation final {
    enum class Phase { fresh,running,complete,failed,diagnostic_retired };
    Phase phase{Phase::fresh};
    NativeGameStorage* owner{};
    NativeGameConstructionContext* context{};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    void* current_allocation{};
    std::uint32_t worker_count;
    DynWorldDescriptor descriptor;
    NativePlayerProfileOperation profile;
    NativeGameConstructionOperation() noexcept=default;
    ~NativeGameConstructionOperation();
    NativeGameConstructionOperation(const NativeGameConstructionOperation&)=delete;
    NativeGameConstructionOperation& operator=(const NativeGameConstructionOperation&)=delete;
    // Only after the retained graph has been resolved by a diagnostic caller.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Full4DDB90 normal parent schedule. Native ECX=71A0 game, stack=actual8h
// string header pointer, EAX=same game, RET4. Real input/Lua/string primitives
// and the raw profile constructor/reset are reused; remaining callees above
// and the profile's remaining services are required dependencies. The input
// header may alias game+7164, which is initialized BEFORE the self-copy test.
// Current game publication happens late at4DE105, BEFORE Dyn initialization.
// Source exceptions retain the frame/partial graph and prohibit replay; this
// does not reproduce native FH3 cleanup, parent allocation/free, destruction,
// private-stack aliases, binary ABI or whole-application/gameplay behavior.
NativeGameStorage* construct_native_game_004ddb90(NativeGameStorage&,
    const void* actual_name_header,NativeGameConstructionContext&,NativeGameConstructionOperation&);
} // namespace bsp
