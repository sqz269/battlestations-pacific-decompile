#include "bsp/native_global_config_load.hpp"
#include "bsp/native_checked_string_storage.hpp"
#include "bsp/native_lua_color.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <array>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
namespace bsp { namespace {
using Word=std::uint32_t;
struct Field {const char* key;Word receiver,scratch,offset,lookup,read,release,fallback;};
constexpr Field f_UnitCameraSpeedMul{"UnitCameraSpeedMul",0x60,0x38,0x4,0x87d8bd,0x87d8cc,0x87d8e0,0x0};
constexpr Field f_LimboMinTime{"LimboMinTime",0x60,0x38,0x8,0x87d8f3,0x87d902,0x87d916,0x0};
constexpr Field f_ExtraEffects{"ExtraEffects",0x60,0x4c,0x0,0x87d929,0x87d93d,0x0,0x0};
constexpr Field f_Difficulty{"Difficulty",0x60,0x38,0x0,0x87da55,0x87da67,0x87da78,0x0};
constexpr Field f_HPMultipliers{"HPMultipliers",0x4c,0xb0,0x0,0x87da8e,0x87daac,0x0,0x0};
constexpr Field f_ScoreMultipliers{"ScoreMultipliers",0x4c,0x114,0x0,0x87daac,0x87daca,0x0,0x0};
constexpr Field f_LockRadiusMultipliers{"LockRadiusMultipliers",0x4c,0x13c,0x0,0x87daca,0x87dae8,0x0,0x0};
constexpr Field f_PlayerCheatMultipliers{"PlayerCheatMultipliers",0x4c,0x128,0x0,0x87dae8,0x87db08,0x0,0x0};
constexpr Field f_LockRadiusZoomModifier{"LockRadiusZoomModifier",0x4c,0x38,0x5c,0x87ddf7,0x87de10,0x87de23,0x3f4ccccd};
constexpr Field f_Damage{"Damage",0x60,0x38,0x0,0x87de36,0x87de48,0x87de58,0x0};
constexpr Field f_Yellow{"Yellow",0x4c,0x38,0x60,0x87de6b,0x87de7a,0x87de8d,0x0};
constexpr Field f_Red{"Red",0x4c,0x38,0x64,0x87dea0,0x87deaf,0x87dec2,0x0};
constexpr Field f_MalfunctionRepairTreshold{"MalfunctionRepairTreshold",0x4c,0x38,0x68,0x87ded5,0x87dee4,0x87def7,0x0};
constexpr Field f_Minimap{"Minimap",0x60,0x38,0x0,0x87df0a,0x87df1c,0x87df2c,0x0};
constexpr Field f_MinimapRange{"MinimapRange",0x4c,0x38,0x6c,0x87df3f,0x87df4e,0x87df61,0x0};
constexpr Field f_VisibilityRange{"VisibilityRange",0x4c,0x38,0x70,0x87df74,0x87df83,0x87df96,0x0};
constexpr Field f_CameraShake{"CameraShake",0x60,0x38,0x0,0x87dfa9,0x87dfbb,0x87dfcb,0x0};
constexpr Field f_GlobalMultiplier{"GlobalMultiplier",0x4c,0x38,0x74,0x87dfde,0x87dfed,0x87e000,0x0};
constexpr Field f_StrengthMax{"StrengthMax",0x4c,0x38,0x78,0x87e013,0x87e022,0x87e035,0x0};
constexpr Field f_Decay{"Decay",0x4c,0x38,0x7c,0x87e048,0x87e057,0x87e06a,0x0};
constexpr Field f_WarningScrollSpeeds{"WarningScrollSpeeds",0x60,0x38,0x0,0x87e07d,0x87e08f,0x87e09f,0x0};
constexpr Field f_actual{"actual",0x4c,0x38,0x80,0x87e0b2,0x87e0c1,0x87e0d7,0x0};
constexpr Field f_history{"history",0x4c,0x38,0x84,0x87e0ea,0x87e0f9,0x87e10f,0x0};
constexpr Field f_WeaponSystems{"WeaponSystems",0x60,0x38,0x0,0x87e122,0x87e134,0x87e144,0x0};
constexpr Field f_WeaponDirectorThinkTime{"WeaponDirectorThinkTime",0x4c,0x38,0x88,0x87e157,0x87e166,0x87e17c,0x0};
constexpr Field f_SafeToFireCacheTimeOut{"SafeToFireCacheTimeOut",0x4c,0x38,0x8c,0x87e18f,0x87e19e,0x87e1b4,0x0};
constexpr Field f_LineOfSight{"LineOfSight",0x60,0x38,0x0,0x87e1c7,0x87e1d9,0x87e1e9,0x0};
constexpr Field f_VisibleTimeOut{"VisibleTimeOut",0x4c,0x38,0xa0,0x87e1fc,0x87e20b,0x87e221,0x0};
constexpr Field f_InvisibleTimeOut{"InvisibleTimeOut",0x4c,0x38,0xa4,0x87e234,0x87e243,0x87e259,0x0};
constexpr Field f_TargetHeightAdd{"TargetHeightAdd",0x4c,0x38,0x90,0x87e26c,0x87e285,0x87e29b,0x40a00000};
constexpr Field f_ViewerHeightAdd{"ViewerHeightAdd",0x4c,0x38,0x94,0x87e2ae,0x87e2c7,0x87e2dd,0x40a00000};
constexpr Field f_TargetHeightMul{"TargetHeightMul",0x4c,0x38,0x98,0x87e2f0,0x87e305,0x87e31b,0x0};
constexpr Field f_ViewerHeightMul{"ViewerHeightMul",0x4c,0x38,0x9c,0x87e32e,0x87e343,0x87e359,0x0};
constexpr Field f_KillReasonMessages{"KillReasonMessages",0x60,0x38,0x0,0x87e36c,0x87e37e,0x87e38e,0x0};
constexpr Field f_harm{"harm",0x4c,0x38,0xa8,0x87e3a1,0x87e3b0,0x87e47f,0x0};
constexpr Field f_soft{"soft",0x4c,0x20,0xb8,0x87e492,0x87e4a1,0x87e570,0x0};
constexpr Field f_landed{"landed",0x4c,0x38,0xb0,0x87e583,0x87e592,0x87e661,0x0};
constexpr Field f_exited{"exited",0x4c,0x20,0xc0,0x87e674,0x87e683,0x87e752,0x0};
constexpr Field f_captured{"captured",0x4c,0x38,0xc8,0x87e765,0x87e774,0x87e843,0x0};
constexpr Field f_sold{"sold",0x4c,0x20,0xd0,0x87e856,0x87e865,0x87e934,0x0};
constexpr Field f_other{"other",0x4c,0x38,0xd8,0x87e947,0x87e956,0x87ea25,0x0};
constexpr Field f_BlackoutDefaultTime{"BlackoutDefaultTime",0x60,0x20,0xe0,0x87ea38,0x87ea47,0x87ea5d,0x0};
constexpr Field f_TargetMarkers{"TargetMarkers",0x60,0x208,0x0,0x87ea73,0x87ea85,0x87ea98,0x0};
constexpr Field f_MarkTargetsRadius{"MarkTargetsRadius",0x4c,0x230,0xe4,0x87eaae,0x87eabd,0x87eae0,0x0};
constexpr Field f_NormalSizeDist{"NormalSizeDist",0x4c,0x168,0xe8,0x87eaf6,0x87eb05,0x87eb1e,0x0};
constexpr Field f_MinIconZoom{"MinIconZoom",0x4c,0x280,0xec,0x87eb34,0x87eb43,0x87eb5c,0x0};
constexpr Field f_MaxIconZoom{"MaxIconZoom",0x4c,0x2a8,0xf0,0x87eb72,0x87eb81,0x87eb9a,0x0};
constexpr Field f_FOVs{"FOVs",0x60,0x2bc,0x0,0x87ebb0,0x87ebc2,0x87ebd5,0x0};
constexpr Field f_Ship{"Ship",0x4c,0x1a4,0xf8,0x87ebfb,0x87ec0a,0x87ec35,0x0};
constexpr Field f_Plane{"Plane",0x4c,0x244,0xfc,0x87ec4b,0x87ec5a,0x87ec85,0x0};
constexpr Field f_UnitSellTime{"UnitSellTime",0x60,0x1cc,0x100,0x87ec9b,0x87ecaa,0x87ecc3,0x0};
constexpr Field f_UnitSellTimeSingle{"UnitSellTimeSingle",0x60,0x294,0x104,0x87ecd9,0x87ece8,0x87ed01,0x0};
constexpr Field f_MapUnitColors{"MapUnitColors",0x60,0x1f4,0x0,0x87ed17,0x87ed29,0x87ed3c,0x0};
constexpr Field f_us{"us",0x4c,0x26c,0x108,0x87ed52,0x87ed66,0x87edf0,0x0};
constexpr Field f_usAI{"usAI",0x4c,0x21c,0x118,0x87ee06,0x87ee1a,0x87eea4,0x0};
constexpr Field f_jap{"jap",0x4c,0x17c,0x128,0x87eeba,0x87eece,0x87ef58,0x0};
constexpr Field f_japAI{"japAI",0x4c,0x190,0x138,0x87ef6e,0x87ef82,0x87f00c,0x0};
constexpr Field f_neutral{"neutral",0x4c,0x1b8,0x148,0x87f022,0x87f036,0x87f0c0,0x0};
constexpr Field f_selected{"selected",0x4c,0x1e0,0x158,0x87f0d6,0x87f0ea,0x87f174,0x0};
constexpr Field f_target{"target",0x4c,0xc4,0x168,0x87f18a,0x87f19e,0x87f228,0x0};
constexpr Field f_usTarget{"usTarget",0x4c,0xd8,0x178,0x87f23e,0x87f252,0x87f2dc,0x0};
constexpr Field f_japTarget{"japTarget",0x4c,0x100,0x188,0x87f2f2,0x87f306,0x87f390,0x0};
constexpr Field f_dead{"dead",0x4c,0xec,0x198,0x87f3a6,0x87f3ba,0x87f444,0x0};
constexpr Field f_MapMarkerColors{"MapMarkerColors",0x60,0x74,0x0,0x87f457,0x87f469,0x87f479,0x0};
constexpr Field f_ChatColors{"ChatColors",0x60,0x74,0x0,0x87f540,0x87f552,0x87f562,0x0};
constexpr Field f_ObjectiveSounds{"ObjectiveSounds",0x60,0x74,0x0,0x87f62a,0x87f63c,0x87f64c,0x0};
constexpr Field f_MapZoomTime{"MapZoomTime",0x60,0x74,0x2b8,0x87f769,0x87f778,0x87f78e,0x0};
constexpr Field f_MapGlobalScale{"MapGlobalScale",0x60,0xec,0x2bc,0x87f7a4,0x87f7b3,0x87f7cc,0x0};
constexpr Field f_SpawnAttemptDelay{"SpawnAttemptDelay",0x60,0x100,0x2dc,0x87f7e2,0x87f7fb,0x87f814,0x3f4ccccd};
constexpr Field f_SpawnPlaneDelayRange{"SpawnPlaneDelayRange",0x60,0xd8,0x2e0,0x87f82a,0x87f843,0x87f85c,0x43960000};
constexpr Field f_SpawnPlaneDelayDistance{"SpawnPlaneDelayDistance",0x60,0xc4,0x2e4,0x87f872,0x87f88b,0x87f8a4,0x43960000};

template<class T> T read(const void* p) noexcept {T v;std::memcpy(&v,p,sizeof(v));return v;}
template<class T> void write(void* p,T v) noexcept {std::memcpy(p,&v,sizeof(v));}
template<class T,class U> T bits(U value) noexcept {static_assert(sizeof(T)==sizeof(U));T result;std::memcpy(&result,&value,sizeof(result));return result;}
std::int32_t increment(std::int32_t n) noexcept {return bits<std::int32_t>(static_cast<Word>(n)+1u);}
float reciprocal(float number) noexcept {float result;__asm {fld number
 fld1
 fdivrp st(1),st(0)
 fstp result} return result;}
float square(float number) noexcept {float result;__asm {fld number
 fmul st(0),st(0)
 fstp result} return result;}
float fov(float number,const volatile float& divisor) noexcept {
    const double pi=bits<double>(0x400921fb60000000ull); //CE3D28, float pi widened to double
    const double degrees=180.0; //CE3D20
    const volatile float* cell=&divisor;float result;
    __asm {fld number
 fmul pi
 fdiv degrees
 mov eax,cell
 fdiv dword ptr[eax]
 fstp result}
    return result;
}
void unpack_color(void* destination,const std::uint8_t* packed) noexcept {
    constexpr unsigned order[]{2,1,0,3};const double denominator=255.0; //CE4B48
    for(unsigned i=0;i<4;++i){const std::int32_t channel=packed[order[i]];float result;
        __asm {fild channel
 fdiv denominator
 fstp result}
        write(static_cast<std::byte*>(destination)+i*4,result);
    }
}
} // namespace
struct NativeGlobalConfigLoadOperation::Impl {
    alignas(8) std::array<std::byte,0x798> storage; // native relative locals + actual Lua4C8 at2D0
    std::array<bool,0x2d0/4> references{};
    NativeGlobalConfigLoadContext* context{};GlobalConfigOwner* owner{};
    Phase phase{Phase::fresh};Word site{};bool lua_live{},temporary_live{};
    void* frame(Word n) noexcept {return storage.data()+n;}
    void* field(Word n) noexcept {return owner->native.data()+n;}
    NativeLuaStateStorage& lua() noexcept {return *static_cast<NativeLuaStateStorage*>(frame(0x2d0));}
    NativeLuaObjectStorage& object(Word n) noexcept {return *static_cast<NativeLuaObjectStorage*>(frame(n));}
    NativeString& temporary() noexcept {return *static_cast<NativeString*>(frame(0x14));}
    void release(Word offset,Word call_site){site=call_site;references[offset/4]=false;destroy_native_lua_object_00b67700(object(offset));}
    void lookup(const Field& f){site=f.lookup;native_lua_get_by_name_00b67800(object(f.receiver),frame(f.scratch),f.key);references[f.scratch/4]=true;}
    void select(const Field& f){lookup(f);site=f.read;assign_native_lua_object_00b67690(object(0x4c),object(f.scratch));release(f.scratch,f.release);}
    void number(const Field& f,bool fallback=false,unsigned transform=0){
        lookup(f);site=f.read;
        float result=fallback?native_lua_number_or_00b66330(object(f.scratch),bits<float>(f.fallback)):native_lua_number_00b66270(object(f.scratch));
        if(transform==1)result=square(result);
        else if(transform==2)result=fov(result,context->fov_divisor_00f889b4);
        write(field(f.offset),result);release(f.scratch,f.release);
    }
    void string(const Field& f){lookup(f);site=f.read;const char* text=native_lua_string_00b662b0(object(f.scratch));assign_native_string_cstring_0041e350(field(f.offset),text,context->strings);release(f.scratch,f.release);}
    void color(const Field& f){lookup(f);site=f.read;auto* packed=static_cast<std::uint8_t*>(frame(0x1c));read_native_lua_color_00b67f10(object(f.scratch),packed);unpack_color(field(f.offset),packed);release(f.scratch,f.release);}
    void make_temporary(const char* text,Word resize_site){
        ::new(frame(0x14)) NativeString;temporary_live=true;
        const auto length=static_cast<Word>(std::strlen(text));site=resize_site;
        resize_native_string_header_0041dd40(frame(0x14),context->strings,length,true);
        char* const data=temporary().data();if(data)std::memcpy(data,text,temporary().length()+1u);
    }
    void release_temporary(char* data,Word length){temporary_live=false;if(data)context->strings.release(data,length+1u);}
    void release_temporary(){auto* data=temporary().data();const auto length=temporary().length();release_temporary(data,length);}
    bool difficulty_present(std::int32_t index,Word lookup_site){
        site=lookup_site;native_lua_get_by_index_00b67720(object(0xb0),frame(0x38),index);references[0x38/4]=true;
        site=lookup_site==0x0087db08?0x0087db17:0x0087ddc2;const bool present=!native_lua_is_nil_00b65fb0(object(0x38));release(0x38,lookup_site==0x0087db08?0x0087db2e:0x0087ddd9);return present;
    }
    void append_difficulty(std::int32_t index,Word table,Word output,bool inverse,Word lookup_site,Word number_site,Word insert_site,Word release_site){
        site=lookup_site;native_lua_get_by_index_00b67720(object(table),frame(0x38),index);references[0x38/4]=true;
        site=number_site;float number=native_lua_number_00b66270(object(0x38));if(inverse)number=reciprocal(number);
        const Word number_bits=bits<Word>(number);auto* header=static_cast<std::byte*>(field(output));
        const Word first=read<Word>(header+4),end=read<Word>(header+8),capacity=read<Word>(header+12);
        const auto count=[](Word a,Word b){return static_cast<Word>(static_cast<std::int32_t>(b-a)>>2);};
        if(first&&count(first,capacity)>count(first,end)){write(reinterpret_cast<void*>(end),number_bits);write(header+8,end+4u);}
        else {site=insert_site;append_global_config_multiplier_storage(header,number_bits);}
        release(0x38,release_site);
    }
    void color_array(unsigned count,Word output,Word lookup_site,Word color_site,Word release_site){
        for(unsigned index=0;index<count;++index){site=lookup_site;native_lua_get_by_index_00b67720(object(0x4c),frame(0x74),static_cast<std::int32_t>(index));references[0x74/4]=true;
            site=color_site;auto* packed=static_cast<std::uint8_t*>(frame(0x1c));read_native_lua_color_00b67f10(object(0x74),packed);unpack_color(field(output+index*0x10u),packed);release(0x74,release_site);
        }
    }
};
NativeGlobalConfigLoadOperation::NativeGlobalConfigLoadOperation():impl_(std::make_unique<Impl>()){}
NativeGlobalConfigLoadOperation::~NativeGlobalConfigLoadOperation(){if(retains_native_state())std::terminate();}
NativeGlobalConfigLoadOperation::Phase NativeGlobalConfigLoadOperation::phase() const noexcept{return impl_->phase;}
bool NativeGlobalConfigLoadOperation::retains_native_state() const noexcept {
    if(impl_->lua_live||impl_->temporary_live)return true;for(bool live:impl_->references)if(live)return true;return false;
}
std::uint32_t NativeGlobalConfigLoadOperation::active_call_site() const noexcept{return impl_->site;}
NativeLuaStateStorage* NativeGlobalConfigLoadOperation::retained_lua_state() noexcept{return impl_->lua_live?&impl_->lua():nullptr;}
void NativeGlobalConfigLoadOperation::discard_retained_state_for_diagnostics(){
    auto& s=*impl_;if(s.phase!=Phase::failed&&s.phase!=Phase::running)throw std::logic_error("global config diagnostic cleanup requires an interrupted operation");
    if(s.temporary_live)s.release_temporary();
    for(Word offset=0x2d0;offset!=0;){offset-=4;if(s.references[offset/4])s.release(offset,s.site);}
    if(s.lua_live){s.lua_live=false;close_native_lua_state_00b669a0(s.lua());}
    s.phase=Phase::diagnostic_retired;
}
void load_native_global_config_0087d7b0(GlobalConfigOwner& owner,NativeGlobalConfigLoadContext& context,NativeGlobalConfigLoadOperation& operation){
    auto& s=*operation.impl_;
    if(s.phase!=NativeGlobalConfigLoadOperation::Phase::fresh)throw std::logic_error("global config load operation is one-shot");
    if(context.bootstrap.do_file_00b69e00!=context.files.do_file_00b69e00||&context.sound.cache.strings!=&context.strings)throw std::invalid_argument("global config loader requires shared actual Lua/string domains");
    s.owner=&owner;s.context=&context;s.phase=NativeGlobalConfigLoadOperation::Phase::running;
    try {
        s.site=0x0087d7de;construct_native_lua_state_00b66bd0(s.frame(0x2d0));s.lua_live=true;
        s.site=0x0087d7f5;open_native_lua_state_00b6a020(s.lua(),1,context.strings,context.bootstrap);
        s.make_temporary("Scripts/datatables/Globals.lua",0x0087d80a);s.site=0x0087d842;run_native_lua_file_00b69d40(s.lua(),s.temporary(),0,context.strings,context.files);s.release_temporary();
        s.site=0x0087d87d;native_lua_globals_00b67980(s.lua(),s.frame(0x258));s.references[0x258/4]=true;
        s.site=0x0087d896;native_lua_get_by_name_00b67800(s.object(0x258),s.frame(0x60),"Globals");s.references[0x60/4]=true;s.release(0x258,0x0087d8aa);
        s.number(f_UnitCameraSpeedMul);s.number(f_LimboMinTime);s.lookup(f_ExtraEffects);
        s.site=0x0087d93d;construct_native_lua_object_00b65f50(s.frame(0x88));s.references[0x88/4]=true;
        s.site=0x0087d951;construct_native_lua_object_00b65f50(s.frame(0x9c));s.references[0x9c/4]=true;
        s.site=0x0087d972;native_lua_iterate_first_00b67080(s.object(0x4c),s.object(0x88),s.object(0x9c));
        while(!native_lua_is_unbound_00b66420(s.object(0x88))){
            s.site=0x0087d997;const char* text=native_lua_string_00b662b0(s.object(0x9c));s.make_temporary(text,0x0087d9c2);
            auto* const captured=s.temporary().data();const auto length=s.temporary().length();s.site=0x0087d9f1;append_checked_native_string_storage(s.field(0xc),s.frame(0x14),context.strings);s.release_temporary(captured,length);
            s.site=0x0087da29;native_lua_iterate_next_00b67190(s.object(0x4c),s.object(0x88),s.object(0x9c));
        }
        s.select(f_Difficulty);s.lookup(f_HPMultipliers);s.lookup(f_ScoreMultipliers);s.lookup(f_LockRadiusMultipliers);s.lookup(f_PlayerCheatMultipliers);
        std::int32_t index=1;Word probe_site=0x0087db08;
        while(s.difficulty_present(index,probe_site)){
            s.append_difficulty(index,0xb0,0x1c,true,0x0087db4d,0x0087db5c,0x0087dbc9,0x0087dbd9);
            s.append_difficulty(index,0x114,0x2c,false,0x0087dbeb,0x0087dbfa,0x0087dc63,0x0087dc73);
            s.append_difficulty(index,0x13c,0x3c,false,0x0087dc85,0x0087dc94,0x0087dcfd,0x0087dd0d);
            s.append_difficulty(index,0x128,0x4c,true,0x0087dd1f,0x0087dd2e,0x0087dd8e,0x0087dd9e);
            index=increment(index);probe_site=0x0087ddb3;
        }
        s.number(f_LockRadiusZoomModifier,true);

        s.select(f_Damage);
        s.number(f_Yellow,false,0);
        s.number(f_Red,false,0);
        s.number(f_MalfunctionRepairTreshold,false,0);
        s.select(f_Minimap);
        s.number(f_MinimapRange,false,0);
        s.number(f_VisibilityRange,false,0);
        s.select(f_CameraShake);
        s.number(f_GlobalMultiplier,false,0);
        s.number(f_StrengthMax,false,0);
        s.number(f_Decay,false,0);
        s.select(f_WarningScrollSpeeds);
        s.number(f_actual,false,0);
        s.number(f_history,false,0);
        s.select(f_WeaponSystems);
        s.number(f_WeaponDirectorThinkTime,false,0);
        s.number(f_SafeToFireCacheTimeOut,false,0);
        s.select(f_LineOfSight);
        s.number(f_VisibleTimeOut,false,0);
        s.number(f_InvisibleTimeOut,false,0);
        s.number(f_TargetHeightAdd,true,0);
        s.number(f_ViewerHeightAdd,true,0);
        s.number(f_TargetHeightMul,true,0);
        s.number(f_ViewerHeightMul,true,0);
        s.select(f_KillReasonMessages);
        s.string(f_harm);
        s.string(f_soft);
        s.string(f_landed);
        s.string(f_exited);
        s.string(f_captured);
        s.string(f_sold);
        s.string(f_other);
        s.number(f_BlackoutDefaultTime,false,0);
        s.select(f_TargetMarkers);
        s.number(f_MarkTargetsRadius,false,1);
        s.number(f_NormalSizeDist,false,0);
        s.number(f_MinIconZoom,false,0);
        s.number(f_MaxIconZoom,false,0);
        s.select(f_FOVs);
        write(s.field(0xf4),Word{0x3f800000}); //D7A24C
        s.number(f_Ship,false,2);
        s.number(f_Plane,false,2);
        s.number(f_UnitSellTime,false,0);
        s.number(f_UnitSellTimeSingle,false,0);
        s.select(f_MapUnitColors);
        s.color(f_us);
        s.color(f_usAI);
        s.color(f_jap);
        s.color(f_japAI);
        s.color(f_neutral);
        s.color(f_selected);
        s.color(f_target);
        s.color(f_usTarget);
        s.color(f_japTarget);
        s.color(f_dead);

        s.select(f_MapMarkerColors);s.color_array(6,0x1a8,0x0087f490,0x0087f4a4,0x0087f51e);
        s.select(f_ChatColors);s.color_array(11,0x208,0x0087f57a,0x0087f58e,0x0087f608);
        s.select(f_ObjectiveSounds);s.site=0x0087f66b;native_lua_iterate_first_00b67080(s.object(0x4c),s.object(0x88),s.object(0x9c));index=0;
        while(!native_lua_is_unbound_00b66420(s.object(0x88))){
            s.site=0x0087f697;const char* text=native_lua_string_00b662b0(s.object(0x9c));s.make_temporary(text,0x0087f6c2);
            s.site=0x0087f6f9;assign_global_config_sound_008dbe90(s.field(0x2c0),index,s.temporary(),context.sound);s.release_temporary();
            s.site=0x0087f738;native_lua_iterate_next_00b67190(s.object(0x4c),s.object(0x88),s.object(0x9c));index=increment(index);
        }
        s.number(f_MapZoomTime);s.number(f_MapGlobalScale);s.number(f_SpawnAttemptDelay,true);s.number(f_SpawnPlaneDelayRange,true);s.number(f_SpawnPlaneDelayDistance,true);
        s.release(0x128,0x0087f8b8);s.release(0x13c,0x0087f8cc);s.release(0x114,0x0087f8e0);s.release(0xb0,0x0087f8f4);
        s.release(0x9c,0x0087f908);s.release(0x88,0x0087f91c);s.release(0x4c,0x0087f92d);s.release(0x60,0x0087f93e);
        s.site=0x0087f955;s.lua_live=false;close_native_lua_state_00b669a0(s.lua());s.phase=NativeGlobalConfigLoadOperation::Phase::complete;
    }catch(...){s.phase=NativeGlobalConfigLoadOperation::Phase::failed;throw;}
}
} // namespace bsp

