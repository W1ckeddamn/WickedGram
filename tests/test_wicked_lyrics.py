import unittest
import json

def parse_lrc_line(line):
    line = line.strip()
    if not line.startswith('[') or ']' not in line:
        return None
    close_idx = line.find(']')
    time_str = line[1:close_idx]
    text = line[close_idx + 1:].strip()
    if ':' not in time_str:
        return None
    parts = time_str.split(':')
    try:
        minutes = int(parts[0])
        sec_parts = parts[1].replace(':', '.').split('.')
        seconds = int(sec_parts[0])
        ms = 0
        if len(sec_parts) > 1:
            ms_str = sec_parts[1]
            if len(ms_str) == 1:
                ms = int(ms_str) * 100
            elif len(ms_str) == 2:
                ms = int(ms_str) * 10
            elif len(ms_str) >= 3:
                ms = int(ms_str[:3])
        return (minutes * 60 + seconds) * 1000 + ms, text
    except ValueError:
        return None

def parse_synced_lines(lrc_text):
    if not lrc_text:
        return []
    lines = []
    for raw in lrc_text.splitlines():
        parsed = parse_lrc_line(raw)
        if parsed is not None:
            time_ms, text = parsed
            lines.append({'time_ms': time_ms, 'text': text})
    lines.sort(key=lambda x: x['time_ms'])
    return lines

def find_current_line_index(lines, current_ms):
    if not lines:
        return -1
    active = -1
    for idx, item in enumerate(lines):
        if item['time_ms'] <= current_ms:
            active = idx
        else:
            break
    return active

def parse_lrclib_json(data, fallback_title='', fallback_artist=''):
    if not data:
        return None
    synced = data.get('syncedLyrics') or ''
    plain = data.get('plainLyrics') or ''
    track = data.get('trackName') or fallback_title
    artist = data.get('artistName') or fallback_artist

    if synced:
        lines = parse_synced_lines(synced)
        if lines:
            return {
                'is_synced': True,
                'lines': lines,
                'plain_text': plain,
                'title': track,
                'artist': artist,
                'source': 'LRCLIB (Синхронизировано)'
            }
    if plain:
        return {
            'is_synced': False,
            'lines': [],
            'plain_text': plain,
            'title': track,
            'artist': artist,
            'source': 'LRCLIB'
        }
    return None

class TestWickedLyrics(unittest.TestCase):
    def test_parse_synced_lines_timestamps(self):
        sample = """
        [ar:Queen]
        [ti:Bohemian Rhapsody]
        [00:00.15]Is this the real life?
        [00:07.13]Caught in a landslide
        [01:14]Open your eyes
        [02:05.123]Look up to the skies
        """
        lines = parse_synced_lines(sample)
        self.assertEqual(len(lines), 4)
        self.assertEqual(lines[0], {'time_ms': 150, 'text': 'Is this the real life?'})
        self.assertEqual(lines[1], {'time_ms': 7130, 'text': 'Caught in a landslide'})
        self.assertEqual(lines[2], {'time_ms': 74000, 'text': 'Open your eyes'})
        self.assertEqual(lines[3], {'time_ms': 125123, 'text': 'Look up to the skies'})

    def test_find_current_line_index(self):
        lines = [
            {'time_ms': 1000, 'text': 'Line 1'},
            {'time_ms': 5000, 'text': 'Line 2'},
            {'time_ms': 10000, 'text': 'Line 3'},
        ]
        self.assertEqual(find_current_line_index(lines, 500), -1)
        self.assertEqual(find_current_line_index(lines, 1000), 0)
        self.assertEqual(find_current_line_index(lines, 3000), 0)
        self.assertEqual(find_current_line_index(lines, 5000), 1)
        self.assertEqual(find_current_line_index(lines, 7500), 1)
        self.assertEqual(find_current_line_index(lines, 10000), 2)
        self.assertEqual(find_current_line_index(lines, 25000), 2)

    def test_parse_lrclib_synced_payload(self):
        payload = {
            'trackName': 'Numb',
            'artistName': 'Linkin Park',
            'syncedLyrics': '[00:20.10]I\'m tired of being what you want me to be\n[00:25.50]Feeling so faithless',
            'plainLyrics': 'I\'m tired of being what you want me to be\nFeeling so faithless'
        }
        res = parse_lrclib_json(payload)
        self.assertIsNotNone(res)
        self.assertTrue(res['is_synced'])
        self.assertEqual(len(res['lines']), 2)
        self.assertEqual(res['title'], 'Numb')
        self.assertEqual(res['artist'], 'Linkin Park')

    def test_parse_lrclib_plain_only_payload(self):
        payload = {
            'trackName': 'Yesterday',
            'artistName': 'The Beatles',
            'plainLyrics': 'Yesterday all my troubles seemed so far away...'
        }
        res = parse_lrclib_json(payload)
        self.assertIsNotNone(res)
        self.assertFalse(res['is_synced'])
        self.assertEqual(res['plain_text'], 'Yesterday all my troubles seemed so far away...')

    def test_backup_roundtrip_lyrics_setting(self):
        cfg = {
            "lyricsInPlayer": True
        }
        root = {
            "app": "WickedGram",
            "schema_version": 1,
            "config": cfg
        }
        json_str = json.dumps(root)
        parsed = json.loads(json_str)["config"]
        self.assertTrue(parsed["lyricsInPlayer"])

if __name__ == '__main__':
    unittest.main()
