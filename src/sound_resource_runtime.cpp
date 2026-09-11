#include "bsp/sound_resource_runtime.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
SoundResourceRuntime::SoundResourceRuntime(SoundSystemState& system,
    SoundSystemOwner* volatile& current_owner, volatile std::uint32_t& actual_bytes,
    VfsMountContext& mounts, const VfsCandidateRegistrations& registrations,
    ResourceLoadEventHost& events, FmodConfigurationLibrary& library,
    NativeStringStorage& strings)
    : system_(system), current_owner_(current_owner), mounts_(mounts),
      registrations_(registrations), events_(events), library_(library),
      strings_(strings), cleanup_{strings, actual_bytes, library, *this} {}

SoundOwnedResource* SoundResourceRuntime::load_00a84740(SoundResourceOwner& owner,
    const NativeString& path, SoundResourceLoadOptions& options, bool clone,
    bool load_if_missing) {
    return load_sound_resource_00a84740(owner, path, options, clone, load_if_missing,
        *this, strings_);
}
void SoundResourceRuntime::destroy_failed_owner_00a85500(SoundResourceOwner& owner) noexcept {
    destroy_sound_resource_owner_base_00a85500(owner, cleanup_);
}
void SoundResourceRuntime::destroy_owner(SoundResourceOwner& owner) {
    destroy_sound_resource_owner_00a85a50(owner, cleanup_);
}
void SoundResourceRuntime::release_resource(SoundOwnedResource& resource) {
    release_sound_resource_00a854e0(resource, cleanup_);
}
void SoundResourceRuntime::pump_load_events_00beccd0() {
    pump_resource_load_events_00beccd0(events_);
}
NativeString SoundResourceRuntime::resolve_00a82ea0(SoundResourceOwner&,
    const NativeString& name, SoundResourceLoadOptions&, NativeStringStorage& strings) {
    return resolve_sound_resource_00a82ea0(name, mounts_, registrations_, strings);
}
SoundOwnedResource* SoundResourceRuntime::create_00a835b0(SoundResourceOwner&,
    const NativeString& name, SoundResourceLoadOptions& options) {
    SoundResourceAssetContext context{mounts_, registrations_, strings_, library_,
        system_.system, system_.event_system, *this};
    return create_sound_resource_00a835b0(name, options, context);
}
SoundOwnedResource* SoundResourceRuntime::retain_00a854c0(SoundResourceOwner&,
    SoundOwnedResource& resource) {
    return retain_sound_resource_00a854c0(resource);
}
std::uint32_t SoundResourceRuntime::resource_size_vslot_0c(SoundOwnedResource& resource) {
    return sound_resource_size_00a818b0(resource);
}
VfsFileDate SoundResourceRuntime::query_metadata_00bdd340(const NativeString& name) {
    return query_vfs_file_date_00bdd340(mounts_,
        name.data() ? std::string(name.data(), name.length()) : std::string{});
}
void SoundResourceRuntime::delete_resource_00a85ac0(SoundOwnedResource& resource,
    std::uint32_t flags) {
    delete_sound_resource_00a85ac0(&resource, flags, cleanup_);
}
void SoundResourceRuntime::remove_from_current_cache(const NativeString& name) {
    auto* current = current_owner_; // native destructor reloads the singleton
    auto* owner = current ? get_sound_resource_owner_00a79910(*current) : nullptr;
    if (!owner)
        throw std::logic_error("Sound cleanup requires the current resource cache owner");
    remove_sound_resource_cache_entry_00a85560(*owner, name, *this, strings_);
}

std::array<std::uint32_t, 4> FrameClockSoundStartupHost::sample_time_14() {
    ClockTimestamp timestamp;
    if (!sample_frame_clock_00bee080(clock_, timestamp))
        throw std::runtime_error("Sound startup clock sample failed");
    std::array<std::uint32_t, 4> words;
    static_assert(sizeof(words) == sizeof(timestamp));
    std::memcpy(words.data(), &timestamp, sizeof(words));
    return words;
}

VfsSoundConfigurationLuaOwner::VfsSoundConfigurationLuaOwner(VfsLuaScriptFiles& files,
    LuaScriptRuntime& runtime, const LuaRuntimeGlobals& globals) noexcept
    : files_(files), runtime_(runtime), globals_(globals) {}
VfsSoundConfigurationLuaOwner::~VfsSoundConfigurationLuaOwner() { close(); }
GuiLuaHost& VfsSoundConfigurationLuaOwner::construct_and_open(std::uint32_t mask) {
    if (owner_) throw std::logic_error("Sound Lua owner is already open");
    owner_ = std::make_unique<PcStorageLuaOwner>(files_.owner_environment(runtime_, globals_));
    try {
        owner_->open_storage_archive_00b6a020(mask);
        reader_ = std::make_unique<GuiLua51Host>(*owner_->storage_lua_38());
    } catch (...) {
        close();
        throw;
    }
    return *reader_;
}
void VfsSoundConfigurationLuaOwner::load_and_call_chunk(const char* path,
    std::int32_t argument) {
    if (!owner_) throw std::logic_error("Sound Lua owner is closed");
    runtime_.run_chunk(owner_->storage_lua_38(), path, argument != 0);
}
void VfsSoundConfigurationLuaOwner::close() noexcept {
    reader_.reset();
    if (owner_) owner_->close_storage_archive_00b65e80();
    owner_.reset();
}
}
