#include "bsp/native_bit_cursor_owned_string.hpp"
#include "bsp/native_bit_cursor_write.hpp"
#include <cstring>
namespace bsp {
void write_native_owned_string_00429ac0(NativeBitCursor* c,const void* header,const char* fallback){
    const std::uint8_t length=*static_cast<const volatile std::uint8_t*>(header);
    write_native_bits_00428f50(c,&length,8);
    const auto* bytes=static_cast<const std::uint8_t*>(header);
    const char* data=*reinterpret_cast<char* const volatile*>(bytes+4);
    if(data==nullptr)data=fallback;
    write_native_bits_00428f50(c,data,static_cast<std::uint32_t>(length)*8u);
}
void read_native_owned_string_00429f20(NativeBitCursor* c,void* header,NativeStringRawPoolContext& strings){
    std::uint8_t length=0;char temporary[256];
    read_native_bits_00428bb0(c,&length,8);
    read_native_bits_00428bb0(c,temporary,static_cast<std::uint32_t>(length)*8u);
    temporary[length]=0;
    const volatile char* end=temporary;while(*end!=0)++end;
    const auto count=static_cast<std::uint32_t>(end-temporary);
    resize_native_string_header_0041dd40(header,strings,count,false);
    auto* bytes=static_cast<std::uint8_t*>(header);
    char* const data=*reinterpret_cast<char* volatile*>(bytes+4);
    if(data!=nullptr)std::memmove(data,temporary,*reinterpret_cast<volatile std::uint32_t*>(header));
}
} // namespace bsp
