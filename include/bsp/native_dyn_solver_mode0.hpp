#pragma once
#include "bsp/avoid_zone_dyn_hull.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {
// Original CALL sites distinguish row and velocity allocation/free boundaries.
// The allocator and callbacks must support concurrent tasks if the world does.
struct NativeDynSolverMode0Calls {
    virtual ~NativeDynSolverMode0Calls()=default;
    virtual void* allocate_00bf55be(std::uint32_t site,std::uint32_t bytes,const AvoidZoneDynHullMemory&);
    virtual void free_00bf65ac(std::uint32_t site,void*,const AvoidZoneDynHullMemory&);
    virtual void free_00bf6989(std::uint32_t site,void*,const AvoidZoneDynHullMemory&);
};
// Complete normal task chain: 00403720, C31C30, C35020, C37B50, C42230,
// C42530, C42BA0, C4DE40 and C4F040. Task storage is 18h bytes: table at0,
// scheduler word4, world8, inclusive first/last group at Ch/10h, float dt14h.
// Uses actual world/group/manifold/body/motion records and native parallel row
// arrays, including signed16 body indices and a private EAAC-byte context.
// Valid records, nonzero dt, successful allocation and normal return required.
// Original FH3/SEH registration, unwind cleanup and allocation failure behavior
// are not supplied by this interface. Math retains the native x87/SSE schedule.
void execute_native_dyn_solver_mode0_00403720(void* task,
    const AvoidZoneDynHullMemory&,NativeDynSolverMode0Calls&,const CameraAxesCrtAccess&);

// Complete one-slot source table corresponding to D7A088 -> 00403720.
// Keep this owner and borrowed callback/CRT pointees alive at stable addresses.
class NativeDynSolverMode0Runtime final {
public:
    NativeDynSolverMode0Runtime(const AvoidZoneDynHullMemory&,NativeDynSolverMode0Calls&,
        const CameraAxesCrtAccess&) noexcept;
    NativeDynSolverMode0Runtime(const NativeDynSolverMode0Runtime&)=delete;
    NativeDynSolverMode0Runtime& operator=(const NativeDynSolverMode0Runtime&)=delete;
    const void* table() const noexcept {return methods_;}
private:
    std::uintptr_t methods_[1];
    AvoidZoneDynHullMemory memory_;
    NativeDynSolverMode0Calls* calls_;
    CameraAxesCrtAccess crt_;
    static void __fastcall run(void*,void*);
};
} // namespace bsp
