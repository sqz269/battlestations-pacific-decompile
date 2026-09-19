#include "bsp/native_bit_cursor_write.hpp"
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native bit writing requires MSVC Win32 pointer arithmetic.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;using B=std::uint8_t;
U address(const void* p){return reinterpret_cast<U>(p);}
const B* pointer(U p){return reinterpret_cast<const B*>(p);}
volatile B* writable(const B* p){return const_cast<volatile B*>(p);}
B left(B b,U n){return static_cast<B>(static_cast<U>(b)<<(n&31u));}
B right(B b,U n){return static_cast<B>(static_cast<U>(b)>>(n&31u));}
}
void write_native_bits_00428f50(NativeBitCursor* cursor,const void* source,U bits){
    volatile auto& c=*cursor;
    auto* in=static_cast<const volatile B*>(source);
    U whole=bits>>3;
    while(whole!=0){
        const B value=*in;
        const U shift=static_cast<U>(c.bit_0c);
        auto* out=writable(c.current_08);
        const B low=left(value,shift);
        ++in;
        *out=static_cast<B>(*out|low);
        c.current_08=pointer(address(c.current_08)+1u);
        // Native reloads the input after its possibly overlapping output OR.
        const B reloaded=in[-1];
        const U carry_shift=8u-static_cast<U>(c.bit_0c);
        out=writable(c.current_08);
        --whole;
        *out=right(reloaded,carry_shift);
    }
    bits&=7u;
    if(bits!=0){
        const B mask=static_cast<B>(0xffu>>(8u-bits));
        const U shift=static_cast<U>(c.bit_0c);
        const B value=static_cast<B>(mask&*in);
        auto* out=writable(c.current_08);
        *out=static_cast<B>(*out|left(value,shift));
        const U carry_shift=8u-static_cast<U>(c.bit_0c);
        c.bit_0c=static_cast<std::int32_t>(static_cast<U>(c.bit_0c)+bits);
        const B carry=right(value,carry_shift);
        const auto offset=c.bit_0c;
        if(offset>7){
            c.current_08=pointer(address(c.current_08)+1u);
            c.bit_0c=offset-8;
            *writable(c.current_08)=carry;
        }
    }
}
void write_native_byte_bits_00428ff0(NativeBitCursor* c,U value,U bits){write_native_bits_00428f50(c,&value,bits);}
void write_native_bool_bit_004290b0(NativeBitCursor* c,U value){
    // CMP uses only the low argument byte, including for values such as100h.
    const B bit=static_cast<B>(value)!=0?1u:0u;
    write_native_bits_00428f50(c,&bit,1);
}
void write_native_word_bits_00429120(NativeBitCursor* c,U value,U bits){write_native_bits_00428f50(c,&value,bits);}
} // namespace bsp
