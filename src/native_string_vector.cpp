#include "bsp/native_string_vector.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <algorithm>
#include <cstring>
namespace bsp {
namespace {
NativeMeshWeightNameStorage* at(NativeMeshWeightNameStorage* data,std::int32_t index){
    return reinterpret_cast<NativeMeshWeightNameStorage*>(reinterpret_cast<std::uintptr_t>(data)+static_cast<std::uint32_t>(index)*8u);
}
void release_name(NativeMeshWeightNameStorage& name,NativeStringStorage& strings){
    char* const data=name.data_04;if(data)strings.release(data,name.length_00+1u);
}
}
void reserve_native_string_vector_00426520(NativeStringVectorStorage& names,std::int32_t requested,NativeStringStorage& strings){
    requested=std::max(requested,1);if(names.capacity_08>=requested)return;
    const auto bytes=static_cast<std::uint32_t>(requested)*8u;
    auto* const fresh=static_cast<NativeMeshWeightNameStorage*>(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));
    for(std::int32_t i=0;i<names.count_04;++i){
        auto* const destination=at(fresh,i);
        if(destination){
            auto* const source=at(names.data_00,i);destination->length_00=0;destination->data_04=nullptr;
            copy_native_string_header_00be0a30_fragment(destination,strings,source);
        }
    }
    for(std::int32_t i=0;i<names.count_04;++i)release_name(*at(names.data_00,i),strings);
    singleton_lifetime_free(names.data_00);names.data_00=fresh;names.capacity_08=requested;
}
void resize_native_string_vector_00427110(NativeStringVectorStorage& names,std::int32_t count,NativeStringStorage& strings){
    if(count>names.capacity_08)reserve_native_string_vector_00426520(names,count,strings);
    for(auto i=names.count_04;i<count;++i){auto* const entry=at(names.data_00,i);if(entry){entry->length_00=0;entry->data_04=nullptr;}}
    while(count<names.count_04){--names.count_04;release_name(*at(names.data_00,names.count_04),strings);}
    names.count_04=count;
}
void destroy_native_string_vector_004d0fa0(NativeStringVectorStorage& names,NativeStringStorage& strings){
    resize_native_string_vector_00427110(names,0,strings);singleton_lifetime_free(names.data_00);
}
} // namespace bsp
