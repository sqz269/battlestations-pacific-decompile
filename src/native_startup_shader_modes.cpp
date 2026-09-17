#include "bsp/native_startup_shader_modes.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
bool contains(const char* text,const char* needle) noexcept {
    if(!text)return false;
    const auto* found=std::strstr(text,needle);
    if(!found)return false;
    const U offset=static_cast<U>(reinterpret_cast<std::uintptr_t>(found)-
        reinterpret_cast<std::uintptr_t>(text));
    return (offset&0x80000000u)==0;
}
}
NativeStartupShaderModeOperation::~NativeStartupShaderModeOperation() {
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void apply_native_startup_shader_modes_0073d4c2(const char* mode,
    NativeStartupShaderModes& flags,const NativeStartupShaderModeLiterals& literals,
    NativeStringRawPoolContext& strings,NativeStartupShaderModeOperation& a) {
    using Phase=NativeStartupShaderModeOperation::Phase;
    if(a.phase!=Phase::fresh)throw std::logic_error("startup shader mode operation is one-shot");
    a.phase=Phase::running;
    try {
        a.native_site=0x0073d4ce;
        construct_native_string_header_0041e870(&a.temporary,strings,mode);a.temporary_live=true;
        const char* text=a.temporary.data();
        a.native_site=0x0073d4e1;
        flags.load_variants_0108d6f0=contains(text,literals.genshaders_00ce7f7c);
        a.native_site=0x0073d507;
        flags.source_mode_0108d6f1=contains(text,literals.devshaders_00ce7f70);
        a.native_site=0x0073d533;
        destroy_native_string_header_0041dd20(&a.temporary,strings);a.temporary_live=false;++a.completed_copies;
        a.native_site=0x0073d544;
        construct_native_string_header_0041e870(&a.temporary,strings,mode);a.temporary_live=true;
        text=a.temporary.data();
        a.native_site=0x0073d557;
        flags.hires_mode_0108d4ba=contains(text,literals.hiresmode_00ce7f98);
        a.native_site=0x0073d57e;
        flags.reload_resources_0108d4bb=contains(text,literals.reloadresources_00ce7f88);
        a.native_site=0x0073d5a9;
        destroy_native_string_header_0041dd20(&a.temporary,strings);a.temporary_live=false;++a.completed_copies;
        a.native_site=0x0073d5ba;
        construct_native_string_header_0041e870(&a.temporary,strings,mode);a.temporary_live=true;
        text=a.temporary.data();a.native_site=0x0073d5cd;
        if(contains(text,literals.devrr_00ce7f68)) {
            flags.source_mode_0108d6f1=1;flags.reload_resources_0108d4bb=1;
        }
        a.native_site=0x0073d5f8;
        destroy_native_string_header_0041dd20(&a.temporary,strings);a.temporary_live=false;++a.completed_copies;
        a.phase=Phase::complete;
    }catch(...) {a.phase=Phase::failed;throw;}
}
} // namespace bsp
