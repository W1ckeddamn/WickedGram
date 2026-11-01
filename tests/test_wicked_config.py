import unittest
import json
import time

def fix_social_links(text, enabled=True):
    if not enabled or not text:
        return text
    result = text
    if "twitter.com/" in result:
        result = result.replace("https://twitter.com/", "https://fxtwitter.com/")
        result = result.replace("http://twitter.com/", "https://fxtwitter.com/")
    if "x.com/" in result:
        result = result.replace("https://x.com/", "https://fixupx.com/")
        result = result.replace("http://x.com/", "https://fixupx.com/")
    if "instagram.com/" in result:
        result = result.replace("https://www.instagram.com/", "https://ddinstagram.com/")
        result = result.replace("https://instagram.com/", "https://ddinstagram.com/")
        result = result.replace("http://instagram.com/", "https://ddinstagram.com/")
    if "tiktok.com/" in result:
        result = result.replace("https://www.tiktok.com/", "https://vxtiktok.com/")
        result = result.replace("https://tiktok.com/", "https://vxtiktok.com/")
    if "reddit.com/" in result:
        result = result.replace("https://www.reddit.com/", "https://rxddit.com/")
        result = result.replace("https://reddit.com/", "https://rxddit.com/")
    if "pixiv.net/" in result:
        result = result.replace("https://www.pixiv.net/", "https://phixiv.net/")
        result = result.replace("https://pixiv.net/", "https://phixiv.net/")
    return result

def mark_deleted_text(original):
    prefix = "🗑 [Удалено] "
    if not original:
        return "🗑 [Удалено]"
    if not original.startswith(prefix):
        return prefix + original
    return original

def apply_edit_history(prev_text, new_text, enabled=True):
    if enabled and prev_text and prev_text != new_text:
        if "[Изменено. Было:" not in prev_text:
            return f"{new_text}\n\n✏️ [Изменено. Было: {prev_text}]"
    return new_text

def estimate_reg_date(user_id, table):
    if not table or user_id <= 0:
        return 0
    if user_id <= table[0][0]:
        return table[0][1]
    if user_id >= table[-1][0]:
        return table[-1][1]
    for i in range(len(table) - 1):
        id0, t0 = table[i]
        id1, t1 = table[i + 1]
        if id0 <= user_id <= id1:
            fraction = (user_id - id0) / (id1 - id0)
            return int(t0 + (t1 - t0) * fraction)
    return table[-1][1]

def export_settings_json(config_dict):
    root = {
        "app": "WickedGram",
        "schema_version": 1,
        "export_timestamp": int(time.time()),
        "config": config_dict
    }
    return json.dumps(root, indent=2)

def import_settings_json(json_str):
    root = json.loads(json_str)
    if root.get("app") not in ("WickedGram", "100gram"):
        return None
    return root.get("config", {})

class TestWickedFeatures(unittest.TestCase):
    def test_twitter_replacement(self):
        url = "Check this out https://twitter.com/user/status/123456"
        expected = "Check this out https://fxtwitter.com/user/status/123456"
        self.assertEqual(fix_social_links(url), expected)

    def test_x_replacement(self):
        url = "Look https://x.com/post/999"
        expected = "Look https://fixupx.com/post/999"
        self.assertEqual(fix_social_links(url), expected)

    def test_instagram_replacement(self):
        url = "Reel https://www.instagram.com/reel/abc123xyz/"
        expected = "Reel https://ddinstagram.com/reel/abc123xyz/"
        self.assertEqual(fix_social_links(url), expected)

    def test_tiktok_replacement(self):
        url = "Video https://www.tiktok.com/@creator/video/111222"
        expected = "Video https://vxtiktok.com/@creator/video/111222"
        self.assertEqual(fix_social_links(url), expected)

    def test_reddit_replacement(self):
        url = "Thread https://www.reddit.com/r/programming/comments/foo"
        expected = "Thread https://rxddit.com/r/programming/comments/foo"
        self.assertEqual(fix_social_links(url), expected)

    def test_disabled_fixer(self):
        url = "https://x.com/test"
        self.assertEqual(fix_social_links(url, enabled=False), url)

    def test_anti_delete_text(self):
        self.assertEqual(mark_deleted_text("Привет"), "🗑 [Удалено] Привет")
        self.assertEqual(mark_deleted_text(""), "🗑 [Удалено]")
        self.assertEqual(mark_deleted_text("🗑 [Удалено] Уже"), "🗑 [Удалено] Уже")

    def test_edit_history_annotation(self):
        old = "Секретное сообщение"
        new = "Обычное сообщение"
        annotated = apply_edit_history(old, new, enabled=True)
        self.assertIn("Обычное сообщение", annotated)
        self.assertIn("✏️ [Изменено. Было: Секретное сообщение]", annotated)

    def test_registration_date_interpolation(self):
        table = [(100, 1000), (200, 2000), (300, 3000)]
        self.assertEqual(estimate_reg_date(50, table), 1000)
        self.assertEqual(estimate_reg_date(100, table), 1000)
        self.assertEqual(estimate_reg_date(150, table), 1500)
        self.assertEqual(estimate_reg_date(200, table), 2000)
        self.assertEqual(estimate_reg_date(250, table), 2500)
        self.assertEqual(estimate_reg_date(400, table), 3000)

    def test_backup_export_and_import(self):
        cfg = {
            "bypassContentProtection": True,
            "ghostMode": True,
            "antiDeleteMessages": True,
            "saveEditHistory": True,
            "showRegistrationDate": True,
            "showOriginalName": True
        }
        exported = export_settings_json(cfg)
        imported = import_settings_json(exported)
        self.assertIsNotNone(imported)
        self.assertEqual(imported["bypassContentProtection"], True)
        self.assertEqual(imported["ghostMode"], True)

    def test_backup_rejects_invalid_app(self):
        fake = json.dumps({"app": "UnknownApp", "config": {}})
        self.assertIsNone(import_settings_json(fake))

if __name__ == "__main__":
    unittest.main()
