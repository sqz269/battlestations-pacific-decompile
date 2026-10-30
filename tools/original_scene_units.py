#!/usr/bin/env python3
"""Bounded read-only original-game entity/name discovery; no native calls.

00925180 reads registry=[game+19CC], head=[[registry+4]], next=entity+38,
and calls vtable+10 for the name. We interpret only tiny MOV/LEA-and-RET
getters from that slot, rather than executing them. Unknown getters fail closed.
"""
import argparse
import json
import struct
from pathlib import Path

from original_motion_capture import CaptureError, ProcessReader, SampleError, motion_sample, pointer, utc_now

GAME_GLOBAL = 0x00E188A8
CONTROLLED_GLOBAL = 0x00E188D8


def read_bytes(reader, address, size):
    # ProcessReader deliberately requires an aligned read base; x86 code and
    # char pointers need not be aligned. Retain that check and slice locally.
    if address < 0x10000 or address + size > 0x100000000:
        raise SampleError('invalid x86 byte span')
    aligned = address & ~3
    skip = address - aligned
    return reader.read(aligned, skip + size)[skip:]


def decoded_name(reader, entity, getter, image_end):
    """Interpret a deliberately small verified x86 getter, never call the game."""
    if not 0x00400000 <= getter < image_end - 16:
        raise SampleError("name getter is outside original image")
    code = read_bytes(reader, getter, 16)
    original_getter = getter
    if code[0] == 0xE9:  # one direct in-image thunk
        getter += 5 + struct.unpack_from('<i', code, 1)[0]
        if not 0x00400000 <= getter < image_end - 16:
            raise SampleError("name getter thunk leaves original image")
        code = read_bytes(reader, getter, 16)
    if code[:2] in (b'\x8b\x41', b'\x8d\x41') and code[3] == 0xC3:
        offset = struct.unpack_from('<b', code, 2)[0]
        indirect = code[0] == 0x8B
    elif code[:2] in (b'\x8b\x81', b'\x8d\x81') and code[6] == 0xC3:
        offset = struct.unpack_from('<i', code, 2)[0]
        indirect = code[0] == 0x8B
    else:
        raise SampleError(f"unsupported name getter 0x{original_getter:08X}: {code.hex()}")
    if not 0 <= offset <= 0x2000:
        raise SampleError("name getter member offset out of bounds")
    address = reader.u32(entity + offset) if indirect else entity + offset
    data = bytearray()
    if address:
        for index in range(256):
            value = read_bytes(reader, address + index, 1)[0]
            if not value:
                break
            data.append(value)
        else:
            raise SampleError("name is not terminated within 256 bytes")
    return data.decode('ascii', errors='backslashreplace'), {
        'virtual_slot': '0x10', 'getter': f'0x{original_getter:08X}',
        'resolved_getter': f'0x{getter:08X}', 'code': code.hex(),
        'member_offset': offset, 'indirect': indirect, 'name_address': f'0x{address:08X}'}


def snapshot(reader, identity, patterns, maximum):
    game = pointer(reader.u32(GAME_GLOBAL))
    registry = pointer(reader.u32(game + 0x19CC))
    head_owner = pointer(reader.u32(registry + 4))
    entity = reader.u32(head_owner)
    controlled = reader.u32(CONTROLLED_GLOBAL)
    seen, entities, units, errors, getters = set(), [], [], [], {}
    image_end = 0x00400000 + identity['pe']['image_size']
    while entity and len(seen) < maximum:
        pointer(entity, 0x3C)
        if entity in seen:
            raise SampleError(f"cycle in native entity links at 0x{entity:08X}")
        seen.add(entity)
        next_entity = reader.u32(entity + 0x38)
        record = {'entity_address': f'0x{entity:08X}'}
        try:
            vtable = pointer(reader.u32(entity), 0x14)
            getter = reader.u32(vtable + 0x10)
            name, proof = decoded_name(reader, entity, getter, image_end)
            record.update(native_name=name, name_proof=proof)
            getters[proof['getter']] = proof
            if not patterns or any(pattern.casefold() in name.casefold() for pattern in patterns):
                motion, _ = motion_sample(reader, entity)
                record['motion'] = motion
                units.append({'unit_address': f'0x{entity:08X}', 'label': name,
                              'native_name': name, 'controlled': entity == controlled})
        except SampleError as error:
            record['error'] = str(error)
            if len(errors) < 40:
                errors.append({'entity_address': record['entity_address'], 'error': str(error)})
        entities.append(record)
        entity = next_entity
    if reader.u32(GAME_GLOBAL) != game or reader.u32(game + 0x19CC) != registry:
        raise SampleError('game or registry changed during snapshot')
    mode = reader.u32(game + 0x1FE4)
    effective_difficulty = reader.u32(game + 0x6AC)
    return {'schema_version': 1, 'kind': 'original_game_named_unit_discovery',
            'recorded_utc': utc_now(), 'executable': identity,
            'game_address': f'0x{game:08X}', 'registry_address': f'0x{registry:08X}',
            'controlled_address': f'0x{controlled:08X}', 'entities_visited': len(seen),
            'mission_context': {'non_campaign_flag': mode,
                                'effective_difficulty_6ac': effective_difficulty,
                                'lua_GetDifficulty': 2 if mode else effective_difficulty,
                                'evidence': '008AE113..008AE130'},
            'complete_link_walk': entity == 0, 'match_patterns': patterns,
            'units': units, 'entities': entities, 'errors_head': errors,
            'getters': getters,
            'evidence': '00925180 assembly; docs/ORIGINAL_SCENE_UNIT_DISCOVERY.md',
            'limitations': ['Unsuspended non-atomic discovery; links/names may change.',
                            'Name getters interpreted only when their exact tiny instruction form is supported.',
                            'Valid motion layout is not proof of ship class or visible identity.']}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--pid', type=int, required=True)
    parser.add_argument('--expect-sha256', required=True)
    parser.add_argument('--match', action='append', default=[])
    parser.add_argument('--max-entities', type=int, default=4096)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if not 1 <= args.max_entities <= 8192:
        parser.error('--max-entities must be 1..8192')
    reader = ProcessReader(args.pid)
    try:
        identity = reader.identity(args.expect_sha256)
        result = snapshot(reader, identity, args.match, args.max_entities)
        result['pid'] = args.pid
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with args.output.open('x', encoding='utf-8', newline='\n') as handle:
            json.dump(result, handle, indent=2)
            handle.write('\n')
        print(json.dumps({'output': str(args.output), 'entities': result['entities_visited'],
                          'complete': result['complete_link_walk'], 'units': result['units'],
                          'getter_forms': list(result['getters'])}))
        return 0 if result['units'] else 2
    except (CaptureError, OSError, KeyError) as error:
        print(json.dumps({'error': str(error)}))
        return 1
    finally:
        reader.close()


if __name__ == '__main__':
    raise SystemExit(main())
