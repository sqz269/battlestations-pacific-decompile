// The tail of cSkeletonAppMidway::Init 0073d410. See docs/APP_INIT_TAIL.md.
// Addresses: 0073c3b0, 00736b60, 00736c30, 00be0660, 00736dd0, 00736ea0,
//            00b80a50, 00af06a0, 0073fa70, 0073e9a0, 00740410, 00740840.

#include "bsp/app_init_tail.hpp"
#include "bsp/lua_object.hpp"

#include <cctype>

namespace bsp {
namespace {

// 00449af0 BSP_NativeString_NotEqualCaseInsensitive, reached through the two
// native string temporaries built at 0073c59d and 0073c5bf.
bool equal_case_insensitive(const std::string& left, const std::string& right) noexcept
{
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t i = 0; i < left.size(); ++i) {
        const unsigned char a = static_cast<unsigned char>(left[i]);
        const unsigned char b = static_cast<unsigned char>(right[i]);
        if (std::tolower(a) != std::tolower(b)) {
            return false;
        }
    }
    return true;
}

std::string decimal(std::uint32_t value)
{
    return std::to_string(value);
}

} // namespace

// ===========================================================================
// A. 0073c3b0
// ===========================================================================

std::vector<HardwareProbeChange> hardware_probe_changes_0073c3b0(
    const HardwareProbeInputs& inputs)
{
    std::vector<HardwareProbeChange> changes;

    // 0073c49a..0073c4ab. The comparison is on the device ids; the names only
    // reach the message.
    if (inputs.current.gpu_device_id != inputs.stored.gpu_device_id) {
        HardwareProbeChange change;
        change.check = HardwareCheck::GpuDevice;
        change.stored_text = inputs.stored_gpu_name;
        change.current_text = inputs.current_gpu_name;
        change.stored_value = inputs.stored.gpu_device_id;
        change.current_value = inputs.current.gpu_device_id;
        changes.push_back(change);
    }

    // 0073c594..0073c61d.
    if (!equal_case_insensitive(inputs.stored.sound_device, inputs.current.sound_device)) {
        HardwareProbeChange change;
        change.check = HardwareCheck::SoundDevice;
        change.stored_text = inputs.stored.sound_device;
        change.current_text = inputs.current.sound_device;
        changes.push_back(change);
    }

    // 0073c6f7..0073c700: the two integers go straight into the message.
    if (inputs.current.cpu_speed != inputs.stored.cpu_speed) {
        HardwareProbeChange change;
        change.check = HardwareCheck::CpuSpeed;
        change.stored_value = inputs.stored.cpu_speed;
        change.current_value = inputs.current.cpu_speed;
        change.numeric = true;
        changes.push_back(change);
    }

    // 0073c75e..0073c767.
    if (inputs.current.mem_size != inputs.stored.mem_size) {
        HardwareProbeChange change;
        change.check = HardwareCheck::MemSize;
        change.stored_value = inputs.stored.mem_size;
        change.current_value = inputs.current.mem_size;
        change.numeric = true;
        changes.push_back(change);
    }

    return changes;
}

std::string hardware_probe_message_0073c3b0(
    const std::vector<HardwareProbeChange>& changes,
    const std::vector<std::string>& fragment_text,
    const std::string& trailer_text)
{
    // The four buffers are zeroed before the checks run (0073c46c, 0073c576,
    // 0073c6d2, 0073c739) and are concatenated whether or not their check
    // fired, so an absent fragment contributes nothing and no separator.
    std::string message;
    for (std::size_t i = 0; i < changes.size() && i < fragment_text.size(); ++i) {
        message += fragment_text[i];
    }
    message += '\n';
    message += trailer_text;
    return message;
}

HardwareProbeResult run_hardware_probe_tail_0073c3b0(const HardwareProfile& stored,
    HardwareProbeTailHost& host)
{
    HardwareProbeInputs inputs;
    inputs.stored = stored;
    inputs.current.gpu_device_id = host.current_gpu_device_id();
    inputs.current.sound_device = host.current_sound_device();
    inputs.current.cpu_speed = host.current_cpu_speed();
    inputs.current.mem_size = host.current_memory_size();
    inputs.stored_gpu_name = host.gpu_device_name(stored.gpu_device_id);
    inputs.current_gpu_name = host.gpu_device_name(inputs.current.gpu_device_id);

    const std::vector<HardwareProbeChange> changes =
        hardware_probe_changes_0073c3b0(inputs);
    if (changes.empty()) {
        // 0073c7a4 falls straight to the object teardown: no message box and
        // no write-back, so the stored profile keeps its old values.
        return HardwareProbeResult::requirements_met;
    }

    std::vector<std::string> fragment_text;
    fragment_text.reserve(changes.size());
    for (const HardwareProbeChange& change : changes) {
        const std::string first = change.numeric
            ? decimal(change.stored_value) : change.stored_text;
        const std::string second = change.numeric
            ? decimal(change.current_value) : change.current_text;
        fragment_text.push_back(host.localized_message(change.message_index(),
            first, second));
    }
    const std::string trailer =
        host.localized_message(kHardwareProbeTrailerMessage, std::string(), std::string());
    const std::string caption =
        host.localized_message(kHardwareProbeCaptionMessage, std::string(), std::string());

    const bool write_defaults = host.ask_write_default_options(
        hardware_probe_message_0073c3b0(changes, fragment_text, trailer), caption);
    if (write_defaults) {
        host.write_default_options_file(kDefaultOptionsFileName);
    }

    // 0073c843..0073c89e, on both answers and in this order.
    host.write_hardware_dword("MemSize", inputs.current.mem_size);
    host.write_hardware_dword("CPUSpeed", inputs.current.cpu_speed);
    host.write_hardware_dword("GPUDeviceID", inputs.current.gpu_device_id);
    host.write_hardware_string("SoundDevice", inputs.current.sound_device);

    return write_defaults ? HardwareProbeResult::defaults_written
                          : HardwareProbeResult::defaults_declined;
}

// ===========================================================================
// B. 0073d94f-0073d98d
// ===========================================================================

void vfs_register_provider_factory_00be0660(std::vector<const void*>& factories,
    const void* factory)
{
    // The native never inspects the pointer: a null factory is appended like
    // any other and the list length guard at 00bded00 is the only refusal.
    factories.push_back(factory);
}

PakArchiveRegistryState pak_archive_registry_state_00bb4fb0() noexcept
{
    return PakArchiveRegistryState{};
}

// ===========================================================================
// C. 0073db41-0073db69
// ===========================================================================

void run_resource_parser_registration_0073db41(ResourceParserRegistrationHost& host)
{
    void* manager = host.resource_manager_004c1400();               // 0073db41
    void* animation = host.parser_singleton(kAnimationChannelsParser_00736dd0); // 0073db48
    host.register_type_parser_00b80a50(manager, animation);         // 0073db50

    manager = host.resource_manager_004c1400();                     // 0073db55
    void* bone = host.parser_singleton(kBoneParser_00736ea0);       // 0073db5c
    host.register_type_parser_00b80a50(manager, bone);              // 0073db64
}

// ===========================================================================
// D. 00af06a0 and 0073fa70
// ===========================================================================

void publish_startup_singleton_00af06a0(StartupSingletonPublicationHost& host,
    void* instance)
{
    host.enter_lifetime_lock();
    host.publish(instance);
    host.register_lifetime(instance);
    host.leave_lifetime_lock();
}

// ===========================================================================
// E. 00740840
// ===========================================================================

std::int32_t decal_vector_growth_0073e9a0(std::int32_t capacity) noexcept
{
    const std::int32_t doubled = capacity * 2;
    return doubled < 2 ? 1 : doubled;
}

std::vector<DecalDefinition> parse_decal_definitions_00740840(GuiLuaHost& lua,
    const GuiLuaRef& decals_table, const bool& crt_sse2_conversion)
{
    std::vector<DecalDefinition> records;
    if (!decals_table.valid()) {
        return records;
    }

    for (LuaTableScan scan(lua, decals_table); !scan.at_end(); scan.advance()) {
        // 007409b9: the key, not the value, decides whether a record is made.
        if (lua.type_of(scan.key()) != GuiLuaType::String) {
            continue;
        }

        DecalDefinition record;
        // 007409f9, copied into the record's own string at +00/+04.
        const char* name = lua.to_string(scan.key());
        record.name = name != nullptr ? std::string(name) : std::string();

        const GuiLuaRef& value = scan.value();
        record.size = lua_field_number_00b66270(lua, value, kDecalKeySize);
        record.radius = lua_field_number_00b66270(lua, value, kDecalKeyRadius);
        record.max_count = lua_field_integer_00b66290(lua, value, kDecalKeyMaxnum,
            crt_sse2_conversion);
        record.texture_name = lua_field_string_00b662b0(lua, value, kDecalKeyTexture);
        record.shader_name = lua_field_string_00b662b0(lua, value, kDecalKeyShader);
        record.life_time = lua_field_number_00b66270(lua, value, kDecalKeyLifeTime);
        record.fade_out_time = lua_field_number_00b66270(lua, value, kDecalKeyFadeOutTime);

        // 00740d1d: the doubling vector on the system object. std::vector's own
        // growth is not the native rule, so the capacity is stepped explicitly.
        const std::int32_t count = static_cast<std::int32_t>(records.size());
        if (count == static_cast<std::int32_t>(records.capacity())) {
            records.reserve(static_cast<std::size_t>(
                decal_vector_growth_0073e9a0(static_cast<std::int32_t>(records.capacity()))));
        }
        records.push_back(record);
    }
    return records;
}

std::vector<DecalDefinition> load_decal_definitions_00740840(GuiLuaHost& lua,
    const bool& crt_sse2_conversion)
{
    const GuiLuaRef table = lua_global_by_name_00b67980(lua, kDecalTableGlobal);
    std::vector<DecalDefinition> records;
    if (lua.type_of(table) == GuiLuaType::Table) {
        records = parse_decal_definitions_00740840(lua, table, crt_sse2_conversion);
    }
    if (table.valid()) {
        lua.release(table); // 00740da8
    }
    return records;
}

} // namespace bsp
