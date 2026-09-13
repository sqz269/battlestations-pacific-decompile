#include "bsp/native_vfs_device_route.hpp"
#include "bsp/native_vfs_lookup_routes.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "original_data.hpp"

#include <Windows.h>
#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using U = std::uint32_t;
struct Case { bool empty, owner, alias, nul_prefix; std::uint8_t result; };
constexpr Case cases[] = {
    {true,false,false,false,0x80}, {false,false,false,false,0x80},
    {false,false,false,false,0}, {false,false,true,false,0},
    {false,false,true,true,0x80}, {true,false,false,false,0},
    {false,true,false,false,0x80}, {false,true,false,false,0}
};
const Case* selected;
unsigned case_id, visits;
unsigned char* arena;
unsigned char* code;
bsp::NativeStringPoolStorage* volatile pool;
volatile U return_gate;
void* volatile publication;
bsp::SingletonLifetimeDomain* lifetime;
bsp::NativeVfsLookupRouteContext* lookup;
bsp::NativeVfsDeviceRouteContext* device;
std::string seen_name;

void* at(U offset) { return arena + offset; }
U word(const void* p, U offset=0) { U v; std::memcpy(&v,static_cast<const char*>(p)+offset,4); return v; }
void put(void* p,U offset,U v) { std::memcpy(static_cast<char*>(p)+offset,&v,4); }
void ptr(U offset,U target) { put(arena,offset,reinterpret_cast<U>(at(target))); }
void text(U header,U data,const char* s) {
    put(arena,header,static_cast<U>(std::strlen(s)));ptr(header+4,data);
    std::memcpy(at(data),s,std::strlen(s)+1);
}
void fail(const char* why) { throw std::runtime_error(why); }
void invalid(void*) { fail("unexpected invalid-parameter callback"); }
void unused_destroy(void*,void*,U) noexcept { std::terminate(); }
const bsp::SingletonLifetimeCallbacks crt{nullptr,&unused_destroy,&invalid};
void* original(U address) { return code + address - 0x400000u; }

void __fastcall resize_bridge(void* h,void*,U n,U preserve) {
    bsp::resize_native_string_header_0041dd40(h,lookup->physical.strings,n,preserve!=0);
}
void* __fastcall cstring_bridge(void* h,void*,const char* s) {
    return bsp::construct_native_string_cstring_0041e870(h,s,lookup->physical.strings);
}
void* __fastcall concat_bridge(const void* left,void*,void* out,const void* right) {
    return bsp::concatenate_native_string_headers_004261a0(left,out,right,lookup->physical.strings);
}
void __fastcall normalize_bridge(void* h,void*) {
    bsp::normalize_native_resource_path_header_00bee690(h,lookup->physical.strings);
}
void __fastcall traverse_bridge(void* manager,void*,const void* name,void* visitor) {
    bsp::visit_native_vfs_lookup_mounts_00bdd0a0(manager,name,visitor,*lookup);
}
void* __cdecl getter_bridge() { return bsp::native_string_pool_get_or_create_00419cc0(pool,*lifetime); }
void __fastcall return_bridge(bsp::NativeStringPoolStorage* owner,void*,void* block,U n,U) {
    bsp::return_native_string_pool_00bd1510(owner,block,n,return_gate);
}
std::uint8_t __fastcall resolve_bridge(void* provider,void*,const void* suffix,void* output) {
    if(provider!=at(0x300)) fail("captured provider changed before invocation");
    ++visits;
    const auto* data=reinterpret_cast<const char*>(word(suffix,4));
    seen_name=data?std::string(data,word(suffix)):std::string();
    ptr(0x518,0x340); // The callback's later payload reload must select this provider.
    if(selected->result!=0)
        bsp::assign_native_string_cstring_0041e350(output,"b",lookup->physical.strings);
    return selected->result;
}
class Resolver final : public bsp::NativeVfsProviderResolveDispatch {
public:
    std::uint8_t invoke_resolve(std::uintptr_t entry,void* provider,const void* suffix,void* out) override {
        if(entry!=reinterpret_cast<std::uintptr_t>(&resolve_bridge)) fail("captured resolve entry");
        return resolve_bridge(provider,nullptr,suffix,out);
    }
};
__declspec(naked) U call0(void*,void*) { __asm {
    mov eax,[esp+4]
    mov ecx,[esp+8]
    call eax
    ret
} }
__declspec(naked) U call2(void*,void*,const void*,const void*) { __asm {
    mov eax,[esp+4]
    mov ecx,[esp+8]
    mov edx,[esp+16]
    push edx
    mov edx,[esp+16]
    push edx
    call eax
    ret
} }
void jump(U address,void* target) {
    auto* p=static_cast<unsigned char*>(original(address));p[0]=0xe9;
    const U delta=reinterpret_cast<U>(target)-reinterpret_cast<U>(p)-5u;
    std::memcpy(p+1,&delta,4);
}
void* fixed(U address,std::size_t bytes,DWORD protection) {
    void* p=VirtualAlloc(reinterpret_cast<void*>(address),bytes,MEM_RESERVE|MEM_COMMIT,protection);
    if(p!=reinterpret_cast<void*>(address)) fail("fixed fixture mapping unavailable");
    return p;
}
void install() {
    arena=static_cast<unsigned char*>(fixed(0x31000000,0x10000,PAGE_READWRITE));
    code=static_cast<unsigned char*>(fixed(0x34000000,0x800000,PAGE_EXECUTE_READWRITE));
    fixed(0xd60000,0x10000,PAGE_READWRITE);fixed(0xce0000,0x10000,PAGE_READWRITE);
    std::memcpy(reinterpret_cast<void*>(0xce7898),"/",2);
    const U profile[3]={0xbdbe00,0xbdbc70,0xbdb670};
    std::memcpy(reinterpret_cast<void*>(0xd683f4),profile,sizeof profile);
    for(const auto& s:spans) std::memcpy(original(s.address),s.bytes,s.size);
    jump(0x41dd40,reinterpret_cast<void*>(&resize_bridge));
    jump(0x41e870,reinterpret_cast<void*>(&cstring_bridge));
    jump(0x4261a0,reinterpret_cast<void*>(&concat_bridge));
    jump(0xbee690,reinterpret_cast<void*>(&normalize_bridge));
    jump(0xbdd0a0,reinterpret_cast<void*>(&traverse_bridge));
    jump(0x419cc0,reinterpret_cast<void*>(&getter_bridge));
    jump(0xbd1510,reinterpret_cast<void*>(&return_bridge));
    jump(0xbf7680,reinterpret_cast<void*>(&std::memmove));
    FlushInstructionCache(GetCurrentProcess(),nullptr,0);
}
void setup() {
    std::memset(arena,0,0x1000);visits=0;seen_name.clear();publication=arena;
    ptr(0x300,0x800);ptr(0x340,0x800);put(arena,0x310,0x1111);put(arena,0x350,0x2222);
    put(arena,0x824,reinterpret_cast<U>(&resolve_bridge));
    ptr(0x40,0x480);ptr(0x480,0x500);ptr(0x484,0x500);ptr(0x488,0x500);arena[0x4a1]=1;
    ptr(0x500,0x480);ptr(0x504,0x480);ptr(0x508,0x480);ptr(0x518,0x300);
    text(0x510,0xa80,selected->empty?"":"m");
    if(selected->nul_prefix) *static_cast<char*>(at(0xa80))=0; // Recorded length remains one.
    text(0x400,0xa00,selected->owner?" M\\FILE ":"file");
    put(arena,0x100,0xd683f4);put(arena,0x104,0xcccccc7f);put(arena,0x110,0x3333);
    if(selected->alias) {
        put(arena,0x108,selected->result?3u:2u);ptr(0x10c,0x104);
    } else if(!selected->owner) {
        bsp::construct_native_string_cstring_0041e870(at(0x108),"old",lookup->physical.strings);
    }
}
std::uint64_t hash(const void* data,std::size_t n) {
    std::uint64_t h=1469598103934665603ull;
    for(std::size_t i=0;i<n;++i) { h^=static_cast<const unsigned char*>(data)[i];h*=1099511628211ull; }
    return h;
}
std::array<unsigned char,0x1000> normalized_arena() {
    std::array<unsigned char,0x1000> out{};std::memcpy(out.data(),arena,out.size());
    // Only the fixture entry pointer varies with host linking; raw owners,
    // string data pointers and all other actual arena bytes remain compared.
    std::memset(out.data()+0x824,0,4);return out;
}
std::string hex_name(const void* header) {
    static constexpr char digits[]="0123456789abcdef";
    std::string out;const auto n=word(header);if(n>40) fail("unexpected name length");
    const auto* p=reinterpret_cast<const unsigned char*>(word(header,4));
    for(U i=0;i<n;++i) { out+=digits[p[i]>>4];out+=digits[p[i]&15]; }return out;
}
struct Outcome { U device,raw; std::string name,seen; unsigned visits; };
Outcome invoke(bool source) {
    Outcome result{};
    if(selected->owner) {
        result.device=source?static_cast<U>(bsp::select_native_vfs_device_00bdd850(arena,at(0x400),at(0x400),*device)):
            call2(original(0xbdd850),arena,at(0x400),at(0x400));
        result.raw=result.device!=0xffffffffu;
    } else {
        if(source) bsp::read_native_vfs_device_provider_00bdbc70(at(0x100),at(0x510),at(0x400),*device);
        else call2(original(0xbdbc70),at(0x100),at(0x510),at(0x400));
        result.raw=source?bsp::read_native_vfs_device_result_00bdb670(at(0x100)):
            call0(original(0xbdb670),at(0x100))&255u;
        result.device=word(arena,0x110);result.name=hex_name(at(0x108));
        if(!selected->alias) {
            if(source) bsp::destroy_native_vfs_device_visitor_00bdb680(at(0x100),lookup->physical.strings);
            else call0(original(0xbdb680),at(0x100));
            if(word(arena,0x100)!=0xd68380) fail("destructor base reset");
        }
    }
    result.visits=visits;result.seen=seen_name;return result;
}
void emit(std::ofstream& file,const Outcome& result,std::uint64_t ah,std::uint64_t ph) {
    file<<"{\"case\":"<<case_id<<",\"device\":"<<result.device<<",\"raw\":"<<result.raw
        <<",\"name_hex\":\""<<result.name<<"\",\"seen\":\""<<result.seen<<"\",\"visits\":"<<result.visits
        <<",\"arena_hash\":\""<<ah<<"\",\"pool_hash\":\""<<ph<<"\"}\n";file.flush();
}
}
int main(int argc,char** argv) { try {
    if(argc!=2) fail("output directory argument required");
    const std::string out=argv[1];
    std::ofstream inputs(out+"/original_inputs.jsonl"),native(out+"/original_outcomes.jsonl"),source(out+"/source_outcomes.jsonl");
    if(!inputs||!native||!source) fail("output files unavailable");
    install();void* allocation=fixed(0x32000000,sizeof(bsp::NativeStringPoolStorage),PAGE_READWRITE);
    pool=bsp::construct_native_string_pool_00bd1480(allocation);
    bsp::SingletonLifetimeDomain domain(crt);lifetime=&domain;
    bsp::ActualNativeStringPoolStorage strings(pool,return_gate,domain);
    bsp::NativePhysicalFileDateContext physical{publication,strings,crt};
    bsp::NativeVfsLookupRouteContext lookup_context{physical,nullptr,nullptr};lookup=&lookup_context;
    Resolver resolver;bsp::NativeVfsDeviceRouteContext device_context{lookup_context,reinterpret_cast<void*>(0xd683f4),resolver};
    device=&device_context;lookup_context.device=device;
    const auto prefix=offsetof(bsp::NativeStringPoolStorage,critical_section_8ad484);
    std::vector<unsigned char> baseline(prefix),native_pool(prefix);std::memcpy(baseline.data(),pool,prefix);
    const U expected_device[]={0x2222,0x2222,0x3333,0x2222,0x3333,0x3333,0x2222,0xffffffffu};
    const U expected_raw[]={0x80,0x80,0,0x6d,0,0,1,0};
    const char* expected_name[]={"62","6d2f62","6d2f","6d2f","002f62","6f6c64","",""};
    for(case_id=0;case_id<sizeof(cases)/sizeof(cases[0]);++case_id) {
        selected=&cases[case_id];std::memcpy(pool,baseline.data(),prefix);setup();
        const auto input_arena=normalized_arena();const auto input_pool=hash(pool,prefix);
        inputs<<"{\"case\":"<<case_id<<",\"arena_hash\":\""<<hash(input_arena.data(),input_arena.size())
            <<"\",\"pool_hash\":\""<<input_pool<<"\"}\n";inputs.flush(); // Before native execution.
        const auto n=invoke(false);const auto native_arena=normalized_arena();std::memcpy(native_pool.data(),pool,prefix);
        emit(native,n,hash(native_arena.data(),native_arena.size()),hash(pool,prefix)); // Before source execution.
        if(n.device!=expected_device[case_id]||n.raw!=expected_raw[case_id]||n.name!=expected_name[case_id]||n.visits!=1||n.seen!="file")
            fail("original outcome disagrees with frozen expected behavior");
        std::memcpy(pool,baseline.data(),prefix);setup();
        if(input_arena!=normalized_arena()||input_pool!=hash(pool,prefix)) fail("source input differs from original input");
        const auto a=invoke(true);const auto actual_arena=normalized_arena();
        emit(source,a,hash(actual_arena.data(),actual_arena.size()),hash(pool,prefix));
        if(n.device!=a.device||n.raw!=a.raw||n.name!=a.name||n.seen!=a.seen||n.visits!=a.visits||
            native_arena!=actual_arena||std::memcmp(native_pool.data(),pool,prefix)!=0)
            fail("native/source output, arena or actual pool mismatch");
    }
    bsp::destroy_native_string_pool_00bd14c0(*pool,pool,return_gate);VirtualFree(allocation,0,MEM_RELEASE);
    std::puts("PASS 8 focused device variants; original inputs and outcomes fixed before source; complete normalized arena and actual pool prefix agree");
    return 0;
} catch(const std::exception& error) { std::printf("FAIL case=%u %s\n",case_id,error.what());return 1; } }
