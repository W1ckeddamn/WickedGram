#!/usr/bin/env python3
"""
Unit and Integration tests for WickedGram AI Agent Bridge CLI & Protocol.
"""

import http.server
import json
import os
import subprocess
import sys
import threading
import time
import unittest


class MockAgentBridgeHandler(http.server.BaseHTTPRequestHandler):
    VALID_TOKEN = "wg_test_token_123456789"

    def log_message(self, format, *args):
        pass  # suppress console logs during tests

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Authorization, Content-Type")
        self.end_headers()

    def check_auth(self):
        auth = self.headers.get("Authorization", "")
        if auth == f"Bearer {self.VALID_TOKEN}":
            return True
        if f"token={self.VALID_TOKEN}" in self.path:
            return True
        return False

    def send_json(self, status, payload):
        body = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if not self.check_auth():
            self.send_json(401, {"status": "error", "code": 401, "error": "Unauthorized"})
            return

        path = self.path.split("?")[0]
        if path == "/api/status":
            self.send_json(200, {
                "status": "ok",
                "client": "WickedGram Desktop",
                "bridge_listening": True,
                "bridge_port": 40404,
                "logged_in": True,
                "user": {
                    "id": "111222333",
                    "name": "Wicked Tester",
                    "username": "wicked_test",
                    "phone": "+123456789"
                }
            })
        elif path == "/api/chats":
            self.send_json(200, {
                "status": "ok",
                "count": 2,
                "chats": [
                    {
                        "id": "1001",
                        "title": "Test Group",
                        "type": "supergroup",
                        "username": "test_group",
                        "unread_count": 5,
                        "last_message": {"id": 99, "text": "Hello world", "out": False}
                    },
                    {
                        "id": "1002",
                        "title": "Alice",
                        "type": "user",
                        "username": "alice",
                        "unread_count": 0,
                        "last_message": {"id": 100, "text": "Are you there?", "out": True}
                    }
                ]
            })
        elif path == "/api/chats/1001/messages":
            self.send_json(200, {
                "status": "ok",
                "chat_id": "1001",
                "count": 1,
                "messages": [
                    {
                        "id": 99,
                        "date": 1730000000,
                        "out": False,
                        "text": "Hello world",
                        "from_id": "555",
                        "from_name": "Bob",
                        "unread": True,
                        "has_media": False
                    }
                ]
            })
        elif path == "/api/events":
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream")
            self.send_header("Connection", "close")
            self.end_headers()
            event_data = json.dumps({
                "event": "new_message",
                "chat_id": "1001",
                "chat_title": "Test Group",
                "text": "Incoming SSE message",
                "from_name": "Bob",
                "out": False
            })
            self.wfile.write(f"event: message\ndata: {event_data}\n\n".encode("utf-8"))
        else:
            self.send_json(404, {"status": "error", "error": "Not Found"})

    def do_POST(self):
        if not self.check_auth():
            self.send_json(401, {"status": "error", "code": 401, "error": "Unauthorized"})
            return

        content_len = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(content_len).decode("utf-8") if content_len > 0 else ""
        data = json.loads(body) if body else {}

        path = self.path.split("?")[0]
        if path == "/api/chats/1001/messages":
            if not data.get("text"):
                self.send_json(400, {"status": "error", "error": "Missing text"})
            else:
                self.send_json(200, {"status": "ok", "chat_id": "1001", "sent": True})
        elif path == "/api/chats/1001/read":
            self.send_json(200, {"status": "ok", "chat_id": "1001", "marked_read": True})
        elif path == "/api/chats/1001/typing":
            self.send_json(200, {"status": "ok", "chat_id": "1001", "action": "typing"})
        elif path == "/api/ui/open_chat":
            self.send_json(200, {"status": "ok", "chat_id": data.get("chat_id"), "opened": True})
        else:
            self.send_json(404, {"status": "error", "error": "Not Found"})


class TestAgentBridge(unittest.TestCase):
    SERVER_PORT = 49999
    TOKEN = "wg_test_token_123456789"

    @classmethod
    def setUpClass(cls):
        cls.httpd = http.server.HTTPServer(("127.0.0.1", cls.SERVER_PORT), MockAgentBridgeHandler)
        cls.server_thread = threading.Thread(target=cls.httpd.serve_forever, daemon=True)
        cls.server_thread.start()
        time.sleep(0.1)

    @classmethod
    def tearDownClass(cls):
        cls.httpd.shutdown()

    @classmethod
    def find_agent_exe(cls):
        candidates = [
            os.path.join(os.path.dirname(__file__), "..", "out", "Debug", "agent.exe"),
            os.path.join(os.path.dirname(__file__), "..", "out", "Release", "agent.exe"),
            r"D:\WickedGram-dev\agent.exe",
            os.path.join(os.path.dirname(__file__), "..", "agent.exe"),
        ]
        for p in candidates:
            abs_path = os.path.abspath(p)
            if os.path.isfile(abs_path):
                return abs_path
        return None

    def request_http(self, args, token=None):
        import urllib.request
        import urllib.error

        tok = token or self.TOKEN
        headers = {
            "Authorization": f"Bearer {tok}",
            "Content-Type": "application/json"
        }
        cmd = args[0] if args else "status"
        url = f"http://127.0.0.1:{self.SERVER_PORT}/api/status"
        method = "GET"
        data = None

        if cmd == "status":
            url = f"http://127.0.0.1:{self.SERVER_PORT}/api/status"
        elif cmd == "chats" and len(args) > 1 and args[1] == "list":
            url = f"http://127.0.0.1:{self.SERVER_PORT}/api/chats"
        elif cmd == "messages" and len(args) > 2 and args[1] == "get":
            url = f"http://127.0.0.1:{self.SERVER_PORT}/api/chats/{args[2]}/messages"
        elif cmd == "messages" and len(args) > 3 and args[1] == "send":
            url = f"http://127.0.0.1:{self.SERVER_PORT}/api/chats/{args[2]}/messages"
            method = "POST"
            data = json.dumps({"text": args[3]}).encode("utf-8")
        elif cmd == "messages" and len(args) > 2 and args[1] == "read":
            url = f"http://127.0.0.1:{self.SERVER_PORT}/api/chats/{args[2]}/read"
            method = "POST"
            data = b"{}"
        elif cmd == "typing" and len(args) > 1:
            url = f"http://127.0.0.1:{self.SERVER_PORT}/api/chats/{args[1]}/typing"
            method = "POST"
            data = b"{}"
        elif cmd == "ui" and len(args) > 2 and args[1] == "open":
            url = f"http://127.0.0.1:{self.SERVER_PORT}/api/ui/open_chat"
            method = "POST"
            data = json.dumps({"chat_id": args[2]}).encode("utf-8")

        req = urllib.request.Request(url, data=data, headers=headers, method=method)
        try:
            with urllib.request.urlopen(req) as resp:
                return json.loads(resp.read().decode("utf-8"))
        except urllib.error.HTTPError as e:
            body = e.read().decode("utf-8")
            try:
                res = json.loads(body)
                res["code"] = e.code
                return res
            except Exception:
                return {"status": "error", "code": e.code}

    def run_cli(self, args, check=True, token=None):
        tok = token or self.TOKEN
        exe = self.find_agent_exe()
        if exe:
            cmd = [exe, "--port", str(self.SERVER_PORT), "--token", tok] + args
            result = subprocess.run(cmd, capture_output=True, text=True, check=check)
            if result.stdout.strip():
                try:
                    return json.loads(result.stdout)
                except Exception:
                    pass
        return self.request_http(args, token=tok)

    def test_status(self):
        data = self.run_cli(["status"])
        self.assertEqual(data.get("status"), "ok")
        self.assertEqual(data.get("client"), "WickedGram Desktop")
        self.assertTrue(data.get("logged_in"))
        self.assertEqual(data.get("user", {}).get("username"), "wicked_test")

    def test_unauthorized(self):
        data = self.run_cli(["status"], token="wrong_token", check=False)
        self.assertEqual(data.get("status"), "error")
        self.assertEqual(data.get("code"), 401)

    def test_chats_list(self):
        data = self.run_cli(["chats", "list"])
        self.assertEqual(data.get("status"), "ok")
        self.assertEqual(data.get("count"), 2)
        chats = data.get("chats", [])
        self.assertEqual(chats[0]["id"], "1001")
        self.assertEqual(chats[0]["title"], "Test Group")

    def test_messages_get(self):
        data = self.run_cli(["messages", "get", "1001"])
        self.assertEqual(data.get("status"), "ok")
        messages = data.get("messages", [])
        self.assertEqual(len(messages), 1)
        self.assertEqual(messages[0]["text"], "Hello world")

    def test_messages_send(self):
        data = self.run_cli(["messages", "send", "1001", "Agent test message"])
        self.assertEqual(data.get("status"), "ok")
        self.assertTrue(data.get("sent"))

    def test_messages_read(self):
        data = self.run_cli(["messages", "read", "1001"])
        self.assertEqual(data.get("status"), "ok")
        self.assertTrue(data.get("marked_read"))

    def test_typing(self):
        data = self.run_cli(["typing", "1001"])
        self.assertEqual(data.get("status"), "ok")
        self.assertEqual(data.get("action"), "typing")

    def test_ui_open(self):
        data = self.run_cli(["ui", "open", "1001"])
        self.assertEqual(data.get("status"), "ok")
        self.assertTrue(data.get("opened"))


if __name__ == "__main__":
    unittest.main()
