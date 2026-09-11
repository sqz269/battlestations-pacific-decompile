#pragma once

#include "bsp/xlive_pipe_globals.hpp"
#include <memory>
#include <string>

namespace bsp {

// Reads only the two required original PE32 data ranges. Their existing views
// independently verify exact SHA-256 values. Owns immutable backing storage;
// keep it alive through all table/global-data consumers. Never loads/executes
// the original EXE, writes its installation, or alters the DLL search path.
class XLivePipeOriginalData final {
public:
    explicit XLivePipeOriginalData(const std::wstring& absolute_executable_path);
    ~XLivePipeOriginalData();
    XLivePipeOriginalData(const XLivePipeOriginalData&) = delete;
    XLivePipeOriginalData& operator=(const XLivePipeOriginalData&) = delete;
    const XLivePipeProtocolTables& tables() const noexcept;
    const XLivePipeGlobalData& globals() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Real std::atexit registration of the six recovered cleanup callbacks. The
// process has one canonical owner; that owner must stay alive until every
// registered callback has run. The host itself need not stay alive. Arrange
// owner storage lifetime before registration (e.g. a static owner constructed
// before initialize_xlive_pipe_globals), not a shorter-lived automatic owner.
// Native registration failures are returned unchanged, without rollback.
class CrtXLivePipeGlobalsStartupHost final : public XLivePipeGlobalsStartupHost {
public:
    explicit CrtXLivePipeGlobalsStartupHost(XLivePipeGlobalsOwner&) noexcept;
    XLivePipeValueLock* create_value_lock_00a5fa5a() override;
    int register_cleanup_00bf6ff5(std::uint32_t native_address,
        XLivePipeGlobalCleanup callback, XLivePipeGlobalsOwner&) override;
private:
    XLivePipeGlobalsOwner& owner_;
};

} // namespace bsp
