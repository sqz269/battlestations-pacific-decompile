// Bounded post-8383FE differential, not a whole-function/interpolation proof.
// Pattern reused from cc16_dyn_ring_native_fixture: pinned PE, private payload,
// exact operand relocations, immutable postimages and scoped thread FP state.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#include <xmmintrin.h>
#include "bsp/gameplay_settings_tail.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#pragma comment(lib, "bcrypt.lib")

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This isolated original-machine fixture requires MSVC Win32.
#endif

namespace {
using U32 = std::uint32_t;
using Byte = unsigned char;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::size_t) == sizeof(ULONG));
constexpr char kSource[] = "9434d4a37f2a3c4528d826968460a982a4213e53";
constexpr char kBinarySha[] = "b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6";
constexpr char kCodeSha[] = "95684f4f4b5bfbec18a19e78354dbb397d8e48bfe9e24e232f2fd5a72c538c6e";
constexpr char kDataSha[] = "3fef94215ebdf41133b4da57c3fb4f1272eb92bf642a978ccf2538ee26993dfb";
struct Span { U32 address, bytes, offset; };
constexpr Span kCodeSpans[]{{0x00838402u,293u,0u},{0x00bf7420u,28u,304u},{0x00bf7456u,117u,336u}};
constexpr Span kDataSpans[]{{0x00d7a24cu,4u,0u},{0x00d7a3a0u,8u,8u},
    {0x00d0a198u,8u,16u},{0x00ce3dc0u,8u,24u}};
enum class Kind { data, mode, rel32, rel8 };
struct Patch { U32 offset, bytes, target, expected; Kind kind; };
constexpr Patch kPatches[]{
    {8,4,0,0x00d7a24c,Kind::data}, {47,4,8,0x00d7a3a0,Kind::data},
    {99,4,16,0x00d0a198,Kind::data}, {106,4,304,0x003befb0,Kind::rel32},
    {133,4,24,0x00ce3dc0,Kind::data}, {276,4,0,0x00d7a24c,Kind::data},
    {306,4,0,0x0109eea4,Kind::mode}, {312,1,336,0x2d,Kind::rel8}
};
[[noreturn]] void fail(const char* reason) { throw std::runtime_error(reason); }
void require(bool value, const char* reason) { if (!value) fail(reason); }

std::string sha256(const void* input, std::size_t bytes) {
    BCRYPT_ALG_HANDLE algorithm{}; BCRYPT_HASH_HANDLE hash{};
    DWORD object_bytes{}, returned{}; std::array<Byte,32> digest{};
    require(BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0,
        "SHA256 provider failed");
    if (BCryptGetProperty(algorithm,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&object_bytes),
        sizeof(object_bytes),&returned,0)<0) {
        BCryptCloseAlgorithmProvider(algorithm,0); fail("SHA256 object query failed");
    }
    std::vector<Byte> object(object_bytes);
    const bool ok=BCryptCreateHash(algorithm,&hash,object.data(),object_bytes,nullptr,0,0)>=0 &&
        BCryptHashData(hash,const_cast<PUCHAR>(static_cast<const Byte*>(input)),
            static_cast<ULONG>(bytes),0)>=0 &&
        BCryptFinishHash(hash,digest.data(),static_cast<ULONG>(digest.size()),0)>=0;
    if (hash) BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(algorithm,0);
    require(ok,"SHA256 failed");
    constexpr char hex[]="0123456789abcdef"; std::string result;
    for (Byte b:digest) { result+=hex[b>>4]; result+=hex[b&15]; }
    return result;
}

class Reference final {
public:
    explicit Reference(const wchar_t* path) {
        std::ifstream file(std::filesystem::path(path),std::ios::binary);
        require(static_cast<bool>(file),"cannot read installed PE");
        const std::vector<Byte> image((std::istreambuf_iterator<char>(file)),{});
        require(sha256(image.data(),image.size())==kBinarySha,"installed PE hash changed");
        IMAGE_DOS_HEADER dos{}; std::memcpy(&dos,image.data(),sizeof(dos));
        IMAGE_NT_HEADERS32 nt{}; std::memcpy(&nt,image.data()+dos.e_lfanew,sizeof(nt));
        require(dos.e_magic==IMAGE_DOS_SIGNATURE && nt.Signature==IMAGE_NT_SIGNATURE &&
            nt.FileHeader.Machine==IMAGE_FILE_MACHINE_I386 &&
            nt.OptionalHeader.Magic==IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
            nt.OptionalHeader.ImageBase==0x00400000u,"unexpected PE target");
        const std::size_t section_offset=dos.e_lfanew+sizeof(DWORD)+sizeof(IMAGE_FILE_HEADER)+
            nt.FileHeader.SizeOfOptionalHeader;
        const auto copy_span=[&](const Span& span,std::vector<Byte>& output) {
            const U32 rva=span.address-nt.OptionalHeader.ImageBase;
            for (WORD i=0;i<nt.FileHeader.NumberOfSections;++i) {
                IMAGE_SECTION_HEADER section{};
                std::memcpy(&section,image.data()+section_offset+i*sizeof(section),sizeof(section));
                if (rva>=section.VirtualAddress &&
                    rva-section.VirtualAddress+span.bytes<=section.SizeOfRawData) {
                    const std::size_t source=section.PointerToRawData+rva-section.VirtualAddress;
                    require(source+span.bytes<=image.size() && span.offset+span.bytes<=output.size(),
                        "span outside image/payload");
                    std::memcpy(output.data()+span.offset,image.data()+source,span.bytes); return;
                }
            }
            fail("span is not disk backed");
        };
        std::vector<Byte> code(464,0xcc),data(32,0xcc);
        for (const auto& span:kCodeSpans) copy_span(span,code);
        for (const auto& span:kDataSpans) copy_span(span,data);
        require(sha256(code.data(),code.size())==kCodeSha,"packed code hash changed");
        require(sha256(data.data(),data.size())==kDataSha,"constant hash changed");
        code_=static_cast<Byte*>(VirtualAlloc(nullptr,code.size(),MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
        data_=static_cast<Byte*>(VirtualAlloc(nullptr,data.size(),MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
        require(code_ && data_,"private allocation failed");
        std::memcpy(code_,code.data(),code.size()); std::memcpy(data_,data.data(),data.size());
        std::array<bool,464> allowed{};
        for (const auto& patch:kPatches) {
            U32 old{}; std::memcpy(&old,code_+patch.offset,patch.bytes);
            require(old==patch.expected,"relocation preimage mismatch");
            U32 value{};
            if (patch.kind==Kind::data) value=reinterpret_cast<U32>(data_)+patch.target;
            if (patch.kind==Kind::mode) value=reinterpret_cast<U32>(&mode_);
            if (patch.kind==Kind::rel32) value=patch.target-(patch.offset+4u);
            if (patch.kind==Kind::rel8) {
                const auto delta=static_cast<std::int32_t>(patch.target)-
                    static_cast<std::int32_t>(patch.offset+1u);
                require(delta>=-128 && delta<=127,"rel8 out of range");
                value=static_cast<Byte>(delta);
            }
            std::memcpy(code_+patch.offset,&value,patch.bytes);
            for (U32 i=0;i<patch.bytes;++i) { require(!allowed[patch.offset+i],"overlapping patch"); allowed[patch.offset+i]=true; }
        }
        for (U32 i=0;i<code.size();++i) require(allowed[i] || code_[i]==code[i],"unlisted code edit");
        sealed_=sha256(code_,464);
        DWORD old{};
        require(VirtualProtect(code_,464,PAGE_EXECUTE_READ,&old)!=0,"code protection failed");
        require(VirtualProtect(data_,32,PAGE_READONLY,&old)!=0,"constant protection failed");
        require(FlushInstructionCache(GetCurrentProcess(),code_,464)!=0,"cache flush failed");
        verify();
    }
    ~Reference() { if (code_) VirtualFree(code_,0,MEM_RELEASE); if (data_) VirtualFree(data_,0,MEM_RELEASE); }
    Reference(const Reference&)=delete; Reference& operator=(const Reference&)=delete;
    void verify() const {
        require(sha256(code_,464)==sealed_,"private code postimage changed");
        require(sha256(data_,32)==kDataSha,"private constants changed");
    }
    void* window() const { return code_; }
    void* converter() const { return code_+304; }
    const double* negative_ten() const { return reinterpret_cast<const double*>(data_+16); }
    volatile U32& mode() { return mode_; }
    const std::string& sealed() const { return sealed_; }
private:
    Byte* code_{}; Byte* data_{}; volatile U32 mode_{}; std::string sealed_;
};

// Frame-only tail adapter, not a returning stub. At original838402 entry:
// [ESP]=savedESI,+4/+8 locals,+C actual return,+10 stored weight,+14 fraction.
// Original POP ESI; ADD ESP,8; RET8 performs the real return and stack cleanup.
__declspec(naked) float __fastcall invoke_window(void*,const bsp::WeaponHitAccuracyProfile*,float,float) {
    __asm {
        sub esp, 8
        push esi
        mov esi, edx
        jmp ecx
    }
}

struct alignas(16) FxImage { std::array<Byte,512> bytes; };
void save_fp(FxImage& image) { auto* p=image.bytes.data(); __asm { mov eax,p } __asm { fxsave [eax] } }
struct FpObserved { std::uint16_t cw{},exceptions{},top{}; U32 mxcsr{}; };
FpObserved observe_fp() {
    FxImage image{}; save_fp(image); FpObserved result; std::uint16_t status{};
    std::memcpy(&result.cw,image.bytes.data(),2); std::memcpy(&status,image.bytes.data()+2,2);
    std::memcpy(&result.mxcsr,image.bytes.data()+24,4);
    result.exceptions=static_cast<std::uint16_t>(status&0x3f);
    result.top=static_cast<std::uint16_t>(status&0x3800); return result;
}
class FpScope final {
public:
    explicit FpScope(std::uint16_t control) {
        save_fp(saved_);
        __asm { fninit } __asm { fldcw control }
        _mm_setcsr(0x1f80u);
    }
    ~FpScope() { auto* p=saved_.bytes.data(); __asm { mov eax,p } __asm { fxrstor [eax] } }
private:
    FxImage saved_{};
};
bool same_fp(const FpObserved& a,const FpObserved& b) {
    return a.cw==b.cw && a.exceptions==b.exceptions && a.top==b.top && a.mxcsr==b.mxcsr;
}
U32 bits(float value) { U32 result{}; std::memcpy(&result,&value,4); return result; }
float from_bits(U32 value) { float result{}; std::memcpy(&result,&value,4); return result; }
struct FloatResult { U32 bits{}; FpObserved fp; };
FloatResult window_result(Reference& ref,const bsp::WeaponHitAccuracyProfile& profile,
    float fraction,std::uint16_t cw,bool source) {
    FpScope fp(cw);
    const float value=source ? bsp::weapon_hit_accuracy_sample_008383d0(
        profile,profile.small_target_size,fraction,ref.mode()) :
        invoke_window(ref.window(),&profile,1.0f,fraction);
    const U32 word=bits(value); const auto observed=observe_fp(); return {word,observed};
}

struct ConversionResult { U32 eax{},surviving[3]{}; FpObserved fp; };
void invoke_conversion(void* entry,const volatile U32* mode,float fraction,
    const double* negative_ten,ConversionResult* output) {
    __asm {
        mov esi, output
        fld1 // lower size weight1
        fld1 // native live1
        fld dword ptr fraction
        mov eax, negative_ten
        fld qword ptr [eax]
        fmul st(0), st(1) // unspilled same native FTOL input
        mov ecx, mode
        mov eax, entry
        call eax
        mov dword ptr [esi], eax
        fstp dword ptr [esi + 4]
        fstp dword ptr [esi + 8]
        fstp dword ptr [esi + 12]
    }
}
ConversionResult conversion_result(Reference& ref,float fraction,std::uint16_t cw,bool source) {
    FpScope fp(cw); ConversionResult result;
    void* entry=source ? reinterpret_cast<void*>(&bsp::native_crt_truncate_st0_00bf7420) : ref.converter();
    invoke_conversion(entry,&ref.mode(),fraction,ref.negative_ten(),&result);
    result.fp=observe_fp(); return result;
}
void print_fp(const FpObserved& fp) {
    std::printf("{\"cw\":%u,\"exceptions\":%u,\"top\":%u,\"mxcsr\":%u}",
        static_cast<unsigned>(fp.cw),static_cast<unsigned>(fp.exceptions),
        static_cast<unsigned>(fp.top),fp.mxcsr);
}

int run_child(const wchar_t* path) {
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
    const auto initial=observe_fp(); Reference ref(path);
    bsp::WeaponHitAccuracyProfile profile;
    profile.small_target_size=100.0f; profile.large_target_size=200.0f;
    for (int i=0;i<10;++i) {
        profile.small_target_accuracy[i]=(i&1) ? 0.75f : 0.25f;
        profile.large_target_accuracy[i]=(i&1) ? 0.125f : 0.875f;
    }
    const auto original_profile=profile;
    unsigned cases=0,legacy_differences=0;
    std::printf("{\"source\":\"%s\",\"scope\":\"original838402 post-spill window; endpoint weight1 only; synthetic relocated cell\",\"code_sha256\":\"%s\",\"sealed_code_sha256\":\"%s\",\"watchdog_ms\":15000,\"cases\":[",
        kSource,kCodeSha,ref.sealed().c_str());
    for (const std::uint16_t cw:{std::uint16_t{0x007f},std::uint16_t{0x027f}}) {
        for (const U32 input:{0x3f333333u,0x7fc12345u}) {
            for (const U32 mode:{0u,1u}) {
                ref.mode()=mode; const float fraction=from_bits(input);
                const auto native=window_result(ref,profile,fraction,cw,false); ref.verify();
                const auto source=window_result(ref,profile,fraction,cw,true); ref.verify();
                require(native.bits==source.bits,"post-spill result differs");
                require(same_fp(native.fp,source.fp),"post-spill tested FP state differs");
                require(native.fp.cw==cw && native.fp.top==0,"post-spill controls/stack changed");
                const auto native_conversion=conversion_result(ref,fraction,cw,false); ref.verify();
                const auto source_conversion=conversion_result(ref,fraction,cw,true); ref.verify();
                require(native_conversion.eax==source_conversion.eax &&
                    std::memcmp(native_conversion.surviving,source_conversion.surviving,12)==0 &&
                    same_fp(native_conversion.fp,source_conversion.fp),"conversion boundary differs");
                require(native_conversion.fp.cw==cw && native_conversion.fp.top==0,
                    "conversion controls/stack changed");
                U32 legacy{};
                { FpScope fp(cw); legacy=bits(bsp::weapon_hit_accuracy_sample_008383d0(
                    profile,profile.small_target_size,fraction)); }
                const bool old_diff=legacy!=native.bits; legacy_differences+=old_diff ? 1u : 0u;
                require(old_diff==(input==0x7fc12345u || cw==0x027fu),"negative control did not discriminate");
                require(ref.mode()==mode && std::memcmp(&profile,&original_profile,sizeof(profile))==0,
                    "synthetic cell/profile changed");
                if (cases++) std::printf(",");
                std::printf("{\"cw\":%u,\"mode\":%u,\"input\":\"%08x\",\"native\":\"%08x\",\"source\":\"%08x\",\"legacy\":\"%08x\",\"conversion_eax\":\"%08x\",\"fp\":",
                    static_cast<unsigned>(cw),mode,input,native.bits,source.bits,legacy,native_conversion.eax);
                print_fp(native.fp); std::printf("}");
            }
        }
    }
    ref.verify(); const auto restored=observe_fp();
    require(restored.cw==initial.cw && restored.mxcsr==initial.mxcsr,"thread controls not restored");
    std::printf("],\"post_window_pairs\":%u,\"conversion_pairs\":%u,\"legacy_differences\":%u,\"controls_restored\":true,\"postimages_unchanged\":true}\n",
        cases,cases,legacy_differences);
    return 0;
}

int watch_child(const wchar_t* original) {
    std::array<wchar_t,MAX_PATH> executable{};
    require(GetModuleFileNameW(nullptr,executable.data(),static_cast<DWORD>(executable.size()))!=0,
        "cannot identify own fixture");
    require(std::wcschr(original,L'\"')==nullptr,"invalid original path");
    std::wstring command=L"\""+std::wstring(executable.data())+L"\" --child \""+original+L"\"";
    std::vector<wchar_t> mutable_command(command.begin(),command.end()); mutable_command.push_back(0);
    STARTUPINFOW startup{}; startup.cb=sizeof(startup); startup.dwFlags=STARTF_USESTDHANDLES;
    startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE); startup.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
    startup.hStdError=GetStdHandle(STD_ERROR_HANDLE); PROCESS_INFORMATION child{};
    require(CreateProcessW(executable.data(),mutable_command.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW,
        nullptr,nullptr,&startup,&child)!=0,"cannot start own fixture child");
    CloseHandle(child.hThread);
    const DWORD waited=WaitForSingleObject(child.hProcess,15000);
    if (waited!=WAIT_OBJECT_0) {
        TerminateProcess(child.hProcess,124); WaitForSingleObject(child.hProcess,1000);
        CloseHandle(child.hProcess); fail("own-child watchdog stopped fixture");
    }
    DWORD result{}; const bool read=GetExitCodeProcess(child.hProcess,&result)!=0; CloseHandle(child.hProcess);
    require(read,"cannot read own child status");
    if (result) { std::fprintf(stderr,"fixture child failed: 0x%08lx\n",result); return 2; }
    return 0;
}
} // namespace

int wmain(int argc,wchar_t** argv) {
    try {
        if (argc==3 && std::wcscmp(argv[1],L"--child")==0) return run_child(argv[2]);
        require(argc==2,"pass the supported installed PE path");
        return watch_child(argv[1]);
    } catch (const std::exception& error) {
        std::fprintf(stderr,"%s\n",error.what()); return 2;
    }
}
