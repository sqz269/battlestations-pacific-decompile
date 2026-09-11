#include "bsp/xlive_pipe_bootstrap.hpp"

#include <atomic>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace bsp {
namespace {
struct ImageSpans {
    std::vector<std::uint8_t> tables;
    std::vector<std::uint8_t> globals;
};

ImageSpans read_image_spans(const std::wstring& filename) {
    const std::filesystem::path path(filename);
    if (!path.is_absolute()) throw std::invalid_argument("Original game image path must be absolute");
    std::ifstream image(path, std::ios::binary);
    if (!image) throw std::runtime_error("Cannot open original game image for reading");
    image.seekg(0, std::ios::end);
    const auto end = image.tellg();
    if (end < std::streampos(0)) throw std::runtime_error("Cannot size original game image");
    const auto file_bytes = static_cast<std::uint64_t>(static_cast<std::streamoff>(end));
    auto read_at = [&](std::uint64_t offset, void* output, std::size_t count) {
        if (offset > file_bytes || count > file_bytes - offset)
            throw std::runtime_error("Original game PE data exceeds file bounds");
        image.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
        if (!image.read(static_cast<char*>(output), static_cast<std::streamsize>(count)))
            throw std::runtime_error("Cannot read original game PE data");
    };
    IMAGE_DOS_HEADER dos{};
    read_at(0, &dos, sizeof dos);
    if (dos.e_magic != IMAGE_DOS_SIGNATURE || dos.e_lfanew < 0)
        throw std::invalid_argument("Original game image has no valid DOS header");
    const auto nt = static_cast<std::uint64_t>(dos.e_lfanew);
    DWORD signature;
    IMAGE_FILE_HEADER file_header{};
    IMAGE_OPTIONAL_HEADER32 optional{};
    read_at(nt, &signature, sizeof signature);
    read_at(nt + sizeof signature, &file_header, sizeof file_header);
    if (signature != IMAGE_NT_SIGNATURE || file_header.Machine != IMAGE_FILE_MACHINE_I386 ||
        file_header.NumberOfSections == 0 || file_header.NumberOfSections > 96 ||
        file_header.SizeOfOptionalHeader < sizeof optional)
        throw std::invalid_argument("Original game image is not the expected PE32 structure");
    read_at(nt + sizeof signature + sizeof file_header, &optional, sizeof optional);
    if (optional.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC || optional.ImageBase != 0x00400000)
        throw std::invalid_argument("Original game image has a different PE32 base");
    std::vector<IMAGE_SECTION_HEADER> sections(file_header.NumberOfSections);
    read_at(nt + sizeof signature + sizeof file_header + file_header.SizeOfOptionalHeader,
        sections.data(), sections.size() * sizeof(IMAGE_SECTION_HEADER));
    auto read_span = [&](std::uint32_t va, std::size_t count) {
        const auto rva = va - optional.ImageBase;
        for (const auto& section : sections) {
            if (rva < section.VirtualAddress) continue;
            const auto delta = rva - section.VirtualAddress;
            if (delta > section.SizeOfRawData || count > section.SizeOfRawData - delta) continue;
            std::vector<std::uint8_t> bytes(count);
            read_at(static_cast<std::uint64_t>(section.PointerToRawData) + delta, bytes.data(), count);
            return bytes;
        }
        throw std::invalid_argument("Required original data range is not fully backed by PE file bytes");
    };
    return {read_span(XLivePipeProtocolTables::first_va, XLivePipeProtocolTables::size),
        read_span(XLivePipeGlobalData::first_va, XLivePipeGlobalData::size)};
}

std::atomic<XLivePipeGlobalsOwner*> registered_owner{nullptr};
constexpr std::array<XLivePipeGlobalCleanup, 6> cleanup_functions{
    &cleanup_xlive_pipe_acquisition_00ce099a,
    &cleanup_xlive_pipe_frame_value_00ce09a4,
    &cleanup_xlive_pipe_frame_value_00ce09b5,
    &cleanup_xlive_pipe_seed_00ce09c6,
    &cleanup_xlive_pipe_frame_value_00ce09d7,
    &cleanup_xlive_pipe_key_00ce09e8};
constexpr std::array<std::uint32_t, 6> cleanup_addresses{
    0x00ce099a, 0x00ce09a4, 0x00ce09b5, 0x00ce09c6, 0x00ce09d7, 0x00ce09e8};

template<std::size_t Index>
void cleanup_at_exit() noexcept {
    auto* owner = registered_owner.load();
    if (owner == nullptr) std::terminate();
    try { cleanup_functions[Index](*owner); }
    catch (...) { std::terminate(); }
}
constexpr std::array<void (*)(), 6> cleanup_thunks{
    &cleanup_at_exit<0>, &cleanup_at_exit<1>, &cleanup_at_exit<2>,
    &cleanup_at_exit<3>, &cleanup_at_exit<4>, &cleanup_at_exit<5>};
}

struct XLivePipeOriginalData::Impl {
    explicit Impl(const std::wstring& filename)
        : data(read_image_spans(filename)),
          table_view(data.tables.data(), data.tables.size()),
          global_view(data.globals.data(), data.globals.size()) {}
    ImageSpans data;
    XLivePipeProtocolTables table_view;
    XLivePipeGlobalData global_view;
};

XLivePipeOriginalData::XLivePipeOriginalData(const std::wstring& path)
    : impl_(std::make_unique<Impl>(path)) {}
XLivePipeOriginalData::~XLivePipeOriginalData() = default;
const XLivePipeProtocolTables& XLivePipeOriginalData::tables() const noexcept { return impl_->table_view; }
const XLivePipeGlobalData& XLivePipeOriginalData::globals() const noexcept { return impl_->global_view; }

CrtXLivePipeGlobalsStartupHost::CrtXLivePipeGlobalsStartupHost(XLivePipeGlobalsOwner& owner) noexcept
    : owner_(owner) {}
XLivePipeValueLock* CrtXLivePipeGlobalsStartupHost::create_value_lock_00a5fa5a() {
    return create_xlive_pipe_value_lock_00a5fa5a();
}
int CrtXLivePipeGlobalsStartupHost::register_cleanup_00bf6ff5(
    std::uint32_t address, XLivePipeGlobalCleanup callback, XLivePipeGlobalsOwner& owner) {
    if (&owner != &owner_) throw std::invalid_argument("CRT cleanup belongs to a different pipe globals owner");
    for (std::size_t i = 0; i < cleanup_addresses.size(); ++i) {
        if (cleanup_addresses[i] != address) continue;
        if (cleanup_functions[i] != callback)
            throw std::invalid_argument("CRT pipe cleanup identity and recovered callback disagree");
        auto* expected = static_cast<XLivePipeGlobalsOwner*>(nullptr);
        if (!registered_owner.compare_exchange_strong(expected, &owner) && expected != &owner)
            throw std::logic_error("CRT pipe callbacks already bind another canonical owner");
        return std::atexit(cleanup_thunks[i]);
    }
    throw std::invalid_argument("Unreconstructed pipe globals cleanup identity");
}

} // namespace bsp
