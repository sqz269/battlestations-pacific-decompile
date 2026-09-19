#include "bsp/native_bit_cursor_read.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native bit cursors require MSVC Win32 pointer arithmetic.
#endif

namespace bsp {
namespace {
using U=std::uint32_t;
using B=std::uint8_t;
U address(const void* p) {return reinterpret_cast<U>(p);}
const B* pointer(U p) {return reinterpret_cast<const B*>(p);}
B load(const B* p) {return *static_cast<const volatile B*>(p);}
B right_byte(B v,U n) {return static_cast<B>(static_cast<U>(v)>>(n&31u));}
B left_byte(B v,U n) {return static_cast<B>(static_cast<U>(v)<<(n&31u));}
}

void rewind_native_bits_00428b80(NativeBitCursor* cursor,U bits) {
    volatile auto& c=*cursor;
    c.current_08=pointer(address(c.current_08)-(bits>>3));
    c.bit_0c=static_cast<std::int32_t>(static_cast<U>(c.bit_0c)-(bits&7u));
    if(c.bit_0c<0) {
        c.current_08=pointer(address(c.current_08)-1u);
        c.bit_0c=static_cast<std::int32_t>(static_cast<U>(c.bit_0c)+8u);
    }
}

void read_native_bits_00428bb0(NativeBitCursor* cursor,void* destination,U bits) {
    volatile auto& c=*cursor;
    auto* out=static_cast<volatile B*>(destination);
    U whole=bits>>3;
    while(whole!=0) {
        // Preserve the store before advancing the cursor and reading a carry.
        *out=right_byte(load(c.current_08),static_cast<U>(c.bit_0c));
        c.current_08=pointer(address(c.current_08)+1u);
        const auto offset=c.bit_0c;
        const auto* next=c.current_08;
        if(offset>0) *out=static_cast<B>(*out|left_byte(load(next),8u-static_cast<U>(offset)));
        bits-=8;
        if(bits!=0) ++out;
        --whole;
    }
    bits&=7u;
    if(bits!=0) {
        const B mask=static_cast<B>(0xffu>>(8u-bits));
        const B low=static_cast<B>(right_byte(load(c.current_08),static_cast<U>(c.bit_0c))&mask);
        *out=low;
        const U next=address(c.current_08)+1u;
        const U length=c.length_04;
        const U end=length+address(c.base_00);
        if(next<end) {
            const B high=load(pointer(next));
            *out=static_cast<B>((left_byte(high,8u-static_cast<U>(c.bit_0c))&mask)|low);
        }
        c.bit_0c=static_cast<std::int32_t>(static_cast<U>(c.bit_0c)+bits);
        const auto offset=c.bit_0c;
        if(offset>7) {
            c.current_08=pointer(address(c.current_08)+1u);
            c.bit_0c=offset-8;
        }
    }
}

void read_native_u8_bits_00428c70(NativeBitCursor* c,B* out,U bits) {
    *out=0;read_native_bits_00428bb0(c,out,bits);
}
void read_native_i8_bits_00428c80(NativeBitCursor* c,std::int8_t* output,U bits) {
    auto* out=reinterpret_cast<B*>(output);
    *out=0;read_native_bits_00428bb0(c,out,bits);
    if(static_cast<std::int32_t>(bits)<8) {
        const B mask=left_byte(0xffu,bits-1u);
        if((*out&mask)!=0) *out=static_cast<B>(*out|mask);
    }
}
void read_native_u16_bits_00428cb0(NativeBitCursor* c,std::uint16_t* out,U bits) {
    *out=0;read_native_bits_00428bb0(c,out,bits);
}
void read_native_i16_bits_00428cd0(NativeBitCursor* c,std::int16_t* output,U bits) {
    auto* out=reinterpret_cast<std::uint16_t*>(output);
    *out=0;read_native_bits_00428bb0(c,out,bits);
    if(static_cast<std::int32_t>(bits)<16) {
        const auto mask=static_cast<std::uint16_t>(0xffffu<<((bits-1u)&31u));
        if((*out&mask)!=0) *out=static_cast<std::uint16_t>(*out|mask);
    }
}
void read_native_u32_bits_00428d10(NativeBitCursor* c,U* out,U bits) {
    *out=0;read_native_bits_00428bb0(c,out,bits);
}
void read_native_i32_bits_00428d30(NativeBitCursor* c,std::int32_t* output,U bits) {
    auto* out=reinterpret_cast<U*>(output);
    *out=0;read_native_bits_00428bb0(c,out,bits);
    if(static_cast<std::int32_t>(bits)<32) {
        const U mask=0xffffffffu<<((bits-1u)&31u);
        if((*out&mask)!=0) *out|=mask;
    }
}
void read_native_bool_bit_00428d70(NativeBitCursor* c,bool* out) {
    B value=0;read_native_bits_00428bb0(c,&value,1);*out=value==1;
}
void allocate_native_bit_string_00428da0(NativeBitCursor* c,char** out) {
    B length=0;read_native_bits_00428bb0(c,&length,8);
    const U bytes=static_cast<U>(length)+1u;
    auto* data=static_cast<char*>(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));
    *out=data;
    read_native_bits_00428bb0(c,data,static_cast<U>(length)*8u);
    (*out)[length]=0;
}
void read_native_bit_string_00428df0(NativeBitCursor* c,char* out) {
    B length=0;read_native_bits_00428bb0(c,&length,8);
    read_native_bits_00428bb0(c,out,static_cast<U>(length)*8u);out[length]=0;
}
void read_native_word_bits_00428e30(NativeBitCursor* c,std::uint16_t* out,U bits) {
    *out=0;read_native_bits_00428bb0(c,out,bits);
}
void read_native_word_array_00428e50(NativeBitCursor* c,std::uint16_t* out,U count,U bits) {
    while(count!=0) {*out=0;read_native_bits_00428bb0(c,out,bits);++out;--count;}
}
void read_native_u64_bits_00428e90(NativeBitCursor* c,U* out) {
    U value[2];read_native_bits_00428bb0(c,value,64);
    out[0]=value[0];out[1]=value[1];
}
} // namespace bsp
