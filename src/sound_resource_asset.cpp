#include "bsp/sound_resource_asset.hpp"

#include "bsp/memory_stream.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/resource_lookup.hpp"

#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>

namespace bsp {
namespace {
struct StringCleanup {
    NativeString& value;
    NativeStringStorage& storage;
    ~StringCleanup() { value.release_to(storage); }
};

std::string string_value(const NativeString& value) {
    return value.data() ? std::string(value.data(), value.length()) : std::string{};
}

void replace_value(NativeString& value, const std::string& text,
    NativeStringStorage& storage) {
    value.resize_0041dd40(storage, static_cast<std::uint32_t>(text.size()), true);
    if (!text.empty()) std::memcpy(value.data(), text.data(), text.size());
}

void check_memory(FmodResult result, SoundResourceAssetFmodHost& fmod) {
    if (result == FmodResult::err_memory) {
        std::int32_t current, maximum;
        fmod.memory_get_stats(&current, &maximum); // actual 00A7A460, no log
    }
}

void require_output(FmodResult result, const char* operation) {
    // Native continues after every FMOD error and can read unwritten locals.
    // This is a host-domain guard, not a recovered native error branch.
    if (result != FmodResult::ok) throw std::runtime_error(operation);
}

bool extension_equals(const NativeString& name, const char* extension,
    NativeStringStorage& strings) {
    NativeString suffix;
    StringCleanup cleanup{suffix, strings};
    construct_native_string_substring_00469840(&name, &suffix,
        name.length() - 4u, 0x7fffffffu, strings);
    return suffix.data() && _stricmp(suffix.data(), extension) == 0;
}

struct FileBytes {
    std::unique_ptr<char[]> bytes;
    std::uint32_t size;
};

// 00BDC8B0 projected over the canonical VFS memory-stream adapter: name copy,
// flags2 open, low size, exact allocation/read, release stream, publish size.
// The adapter already establishes fully initialized host backing. Unavailable
// or incomplete sources are outside its domain; native lacks those guards.
FileBytes load_file_bytes_00bdc8b0_fragment(const NativeString& name,
    SoundResourceAssetContext& context) {
    NativeString copy;
    StringCleanup cleanup{copy, context.strings};
    copy.copy_from_00be0a30_fragment(context.strings, name);
    auto opened = open_resource_memory_00bdf310_fragment(context.mounts,
        string_value(copy), 2);
    if (!opened.stream || !opened.stream->fully_initialized())
        throw std::runtime_error("Sound VFS source unavailable or incomplete");
    const auto size = static_cast<std::uint32_t>(opened.stream->size_00bef600());
    std::unique_ptr<char[]> bytes(new char[size]);
    if (!opened.stream->read_00bef590(bytes.get(), size, nullptr))
        throw std::runtime_error("Sound VFS memory-stream read failed");
    opened.stream.reset();
    return {std::move(bytes), size};
}

std::uint32_t loop_point(float value) noexcept {
    // Native changes x87 rounding to truncation, FISTP signed qword, takes its
    // low DWORD. Invalid/out-of-range conversions produce integer-indefinite
    // 8000000000000000, whose low DWORD is zero, with masked x87 exceptions.
    std::uint16_t saved_control, truncate_control;
    std::int64_t converted;
    __asm {
        fld value
        fnstcw saved_control
        mov ax, saved_control
        or ax, 0c00h
        mov truncate_control, ax
        fldcw truncate_control
        fistp qword ptr converted
        fldcw saved_control
    }
    return static_cast<std::uint32_t>(converted);
}

void set_timing(SoundOwnedResource& resource, std::uint32_t pcm_count,
    float base_frequency, float multiplier) noexcept {
    float scaled, duration;
    const float unsigned_bias = 4294967296.0f; // image CE3978 = 4F800000
    __asm {
        fld multiplier
        fmul base_frequency
        fstp scaled
        fild dword ptr pcm_count
        cmp pcm_count, 0
        jge nonnegative_length
        fadd unsigned_bias
    nonnegative_length:
        fdiv scaled
        fstp duration
    }
    resource.frequency_20 = scaled;
    resource.duration_24 = duration;
}
} // namespace

NativeString resolve_sound_resource_00a82ea0(const NativeString& name,
    VfsMountContext& mounts, const VfsCandidateRegistrations& registrations,
    NativeStringStorage& strings) {
    NativeString copy;
    StringCleanup cleanup{copy, strings};
    copy.copy_from_00be0a30_fragment(strings, name);
    auto resolved = string_value(copy);
    const bool found = resolve_existing_resource_00bdf4c0_fragment(
        mounts, registrations, resolved);
    replace_value(copy, resolved, strings);
    NativeString fallback;
    StringCleanup fallback_cleanup{fallback, strings};
    if (!found) fallback.assign_0041e870(strings, kSoundErrorResourcePath);
    NativeString output;
    try {
        output.copy_from_00be0a30_fragment(strings, found ? copy : fallback);
    } catch (...) {
        output.release_to(strings);
        throw;
    }
    return output;
}

void load_sound_bank_00a823f0(SoundOwnedResource& resource,
    const SoundResourceLoadOptions& options, SoundResourceAssetContext& context) {
    auto& fmod = context.fmod;
    SoundFmodCreateInfo41804 extra;
    auto file = load_file_bytes_00bdc8b0_fragment(resource.name_14, context);
    extra.length = file.size;
    std::uint32_t mode = options.field_00 == 0 ? 0xa4au :
        options.field_00 == 1 ? 0xa52u : 0x802u;
    if (options.flags_20[0]) mode |= 0x200000u;
    resource.fsb_bank_0c = nullptr;
    resource.fsb_subsound_10 = nullptr;
    std::int32_t before, after;
    fmod.memory_get_stats(&before, nullptr);
    auto result = fmod.create_sound(context.system, file.bytes.get(), mode,
        &extra, &resource.fsb_bank_0c);
    check_memory(result, fmod);
    fmod.memory_get_stats(&after, nullptr);
    // Native debug print 004254B0 has an empty body; before/after otherwise unused.
    std::uint32_t raw_size;
    result = fmod.sound_get_length(resource.fsb_bank_0c, &raw_size, 8);
    check_memory(result, fmod);
    require_output(result, "FMOD bank raw-length output unavailable");
    resource.size_28 = raw_size;
    file.bytes.reset(); // native VFS +14 / 00BD91F0, before subsound queries
    std::int32_t subsound_count;
    result = fmod.sound_get_num_subsounds(resource.fsb_bank_0c, &subsound_count);
    check_memory(result, fmod); // count is not consumed or tested by native
    result = fmod.sound_get_subsound(resource.fsb_bank_0c, 0, &resource.fsb_subsound_10);
    check_memory(result, fmod);
    result = fmod.sound_get_mode(resource.fsb_subsound_10, &mode);
    check_memory(result, fmod);
    require_output(result, "FMOD subsound mode output unavailable");
    std::uint32_t pcm_length;
    result = fmod.sound_get_length(resource.fsb_subsound_10, &pcm_length, 2);
    check_memory(result, fmod);
    require_output(result, "FMOD subsound PCM-length output unavailable");
    resource.pcm_length_1c = pcm_length;
    float frequency;
    result = fmod.sound_get_defaults(resource.fsb_subsound_10, &frequency,
        nullptr, nullptr, nullptr);
    check_memory(result, fmod);
    require_output(result, "FMOD subsound frequency output unavailable");
    // Preserve x87 precision/rounding, single product materialization, and the
    // signed-FILD/2^32 correction. Ordinary uint32-to-float changes that path.
    set_timing(resource, pcm_length, frequency, options.field_08);
    if (mode & 0x10u) {
        result = fmod.sound_set_3d_min_max_distance(resource.fsb_subsound_10,
            options.field_0c, options.field_10);
        check_memory(result, fmod);
    }
    result = fmod.sound_set_variations(resource.fsb_subsound_10,
        options.field_30, options.field_34, 0.0f);
    check_memory(result, fmod);
    if (options.flags_14[2]) {
        const auto end = loop_point(options.field_1c);
        const auto start = loop_point(options.field_18);
        result = fmod.sound_set_loop_points(resource.fsb_subsound_10, start, 1, end, 1);
        check_memory(result, fmod);
    }
    mode = options.flags_14[0] ? (mode & 0xfffffdffu) | 0x100u :
        (mode & 0xfffffeffu) | 0x200u;
    result = fmod.sound_set_mode(resource.fsb_subsound_10, mode);
    check_memory(result, fmod);
}

void load_sound_event_project_00a82720(SoundOwnedResource& resource,
    SoundResourceAssetContext& context) {
    std::int32_t before, after;
    context.fmod.memory_get_stats(&before, nullptr);
    const auto result = context.fmod.event_system_load(context.event_system,
        resource.name_14.data() ? resource.name_14.data() : "", nullptr,
        &resource.event_project_08);
    check_memory(result, context.fmod);
    context.fmod.memory_get_stats(&after, nullptr);
    resource.size_28 = static_cast<std::uint32_t>(after) -
        static_cast<std::uint32_t>(before);
}

SoundOwnedResource& construct_sound_resource_00a83450(SoundOwnedResource& resource,
    const NativeString& name, const SoundResourceLoadOptions& options,
    SoundResourceAssetContext& context) {
    resource.native_vtable_00 = 0x00ceb130;
    resource.references_04 = 1;
    resource.native_vtable_00 = 0x00d5b118;
    resource.size_28 = 0;
    try {
        resource.name_14.copy_from_00be0a30_fragment(context.strings, name);
        resource.fsb_bank_0c = nullptr;
        resource.fsb_subsound_10 = nullptr;
        resource.event_project_08 = nullptr;
        if (extension_equals(resource.name_14, ".fsb", context.strings))
            load_sound_bank_00a823f0(resource, options, context);
        else if (extension_equals(resource.name_14, ".fev", context.strings))
            load_sound_event_project_00a82720(resource, context);
    } catch (...) {
        // Native constructor unwind destroys its constructed string subobject;
        // it does not run the completed resource's destructor.
        resource.name_14.release_to(context.strings);
        throw;
    }
    return resource;
}

SoundOwnedResource* create_sound_resource_00a835b0(const NativeString& name,
    const SoundResourceLoadOptions& options, SoundResourceAssetContext& context) {
    NativeString copy;
    StringCleanup cleanup{copy, context.strings};
    copy.copy_from_00be0a30_fragment(context.strings, name);
    if (!exists_resource_00bdd440_fragment(context.mounts, string_value(name)))
        return nullptr; // native missing-sound debug print has an empty body
    std::unique_ptr<SoundOwnedResource> resource(new (std::nothrow) SoundOwnedResource);
    if (!resource) return nullptr;
    construct_sound_resource_00a83450(*resource, name, options, context);
    auto* output = resource.release();
    if (output->fsb_subsound_10 || output->event_project_08) return output;
    context.lifetime.delete_resource_00a85ac0(*output, 1);
    return nullptr;
}

} // namespace bsp
