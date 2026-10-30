"""One synthetic regression for exact-entry positives, unknowns and null counts."""
import json
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import coverage_census as c


def main():
    starts = [0x401000, 0x401010, 0x401020]
    functions = {a: dict(name=f"game_{a:x}", board_game=True, category="core", tag_category=None,
        eh_funclet=False, recon_kind="function", recon_status="reconstructed_build_tested", recon_source="src/test.cpp") for a in starts}
    functions[0x400ff0] = {**functions[starts[0]], "name": "Unwind@parent", "board_game": False, "eh_funclet": True}
    catalog = SimpleNamespace(functions=functions, range_starts=set(functions), parents={0x400ff0: starts[0]},
        callers={starts[0]: 1, starts[1]: 9}, ranges=c.RangeIndex([(0x400ff0, 0x401040, 0x400ff0)]))
    with tempfile.TemporaryDirectory() as temporary:
        root = Path(temporary)
        image_path, index, wire = (root / name for name in ("image.bin", "index.bin", "wire.json"))
        image_path.write_bytes(b"\x55" * 64); index.write_bytes(b"frozen-index"); wire.write_text("{}")
        image = dict(path=str(image_path), sha256=c.sha256(image_path), machine=0x14c, image_base=0x400000,
            image_size=0x2000, sections=[dict(rva=0x1000, file_size=64, file_offset=0, flags=0x20000000)])
        pins = dict(image_sha256=image["sha256"], image_base=image["image_base"], image_size=image["image_size"],
                    index_sha256=c.sha256(index), wire_sha256=c.sha256(wire))
        manifest = root / "targets.json"
        manifest.write_text(json.dumps(dict(schema="bsp.original-entry.targets.v1", **pins,
            points=[dict(address=f"{a:08x}", expected_byte=0x55, board_target=a != starts[1]) for a in starts])))
        identity = dict(run_id="synthetic-only", pid=7, process_created_filetime=134000000000000007)
        records = [dict(schema="bsp.original-entry.trace.v1", kind="header", **pins, **identity,
                        manifest_sha256=c.sha256(manifest))]
        records += [dict(kind="entry_hit", address=f"{a:08x}", pid=7, thread_id=19, sequence=n)
                    for n, a in enumerate(starts[:2], 1)]
        records += [dict(schema="bsp.original-entry.trace.v1", kind="terminal", **identity,
            hit_count=2, reason="game_time_target", stop_classification="forced_window_stop", admitted=True,
            clock=dict(game_time=240.125, fixed_step_count=4802), source_preservation_verified=True,
            final_job_active=[0], same_scene_verified=True, complete_window=True, root_exit_code=0xe0000011)]
        trace = root / "entries.jsonl"

        def read():
            trace.write_text("".join(json.dumps(row) + "\n" for row in records))
            return c.read_original_entries(trace, manifest, c.sha256(manifest), index, wire, image, catalog)

        original, receipt = read()
        bsp = dict(hits={starts[2]: {c.Observation(0x1020, 1)}}, complete=True,
                   present=set(starts), possible=set(), entry_counts={}, metadata={})
        with patch.object(c, "host_mentions", return_value={}):
            result = c.report(catalog, c.Resolver({}), original, bsp, [receipt], [])
        rows = {int(row["address"], 16): row for row in result["per_original_address"]}
        assert rows[starts[0]]["orig_executed"] is True
        assert rows[starts[2]]["orig_executed"] is None and rows[0x400ff0]["orig_executed"] is None
        assert all(row["orig_distinct_blocks"] is None and row["orig_entry_count"] is None for row in rows.values())
        assert result["original_details"]["undefined_chunks_executed"] is None
        summary = result["summary"]["original"]
        assert (summary["orig_live"], summary["all_game_functions_executed"], summary["outside_board_target_functions_executed"]) == (1, 2, 1)
        assert summary["eh_funclets_executed"] is None and result["summary"]["bsp"]["board_game_functions_executed"] == 1
        assert result["wiring_frontier"] == [f"{a:08x}" for a in reversed(starts[:2])]
        assert rows[starts[0]]["orig_first_entry_sequence"] == 1 and not result["orig_no_bsp_yes"]
        records[1]["address"] = "00401001"
        try:
            read()
        except c.CensusError:
            pass
        else:
            raise AssertionError("interior hit was accepted as a function entry")
        records[1]["address"] = "00401000"; records[-1]["complete_window"] = False
        try:
            read()
        except c.CensusError:
            pass
        else:
            raise AssertionError("partial window was accepted as qualified")
    print("coverage census exact-entry/unknown/null regression: PASS")


if __name__ == "__main__":
    main()
