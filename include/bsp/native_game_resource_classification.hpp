#pragma once
#include "bsp/native_resource_root_dispatch.hpp"
#include <cstdint>
namespace bsp {
class NativeResourceItemTypeCalls {
public:
    virtual ~NativeResourceItemTypeCalls()=default;
    virtual std::uint8_t matches_type(std::uintptr_t captured_target,void* item,std::uint32_t token)=0;
};
struct NativeGameResourceClassificationContext {
    const volatile std::uint32_t& type64_00e19b64;
    const volatile std::uint32_t& type54_00e19a98;
    const volatile std::uint32_t& type44_00e19be4;
    const volatile std::uint32_t* fallback_tokens_0109021c; // exactly3 current cells
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeResourceItemTypeCalls& other_types;
};
// Native getters: MOV EAX,current global; RET. No owner or stack input.
std::uint32_t read_native_resource_type54_006f9a80(const volatile std::uint32_t&);
std::uint32_t read_native_resource_type44_00721c90(const volatile std::uint32_t&);
// B86950: stacked token; AL boolean, upper EAX unspecified; RET4. ECX object
// ignored. Compare current three token cells in order, stopping at first match.
std::uint8_t matches_native_fallback_type_00b86950(std::uint32_t token,const volatile std::uint32_t* current_tokens);
// 71BB40: ECX actual74h game resource, stack raw item, RET4. Base append first,
// including null; then first match64,54,44 appends only the borrowed pointer.
// Table is reacquired between predicates. For54/44 capture table BEFORE getter,
// then read its slotC afterward. No rollback when predicate/list growth throws.
void append_and_classify_native_game_item_0071bb40(void* resource,void* item,NativeGameResourceClassificationContext&);

// Root-dispatch composition for the actual CFD8CC slotC target71BB40. Unknown
// targets forward to the supplied bindings; no default-resource alias is used.
class NativeGameResourceDispatchCalls final:public NativeResourceDispatchCalls {
public:
    NativeGameResourceDispatchCalls(NativeResourceDispatchCalls&,NativeGameResourceClassificationContext&);
    void renderer_hook(std::uintptr_t,void*) override;
    void* parse_item(std::uintptr_t,void*,void*) override;
    void append_item(std::uintptr_t,void*,void*) override;
private:
    NativeResourceDispatchCalls& other_;
    NativeGameResourceClassificationContext& context_;
};
// New source ABI and borrowed actual token cells; their startup publication is
// not invented here. No native CRT/FH3/SEH or game validation claim.
} // namespace bsp
