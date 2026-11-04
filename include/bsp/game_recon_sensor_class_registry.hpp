#pragma once

#include "bsp/sensor_tables.hpp"

#include <map>
#include <memory>

struct lua_State;

namespace bsp::game {

// Mission-owned equivalent of 00F874E4's cached sensor records. Lua rows are
// read only on a cache miss, after the native ID/Got writes. The returned
// address remains stable until this owner is destroyed; no Lua refs survive
// the scoped binding. The caller supplies the mission's loaded Lua state and real
// VehicleClass.ReconClass id, as 00962419 does in the image.
class GameReconSensorClassRegistry {
public:
    std::unique_ptr<SensorClassRegistryHost> bind(lua_State& state);

private:
    std::map<int, std::unique_ptr<SensorClassTable>> records_;
};

} // namespace bsp::game
