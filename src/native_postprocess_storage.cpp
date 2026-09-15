#include "bsp/native_postprocess_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This recovered storage ABI requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result;std::memcpy(&result,&bits,4);return result;
}
std::int32_t increment(std::int32_t value) noexcept {
    return signed_bits(static_cast<std::uint32_t>(value)+1u);
}
std::uintptr_t offset(const void* base,std::int32_t index,std::uint32_t stride) noexcept {
    return reinterpret_cast<std::uintptr_t>(base)+static_cast<std::uint32_t>(index)*stride;
}
void destroy_items(NativeRenderPointerArrayStorage& items) {
    resize_native_instance_entry_pointers_00b1c770(items,0);
    singleton_lifetime_free(items.data_00);
}
}

NativeRenderPointerArrayStorage* __fastcall copy_native_postprocess_item_pointers_00b77790(
    NativeRenderPointerArrayStorage& destination,void*,const NativeRenderPointerArrayStorage& source) {
    resize_native_instance_entry_pointers_00b1c770(destination,0);
    volatile auto& dst=destination;
    const volatile auto& src=source;
    reserve_native_instance_entry_pointers_00b1c500(destination,src.count_04);
    for(std::int32_t i=0;i<src.count_04;i=increment(i)) {
        const auto capacity=dst.capacity_08;
        const bool grow=dst.count_04==capacity;
        const auto input=offset(src.data_00,i,4);
        if(grow) {
            auto doubled=signed_bits(static_cast<std::uint32_t>(capacity)*2u);
            if(doubled<=1)doubled=1;
            reserve_native_instance_entry_pointers_00b1c500(destination,doubled);
        }
        const auto count=dst.count_04;
        const auto output=offset(dst.data_00,count,4);
        if(output)std::memcpy(reinterpret_cast<void*>(output),reinterpret_cast<const void*>(input),4);
        dst.count_04=increment(dst.count_04);
    }
    return &destination;
}

void __fastcall reserve_native_postprocess_node_tracks_00b78a90(
    NativePostprocessNodeTracksArray& array,void*,std::int32_t requested) {
    if(requested<1)requested=1;
    volatile auto& actual=array;
    if(actual.capacity_08>=requested)return;
    const auto bytes=static_cast<std::uint32_t>(requested)*16u;
    auto* replacement=static_cast<NativePostprocessNodeTracks*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object,bytes,bytes}));
    for(std::int32_t i=0;i<actual.count_04;i=increment(i)) {
        const auto target=offset(replacement,i,16);
        if(target) {
            auto* dst=reinterpret_cast<NativePostprocessNodeTracks*>(target);
            const auto* src=reinterpret_cast<const NativePostprocessNodeTracks*>(offset(actual.data_00,i,16));
            dst->node_00=src->node_00;
            dst->items_04.data_00=nullptr;
            dst->items_04.count_04=0;
            dst->items_04.capacity_08=0;
            // Native state0 only invokes placement delete, whose body is RET.
            // Do not invent cleanup of partial copies or the new outer block.
            copy_native_postprocess_item_pointers_00b77790(dst->items_04,nullptr,src->items_04);
        }
    }
    for(std::int32_t i=0;i<actual.count_04;i=increment(i)) {
        auto* row=reinterpret_cast<NativePostprocessNodeTracks*>(offset(actual.data_00,i,16));
        destroy_items(row->items_04);
    }
    singleton_lifetime_free(actual.data_00);
    actual.data_00=replacement;
    actual.capacity_08=requested;
}

void __fastcall resize_native_postprocess_node_tracks_00b78d70(
    NativePostprocessNodeTracksArray& array,void*,std::int32_t requested) {
    volatile auto& actual=array;
    if(actual.capacity_08<requested)reserve_native_postprocess_node_tracks_00b78a90(array,nullptr,requested);
    auto current=actual.count_04;
    if(current<requested) {
        auto displacement=static_cast<std::uint32_t>(current)*16u;
        auto remaining=static_cast<std::uint32_t>(requested)-static_cast<std::uint32_t>(current);
        do {
            const auto target=reinterpret_cast<std::uintptr_t>(actual.data_00)+displacement;
            if(target) {
                auto& row=*reinterpret_cast<NativePostprocessNodeTracks*>(target);
                row.items_04.data_00=nullptr;
                row.items_04.count_04=0;
                row.items_04.capacity_08=0;
            }
            displacement+=16u;
        } while(--remaining);
    }
    while(requested<actual.count_04) {
        auto* backing=actual.data_00;
        actual.count_04=signed_bits(static_cast<std::uint32_t>(actual.count_04)-1u);
        auto* row=reinterpret_cast<NativePostprocessNodeTracks*>(offset(backing,actual.count_04,16));
        destroy_items(row->items_04);
    }
    actual.count_04=requested;
}

__declspec(naked) std::int32_t __fastcall native_track_item_count_00b8a2d0(const void*) noexcept {
    __asm {mov eax,dword ptr [ecx+0xc]
        ret}
}
__declspec(naked) void* __fastcall native_track_item_at_00b8a370(const void*,void*,std::uint32_t) noexcept {
    __asm {mov eax,dword ptr [ecx+8]
        mov ecx,dword ptr [esp+4]
        mov eax,dword ptr [eax+ecx*4]
        ret 4}
}

} // namespace bsp
