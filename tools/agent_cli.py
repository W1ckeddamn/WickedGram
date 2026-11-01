#!/usr/bin/env python3
"""
WickedGram AI Agent CLI
Tool for external AI agents and automation scripts to control WickedGram Desktop
via the local AI Agent Bridge (HTTP / SSE API).
"""

import argparse
import json
import os
import pathlib
import sys
import urllib.error
import urllib.parse
import urllib.request


def find_config():
    """Locate wickedgram_agent.json in common working directories."""
    candidates = [
        pathlib.Path("tdata/wickedgram_agent.json"),
        pathlib.Path("../tdata/wickedgram_agent.json"),
        pathlib.Path("out/Debug/tdata/wickedgram_agent.json"),
        pathlib.Path("out/Release/tdata/wickedgram_agent.json"),
        pathlib.Path.home() / "AppData/Roaming/Telegram Desktop/tdata/wickedgram_agent.json",
    ]
    for c in candidates:
        if c.is_file():
            try:
                with open(c, "r", encoding="utf-8") as f:
                    return json.load(f)
            except Exception:
                pass
    return None


def get_connection_info(args):
    """Determine port and token from arguments, environment, or config file."""
    config = find_config() or {}

    port = args.port or os.environ.get("WICKEDGRAM_AGENT_PORT") or config.get("port") or 40404
    token = args.token or os.environ.get("WICKEDGRAM_AGENT_TOKEN") or config.get("token") or ""

    return int(port), str(token).strip()


def api_request(port, token, method, path, params=None, body=None):
    """Execute an HTTP request against the WickedGram AI Agent Bridge."""
    query_str = ""
    if params:
        query_str = "?" + urllib.parse.urlencode({k: v for k, v in params.items() if v is not None})

    url = f"http://127.0.0.1:{port}{path}{query_str}"
    headers = {
        "Accept": "application/json",
        "Authorization": f"Bearer {token}",
    }

    data_bytes = None
    if body is not None:
        headers["Content-Type"] = "application/json; charset=utf-8"
        data_bytes = json.dumps(body).encode("utf-8")

    req = urllib.request.Request(url, data=data_bytes, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            content = resp.read().decode("utf-8")
            if not content:
                return {"status": "ok", "code": resp.status}
            return json.loads(content)
    except urllib.error.HTTPError as e:
        err_body = e.read().decode("utf-8", errors="replace")
        try:
            parsed = json.loads(err_body)
            return {"error": parsed.get("error", err_body), "code": e.code, "status": "error"}
        except Exception:
            return {"error": err_body or str(e), "code": e.code, "status": "error"}
    except urllib.error.URLError as e:
        return {
            "error": f"Failed to connect to WickedGram Agent Bridge at 127.0.0.1:{port}. Make sure WickedGram is running and the Bridge is enabled in Settings.",
            "details": str(e),
            "status": "error",
        }


def cmd_status(args):
    port, token = get_connection_info(args)
    res = api_request(port, token, "GET", "/api/status")
    if args.json:
        print(json.dumps(res, indent=2, ensure_ascii=False))
    else:
        if res.get("status") == "error":
            print(f"[!] Error: {res.get('error')}", file=sys.stderr)
            sys.exit(1)
        print("=== WickedGram Agent Bridge Status ===")
        print(f"Client:       {res.get('client')}")
        print(f"Bridge Port:  {res.get('bridge_port')} (Listening: {res.get('bridge_listening')})")
        print(f"Logged In:    {res.get('logged_in')}")
        user = res.get("user")
        if user:
            print(f"User ID:      {user.get('id')}")
            print(f"User Name:    {user.get('name')}")
            print(f"Username:     @{user.get('username')}" if user.get('username') else "Username:     None")
            print(f"Phone:        {user.get('phone')}")


def cmd_chats_list(args):
    port, token = get_connection_info(args)
    params = {"limit": args.limit}
    if args.query:
        params["query"] = args.query

    res = api_request(port, token, "GET", "/api/chats", params=params)
    if args.json:
        print(json.dumps(res, indent=2, ensure_ascii=False))
    else:
        if res.get("status") == "error":
            print(f"[!] Error: {res.get('error')}", file=sys.stderr)
            sys.exit(1)
        chats = res.get("chats", [])
        print(f"Found {len(chats)} chats:")
        for c in chats:
            unread = f" [{c.get('unread_count')} unread]" if c.get("unread_count") else ""
            uname = f" (@{c.get('username')})" if c.get("username") else ""
            print(f"- [{c.get('id')}] {c.get('title')}{uname} ({c.get('type')}){unread}")
            last = c.get("last_message")
            if last:
                prefix = "You: " if last.get("out") else (f"{last.get('from_name')}: " if last.get("from_name") else "")
                print(f"    Last: {prefix}{last.get('text', '')[:60]}")


def cmd_chats_get(args):
    port, token = get_connection_info(args)
    res = api_request(port, token, "GET", f"/api/chats/{args.chat_id}")
    if args.json:
        print(json.dumps(res, indent=2, ensure_ascii=False))
    else:
        if res.get("status") == "error":
            print(f"[!] Error: {res.get('error')}", file=sys.stderr)
            sys.exit(1)
        chat = res.get("chat", {})
        print(f"Chat ID:   {chat.get('id')}")
        print(f"Title:     {chat.get('title')}")
        print(f"Type:      {chat.get('type')}")
        print(f"Username:  {chat.get('username') or 'None'}")
        print(f"Unread:    {chat.get('unread_count')}")


def cmd_messages_get(args):
    port, token = get_connection_info(args)
    params = {"limit": args.limit}
    res = api_request(port, token, "GET", f"/api/chats/{args.chat_id}/messages", params=params)
    if args.json:
        print(json.dumps(res, indent=2, ensure_ascii=False))
    else:
        if res.get("status") == "error":
            print(f"[!] Error: {res.get('error')}", file=sys.stderr)
            sys.exit(1)
        messages = res.get("messages", [])
        print(f"=== Messages in chat {args.chat_id} ({len(messages)}) ===")
        for m in messages:
            sender = "You" if m.get("out") else (m.get("from_name") or m.get("from_id") or "Peer")
            reply = f" [Reply to {m.get('reply_to_id')}]" if m.get("reply_to_id") else ""
            print(f"[{m.get('id')}] {sender}{reply}: {m.get('text')}")


def cmd_messages_send(args):
    port, token = get_connection_info(args)
    body = {"text": args.text}
    if args.reply_to:
        body["reply_to"] = int(args.reply_to)

    res = api_request(port, token, "POST", f"/api/chats/{args.chat_id}/messages", body=body)
    if args.json:
        print(json.dumps(res, indent=2, ensure_ascii=False))
    else:
        if res.get("status") == "error":
            print(f"[!] Failed to send: {res.get('error')}", file=sys.stderr)
            sys.exit(1)
        print(f"[✓] Message sent to chat {args.chat_id}")


def cmd_messages_read(args):
    port, token = get_connection_info(args)
    res = api_request(port, token, "POST", f"/api/chats/{args.chat_id}/read")
    if args.json:
        print(json.dumps(res, indent=2, ensure_ascii=False))
    else:
        if res.get("status") == "error":
            print(f"[!] Error: {res.get('error')}", file=sys.stderr)
            sys.exit(1)
        print(f"[✓] Marked chat {args.chat_id} as read")


def cmd_typing(args):
    port, token = get_connection_info(args)
    res = api_request(port, token, "POST", f"/api/chats/{args.chat_id}/typing")
    if args.json:
        print(json.dumps(res, indent=2, ensure_ascii=False))
    else:
        if res.get("status") == "error":
            print(f"[!] Error: {res.get('error')}", file=sys.stderr)
            sys.exit(1)
        print(f"[✓] Sent typing indicator to chat {args.chat_id}")


def cmd_ui_open(args):
    port, token = get_connection_info(args)
    res = api_request(port, token, "POST", "/api/ui/open_chat", body={"chat_id": args.chat_id})
    if args.json:
        print(json.dumps(res, indent=2, ensure_ascii=False))
    else:
        if res.get("status") == "error":
            print(f"[!] Error: {res.get('error')}", file=sys.stderr)
            sys.exit(1)
        print(f"[✓] Opened chat {args.chat_id} in WickedGram window")


def cmd_events_listen(args):
    port, token = get_connection_info(args)
    url = f"http://127.0.0.1:{port}/api/events?token={urllib.parse.quote(token)}"
    headers = {
        "Accept": "text/event-stream",
        "Authorization": f"Bearer {token}",
    }

    req = urllib.request.Request(url, headers=headers)
    print(f"[*] Listening for real-time events on 127.0.0.1:{port} (Press Ctrl+C to stop)...", file=sys.stderr)

    try:
        with urllib.request.urlopen(req, timeout=600) as resp:
            event_name = "message"
            for raw_line in resp:
                line = raw_line.decode("utf-8", errors="replace").strip()
                if not line or line.startswith(":"):
                    continue  # Keepalive ping
                if line.startswith("event:"):
                    event_name = line[6:].strip()
                elif line.startswith("data:"):
                    data_str = line[5:].strip()
                    try:
                        data_obj = json.loads(data_str)
                    except Exception:
                        data_obj = {"raw": data_str}

                    if args.filter_chat and str(data_obj.get("chat_id")) != str(args.filter_chat):
                        continue

                    if args.json:
                        print(json.dumps({"event": event_name, "data": data_obj}, ensure_ascii=False))
                        sys.stdout.flush()
                    else:
                        sender = "You" if data_obj.get("out") else data_obj.get("from_name", "Unknown")
                        print(f"[{data_obj.get('chat_title')}] {sender}: {data_obj.get('text')}")
                        sys.stdout.flush()
    except KeyboardInterrupt:
        print("\n[*] Stopped listening.", file=sys.stderr)
    except Exception as e:
        print(f"[!] Stream error: {e}", file=sys.stderr)
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description="WickedGram AI Agent Controller")
    parser.add_argument("--port", type=int, help="WickedGram Agent Bridge port (default: 40404)")
    parser.add_argument("--token", type=str, help="Bearer authorization token")
    parser.add_argument("--json", action="store_true", help="Output raw JSON for agent/machine processing")

    subparsers = parser.add_subparsers(dest="command", required=True)

    # status
    p_status = subparsers.add_parser("status", help="Get client and bridge status")
    p_status.set_defaults(func=cmd_status)

    # chats
    p_chats = subparsers.add_parser("chats", help="Chat management")
    chats_sub = p_chats.add_subparsers(dest="subcommand", required=True)

    p_chats_list = chats_sub.add_parser("list", help="List recent dialogs")
    p_chats_list.add_argument("--limit", type=int, default=50, help="Max chats to return")
    p_chats_list.add_argument("--query", type=str, help="Search filter")
    p_chats_list.set_defaults(func=cmd_chats_list)

    p_chats_get = chats_sub.add_parser("get", help="Get single chat info")
    p_chats_get.add_argument("chat_id", type=str, help="Target chat/peer ID")
    p_chats_get.set_defaults(func=cmd_chats_get)

    # messages
    p_msg = subparsers.add_parser("messages", help="Message operations")
    msg_sub = p_msg.add_subparsers(dest="subcommand", required=True)

    p_msg_get = msg_sub.add_parser("get", help="Get recent messages in chat")
    p_msg_get.add_argument("chat_id", type=str, help="Chat ID")
    p_msg_get.add_argument("--limit", type=int, default=30, help="Max messages to fetch")
    p_msg_get.set_defaults(func=cmd_messages_get)

    p_msg_send = msg_sub.add_parser("send", help="Send a message")
    p_msg_send.add_argument("chat_id", type=str, help="Chat ID")
    p_msg_send.add_argument("text", type=str, help="Message text")
    p_msg_send.add_argument("--reply-to", type=int, help="Reply to message ID")
    p_msg_send.set_defaults(func=cmd_messages_send)

    p_msg_read = msg_sub.add_parser("read", help="Mark chat as read")
    p_msg_read.add_argument("chat_id", type=str, help="Chat ID")
    p_msg_read.set_defaults(func=cmd_messages_read)

    # typing
    p_typing = subparsers.add_parser("typing", help="Send typing status")
    p_typing.add_argument("chat_id", type=str, help="Chat ID")
    p_typing.set_defaults(func=cmd_typing)

    # ui open
    p_ui = subparsers.add_parser("ui", help="UI navigation")
    ui_sub = p_ui.add_subparsers(dest="subcommand", required=True)

    p_ui_open = ui_sub.add_parser("open", help="Open chat in Telegram window")
    p_ui_open.add_argument("chat_id", type=str, help="Chat ID")
    p_ui_open.set_defaults(func=cmd_ui_open)

    # events
    p_events = subparsers.add_parser("events", help="Real-time events stream (SSE)")
    events_sub = p_events.add_subparsers(dest="subcommand", required=True)

    p_events_listen = events_sub.add_parser("listen", help="Listen to incoming messages")
    p_events_listen.add_argument("--filter-chat", type=str, help="Filter by specific chat ID")
    p_events_listen.set_defaults(func=cmd_events_listen)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
