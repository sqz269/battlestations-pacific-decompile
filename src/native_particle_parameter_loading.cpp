#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_pooled_text.hpp"
#include "bsp/native_weak_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle parameter loading requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Bindings = NativeParticleParameterLoadingBindings;
using RawContext = NativeParticleParameterBuilderRawContext;
using Builder = NativeParticleParameterBuilderStorage;
struct Key {
    float x, y, incoming[2], outgoing[2];
    std::uint32_t kind;
    std::byte coefficients[16];
};
static_assert(sizeof(Key)==0x2c && sizeof(Builder)==0x10 && sizeof(void*)==4);
static_assert(sizeof(Bindings)==68 && offsetof(Bindings,backward_limit_00ce3928)==12);
static_assert(offsetof(Bindings,tangent_scale_00cf1450)==36 && offsetof(Bindings,integral_quarter_00d7a348)==48);
template<class T> T read(const void* p,std::size_t n) noexcept {
    T v; std::memcpy(&v,static_cast<const std::byte*>(p)+n,sizeof v); return v;
}
template<class T> void write(void* p,std::size_t n,T v) noexcept {
    std::memcpy(static_cast<std::byte*>(p)+n,&v,sizeof v);
}
Key* key(Builder& p,std::int32_t i) noexcept {
    return reinterpret_cast<Key*>(static_cast<std::byte*>(p.records_00)+static_cast<std::uint32_t>(i)*0x2cu);
}
void copy_words(void* destination,const void* source,std::size_t count) noexcept {
    // REP MOVSD's forward copy, including overlap and opaque scratch bytes.
    for(std::size_t i=0;i<count;++i) write(destination,i*4,read<std::uint32_t>(source,i*4));
}
// Every member points at the ORIGINAL binding's pointer member, not a cached
// numeric value/address. A CRT callback can rebind any later constant load.
struct EndpointNumericView {
    const CameraAxesCrtAccess* crt;
    const volatile double* const* backward_limit;
    const volatile float* const* backward_clamp;
    const volatile double* const* forward_limit;
    const volatile float* const* forward_clamp;
    const volatile float* const* endpoint_time;
    const volatile float* const* endpoint_backward;
};
struct EndpointCallContext {
    EndpointNumericView numeric;
    Bindings* legacy;
    RawContext* raw;
};
static_assert(sizeof(EndpointNumericView)==28 && offsetof(EndpointCallContext,numeric)==0);
static_assert(offsetof(EndpointNumericView,backward_limit)==4 &&
    offsetof(EndpointNumericView,endpoint_backward)==24);
template<class Context> EndpointNumericView numeric_view(Context& a) noexcept {
    return {&a.crt,&a.backward_limit_00ce3928,&a.backward_clamp_00ce3cb4,
        &a.forward_limit_00d7a3a0,&a.forward_clamp_00d7a2f0,
        &a.endpoint_time_00ce3d08,&a.endpoint_backward_00ce65d8};
}
std::int32_t __fastcall insert_bridge(void*,EndpointCallContext*,const void*);

// Native 00AFB670..00AFB6A7; symbolic current-data loads only.
__declspec(naked) void* __fastcall key_construct(void*) {
    __asm {
        mov edx, dword ptr [esp - 4] // 00afb670
        xorps xmm0, xmm0 // 00afb674
        mov eax, ecx // 00afb677
        mov ecx, dword ptr [esp - 8] // 00afb679
        mov dword ptr [eax + 8], ecx // 00afb67d
        mov ecx, dword ptr [esp - 8] // 00afb680
        mov dword ptr [eax + 0ch], edx // 00afb684
        mov edx, dword ptr [esp - 4] // 00afb687
        sub esp, 8 // 00afb68b
        movss dword ptr [eax], xmm0 // 00afb68e
        movss dword ptr [eax + 4], xmm0 // 00afb692
        mov dword ptr [eax + 018h], 1 // 00afb697
        mov dword ptr [eax + 010h], ecx // 00afb69e
        mov dword ptr [eax + 014h], edx // 00afb6a1
        add esp, 8 // 00afb6a4
        ret // 00afb6a7
    }
}

// Native 00AFBC90..00AFBD02; symbolic current-data loads only.
__declspec(naked) void __fastcall incoming(float*,const EndpointNumericView*,float,float) {
    __asm {
        push ebx
        mov ebx,edx
        push ecx // 00afbc90
        push eax
        mov eax,[ebx+4]
        mov eax,[eax]
        fld qword ptr [eax] // 00afbc91
        pop eax
        push esi // 00afbc97
        fld dword ptr [esp + 010h] // 00afbc98
        mov esi, ecx // 00afbc9c
        fcomip st(0), st(1) // 00afbc9e
        fstp st(0) // 00afbca0
        jbe L_00afbcae // 00afbca2
        push eax
        mov eax,[ebx+8]
        mov eax,[eax]
        movss xmm0, dword ptr [eax] // 00afbca4
        pop eax
        jmp L_00afbcb4 // 00afbcac
    L_00afbcae:
        movss xmm0, dword ptr [esp + 010h] // 00afbcae
    L_00afbcb4:
        fld dword ptr [esp + 014h] // 00afbcb4
        movss dword ptr [esi], xmm0 // 00afbcb8
        fld dword ptr [esi] // 00afbcbc
        fstp dword ptr [esp + 010h] // 00afbcbe
        fld dword ptr [esp + 010h] // 00afbcc2
        fld st(1) // 00afbcc6
        fmulp st(2), st(0) // 00afbcc8
        fmul st(0), st(0) // 00afbcca
        faddp st(1), st(0) // 00afbccc
        fstp dword ptr [esp + 04h] // 00afbcce
        fld dword ptr [esp + 04h] // 00afbcd2
        mov ecx,[ebx+0]
        call native_crt_sqrt_st0_00bf7030 // 00afbcd6
        fstp dword ptr [esp + 04h] // 00afbcdb
        fld dword ptr [esp + 04h] // 00afbcdf
        fstp dword ptr [esp + 04h] // 00afbce3
        fld dword ptr [esp + 010h] // 00afbce7
        fld dword ptr [esp + 04h] // 00afbceb
        fld st(0) // 00afbcef
        fdivp st(2), st(0) // 00afbcf1
        fxch st(1) // 00afbcf3
        fstp dword ptr [esi] // 00afbcf5
        fdivr dword ptr [esp + 014h] // 00afbcf7
        fstp dword ptr [esi + 4] // 00afbcfb
        pop esi // 00afbcfe
        pop ecx // 00afbcff
        pop ebx
        ret 8 // 00afbd00
    }
}

// Native 00AFBD10..00AFBD82; symbolic current-data loads only.
__declspec(naked) void __fastcall outgoing(float*,const EndpointNumericView*,float,float) {
    __asm {
        push ebx
        mov ebx,edx
        push ecx // 00afbd10
        fld dword ptr [esp + 0ch] // 00afbd11
        push esi // 00afbd15
        push eax
        mov eax,[ebx+12]
        mov eax,[eax]
        fld qword ptr [eax] // 00afbd16
        pop eax
        mov esi, ecx // 00afbd1c
        fcomip st(0), st(1) // 00afbd1e
        fstp st(0) // 00afbd20
        jbe L_00afbd2e // 00afbd22
        push eax
        mov eax,[ebx+16]
        mov eax,[eax]
        movss xmm0, dword ptr [eax] // 00afbd24
        pop eax
        jmp L_00afbd34 // 00afbd2c
    L_00afbd2e:
        movss xmm0, dword ptr [esp + 010h] // 00afbd2e
    L_00afbd34:
        fld dword ptr [esp + 014h] // 00afbd34
        movss dword ptr [esi], xmm0 // 00afbd38
        fld dword ptr [esi] // 00afbd3c
        fstp dword ptr [esp + 010h] // 00afbd3e
        fld dword ptr [esp + 010h] // 00afbd42
        fld st(1) // 00afbd46
        fmulp st(2), st(0) // 00afbd48
        fmul st(0), st(0) // 00afbd4a
        faddp st(1), st(0) // 00afbd4c
        fstp dword ptr [esp + 04h] // 00afbd4e
        fld dword ptr [esp + 04h] // 00afbd52
        mov ecx,[ebx+0]
        call native_crt_sqrt_st0_00bf7030 // 00afbd56
        fstp dword ptr [esp + 04h] // 00afbd5b
        fld dword ptr [esp + 04h] // 00afbd5f
        fstp dword ptr [esp + 04h] // 00afbd63
        fld dword ptr [esp + 010h] // 00afbd67
        fld dword ptr [esp + 04h] // 00afbd6b
        fld st(0) // 00afbd6f
        fdivp st(2), st(0) // 00afbd71
        fxch st(1) // 00afbd73
        fstp dword ptr [esi] // 00afbd75
        fdivr dword ptr [esp + 014h] // 00afbd77
        fstp dword ptr [esi + 4] // 00afbd7b
        pop esi // 00afbd7e
        pop ecx // 00afbd7f
        pop ebx
        ret 8 // 00afbd80
    }
}

// Native 00AFC360..00AFC46B; symbolic current-data loads only.
__declspec(naked) void __fastcall endpoints(void*,EndpointCallContext*,float,float) {
    __asm {
        push ebx
        mov ebx,edx
        sub esp, 034h // 00afc360
        xorps xmm0, xmm0 // 00afc363
        fld dword ptr [esp + 040h] // 00afc366
        fsub dword ptr [esp + 03ch] // 00afc36a
        mov eax, dword ptr [esp + 00h] // 00afc36e
        movss xmm1, dword ptr [esp + 03ch] // 00afc371
        movss dword ptr [esp + 00h], xmm0 // 00afc377
        mov edx, dword ptr [esp + 00h] // 00afc37c
        fstp dword ptr [esp + 00h] // 00afc37f
        fld dword ptr [esp + 00h] // 00afc382
        push esi // 00afc385
        mov esi, ecx // 00afc386
        mov ecx, dword ptr [esp + 08h] // 00afc388
        sub esp, 8 // 00afc38c
        fstp dword ptr [esp + 04h] // 00afc38f
        mov dword ptr [esp + 01ch], eax // 00afc393
        push eax
        mov eax,[ebx+20]
        mov eax,[eax]
        fld dword ptr [eax] // 00afc397
        pop eax
        movss dword ptr [esp + 010h], xmm0 // 00afc39d
        mov eax, dword ptr [esp + 010h] // 00afc3a3
        fstp dword ptr [esp + 00h] // 00afc3a7
        mov dword ptr [esp + 020h], ecx // 00afc3aa
        lea ecx, [esp + 024h] // 00afc3ae
        movss dword ptr [esp + 014h], xmm0 // 00afc3b2
        movss dword ptr [esp + 018h], xmm1 // 00afc3b8
        mov dword ptr [esp + 02ch], 0 // 00afc3be
        mov dword ptr [esp + 024h], edx // 00afc3c6
        mov dword ptr [esp + 028h], eax // 00afc3ca
        mov edx,ebx
        call outgoing // 00afc3ce
        lea ecx, [esp + 0ch] // 00afc3d3
        push ecx // 00afc3d7
        mov ecx, esi // 00afc3d8
        mov edx,ebx
        call insert_bridge // 00afc3da
        fld dword ptr [esp + 040h] // 00afc3df
        fsub dword ptr [esp + 044h] // 00afc3e3
        push eax
        mov eax,[ebx+20]
        mov eax,[eax]
        movss xmm0, dword ptr [eax] // 00afc3e7
        pop eax
        mov edx, dword ptr [esp + 04h] // 00afc3ef
        mov eax, dword ptr [esp + 08h] // 00afc3f3
        movss dword ptr [esp + 0ch], xmm0 // 00afc3f7
        fstp dword ptr [esp + 040h] // 00afc3fd
        movss xmm0, dword ptr [esp + 044h] // 00afc401
        fld dword ptr [esp + 040h] // 00afc407
        movss dword ptr [esp + 010h], xmm0 // 00afc40b
        xorps xmm0, xmm0 // 00afc411
        sub esp, 8 // 00afc414
        movss dword ptr [esp + 0ch], xmm0 // 00afc417
        fstp dword ptr [esp + 04h] // 00afc41d
        mov ecx, dword ptr [esp + 0ch] // 00afc421
        push eax
        mov eax,[ebx+24]
        mov eax,[eax]
        fld dword ptr [eax] // 00afc425
        pop eax
        mov dword ptr [esp + 024h], edx // 00afc42b
        fstp dword ptr [esp + 00h] // 00afc42f
        movss dword ptr [esp + 010h], xmm0 // 00afc432
        mov edx, dword ptr [esp + 010h] // 00afc438
        mov dword ptr [esp + 01ch], ecx // 00afc43c
        lea ecx, [esp + 01ch] // 00afc440
        mov dword ptr [esp + 02ch], 2 // 00afc444
        mov dword ptr [esp + 028h], eax // 00afc44c
        mov dword ptr [esp + 020h], edx // 00afc450
        mov edx,ebx
        call incoming // 00afc454
        lea eax, [esp + 0ch] // 00afc459
        push eax // 00afc45d
        mov ecx, esi // 00afc45e
        mov edx,ebx
        call insert_bridge // 00afc460
        pop esi // 00afc465
        add esp, 034h // 00afc466
        pop ebx
        ret 8 // 00afc469
    }
}

void* allocate_keys(Bindings& a,std::uint32_t bytes) { return a.allocate_array_00bf55be(bytes); }
void* allocate_keys(RawContext&,std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes});
}
void free_keys(Bindings& a,void* p) noexcept { a.owners.free_array_00bf6989(p); }
void free_keys(RawContext&,void* p) noexcept { singleton_lifetime_free(p); }
template<class Context> void clear_keys(Builder& p,Context& a) noexcept {
    free_keys(a,p.records_00);
    p.records_00=nullptr; p.count_04=0; p.capacity_08=0;
}
template<class Context> void reserve_00afbdb0(Builder& p,std::int32_t requested,Context& a) {
    if(requested<4) requested=4;
    if(requested<=p.capacity_08) return;
    const auto product=static_cast<std::uint64_t>(static_cast<std::uint32_t>(requested))*0x2cu;
    const auto bytes=product>0xffffffffu?0xffffffffu:static_cast<std::uint32_t>(product);
    void* replacement=allocate_keys(a,bytes);
    // DF3090 state0 owns the new allocation through CBB120. Key construction
    // currently cannot throw; retain the actual ownership interval explicitly.
    struct ReplacementUnwind {
        Context& context;
        void* block;
        bool armed{true};
        ~ReplacementUnwind() noexcept { if(armed) free_keys(context,block); }
    } unwind{a,replacement};
    if(replacement) for(std::int32_t i=0;i<requested;++i)
        key_construct(static_cast<std::byte*>(replacement)+static_cast<std::uint32_t>(i)*0x2cu);
    for(std::int32_t i=0;i<p.count_04;++i)
        copy_words(static_cast<std::byte*>(replacement)+static_cast<std::uint32_t>(i)*0x2cu,key(p,i),11);
    free_keys(a,p.records_00);
    p.records_00=replacement;
    p.capacity_08=requested;
    unwind.armed=false;
}
bool strictly_after(float x,float y) noexcept {
    unsigned char result;
    __asm {
        fld y
        fld x
        fcomip st(0),st(1)
        fstp st(0)
        seta result
    }
    return result!=0;
}
template<class Context> std::int32_t insert_00afc260(Builder& p,const Key* source,Context& a) {
    std::int32_t position=0;
    while(position<p.count_04 && strictly_after(source->x,key(p,position)->x)) ++position;
    if(p.count_04==p.capacity_08)
        reserve_00afbdb0(p,static_cast<std::int32_t>(static_cast<std::uint32_t>(p.capacity_08)*2u),a);
    copy_words(key(p,p.count_04),source,11);
    ++p.count_04;
    if(position<p.count_04-1) {
        for(auto i=p.count_04-1;i>position;--i) copy_words(key(p,i),key(p,i-1),11);
        copy_words(key(p,position),source,11);
    }
    return position;
}
std::int32_t __fastcall insert_bridge(void* p,EndpointCallContext* a,const void* source) {
    auto& builder=*static_cast<Builder*>(p);
    const auto* record=static_cast<const Key*>(source);
    return a->raw?insert_00afc260(builder,record,*a->raw):
        insert_00afc260(builder,record,*a->legacy);
}
struct Token {
    NativePooledTextStorage storage;
    NativeStringStorage& strings;
    Token(const void* line,std::int32_t index,NativeStringStorage& domain):strings(domain) {
        get_native_pooled_text_token_00aee3c0(line,&storage,index,strings);
    }
    ~Token() { destroy_native_pooled_text_00aee2a0(&storage,strings); }
    Token(const Token&)=delete;
    Token& operator=(const Token&)=delete;
};
float token_number(const void* line,std::int32_t index,NativeStringStorage& strings) {
    Token token(line,index,strings);
    return static_cast<float>(std::atof(token.storage.data));
}
void token_pair(const void* line,std::int32_t index,float* destination,NativeStringStorage& strings) {
    // Native evaluates the second token first, obtains the first token, then
    // converts second/first and destroys first/second. Do not reorder pooling.
    Token y(line,index+1,strings);
    Token x(line,index,strings);
    const float second=static_cast<float>(std::atof(y.storage.data));
    const float first=static_cast<float>(std::atof(x.storage.data));
    destination[0]=first;
    destination[1]=second;
}

// The current CRT is the provider boundary. Keep its ST(0) return live until
// the native binary32 spill; a C++ double temporary would change x87 status.
void number_to_float(const char* text,float* destination) {
    __asm {
        push text
        call atof
        add esp,4
        mov eax,destination
        fstp dword ptr[eax]
    }
}
template<class Context> struct ParsePolicy;
template<> struct ParsePolicy<Bindings> {
    Bindings& context;
    const void* line;
    Token type;
    ParsePolicy(const void* input,Bindings& a):context(a),line(input),type(input,0,a.owners.strings) {}
    const char* type_name() const noexcept { return type.storage.data; }
    bool finish(bool result) noexcept { return result; }
    std::int32_t count() {
        Token token(line,1,context.owners.strings);
        return std::atol(token.storage.data);
    }
    void number(std::int32_t index,float* destination) {
        *destination=token_number(line,index,context.owners.strings);
    }
    void pair(std::int32_t index,float* destination,bool) {
        token_pair(line,index,destination,context.owners.strings);
    }
    bool first(float value) { return value==*context.first_time_00d7a218; }
    bool last(float value) { return static_cast<double>(value)==*context.last_time_00d7a220; }
    void constant(Builder& p) {
        Token value(line,1,context.owners.strings);
        const float number=static_cast<float>(std::atof(value.storage.data));
        write(p.records_00,4,number);
    }
};
template<> struct ParsePolicy<RawContext> {
    RawContext& context;
    const void* line;
    NativePooledTextStorage type,incoming_y,outgoing_y;
    int state{-1};
    ParsePolicy(const void* input,RawContext& a):context(a),line(input) {
        get_native_pooled_text_token_00aee3c0(line,&type,0,context.strings);
        state=0;
    }
    // DF3114: only type (state0), incoming-y (1), and outgoing-y (2) own
    // cleanup actions. Count/x/y/value temporaries have no FH3 action.
    ~ParsePolicy() noexcept {
        if(state==2) destroy_native_pooled_text_00aee2a0(&outgoing_y,context.strings);
        if(state==1) destroy_native_pooled_text_00aee2a0(&incoming_y,context.strings);
        if(state>=0) destroy_native_pooled_text_00aee2a0(&type,context.strings);
    }
    const char* type_name() const noexcept { return type.data; }
    bool finish(bool result) {
        state=-1;
        destroy_native_pooled_text_00aee2a0(&type,context.strings);
        return result;
    }
    std::int32_t count() {
        NativePooledTextStorage token;
        get_native_pooled_text_token_00aee3c0(line,&token,1,context.strings);
        const auto result=std::atol(token.data);
        destroy_native_pooled_text_00aee2a0(&token,context.strings);
        return result;
    }
    void number(std::int32_t index,float* destination) {
        NativePooledTextStorage token;
        get_native_pooled_text_token_00aee3c0(line,&token,index,context.strings);
        number_to_float(token.data,destination);
        destroy_native_pooled_text_00aee2a0(&token,context.strings);
    }
    void pair(std::int32_t index,float* destination,bool outgoing) {
        auto& y=outgoing?outgoing_y:incoming_y;
        get_native_pooled_text_token_00aee3c0(line,&y,index+1,context.strings);
        state=outgoing?2:1;
        NativePooledTextStorage x;
        get_native_pooled_text_token_00aee3c0(line,&x,index,context.strings);
        float second,first;
        number_to_float(y.data,&second);
        number_to_float(x.data,&first);
        __asm {
            fld second
            fstp second
        }
        // Native copies the two rounded words before releasing either token.
        std::memcpy(destination,&first,4);
        std::memcpy(destination+1,&second,4);
        destroy_native_pooled_text_00aee2a0(&x,context.strings);
        state=0;
        destroy_native_pooled_text_00aee2a0(&y,context.strings);
    }
    bool first(float value) {
        const auto* limit=context.first_time_00d7a218;
        unsigned char result;
        __asm {
            mov eax,limit
            movss xmm0,value
            ucomiss xmm0,dword ptr[eax]
            lahf
            test ah,44h
            setnp result
        }
        return result!=0;
    }
    bool last(float value) {
        const auto* limit=context.last_time_00d7a220;
        unsigned char result;
        __asm {
            mov eax,limit
            fld qword ptr[eax]
            fld value
            fucomip st(0),st(1)
            fstp st(0)
            lahf
            test ah,44h
            setnp result
        }
        return result!=0;
    }
    void constant(Builder& p) {
        NativePooledTextStorage value;
        get_native_pooled_text_token_00aee3c0(line,&value,1,context.strings);
        float rounded;
        number_to_float(value.data,&rounded);
        // Reload backing after atof, then perform the original FLD32/FSTP32.
        void* current=p.records_00;
        __asm {
            mov eax,current
            fld rounded
            fstp dword ptr[eax+4]
        }
        destroy_native_pooled_text_00aee2a0(&value,context.strings);
    }
};
template<class Context> bool parse_builder(void* raw,const void* line,Context& a) {
    auto& p=*static_cast<Builder*>(raw);
    ParsePolicy<Context> policy(line,a);
    const bool hermite=_stricmp(policy.type_name(),"Hermite")==0;
    const bool linear=!hermite && _stricmp(policy.type_name(),"Linear")==0;
    if(hermite || linear) {
        const auto count=policy.count();
        if(count<2) return policy.finish(false);
        p.kind_0c=hermite?2:1;
        clear_keys(p,a);
        for(std::int32_t i=0;i<count;++i) {
            const auto index=static_cast<std::int32_t>(static_cast<std::uint32_t>(i)*(hermite?6u:2u)+2u);
            Key record; // Coefficients retain native indeterminate scratch.
            policy.number(index,&record.x);
            policy.number(index+1,&record.y);
            if(i==0) { if(!policy.first(record.x)) return policy.finish(false); record.kind=0; }
            else if(i==count-1) { if(!policy.last(record.x)) return policy.finish(false); record.kind=2; }
            else record.kind=1;
            if(hermite) {
                policy.pair(index+2,record.incoming,false);
                policy.pair(index+4,record.outgoing,true);
            } else {
                record.incoming[0]=*a.linear_incoming_00d7a260; record.incoming[1]=0.0f;
                record.outgoing[0]=*a.linear_outgoing_00d7a24c; record.outgoing[1]=0.0f;
            }
            insert_00afc260(p,&record,a);
        }
        return policy.finish(true);
    }
    if(_stricmp(policy.type_name(),"Const")!=0) return policy.finish(false);
    p.kind_0c=0;
    policy.constant(p);
    return policy.finish(true);
}
} // namespace

void* construct_native_particle_parameter_builder_00afbed0(void* raw,Bindings& a) {
    auto& p=*static_cast<Builder*>(raw);
    p.records_00=nullptr; p.count_04=0; p.capacity_08=0;
    try { reserve_00afbdb0(p,32,a); }
    catch(...) { destroy_native_particle_parameter_builder_00af4110(raw,a); throw; }
    return raw;
}
void destroy_native_particle_parameter_builder_00af4110(void* raw,Bindings& a) noexcept {
    clear_keys(*static_cast<Builder*>(raw),a);
}
void initialize_native_particle_parameter_endpoints_00afc360(void* raw,float first,float last,Bindings& a) {
    EndpointCallContext call{numeric_view(a),&a,nullptr};
    endpoints(raw,&call,first,last);
}
float first_native_particle_parameter_value_00afc1b0(const void* raw) {
    return read<float>(read<void*>(raw,0),4);
}
bool parse_native_particle_parameter_00afc470(void* raw,const void* line,Bindings& a) {
    return parse_builder(raw,line,a);
}
void destroy_native_particle_parameter_key_vector_00af4060(void* raw) noexcept {
    auto& p=*static_cast<Builder*>(raw);
    singleton_lifetime_free(p.records_00);
    p.records_00=nullptr; p.count_04=0; p.capacity_08=0;
}
void* construct_native_particle_parameter_builder_00afbed0(void* raw,RawContext& a) {
    auto& p=*static_cast<Builder*>(raw);
    p.records_00=nullptr; p.count_04=0; p.capacity_08=0;
    // DF30BC state0 invokes AF4060 on true unwind. Its noexcept cleanup cannot
    // replace the active allocation exception with a second exception.
    struct Unwind {
        void* object;
        bool armed{true};
        ~Unwind() noexcept { if(armed) destroy_native_particle_parameter_key_vector_00af4060(object); }
    } unwind{raw};
    reserve_00afbdb0(p,32,a);
    unwind.armed=false;
    return raw;
}
void destroy_native_particle_parameter_builder_00af4110(void* raw,RawContext& a) noexcept {
    clear_keys(*static_cast<Builder*>(raw),a);
}
void initialize_native_particle_parameter_endpoints_00afc360(void* raw,float first,float last,RawContext& a) {
    EndpointCallContext call{numeric_view(a),nullptr,&a};
    endpoints(raw,&call,first,last);
}
bool parse_native_particle_parameter_00afc470(void* raw,const void* line,RawContext& a) {
    return parse_builder(raw,line,a);
}
} // namespace bsp
