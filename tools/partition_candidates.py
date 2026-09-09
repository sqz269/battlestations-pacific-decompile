"""Group unnamed non-thunk functions into heuristic address segments for triage.

Link order provides locality clues, not guaranteed source-module boundaries. COMDAT folding,
templates and optimized code can break that correspondence. This tool clusters by static call graph (Louvain), smooths the
cluster labels along the address axis, cuts the axis into disjoint segments where the dominant
cluster changes, labels each segment with the distinctive strings its functions reference,
counts string-based binding hints and function-pointer runs, and computes leaf-first waves
of the thresholded candidate-only graph. A shared wave does not establish independence;
named dependencies and indirect dispatch require separate contract review before delegation.
Output: reports/library_inventory/candidate_partition.{json,md}.

Inputs: exports/bsp/functions.json (fresh snapshot), the call graph and data-reference map written
by tools/callgraph_sweep.py ({"0x401010": ["0xbf6340", ...]}), and the disk PE for strings.
"""
import argparse
import collections
import hashlib
import json
import re
from datetime import datetime, timezone
from pathlib import Path

import networkx as nx
import pefile

ROOT = Path(__file__).resolve().parents[1]
STOP = {'failed', 'error', 'file', 'name', 'null', 'true', 'false', 'invalid', 'cannot', 'could', 'from', 'with',
        'this', 'that', 'must', 'should', 'have', 'been', 'value', 'values', 'type', 'index', 'size', 'count',
        'data', 'load', 'loading', 'loaded', 'unknown', 'found', 'missing', 'while', 'when', 'after', 'before',
        'thistable', 'function', 'string', 'number', 'table', 'expected', 'entry', 'exit', 'warning'}


def load_pe(binary):
    pe = pefile.PE(str(binary), fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    strings, rdata = {}, None
    for section in pe.sections:
        name = section.Name.rstrip(b'\0').decode(errors='ignore')
        if name in ('.rdata', '.data'):
            data = section.get_data()
            va = base + section.VirtualAddress
            for match in re.finditer(rb'[\x20-\x7e]{5,}\x00', data):
                strings[va + match.start()] = match.group()[:-1].decode('ascii', errors='ignore')
            if name == '.rdata':
                rdata = (va, data)
    return strings, rdata


def tokens_of(text):
    out = []
    for tok in re.findall(r'[A-Za-z][A-Za-z0-9_]{3,}', text):
        if tok.startswith('luaMW_'):
            out.append('luaMW')
            tok = tok[6:]
        low = tok.lower()
        if low in STOP or low.startswith('fun_'):
            continue
        out.append(tok if re.match(r'^c[A-Z]', tok) else low)
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--callgraph', default=str(ROOT / 'exports/bsp/callgraph.json'),
                        help='from tools/callgraph_sweep.py')
    parser.add_argument('--datarefs', default=str(ROOT / 'exports/bsp/datarefs.json'))
    parser.add_argument('--resolution', type=float, default=0.5)
    parser.add_argument('--window', type=int, default=12, help='label smoothing half-window (candidates)')
    parser.add_argument('--min-segment', type=int, default=60)
    parser.add_argument('--strong', type=int, default=8, help='unique caller-target relationships needed for a strong inter-segment dependency')
    parser.add_argument('--output', default=str(ROOT / 'reports/library_inventory/candidate_partition'))
    args = parser.parse_args()

    inputs = {}
    def read_json(path):
        path = Path(path)
        raw = path.read_bytes()
        inputs[str(path)] = hashlib.sha256(raw).hexdigest()
        return json.loads(raw)

    config = read_json(ROOT / 'config/target.json')
    snapshot = read_json(ROOT / 'exports/bsp/snapshot.json')
    functions = read_json(ROOT / 'exports/bsp/functions.json')
    for path in (Path(__file__), Path(config['binary'])):
        inputs[str(path)] = hashlib.sha256(path.read_bytes()).hexdigest()
    by_addr = {int(r['address'], 16): r for r in functions}
    candidates = sorted(a for a, r in by_addr.items() if r['name'].startswith('FUN_') and not r['isThunk'])
    cand_set = set(candidates)
    callgraph = {int(k, 16): [int(v, 16) for v in vs] for k, vs in read_json(args.callgraph).items()}
    datarefs = {int(k, 16): [int(v, 16) for v in vs] for k, vs in read_json(args.datarefs).items()}
    strings, (rdata_va, rdata) = load_pe(config['binary'])

    func_tokens, df = {}, collections.Counter()
    for a in candidates:
        toks = [t for ref in datarefs.get(a, []) if ref in strings for t in tokens_of(strings[ref])]
        func_tokens[a] = toks
        df.update(set(toks))

    graph = nx.Graph()
    graph.add_nodes_from(candidates)
    directed = collections.Counter()
    for a in candidates:
        for callee in callgraph.get(a, []):
            if callee in cand_set and callee != a:
                directed[(a, callee)] += 1
                if graph.has_edge(a, callee):
                    graph[a][callee]['weight'] += 1
                else:
                    graph.add_edge(a, callee, weight=1)
    communities = nx.community.louvain_communities(graph, weight='weight', resolution=args.resolution, seed=1)
    label = {}
    for cid, members in enumerate(communities):
        for a in members:
            label[a] = cid

    # Smooth labels along the address axis, then cut into segments where the dominant label changes.
    seq = [label[a] for a in candidates]
    smooth = []
    for i in range(len(seq)):
        window = seq[max(0, i - args.window):i + args.window + 1]
        smooth.append(collections.Counter(window).most_common(1)[0][0])
    segments = []
    start = 0
    for i in range(1, len(smooth) + 1):
        if i == len(smooth) or smooth[i] != smooth[start]:
            segments.append([start, i, smooth[start]])
            start = i
    merged = []
    for seg in segments:
        if merged and (seg[1] - seg[0] < args.min_segment or merged[-1][1] - merged[-1][0] < args.min_segment):
            merged[-1][1] = seg[1]
            if seg[1] - seg[0] > merged[-1][1] - merged[-1][0] - (seg[1] - seg[0]):
                merged[-1][2] = seg[2]
        else:
            merged.append(seg)
    segment_of = {}
    for sid, (lo, hi, _) in enumerate(merged):
        for a in candidates[lo:hi]:
            segment_of[a] = sid

    vtables = []
    run = []
    for off in range(0, len(rdata) - 3, 4):
        ptr = int.from_bytes(rdata[off:off + 4], 'little')
        if ptr in by_addr:
            run.append(ptr)
        else:
            if len(run) >= 3:
                vtables.append(run)
            run = []
    vtables_per_segment = collections.Counter()
    if len(run) >= 3:
        vtables.append(run)
    for slots in vtables:
        segs = [segment_of[s] for s in slots if s in segment_of]
        if segs:
            vtables_per_segment[collections.Counter(segs).most_common(1)[0][0]] += 1

    total = len(candidates)
    summaries = []
    for sid, (lo, hi, dominant) in enumerate(merged):
        members = candidates[lo:hi]
        tf = collections.Counter()
        bindings = 0
        for a in members:
            toks = func_tokens[a]
            bindings += 'luaMW' in toks
            tf.update(set(toks))
        scored = sorted(((cnt / (1 + 100 * df[t] / total), t) for t, cnt in tf.items() if t != 'luaMW'), reverse=True)
        keywords = [t for _, t in scored[:10]]
        # The largest original community share, not a label retained from an
        # earlier small segment during merging. This is not semantic confidence.
        purity = max(collections.Counter(label[a] for a in members).values()) / len(members)
        out_calls, in_calls = collections.Counter(), collections.Counter()
        for (src, dst), w in directed.items():
            if segment_of[src] == sid and segment_of[dst] != sid:
                out_calls[segment_of[dst]] += w
            if segment_of[dst] == sid and segment_of[src] != sid:
                in_calls[segment_of[src]] += w
        summaries.append({'segment': sid, 'start': f'{members[0]:08x}', 'end': f'{members[-1]:08x}',
                          'candidates': len(members), 'purity': round(purity, 2), 'keywords': keywords,
                          'lua_bindings': bindings, 'vtables': vtables_per_segment.get(sid, 0),
                          'calls_out': dict(sorted(out_calls.items(), key=lambda kv: -kv[1])),
                          'calls_in': dict(sorted(in_calls.items(), key=lambda kv: -kv[1])),
                          'calls_out_total': sum(out_calls.values()), 'calls_in_total': sum(in_calls.values())})

    # Leaf-first waves: strong-dependency digraph, condensed over cycles, topological generations.
    dep = nx.DiGraph()
    dep.add_nodes_from(range(len(summaries)))
    for s in summaries:
        for dst, w in s['calls_out'].items():
            if w >= args.strong:
                dep.add_edge(s['segment'], dst)
    condensed = nx.condensation(dep)
    generation_of_scc = {}
    for gen, nodes in enumerate(nx.topological_generations(condensed.reverse())):
        for n in nodes:
            generation_of_scc[n] = gen
    for s in summaries:
        scc = condensed.graph['mapping'][s['segment']]
        s['wave'] = generation_of_scc[scc]
        s['cycle_group_size'] = len(condensed.nodes[scc]['members'])
        s['cycle_group'] = min(condensed.nodes[scc]['members'])

    # Refuse to publish a result assembled from inputs changed during the run.
    for path, digest in inputs.items():
        if hashlib.sha256(Path(path).read_bytes()).hexdigest() != digest:
            raise RuntimeError(f'Input changed during partitioning: {path}; rerun when stable')

    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.with_suffix('.json').write_text(json.dumps({'candidates': total, 'segments': summaries, 'vtables_total': len(vtables),
                                                    'resolution': args.resolution, 'window': args.window,
                                                    'min_segment': args.min_segment, 'strong': args.strong,
                                                    'utc': datetime.now(timezone.utc).isoformat(),
                                                    'snapshot': snapshot,
                                                    'networkx_version': nx.__version__,
                                                    'input_sha256': inputs,
                                                    'edge_weight': 'unique caller-target relationships, including direct tail jumps',
                                                    'purity_definition': 'largest unsmoothed Louvain community share within segment',
                                                    'scope': 'FUN_ non-thunks only; named functions, indirect calls and weak edges are not scheduling-complete'}, indent=1))
    lines = ['# Candidate partition: disjoint link-order segments\n',
             f'{total} unnamed non-thunk FUN_ candidates grouped into {len(summaries)} disjoint function-start ranges '
             f'(Louvain resolution {args.resolution}, smoothing window {args.window}, min segment {args.min_segment}); '
             f'{len(vtables)} candidate function-pointer runs found in .rdata. These are not validated vtables.\n',
             f'Waves use every candidate-to-candidate segment dependency with >= {args.strong} unique caller-target '
             'relationships (including direct tail jumps). JSON retains all edges; only the table display is shortened. '
             'Segments in one SCC share a wave and remain mutually dependent. Wave 0 means no outgoing strong edge '
             'to another SCC in this limited graph, not that the code is ready to implement independently.\n',
             'Named/incomplete functions, indirect calls, shared state and below-threshold dependencies remain outside '
             'this scheduling graph. FUN_ entries may have provisional inventory bookmarks. Purity is the largest '
             'Louvain community share, not source-module confidence. Lua binding counts are string-reference hints. '
             'Segment IDs can change after a new snapshot; delegate explicit function addresses and disjoint files.\n',
             '| Seg | Function-start range | Cands | Purity | Wave | SCC | SCC size | Lua hints | Pointer runs | Keywords | Strong deps out (segment:relationships) |',
             '|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|']
    for s in summaries:
        strong_deps = [(k, v) for k, v in s['calls_out'].items() if v >= args.strong]
        deps = ', '.join(f'{k}:{v}' for k, v in strong_deps[:6])
        if len(strong_deps) > 6:
            deps += f'; +{len(strong_deps) - 6} more in JSON'
        lines.append(f"| {s['segment']} | {s['start']}-{s['end']} | {s['candidates']} | {s['purity']} | {s['wave']} | "
                     f"{s['cycle_group']} | {s['cycle_group_size']} | {s['lua_bindings']} | {s['vtables']} | {', '.join(s['keywords'][:7])} | {deps} |")
    out.with_suffix('.md').write_text('\n'.join(lines) + '\n')
    print('\n'.join(lines))


if __name__ == '__main__':
    main()
