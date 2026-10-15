#pragma once
#include "bsp/avoid_zone_dyn_hull.hpp"
#include <cstdint>
namespace bsp {
// Original checked vector: proxy, data, end, capacity_end. Elements are three
// float words. The actual game's callback is game+1Ch; its vector is game+20h.
struct NativeGameContactReportCalls {
    virtual ~NativeGameContactReportCalls()=default;
    virtual void* allocate_00bf681b(std::uint32_t site,std::uint32_t bytes,const AvoidZoneDynHullMemory&);
    virtual void free_00bf65ac(std::uint32_t site,void*,const AvoidZoneDynHullMemory&);
    virtual void invalid_parameter_00bf6713(std::uint32_t site);
    [[noreturn]] virtual void length_error_0041f870();
    [[noreturn]] virtual void bad_alloc_00415720();
};
struct NativeGameContactReportContext {
    const AvoidZoneDynHullMemory& memory;
    NativeGameContactReportCalls& calls;
};
// Full normal raw-storage bodies, including general insert/erase. Borrow a
// matching CRT allocator for every vector lifetime. Original FH3, allocation
// failure unwinding, invalid iterators and binary entry ABI remain unproven.
// The callback preserves the native uninitialized resize-fill stack preimage;
// only completed report points are defined by that operation.
void receive_native_game_contact_reports_004d4ce0(void* receiver,const void* records,
    std::int32_t count,const NativeGameContactReportContext&); // ECX,stack records,count / RET8
void resize_native_checked_vector12_004d1510(void* vector,std::uint32_t count,
    const void* fill12,const NativeGameContactReportContext&); // ECX,stack count,value12 / RET10h
void insert_native_checked_vector12_0041fa40(void* vector,void* iterator_owner,
    void* position,std::uint32_t count,const void* value12,const NativeGameContactReportContext&);
void* erase_native_checked_vector12_004c82b0(void* vector,void* output_pair,
    void* first_owner,void* first,void* last_owner,void* last,const NativeGameContactReportContext&);
std::int32_t native_checked_vector12_size_00415290(const void* vector);

// Typed public tail entry to the existing instruction-audited 00419330 kernel.
// Six cdecl stack words; EAX is the computed destination end. The last three
// words are ignored by that actual kernel. Retains x87 per-word forward copy,
// overlap order and native signed truncating divide12 address computation.
// Normal admitted ranges require all native source/destination accesses valid.
// No duplicate loop, allocator, context or callback is introduced.
void* __cdecl move_native_vector12_00419330(const void* first, const void* last,
    void* destination, const void* metadata4, const void* metadata5,
    std::uint32_t ignored_tag) noexcept;

// Complete one-slot CE78C0 source table. Stable owner borrows the allocator and
// calls object. bind() publishes only the table; the actual receiver's vector
// must already be initialized and must outlive every call through this table.
class NativeGameContactReportRuntime final {
public:
    NativeGameContactReportRuntime(const AvoidZoneDynHullMemory&,NativeGameContactReportCalls&) noexcept;
    NativeGameContactReportRuntime(const NativeGameContactReportRuntime&)=delete;
    NativeGameContactReportRuntime& operator=(const NativeGameContactReportRuntime&)=delete;
    const void* table() const noexcept {return methods_;}
    void bind(void* receiver) const noexcept;
private:
    const std::uintptr_t methods_[1];
    const AvoidZoneDynHullMemory memory_;
    NativeGameContactReportCalls* const calls_;
    static void __fastcall receive(void*,void*,const void*,std::int32_t);
};
} // namespace bsp
