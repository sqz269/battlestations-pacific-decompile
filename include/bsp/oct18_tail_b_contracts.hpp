#pragma once
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_session_message_base.hpp"
#include <cstdint>

// Actual Win32 storage, explicit required application/library bindings. These
// are new C++ interfaces, not replacements for the original ECX/stack/FH3 ABI.
// Evidence, call contracts, x87 bytes and unwind maps: OCT18_TB_CONTRACTS.md.
namespace bsp::oct18_tail_b_contracts {
struct X87Value { unsigned char bytes[10]; };
static_assert(sizeof(X87Value)==10);
// Capture the REAL original __thiscall(console,channel_ipv4)/RET4 ST0 result without
// rounding to C++ double (MSVC long double is also double). Consumes ST0 only.
void capture_elapsed_00a3bf50(void* entry,void* console,void* channel_ipv4,X87Value&);
bool ordered_x87_greater(const X87Value&,float);
float spill_x87_binary32(const X87Value&);
struct ClockPair {std::int64_t counter,frequency;};
struct PacketTick {std::uint32_t tick,index;};
// Caller CW/PC is retained (main thread PC24). Only FISTP temporarily uses RC
// truncate; native has unsigned DIV before restoring CW. Width must be nonzero.
PacketTick packet_statistics_tick(const ClockPair&,void* actual_statistics);
struct NetworkBindings {
    virtual ~NetworkBindings()=default;
    virtual void elapsed_00a3bf50(void* console,void* channel_ipv4,X87Value&)=0;
    virtual void kick_00784b10(void* network,void* peer,std::uint32_t zero)=0;
    virtual void warn_004ca120(void* game)=0;
    virtual void disconnect_slot3c(void* transport)=0;
    virtual const ClockPair* clock_slot20(void* clock,ClockPair& out)=0;
    // Fresh factory allocation is NOT released by 787850: retain that behavior.
    virtual void* create_message_00768530(NativeSessionReadStream&)=0;
    virtual void free_00bf65ac(void*) noexcept=0;
    [[noreturn]] virtual void invalid_iterator_00bf6713()=0;
};
struct NetworkGlobals {
    void* volatile& console_00f8abdc;
    void* volatile& game_00e188a8;
    void* volatile& clock_01090ab0;
    const volatile float& maximum_00cf4888;
    const volatile float& warning_00ce3814;
    const void* stream_profile_00d0385c;
};
void kick_timed_out_peers_00784c40(void*,float,NetworkGlobals&,NetworkBindings&);
void check_server_silence_007850e0(void*,float,NetworkGlobals&,NetworkBindings&);
// Original RET10: data,length,unused DWORD,direction LOW BYTE (four arguments).
void record_packet_statistics_00787850(void*,const std::uint8_t*,std::uint32_t,
    std::uint32_t,std::uint8_t,NetworkGlobals&,NetworkBindings&);

struct EffectBindings {
    virtual ~EffectBindings()=default;
    virtual void refresh_owner_00414db0(void*)=0;
    virtual void transform_point_004142e0(const void* point,float* out,const void* matrix)=0;
    virtual void retain(void* actual_ref)=0; // InterlockedIncrement(ref+4)
    virtual void release(void* actual_ref) noexcept=0; // Dec+4; virtual+0 at zero
    // Consumes the retained definition on EVERY path, including C++ exceptions;
    // ECX out, EDX manager; stack definition,XYZ,option0,tail0; RET10/EAX out.
    virtual void** create_effect_008685e0(void** out,void* manager,void* consumed,
        const float* position,std::uint8_t option,std::uint32_t tail)=0;
    virtual void owner_slot1ac(void*,float)=0;
    virtual void record_slot10(void*)=0;
    virtual void report_00982c50(void* manager,void* owner,const NativeString&,float)=0;
};
void fail_ship_section_0081b630(void*,void* volatile& game_00e188a8,
    void* volatile& warnings_00f8a0c4,NativeStringStorage&,EffectBindings&);
struct CamoBindings {
    virtual ~CamoBindings()=default;
    virtual void* load_slot64(void* manager,const void* name,std::uint32_t zero)=0;
    virtual void release(void*) noexcept=0;
    virtual void* visual_00476b90(void* unit)=0;
    // Original fastcall ECX model,EDX first ref,stack second ref/RET4.
    virtual void textures_00b850f0(void* model,void* first,void* second)=0;
    virtual void* allocate_00bf681b(std::uint32_t bytes)=0;
    virtual bool child_kind_slot5c(void* child,std::uint32_t kind)=0;
    // ECX model; EDX NativeString key; stack LIVE camo+1C RGBA, RET4.
    virtual void color_00b85200(void* model,const NativeString&,const void* rgba)=0;
    [[noreturn]] virtual void invalid_iterator_00bf6713()=0;
};
void apply_camo_0081b7c0(void*,std::uint32_t,void* volatile& manager_00f8d394,
    NativeStringStorage&,CamoBindings&);
struct VisitPair {std::uint32_t kind;void* value;};
static_assert(sizeof(VisitPair)==8);
struct DirectorVisitor {
    virtual ~DirectorVisitor()=default;
    virtual void begin_slot04(const VisitPair& name)=0;
    virtual void end_slot08()=0;
    virtual void field_slot10(const VisitPair& name,const VisitPair& value)=0;
    virtual void target_00836240(void* director,void* target,std::uint32_t raw_force)=0;
};
void visit_director_enables_00836510(void*,DirectorVisitor&);

struct BindingOnce {volatile std::uint32_t& flags;volatile std::uint32_t& log;};
struct LuaBindings {
    virtual ~LuaBindings()=default;
    virtual int result_count_00b66400(NativeLuaObjectStorage&)=0;
    virtual int argument_count_00b663f0(NativeLuaObjectStorage&)=0;
    virtual std::uint32_t object_handle_00888aa0(NativeLuaObjectStorage&)=0;
    // ECX matrix, EDX first output, stack second/third outputs; RET8.
    virtual void camera_angles_0042d2e0(void* matrix,float* first,float* second,float* third)=0;
    // The CRT helper consumes ST0, returns EAX (EDX:EAX only on legacy path), preserves the other x87
    // entries. A real original-ABI helper is required, never a C++ double shim.
    virtual void* ftol2_entry_00bf7420()=0;
    virtual void insert_arrows_008caee0(void* set,std::uint32_t handle)=0;
    struct Iterator {void* owner;void* node;};
    virtual Iterator find_arrows_005a16f0(void* set,std::uint32_t handle)=0;
    virtual void erase_arrows_005a5070(void* set,Iterator)=0;
    virtual void report_strings_00976cb0(void* warnings,const void* vector,
        std::uint32_t first,std::uint32_t second)=0;
    // E7 message, actual20h storage; native constructor/destructor bindings.
    virtual void* named_message_0088b7b0(void* fresh,const void* first_string_header)=0;
    virtual void destroy_named_message_007656e0(void*) noexcept=0;
    virtual void send_nonlocal_00770b50(void* session,std::uint32_t peer,void* msg)=0;
    [[noreturn]] virtual void invalid_iterator_00bf6713()=0;
};
struct LuaContext {
    NativeStringStorage& strings;
    int (*do_file_00b69e00)(lua_State*);
    const bool& crt_sse2_conversion;
    void* volatile& game_00e188a8;
    void* volatile& gui_00e198c4;
    void* volatile& warnings_00f8a0c4;
};
int free_camera_get_pos_00892660(lua_State&,BindingOnce,LuaContext&,LuaBindings&);
int force_arrows_008cbcb0(lua_State&,BindingOnce,LuaContext&,LuaBindings&);
int display_message_008d08b0(lua_State&,BindingOnce,LuaContext&,LuaBindings&);

struct LobbyBindings {
    virtual ~LobbyBindings()=default;
    virtual void open_00b6a020(NativeLuaStateStorage&,std::uint32_t mask)=0;
    virtual void run_00b69d40(NativeLuaStateStorage&,const NativeString&,std::uint32_t flag)=0;
    virtual void latency_00a3a890(void* console,std::int32_t index,float maximum,
        float minimum,float drop,float weight)=0;
    // Canonical STL subscripts, not replacements for the game loader. First
    // select outer[slot] then inner[option] using the named original helpers.
    virtual std::uint8_t* enabled_005c6850_005c49c0(void* map,std::int32_t slot,std::int32_t option)=0;
    virtual char** text_00575e80_005732c0(void* map,std::int32_t slot,std::int32_t option)=0;
    virtual std::int32_t* number_00575fd0_00573410(void* map,std::int32_t slot,std::int32_t option)=0;
    virtual char* allocate_00bf55be(std::uint32_t bytes)=0;
    virtual std::int32_t atol_00bf8417(const char*)=0;
    [[noreturn]] virtual void invalid_iterator_00bf6713()=0;
};
struct LobbyGlobals {
    void* volatile& console_00f8abdc;
    void* volatile& application_00e198ac;
    volatile std::uint8_t& loaded_00f88950;
    const volatile float& maximum_00cf4888;
    const volatile float& drop_00d7a2f0;
    const volatile float& capture_00ce3930;
    const bool& crt_sse2_conversion;
};
void load_lobby_tables_008d2f50(void*,NativeStringStorage&,LobbyGlobals&,LobbyBindings&);

struct PingBindings {
    virtual ~PingBindings()=default;
    virtual void* sound_queue_004c1b90()=0;
    virtual void construct_base_0075b430(void* fresh,std::uint32_t type)=0;
    // Numeric native vtable00D03608 or a callable translated table whose five
    // methods implement that same message36. No fabricated dispatch fallback.
    virtual const void* message36_profile_00d03608()=0;
    virtual void send_server_00770af0(void* session,void* msg)=0;
    virtual void receive_008e2da0(void* msg)=0;
    virtual void* ftol2_entry_00bf7420()=0;
    virtual NativeString* concat_004261a0(const NativeString& left,NativeString& fresh,const NativeString& right)=0;
    virtual NativeString* concat_integer_004263b0(const NativeString& left,NativeString& fresh,std::int32_t)=0;
    virtual void send_chat_00772170(void* session,const NativeString& prefix,std::uint8_t channel,
        std::uint32_t kind,const NativeString& text,const NativeString& suffix,
        std::int32_t first,std::int32_t second)=0;
};
void send_map_ping_008e2ee0(std::uint32_t mode,float x,float y,
    volatile float& clock_00f876a4,volatile float& cooldown_00f88c28,
    void* volatile& game_00e188a8,NativeStringStorage&,PingBindings&);

struct ScoreLifetimeBindings {
    virtual ~ScoreLifetimeBindings()=default;
    // Ten canonical allocator entries 582630/B0,582700/50/A0/F0,
    // 582840/90/E0,582930. They initialize the original node layout, and either
    // return storage or throw. Returning null retains the native fault domain.
    virtual void* allocate_head(std::uint32_t original_entry,void* actual_tree)=0;
    // Only library member cleanup; exact entry and actual owner from FH3 map.
    // Must not throw. Called in reverse completed-member order on C++ unwind.
    virtual void destroy_member(std::uint32_t original_entry,void* actual_member) noexcept=0;
};
void* construct_score_record_0091ce90(void* fresh_284_bytes,ScoreLifetimeBindings&);
} // namespace bsp::oct18_tail_b_contracts
