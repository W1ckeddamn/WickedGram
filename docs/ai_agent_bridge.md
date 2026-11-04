# WickedGram AI Agent Bridge: Архитектура и API

В **WickedGram Desktop** встроен локальный мост управления для внешних ИИ-агентов (**AI Agent Bridge**). Он предоставляет локальный HTTP REST API и Server-Sent Events (SSE) поток событий, позволяя автономным агентам (Claude Code, Antigravity, AutoGPT, LangChain, n8n, Python/Node.js скриптам) безопасно взаимодействовать с запущенным клиентом Telegram.

---

## 1. Архитектура и безопасность

```mermaid
flowchart LR
    A[Внешний ИИ-агент / Python CLI] <-->|HTTP REST & SSE (127.0.0.1)| B[WickedGram Agent Bridge Server]
    B <-->|Qt Signal/Slot & RPL Stream| C[Telegram Desktop Core / Data::Session]
    C <-->|MTProto| D[Telegram Cloud Servers]
```

### Принципы безопасности:
1. **Loopback Only:** Сервер слушает исключительно `127.0.0.1` (localhost). Любые внешние сетевые подключения отклоняются на уровне сокета.
2. **Bearer Token:** Каждый запрос обязан содержать заголовок `Authorization: Bearer <token>` (или query-параметр `?token=<token>`).
3. **Хранение токена:** Конфигурация хранится в `tdata/wickedgram_agent.json` внутри рабочей директории клиента. При первом старте токен генерируется автоматически.
4. **Управление в GUI:** В разделе *Настройки -> Продвинутые настройки -> WickedGram AI Agent Bridge* доступно управление мостом, просмотр статуса, смена порта и перегенерация токена.

---

## 2. Конфигурация (`tdata/wickedgram_agent.json`)

```json
{
  "enabled": true,
  "port": 40404,
  "token": "wg_agent_a1b2c3d4e5f6..."
}
```

---

## 3. Спецификация REST API

Все ответы возвращаются в формате `application/json; charset=utf-8`.

### `GET /api/status`
Возвращает статус клиента, активную сессию и информацию о текущем авторизованном пользователе.

**Пример ответа:**
```json
{
  "status": "ok",
  "client": "WickedGram Desktop",
  "bridge_listening": true,
  "bridge_port": 40404,
  "logged_in": true,
  "user": {
    "id": "123456789",
    "name": "John Doe",
    "username": "johndoe",
    "phone": "+1234567890"
  }
}
```

---

### `GET /api/chats`
Получить список активных диалогов из списка чатов.

**Параметры запроса (Query):**
* `limit` (int, default: 50, max: 200) — количество чатов.
* `query` (string, optional) — поиск по названию или username.

**Пример ответа:**
```json
{
  "status": "ok",
  "count": 1,
  "chats": [
    {
      "id": "987654321",
      "title": "Project Discussions",
      "type": "supergroup",
      "username": "project_group",
      "unread_count": 3,
      "last_message": {
        "id": 4051,
        "date": 1730424000,
        "out": false,
        "text": "Latest update deployed",
        "from_name": "Alice"
      }
    }
  ]
}
```

---

### `GET /api/chats/{chat_id}`
Получить детальные метаданные конкретного чата.

---

### `GET /api/chats/{chat_id}/messages`
Получить последние сообщения из выбранного диалога.

**Параметры:**
* `limit` (int, default: 30, max: 100) — количество сообщений.

**Пример ответа:**
```json
{
  "status": "ok",
  "chat_id": "987654321",
  "count": 2,
  "messages": [
    {
      "id": 4050,
      "date": 1730423900,
      "out": true,
      "text": "What is the status?",
      "unread": false,
      "from_id": "123456789",
      "from_name": "You",
      "has_media": false
    },
    {
      "id": 4051,
      "date": 1730424000,
      "out": false,
      "text": "Latest update deployed",
      "unread": true,
      "from_id": "555555555",
      "from_name": "Alice",
      "reply_to_id": 4050,
      "has_media": false
    }
  ]
}
```

---

### `POST /api/chats/{chat_id}/messages`
Отправить текстовое сообщение в чат.

**Тело запроса (JSON):**
```json
{
  "text": "Ответ от ИИ-агента",
  "reply_to": 4051
}
```

**Ответ:**
```json
{
  "status": "ok",
  "chat_id": "987654321",
  "sent": true
}
```

---

### `POST /api/chats/{chat_id}/read`
Пометить входящие сообщения в чате прочитанными.

**Ответ:**
```json
{
  "status": "ok",
  "chat_id": "987654321",
  "marked_read": true
}
```

---

### `POST /api/chats/{chat_id}/typing`
Отправить статус набора текста в чат.

---

### `POST /api/ui/open_chat`
Переключить фокус графического окна WickedGram на указанный чат.

**Тело запроса (JSON):**
```json
{
  "chat_id": "987654321"
}
```

---

### `GET /api/events` (Server-Sent Events)
Потоковый SSE-канал в реальном времени. Агент подключается к эндпоинту и непрерывно получает события появления новых сообщений без polling-запросов.

**Пример события:**
```http
event: message
data: {"event":"new_message","id":4052,"chat_id":"987654321","chat_title":"Project Discussions","from_id":"555555555","from_name":"Alice","text":"Can you summarize this?","date":1730424100,"out":false}
```

---

## 4. Консольная утилита `agent.exe`

В комплекте с WickedGram Desktop поставляется нативный C++ бинарник `agent.exe` (располагается в папке приложения рядом с `Telegram.exe`). Он работает автономно без Python и внешних зависимостей, используя системный стек Windows `WinHttp`:

```bash
# Проверить статус моста и профиля
agent.exe status

# Получить список недавних чатов
agent.exe chats list --limit 10

# Прочитать последние сообщения
agent.exe messages get 987654321 --limit 15

# Отправить сообщение
agent.exe messages send 987654321 "Hello from AI Agent!"

# Оценить дату регистрации аккаунта по ID
agent.exe regdate 123456789

# Экспорт / импорт бэкапа настроек
agent.exe backup export
agent.exe backup import

# Слушать входящие сообщения в реальном времени (SSE)
agent.exe events listen
```

---

## 5. Пример интеграции на Python

```python
import json
import urllib.request

TOKEN = "wg_agent_..."
PORT = 40404

def send_telegram_message(chat_id: str, text: str):
    url = f"http://127.0.0.1:{PORT}/api/chats/{chat_id}/messages"
    req = urllib.request.Request(
        url,
        data=json.dumps({"text": text}).encode("utf-8"),
        headers={
            "Authorization": f"Bearer {TOKEN}",
            "Content-Type": "application/json"
        },
        method="POST"
    )
    with urllib.request.urlopen(req) as resp:
        return json.loads(resp.read().decode("utf-8"))

print(send_telegram_message("987654321", "Processed by AI model!"))
```
