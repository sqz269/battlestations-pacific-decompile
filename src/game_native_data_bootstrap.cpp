#include "bsp/game_native_data_bootstrap.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <shellapi.h>
#include <algorithm>
#include <array>
#include <cwchar>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#pragma comment(lib, "shell32.lib")
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native-data suspended-child bootstrap requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
constexpr std::uintptr_t first_band=0x00ce0000;
constexpr std::size_t band_bytes=0x10000;
constexpr std::size_t ro_band_count=19;
constexpr std::size_t band_count=21;
constexpr std::uint32_t mutable_mask=(1u<<19)|(1u<<20);
constexpr std::uint32_t canonical_ro_mask=0x18a;
constexpr std::array<DWORD,3> mutable_pages{0x00e15000,0x00e16000,0x0109e000};
constexpr DWORD handoff_magic=0x48534442; // BDSH, source protocol only
constexpr DWORD handoff_version_v1=1;
constexpr DWORD handoff_version_v2=2;
constexpr LONG state_pending=0;
constexpr LONG state_ready=1;
constexpr LONG state_claimed=2;
constexpr LONG state_mapped=3;
constexpr LONG state_failed=-1;
constexpr wchar_t argument_prefix[]=L"--bsp-native-data-handoff=";

struct HandoffRecord {
    DWORD magic;
    DWORD version;
    DWORD parent_pid;
    DWORD child_pid;
    DWORD band_mask;
    volatile LONG state;
    // Zero in v1. V2 names every committed mutable page and its role.
    DWORD mutable_page[3];
    DWORD mutable_roles;
};
static_assert(sizeof(HandoffRecord) <= 0x1000);

HANDLE as_handle(void* value) noexcept { return static_cast<HANDLE>(value); }
HandoffRecord* as_record(void* value) noexcept { return static_cast<HandoffRecord*>(value); }
std::uintptr_t band_address(std::size_t slot) noexcept {
    return slot<ro_band_count ? first_band+slot*band_bytes :
        slot==19 ? 0x00e10000 : 0x01090000;
}
DWORD plan_version(GameNativeDataPlan plan) noexcept {
    return plan==GameNativeDataPlan::CanonicalCrtV2 ? handoff_version_v2 : handoff_version_v1;
}
std::uint32_t plan_mask(std::uint32_t ro_mask,GameNativeDataPlan plan) {
    if (plan==GameNativeDataPlan::CanonicalCrtV2) {
        if (ro_mask!=canonical_ro_mask)
            throw std::invalid_argument("Canonical CRT plan requires the exact game RO bands");
        return ro_mask|mutable_mask;
    }
    return ro_mask;
}
bool record_plan_matches(const HandoffRecord& record,GameNativeDataPlan plan) noexcept {
    if (record.version!=plan_version(plan)) return false;
    if (plan==GameNativeDataPlan::ReadOnlyV1)
        return record.mutable_page[0]==0 && record.mutable_page[1]==0 &&
            record.mutable_page[2]==0 && record.mutable_roles==0;
    return record.mutable_page[0]==mutable_pages[0] &&
        record.mutable_page[1]==mutable_pages[1] &&
        record.mutable_page[2]==mutable_pages[2] && record.mutable_roles==0x7;
}

void check_platform() {
    SYSTEM_INFO info{};
    GetSystemInfo(&info);
    if (info.dwAllocationGranularity!=band_bytes || info.dwPageSize!=0x1000)
        throw std::runtime_error("Unsupported native-data allocation granularity");
}

bool exact_reservation(HANDLE process, std::uintptr_t address) noexcept {
    MEMORY_BASIC_INFORMATION info{};
    if (VirtualQueryEx(process,reinterpret_cast<void*>(address),&info,sizeof(info))!=sizeof(info))
        return false;
    return info.BaseAddress==reinterpret_cast<void*>(address) &&
        info.AllocationBase==reinterpret_cast<void*>(address) &&
        info.RegionSize==band_bytes && info.State==MEM_RESERVE &&
        info.AllocationProtect==PAGE_NOACCESS && info.Type==MEM_PRIVATE;
}

void release_local(std::array<void*,band_count>& bands) noexcept {
    for (auto& band:bands) if (band) {
        // Only a band previously allocated by this bootstrap may enter bands.
        if (exact_reservation(GetCurrentProcess(),reinterpret_cast<std::uintptr_t>(band)))
            VirtualFree(band,0,MEM_RELEASE);
        band=nullptr;
    }
}

std::wstring handoff_argument(HANDLE mapping) {
    wchar_t digits[16]{};
    swprintf_s(digits,L"%08X",static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(mapping)));
    return std::wstring(argument_prefix)+digits;
}

HANDLE inherited_mapping_argument() {
    int argc=0;
    LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(),&argc);
    if (!argv) throw std::runtime_error("Cannot parse native-data child command line");
    HANDLE result=nullptr;
    bool found=false;
    const auto prefix_length=std::wcslen(argument_prefix);
    for (int i=1;i<argc;++i) {
        if (std::wcsncmp(argv[i],argument_prefix,prefix_length)!=0) continue;
        const wchar_t* digits=argv[i]+prefix_length;
        const auto length=std::wcslen(digits);
        if (found || length!=8) { LocalFree(argv); throw std::invalid_argument("Duplicate or malformed native-data handoff argument"); }
        std::uintptr_t value=0;
        for (const wchar_t* p=digits;*p;++p) {
            const wchar_t c=*p;
            const unsigned nibble=(c>=L'0' && c<=L'9') ? c-L'0' :
                (c>=L'A' && c<=L'F') ? c-L'A'+10 :
                (c>=L'a' && c<=L'f') ? c-L'a'+10 : 16;
            if (nibble>=16) { LocalFree(argv); throw std::invalid_argument("Malformed native-data handoff handle"); }
            value=(value<<4)|nibble;
        }
        result=reinterpret_cast<HANDLE>(value);
        found=true;
    }
    LocalFree(argv);
    if (!found || !result || result==INVALID_HANDLE_VALUE)
        throw std::invalid_argument("Controlled native-data handoff is absent");
    return result;
}
} // namespace

namespace detail {
std::uint32_t native_data_band_mask(const GameNativeDataSpan* spans, std::size_t count) {
    if (!spans || !count)
        throw std::invalid_argument("Native data requires explicit table or literal spans");
    std::uint32_t mask=0;
    for (std::size_t i=0;i<count;++i) {
        const auto address=spans[i].address;
        const auto size=spans[i].bytes;
        if (!size || address<GameNativeReadOnlyData::begin_address ||
            address-GameNativeReadOnlyData::begin_address>=GameNativeReadOnlyData::byte_count ||
            size>GameNativeReadOnlyData::byte_count-(address-GameNativeReadOnlyData::begin_address))
            throw std::out_of_range("Required native data is outside the read-only section");
        const auto first=(address-first_band)/band_bytes;
        const auto last=(address+size-1-first_band)/band_bytes;
        for (auto band=first;band<=last;++band) mask|=1u<<band;
    }
    return mask;
}
} // namespace detail

GameNativeDataReservation::~GameNativeDataReservation() noexcept {
    release_local(bands_);
    if (handoff_view_) finish(false);
}
GameNativeDataReservation::GameNativeDataReservation(GameNativeDataReservation&& other) noexcept {
    *this=std::move(other);
}
GameNativeDataReservation& GameNativeDataReservation::operator=(GameNativeDataReservation&& other) noexcept {
    if (this!=&other) {
        release_local(bands_);
        if (handoff_view_) finish(false);
        bands_=other.bands_; mask_=other.mask_; plan_=other.plan_;
        handoff_view_=other.handoff_view_; handoff_handle_=other.handoff_handle_;
        other.bands_.fill(nullptr); other.mask_=0;
        other.handoff_view_=nullptr; other.handoff_handle_=nullptr;
    }
    return *this;
}
bool GameNativeDataReservation::finish(bool mapped) noexcept {
    if (!handoff_view_) return false;
    auto* record=as_record(handoff_view_);
    // The parent may have cancelled a timed-out handoff. Never overwrite that
    // decision with a late success acknowledgement.
    const bool accepted=InterlockedCompareExchange(&record->state,
        mapped ? state_mapped : state_failed,state_claimed)==state_claimed;
    UnmapViewOfFile(handoff_view_);
    CloseHandle(as_handle(handoff_handle_));
    handoff_view_=nullptr; handoff_handle_=nullptr;
    return accepted;
}
void GameNativeDataReservation::transfer_to(std::array<void*,19>& destination,
    std::uint32_t expected_mask) {
    if (!handoff_view_ || plan_!=GameNativeDataPlan::ReadOnlyV1 || mask_!=expected_mask)
        throw std::invalid_argument("Native-data reservation does not match requested bands");
    for (std::size_t i=0;i<ro_band_count;++i) {
        const bool selected=(mask_ & (1u<<i))!=0;
        if (selected != (bands_[i]!=nullptr) || destination[i])
            throw std::invalid_argument("Incomplete native-data reservation capability");
        if (selected && !exact_reservation(GetCurrentProcess(),band_address(i)))
            throw std::runtime_error("Transferred native-data reservation changed before mapping");
    }
    std::copy_n(bands_.begin(),ro_band_count,destination.begin());
    bands_.fill(nullptr);
    mask_=0;
}
void GameNativeDataReservation::transfer_ro_subset(std::array<void*,19>& destination,
    std::uint32_t expected_mask) {
    if (!handoff_view_ || plan_!=GameNativeDataPlan::CanonicalCrtV2 ||
        expected_mask!=canonical_ro_mask || mask_!=(expected_mask|mutable_mask))
        throw std::invalid_argument("Canonical RO subset does not match the handoff");
    for (std::size_t i=0;i<ro_band_count;++i) {
        const bool selected=(expected_mask & (1u<<i))!=0;
        if (selected!=(bands_[i]!=nullptr) || destination[i] ||
            (selected && !exact_reservation(GetCurrentProcess(),band_address(i))))
            throw std::runtime_error("Canonical RO subset changed before transfer");
    }
    for (std::size_t i=0;i<ro_band_count;++i) if (expected_mask & (1u<<i)) {
        destination[i]=bands_[i]; bands_[i]=nullptr;
    }
    mask_&=~expected_mask;
}
void GameNativeDataReservation::transfer_mutable_subset(std::array<void*,2>& destination) {
    if (!handoff_view_ || plan_!=GameNativeDataPlan::CanonicalCrtV2 || mask_!=mutable_mask)
        throw std::invalid_argument("Canonical mutable subset does not match the handoff");
    for (std::size_t i=0;i<2;++i) if (!bands_[19+i] || destination[i] ||
        !exact_reservation(GetCurrentProcess(),band_address(19+i)))
        throw std::runtime_error("Canonical mutable subset changed before transfer");
    for (std::size_t i=0;i<2;++i) {
        destination[i]=bands_[19+i]; bands_[19+i]=nullptr;
    }
    mask_=0;
}

GameNativeDataReservation accept_native_data_handoff(
    const GameNativeDataSpan* spans, std::size_t count, GameNativeDataPlan plan) {
    check_platform();
    const auto expected=plan_mask(detail::native_data_band_mask(spans,count),plan);
    const HANDLE mapping=inherited_mapping_argument();
    DWORD flags=0;
    if (!GetHandleInformation(mapping,&flags) || !(flags & HANDLE_FLAG_INHERIT))
        throw std::invalid_argument("Native-data handoff handle was not inherited");
    void* const view=MapViewOfFile(mapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,sizeof(HandoffRecord));
    if (!view) throw std::invalid_argument("Native-data handoff mapping is unavailable");
    auto* record=as_record(view);
    if (record->magic!=handoff_magic || !record_plan_matches(*record,plan) ||
        record->child_pid!=GetCurrentProcessId() || record->parent_pid==GetCurrentProcessId() ||
        record->band_mask!=expected || record->state!=state_ready) {
        UnmapViewOfFile(view);
        throw std::invalid_argument("Native-data handoff identity, band set or state is invalid");
    }
    if (InterlockedCompareExchange(&record->state,state_claimed,state_ready)!=state_ready) {
        UnmapViewOfFile(view);
        throw std::invalid_argument("Native-data handoff was already consumed");
    }
    for (std::size_t i=0;i<band_count;++i) if (expected & (1u<<i)) {
        if (!exact_reservation(GetCurrentProcess(),band_address(i))) {
            MEMORY_BASIC_INFORMATION actual{};
            VirtualQuery(reinterpret_cast<void*>(band_address(i)),&actual,sizeof(actual));
            InterlockedExchange(&record->state,state_failed);
            UnmapViewOfFile(view);
            CloseHandle(mapping);
            throw std::runtime_error("Inherited native-data reservation is incomplete or modified at band " +
                std::to_string(i) + " state=" + std::to_string(actual.State) +
                " size=" + std::to_string(actual.RegionSize) +
                " allocation_protect=" + std::to_string(actual.AllocationProtect) +
                " type=" + std::to_string(actual.Type));
        }
    }
    GameNativeDataReservation result;
    result.mask_=expected;
    result.plan_=plan;
    result.handoff_view_=view;
    result.handoff_handle_=mapping;
    for (std::size_t i=0;i<band_count;++i) if (expected & (1u<<i))
        result.bands_[i]=reinterpret_cast<void*>(band_address(i));
    return result;
}

GameNativeDataBootstrapChild::GameNativeDataBootstrapChild(
    const std::filesystem::path& executable, const std::wstring& arguments,
    const GameNativeDataSpan* spans, std::size_t count, GameNativeDataPlan plan)
    : mask_(plan_mask(detail::native_data_band_mask(spans,count),plan)), plan_(plan) {
    check_platform();
    SECURITY_ATTRIBUTES security{sizeof(security),nullptr,TRUE};
    const HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,&security,PAGE_READWRITE,0,0x1000,nullptr);
    if (!mapping) throw std::runtime_error("Cannot create native-data handoff section");
    handoff_handle_=mapping;
    try {
        handoff_view_=MapViewOfFile(mapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,sizeof(HandoffRecord));
        if (!handoff_view_) throw std::runtime_error("Cannot map native-data handoff section");
        auto* record=as_record(handoff_view_);
        record->magic=handoff_magic; record->version=plan_version(plan_);
        record->parent_pid=GetCurrentProcessId(); record->band_mask=mask_;
        for (std::size_t i=0;i<mutable_pages.size();++i)
            record->mutable_page[i]=plan_==GameNativeDataPlan::CanonicalCrtV2 ? mutable_pages[i] : 0;
        record->mutable_roles=plan_==GameNativeDataPlan::CanonicalCrtV2 ? 0x7 : 0;
        record->state=state_pending;

        SIZE_T attribute_bytes=0;
        InitializeProcThreadAttributeList(nullptr,1,0,&attribute_bytes);
        if (!attribute_bytes) throw std::runtime_error("Cannot size child handle inheritance list");
        std::vector<BYTE> storage(attribute_bytes);
        auto* attributes=reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(storage.data());
        if (!InitializeProcThreadAttributeList(attributes,1,0,&attribute_bytes))
            throw std::runtime_error("Cannot initialize child handle inheritance list");
        try {
            HANDLE handles[]={mapping};
            if (!UpdateProcThreadAttribute(attributes,0,PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                    handles,sizeof(handles),nullptr,nullptr))
                throw std::runtime_error("Cannot restrict child handle inheritance");
            STARTUPINFOEXW startup{};
            startup.StartupInfo.cb=sizeof(startup);
            startup.lpAttributeList=attributes;
            const std::wstring path=executable.wstring();
            std::wstring command=L"\""+path+L"\"";
            if (!arguments.empty()) command+=L" "+arguments;
            command+=L" "+handoff_argument(mapping);
            std::vector<wchar_t> mutable_command(command.begin(),command.end());
            mutable_command.push_back(0);
            PROCESS_INFORMATION child{};
            if (!CreateProcessW(path.c_str(),mutable_command.data(),nullptr,nullptr,TRUE,
                    CREATE_SUSPENDED|CREATE_NO_WINDOW|EXTENDED_STARTUPINFO_PRESENT,
                    nullptr,nullptr,&startup.StartupInfo,&child))
                throw std::runtime_error("Cannot create suspended native-data child");
            process_=child.hProcess; thread_=child.hThread; process_id_=child.dwProcessId;
            record->child_pid=process_id_;
        } catch (...) {
            DeleteProcThreadAttributeList(attributes);
            throw;
        }
        DeleteProcThreadAttributeList(attributes);
    } catch (...) { close(); throw; }
}

GameNativeDataBootstrapChild::~GameNativeDataBootstrapChild() noexcept { close(); }
void GameNativeDataBootstrapChild::close() noexcept {
    if (process_ && !mapping_confirmed_) fail_child();
    if (thread_) { CloseHandle(as_handle(thread_)); thread_=nullptr; }
    if (process_) { CloseHandle(as_handle(process_)); process_=nullptr; }
    if (handoff_view_) { UnmapViewOfFile(handoff_view_); handoff_view_=nullptr; }
    if (handoff_handle_) { CloseHandle(as_handle(handoff_handle_)); handoff_handle_=nullptr; }
}
void GameNativeDataBootstrapChild::fail_child() noexcept {
    if (process_) {
        TerminateProcess(as_handle(process_),ERROR_INVALID_DATA);
        WaitForSingleObject(as_handle(process_),5000);
    }
}
void* GameNativeDataBootstrapChild::suspended_process_handle() const noexcept {
    return resumed_ ? nullptr : process_;
}
void* GameNativeDataBootstrapChild::process_handle() const noexcept { return process_; }
std::uint32_t GameNativeDataBootstrapChild::process_id() const noexcept { return process_id_; }

void GameNativeDataBootstrapChild::reserve_and_resume() {
    if (!process_ || resumed_ ||
        InterlockedCompareExchange(&as_record(handoff_view_)->state,state_pending,state_pending)!=state_pending ||
        as_record(handoff_view_)->magic!=handoff_magic ||
        as_record(handoff_view_)->parent_pid!=GetCurrentProcessId() ||
        as_record(handoff_view_)->child_pid!=process_id_ ||
        as_record(handoff_view_)->band_mask!=mask_ ||
        !record_plan_matches(*as_record(handoff_view_),plan_))
        throw std::logic_error("Native-data child is not pending reservation");
    std::array<void*,band_count> owned{};
    for (std::size_t i=0;i<band_count;++i) if (mask_ & (1u<<i)) {
        void* const address=reinterpret_cast<void*>(band_address(i));
        void* const got=VirtualAllocEx(as_handle(process_),address,band_bytes,MEM_RESERVE,PAGE_NOACCESS);
        if (got!=address) {
            const DWORD allocation_error=GetLastError();
            MEMORY_BASIC_INFORMATION actual{};
            const SIZE_T queried=VirtualQueryEx(as_handle(process_),address,&actual,sizeof(actual));
            const DWORD query_error=queried ? ERROR_SUCCESS : GetLastError();
            CONTEXT context{};
            context.ContextFlags=CONTEXT_CONTROL;
            const BOOL context_read=GetThreadContext(as_handle(thread_),&context);
            PROCESS_MITIGATION_ASLR_POLICY parent_aslr{}, child_aslr{};
            GetProcessMitigationPolicy(GetCurrentProcess(),ProcessASLRPolicy,&parent_aslr,sizeof(parent_aslr));
            GetProcessMitigationPolicy(as_handle(process_),ProcessASLRPolicy,&child_aslr,sizeof(child_aslr));
            if (got) VirtualFreeEx(as_handle(process_),got,0,MEM_RELEASE);
            for (auto& band:owned) if (band) {
                if (exact_reservation(as_handle(process_),reinterpret_cast<std::uintptr_t>(band)))
                    VirtualFreeEx(as_handle(process_),band,0,MEM_RELEASE);
                band=nullptr;
            }
            throw std::runtime_error("Cannot reserve native-data band in suspended child: address=" +
                std::to_string(reinterpret_cast<std::uintptr_t>(address)) +
                " returned=" + std::to_string(reinterpret_cast<std::uintptr_t>(got)) +
                " allocation_error=" + std::to_string(allocation_error) +
                " query_error=" + std::to_string(query_error) +
                " parent_pid=" + std::to_string(GetCurrentProcessId()) +
                " child_pid=" + std::to_string(process_id_) +
                " initial_esp=" + std::to_string(context_read ? context.Esp : 0) +
                " parent_aslr_flags=" + std::to_string(parent_aslr.Flags) +
                " child_aslr_flags=" + std::to_string(child_aslr.Flags) +
                " allocation_base=" + std::to_string(reinterpret_cast<std::uintptr_t>(actual.AllocationBase)) +
                " region_base=" + std::to_string(reinterpret_cast<std::uintptr_t>(actual.BaseAddress)) +
                " region_bytes=" + std::to_string(actual.RegionSize) +
                " state=" + std::to_string(actual.State) +
                " protection=" + std::to_string(actual.Protect) +
                " type=" + std::to_string(actual.Type));
        }
        owned[i]=got;
    }
    InterlockedExchange(&as_record(handoff_view_)->state,state_ready);
    if (ResumeThread(as_handle(thread_))==DWORD(-1)) {
        InterlockedExchange(&as_record(handoff_view_)->state,state_failed);
        for (auto band:owned) if (band && exact_reservation(as_handle(process_),reinterpret_cast<std::uintptr_t>(band)))
            VirtualFreeEx(as_handle(process_),band,0,MEM_RELEASE);
        fail_child();
        throw std::runtime_error("Cannot resume native-data child; owned bands rolled back");
    }
    resumed_=true;
    CloseHandle(as_handle(thread_)); thread_=nullptr;
}

void GameNativeDataBootstrapChild::wait_for_mapping(std::uint32_t timeout_ms) {
    if (!resumed_ || !process_) throw std::logic_error("Native-data child was not resumed");
    const ULONGLONG deadline=GetTickCount64()+timeout_ms;
    for (;;) {
        auto* const record=as_record(handoff_view_);
        const LONG state=InterlockedCompareExchange(&record->state,state_pending,state_pending);
        if (state==state_mapped) { mapping_confirmed_=true; return; }
        if (state==state_failed) { fail_child(); throw std::runtime_error("Native-data child rejected the handoff or mapping failed"); }
        if (WaitForSingleObject(as_handle(process_),0)==WAIT_OBJECT_0) {
            // A fast child can publish the result and exit after the first
            // state read. Process exit makes this second read final.
            const LONG final_state=InterlockedCompareExchange(
                &record->state,state_pending,state_pending);
            if (final_state==state_mapped) { mapping_confirmed_=true; return; }
            if (final_state==state_failed)
                throw std::runtime_error("Native-data child rejected the handoff or mapping failed");
            DWORD code=0;
            GetExitCodeProcess(as_handle(process_),&code);
            throw std::runtime_error("Native-data child exited before mapper handoff completed (exit " +
                std::to_string(code) + ")");
        }
        const auto now=GetTickCount64();
        if (now>=deadline) {
            // Atomically decide timeout against the child's final ACK. If the
            // state changed meanwhile, re-evaluate it before terminating.
            if (InterlockedCompareExchange(&record->state,state_failed,state)!=state)
                continue;
            fail_child();
            throw std::runtime_error("Native-data child handoff timed out");
        }
        Sleep(static_cast<DWORD>(std::min<ULONGLONG>(10,deadline-now)));
    }
}
} // namespace bsp::game
