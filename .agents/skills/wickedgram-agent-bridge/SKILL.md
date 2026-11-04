---
name: wickedgram-agent-bridge
description: Control running WickedGram Desktop client via local AI Agent Bridge (HTTP REST & SSE). Read dialogs, fetch messages, send replies, mark read, send typing indicator, monitor live events, calculate registration dates, and manage backups.
allowed-tools: run_command(Tools/agent.cmd:*), run_command(python Tools/agent_cli.py:*)
---

# WickedGram AI Agent Bridge

This skill enables external AI agents (Antigravity, Claude, AutoGPT, Python scripts) to interact directly with an active **WickedGram Desktop** instance through its built-in local HTTP/SSE Bridge (`127.0.0.1:40404`).

## Capabilities

- **Status & Session**: Check whether WickedGram is running, bridge status, port, and current logged-in user profile.
- **Chats & Dialogs**: Search and list dialogs, unread badges, peer IDs, and chat metadata.
- **Messages**: Read recent chat history, send text messages, reply to specific messages, and mark chats as read.
- **Real-Time Stream (SSE)**: Subscribe to Server-Sent Events for instant notification of new incoming and outgoing messages.
- **Account Intelligence**: Estimate Telegram user account registration date and timestamp from raw User ID.
- **Settings Backup**: Programmatically export or import encrypted/JSON WickedGram settings and feature toggles.
- **Desktop UI Control**: Focus and bring any chat into view in the active Telegram window.

---

## When to Use

- When an agent needs to read Telegram messages or channel history in real time.
- When an agent needs to reply to messages or send notifications via the user's active Telegram account.
- When automating Telegram workflows (auto-responder, assistant, archiving, monitoring).
- When validating Telegram account age/registration date.
- When exporting or importing WickedGram configuration and backups.

## When NOT to Use

- Do **not** use when WickedGram Desktop is stopped or not yet started (the bridge runs inside the client process).
- Do **not** use for modifying source code or compiling the project.
- Do **not** use for raw MTProto network attacks; all operations flow through the official desktop client session with full UI safety.

---

## Connection & Authentication

The bridge server binds to `127.0.0.1:40404` by default.
Authorization uses Bearer tokens.

1. **Automatic Discovery**: `Tools/agent.cmd` automatically detects the token and port from `tdata/wickedgram_agent.json`.
2. **Manual Override**: Provide `--port <PORT>` and `--token <TOKEN>` or set environment variables:
   - `WICKEDGRAM_AGENT_PORT`
   - `WICKEDGRAM_AGENT_TOKEN`
3. **UI Configuration**: The user can enable/disable the bridge, change ports, view the token, or generate a new token anytime in:
   `Settings` -> `Wicked Settings` -> `AI Agent Bridge`.

---

## Quick Reference Commands

All commands support the `--json` flag for easy parsing by AI agents.

### 1. Check Status & Active User
```bash
Tools\agent.cmd status --json
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
Tools\agent.cmd chats list --limit 20 --json

# Search chats by title or username
Tools\agent.cmd chats list --query "Support" --json
```

### 3. Read Messages from Chat
```bash
Tools\agent.cmd messages get <chat_id> --limit 30 --json
```

### 4. Send a Message or Reply
```bash
# Send regular message
Tools\agent.cmd messages send <chat_id> "Hello from AI Agent" --json

# Reply to a specific message ID
Tools\agent.cmd messages send <chat_id> "Acknowledged" --reply-to <message_id> --json
```

### 5. Mark Messages as Read
```bash
Tools\agent.cmd messages read <chat_id> --json
```

### 6. Send Typing Indicator
```bash
Tools\agent.cmd typing <chat_id> --json
```

### 7. Switch UI Focus to Chat
```bash
Tools\agent.cmd ui open <chat_id> --json
```

### 8. Estimate User Registration Date
```bash
Tools\agent.cmd regdate <user_id> --json
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
Tools\agent.cmd backup export --json

# Import latest backup automatically
Tools\agent.cmd backup import --json

# Import specific backup file
Tools\agent.cmd backup import --file "C:/path/to/backup.json" --json
```

### 10. Listen to Real-Time Events (SSE)
```bash
# Stream all incoming/outgoing messages
Tools\agent.cmd events listen --json

# Filter events for a single chat
Tools\agent.cmd events listen --filter-chat <chat_id> --json
```

---

## Direct REST API Reference

Agents running in Python or Node.js can also make HTTP requests directly to `http://127.0.0.1:40404` with header `Authorization: Bearer <token>`:

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

---

## Common Workflows

### Auto-Response Pattern
1. Start SSE listener with `Tools\agent.cmd events listen --json`.
2. On event `new_message`:
   - Check if message is incoming (`"out": false`).
   - If response is needed, send typing action: `Tools\agent.cmd typing <chat_id>`.
   - Send reply: `Tools\agent.cmd messages send <chat_id> "<reply>" --reply-to <message_id>`.
   - Mark read: `Tools\agent.cmd messages read <chat_id>`.

### Backup Synchronization
Before applying experimental settings changes:
1. `Tools\agent.cmd backup export --json`
2. Apply changes.
3. If rollback is needed: `Tools\agent.cmd backup import --json`.
