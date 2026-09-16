#include "bsp/native_skinned_animation_dispatch.hpp"
#include "bsp/native_skinned_animation_item.hpp"
#include <stdexcept>

namespace bsp {
NativeSkinnedAnimationResourceCalls::NativeSkinnedAnimationResourceCalls(
    NativeResourceDispatchCalls& other,NativeResourceStreamReadContext& reads) noexcept
    :other_(other),reads_(reads) {}
void NativeSkinnedAnimationResourceCalls::renderer_hook(std::uintptr_t target,void* renderer) {
    other_.renderer_hook(target,renderer);
}
void* NativeSkinnedAnimationResourceCalls::parse_item(std::uintptr_t target,void* parser,void* handle) {
    if(target==0x00b92f20u)return parse_native_skinned_animation_item_00b92f20(handle,reads_);
    return other_.parse_item(target,parser,handle);
}
void NativeSkinnedAnimationResourceCalls::append_item(std::uintptr_t target,void* resource,void* item) {
    other_.append_item(target,resource,item);
}

NativeSkinnedAnimationReferences::NativeSkinnedAnimationReferences(
    NativeAdoptedSubstreamDispatch& other,NativeStringRawPoolContext& strings,
    const volatile std::uint32_t* profile) noexcept
    :other_(other),strings_(strings),profile_(profile) {}
std::uint8_t NativeSkinnedAnimationReferences::source_is_open(std::uintptr_t e,void* p) {
    return other_.source_is_open(e,p);
}
std::uint32_t NativeSkinnedAnimationReferences::source_seek(std::uintptr_t e,void* p,
    std::uint32_t low,std::uint32_t high,std::uint32_t origin) {
    return other_.source_seek(e,p,low,high,origin);
}
void NativeSkinnedAnimationReferences::source_read(std::uintptr_t e,void* p,void* data,
    std::uint32_t size,std::uint32_t* actual) {
    other_.source_read(e,p,data,size,actual);
}
void NativeSkinnedAnimationReferences::source_write(std::uintptr_t e,void* p,const void* data,
    std::uint32_t size,std::uint32_t* actual) {
    other_.source_write(e,p,data,size,actual);
}
void NativeSkinnedAnimationReferences::source_zero_reference(std::uintptr_t target,
    void* item,std::uintptr_t captured_table) {
    if(target!=0x00bd30e0u||captured_table!=0x00d63700u) {
        other_.source_zero_reference(target,item,captured_table);
        return;
    }
    // BD30E0 itself tests null, then reads the CURRENT object profile/slot4.
    if(!item)return;
    const auto current=*static_cast<const volatile std::uint32_t*>(item);
    if(current!=0x00d63700u||!profile_||profile_[1]!=0x00b92fa0u)
        throw std::runtime_error("Current skinned animation item terminal is outside the reconstructed domain");
    delete_native_skinned_animation_item_00b92fa0(item,1,strings_);
}
} // namespace bsp
