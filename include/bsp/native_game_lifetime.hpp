#pragma once
#include "bsp/native_game_array_lifetime.hpp"
#include "bsp/native_game_embedded_lifetime.hpp"
#include "bsp/native_game_profile_lifetime.hpp"
#include "bsp/native_game_container_lifetime.hpp"
#include "bsp/native_award_registry_lifetime.hpp"
#include "bsp/native_game_tree_lifetime.hpp"
#include <cstdint>

namespace bsp {
struct NativeGameStorage;
// Address-named dependencies deliberately require real bindings. No successful
// empty cleanup is supplied. The parent does not establish these callee bodies.
struct NativeGameLifetimeCalls : NativeGameArrayLifetimeCalls,NativeGameEmbeddedLifetimeCalls,NativeGameProfileLifetimeCalls,NativeGameContainerLifetimeCalls {
    virtual ~NativeGameLifetimeCalls()=default;
    virtual void call_00c4dde0(void*)=0;
    virtual void call_0076a760(void*,NativeGameEmbeddedLifetimeContext*);
    virtual void call_008d88f0()=0;
    virtual void call_004bf930(void*,NativeGameContainerLifetimeProgress&);
    virtual void call_004a9ac0()=0;
    virtual void call_006b9380(void*,NativeGameProfileLifetimeContext*,NativeAwardRegistryLifetimeOperation&);
    virtual void call_004c0c30()=0;
    virtual void call_004c0ce0()=0;
    virtual void call_00b6cf90()=0;
    virtual void call_004c0d90()=0;
    virtual void* call_004c1400()=0;
    virtual void call_00b806f0(void*)=0;
    virtual void call_004c7dd0(void*,NativeGameContainerLifetimeProgress&);
    virtual void call_0041cc80(void*);
    virtual void call_004cb220(void*,std::uint32_t,NativeGameContainerLifetimeProgress&);
    virtual void call_004c4b40(void*,NativeGameContainerLifetimeProgress&);
    virtual void* call_00419cc0(void* block,std::uint32_t size,std::uint32_t factor,NativeGameProfileLifetimeContext*);
    virtual void call_00bd1510(void* pool,void* block,std::uint32_t size,std::uint32_t factor,NativeGameProfileLifetimeContext*);
    virtual void call_0076f000(void*,NativeGameEmbeddedLifetimeContext*,NativeGameEmbeddedLifetimeOperation&);
    virtual void call_00b669a0(void*);
    virtual void call_004bf8e0(void*,NativeGameContainerLifetimeProgress&);
    virtual void call_007ff9f0(void*,NativeGameProfileLifetimeContext*);
    virtual void call_007fd8a0(void*,NativeGameProfileLifetimeContext*,NativeGameProfileLifetimeOperation&);
    virtual void call_004cf3f0(void*,NativeGameContainerLifetimeProgress&);
    virtual void call_004c2ce0(void*,NativeGameContainerLifetimeProgress&);
    virtual void call_004c4a50(void*,NativeGameContainerLifetimeProgress&);
    virtual void call_004dceb0(void*);
    // Checked-STL boundaries: ECX tree, five stack arguments, RET14h.
    // output is a private8h iterator. No private-stack alias contract is claimed.
    virtual void call_004d1a50(void* tree,void* output,void* first_owner,
        void* first_position,void* last_owner,void* last_position,NativeGameProfileLifetimeContext*,NativeGameTreeLifetimeProgress&);
    virtual void call_004d22f0(void*,void*,void*,void*,void*,void*,NativeGameProfileLifetimeContext*,NativeGameTreeLifetimeProgress&);
    virtual void call_004d2000(void*,void*,void*,void*,void*,void*,NativeGameProfileLifetimeContext*,NativeGameTreeLifetimeProgress&);
    virtual void call_004d41a0(void*,void*,void*,void*,void*,void*,NativeGameProfileLifetimeContext*,NativeGameTreeLifetimeProgress&);
    virtual void call_004cef40(void*,void*,void*,void*,void*,void*,NativeGameProfileLifetimeContext*,NativeGameTreeLifetimeProgress&);
    // CRT reverse array iteration remains a library boundary. Destructor is
    // an original address identifier, not a directly callable host pointer.
    virtual void array_destroy_00bf7c6e(void* base,std::uint32_t stride,
        std::uint32_t count,std::uint32_t destructor,NativeGameArrayLifetimeContext*,
        NativeGameArrayLifetimeOperation&);
    virtual void virtual_scalar(void* captured,std::uint32_t vtable_offset,
        std::uint32_t flags)=0;
    virtual void virtual_terminal(void* captured)=0;
    // Concrete Win32 atomic/CRT defaults; fixtures may observe each boundary.
    virtual std::int32_t interlocked_decrement(volatile std::int32_t*);
    void* allocate_00bf55be(std::uint32_t) override;
    virtual void free_00bf65ac(void*);
    virtual void free_00bf6989(void*);
};
struct NativeGameLifetimeContext {
    volatile std::uint32_t& small_returns_disabled_01090aa4;
    void* volatile& publication_00e18678;
    void* volatile& publication_00e1867c;
    void* volatile& publication_00e19900;
    void* volatile& publication_00e18db0;
    void* volatile& publication_00e19698;
    void* volatile& publication_00e1930c;
    void* volatile& publication_00e198bc;
    void* volatile& movie_00e18d48;
    void* volatile& game_00e188a8;
    void* volatile& grid_00e19b0c;
    void* volatile& grid_00e19b08;
    void* volatile& grid_00e19b04;
    NativeGameLifetimeCalls& calls;
    // Required by the concrete array default. Same call service and actual
    // string/observer domain as the constructed game; null fails when reached.
    NativeGameArrayLifetimeContext* arrays;
    NativeGameEmbeddedLifetimeContext* embedded;
    NativeGameProfileLifetimeContext* profile;
};
struct NativeGameLifetimeProgress {
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
};
struct NativeGameLifetimeOperation final : NativeGameLifetimeProgress {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    NativeGameStorage* owner{};
    NativeGameLifetimeContext* context{};
    NativeGameArrayLifetimeOperation arrays[4];
    NativeGameEmbeddedLifetimeOperation embedded;
    NativeGameProfileLifetimeOperation profile;
    NativeGameContainerLifetimeProgress containers;
    NativeAwardRegistryLifetimeOperation awards;
    NativeGameTreeLifetimeProgress trees;
    NativeGameLifetimeOperation()=default;
    ~NativeGameLifetimeOperation();
    NativeGameLifetimeOperation(const NativeGameLifetimeOperation&)=delete;
    NativeGameLifetimeOperation& operator=(const NativeGameLifetimeOperation&)=delete;
    // Caller must resolve the partially destroyed graph first. No rollback,
    // retry, owner release, or native FH3 cleanup is performed by this method.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Full78B4D27C0. Capture *cell; free its current+14/+4 then the captured owner;
// clear corresponding triples and finally the CURRENT publication cell.
void destroy_native_game_nested_storage_004d27c0(void* cell,
    NativeGameLifetimeCalls&,NativeGameLifetimeProgress&);
// Full62B4CC760: ECX first,EDX last; two unused stack arguments,RET8.
// Capture each pointer, decrement+4, currentvslot0 onzero, then clear its cell.
// Captured end, ascending4-byte steps; no validation or free of the array.
void release_native_game_pointer_range_004cc760(void* first,void* last,
    NativeGameLifetimeCalls&,NativeGameLifetimeProgress&);
// Full normal1559B4DCF90 and30B4DE270 parent schedules. Required callees above
// are contracts, not implementations. Scalar flags use only lowbyte bit0;
// return the original owner even after free. Explicit C++ interfaces do not
// implement original calling convention, native FH3 unwind or app admission.
void destroy_native_game_004dcf90(NativeGameStorage&,NativeGameLifetimeContext&,
    NativeGameLifetimeOperation&);
NativeGameStorage* delete_native_game_004de270(NativeGameStorage&,std::uint32_t flags,
    NativeGameLifetimeContext&,NativeGameLifetimeOperation&);
} // namespace bsp
