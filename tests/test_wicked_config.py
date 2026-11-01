import unittest

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

if __name__ == "__main__":
    unittest.main()
