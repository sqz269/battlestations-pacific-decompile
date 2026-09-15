#include "bsp/game_native_mutable_crt_data.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include "bsp/native_crt_canonical_cookie_initialization.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <wincrypt.h>
#include <algorithm>
#include <atomic>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>

#pragma comment(lib, "advapi32.lib")
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Canonical native CRT data requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
constexpr DWORD image_bytes=12223752;
constexpr std::uintptr_t first_page=0x00e15000;
constexpr std::uintptr_t second_page=0x00e16000;
constexpr std::uintptr_t zero_page=0x0109e000;
constexpr std::size_t page_bytes=0x1000;
constexpr std::array<BYTE,32> image_sha256{
    0xb6,0x82,0xa8,0x2c,0x52,0xf8,0x1f,0x95,0x7b,0x2c,0x70,0x22,0x20,0x77,0x30,0x5a,
    0x93,0x3f,0x72,0x48,0x16,0x86,0xc8,0x88,0x43,0x07,0x7f,0x71,0x4b,0x95,0x6d,0xd6};

bool matches_image(const BYTE* bytes) {
    HCRYPTPROV provider=0;
    if (!CryptAcquireContextW(&provider,nullptr,nullptr,PROV_RSA_AES,CRYPT_VERIFYCONTEXT))
        throw std::runtime_error("Cannot acquire canonical CRT image SHA-256 provider");
    HCRYPTHASH hash=0;
    std::array<BYTE,32> digest{};
    DWORD size=static_cast<DWORD>(digest.size());
    const bool ok=CryptCreateHash(provider,CALG_SHA_256,0,0,&hash) &&
        CryptHashData(hash,bytes,image_bytes,0) &&
        CryptGetHashParam(hash,HP_HASHVAL,digest.data(),&size,0);
    if (hash) CryptDestroyHash(hash);
    CryptReleaseContext(provider,0);
    if (!ok) throw std::runtime_error("Cannot hash canonical CRT image");
    return size==digest.size() && digest==image_sha256;
}
void release_owned(std::array<void*,2>& bands) noexcept {
    // These exact allocation bases entered only through the checked transfer.
    // A committed subpage splits VirtualQuery regions; MEM_RESERVE identity is
    // deliberately not consulted after transfer.
    for (auto& band:bands) if (band) {
        VirtualFree(band,0,MEM_RELEASE);
        band=nullptr;
    }
}
void require_page(std::uintptr_t page,std::uintptr_t allocation,DWORD protection) {
    MEMORY_BASIC_INFORMATION info{};
    if (VirtualQuery(reinterpret_cast<void*>(page),&info,sizeof(info))!=sizeof(info) ||
        reinterpret_cast<std::uintptr_t>(info.BaseAddress)>page ||
        page-reinterpret_cast<std::uintptr_t>(info.BaseAddress)>info.RegionSize ||
        page_bytes>info.RegionSize-(page-reinterpret_cast<std::uintptr_t>(info.BaseAddress)) ||
        info.AllocationBase!=reinterpret_cast<void*>(allocation) ||
        info.State!=MEM_COMMIT ||
        info.Protect!=protection || info.Type!=MEM_PRIVATE)
        throw std::runtime_error("Canonical CRT page protection or ownership changed");
}
std::atomic_flag process_claim=ATOMIC_FLAG_INIT;
std::atomic<GameNativeCanonicalDataOwner*> process_owner{};
} // namespace

GameNativeMutableCrtData::GameNativeMutableCrtData(const std::filesystem::path& path,
    GameNativeDataReservation& reservation) {
    (void)path;
    reservation.transfer_mutable_subset(reservations_);
}
void GameNativeMutableCrtData::initialize(const std::filesystem::path& path) {
    try {
        std::ifstream file(path,std::ios::binary|std::ios::ate);
        if (!file || file.tellg()!=static_cast<std::streamoff>(image_bytes))
            throw std::invalid_argument("Canonical CRT requires the supported original executable size");
        std::vector<BYTE> bytes(image_bytes);
        file.seekg(0);
        if (!file.read(reinterpret_cast<char*>(bytes.data()),image_bytes) ||
            !matches_image(bytes.data()))
            throw std::invalid_argument("Canonical CRT original executable SHA-256 mismatch");
        for (const auto page:{first_page,second_page,zero_page}) {
            void* const address=reinterpret_cast<void*>(page);
            if (VirtualAlloc(address,page_bytes,MEM_COMMIT,PAGE_READWRITE)!=address)
                throw std::runtime_error("Cannot commit canonical CRT page");
        }
        // Exact PE .data raw offsets for the complete two initialized pages.
        std::memcpy(reinterpret_cast<void*>(first_page),bytes.data()+0x00a15000,page_bytes);
        std::memcpy(reinterpret_cast<void*>(second_page),bytes.data()+0x00a16000,page_bytes);
        // The third page is wholly beyond SizeOfRawData: loader zero-fill.
        std::memset(reinterpret_cast<void*>(zero_page),0,page_bytes);
        if (std::memcmp(reinterpret_cast<void*>(first_page),bytes.data()+0x00a15000,page_bytes) ||
            std::memcmp(reinterpret_cast<void*>(second_page),bytes.data()+0x00a16000,page_bytes))
            throw std::runtime_error("Canonical CRT initialized page copy differs from original PE");
        const auto* zeros=reinterpret_cast<const BYTE*>(zero_page);
        if (std::any_of(zeros,zeros+page_bytes,[](BYTE b){return b!=0;}))
            throw std::runtime_error("Canonical CRT virtual page was not zero-filled");
        verify_pages();
        if (cookie()!=0xbb40e64e || complement()!=0x44bf19b1 ||
            nlg_descriptor().signature!=0x19930520 ||
            nlg_descriptor().destination || nlg_descriptor().code || nlg_descriptor().frame ||
            feature_word() || debugger_hook())
            throw std::runtime_error("Canonical CRT initial cells differ from the supported PE");
    } catch (...) {
        release_owned(reservations_);
        throw;
    }
}
GameNativeMutableCrtData::~GameNativeMutableCrtData() noexcept { release_owned(reservations_); }
void GameNativeMutableCrtData::verify_pages() const {
    if (reservations_[0]!=reinterpret_cast<void*>(0x00e10000) ||
        reservations_[1]!=reinterpret_cast<void*>(0x01090000))
        throw std::runtime_error("Canonical CRT allocation bases are not owned");
    require_page(first_page,0x00e10000,PAGE_READWRITE);
    require_page(second_page,0x00e10000,PAGE_READWRITE);
    require_page(zero_page,0x01090000,PAGE_READWRITE);
}
volatile std::uint32_t& GameNativeMutableCrtData::cookie() const noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(0x00e15590);
}
volatile std::uint32_t& GameNativeMutableCrtData::complement() const noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(0x00e15594);
}
NativeCrtNlgDescriptor& GameNativeMutableCrtData::nlg_descriptor() const noexcept {
    return *reinterpret_cast<NativeCrtNlgDescriptor*>(0x00e16830);
}
NativeCrtFailureBlock& GameNativeMutableCrtData::failure_block() const noexcept {
    return *reinterpret_cast<NativeCrtFailureBlock*>(0x0109e568);
}
volatile std::uint32_t& GameNativeMutableCrtData::feature_word() const noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(0x0109eea4);
}
volatile std::uint32_t& GameNativeMutableCrtData::debugger_hook() const noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(0x0109eea8);
}

GameNativeCanonicalDataOwner::GameNativeCanonicalDataOwner(const std::filesystem::path& path,
    const GameNativeDataSpan* spans,std::size_t count,GameNativeDataReservation& reservation) {
    // Read-only and mutable constructors each own rollback once their disjoint
    // subset has transferred. The capability owns any subset not yet transferred.
    readonly_.reset(new GameNativeReadOnlyData(path,spans,count,reservation,true));
    mutable_.reset(new GameNativeMutableCrtData(path,reservation));
    // Both owners now have their allocation bases; only now may a subpage commit.
    readonly_->initialize(path,spans,count,nullptr,false,true);
    mutable_->initialize(path);
    verify_joint();
    bsp::initialize_native_crt_canonical_security_cookie_00c1815e();
    if ((mutable_->cookie() ^ mutable_->complement())!=0xffffffffu ||
        mutable_->cookie()==0xbb40e64e)
        throw std::runtime_error("Canonical CRT cookie initialization did not publish its complement");
    mutable_->verify_pages();
}
void GameNativeCanonicalDataOwner::verify_joint() const {
    mutable_->verify_pages();
    const auto& pair=exception_pair();
    if (pair.record!=0x0109e568 || pair.context!=0x0109e5c0)
        throw std::runtime_error("Canonical CRT D6 exception pair does not point into the failure block");
    require_page(0x00d6e000,0x00d60000,PAGE_READONLY);
    if (reinterpret_cast<std::uintptr_t>(&mutable_->failure_block().context)!=pair.context)
        throw std::runtime_error("Canonical CRT exception context relationship is invalid");
}
const NativeCrtExceptionPair& GameNativeCanonicalDataOwner::exception_pair() const {
    return *static_cast<const NativeCrtExceptionPair*>(readonly_->data_at(0x00d6e1cc,8));
}
void GameNativeCanonicalDataOwner::RollbackDelete::operator()(
    GameNativeCanonicalDataOwner* owner) const noexcept { delete owner; }
const GameNativeCanonicalDataOwner& GameNativeCanonicalDataOwner::initialize(
    const std::filesystem::path& path,const GameNativeDataSpan* spans,std::size_t count,
    GameNativeDataReservation&& reservation) {
    // The API consumes its incoming capability even if a duplicate process
    // owner or an early constructor failure is reported to a catching caller.
    GameNativeDataReservation owned(std::move(reservation));
    if (process_claim.test_and_set(std::memory_order_acq_rel))
        throw std::logic_error("Canonical CRT process owner already claimed");
    try {
        // All heap ownership metadata exists before the success ACK. The
        // remaining publication is pointer release and an atomic store only.
        std::unique_ptr<GameNativeCanonicalDataOwner,RollbackDelete> pending(
            new GameNativeCanonicalDataOwner(path,spans,count,owned));
        if (!owned.finish(true))
            throw std::runtime_error("Canonical CRT parent cancelled readiness ACK");
        auto* const permanent=pending.release();
        process_owner.store(permanent,std::memory_order_release);
        return *permanent;
    } catch (...) {
        process_claim.clear(std::memory_order_release);
        throw;
    }
}
} // namespace bsp::game
