"""Wreck-sink reach over a frozen bsp_game log list (packets cc13-leak92, cc13-sinkrng).

  freeze: python tools/cc13_sink_log_scan.py freeze --until 2026-10-19T12:30 --out LIST DIR...
          (logs over 100 KB written in the 36 hours before --until, UTC; each line is
          "path<TAB>size<TAB>mtime", mtime in UTC ISO seconds)
  scan:   python tools/cc13_sink_log_scan.py scan LIST
          (one row per log, then per-mission summaries; warns when a log's size or
          mtime no longer matches the frozen line)
"""
import argparse, collections, datetime, os, re

P_MISSION = re.compile(r"menu_select=(\S+)")
P_STREAMS = re.compile(r"per-consumer random streams ON")
P_POSTS = re.compile(r"LeakManager::route_ship_sink_92h\s+0077c2a0\s+\S+\s+calls=(\d+)")
P_WRECK_HITS = re.compile(r"summary live hull leak .*applied_wreck=(\d+)")
P_WRECK = re.compile(r"^  wreck (.+?)\s+leaks bound=1 built=1 redistributed=1")
P_DEATH = re.compile(r"^death row: victim=(.+?) t=([-0-9.]+)")
P_PLANE = re.compile(r"^(val rack drop|bomb drop scatter): unit=.+? t=([-0-9.]+)")
P_SINK = re.compile(r"^sunk ship kill depth: unit=.+? t=[-0-9.]+ sinkTime=([-0-9.]+)")


def freeze(args):
    until = datetime.datetime.fromisoformat(args.until).replace(tzinfo=datetime.timezone.utc).timestamp()
    paths = []
    for d in args.dirs:
        for name in sorted(os.listdir(d)):
            p = os.path.join(d, name)
            if name.endswith(".log") and os.path.getsize(p) >= 100000 \
                    and until - 36 * 3600 <= os.path.getmtime(p) <= until:
                paths.append(p)
    with open(args.out, "w") as fh:
        fh.write("".join(stamp(p) + "\n" for p in paths))
    print(len(paths), "logs ->", args.out)


def mtime_iso(path):
    t = datetime.datetime.fromtimestamp(int(os.path.getmtime(path)), datetime.timezone.utc)
    return t.strftime("%Y-%m-%dT%H:%M:%SZ")


def stamp(path):
    return f"{path}\t{os.path.getsize(path)}\t{mtime_iso(path)}"


def scan_one(path):
    r = dict(mission=None, streams=False, posts=0, wreck_hits=0, wrecks=set(), deaths=[],
             plane=[], sink_times=[])
    with open(path, "r", errors="replace") as fh:
        for line in fh:
            if r["mission"] is None:
                m = P_MISSION.search(line)
                if m: r["mission"] = m.group(1)
            if P_STREAMS.search(line): r["streams"] = True
            for pat, key in ((P_POSTS, "posts"), (P_WRECK_HITS, "wreck_hits")):
                m = pat.search(line)
                if m: r[key] = int(m.group(1))
            m = P_WRECK.match(line)
            if m: r["wrecks"].add(m.group(1).strip())
            m = P_DEATH.match(line)
            if m: r["deaths"].append((m.group(1).strip(), float(m.group(2))))
            m = P_PLANE.match(line)
            if m: r["plane"].append(float(m.group(2)))
            m = P_SINK.match(line)
            if m: r["sink_times"].append(float(m.group(1)))
    sinks = sorted(t for v, t in r["deaths"] if v in r["wrecks"])
    r["first_sink"] = sinks[0] if sinks else None
    r["plane_after"] = 0 if not sinks else (
        sum(1 for v, t in r["deaths"] if v not in r["wrecks"] and t > sinks[0])
        + sum(1 for t in r["plane"] if t > sinks[0]))
    return r


def scan(args):
    rows = [line.split("\t") for line in open(args.list).read().split("\n") if line]
    paths = [r[0] for r in rows]
    by = collections.defaultdict(list)
    mismatches = 0
    for row in rows:
        p = row[0]
        if len(row) >= 3 and os.path.exists(p) and stamp(p) != "\t".join(row[:3]):
            mismatches += 1
            print("WARNING changed since freeze:", p, "frozen", row[1], row[2],
                  "now", os.path.getsize(p), mtime_iso(p))
        try:
            r = scan_one(p)
        except OSError:
            print("unreadable", p)
            continue
        by[r["mission"]].append((os.path.basename(p), r))
        st = r["sink_times"]
        print(r["mission"], os.path.basename(p), "streams" if r["streams"] else "shared",
              f"posts={r['posts']} wreck_hits={r['wreck_hits']} first_sink={r['first_sink']}",
              f"plane_after={r['plane_after']}",
              f"sinkTime={min(st):.1f}-{max(st):.1f}" if st else "sinkTime=-")
    print("== per mission")
    for m, rows in sorted(by.items(), key=lambda kv: str(kv[0])):
        posts = sorted(r["posts"] for _, r in rows)
        st = [t for _, r in rows for t in r["sink_times"]]
        shared = [r for _, r in rows if not r["streams"]]
        print(m, f"logs={len(rows)} shared={len(shared)}",
              f"posts={posts[0]}/{posts[len(posts) // 2]}/{posts[-1]}",
              f"wreck_hit_logs={sum(1 for _, r in rows if r['wreck_hits'])}",
              f"max_wreck_hits={max(r['wreck_hits'] for _, r in rows)}",
              f"shared_live={sum(1 for r in shared if r['first_sink'] is not None and r['plane_after'])}",
              f"sinkTime={min(st):.1f}-{max(st):.1f}" if st else "sinkTime=-")
    print("logs", len(paths), "changed_since_freeze", mismatches)


ap = argparse.ArgumentParser()
sub = ap.add_subparsers(dest="cmd", required=True)
f = sub.add_parser("freeze"); f.add_argument("--until", required=True); f.add_argument("--out", required=True)
f.add_argument("dirs", nargs="+")
s = sub.add_parser("scan"); s.add_argument("list")
a = ap.parse_args()
freeze(a) if a.cmd == "freeze" else scan(a)
