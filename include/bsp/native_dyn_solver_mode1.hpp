#pragma once
#include "bsp/native_dyn_solver_mode0.hpp"
#include "bsp/dyn_profile_scopes.hpp"

namespace bsp {
struct NativeDynSolverMode1Calls : NativeDynSolverMode0Calls {
    // Default executes actual, unserialized RDTSC. An override must retain FP
    // state and return the timestamp for the supplied original instruction site.
    virtual std::uint64_t read_timestamp(std::uint32_t site) noexcept;
};
// Complete normal mode1 task 00403850 and its nine solver dependencies.
// Actual18h task/world/group/body/motion/manifold records; EAC8-byte private
// context, per-manifold four-contact rows and original x87/SSE instruction order.
// Profiling uses the actual published owner, cache and child nodes. The supplied
// profile allocator must be the world's allocator. Valid records, successful
// allocation and normal return required. Native FH3/SEH and unwinding are not
// supplied; this explicit source interface is not a drop-in exception ABI.
void execute_native_dyn_solver_mode1_00403850(void* task,
    const DynProfileScopeContext&,NativeDynSolverMode1Calls&,const CameraAxesCrtAccess&);

// Complete one-slot source table corresponding to D7A090 -> 00403850. Keep
// this owner and borrowed profile/callback/CRT pointees alive and stable.
class NativeDynSolverMode1Runtime final {
public:
    NativeDynSolverMode1Runtime(const DynProfileScopeContext&,NativeDynSolverMode1Calls&,
        const CameraAxesCrtAccess&) noexcept;
    NativeDynSolverMode1Runtime(const NativeDynSolverMode1Runtime&)=delete;
    NativeDynSolverMode1Runtime& operator=(const NativeDynSolverMode1Runtime&)=delete;
    const void* table() const noexcept {return methods_;}
private:
    std::uintptr_t methods_[1];
    DynProfileScopeContext profile_;
    NativeDynSolverMode1Calls* calls_;
    CameraAxesCrtAccess crt_;
    static void __fastcall run(void*,void*);
};
} // namespace bsp
