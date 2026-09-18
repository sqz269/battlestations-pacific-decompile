#pragma once
#include "bsp/native_dyn_sap_lifetime.hpp"

namespace bsp {
struct NativeDynSapPairCalls : NativeDynSapLifetimeCalls {
    virtual void* sap_malloc_00bf9f1a(std::uint32_t,const AvoidZoneDynHullMemory&);
};
// Seven complete normal bodies. Actual248h SAP manager,50h proxies and10h
// pair records from their existing constructors. Original allocator required.
// EAX first proxy,stack second,RET4; compare signed captured vector counts,
// search the shorter list (EAX list on a tie),return matching pair ornull.
void* find_native_dyn_sap_pair_00c37300(void* eax_proxy,void* stack_proxy) noexcept;
// ESI proxy,stack pair,RET4. Grow capacity*2+2 on equality,copy current count,
// free old vector,publish new storage,append pair and increment count.
void append_native_dyn_sap_pair_00c37290(void* proxy,void* pair,
    const AvoidZoneDynHullMemory&,NativeDynSapPairCalls&,NativeDynSapLifetimeProgress&);
// EDI manager,two stack proxies,RET8. Suppress existing pair,extend1000-slot
// page pool if needed,append active pair sorted by raw unsigned pointer value,
// then append to FIRST input proxy followed by SECOND,regardless of sort order.
void create_native_dyn_sap_pair_00c3ffe0(void* manager,void* first,void* second,
    const AvoidZoneDynHullMemory&,NativeDynSapPairCalls&,NativeDynSapLifetimeProgress&);
// EAX first,EBX second,stack manager,RET4. Search/remove from both vectors,
// unlink/recycle pair. Missing pair is a no-op. Existing C40D80 is reused.
void erase_native_dyn_sap_pair_00c4bcd0(void* manager,void* first,void* second,
    const AvoidZoneDynHullMemory&,NativeDynSapPairCalls&,NativeDynSapLifetimeProgress&);
// Manager ECX. Count/head RET; next additionally receives stackpair,RET4.
std::uint32_t count_native_dyn_sap_pairs_00c32b30(const void* manager) noexcept;
void* first_native_dyn_sap_pair_00c32b10(const void* manager) noexcept;
void* next_native_dyn_sap_pair_00c32af0(const void* manager,const void* pair) noexcept;
// Explicit C++ interfaces,not native register/FH3/SEH ABI. Valid storage and
// successful allocations required; no added concurrency or failure recovery.
// Incremental endpoint updates and the >50 pending batch remain separate.
} // namespace bsp
