#define _CRT_SECURE_NO_WARNINGS
#include "bsp/native_particle_type_property.hpp"
#include "bsp/native_particle_type_resources.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_pooled_text.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle properties require MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+n);
}
const void* at(const void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p)+n);
}
template<class T> T read(const void* p, std::uint32_t n=0) noexcept {
    T v; std::memcpy(&v,at(p,n),sizeof v); return v;
}
template<class T> void put(void* p, std::uint32_t n, T v) noexcept {
    std::memcpy(at(p,n),&v,sizeof v);
}
struct ScopedString {
    NativeString value;
    NativeStringStorage& storage;
    explicit ScopedString(NativeStringStorage& s):storage(s) {}
    ~ScopedString() { destroy_native_string_header_0041dd20(&value,storage); }
};
struct ScopedText {
    NativePooledTextStorage value{};
    NativeStringStorage& storage;
    explicit ScopedText(NativeStringStorage& s):storage(s) {}
    ~ScopedText() { destroy_native_pooled_text_00aee2a0(&value,storage); }
};
const char* text(const NativeString& s, const char* fallback) noexcept {
    return s.data()?s.data():fallback;
}
std::int32_t last_index(const char* s) noexcept {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(std::strlen(s))-1u);
}
void copy_record(void* destination, const void* source) noexcept {
    __asm {
        mov edi,destination
        mov esi,source
        mov ecx,7
        rep movsd
    }
}
void copy_query_suffix(NativeString& query, std::uint32_t start,
    NativeStringStorage& storage) {
    ScopedString suffix(storage);
    construct_native_string_substring_00469840(&query,&suffix.value,start,0x7fffffffu,storage);
    copy_native_string_header_00be0a30_fragment(&query,storage,&suffix.value);
}
std::int32_t find_substring(const void* s, const void* needle, std::uint32_t start) {
    const auto data=read<const char*>(s,4), pattern=read<const char*>(needle,4);
    if (!data || !pattern) return -1;
    if (static_cast<std::int32_t>(start)<0) start=0;
    else if (read<std::uint32_t>(s)<start) return -1;
    const char* match=std::strstr(data+start,pattern);
    return match?static_cast<std::int32_t>(match-data):-1;
}
void convert_sse_record(void* record, const void* uv,
    const NativeD3dx9Float32To16Import& half) {
    for (std::uint32_t n=0;n<16;n+=4) put(record,n,read<std::uint32_t>(uv,n));
    half.convert(static_cast<std::uint16_t*>(at(record,16)),
        static_cast<const float*>(record),4);
}
} // namespace

bool has_native_particle_frame_suffix_00af3750(const char* filename) noexcept {
    std::uint32_t zeros=0;
    for (auto i=last_index(filename);i>0 && filename[i]=='0';--i)
        if (++zeros==3) return true;
    return false;
}

void resize_native_particle_string_fill_0043bbf0(void* p,
    std::uint32_t length, std::int8_t fill, NativeStringStorage& storage) {
    const auto old=read<std::uint32_t>(p);
    resize_native_string_header_0041dd40(p,storage,length,true);
    const auto current=read<std::uint32_t>(p);
    if (old<current) std::memset(at(read<void*>(p,4),old),fill,current-old);
}

void* construct_native_particle_texture_stem_00af37d0(void* output,
    const char* filename, NativeStringStorage& storage) {
    construct_native_string_cstring_0041e870(output,filename,storage);
    try {
        std::int32_t dot;
        {
            ScopedString needle(storage);
            needle.value.assign_0041e870(storage,".");
            dot=reverse_find_native_string_header_00467cf0(output,&needle.value,0x7fffffffu);
        }
        if (dot!=-1) resize_native_particle_string_fill_0043bbf0(output,
            static_cast<std::uint32_t>(dot),0x20,storage);
    } catch (...) {
        destroy_native_string_header_0041dd20(output,storage); throw;
    }
    return output;
}

void* construct_native_particle_texture_extension_00af38b0(void* output,
    const char* filename, NativeStringStorage& storage) {
    std::int32_t dot=0;
    for (auto i=last_index(filename);i>0;--i)
        if (filename[i]=='.') { dot=i; break; }
    char buffer[256];
    std::strcpy(buffer,filename+dot);
    return construct_native_string_cstring_0041e870(output,buffer,storage);
}

void* construct_native_particle_texture_prefix_00af3960(void* output,
    const char* stem, NativeStringStorage& storage) {
    char buffer[256]; std::strcpy(buffer,stem);
    std::uint32_t zeros=0;
    for (auto i=last_index(buffer);i>0;--i)
        if (buffer[i]=='0' && ++zeros==3) { buffer[i]='\0'; break; }
    return construct_native_string_cstring_0041e870(output,buffer,storage);
}

void replace_native_particle_string_substrings_004cad40(void* value,
    const void* search, const void* replacement, std::uint32_t count,
    NativeStringStorage& storage) {
    auto position=find_substring(value,search,0);
    while (position!=-1) {
        {
            ScopedString suffix(storage), prefix(storage), head(storage), joined(storage);
            construct_native_string_substring_00469840(value,&suffix.value,
                read<std::uint32_t>(search)+static_cast<std::uint32_t>(position),
                0x7fffffffu,storage);
            construct_native_string_substring_00469840(value,&prefix.value,0,
                static_cast<std::uint32_t>(position),storage);
            concatenate_native_string_headers_004261a0(&prefix.value,&head.value,
                replacement,storage);
            concatenate_native_string_headers_004261a0(&head.value,&joined.value,
                &suffix.value,storage);
            copy_native_string_header_00be0a30_fragment(value,storage,&joined.value);
        }
        position=find_substring(value,search,
            read<std::uint32_t>(replacement)+static_cast<std::uint32_t>(position));
        if (--count==0) return;
    }
}

bool matches_native_particle_string_at_0043e9a0(const void* candidate,
    const void* pattern, std::uint32_t start, const char* null_pattern) noexcept {
    const char* wanted=read<const char*>(pattern,4);
    if (!wanted) wanted=null_pattern;
    const char* value=read<const char*>(candidate,4);
    if (!value || !wanted || start>read<std::uint32_t>(candidate)) return false;
    value+=start;
    while (*value && *wanted && *value==*wanted) { ++value; ++wanted; }
    return *wanted=='\0';
}

bool matches_native_particle_atlas_item_00aee0f0(const void* item,
    const void* query, const char* null_pattern) {
    const void* name=at(item,12);
    if (equal_native_string_headers_00435c40(query,name)) return true;
    const auto query_length=read<std::uint32_t>(query);
    const auto item_length=read<std::uint32_t>(name);
    if (query_length>=item_length) return false;
    const auto offset=item_length-query_length;
    return read<const char*>(item,16)[offset-1u]=='/' &&
        matches_native_particle_string_at_0043e9a0(name,query,offset,null_pattern);
}

void* find_native_particle_atlas_item_00aefb20(void* manager,
    const char* filename, NativeStringStorage& storage, const char* null_pattern) {
    if (!filename || !*filename) return nullptr;
    ScopedString query(storage);
    construct_native_particle_texture_stem_00af37d0(&query.value,filename,storage);
    {
        ScopedString slash(storage), backslash(storage);
        slash.value.assign_0041e870(storage,"/");
        backslash.value.assign_0041e870(storage,"\\");
        replace_native_particle_string_substrings_004cad40(&query.value,
            &backslash.value,&slash.value,0x7fffffffu,storage);
    }
    // Native dereferences query data even after a normalization made it empty.
    while (*query.value.data()=='/') copy_query_suffix(query.value,1,storage);
    lowercase_native_string_header_004bcc00(&query.value);
    for (std::uint32_t i=0;static_cast<std::int32_t>(i)<read<std::int32_t>(manager,8);++i) {
        void* item=read<void*>(read<void*>(manager,4),i*4u);
        if (matches_native_particle_atlas_item_00aee0f0(item,&query.value,null_pattern))
            return read<void*>(read<void*>(manager,4),i*4u);
    }
    std::int32_t slash;
    {
        ScopedString needle(storage); needle.value.assign_0041e870(storage,"/");
        slash=reverse_find_native_string_header_00467cf0(&query.value,&needle.value,0x7fffffffu);
    }
    if (slash>=0) {
        copy_query_suffix(query.value,static_cast<std::uint32_t>(slash)+1u,storage);
        for (std::uint32_t i=0;static_cast<std::int32_t>(i)<read<std::int32_t>(manager,8);++i) {
            void* item=read<void*>(read<void*>(manager,4),i*4u);
            if (matches_native_particle_atlas_item_00aee0f0(item,&query.value,null_pattern))
                return read<void*>(read<void*>(manager,4),i*4u);
        }
    }
    return nullptr;
}

std::int32_t count_native_particle_texture_frames_00af3a20(const char* filename,
    NativeParticleTypePropertyBindings& a) {
    auto& storage=a.base.owners.strings;
    ScopedString stem(storage);
    construct_native_particle_texture_stem_00af37d0(&stem.value,filename,storage);
    if (!has_native_particle_frame_suffix_00af3750(text(stem.value,a.empty_stem_00f8c2c1))) return 0;
    ScopedString prefix(storage);
    construct_native_particle_texture_prefix_00af3960(&prefix.value,text(stem.value,a.empty_stem_00f8c2c1),storage);
    std::uint32_t index=0;
    char buffer[32];
    for (;;) {
        std::sprintf(buffer,"%s%03d",text(prefix.value,a.empty_stem_00f8c2c1),static_cast<std::int32_t>(index));
        if (!find_native_particle_atlas_item_00aefb20(a.actual_atlas_manager_00f8c26c,
                buffer,storage,a.null_pattern_00e17bf0)) return static_cast<std::int32_t>(index);
        ++index;
    }
}

void* construct_native_particle_texture_frame_name_00af3b50(void* output,
    const char* filename, std::int32_t index, NativeParticleTypePropertyBindings& a) {
    auto& storage=a.base.owners.strings;
    ScopedString stem(storage), prefix(storage), extension(storage), number(storage),
        head(storage), joined(storage);
    construct_native_particle_texture_stem_00af37d0(&stem.value,filename,storage);
    construct_native_particle_texture_prefix_00af3960(&prefix.value,text(stem.value,a.empty_stem_00f8c2c1),storage);
    construct_native_particle_texture_extension_00af38b0(&extension.value,filename,storage);
    char buffer[32]; std::sprintf(buffer,"%03d",index);
    number.value.assign_0041e870(storage,buffer);
    concatenate_native_string_headers_004261a0(&prefix.value,&head.value,&number.value,storage);
    concatenate_native_string_headers_004261a0(&head.value,&joined.value,&extension.value,storage);
    return copy_construct_native_string_header_00426060(output,&joined.value,storage);
}

void* construct_native_particle_uv_record_00b00880(void* output,
    const void* uv_min, const void* uv_max, const NativeD3dx9Float32To16Import& half) {
    __asm {
        mov eax,uv_min
        mov edx,output
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax+4]
        fstp dword ptr [edx+4]
        mov eax,uv_max
        fld dword ptr [eax]
        fstp dword ptr [edx+8]
        fld dword ptr [eax+4]
        fstp dword ptr [edx+12]
    }
    half.convert(static_cast<std::uint16_t*>(at(output,16)),
        static_cast<const float*>(output),4);
    return output;
}

void append_native_particle_type_record_00b00ee0(void* descriptor,
    const void* record, NativeParticleTypeBaseBindings& base) {
    const auto capacity=read<std::uint32_t>(descriptor,8);
    if (read<std::uint32_t>(descriptor,4)==capacity) {
        auto next=static_cast<std::int32_t>(capacity*2u);
        if (next<=1) next=1;
        reserve_native_particle_type_records_00b00c20(descriptor,next,base);
    }
    void* destination=at(read<void*>(descriptor),read<std::uint32_t>(descriptor,4)*0x1cu);
    if (destination) copy_record(destination,record);
    put(descriptor,4,read<std::uint32_t>(descriptor,4)+1u);
}

void clear_native_particle_type_records_00b00f30(void* descriptor,
    NativeParticleTypeBaseBindings& base) {
    if (read<std::int32_t>(descriptor,8)<0)
        reserve_native_particle_type_records_00b00c20(descriptor,0,base);
    while (read<std::int32_t>(descriptor,4)>0)
        put(descriptor,4,read<std::uint32_t>(descriptor,4)-1u);
    put<std::uint32_t>(descriptor,4,0);
}

bool load_native_particle_type_texture_00b01350(void* definition,
    const char* filename, NativeParticleTypePropertyBindings& a) {
    if (!filename || !*filename) return false;
    auto& storage=a.base.owners.strings;
    void* descriptor=at(definition,0x68);
    clear_native_particle_type_records_00b00f30(descriptor,a.base);
    void* item=find_native_particle_atlas_item_00aefb20(
        a.actual_atlas_manager_00f8c26c,filename,storage,a.null_pattern_00e17bf0);
    alignas(4) std::uint32_t first[7]; first[6]=a.first_record_stack_word18;
    if (item) {
        construct_native_particle_uv_record_00b00880(first,at(item,0x14),
            at(item,0x1c),a.base.half_import);
        append_native_particle_type_record_00b00ee0(descriptor,first,a.base);
    } else {
        void* resource;
        {
            ScopedString name(storage); name.value.assign_0041e870(storage,filename);
            void* renderer=a.actual_renderer_00f8d394;
            auto target=read<std::uint32_t>(read<void*>(renderer),0x64);
            resource=a.renderer_virtual64(a.context,renderer,target,&name.value,0);
        }
        first[0]=0; first[1]=0; first[2]=*a.base.one_00d7a24c; first[3]=first[2];
        a.base.half_import.convert(reinterpret_cast<std::uint16_t*>(first+4),
            reinterpret_cast<const float*>(first),4);
        append_native_particle_type_record_00b00ee0(descriptor,first,a.base);
        if (InterlockedDecrement(static_cast<volatile LONG*>(at(resource,4)))==0) {
            const auto target=read<std::uint32_t>(read<void*>(resource));
            a.base.owners.member_virtual00(a.base.owners.context,resource,target);
        }
    }
    const auto count=count_native_particle_texture_frames_00af3a20(filename,a);
    if (count>1) {
        reserve_native_particle_type_records_00b00c20(descriptor,count,a.base);
        for (std::int32_t i=1;i<count;++i) {
            ScopedString name(storage);
            construct_native_particle_texture_frame_name_00af3b50(&name.value,filename,i,a);
            item=find_native_particle_atlas_item_00aefb20(
                a.actual_atlas_manager_00f8c26c,text(name.value,a.empty_texture_name_00f8d37c),storage,a.null_pattern_00e17bf0);
            if (!item) break;
            alignas(4) std::uint32_t later[7]; later[6]=a.later_record_stack_word18;
            convert_sse_record(later,at(item,0x14),a.base.half_import);
            append_native_particle_type_record_00b00ee0(descriptor,later,a.base);
        }
    }
    auto value=static_cast<std::int32_t>(read<std::uint32_t>(definition,0x6c)-1u);
    if (read<std::int32_t>(definition,0x54)<value) value=read<std::int32_t>(definition,0x54);
    put(definition,0x54,value);
    if (read<std::int32_t>(definition,0x50)<=value) value=read<std::int32_t>(definition,0x50);
    put(definition,0x50,value);
    return true;
}

std::int32_t find_native_particle_layer_00af4360(void* owner,
    std::uint32_t length, char* consumed, NativeStringStorage& storage) {
    std::int32_t found=0;
    for (std::uint32_t i=0;static_cast<std::int32_t>(i)<read<std::int32_t>(owner,0x54);++i) {
        void* entry=read<void*>(owner,0x34+i*4u);
        if (read<std::uint32_t>(entry,8)==length &&
                (!length || _stricmp(read<const char*>(entry,12),consumed)==0)) {
            found=static_cast<std::int32_t>(i); break;
        }
    }
    if (consumed) storage.release(consumed,length+1u);
    return found;
}

bool load_native_particle_type_property_00b015c0(void* definition,
    const void* suffix, NativeParticleTypePropertyBindings& a) {
    auto& storage=a.base.owners.strings;
    static const char* names[]={"Texture","Layer","Shader","Stops",
        "RandomRotationDirection","EmitLight","Distort","AnimOnOff",
        "AnimRndStartFrame","AnimRandomPlay","AnimLoop","AnimFade"};
    static const std::uint32_t offsets[]={0,0,0,0x28,0x29,0x64,0x65,0x4c,0x60,0x61,0x62,0x63};
    for (std::uint32_t i=0;i<12;++i) {
        bool matched;
        {
            ScopedText token(storage);
            get_native_pooled_text_token_00aee3c0(suffix,&token.value,0,storage);
            matched=i<5?_stricmp(token.value.data,names[i])==0:
                equal_native_pooled_text_00aedf80(&token.value,names[i]);
        }
        if (!matched) continue;
        ScopedText token(storage);
        get_native_pooled_text_token_00aee3c0(suffix,&token.value,1,storage);
        if (i==0) load_native_particle_type_texture_00b01350(definition,token.value.data,a);
        else if (i==1) {
            ScopedText layer(storage);
            construct_native_pooled_text_00af5660(&layer.value,token.value.data,storage);
            destroy_native_pooled_text_00aee2a0(&token.value,storage);
            NativeString consumed;
            consumed.assign_0041e870(storage,layer.value.data);
            const auto found=find_native_particle_layer_00af4360(read<void*>(definition,0x14),
                consumed.length(),consumed.data(),storage);
            put(definition,0x74,found);
        } else if (i==2) {
            const auto target=read<std::uint32_t>(read<void*>(definition),0x10);
            if (!dispatch_known_native_particle_shader(definition,target,token.value.data,storage))
                a.shader_virtual10(a.context,definition,target,token.value.data);
        } else put<std::uint8_t>(definition,offsets[i],std::atol(token.value.data)>0?1:0);
        return true;
    }
    return false;
}
} // namespace bsp
