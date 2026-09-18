#pragma once
#include "bsp/native_dyn_sap_pairs.hpp"
#include "bsp/dyn_body_creation.hpp"

namespace bsp {
// Complete normal endpoint allocation, insertion, movement and processing.
// Actual248h manager/50h proxy storage, matching construction allocator,
// valid axis0..2 and successful allocations required. Float comparisons keep
// the original x87 instruction order; these are explicit source interfaces.
void allocate_native_dyn_sap_endpoints_00c36c30(void* manager,void* proxy,std::uint32_t axis,
    const AvoidZoneDynHullMemory&,NativeDynSapPairCalls&,NativeDynSapLifetimeProgress&);
void insert_native_dyn_sap_endpoints_00c36e20(void* manager,void* proxy,std::uint32_t axis) noexcept;
void decide_native_dyn_sap_pair_00c4bd60(void* manager,void* first,void* peer,std::uint32_t axis,std::uint32_t add,
    const AvoidZoneDynHullMemory&,NativeDynSapPairCalls&,NativeDynSapLifetimeProgress&);
void move_native_dyn_sap_endpoints_00c4be10(void* manager,void* proxy,std::uint32_t axis,
    const AvoidZoneDynHullMemory&,NativeDynSapPairCalls&,NativeDynSapLifetimeProgress&);
void insert_native_dyn_sap_pending_00c4c2a0(void* manager,
    const AvoidZoneDynHullMemory&,NativeDynSapPairCalls&,NativeDynSapLifetimeProgress&);
void update_native_dyn_sap_proxy_00c4c270(void* manager,void* proxy,
    const AvoidZoneDynHullMemory&,NativeDynSapPairCalls&,NativeDynSapLifetimeProgress&);
void batch_native_dyn_sap_pending_00c40140(void* manager,
    const AvoidZoneDynHullMemory&,NativeDynSapPairCalls&,NativeDynSapLifetimeProgress&);
void process_native_dyn_sap_00c4c320(void* manager,
    const AvoidZoneDynHullMemory&,NativeDynSapPairCalls&,NativeDynSapLifetimeProgress&);

// Complete eight-slot Win32 callable source table. The stable owner, borrowed
// allocator context, calls and progress must outlive every manager using it.
// No original RTTI/FH3/SEH metadata or concurrency/failure recovery is supplied.
class NativeDynSapRuntime final {
public:
    NativeDynSapRuntime(const AvoidZoneDynHullMemory&,NativeDynSapPairCalls&,NativeDynSapLifetimeProgress&) noexcept;
    NativeDynSapRuntime(const NativeDynSapRuntime&)=delete;
    NativeDynSapRuntime& operator=(const NativeDynSapRuntime&)=delete;
    const void* table() const noexcept {return methods_;}
private:
    std::uintptr_t methods_[8];
    AvoidZoneDynHullMemory memory_;
    NativeDynSapPairCalls* calls_;
    NativeDynSapLifetimeProgress* progress_;
    static NativeDynSapRuntime& owner(void*) noexcept;
    static void* __fastcall create(void*,void*,DynBodyStorage*,const DynAabb*,std::uint32_t);
    static void __fastcall remove(void*,void*,void*);
    static void __fastcall update(void*,void*,void*);
    static void __fastcall process(void*,void*);
    static std::uint32_t __fastcall count(void*,void*);
    static void* __fastcall first(void*,void*);
    static void* __fastcall next(void*,void*,void*);
    static void* __fastcall scalar(void*,void*,std::uint32_t);
};
} // namespace bsp
