"""Bounded Win32 x87/FH3-contract probe. Reads the seed; never runs the game.

Extracts the tested C++ functions verbatim, relocates the original timing and
camera instruction slices, and derives cleanup expectations from original FH3
data. The camera's supplied conversion binding exercises the CRT SSE2 branch.
This is source/instruction evidence, not original whole-function ABI or Lua,
application, allocator, native exception, or gameplay validation.
"""
import hashlib
import json
from pathlib import Path
import struct
import subprocess

import capstone
import pefile

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "local"


def function(text, signature):
    start = text.index(signature)
    end = text.index("{", start)
    depth = 1
    while depth:
        end += 1
        depth += (text[end] == "{") - (text[end] == "}")
    return text[start:end + 1]


def main():
    target = json.loads((ROOT / "config/target.json").read_text())
    binary = Path(target["binary"])
    pe = pefile.PE(str(binary), fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    raw = lambda address, count: pe.get_data(address - base, count)
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    decoder.detail = True
    info = struct.unpack("<9I", raw(0xDD9414, 36))
    assert info[:2] == (0x19930522, 38)
    cleanup = []
    for index in range(38):
        previous, action = struct.unpack("<iI", raw(info[2] + 8 * index, 8))
        assert previous == index - 1
        instructions = list(decoder.disasm(raw(action, 14), action))
        assert [item.mnemonic for item in instructions[:3]] == ["mov", "add", "jmp"]
        cleanup.append((instructions[1].operands[1].imm, instructions[2].operands[0].imm))
    allocators = {0x582630, 0x5826B0, 0x582700, 0x582750, 0x5827A0,
                  0x5827F0, 0x582840, 0x582890, 0x5828E0, 0x582930}
    calls = [item.operands[0].imm for item in decoder.disasm(raw(0x91CE90, 0x789), 0x91CE90)
             if item.mnemonic == "call" and item.operands[0].type == capstone.x86.X86_OP_IMM
             and item.operands[0].imm in allocators]
    assert len(calls) == 36
    source = (ROOT / "src/leftovers_cc13.cpp").read_text()
    source = source[source.index("namespace bsp::oct18_tail_b_contracts {"):]
    helpers = source[source.index("void* offset("):source.index("struct StringLocal")]
    tested = "\n".join(function(source, signature) for signature in [
        "bool ordered_x87_greater(", "float spill_x87_binary32(",
        "PacketTick packet_statistics_tick(", "AngleIntegers camera_angle_integers(",
        "void* construct_score_record_0091ce90("])
    packet = raw(0x787880, 0x65)
    camera = raw(0x8927BA, 0x27)
    array = lambda data: ",".join(str(byte) for byte in data)
    expectations = ",".join("{%d,%d}" % item for item in cleanup)
    cpp = TEMPLATE.replace("@HELPERS@", helpers).replace("@TESTED@", tested)
    cpp = cpp.replace("@PACKET@", array(packet)).replace("@CAMERA@", array(camera))
    cpp = cpp.replace("@CLEANUP@", expectations).replace("@ALLOCATORS@", ",".join(map(str, calls)))
    OUT.mkdir(exist_ok=True)
    source_file = OUT / "oct18_tb_contract_probe.cpp"
    source_file.write_text(cpp)
    vswhere = Path(r"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe")
    installation = subprocess.check_output([str(vswhere), "-latest", "-products", "*",
        "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"], text=True).strip()
    vcvars = Path(installation) / "VC/Auxiliary/Build/vcvars32.bat"
    batch = OUT / "oct18_tb_contract_probe_build.cmd"
    exe = OUT / "oct18_tb_contract_probe.exe"
    batch.write_text(f'@echo off\ncall "{vcvars}" >nul\nif errorlevel 1 exit /b 1\n'
        f'cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /I "{ROOT / "include"}" "{source_file}" '
        f'/Fo"{OUT / "oct18_tb_contract_probe.obj"}" /Fe"{exe}" /link /MANIFEST:EMBED\n'
        f'if errorlevel 1 exit /b 1\n"{exe}"\n')
    print("seed sha256=" + hashlib.sha256(binary.read_bytes()).hexdigest(), flush=True)
    print("packet-slice sha256=" + hashlib.sha256(packet).hexdigest(), flush=True)
    print("camera-slice sha256=" + hashlib.sha256(camera).hexdigest(), flush=True)
    subprocess.run(["cmd.exe", "/d", "/c", str(batch)], cwd=ROOT, check=True)


TEMPLATE = r'''
#define NOMINMAX
#include <windows.h>
#include "bsp/oct18_tail_b_contracts.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vector>
namespace bsp::oct18_tail_b_contracts {
struct AngleIntegers {std::int32_t first,second;};
@HELPERS@
@TESTED@
}
using namespace bsp::oct18_tail_b_contracts;
unsigned short control() {
    unsigned short value;
    __asm fnstcw value
    return value;
}
void control(unsigned short value) {
    __asm fldcw value
}
void require(bool condition) {if(!condition)throw std::runtime_error("contract mismatch");}
void append(std::vector<unsigned char>& out,const unsigned char* bytes,std::size_t size) {out.insert(out.end(),bytes,bytes+size);}
void absolute(unsigned char* memory,std::size_t length,std::uint32_t original,const void* actual) {
    for(std::size_t i=0;i+4<=length;++i)if(std::memcmp(memory+i,&original,4)==0) {
        const auto address=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(actual));
        std::memcpy(memory+i,&address,4);i+=3;
    }
}
void* executable(const std::vector<unsigned char>& bytes) {
    void* memory=VirtualAlloc(nullptr,bytes.size(),MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);
    require(memory!=nullptr);std::memcpy(memory,bytes.data(),bytes.size());return memory;
}
__declspec(naked) void conversion_sse2() {
    __asm {
        sub esp,8
        fstp qword ptr [esp]
        cvttsd2si eax,qword ptr [esp]
        add esp,8
        ret
    }
}
struct CleanupRow {std::uint32_t owner,destructor;};
const CleanupRow native_cleanup[]={@CLEANUP@};
const std::uint32_t native_allocators[]={@ALLOCATORS@};
struct ScoreFixture final : ScoreLifetimeBindings {
    unsigned char* record;int fail=0,count=0;std::vector<void*> allocations;
    std::vector<CleanupRow> cleaned;
    explicit ScoreFixture(unsigned char* storage,int failure):record(storage),fail(failure) {}
    ~ScoreFixture(){for(void* p:allocations)std::free(p);}
    void* allocate_head(std::uint32_t entry,void*) override {
        require(entry==native_allocators[count]);if(++count==fail)throw std::bad_alloc();
        void* p=std::malloc(0x40);require(p!=nullptr);std::memset(p,0xbc,0x40);allocations.push_back(p);return p;
    }
    void destroy_member(std::uint32_t entry,void* member) noexcept override {
        cleaned.push_back({static_cast<std::uint32_t>(static_cast<unsigned char*>(member)-record),entry});
    }
};
int main() {
    const unsigned short original=control();
    const unsigned char packet_body[]={@PACKET@};
    const unsigned char prefix[]={0x55,0x53,0x56,0x57,0x83,0xec,0x40,0x8b,0x74,0x24,0x54,0x8b,0x44,0x24,0x58};
    const unsigned char suffix[]={0x8b,0x44,0x24,0x5c,0x89,0x08,0x89,0x50,0x04,0x83,0xc4,0x40,0x5f,0x5e,0x5b,0x5d,0xc3};
    std::vector<unsigned char> packet;append(packet,prefix,sizeof prefix);append(packet,packet_body,sizeof packet_body);append(packet,suffix,sizeof suffix);
    auto* packet_memory=static_cast<unsigned char*>(executable(packet));
    const float correction=4294967296.0f;absolute(packet_memory,packet.size(),0xce3978,&correction);
    using PacketReference=void(__cdecl*)(void*,const ClockPair*,PacketTick*);
    auto packet_reference=reinterpret_cast<PacketReference>(packet_memory);
    const unsigned char camera_body[]={@CAMERA@};
    const unsigned char camera_prefix[]={0x53,0x56,0x83,0xec,0x30,0x8b,0x44,0x24,0x3c,0x89,0x44,0x24,0x28,0x8b,0x44,0x24,0x40,0x89,0x44,0x24,0x10};
    const unsigned char camera_suffix[]={0x8b,0x14,0x24,0x83,0xc4,0x04,0x8b,0x4c,0x24,0x44,0x89,0x11,0x89,0x41,0x04,0x83,0xc4,0x30,0x5e,0x5b,0xc3};
    std::vector<unsigned char> camera;append(camera,camera_prefix,sizeof camera_prefix);append(camera,camera_body,sizeof camera_body);append(camera,camera_suffix,sizeof camera_suffix);
    auto* camera_memory=static_cast<unsigned char*>(executable(camera));
    const double pi=3.1415927410125732421875,degrees=180;
    absolute(camera_memory,camera.size(),0xce3d28,&pi);absolute(camera_memory,camera.size(),0xce3d20,&degrees);
    for(std::uint32_t address:{0x8927d0u,0x8927dcu}) {
        auto* call=camera_memory+sizeof camera_prefix+(address-0x8927ba);
        require(*call==0xe8);const auto displacement=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&conversion_sse2)-reinterpret_cast<std::uintptr_t>(call+5));
        std::memcpy(call+1,&displacement,4);
    }
    using CameraReference=void(__cdecl*)(float,float,AngleIntegers*);
    auto camera_reference=reinterpret_cast<CameraReference>(camera_memory);
    std::vector<unsigned char> statistics(0x7649b0,0);
    const ClockPair clocks[]={{10000001,10000000},{16777217,3},{9007199254740993LL,10000000},{-1,3},{0x7fffffffffffffffLL,1}};
    unsigned int packet_cases=0,camera_cases=0;
    for(auto precision:{0u,0x200u,0x300u})for(auto rounding:{0u,0x400u,0x800u,0xc00u}) {
        const auto selected=static_cast<unsigned short>((original&~0xf00u)|precision|rounding|0x3fu);control(selected);
        for(const auto& pair:clocks)for(std::uint32_t origin:{0u,0x7fffffffu,0xffffffffu})for(float quantum:{0.0166666675f,1.0f}) {
            write(statistics.data(),0x762958,100u);write(statistics.data(),0x76295c,1100u);
            write(statistics.data(),0x762968,origin);write(statistics.data(),0x762964,quantum);
            PacketTick expected;packet_reference(statistics.data(),&pair,&expected);require(control()==selected);
            const auto actual=packet_statistics_tick(pair,statistics.data());require(control()==selected);
            require(expected.tick==actual.tick && expected.index==actual.index);++packet_cases;
        }
        for(float a:{0.0f,0.1f,-0.1f,1.57079637f,3.14159274f}) {
            const float b=-a*0.375f;AngleIntegers expected;camera_reference(a,b,&expected);
            const auto actual=camera_angle_integers(a,b,reinterpret_cast<void*>(&conversion_sse2));
            require(control()==selected);require(actual.first==expected.first && actual.second==expected.second);++camera_cases;
        }
        X87Value extended{};std::uint64_t significand=0x8000000200000000ULL;std::uint16_t exponent=0x3fff;
        std::memcpy(extended.bytes,&significand,8);std::memcpy(extended.bytes+8,&exponent,2);
        require(ordered_x87_greater(extended,1.0f));
        if(rounding==0)require(spill_x87_binary32(extended)==1.0f);
        significand=0xc000000000000000ULL;exponent=0x7fff;
        std::memcpy(extended.bytes,&significand,8);std::memcpy(extended.bytes+8,&exponent,2);
        require(!ordered_x87_greater(extended,1.0f));
    }
    control(original);
    alignas(4) unsigned char record[0x284];std::memset(record,0xa5,sizeof record);
    {ScoreFixture fixture(record,36);bool failed=false;
        try{construct_score_record_0091ce90(record,fixture);}catch(const std::bad_alloc&){failed=true;}
        require(failed && fixture.cleaned.size()==38);
        for(std::size_t i=0;i<38;++i)require(fixture.cleaned[i].owner==native_cleanup[37-i].owner && fixture.cleaned[i].destructor==native_cleanup[37-i].destructor);
    }
    std::memset(record,0xa5,sizeof record);
    {ScoreFixture fixture(record,0);require(construct_score_record_0091ce90(record,fixture)==record && fixture.count==36 && fixture.cleaned.empty());
        for(const auto& member:native_cleanup)require(read<std::uint32_t>(record,member.owner)==0xa5a5a5a5);
        require(read<std::uint32_t>(record,0x240)==0xa5a5a5a5 && read<std::uint32_t>(record,0x280)==0 && record[0x24c]==0);
        require(record[0x24d]==0xa5 && record[0x24e]==0xa5 && record[0x24f]==0xa5);
        void* head=read<void*>(record,0x244);require(read<void*>(head,0)==head && read<void*>(head,4)==head && read<void*>(head,8)==head && read<unsigned char>(head,0x21)==1);
    }
    VirtualFree(packet_memory,0,MEM_RELEASE);VirtualFree(camera_memory,0,MEM_RELEASE);
    std::printf("PASS packet=%u camera=%u PC24/53/64 x RC4; ST0/spill/NaN; FH3 reverse38 + success/preimage\n",packet_cases,camera_cases);
}
'''


if __name__ == "__main__":
    main()
