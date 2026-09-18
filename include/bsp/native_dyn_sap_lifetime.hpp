#pragma once
#include "bsp/avoid_zone_dyn_hull.hpp"
#include <cstdint>

namespace bsp {
struct NativeDynSapLifetimeCalls {
    virtual ~NativeDynSapLifetimeCalls()=default;
    virtual void* sap_allocate_00bf55be(std::uint32_t,const AvoidZoneDynHullMemory&);
    virtual void sap_free_00bf65ac(void*,const AvoidZoneDynHullMemory&);
    virtual void sap_free_00bf6989(void*,const AvoidZoneDynHullMemory&);
    virtual void sap_free_00bf9dc8(void*,const AvoidZoneDynHullMemory&);
};
struct NativeDynSapLifetimeProgress {
    std::uint32_t native_site{};
    void* manager{};
    void* proxy{};
    void* pair{};
    void* cursor{};
    std::uint32_t completed_endpoint_pools{};
};
// Complete147B C40D80: native ESI proxy,stack index,RET4. Swap with last,
// decrement count and retain the original unsigned-underflow growth branch.
// Normal removal requires index<count and nonzero count.
void remove_native_dyn_sap_pair_reference_00c40d80(void*,std::uint32_t,const AvoidZoneDynHullMemory&,NativeDynSapLifetimeCalls&,NativeDynSapLifetimeProgress&);
// Complete128B C4BC50: native EBX manager,stack proxy,RET4. Remove each pair
// from its peer vector, unlink/recycle the pair, retain the source proxy vector.
void clear_native_dyn_sap_proxy_pairs_00c4bc50(void*,void*,const AvoidZoneDynHullMemory&,NativeDynSapLifetimeCalls&,NativeDynSapLifetimeProgress&);
// Complete452B C4C380: ECX actual248h SAPBroadPhaseManager2,stack actual50h
// proxy,RET4. Inserted proxies return six endpoints and clear pairs; queued
// proxies shift their pending-vector suffix. Captured static/dynamic choice
// survives the pair-vector free. Proxy fields remain stale after recycling.
void remove_native_dyn_sap_proxy_00c4c380(void*,void*,const AvoidZoneDynHullMemory&,NativeDynSapLifetimeCalls&,NativeDynSapLifetimeProgress&);
// Complete56B endpoint-pool destructor,ECX pool/RET. Free each page using the
// current vector/count,then its pointer vector. Storage remains unchanged.
void destroy_native_dyn_sap_endpoint_pool_0040b590(void*,const AvoidZoneDynHullMemory&,NativeDynSapLifetimeCalls&,NativeDynSapLifetimeProgress&);
// Complete283B manager destructor,stack manager/RET4. Free pending vector,
// static/dynamic proxy and pair pools,then three endpoint pools in reverse
// order (the consumed normal BF7C6E contract),finally stamp D7A0E4 base profile.
void destroy_native_dyn_sap_manager_004043f0(void*,const AvoidZoneDynHullMemory&,NativeDynSapLifetimeCalls&,NativeDynSapLifetimeProgress&);
// Complete31B scalar,ECX owner/stackflags/EAX capturedowner/RET4. Test low bit0
// after destructor; optionally free the captured248h allocation. Uses the SAME
// allocator as construction. No synthesized partial runtime vtable is supplied.
void* delete_native_dyn_sap_manager_004043d0(void*,std::uint32_t,const AvoidZoneDynHullMemory&,NativeDynSapLifetimeCalls&,NativeDynSapLifetimeProgress&);
// All interfaces describe normal source behavior. Native FH3/SEH, private
// stack aliases, concurrency, malformed queues and repeated cleanup are outside
// the contract. Caller retains partial-ownership diagnostics on source failure.
} // namespace bsp
