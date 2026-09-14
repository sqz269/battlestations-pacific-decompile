#pragma once
#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_resource_stream_reads.hpp"
#include <cstdint>
namespace bsp {
// Actual24h node: profile0/ref4/borrowed reader8/parentC, owning tag10/14,
// depth18, declared1C and remaining20. No projected node or shared_ptr.
// BEA380: ECX node, stacked reader, EAX node, RET4. Read tag/payload with local
// budget1000, increment CURRENT reader path index before copying tag into it.
// No magic, size, depth, truncation or attachment validation is added.
void* construct_native_resource_root_00bea380(void* node,void* reader,
    NativeResourceStreamReadContext&);
// BEA700: ECX reader, stacked output4h handle, EAX output, RET4. Allocate24h,
// construct, publish. Construction failure frees raw allocation after native
// constructor actions; output is untouched until success/null allocation.
void* create_native_resource_root_00bea700(void* reader,void* output,
    NativeResourceStreamReadContext&);
// BE9DF0: ECX node, RET. If attached, debit parent by declared payload, pop
// current reader path and clear reader8. Return tag, stamp base; NEVER seek.
void destroy_native_resource_node_00be9df0(void* node,NativeStringRawPoolContext&);
// BE9FC0: ECX node, stacked flags, EAX captured node, RET4. Free only bit0
// after destruction succeeds; neither decrement nor clear stale name fields.
void* delete_native_resource_node_00be9fc0(void* node,std::uint32_t flags,
    NativeStringRawPoolContext&);

// Borrow an existing concrete stream dispatch and actual pool. Add only the
// D68BB4 node's BD30E0 -> current slot4/BE9FC0 terminal path; all stream calls
// forward unchanged. Native numeric identities are never process-callable.
class NativeResourceRootDispatch final:public NativeAdoptedSubstreamDispatch {
public:
    NativeResourceRootDispatch(NativeAdoptedSubstreamDispatch&,NativeStringRawPoolContext&);
    std::uint8_t source_is_open(std::uintptr_t,void*) override;
    std::uint32_t source_seek(std::uintptr_t,void*,std::uint32_t,std::uint32_t,std::uint32_t) override;
    void source_read(std::uintptr_t,void*,void*,std::uint32_t,std::uint32_t*) override;
    void source_write(std::uintptr_t,void*,const void*,std::uint32_t,std::uint32_t*) override;
    void source_zero_reference(std::uintptr_t,void*,std::uintptr_t) override;
private:
    NativeAdoptedSubstreamDispatch& streams_;
    NativeStringRawPoolContext& strings_;
};
// New C++ ABI. Original native stack/EH-spill aliases, CRT/FH3/SEH identity,
// hardware faults, arbitrary alternate profiles and gameplay remain unproved.
// Ordinary C++ unwind order follows recovered maps; a failing unwind terminates.
}
