#include "bsp/native_word_array_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using W=std::uint16_t;
U address(const void* p){return reinterpret_cast<U>(p);}
I increment(I n){return static_cast<I>(static_cast<U>(n)+1u);}
}
void reserve_native_word_array_005296a0(NativeWordArrayStorage* array,I requested){
    const I capacity=requested<1?1:requested;volatile auto& a=*array;
    if(a.capacity_08>=capacity)return;
    const U bytes=static_cast<U>(capacity)*2u;
    auto* block=static_cast<W*>(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));
    I i=0;U destination=address(block);
    while(i<a.size_04){
        if(destination!=0){
            const auto* source=reinterpret_cast<const volatile W*>(address(a.data_00)+static_cast<U>(i)*2u);
            *reinterpret_cast<volatile W*>(destination)=*source;
        }
        i=increment(i);destination+=2u;
    }
    singleton_lifetime_free(a.data_00);
    a.data_00=block;a.capacity_08=capacity;
}
void resize_native_word_array_00529980(NativeWordArrayStorage* array,I size){
    volatile auto& a=*array;if(size>a.capacity_08)reserve_native_word_array_005296a0(array,size);
    I i=a.size_04;
    while(i<size){
        const U destination=address(a.data_00)+static_cast<U>(i)*2u;
        if(destination!=0)*reinterpret_cast<volatile W*>(destination)=0;
        i=increment(i);
    }
    while(size<a.size_04)a.size_04=static_cast<I>(static_cast<U>(a.size_04)-1u);
    a.size_04=size;
}
void destroy_native_word_array_0052ad30(NativeWordArrayStorage* array){
    resize_native_word_array_00529980(array,0);singleton_lifetime_free(static_cast<volatile NativeWordArrayStorage&>(*array).data_00);
}
} // namespace bsp
