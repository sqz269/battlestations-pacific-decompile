#include "bsp/sound_sample.hpp"
#include "bsp/sound_sample_cache.hpp"
#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/memory_stream.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T get(const void* p, std::size_t n) noexcept {
    T v; std::memcpy(&v, static_cast<const unsigned char*>(p) + n, sizeof v); return v;
}
template<class T> void put(void* p, std::size_t n, T v) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + n, &v, sizeof v);
}
void* at(void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
NativeString& name(void* p, std::size_t n) noexcept {
    return *reinterpret_cast<NativeString*>(static_cast<unsigned char*>(p) + n);
}
std::int32_t add32(std::int32_t n, std::uint32_t k) noexcept {
    const auto bits = static_cast<std::uint32_t>(n) + k;
    std::memcpy(&n, &bits, sizeof n); return n;
}
struct Name {
    NativeStringStorage& strings;
    NativeString value;
    ~Name() { destroy_native_string_header_0041dd20(&value, strings); }
};
void memory_result(FmodResult result, SoundSampleContext& context) {
    if (static_cast<std::uint32_t>(result) == 0x2b)
        context.fmod.on_out_of_sound_memory("Out of sounjd memory:");
}
const char* data(const NativeString& value, SoundSampleContext& context) {
    return value.data() ? value.data() : context.null_data_00f8bbec;
}
template<std::uint32_t Size> void reserve_trivial(void* header, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (get<std::int32_t>(header, 8) >= requested) return;
    const auto bytes = static_cast<std::uint32_t>(requested) * Size;
    void* const allocated = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    for (std::int32_t i = 0; i < get<std::int32_t>(header, 4); i = add32(i, 1)) {
        const auto offset = static_cast<std::uint32_t>(i) * Size;
        if (void* destination = at(allocated, offset)) {
            void* source = at(get<void*>(header, 0), offset);
            for (std::uint32_t word = 0; word < Size; word += 4)
                put(destination, word, get<std::uint32_t>(source, word));
        }
    }
    singleton_lifetime_free(get<void*>(header, 0));
    put(header, 0, allocated);
    put(header, 8, requested);
}
template<std::uint32_t Size> void resize_trivial(void* header, std::int32_t requested) {
    if (requested > get<std::int32_t>(header, 8)) reserve_trivial<Size>(header, requested);
    for (auto i = get<std::int32_t>(header, 4); i < requested; i = add32(i, 1)) {
        if (void* destination = at(get<void*>(header, 0), static_cast<std::uint32_t>(i) * Size))
            for (std::uint32_t word = 0; word < Size; word += 4) put<std::uint32_t>(destination, word, 0);
    }
    while (requested < get<std::int32_t>(header, 4))
        put(header, 4, add32(get<std::int32_t>(header, 4), 0xffffffffu));
    put(header, 4, requested);
}
void parse_settings(void* sample, NativeTextTokens& scanner) {
    for (;;) {
        const auto& token = scanner.peek_00bee8e0();
        if (scanner.eof_at_token_start() || (token.empty() && !scanner.quoted())) return;
        bool success = true; // Native writes it but never branches on the result.
        if (_stricmp(token.c_str(), "3D") == 0) {
            scanner.accept_00bee800(); put<std::uint32_t>(sample, 8, 1);
        } else if (_stricmp(token.c_str(), "2D") == 0) {
            scanner.accept_00bee800(); put<std::uint32_t>(sample, 8, 0);
        } else if (_stricmp(token.c_str(), "Vol") == 0 || _stricmp(token.c_str(), "Volume") == 0) {
            scanner.accept_00bee800(); put(sample, 0xc, scanner.read_float_00bef170(success));
        } else if (_stricmp(token.c_str(), "Pitch") == 0) {
            scanner.accept_00bee800(); put(sample, 0x10, scanner.read_float_00bef170(success));
        } else if (scanner.accept_keyword_008d4390("FreqRndHz")) put(sample, 0x38, scanner.read_float_00bef170(success));
        else if (scanner.accept_keyword_008d4390("VolRnd")) put(sample, 0x3c, scanner.read_float_00bef170(success));
        else if (scanner.accept_keyword_008d4390("Linear")) put<std::uint8_t>(sample, 0x28, 1);
        else if (scanner.accept_keyword_008d4390("Loop")) put<std::uint8_t>(sample, 0x1d, 1);
        else if (scanner.accept_keyword_008d4390("LoopPointsMillisec")) {
            put<std::uint8_t>(sample, 0x1d, 1); put<std::uint8_t>(sample, 0x1e, 1);
            put(sample, 0x20, scanner.read_float_00bef170(success));
            put(sample, 0x24, scanner.read_float_00bef170(success));
        } else if (scanner.accept_keyword_008d4390("StopAtLoopEnd")) put<std::uint8_t>(sample, 0x29, 1);
        else if (scanner.accept_keyword_008d4390("StopAtSampleEnd")) put<std::uint8_t>(sample, 0x2a, 1);
        else if (scanner.accept_keyword_008d4390("MinDistance")) put(sample, 0x14, scanner.read_float_00bef170(success));
        else if (scanner.accept_keyword_008d4390("MaxDistance")) put(sample, 0x18, scanner.read_float_00bef170(success));
        else if (scanner.accept_keyword_008d4390("Decompress")) put<std::uint8_t>(sample, 0x1c, 1);
        else if (scanner.accept_keyword_008d4390("mActiveLimit")) put(sample, 0x2c, scanner.read_integer_00bef100(success));
        else if (scanner.accept_keyword_008d4390("Ambient")) {
            put<std::uint8_t>(sample, 0x1d, 1); put<std::uint8_t>(sample, 0x40, 1);
            put<std::uint32_t>(sample, 8, 0);
        } else sound_sample_unknown_token_008d5b20(); // Native repeats the cached unknown token forever.
    }
}
SoundOwnedResource* load_resource(void* sample, const NativeString& path, SoundSampleContext& context) {
    auto& owner = context.host.current_resource_owner_00a79910();
    auto* resource = context.host.load_resource_00a84740(owner, path, at(sample, 8), false, true);
    put(sample, 0x78, resource);
    return resource;
}
void copy_resource_timing(void* sample, SoundOwnedResource& resource) {
    put(sample, 0x30, resource.pcm_length_1c.value());
    const float* source = &resource.duration_24.value();
    void* destination = at(sample, 0x34);
    __asm { mov eax, source }
    __asm { fld dword ptr [eax] }
    __asm { mov eax, destination }
    __asm { fstp dword ptr [eax] }
}
void destroy_members(void* sample, NativeStringStorage& strings) {
    // Constructor EH states5..0 at DEBF50 and destructor member sequence.
    // Resource78 is a raw pointer, not an owning subobject in this unwind map.
    resize_sound_sample_pointers_00a7b3d0(at(sample, 0x6c), 0);
    singleton_lifetime_free(get<void*>(sample, 0x6c));
    for (auto offset : {0x60u, 0x58u, 0x50u}) destroy_native_string_header_0041dd20(at(sample, offset), strings);
    resize_sound_sample_parameters_0093f950(at(sample, 0x44), 0);
    singleton_lifetime_free(get<void*>(sample, 0x44));
    put<std::uint32_t>(sample, 0, 0x00ceb130);
}
}
SoundSampleScanner::SoundSampleScanner(std::vector<std::uint8_t> bytes, NativeString&& file,
    std::shared_ptr<MemoryStream> source, NativeStringStorage& pool)
    : tokens(std::move(bytes)), filename(std::move(file)), stream(std::move(source)), strings(pool) {}
SoundSampleScanner::~SoundSampleScanner() {
    stream.reset();
    destroy_native_string_header_0041dd20(&filename, strings);
}
void sound_sample_unknown_token_008d5b20() noexcept {}
void reserve_sound_sample_parameters_0093f6e0(void* header, std::int32_t n) { reserve_trivial<16>(header, n); }
void resize_sound_sample_parameters_0093f950(void* header, std::int32_t n) { resize_trivial<16>(header, n); }
void reserve_sound_sample_pointers_00a7a9e0(void* header, std::int32_t n) { reserve_trivial<4>(header, n); }
void resize_sound_sample_pointers_00a7b3d0(void* header, std::int32_t n) { resize_trivial<4>(header, n); }

void* construct_sound_sample_00a84d70(void* sample, const NativeString& input, SoundSampleContext& context) {
    put<std::uint32_t>(sample, 0, 0x00ceb130); put<std::uint32_t>(sample, 4, 1);
    put<std::uint32_t>(sample, 0, 0x00d5b074);
    for (auto offset : {0x44u, 0x48u, 0x4cu}) put<std::uint32_t>(sample, offset, 0);
    put(sample, 0xc, 1.0f); put(sample, 0x10, 1.0f);
    put(sample, 0x14, 10.0f); put(sample, 0x18, 1000000000.0f);
    put<std::uint32_t>(sample, 8, 0);
    for (auto offset : {0x1du, 0x1eu, 0x1cu}) put<std::uint8_t>(sample, offset, 0);
    put(sample, 0x24, 0.0f); put(sample, 0x20, 0.0f);
    put<std::uint32_t>(sample, 0x2c, 0xffffffffu);
    for (auto offset : {0x40u, 0x29u, 0x2au, 0x28u}) put<std::uint8_t>(sample, offset, 0);
    put(sample, 0x38, 0.0f); put(sample, 0x3c, 0.0f);
    for (auto offset : {0x50u, 0x58u, 0x60u}) new(at(sample, offset)) NativeString();
    for (auto offset : {0x6cu, 0x70u, 0x74u}) put<std::uint32_t>(sample, offset, 0);
    try {
    name(sample, 0x58).copy_from_00be0a30_fragment(context.strings, input);
    put<std::uint32_t>(sample, 0x68, 0);
    auto& requested = name(sample, 0x58);
    const char* colon = requested.data() ? std::strstr(requested.data(), ":") : nullptr;
    const auto position = colon ? static_cast<std::int32_t>(colon - requested.data()) : -1;
    if (position >= 0) {
        name(sample, 0x50).copy_from_00be0a30_fragment(context.strings, requested);
        Name file{context.strings, {}};
        construct_native_string_substring_00469840(&requested, &file.value, 0, static_cast<std::uint32_t>(position), context.strings);
        context.host.resolve_name_00bdf4c0(file.value);
        {
            Name event{context.strings, {}};
            construct_native_string_substring_00469840(&requested, &event.value,
                static_cast<std::uint32_t>(position) + 1u, 0x7fffffffu, context.strings);
            name(sample, 0x60).copy_from_00be0a30_fragment(context.strings, event.value);
        }
        load_resource(sample, file.value, context);
        load_sound_sample_event_group_00a83850(sample, context);
        put<std::uint32_t>(sample, 8, 1);
        read_sound_sample_event_parameters_00a828b0(sample, context);
    } else {
        context.host.resolve_name_00bdf4c0(requested);
        {
            Name extension{context.strings, {}}, prefix{context.strings, {}}, definition{context.strings, {}};
            extension.value.assign_0041e870(context.strings, ".def");
            construct_native_string_substring_00469840(&requested, &prefix.value, 0, requested.length() - 4u, context.strings);
            concatenate_native_string_headers_004261a0(&prefix.value, &definition.value, &extension.value, context.strings);
            name(sample, 0x50).copy_from_00be0a30_fragment(context.strings, definition.value);
        }
        SoundOwnedResource* resource;
        if (context.host.resolve_name_00bdf4c0(name(sample, 0x50))) {
            {
                auto scanner = context.host.open_scanner_00bef2e0(name(sample, 0x50));
                if (!scanner) throw std::runtime_error("Sample definition scanner allocation/open failed");
                parse_settings(sample, scanner->tokens);
            }
            resource = load_resource(sample, requested, context);
        } else resource = load_resource(sample, input, context); // Native uses the original argument here.
        if (!resource) throw std::runtime_error("Sample bank load did not produce the required resource");
        copy_resource_timing(sample, *resource);
    }
    return sample;
    } catch (...) {
        destroy_members(sample, context.strings);
        throw;
    }
}

void load_sound_sample_event_group_00a83850(void* sample, SoundSampleContext& context) {
    std::int32_t before{}, after{};
    context.fmod.memory_get_stats(&before, nullptr);
    struct Parts {
        NativeStringStorage& strings; std::vector<NativeString> values;
        ~Parts() { for (auto i = values.rbegin(); i != values.rend(); ++i) destroy_native_string_header_0041dd20(&*i, strings); }
    } parts{context.strings, {}};
    parts.values.reserve(8); // 00426520/00BD20A0/00427110 library-container contract.
    auto& event_name = name(sample, 0x60);
    std::uint32_t start = 0;
    while (start < event_name.length()) {
        if (event_name.data()[start] == '/') { ++start; continue; }
        auto end = start;
        while (end < event_name.length() && event_name.data()[end] != '/') ++end;
        Name part{context.strings, {}};
        construct_native_string_substring_00469840(&event_name, &part.value, start, end - start, context.strings);
        Name key{context.strings, {}};
        key.value.copy_from_00be0a30_fragment(context.strings, part.value);
        parts.values.push_back(std::move(key.value));
        start = end;
    }
    if (parts.values.size() < 2) throw std::invalid_argument("Native event path requires project/group segments");
    auto* resource = get<SoundOwnedResource*>(sample, 0x78);
    if (!resource) throw std::runtime_error("Event sample requires a loaded project resource");
    auto** output = reinterpret_cast<void**>(at(sample, 0x68));
    memory_result(context.fmod.event_project_get_group(resource->event_project_08,
        data(parts.values[1], context), 0, output), context);
    for (std::size_t i = 2; i + 1 < parts.values.size(); ++i)
        memory_result(context.fmod.event_group_get_group(get<void*>(sample, 0x68),
            data(parts.values[i], context), 0, output), context);
    memory_result(context.fmod.event_group_load_event_data(get<void*>(sample, 0x68), 0, 0), context);
    context.fmod.memory_get_stats(&after, nullptr);
    get<SoundOwnedResource*>(sample, 0x78)->size_28 += static_cast<std::uint32_t>(after) - static_cast<std::uint32_t>(before);
}
void read_sound_sample_event_parameters_00a828b0(void* sample, SoundSampleContext& context) {
    void* system = context.host.current_event_system_00f8bbd8_48();
    void* event{};
    memory_result(context.fmod.event_system_get_event(system, data(name(sample, 0x60), context), 4, &event), context);
    if (!event) throw std::runtime_error("FMOD event query left its required output unavailable");
    std::int32_t count = 0;
    memory_result(context.fmod.event_get_num_parameters(event, &count), context);
    void* header = at(sample, 0x44);
    reserve_sound_sample_parameters_0093f6e0(header, count);
    for (std::int32_t i = 0; i < count; i = add32(i, 1)) {
        resize_sound_sample_parameters_0093f950(header, add32(i, 1));
        void* parameter{};
        memory_result(context.fmod.event_get_parameter_by_index(event, i, &parameter), context);
        if (!parameter) throw std::runtime_error("FMOD parameter query left its required output unavailable");
        const auto offset = static_cast<std::uint32_t>(i) * 16u;
        void* record = at(get<void*>(header, 0), offset);
        memory_result(context.fmod.event_parameter_get_range(parameter,
            static_cast<float*>(at(record, 4)), static_cast<float*>(at(record, 8))), context);
        record = at(get<void*>(header, 0), offset); // Reload after range and memory callbacks.
        std::int32_t unused_index;
        memory_result(context.fmod.event_parameter_get_info(parameter, &unused_index,
            static_cast<char**>(at(record, 12))), context);
        put(at(get<void*>(header, 0), offset), 0, i);
    }
}
void remove_sound_sample_by_pointer_00a82b70(SoundAuxiliaryTreeOwner& owner, void* sample,
    SoundSampleCacheContext& context) {
    for (auto i = owner.tree_04->begin(); i != owner.tree_04->end(); ++i) {
        if (i->second.sample_14 != sample) continue;
        if (owner.native_vtable_00 == 0x00d5b460) erase_sound_sample_iterator_00a83e80(owner, i, context.strings);
        else context.host.unknown_cache_erase_slot_08(owner, i);
        return;
    }
}
void destroy_sound_sample_00a82c60(void* sample, SoundSampleContext& context) {
    put<std::uint32_t>(sample, 0, 0x00d5b074);
    try {
    if (get<void*>(sample, 0x68)) {
        std::int32_t before{}, after{};
        context.fmod.memory_get_stats(&before, nullptr);
        memory_result(context.fmod.event_group_free_event_data(get<void*>(sample, 0x68), nullptr, 1), context);
        memory_result(context.fmod.event_group_free_event_data(get<void*>(sample, 0x68), nullptr, 1), context);
        context.fmod.memory_get_stats(&after, nullptr);
        get<SoundOwnedResource*>(sample, 0x78)->size_28 += static_cast<std::uint32_t>(after) - static_cast<std::uint32_t>(before);
    }
    auto& cache = context.host.current_sample_cache_00f8bbe8();
    remove_sound_sample_by_pointer_00a82b70(cache, sample, context.host.sample_cache_context());
    if (auto* resource = get<SoundOwnedResource*>(sample, 0x78)) {
        context.host.release_resource_00a854e0(*resource);
        put<void*>(sample, 0x78, nullptr);
    }
    } catch (...) {
        destroy_members(sample, context.strings);
        throw;
    }
    destroy_members(sample, context.strings);
}
void* scalar_delete_sound_sample_00a82e80(void* sample, std::uint32_t flags, SoundSampleContext& context) {
    destroy_sound_sample_00a82c60(sample, context);
    if (flags & 1u) singleton_lifetime_free(sample);
    return sample;
}
void destroy_native_text_scanner_00bef220(void* scanner, const void* default_separators,
    NativeStringStorage& strings, GameplayEffectComponentLifetime& lifetime) {
    void* separator = get<void*>(scanner, 0x814);
    if (separator != default_separators && separator) {
        singleton_lifetime_free(separator);
        put<void*>(scanner, 0x814, nullptr);
    }
    if (void* stream = get<void*>(scanner, 0x824)) {
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(at(stream, 4))) == 0)
            lifetime.zero_references_slot_00(stream);
        put<void*>(scanner, 0x824, nullptr);
    }
    destroy_native_string_header_0041dd20(at(scanner, 0x81c), strings);
}
}
