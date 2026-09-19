#include "bsp/native_bit_cursor_fields.hpp"
namespace bsp {
using U=std::uint32_t;
void write_native_signed_byte_bits_00429010(NativeBitCursor* c,U value,U bits){write_native_bits_00428f50(c,&value,bits);}
void write_native_signed_word_bits_00429030(NativeBitCursor* c,U value,U bits){write_native_bits_00428f50(c,&value,bits);}
void write_native_unsigned_dword_bits_00429070(NativeBitCursor* c,U value,U bits){write_native_bits_00428f50(c,&value,bits);}
void write_native_bit_string_004290d0(NativeBitCursor* c,const char* value){
    const volatile char* end=value;
    while(*end!=0)++end;
    const auto length=static_cast<std::uint8_t>(reinterpret_cast<std::uintptr_t>(end)-reinterpret_cast<std::uintptr_t>(value));
    write_native_bits_00428f50(c,&length,8);
    write_native_bits_00428f50(c,value,static_cast<U>(length)*8u);
}
void write_native_u64_bits_00429180(NativeBitCursor* c,U low,U high){
    const U words[]={low,high};write_native_bits_00428f50(c,words,64);
}
} // namespace bsp
