// Packet cc_lua_core. See include/bsp/lua_binding_core.hpp for the evidence per routine.
// Every name is a hypothesis, not a recovered symbol. Nothing here invents a global or stubs
// an unresolved call: every native step is a host method the caller supplies.

#include "bsp/lua_binding_core.hpp"

#include <string>

namespace bsp {
namespace {

// 004260B0 BSP_NativeString_FromInt is the decimal text of the value; 004263B0 appends it to a
// copy of a NativeString. std::to_string is the same conversion for the domain PrepareClass
// reaches, which is whatever 00B66290 truncated an argument to.
std::string decimal(int value)
{
    return std::to_string(value);
}

} // namespace

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

int lua_binding_difficulty_value(int non_campaign_flag, int effective_difficulty) noexcept
{
    // 008AE113 CMP dword [EAX+1FE4h],EBP with EBP zero; 008AE123 is the taken-away branch.
    if (non_campaign_flag != 0) {
        return kNonCampaignReportedDifficulty;
    }
    return effective_difficulty;
}

std::string vehicle_class_race_path(int class_id)
{
    std::string path(kVehicleClassPathPrefix);
    path += decimal(class_id);
    path += kVehicleClassRacePathSuffix;
    return path;
}

int vehicle_class_party_from_race(int race) noexcept
{
    // 008C9282 CMP EAX,1 / 008C9287 CMP EAX,4, both jumping to 008C9290 MOV EDX,1;
    // 008C928C XOR EDX,EDX otherwise.
    if (race == 1 || race == 4) {
        return 1;
    }
    return 0;
}

int enable_messages_flag_slot(int argument_count) noexcept
{
    if (argument_count >= 2) {
        return 1;
    }
    if (argument_count >= 1) {
        return 0;
    }
    return -1;
}

bool message_suppression_byte(bool messages_enabled) noexcept
{
    return !messages_enabled;
}

double lua_binding_degrees_to_radians(double degrees) noexcept
{
    // FMUL double [00CE3D28] then FDIV double [00CE3D20], in that order, so the multiply
    // happens first and the reconstruction keeps the same association.
    return (degrees * 3.14159265358979311600) / 180.0;
}

bool mission_binding_alias_is_indistinguishable(std::uint32_t handler) noexcept
{
    for (std::size_t i = 0; i < kMissionLuaBindingAliasCount; ++i) {
        if (kMissionLuaBindingAliases[i].handler == handler) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// The ten routines
// ---------------------------------------------------------------------------

int lua_binding_setlog() noexcept
{
    // 0088C620..0088C74B. Between 00B679B0 (0088C6F9) and 00B66400 (0088C70A) there is not one
    // instruction that reads an argument or touches game state.
    return 0;
}

int lua_binding_get_difficulty(LuaBindingResultWriter& results, LuaBindingCoreHost& host)
{
    const int flag = host.game_non_campaign_flag();
    int value = kNonCampaignReportedDifficulty;
    if (flag == 0) {
        // 008AE12A is only reached on the zero branch, so the campaign difficulty is not read
        // at all outside a campaign.
        value = host.game_effective_difficulty();
    }
    results.push_number(lua_binding_difficulty_value(flag, value));
    return 1;
}

int lua_binding_prepare_class(LuaBindingArgumentReader& args, LuaBindingCoreHost& host)
{
    // 008C9066 XOR EDI,EDI then 008C92EC re-reads the count every iteration, so a binding that
    // changed the stack would change the bound. None of the calls in the body does.
    for (int index = 0; index < args.count(); ++index) {
        // Two separate reads of the same argument, 008C908E and 008C90CB; the first feeds the
        // log line and the second is the value kept in EBP.
        host.log_prepare_class(args.get_integer(index));
        const int class_id = args.get_integer(index);

        int race = 0;
        if (host.resolve_global_integer(vehicle_class_race_path(class_id), race)) {
            host.vehicle_class_mark_party_required(class_id, vehicle_class_party_from_race(race));
        }

        // 008C929C re-reads the argument a third time for the factory call, and 008C92BA sets
        // DL to 1 unconditionally.
        host.vehicle_class_get_or_create(args.get_integer(index), true);
    }
    return 0;
}

int lua_binding_music_control_set_level(LuaBindingArgumentReader& args, LuaBindingCoreHost& host)
{
    host.music_director_set_level(args.get_integer(0));
    if (host.game_non_campaign_flag() == 1) {
        // 008C4E47 re-reads argument 0 rather than reusing the value it already narrowed.
        host.session_send_music_level(args.get_integer(0));
    }
    return 0;
}

int lua_binding_set_party(LuaBindingArgumentReader& args,
                          LuaBindingResultWriter& results,
                          LuaBindingCoreHost& host)
{
    void* entity = args.entity_at(0);
    const int party = args.get_integer(1);
    if (entity == nullptr) {
        // 008A8A7B MOV EAX,dword ptr [ESI] with no preceding test: the native faults here.
        // Reporting one result keeps the stack discipline the caller expects.
        results.push_number(party);
        return 1;
    }

    if (host.entity_vcall_5c(entity, kEntityPartyQuerySelector)) {
        host.session_route_party_message(entity, party);
    } else {
        host.entity_vcall_2c(entity, party, 0u);
    }

    // 008A8AE9 onwards: the push happens on both paths, after the branch rejoins.
    results.push_number(party);
    return 1;
}

int lua_binding_scoring_real_play_time_running(LuaBindingArgumentReader& args,
                                               LuaBindingResultWriter& results,
                                               LuaBindingCoreHost& host)
{
    const bool running = args.get_boolean(0);
    host.scoring_set_real_play_time_running(running);
    // 008B890A pushes the EAX the setter returned. 00905340 writes AL from its stack argument
    // and never touches EAX again, so the byte pushed is the byte the script passed.
    results.push_boolean(running);
    return 1;
}

int lua_binding_scoring_set_final_scoring_function_name(LuaBindingArgumentReader& args,
                                                        LuaBindingCoreHost& host)
{
    host.scoring_set_final_scoring_function_name(args.get_string(0));
    return 0;
}

int lua_binding_load_message_map(LuaBindingArgumentReader& args, LuaBindingCoreHost& host)
{
    const std::string name = args.get_string(0);
    const int index = args.get_integer(1);
    host.message_map_load(name, index);

    if (host.game_non_campaign_flag() == 1) {
        // 008C6381 and 008C63B4 re-read both arguments for the session branch.
        host.call_0088b6d0_0076a9f0_00765590(args.get_string(0), args.get_integer(1));
    }
    return 0;
}

int lua_binding_set_think(LuaBindingArgumentReader& args, LuaBindingCoreHost& host)
{
    void* entity = args.entity_at(0);
    const std::string name = args.get_string(1);
    if (entity == nullptr) {
        // 008980E6 MOV ECX,ESI with no test, the same missing check as SetParty.
        return 0;
    }
    host.entity_set_think_script_name(entity, name);
    return 0;
}

int lua_binding_enable_messages(LuaBindingArgumentReader& args, LuaBindingCoreHost& host)
{
    const int argument_count = args.count();

    void* entity = nullptr;
    if (argument_count >= 2) {
        entity = args.entity_at(0);
    }

    bool enabled = true; // 008CFF6A MOV BL,1
    const int flag_slot = enable_messages_flag_slot(argument_count);
    if (flag_slot >= 0) {
        enabled = args.get_boolean(flag_slot);
    }

    const bool suppressed = message_suppression_byte(enabled);
    if (entity != nullptr) {
        host.set_entity_message_suppression(entity, suppressed);
    } else {
        host.set_global_message_suppression(suppressed);
        if (!enabled) {
            // 008D0006 JNZ skips the call when the flag was set, so the drain runs only when
            // messages are being turned off.
            host.message_system_drain_queue();
        }
    }
    return 0;
}

} // namespace bsp
