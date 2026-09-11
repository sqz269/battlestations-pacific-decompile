#pragma once
#include "bsp/native_text_tokens.hpp"
#include "bsp/sound_resource_asset.hpp"
#include <cstddef>

namespace bsp {
struct SoundSampleCacheContext;
struct GameplayEffectComponentLifetime;
// Actual 7Ch sample storage, including the intrusive count at4 and actual
// pooled string/array headers. Unwritten bytes are preserved by construction.
struct SoundSampleStorage { alignas(4) std::array<std::byte, 0x7c> bytes; };
static_assert(sizeof(SoundSampleStorage) == 0x7c);

class SoundSampleFmodHost {
public:
    virtual ~SoundSampleFmodHost() = default;
    virtual void memory_get_stats(std::int32_t*, std::int32_t*) = 0;
    virtual void on_out_of_sound_memory(const char*) = 0;
    virtual FmodResult event_project_get_group(void*, const char*, std::int32_t, void**) = 0;
    virtual FmodResult event_group_get_group(void*, const char*, std::int32_t, void**) = 0;
    virtual FmodResult event_group_load_event_data(void*, std::uint32_t, std::uint32_t) = 0;
    virtual FmodResult event_group_free_event_data(void*, void*, std::int32_t) = 0;
    virtual FmodResult event_system_get_event(void*, const char*, std::uint32_t, void**) = 0;
    virtual FmodResult event_get_num_parameters(void*, std::int32_t*) = 0;
    virtual FmodResult event_get_parameter_by_index(void*, std::int32_t, void**) = 0;
    virtual FmodResult event_parameter_get_range(void*, float*, float*) = 0;
    virtual FmodResult event_parameter_get_info(void*, std::int32_t*, char**) = 0;
};

// Scanner/file are existing host projections; retain the source stream until
// scanner destruction, then release its pooled filename. Tokens use the
// recovered immutable-byte parser, not the original 828h stream ABI.
struct SoundSampleScanner {
    NativeTextTokens tokens;
    NativeString filename;
    std::shared_ptr<MemoryStream> stream;
    NativeStringStorage& strings;
    SoundSampleScanner(std::vector<std::uint8_t>, NativeString&&,
        std::shared_ptr<MemoryStream>, NativeStringStorage&);
    ~SoundSampleScanner();
};
class SoundSampleHost {
public:
    virtual ~SoundSampleHost() = default;
    virtual bool resolve_name_00bdf4c0(NativeString&) = 0;
    virtual std::unique_ptr<SoundSampleScanner> open_scanner_00bef2e0(const NativeString&) = 0;
    virtual SoundResourceOwner& current_resource_owner_00a79910() = 0;
    // Options point into the current actual sample+8 record. Runtime adapts
    // its fields into the existing resource-loader projection at this boundary.
    virtual SoundOwnedResource* load_resource_00a84740(SoundResourceOwner&,
        const NativeString&, void* actual_options, bool clone, bool load_if_missing) = 0;
    virtual void* current_event_system_00f8bbd8_48() = 0;
    virtual SoundAuxiliaryTreeOwner& current_sample_cache_00f8bbe8() = 0;
    virtual SoundSampleCacheContext& sample_cache_context() = 0;
    virtual void release_resource_00a854e0(SoundOwnedResource&) = 0;
};
struct SoundSampleContext {
    NativeStringStorage& strings;
    SoundSampleHost& host;
    SoundSampleFmodHost& fmod;
    const char* null_data_00f8bbec; // Required live writable storage.
};

void* construct_sound_sample_00a84d70(void*, const NativeString&, SoundSampleContext&);
void load_sound_sample_event_group_00a83850(void*, SoundSampleContext&);
void read_sound_sample_event_parameters_00a828b0(void*, SoundSampleContext&);
void destroy_sound_sample_00a82c60(void*, SoundSampleContext&);
void* scalar_delete_sound_sample_00a82e80(void*, std::uint32_t, SoundSampleContext&);
void remove_sound_sample_by_pointer_00a82b70(SoundAuxiliaryTreeOwner&, void*, SoundSampleCacheContext&);
void reserve_sound_sample_parameters_0093f6e0(void* actual_array_header, std::int32_t);
void resize_sound_sample_parameters_0093f950(void* actual_array_header, std::int32_t);
void reserve_sound_sample_pointers_00a7a9e0(void* actual_array_header, std::int32_t);
void resize_sound_sample_pointers_00a7b3d0(void* actual_array_header, std::int32_t);
// Normal 008D5B20 only installs/removes an EH frame; it does not accept a token.
void sound_sample_unknown_token_008d5b20() noexcept;
// Separate actual 828h scanner destruction body, including stream refcount.
// The immutable-byte SoundSampleScanner above is explicitly a host projection.
void destroy_native_text_scanner_00bef220(void*, const void* current_default_separators,
    NativeStringStorage&, GameplayEffectComponentLifetime&);
}
