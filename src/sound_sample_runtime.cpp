#include "bsp/sound_sample_runtime.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T get(const void* p, std::size_t n) {
    T result; std::memcpy(&result, static_cast<const unsigned char*>(p) + n, sizeof result); return result;
}
std::string text(const NativeString& value) {
    return value.data() ? std::string(value.data(), value.length()) : std::string{};
}
SoundResourceLoadOptions project_options(const void* p) {
    SoundResourceLoadOptions options;
    options.field_00 = get<std::uint32_t>(p, 0);
    options.field_04 = get<float>(p, 4); options.field_08 = get<float>(p, 8);
    options.field_0c = get<float>(p, 0xc); options.field_10 = get<float>(p, 0x10);
    for (std::size_t i = 0; i != 3; ++i) {
        options.flags_14[i] = get<std::uint8_t>(p, 0x14 + i);
        options.flags_20[i] = get<std::uint8_t>(p, 0x20 + i);
    }
    options.field_18 = get<float>(p, 0x18); options.field_1c = get<float>(p, 0x1c);
    options.field_24 = get<std::int32_t>(p, 0x24);
    // +28/+2C are not initialized at constructor load sites and are not read
    // by the recovered resource loader; leave the projection explicitly unknown.
    options.field_30 = get<float>(p, 0x30); options.field_34 = get<float>(p, 0x34);
    options.flag_38 = get<std::uint8_t>(p, 0x38);
    const auto* records = get<const std::array<std::uint32_t, 4>*>(p, 0x3c);
    const auto count = get<std::int32_t>(p, 0x40);
    if (count > 0) options.records_3c.assign(records, records + count);
    options.capacity_44 = get<std::int32_t>(p, 0x44);
    return options;
}
}
SoundSampleRuntime::SoundSampleRuntime(SoundSystemOwner* volatile& owner,
    SoundAuxiliaryTreeOwner* volatile& current_cache, std::uint32_t& bytes, VfsMountContext& mounts,
    const VfsCandidateRegistrations& registrations, SoundResourceRuntime& resources,
    FmodConfigurationLibrary& fmod, NativeStringStorage& strings, const char* null_pattern, const char* null_data)
    : current_owner_(owner), current_cache_(current_cache), mounts_(mounts),
      registrations_(registrations), resources_(resources), strings_(strings),
      cache_{strings, *this, bytes, null_pattern, null_data}, sample_{strings, *this, fmod, null_data} {}
void* SoundSampleRuntime::construct_sample_00a84d70(void* sample, const NativeString& name) {
    return construct_sound_sample_00a84d70(sample, name, sample_);
}
void* SoundSampleRuntime::unknown_cache_create_slot_04(SoundAuxiliaryTreeOwner&, NativeString&) {
    throw std::logic_error("Sample runtime requires the recovered D5B460 cache profile");
}
void SoundSampleRuntime::unknown_cache_erase_slot_08(SoundAuxiliaryTreeOwner&, SoundAuxiliaryTreeOwner::Tree::iterator) {
    throw std::logic_error("Sample runtime requires the recovered D5B460 cache profile");
}
void SoundSampleRuntime::zero_references_slot_00(void* sample) {
    if (get<std::uint32_t>(sample, 0) != 0x00d5b074)
        throw std::logic_error("Sample final release requires the recovered D5B074 profile");
    scalar_delete_sound_sample_00a82e80(sample, 1, sample_);
}
bool SoundSampleRuntime::resolve_name_00bdf4c0(NativeString& name) {
    auto resolved = text(name);
    const bool found = resolve_existing_resource_00bdf4c0_fragment(mounts_, registrations_, resolved);
    assign_native_string_cstring_0041e350(&name, resolved.c_str(), strings_);
    return found;
}
std::unique_ptr<SoundSampleScanner> SoundSampleRuntime::open_scanner_00bef2e0(const NativeString& path) {
    NativeString filename;
    copy_construct_native_string_header_00426060(&filename, &path, strings_);
    try {
        auto opened = open_resource_memory_00bdf310_fragment(mounts_, text(filename), 0x32);
        if (!opened.provider_opened || !opened.stream || !opened.stream->has_backing() || !opened.stream->fully_initialized())
            throw std::runtime_error("Sample definition scanner requires a complete VFS file");
        const auto length = opened.stream->size_00bef600();
        if (length < 0 || length > INT32_MAX) throw std::length_error("Sample definition exceeds signed Win32 size");
        std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
        if (length != 0) {
            std::uint32_t actual{};
            if (!opened.stream->read_00bef590(bytes.data(), static_cast<std::uint32_t>(length), &actual)
                || actual != static_cast<std::uint32_t>(length))
                throw std::runtime_error("Sample definition VFS read was incomplete");
        }
        return std::make_unique<SoundSampleScanner>(std::move(bytes), std::move(filename), std::move(opened.stream), strings_);
    } catch (...) { destroy_native_string_header_0041dd20(&filename, strings_); throw; }
}
SoundResourceOwner& SoundSampleRuntime::current_resource_owner_00a79910() {
    auto* owner = current_owner_;
    if (!owner || !owner->resource_owner_54) throw std::logic_error("Sample load requires the current resource owner");
    return *owner->resource_owner_54;
}
SoundOwnedResource* SoundSampleRuntime::load_resource_00a84740(SoundResourceOwner& owner,
    const NativeString& path, void* actual_options, bool clone, bool load_if_missing) {
    auto options = project_options(actual_options);
    return resources_.load_00a84740(owner, path, options, clone, load_if_missing);
}
void* SoundSampleRuntime::current_event_system_00f8bbd8_48() {
    auto* owner = current_owner_;
    if (!owner) throw std::logic_error("Sample parameter query requires the current sound owner");
    return owner->system.event_system;
}
SoundAuxiliaryTreeOwner& SoundSampleRuntime::current_sample_cache_00f8bbe8() {
    auto* cache = current_cache_;
    if (!cache) throw std::logic_error("Sample destruction requires the current sample cache");
    return *cache;
}
void SoundSampleRuntime::release_resource_00a854e0(SoundOwnedResource& resource) {
    resources_.release_resource(resource);
}
}
