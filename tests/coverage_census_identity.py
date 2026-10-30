"""Focused regression: an executed adapter suffix is not a body hit."""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from coverage_census import Resolver


def main():
    resolver = Resolver({"rows": [
        {"address": "0059cb90", "name": "hud_unit_icon_index_0059cb90",
         "source": "src/frontend_menus.cpp", "state": "wired"},
        {"address": "0064d610", "name": "GameHudHost::update_ship_view_screen_0064d610",
         "source": "src/game_hosts_hud.cpp", "state": "wired"},
    ]})
    host_source = r"J:\frozen\src\game_hosts_hud.cpp"
    adapter = "bsp::game::ShipScreenApplyUnitAdapter::unit_hud_icon_index_0059cb90"
    body = "bsp::hud_unit_icon_index_0059cb90"
    assert resolver.resolve(adapter, host_source, allow_unverified=True) == {0x0059CB90}
    assert resolver.resolve(adapter, host_source) == set()
    assert resolver.body_identity(adapter, host_source)[0x0059CB90]["status"] == "implementation_source_mismatch"
    assert resolver.resolve(body, r"J:\frozen\src\frontend_menus.cpp") == {0x0059CB90}
    assert resolver.resolve(body) == set()  # A suffix without body evidence stays unknown.
    assert resolver.resolve("bsp::game::GameHudHost::update_ship_view_screen_0064d610",
                            host_source) == {0x0064D610}
    print("coverage census body identity regression: PASS")


if __name__ == "__main__":
    main()
