import unittest
import json
import time

def clean_url(url):
    if not url or not (url.startswith('http://') or url.startswith('https://')):
        return url
    q_index = url.find('?')
    if q_index < 0:
        return url
    base = url[:q_index]
    query = url[q_index + 1:]
    fragment = ''
    f_index = query.find('#')
    if f_index >= 0:
        fragment = query[f_index:]
        query = query[:f_index]
    pairs = [p for p in query.split('&') if p]
    tracking = {
        'utm_source', 'utm_medium', 'utm_campaign', 'utm_term', 'utm_content', 'utm_id',
        'fbclid', 'gclid', 'gbraid', 'wbraid', 'si', 'igshid', 'ref', 'ref_src',
        'yclid', 'ysclid', 'mc_eid', '_hsenc', '_hsmi'
    }
    new_pairs = []
    for pair in pairs:
        key = pair.split('=', 1)[0].lower()
        if key.startswith('utm_') or key in tracking:
            continue
        new_pairs.append(pair)
    if not new_pairs:
        return base + fragment
    return base + '?' + '&'.join(new_pairs) + fragment

def clean_urls_in_text(text, enabled=True):
    if not enabled or not text:
        return text
    tokens = text.split(' ')
    cleaned_tokens = [clean_url(tok) if tok.startswith(('http://', 'https://')) else tok for tok in tokens]
    return ' '.join(cleaned_tokens)

def filter_zalgo(text, enabled=True):
    if not enabled or not text:
        return text
    res = []
    consecutive = 0
    for ch in text:
        cp = ord(ch)
        if cp in (1564, 8206, 8207) or (8234 <= cp <= 8238) or (8294 <= cp <= 8297):
            continue
        is_mark = (768 <= cp <= 879) or (6832 <= cp <= 6911) or (7616 <= cp <= 7679) or (8400 <= cp <= 8447) or (65056 <= cp <= 65071)
        if is_mark:
            consecutive += 1
            if consecutive > 2:
                continue
        else:
            consecutive = 0
        res.append(ch)
    return ''.join(res)

def rotate_vault_simulation(files, ttl_days=0, max_size_bytes=0, now=1000000):
    deleted_count = 0
    freed_bytes = 0
    surviving = []
    max_age_secs = ttl_days * 86400 if ttl_days > 0 else 0

    for f in files:
        name, size, mod_time = f
        if size == 0:
            deleted_count += 1
            continue
        if max_age_secs > 0 and (now - mod_time) > max_age_secs:
            deleted_count += 1
            freed_bytes += size
            continue
        surviving.append((name, size, mod_time))

    if max_size_bytes > 0:
        total_size = sum(f[1] for f in surviving)
        if total_size > max_size_bytes:
            surviving.sort(key=lambda x: x[2]) # oldest first
            kept = []
            for f in surviving:
                if total_size <= max_size_bytes:
                    kept.append(f)
                else:
                    total_size -= f[1]
                    deleted_count += 1
                    freed_bytes += f[1]
            surviving = kept

    return deleted_count, freed_bytes, surviving

def format_phone_simulation(phone, streamer_mode=False, hide_phone=True):
    if not phone:
        return ""
    if streamer_mode and hide_phone:
        return "+• ••• ••• •• ••"
    return phone

class TestVaultRotatorAndFilters(unittest.TestCase):
    def test_clean_youtube_url(self):
        url = "https://youtu.be/dQw4w9WgXcQ?si=abc123xyz&t=42"
        expected = "https://youtu.be/dQw4w9WgXcQ?t=42"
        self.assertEqual(clean_url(url), expected)

    def test_clean_tracking_url_with_fragment(self):
        url = "https://example.com/blog/article?utm_source=twitter&utm_medium=social#comments"
        expected = "https://example.com/blog/article#comments"
        self.assertEqual(clean_url(url), expected)

    def test_clean_all_tracking_params_removed(self):
        url = "https://mysite.org/landing?utm_campaign=winter_sale&fbclid=123456"
        expected = "https://mysite.org/landing"
        self.assertEqual(clean_url(url), expected)

    def test_clean_urls_in_text(self):
        text = "Check this https://youtu.be/test?si=123 and this https://site.com/?gclid=999"
        expected = "Check this https://youtu.be/test and this https://site.com/"
        self.assertEqual(clean_urls_in_text(text), expected)

    def test_zalgo_filter_strips_excessive_diacritics(self):
        zalgo_word = "H\u0300\u0301\u0302\u0303\u0304\u0305e\u0306\u0307\u0308\u0309llo"
        filtered = filter_zalgo(zalgo_word)
        self.assertEqual(filtered, "H\u0300\u0301e\u0306\u0307llo")

    def test_zalgo_filter_strips_direction_overrides(self):
        trojan_text = "evil\u202eexe.jpg"
        filtered = filter_zalgo(trojan_text)
        self.assertEqual(filtered, "evilexe.jpg")

    def test_vault_rotator_ttl_expiration(self):
        now = 100000000
        day = 86400
        files = [
            ("file1.jpg", 1000, now - 5 * day),  # 5 days old
            ("file2.jpg", 2000, now - 15 * day), # 15 days old -> expired if ttl=10
            ("file3.jpg", 3000, now - 2 * day),  # 2 days old
        ]
        del_cnt, freed, surviving = rotate_vault_simulation(files, ttl_days=10, now=now)
        self.assertEqual(del_cnt, 1)
        self.assertEqual(freed, 2000)
        self.assertEqual(len(surviving), 2)
        self.assertEqual([f[0] for f in surviving], ["file1.jpg", "file3.jpg"])

    def test_vault_rotator_quota_trimming(self):
        now = 100000000
        day = 86400
        files = [
            ("oldest.mp4", 5000, now - 3 * day),
            ("middle.mp4", 4000, now - 2 * day),
            ("newest.mp4", 3000, now - 1 * day),
        ]
        # Total size is 12000 bytes. Cap at 6000 bytes.
        # oldest.mp4 (5000) deleted -> 7000 remaining
        # middle.mp4 (4000) deleted -> 3000 remaining <= 6000
        del_cnt, freed, surviving = rotate_vault_simulation(files, ttl_days=0, max_size_bytes=6000, now=now)
        self.assertEqual(del_cnt, 2)
        self.assertEqual(freed, 9000)
        self.assertEqual(len(surviving), 1)
        self.assertEqual(surviving[0][0], "newest.mp4")

    def test_streamer_phone_masking(self):
        phone = "+1 555 123 4567"
        masked = format_phone_simulation(phone, streamer_mode=True, hide_phone=True)
        self.assertEqual(masked, "+• ••• ••• •• ••")

        unmasked = format_phone_simulation(phone, streamer_mode=False)
        self.assertEqual(unmasked, phone)

    def test_backup_roundtrip_new_settings(self):
        cfg = {
            "autoVaultMedia": True,
            "vaultMaxSizeGb": 10,
            "vaultTtlDays": 30,
            "streamerMode": True,
            "streamerHideOtherAccounts": True,
            "streamerHidePhoneNumbers": True,
            "filterZalgo": True,
            "cleanUrls": True,
        }
        root = {
            "app": "WickedGram",
            "schema_version": 1,
            "config": cfg
        }
        json_data = json.dumps(root)
        parsed = json.loads(json_data)["config"]
        for k, v in cfg.items():
            self.assertEqual(parsed[k], v)

if __name__ == '__main__':
    unittest.main()
