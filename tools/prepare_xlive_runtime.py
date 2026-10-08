"""Extract the game's genuine XLive runtime without executing its installer."""
import argparse
import hashlib
import json
import os
import shutil
import struct
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REDIST_SHA = "82b3088d0af4bad22bf8058659fb80654040fbafdccbd0fc8df996153f597217"
CAB_SHA = "861bd4b7df0df76d65db4ed64e3d3d662973857e9b444b19b2091377e396d0ea"
FILES = {
    "xlive.dll": ("xlive", "79da26ab6b2dc25936c3354087de0ba41da1a8b62924972e9100c29b95d34385"),
    "msidcrl40.dll": ("SDKCOMPONENTS_PPCRL_MSIDCRL40.DLL.1312FADD_90E2_487F_B4BC_5B3F1469FB3C",
                      "623cb6ca98e566357abbd0e76b15713921e1d7e1144c0c4f589a0407c7eff1ee"),
    "ppcrlconfig.dll": ("SDKCOMPONENTS_PPCRL_PPCRLCONFIG.DLL.1312FADD_90E2_487F_B4BC_5B3F1469FB3C",
                        "649557d6349ea0658808952c1bbf9a111ec2283c29345c7f904919fd599e5d61"),
    "xlivefnt.dll": ("xlivefnt", "a07e20c09a0c3eac2ed3e6288d67060e82b70595053153866b1cfb4f9958f9a4"),
}


def sha(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def prepare(game_root, output, seven_zip):
    output = output.resolve()
    if not output.is_relative_to((ROOT / "local").resolve()):
        raise ValueError("Private runtime output must stay under this checkout's local directory")
    for name, (_, digest) in FILES.items():
        destination = output / name
        if destination.exists() and sha(destination) != digest:
            raise ValueError(f"Refusing to overwrite an unexpected runtime file: {destination}")
    if all((output / name).is_file() for name in FILES):
        return {"directory": str(output), "files": {name: sha(output / name) for name in FILES},
                "extracted": False, "installer_executed": False}

    source = game_root / "redist/gfwlivesetup.exe"
    if sha(source) != REDIST_SHA:
        raise ValueError("The bundled GFWL redistributable is not the verified 2.0.0673.0 package")
    if not seven_zip.is_file():
        raise FileNotFoundError(f"7-Zip is required for offline extraction: {seven_zip}")
    local = (ROOT / "local").resolve()
    local.mkdir(exist_ok=True)
    # Cleanup is confined to this verified checkout-local temporary directory.
    with tempfile.TemporaryDirectory(prefix="gfwl-extract-", dir=local) as temporary:
        workspace = Path(temporary).resolve()
        if not workspace.is_relative_to(local):
            raise ValueError("Extraction workspace escaped the checkout")
        cabinet = workspace / "payload.cab"
        with source.open("rb") as stream, cabinet.open("wb") as target:
            stream.seek(0x8C00)
            header = stream.read(36)
            if header[:4] != b"MSCF" or struct.unpack_from("<I", header, 8)[0] != 178584234:
                raise ValueError("Verified redistributable CAB header changed")
            target.write(header)
            remaining = 178584234 - len(header)
            while remaining:
                block = stream.read(min(1024 * 1024, remaining))
                if not block:
                    raise ValueError("Truncated redistributable CAB")
                target.write(block)
                remaining -= len(block)
        if sha(cabinet) != CAB_SHA:
            raise ValueError("Embedded CAB identity changed")

        def extract(archive, names):
            result = subprocess.run([str(seven_zip), "e", str(archive), "-y",
                                     f"-o{workspace}", *names], capture_output=True, text=True)
            if result.returncode:
                raise RuntimeError(f"7-Zip extraction failed ({result.returncode}): {result.stderr.strip()}")

        extract(cabinet, ["pkg/xLiveRedist.msi"])
        extract(workspace / "xLiveRedist.msi", ["XLive.cab"])
        extract(workspace / "XLive.cab", [entry for entry, _ in FILES.values()])
        for name, (entry, digest) in FILES.items():
            if sha(workspace / entry) != digest:
                raise ValueError(f"Extracted runtime identity changed: {name}")
        output.mkdir(parents=True, exist_ok=True)
        for name, (entry, _) in FILES.items():
            shutil.copyfile(workspace / entry, output / name)
    result = {"directory": str(output), "source": str(source), "source_sha256": REDIST_SHA,
              "cab_sha256": CAB_SHA, "files": {name: sha(output / name) for name in FILES},
              "extracted": True, "installer_executed": False}
    (output / "provenance.json").write_text(json.dumps(result, indent=2) + "\n")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-root", type=Path)
    parser.add_argument("--output", type=Path, default=ROOT / "local/gfwl-private-runtime")
    parser.add_argument("--seven-zip", type=Path,
                        default=Path(os.environ.get("ProgramFiles", "C:/Program Files")) / "7-Zip/7z.exe")
    args = parser.parse_args()
    game_root = args.game_root
    if game_root is None:
        game_root = Path(json.loads((ROOT / "config/target.json").read_text())["binary"]).parent
    print(json.dumps(prepare(game_root.resolve(), args.output, args.seven_zip), indent=2))


if __name__ == "__main__":
    main()
