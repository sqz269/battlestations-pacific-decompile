#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "../src/native_dyn_world_step.cpp"
#include "bsp/dyn_world_runtime.hpp"

template<class T> T& field(void* p,unsigned offset){return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset);}
static void require(bool value){if(!value)std::abort();}
static unsigned bits(float value){unsigned word;std::memcpy(&word,&value,4);return word;}
static void* allocate(void*,unsigned){std::abort();}
static void release(void*,void*){std::abort();}
struct Calls final:bsp::NativeDynWorldStepCalls {
    void* world{};void* scene{};const bsp::NativeDynWorldStepContext* owner{};
    std::vector<unsigned> phases,timesteps,sites;
    unsigned serial{};
    void integrate_velocities_00c41550(void* p,float dt)override{
        require(p==world);phases.push_back(1);timesteps.push_back(bits(dt));
    }
    void collision_pass_00c57070(void* p,const bsp::NativeDynCollisionPassContext& c)override{
        require(p==scene && &c==&owner->collision);phases.push_back(2);
    }
    void create_groups_00c4b610(void* p,const bsp::NativeDynContactGroupContext& c)override{
        require(p==static_cast<unsigned char*>(world)+0x448 && &c==&owner->groups);phases.push_back(3);
    }
    void integrate_positions_00c5b1b0(void* p,float dt,const bsp::CameraAxesCrtAccess& c)override{
        require(p==world && &c==&owner->crt && bits(dt)==timesteps.back());phases.push_back(4);
    }
    void sleep_groups_00c4b550(void* p,const bsp::NativeDynContactGroupContext& c)override{
        require(p==static_cast<unsigned char*>(world)+0x448 && &c==&owner->groups);phases.push_back(5);
    }
    std::uint64_t read_timestamp(unsigned site)noexcept override{sites.push_back(site);return 1000ULL+17ULL*serial++;}
};

// Exercise the real simulation caller and check all Win32 callee-saved GPRs
// and ESP across both full-step and remainder dispatches.
__declspec(naked) unsigned __cdecl invoke(void*,float,const bsp::NativeDynWorldStepContext*){
    __asm {
        push ebp
        push ebx
        push esi
        push edi
        mov ebp,esp
        mov ebx,011223344h
        mov esi,055667788h
        mov edi,099aabbcch
        push dword ptr [ebp+1ch]
        push dword ptr [ebp+18h]
        push dword ptr [ebp+14h]
        call bsp::simulate_native_dyn_world_00c5c540
        add esp,0ch
        xor eax,eax
        cmp esp,ebp
        jne done
        cmp ebx,011223344h
        jne done
        cmp esi,055667788h
        jne done
        cmp edi,099aabbcch
        jne done
        mov eax,1
    done:
        mov esp,ebp
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret
    }
}
int main(){
    unsigned short saved,cw=0x007f,after,sw;__asm fnstcw saved
    __asm fldcw cw
    bsp::DynWorldStorage world{};bsp::DynEngineStorage engine{};bsp::DynProfileStorage profile{};
    bsp::DynProfileNodeStorage root{},simulate{},groups{},positions{},sleep{};
    unsigned scene{};void* engine_slot=&engine;void* profile_slot=&profile;
    field<void*>(&engine,0x0c)=&profile;field<void*>(&profile,0)=&root;field<void*>(&profile,4)=&root;
    field<void*>(&profile,0x10)=&simulate;field<void*>(&profile,0x64)=&groups;
    field<void*>(&profile,0x78)=&positions;field<void*>(&profile,0x7c)=&sleep;
    field<float>(&world,0)=0.05f;field<unsigned>(&world,0x34)=4;
    field<void*>(&world,0x204)=world.bytes+0x208;field<void*>(&world,0x444)=&scene;
    field<void*>(&world,0x448)=&world;
    bsp::AvoidZoneDynHullMemory memory{nullptr,&allocate,&release};Calls calls;calls.world=&world;calls.scene=&scene;
    bsp::DynProfileScopeContext profile_context{memory,&profile_slot};
    bsp::NativeDynCollisionPassContext collision{memory,profile_context,&engine_slot,calls};
    bsp::NativeDynContactGroupCalls group_calls;bsp::NativeDynContactGroupContext group_context{memory,group_calls};
    bsp::CameraAxesCrtAccess crt{};bsp::NativeDynWorldStepContext context{collision,group_context,crt,calls};calls.owner=&context;
    require(invoke(&world,0.125f,&context)==1);
    __asm fnstcw after
    __asm fnstsw sw
    require(after==cw&&(sw&0x3800)==0);
    require(calls.timesteps.size()==3&&calls.phases.size()==15);
    for(unsigned i=0;i<calls.phases.size();++i)require(calls.phases[i]==i%5+1);
    require(calls.timesteps[0]==bits(0.05f)&&calls.timesteps[1]==bits(0.05f));
    require(field<unsigned>(&world,0x2c)==1&&field<unsigned>(&world,0x48)==0);
    require(field<void*>(&profile,4)==&root&&field<unsigned>(&simulate,0x40)==1);
    for(auto* node:{&groups,&positions,&sleep})require(field<unsigned>(node,0x40)==3);
    std::printf("PASS full/full/remainder; ESP+EBX+ESI+EDI+EBP preserved; CW=%04x TOP=%u\n",after,(sw>>11)&7);
    for(unsigned dt:calls.timesteps)std::printf("dt=%08x\n",dt);
    for(unsigned site:calls.sites)std::printf("timestamp site=%08x\n",site);
    __asm fldcw saved
    return 0;
}
