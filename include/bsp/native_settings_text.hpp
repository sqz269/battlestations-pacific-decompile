#pragma once
#include "bsp/native_string.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace bsp {
class ActualNativeStringPoolStorage;
struct NativeSettingsTextCalls {
    virtual ~NativeSettingsTextCalls()=default;
    virtual int personal_folder_00ce22f0(char* buffer_260); // HWND0,CSIDL5,create1
    virtual int create_directory_00ce227c(const char*); // null security attributes
    virtual void* open_00bf838e(const char*,const char*);
    virtual std::uint32_t write_00bfb08b(const void*,std::uint32_t size,std::uint32_t count,void* file);
    virtual int close_00bf8081(void*);
};
enum class NativeSettingsTextPhase {fresh,running,complete,failed,diagnostic_retired};
struct NativeSettingsPathContext {
    NativeStringRawPoolContext& strings;
    NativeSettingsTextCalls& calls;
    const char* directory_suffix_00d15af0; // backslash Battlestations-Pacific
    const char* options_suffix_00d15ae0; // backslash options.txt
    const char* empty_00f88a3c;
};
struct NativeSettingsPathOperation final {
    NativeSettingsTextPhase phase{NativeSettingsTextPhase::fresh};
    std::array<char,260> personal_buffer;
    alignas(4) std::byte directory[8],temporary[8];
    void* output{};void* captured_temporary{};
    std::uint32_t captured_length{},native_site{};
    std::int32_t unwind_state{};
    bool directory_live{},temporary_live{},output_constructed{};
    explicit NativeSettingsPathOperation(const std::array<char,260>& preimage) noexcept:personal_buffer(preimage){}
    ~NativeSettingsPathOperation();
    void acknowledge_diagnostic_cleanup() noexcept;
    NativeSettingsPathOperation(const NativeSettingsPathOperation&)=delete;
    NativeSettingsPathOperation& operator=(const NativeSettingsPathOperation&)=delete;
};
// 8D5150: stack output header, EAX output, RET4; ECX unused. Ignore both OS
// return values. Preserve captured root/suffix releases, current directory
// release, concat output ownership, and wrapped inlined suffix append.
void* build_native_settings_path_008d5150(void*,NativeSettingsPathContext&,NativeSettingsPathOperation&);
struct NativeSettingsBuilderCopyOperation final {
    NativeSettingsTextPhase phase{NativeSettingsTextPhase::fresh};
    void* output{};const void* source{};
    std::uint32_t completed_members{},native_site{};
    std::int32_t unwind_state{-1};
    ~NativeSettingsBuilderCopyOperation();
    void acknowledge_diagnostic_cleanup() noexcept;
    NativeSettingsBuilderCopyOperation()=default;
    NativeSettingsBuilderCopyOperation(const NativeSettingsBuilderCopyOperation&)=delete;
    NativeSettingsBuilderCopyOperation& operator=(const NativeSettingsBuilderCopyOperation&)=delete;
};
// 8D5340: ECX actual18h destination, stack source, EAX destination, RET4.
// Zero/copy three headers in order. Identity abandons previous allocations.
// Check CURRENT source length after resize and copy CURRENT destination length.
void* copy_native_settings_builder_008d5340(void*,const void*,NativeStringRawPoolContext&,NativeSettingsBuilderCopyOperation&);
struct NativeSettingsTextContext {
    NativeSettingsPathContext& path;
    ActualNativeStringPoolStorage& strings;
    void* const volatile& actual_catalog_00f88974;
    // Same pool cells as path.strings. Labels are Language, Fullscreen,
    // Resolution, Vsync, ShaderModel, Antialias, Clouds, Foliage, Shadow,
    // Reflection, TextureDetail, ObjectDetail, SoundEnabled, Firewall,
    // HardwareReported, each including its native trailing space.
    std::array<const char*,15> labels;
    const char* newline_00ce4390;
    const char* space_00ce3a90;
    const char* write_mode_00d15b38;
    const char* null_integer_format_01090ab4;
    const std::array<char,260>& personal_buffer_preimage;
};
struct NativeSettingsTextOperation final {
    NativeSettingsTextPhase phase{NativeSettingsTextPhase::fresh};
    void* owner{};void* file{};
    alignas(4) std::byte builder[24],copy[24],path_header[8];
    const char* captured_language{};
    std::array<std::int32_t,15> captured_values{};
    std::uint32_t native_site{},append_calls{};
    std::int32_t unwind_state{-1};
    bool builder_live{},copy_live{},path_live{},file_live{};
    std::optional<NativeSettingsBuilderCopyOperation> copy_operation;
    std::optional<NativeSettingsPathOperation> path_operation;
    ~NativeSettingsTextOperation();
    void acknowledge_diagnostic_cleanup() noexcept;
    NativeSettingsTextOperation()=default;
    NativeSettingsTextOperation(const NativeSettingsTextOperation&)=delete;
    NativeSettingsTextOperation& operator=(const NativeSettingsTextOperation&)=delete;
};
// Complete 8D6170, native ECX actual BCh settings, RET. Construct builder,
// capture language and ALL scalar arguments before the first append; preserve
// raw byte values rather than converting them to bool. Clone builder, destroy
// original, build path, fopen wt, optional fwrite/fclose, release path and copy.
// Explicit retained source failure state; caller resolves ownership before ack.
// Existing builder/host CRT contracts apply; no original ABI/FH3/SEH claim.
void write_native_settings_text_008d6170(void*,NativeSettingsTextContext&,NativeSettingsTextOperation&);
} // namespace bsp
