import unittest
import json
import time

# Simulation of NetworkBoostMode
class NetworkBoostMode:
    Off = 0
    Fast = 1
    Ultra = 2

kDownloadPartSize = 128 * 1024

class NetworkBoostLogic:
    @staticmethod
    def initial_download_sessions_count(mode):
        if mode == NetworkBoostMode.Ultra:
            return 8
        elif mode == NetworkBoostMode.Fast:
            return 4
        return 1

    @staticmethod
    def max_download_sessions_count():
        return 8

    @staticmethod
    def initial_download_waited_amount(mode):
        if mode == NetworkBoostMode.Ultra:
            return 32 * kDownloadPartSize
        elif mode == NetworkBoostMode.Fast:
            return 16 * kDownloadPartSize
        return 4 * kDownloadPartSize

    @staticmethod
    def max_download_waited_amount(mode):
        if mode == NetworkBoostMode.Ultra:
            return 64 * kDownloadPartSize
        elif mode == NetworkBoostMode.Fast:
            return 32 * kDownloadPartSize
        return 16 * kDownloadPartSize

    @staticmethod
    def target_upload_sessions_count(mode, file_size, file_size_aware=True):
        if mode == NetworkBoostMode.Off:
            return 4
        if file_size_aware:
            if 0 < file_size < 10 * 1024 * 1024:
                return 4
            if file_size >= 100 * 1024 * 1024 and mode == NetworkBoostMode.Ultra:
                return 8
            return 8 if mode == NetworkBoostMode.Ultra else 6
        return 8 if mode == NetworkBoostMode.Ultra else 6

    @staticmethod
    def max_upload_per_session(mode):
        if mode == NetworkBoostMode.Ultra:
            return 4 * 1024 * 1024
        elif mode == NetworkBoostMode.Fast:
            return 2 * 1024 * 1024
        return 1024 * 1024

    @staticmethod
    def upload_request_interval(mode):
        if mode == NetworkBoostMode.Ultra:
            return 60
        elif mode == NetworkBoostMode.Fast:
            return 120
        return 250

class TrafficTracker:
    def __init__(self):
        self.download_accumulated = 0
        self.last_download_time = 0
        self.current_download_speed = 0.0

        self.upload_accumulated = 0
        self.last_upload_time = 0
        self.current_upload_speed = 0.0

    def track_bytes(self, is_upload, byte_count, current_time):
        if byte_count <= 0:
            return
        if is_upload:
            self.upload_accumulated += byte_count
            if self.last_upload_time == 0:
                self.last_upload_time = current_time
            elif current_time - self.last_upload_time >= 1.0:
                elapsed = max(0.1, current_time - self.last_upload_time)
                self.current_upload_speed = (self.upload_accumulated / (1024.0 * 1024.0)) / elapsed
                self.upload_accumulated = 0
                self.last_upload_time = current_time
        else:
            self.download_accumulated += byte_count
            if self.last_download_time == 0:
                self.last_download_time = current_time
            elif current_time - self.last_download_time >= 1.0:
                elapsed = max(0.1, current_time - self.last_download_time)
                self.current_download_speed = (self.download_accumulated / (1024.0 * 1024.0)) / elapsed
                self.download_accumulated = 0
                self.last_download_time = current_time

    def get_download_speed(self, current_time):
        if self.last_download_time and current_time - self.last_download_time > 2.5:
            self.current_download_speed = 0.0
        return self.current_download_speed

    def get_upload_speed(self, current_time):
        if self.last_upload_time and current_time - self.last_upload_time > 2.5:
            self.current_upload_speed = 0.0
        return self.current_upload_speed

    @staticmethod
    def format_speed(mbps):
        if mbps < 0.01:
            return "0.00 MB/s"
        return f"{mbps:.2f} MB/s"


def export_backup_with_boost(config_dict):
    root = {
        "app": "WickedGram",
        "schema_version": 1,
        "export_timestamp": int(time.time()),
        "config": config_dict
    }
    return json.dumps(root, indent=2)


def import_backup_with_boost(json_str):
    root = json.loads(json_str)
    if root.get("app") not in ("WickedGram", "100gram"):
        return None
    config = root.get("config", root)
    res = {}
    if "downloadBoost" in config:
        val = int(config["downloadBoost"])
        res["downloadBoost"] = NetworkBoostMode.Fast if val == 3 else max(0, min(2, val))
    if "uploadBoost" in config:
        val = int(config["uploadBoost"])
        res["uploadBoost"] = NetworkBoostMode.Fast if val == 3 else max(0, min(2, val))
    if "uploadBoostFileSizeAware" in config:
        res["uploadBoostFileSizeAware"] = bool(config["uploadBoostFileSizeAware"])
    return res


class TestNetworkBoost(unittest.TestCase):
    def test_download_sessions_and_waited(self):
        # Off
        self.assertEqual(NetworkBoostLogic.initial_download_sessions_count(NetworkBoostMode.Off), 1)
        self.assertEqual(NetworkBoostLogic.initial_download_waited_amount(NetworkBoostMode.Off), 4 * kDownloadPartSize)
        self.assertEqual(NetworkBoostLogic.max_download_waited_amount(NetworkBoostMode.Off), 16 * kDownloadPartSize)

        # Fast
        self.assertEqual(NetworkBoostLogic.initial_download_sessions_count(NetworkBoostMode.Fast), 4)
        self.assertEqual(NetworkBoostLogic.initial_download_waited_amount(NetworkBoostMode.Fast), 16 * kDownloadPartSize)
        self.assertEqual(NetworkBoostLogic.max_download_waited_amount(NetworkBoostMode.Fast), 32 * kDownloadPartSize)

        # Ultra
        self.assertEqual(NetworkBoostLogic.initial_download_sessions_count(NetworkBoostMode.Ultra), 8)
        self.assertEqual(NetworkBoostLogic.initial_download_waited_amount(NetworkBoostMode.Ultra), 32 * kDownloadPartSize)
        self.assertEqual(NetworkBoostLogic.max_download_waited_amount(NetworkBoostMode.Ultra), 64 * kDownloadPartSize)

        self.assertEqual(NetworkBoostLogic.max_download_sessions_count(), 8)

    def test_upload_sessions_adaptive(self):
        # Small file (<10MB)
        small_file = 5 * 1024 * 1024
        self.assertEqual(NetworkBoostLogic.target_upload_sessions_count(NetworkBoostMode.Ultra, small_file, file_size_aware=True), 4)
        self.assertEqual(NetworkBoostLogic.target_upload_sessions_count(NetworkBoostMode.Fast, small_file, file_size_aware=True), 4)
        self.assertEqual(NetworkBoostLogic.target_upload_sessions_count(NetworkBoostMode.Off, small_file, file_size_aware=True), 4)

        # Medium file (50MB)
        medium_file = 50 * 1024 * 1024
        self.assertEqual(NetworkBoostLogic.target_upload_sessions_count(NetworkBoostMode.Fast, medium_file, file_size_aware=True), 6)
        self.assertEqual(NetworkBoostLogic.target_upload_sessions_count(NetworkBoostMode.Ultra, medium_file, file_size_aware=True), 8)

        # Large file (500MB)
        large_file = 500 * 1024 * 1024
        self.assertEqual(NetworkBoostLogic.target_upload_sessions_count(NetworkBoostMode.Ultra, large_file, file_size_aware=True), 8)
        self.assertEqual(NetworkBoostLogic.target_upload_sessions_count(NetworkBoostMode.Fast, large_file, file_size_aware=True), 6)

        # Disabled adaptive (flat mode)
        self.assertEqual(NetworkBoostLogic.target_upload_sessions_count(NetworkBoostMode.Ultra, small_file, file_size_aware=False), 8)
        self.assertEqual(NetworkBoostLogic.target_upload_sessions_count(NetworkBoostMode.Fast, small_file, file_size_aware=False), 6)

    def test_upload_buffer_and_interval(self):
        self.assertEqual(NetworkBoostLogic.max_upload_per_session(NetworkBoostMode.Off), 1024 * 1024)
        self.assertEqual(NetworkBoostLogic.max_upload_per_session(NetworkBoostMode.Fast), 2 * 1024 * 1024)
        self.assertEqual(NetworkBoostLogic.max_upload_per_session(NetworkBoostMode.Ultra), 4 * 1024 * 1024)

        self.assertEqual(NetworkBoostLogic.upload_request_interval(NetworkBoostMode.Off), 250)
        self.assertEqual(NetworkBoostLogic.upload_request_interval(NetworkBoostMode.Fast), 120)
        self.assertEqual(NetworkBoostLogic.upload_request_interval(NetworkBoostMode.Ultra), 60)

    def test_traffic_tracker_speed_and_timeout(self):
        tracker = TrafficTracker()
        t = 1000.0
        # Feed 10 MB at start
        tracker.track_bytes(is_upload=False, byte_count=10 * 1024 * 1024, current_time=t)
        # 1.0 second later, next part arrives
        t += 1.0
        tracker.track_bytes(is_upload=False, byte_count=1024, current_time=t)
        # Should be ~10 MB/s
        speed = tracker.get_download_speed(t)
        self.assertAlmostEqual(speed, 10.0, places=1)
        self.assertEqual(TrafficTracker.format_speed(speed), "10.00 MB/s")

        # Inactivity for 3 seconds -> decays to 0.0
        t += 3.0
        self.assertEqual(tracker.get_download_speed(t), 0.0)
        self.assertEqual(TrafficTracker.format_speed(tracker.get_download_speed(t)), "0.00 MB/s")

    def test_backup_roundtrip_and_100gram_compatibility(self):
        # Native WickedGram export
        cfg = {
            "downloadBoost": NetworkBoostMode.Ultra,
            "uploadBoost": NetworkBoostMode.Fast,
            "uploadBoostFileSizeAware": True
        }
        raw_json = export_backup_with_boost(cfg)
        imported = import_backup_with_boost(raw_json)
        self.assertEqual(imported["downloadBoost"], NetworkBoostMode.Ultra)
        self.assertEqual(imported["uploadBoost"], NetworkBoostMode.Fast)
        self.assertTrue(imported["uploadBoostFileSizeAware"])

        # 100gram backup with auto mode (3) -> maps safely to Fast (1) on PC
        hundredgram_json = json.dumps({
            "app": "100gram",
            "downloadBoost": 3,
            "uploadBoost": 3,
            "uploadBoostFileSizeAware": False
        })
        imported_hg = import_backup_with_boost(hundredgram_json)
        self.assertEqual(imported_hg["downloadBoost"], NetworkBoostMode.Fast)
        self.assertEqual(imported_hg["uploadBoost"], NetworkBoostMode.Fast)
        self.assertFalse(imported_hg["uploadBoostFileSizeAware"])


if __name__ == "__main__":
    unittest.main()
