#include "bsp/native_bit_cursor_numeric_array.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;
U x87_bits(const volatile void* p){U bits;__asm {mov ecx,p
 fld dword ptr [ecx]
 fstp dword ptr bits}return bits;}
}
void write_native_numeric_float_array_00429790(NativeBitCursor* c,const U* values,U count,U zero,U sign,U scale,U width,const NativeBitNumericContext& context){
    const U base=reinterpret_cast<U>(values);
    for(U index=0;index<count;++index){
        const U factor=x87_bits(&scale);const U value=x87_bits(reinterpret_cast<const void*>(base+index*4u));
        write_native_numeric_float_004295c0(c,value,zero,sign,factor,width,context);
    }
}
void read_native_numeric_float_array_004294f0(NativeBitCursor* c,U* values,U count,U zero,U sign,U scale,U width,const NativeBitNumericContext& context){
    U address=reinterpret_cast<U>(values);
    while(count!=0){const U factor=x87_bits(&scale);read_native_numeric_float_004293f0(c,reinterpret_cast<U*>(address),zero,sign,factor,width,context);address+=4u;--count;}
}
} // namespace bsp
