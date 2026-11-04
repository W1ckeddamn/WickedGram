---
name: wickedgram-agent-bridge
description: Control running WickedGram Desktop client via native agent.exe CLI (HTTP REST & SSE). Read dialogs, fetch messages, send replies, mark read, send typing indicator, monitor live events, calculate registration dates, and manage backups.
allowed-tools: run_command(agent.exe:*), run_command(Tools/agent.exe:*)
---

# WickedGram AI Agent Bridge

This skill enables external AI agents (Antigravity, Claude, AutoGPT) to interact directly with an active **WickedGram Desktop** instance through the native **`agent.exe`** CLI and its built-in local HTTP/SSE Bridge (`127.0.0.1:40404`).

**100% Native C++ (Zero Dependencies):** No Python or external runtimes required. `agent.exe` is bundled directly alongside `WickedGram.exe` in the release archive.

## Capabilities

- **Status & Session**: Check whether WickedGram is running, bridge status, port, and current logged-in user profile.
- **Chats & Dialogs**: Search and list dialogs, unread badges, peer IDs, and chat metadata.
- **Messages**: Read recent chat history, send text messages, reply to specific messages, and mark chats as read.
- **Real-Time Stream (SSE)**: Subscribe to Server-Sent Events for instant notification of new incoming and outgoing messages.
- **Account Intelligence**: Estimate Telegram user account registration date and timestamp from raw User ID.
- **Settings Backup**: Programmatically export or import WickedGram settings and feature toggles.
- **Desktop UI Control**: Focus and bring any chat into view in the active Telegram window.

---

## When to Use

- When an agent needs to read Telegram messages or channel history in real time.
- When an agent needs to reply to messages or send notifications via the user's active Telegram account.
- When automating Telegram workflows (auto-responder, assistant, archiving, monitoring).
- When validating Telegram account age/registration date.
- When exporting or importing WickedGram configuration and backups.

## When NOT to Use

- Do **not** use when WickedGram Desktop is stopped (the bridge runs inside the client process).
- Do **not** use for modifying source code or compiling the project.
- Do **not** use for raw MTProto network attacks; all operations flow through the official desktop client session with full UI safety.

---

## Connection & Authentication

The bridge server binds to `127.0.0.1:40404` by default.
Authorization uses Bearer tokens.

1. **Automatic Discovery**: `agent.exe` automatically detects the token and port from `tdata/wickedgram_agent.json`.
2. **Manual Override**: Provide `--port <PORT>` and `--token <TOKEN>` or set environment variables:
   - `WICKEDGRAM_AGENT_PORT`
   - `WICKEDGRAM_AGENT_TOKEN`
3. **UI Configuration**: The user can enable/disable the bridge, change ports, view the token, or generate a new token anytime in:
   `Settings` -> `Wicked Settings` -> `AI Agent Bridge`.

---

## Quick Reference Commands

All commands output structured JSON by default for fast, reliable parsing by AI agents.

### 1. Check Status & Active User
```bash
agent.exe status
```
Example response:
```json
{
  "client": "WickedGram Desktop",
  "bridge_listening": true,
  "bridge_port": 40404,
  "logged_in": true,
  "status": "ok",
  "user": {
    "id": "123456789",
    "name": "Wicked User",
    "phone": "+1234567890",
    "username": "wicked_user"
  }
}
```

### 2. List Chats & Search
```bash
# List up to 20 recent chats
agent.exe chats list --limit 20

# Search chats by title or username
agent.exe chats list --query "Support"
```

### 3. Read Messages from Chat
```bash
agent.exe messages get <chat_id> --limit 30
```

### 4. Send a Message or Reply
```bash
# Send regular message
agent.exe messages send <chat_id> "Hello from AI Agent"

# Reply to a specific message ID
agent.exe messages send <chat_id> "Acknowledged" --reply-to <message_id>
```

### 5. Mark Messages as Read
```bash
agent.exe messages read <chat_id>
```

### 6. Send Typing Indicator
```bash
agent.exe typing <chat_id>
```

### 7. Switch UI Focus to Chat
```bash
agent.exe ui open <chat_id>
```

### 8. Estimate User Registration Date
```bash
agent.exe regdate <user_id>
```
Example response:
```json
{
  "status": "ok",
  "user_id": "98765432",
  "timestamp": 1451606400,
  "registration_date": "January 2016"
}
```

### 9. Export & Import Settings Backup
```bash
# Export settings to default backup directory
agent.exe backup export

# Import latest backup automatically
agent.exe backup import

# Import specific backup file
agent.exe backup import --file "C:/path/to/backup.json"
```

### 10. Listen to Real-Time Events (SSE)
```bash
# Stream all incoming/outgoing messages
agent.exe events listen

# Filter events for a single chat
agent.exe events listen --filter-chat <chat_id>
```

---

## Direct REST API Reference

Agents can also make HTTP requests directly to `http://127.0.0.1:40404` with header `Authorization: Bearer <token>`:

| Endpoint | Method | Params / Body | Description |
|---|---|---|---|
| `/api/status` | `GET` | — | Bridge status and logged-in user profile |
| `/api/chats` | `GET` | `?limit=50&query=...` | List chats and unread counts |
| `/api/chats/{chat_id}` | `GET` | — | Single chat details |
| `/api/chats/{chat_id}/messages` | `GET` | `?limit=30` | Get recent message history |
| `/api/chats/{chat_id}/messages` | `POST` | `{"text": "...", "reply_to": 123}` | Send message or reply |
| `/api/chats/{chat_id}/read` | `POST` | — | Mark chat inbox as read |
| `/api/chats/{chat_id}/typing` | `POST` | — | Trigger "typing..." indicator |
| `/api/ui/open_chat` | `POST` | `{"chat_id": "123"}` | Focus chat in Desktop window |
| `/api/user/regdate` | `GET` | `?user_id=123` | Estimate user registration date |
| `/api/settings/backup/export` | `POST` | — | Export settings to JSON |
| `/api/settings/backup/import` | `POST` | `{"file_path": "..."}` | Import settings from JSON |
| `/api/events` | `GET` | (SSE stream) | Server-Sent Events real-time event bus |
