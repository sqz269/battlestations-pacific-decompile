#include "bsp/native_gui_widget_property_prefix.hpp"
#include "bsp/native_string_compare.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI property prefix requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word=std::uint32_t;
void* at(const void* p,Word n=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+n);
}
Word load(const void* p,Word n=0) noexcept {
    return *static_cast<const volatile Word*>(at(p,n));
}
void store(void* p,Word n,Word value) noexcept {
    *static_cast<volatile Word*>(at(p,n))=value;
}
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
Word scalar(const volatile float& value) noexcept {
    const volatile float* p=&value;
    Word result;
    __asm {
        mov eax,p
        movss xmm0,dword ptr [eax]
        movd result,xmm0
    }
    return result;
}
void white(NativeGuiWidgetPropertyBindings& b) noexcept {
    if ((*reinterpret_cast<const volatile unsigned char*>(&b.white_guard_00f8bcf0)&1)==0) {
        const Word one=scalar(b.one_00d7a24c);
        b.white_guard_00f8bcf0|=1;
        store(b.white_00f8bce0,0,one); store(b.white_00f8bce0,4,one);
        store(b.white_00f8bce0,8,one); store(b.white_00f8bce0,12,one);
    }
}
} // namespace
void read_native_gui_widget_property_prefix_00aaa710(void* widget,void* visitor,
    NativeGuiWidgetPropertyScratch& scratch,NativeGuiWidgetPropertyBindings& b) {
    void* const key=scratch.bytes;
    void* const field=at(key,8);
    void* const fallback=at(key,0x10);
    void* const local=at(key,0x28);
    void* const header=at(local,4);
    void* const position=at(widget,0xc); // native EBP survives every callback
    auto metadata=[&](void* pair) { store(local,0,bits(pair)); };
    auto call=[&](unsigned index) {
        store(key,0,0); store(key,4,bits(b.literals.keys[index]));
        const Word profile=load(visitor);
        const Word target=load(reinterpret_cast<void*>(profile),0xc);
        metadata(key); // deliberately AFTER current target load
        b.dispatch.call_virtual0c(target,visitor,key,field,fallback);
    };
    // Pos: zero three actual local words, then stage fallback/field/key.
    store(local,0x18,0); store(local,0x1c,0); store(local,0x20,0);
    store(fallback,0,5); metadata(fallback);
    store(fallback,4,bits(at(local,0x18)));
    store(field,0,5); metadata(field); store(field,4,bits(position)); call(0);
    // Size/Pivot/Scale have metadata-before-field-tag ordering.
    store(header,0,0); store(header,4,0);
    store(fallback,0,6); metadata(fallback); store(fallback,4,bits(header));
    metadata(field); store(field,0,6); store(field,4,bits(at(widget,0x20))); call(1);
    store(header,0,0); store(header,4,0);
    store(fallback,0,6); metadata(fallback); store(fallback,4,bits(header));
    metadata(field); store(field,0,6); store(field,4,bits(at(widget,0x18))); call(2);
    const Word scale=scalar(b.one_00d7a24c);
    store(header,0,scale); store(header,4,scale);
    store(fallback,0,6); metadata(fallback); store(fallback,4,bits(header));
    metadata(field); store(field,0,6); store(field,4,bits(at(widget,0x28))); call(3);
    store(fallback,0,2); store(fallback,4,0); metadata(fallback);
    store(field,0,2); metadata(field); store(field,4,bits(at(widget,0x48))); call(4);
    white(b);
    store(fallback,0,8); metadata(fallback);
    void* const captured_white=b.white_00f8bce0; // native EBX until HighColor
    store(fallback,4,bits(captured_white));
    store(field,0,8); metadata(field); store(field,4,bits(at(widget,0x50))); call(5);
    if ((*reinterpret_cast<const volatile unsigned char*>(&b.black_guard_00f8bcdc)&1)==0) {
        b.black_guard_00f8bcdc|=1;
        store(b.black_00f8bccc,0,0); store(b.black_00f8bccc,4,0); store(b.black_00f8bccc,8,0);
        const Word alpha=scalar(b.one_00d7a24c); // after all three zero stores
        store(b.black_00f8bccc,12,alpha);
    }
    store(fallback,0,8); store(fallback,4,bits(b.black_00f8bccc)); metadata(fallback);
    store(field,0,8); metadata(field); store(field,4,bits(at(widget,0xa4))); call(6);
    white(b); // LowColor's Lua callback can reset this SAME shared guard.
    store(fallback,4,bits(captured_white)); store(fallback,0,8); metadata(fallback);
    store(field,0,8); metadata(field); store(field,4,bits(at(widget,0xb4))); call(7);
    store(fallback,0,2); store(fallback,4,0); metadata(fallback);
    store(field,0,2); metadata(field); store(field,4,bits(at(widget,0xc4))); call(8);
    store(header,0,0); store(header,4,0);
    store(fallback,0,0); store(fallback,4,bits(b.literals.empty_00ce3a0c)); metadata(fallback);
    store(field,0,0); metadata(field); store(field,4,bits(header)); call(9);
    const char* const data=reinterpret_cast<const char*>(load(header,4));
    if (data!=nullptr && _stricmp(data,b.literals.left_00ce92e4)==0) {
        store(widget,0xe0,1);
    } else {
        const bool right=equal_native_string_header_00425850(header,b.literals.right_00ce92dc);
        store(widget,0xe0,right?2u:0u);
    }
    // Native state becomes -1 BEFORE resolving the current pool for release.
    // No C++ owner destructor: successful return leaves the actual header stale.
    destroy_native_string_header_0041dd20(header,b.strings);
    const Word mode=load(widget,0xe0);
    const Word x=load(position);
    store(local,0xc,x); store(widget,8,x);
    if (mode==0) return;
    const void* const platform=b.platform_0109cf04;
    if (*static_cast<const volatile unsigned char*>(at(platform,0xd))==0) return;
    const void* const spill=at(local,0xc);
    const volatile double* const adjustment=&b.widescreen_offset_00d5c118;
    if (mode==1) {
        __asm {
            mov eax,spill
            fld dword ptr [eax]
            mov eax,adjustment
            fsub qword ptr [eax]
            mov eax,position
            fstp dword ptr [eax]
        }
    } else if (mode==2) {
        __asm {
            mov eax,spill
            fld dword ptr [eax]
            mov eax,adjustment
            fadd qword ptr [eax]
            mov eax,position
            fstp dword ptr [eax]
        }
    }
}
} // namespace bsp
