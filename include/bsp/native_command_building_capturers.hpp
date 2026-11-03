#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace bsp::cc13_paratrooper {

// Owns only the +764h lock and +7DCh vector consulted by 006F39B0/006F34B0,
// not a whole MCommandBuilding or an executable native vtable. The opaque values
// are RoundWorld's real round IDs: these bodies only store/compare them, never
// dereference them. Capture scoring resolves the IDs back to the live rounds.
class NativeCommandBuildingCapturers {
public:
    NativeCommandBuildingCapturers();
    ~NativeCommandBuildingCapturers();
    NativeCommandBuildingCapturers(const NativeCommandBuildingCapturers&) = delete;
    NativeCommandBuildingCapturers& operator=(const NativeCommandBuildingCapturers&) = delete;

    void add(std::uint32_t round);
    bool remove(std::uint32_t round);
    void read(std::vector<std::uint32_t>& out) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}
