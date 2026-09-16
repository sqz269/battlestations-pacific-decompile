#pragma once
#include "bsp/native_resource_parser_singletons.hpp"

namespace bsp {
struct NativeStringRawPoolContext;

// Stable bindings to the application's actual singleton-manager publication
// and the two actual parser publication cells. The referenced cells may change
// value; retain these contexts through the shared singleton-manager drain.
struct NativeResourceExtraParserContexts {
    NativeResourceParserSingletonContext animation_channels;
    NativeResourceParserSingletonContext bone;
};

// AnimationChannels: publication 01090298, primary CFEA38, secondary CFEA34.
void* get_native_animation_channels_parser_00736dd0(NativeResourceParserSingletonContext&);
void* delete_native_animation_channels_parser_00737150(
    void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* delete_native_animation_channels_parser_secondary_00735d90(
    void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* name_native_animation_channels_parser_00b8b050(
    void* actual_parser, void* actual_output_header, NativeStringRawPoolContext&);

// Bone: publication 0109029C, primary CFEA48, secondary CFEA44.
void* get_native_bone_parser_00736ea0(NativeResourceParserSingletonContext&);
void* delete_native_bone_parser_00737190(
    void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* delete_native_bone_parser_secondary_00735dc0(
    void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* name_native_bone_parser_00b8b080(
    void* actual_parser, void* actual_output_header, NativeStringRawPoolContext&);

// Source composition adapter for the two registered +4 secondary profiles.
// Unknown profiles are rejected; the caller selects this adapter only for an
// owner that was admitted under CFEA34 or CFEA44.
void delete_native_resource_extra_registered_owner(std::uintptr_t captured_profile,
    void* popped_secondary, std::uint32_t flags, NativeResourceExtraParserContexts&);

// Extend an existing finite name map with the two actual extra-parser slot4
// targets. All other captured targets are forwarded without rereading a table.
class NativeResourceExtraParserNameCalls final : public NativeResourceParserNameCalls {
public:
    NativeResourceExtraParserNameCalls(NativeStringRawPoolContext& strings,
        NativeResourceParserNameCalls& remaining) noexcept
        : strings_(strings), remaining_(remaining) {}
    void* type_name(std::uintptr_t captured_target, void* actual_parser,
        void* actual_output_header) override;
private:
    NativeStringRawPoolContext& strings_;
    NativeResourceParserNameCalls& remaining_;
};

// New explicit-service C++ interfaces. Profiles are native identity DWORDs,
// not callable C++ vtables. Requires real Win32 sections, source CRT allocation,
// actual singleton registration/string pool and mapped literal data. Native
// FH3/SEH identity, parse slot8 bodies and gameplay remain separate boundaries.
} // namespace bsp
