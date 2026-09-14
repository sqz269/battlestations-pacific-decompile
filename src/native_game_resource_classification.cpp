#include "bsp/native_game_resource_classification.hpp"
#include "bsp/native_game_resource_lists.hpp"

namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
void* at(const void* p,U offset=0) noexcept {return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+offset);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
std::uint8_t matches(U target,void* item,U token,NativeGameResourceClassificationContext& c) {
    if(target==0x00b86950) return matches_native_fallback_type_00b86950(token,c.fallback_tokens_0109021c);
    return c.other_types.matches_type(target,item,token);
}
}
U read_native_resource_type54_006f9a80(const volatile U& value) {return value;}
U read_native_resource_type44_00721c90(const volatile U& value) {return value;}
std::uint8_t matches_native_fallback_type_00b86950(U token,const volatile U* current) {
    for(unsigned i=0;i!=3;++i) if(current[i]==token) return 1;
    return 0;
}
void append_and_classify_native_game_item_0071bb40(void* resource,void* item,NativeGameResourceClassificationContext& context) {
    append_native_resource_item_00b87aa0(resource,item);
    if(!item) return;
    U token=context.type64_00e19b64;
    U table=word(item);
    U target=word(reinterpret_cast<void*>(table),0x0c);
    if(matches(target,item,token,context)!=0) {
        append_native_game_type64_pointer_0071b9b0(at(resource,0x64),&item,context.invalid_parameters);
        return;
    }
    table=word(item);
    token=read_native_resource_type54_006f9a80(context.type54_00e19a98);
    target=word(reinterpret_cast<void*>(table),0x0c);
    if(matches(target,item,token,context)!=0) {
        append_native_game_type54_pointer_0071b940(at(resource,0x54),&item,context.invalid_parameters);
        return;
    }
    table=word(item);
    token=read_native_resource_type44_00721c90(context.type44_00e19be4);
    target=word(reinterpret_cast<void*>(table),0x0c);
    if(matches(target,item,token,context)!=0)
        append_native_game_type44_pointer_0071b8d0(at(resource,0x44),&item,context.invalid_parameters);
}
NativeGameResourceDispatchCalls::NativeGameResourceDispatchCalls(NativeResourceDispatchCalls& other,NativeGameResourceClassificationContext& context)
    :other_(other),context_(context) {}
void NativeGameResourceDispatchCalls::renderer_hook(std::uintptr_t target,void* renderer) {other_.renderer_hook(target,renderer);}
void* NativeGameResourceDispatchCalls::parse_item(std::uintptr_t target,void* parser,void* handle) {return other_.parse_item(target,parser,handle);}
void NativeGameResourceDispatchCalls::append_item(std::uintptr_t target,void* resource,void* item) {
    if(target==0x0071bb40) append_and_classify_native_game_item_0071bb40(resource,item,context_);
    else other_.append_item(target,resource,item);
}
} // namespace bsp
