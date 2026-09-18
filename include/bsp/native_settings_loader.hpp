#pragma once
#include "bsp/native_language_catalog_producer.hpp"
#include "bsp/native_scene_token_values.hpp"
#include "bsp/native_settings_renderer.hpp"
#include "bsp/native_settings_text.hpp"

namespace bsp {
using NativeSettingsDecrement=std::int32_t(__stdcall*)(volatile std::int32_t*);
struct NativeSettingsLoaderCalls {
    virtual ~NativeSettingsLoaderCalls()=default;
    virtual int seek_00bfb72b(void*,std::int32_t,int);
    virtual std::int32_t tell_00bfb636(void*);
    virtual std::uint32_t read_00bfb483(void*,std::uint32_t,std::uint32_t,void*);
    virtual void* allocate_backing_00bf681b();
    virtual int compare_00438e10(const char*,const char*);
    // 4254B0 is a verified RET in this binary. Defaults preserve that no-op;
    // diagnostics may observe the native format and already-captured arguments.
    virtual void unknown_token_004254b0(const char* format,const char* token);
    virtual void desktop_size_004254b0(const char* format,std::int32_t width,std::int32_t height);
    virtual void* desktop_window_00ce2360();
    virtual int window_rect_00ce235c(void*,void* rectangle_16);
    virtual std::int32_t registry_open_00ce2010(std::uint32_t,const char*,std::uint32_t,std::uint32_t,std::uint32_t*);
    virtual std::int32_t registry_query_00ce200c(std::uint32_t,const char*,std::uint32_t*,void*,std::uint32_t*);
    virtual std::int32_t registry_close_00ce2008(std::uint32_t);
    // Default admits the actual D5F0A8 renderer and verified slot104 B1FF50,
    // then calls that complete raw getter. Other profiles require a binding.
    virtual const void* renderer_capabilities_slot104(void*,const volatile std::uint32_t* actual_profile_00d5f0a8);
    virtual void memory_zero_reference_slot0(void*,NativeRetainedMemoryOwnerContext&);
};
// 467CC0: ECX tokenizer, stack keyword, RET4; result is AL only. Peek, compare
// case-insensitively, consume only equality. High EAX bits are not the result.
bool match_native_scene_keyword_00467cc0(void*,const char*,NativeSceneTokenizerContext&,NativeSettingsLoaderCalls&);
struct NativeSettingsLoaderPreimages {
    std::array<std::byte,0x838> tokenizer;
    std::array<std::uint32_t,4> desktop_rectangle;
    std::array<std::byte,0x400> registry_buffer;
    std::uint32_t registry_key,registry_type,catalog_list_word;
};
struct NativeSettingsLoaderContext {
    NativeLanguageCatalogProducerContext& catalog;
    NativeSettingsTextContext& text;
    NativeSettingsLanguageContext& language;
    NativeSceneTokenValueContext& values;
    NativeSettingsChoiceCalls& choices;
    NativeSettingsLoaderCalls& calls;
    void* const volatile& actual_renderer_00f8d394;
    void* actual_resolution_header_00f8895c;
    void* actual_antialias_header_00f88968;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    NativeSettingsDecrement const volatile& decrement_import_00ce2220;
    const NativeSettingsLoaderPreimages& preimages;
    // Language, Fullscreen, HiResShadow, NoLOD, Resolution, VSync,
    // ShaderModel, Antialias, Clouds, Foliage, Shadow, Reflection,
    // TextureDetail, ObjectDetail, SoundEnabled, Firewall.
    std::array<const char*,16> keywords;
    // English, German, Spanish, French, Italian (LCIDs default,407,40A,40C,410).
    std::array<const char*,5> registry_languages;
    const char* read_mode_00d15f28;
    const char* unknown_format_00d15e60;
    const char* desktop_format_00d15e4c;
    const char* registry_path_00d15e24;
    const char* registry_value_00d15e18;
};
struct NativeSettingsLoaderOperation final {
    using Phase=NativeSettingsTextPhase;
    Phase phase{Phase::fresh};void* owner{};void* file{};void* backing{};void* stream{};
    alignas(4) std::byte path_header[8];
    alignas(4) std::array<std::byte,0x838> tokenizer;
    std::array<std::uint32_t,4> desktop_rectangle;
    alignas(4) std::array<std::byte,0x400> registry_buffer;
    std::uint32_t registry_key{},registry_type{},registry_bytes{},native_site{};
    std::int32_t file_length{},unwind_state{-1};std::uint8_t ignored_value_success{};
    bool path_live{},file_live{},backing_live{},stream_live{},tokenizer_live{},registry_live{};
    std::optional<NativeLanguageCatalogProducerOperation> catalog;
    std::optional<NativeSettingsPathOperation> path;
    std::optional<NativeSceneTokenizerOperation> tokenizer_operation;
    std::optional<NativeSettingsLanguageOperation> language;
    std::optional<NativeSettingsTextOperation> writer;
    explicit NativeSettingsLoaderOperation(const NativeSettingsLoaderPreimages&) noexcept;
    ~NativeSettingsLoaderOperation();
    void acknowledge_diagnostic_cleanup() noexcept;
    NativeSettingsLoaderOperation(const NativeSettingsLoaderOperation&)=delete;
    NativeSettingsLoaderOperation& operator=(const NativeSettingsLoaderOperation&)=delete;
};
// Complete normal 8D8190 over actual BCh owner and native raw publications.
// All contexts must share canonical pool, catalog and retained-memory domains.
// File branch preserves ignored read results and backing/stream/tokenizer release
// order. Missing file uses desktop/registry, writes settings BEFORE renderer tail.
// SoundEnabled consumes no value; duplicate resolution/sample matches keep the
// last index. Empty AA tables and invalid negative indices are not repaired.
// Retained source failure is explicit; caller resolves ownership before ack.
// New C++ interface: no original calling ABI, FH3/SEH or private-stack alias claim.
void load_native_game_settings_008d8190(void*,NativeSettingsLoaderContext&,NativeSettingsLoaderOperation&);
} // namespace bsp
