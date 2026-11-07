# 100gram features adapted for Desktop

## First batch: calls and chat readability

All four options are off by default, persist in WickedGram's JSON preferences,
and participate in settings backup and restore using their 100gram key names.
Missing or non-boolean imported values leave the current option unchanged.

| Feature | Desktop adaptation | Reason |
| --- | --- | --- |
| `confirmCalls` | Require the existing native call confirmation panel, including calls started from the top-bar menu | Avoid an accidental voice or video call without introducing a second confirmation UI |
| `showWeekdayInDate` | Add the localized weekday to chat date badges and keep their hit-test width consistent in both message-list implementations | Helps navigate long work conversations |
| `hideShareButton` | Hide only the fast-forward arrow; keep comments, navigation to originals and context-menu forwarding | Reduce visual clutter without removing useful desktop actions |
| `disableGreetingSticker` | Omit the sticker in empty-chat introductions; preserve text, premium/payment notices and the business-intro settings preview | Reduce animation and accidental sends in empty chats |

The call option applies immediately. Restart the client after changing the three
appearance options to rebuild existing message layouts and date-badge caches.

## Deliberately excluded

Pull-to-next-channel suppression already exists in Desktop's native preferences.
CameraX/Camera2 selection, lens switching and Android screenshot flags depend on
Android APIs. Bottom navigation and touch swipe menus need a separate desktop
interaction design. They are not copied into this batch.

## Runtime checks

- With call confirmation enabled, start voice and video calls from the top bar
  and profile, cancel, then confirm. Incoming calls should be unaffected.
- Enable weekdays, restart, and inspect date badges and their clickable area in
  ordinary chats, topic/reply lists and scheduled messages. Check another language.
- Hide the share arrow, restart, and inspect channel and bot messages. Confirm
  that forwarding from the context menu, comment buttons and original-message
  navigation still work.
- Hide the greeting sticker, restart, and open an empty user chat. Verify that
  business-intro editing previews and premium/payment notices remain present.
- Export the four options, change them, then restore the backup. Older backups
  without these keys should preserve their current values.

These are runtime verification steps; compilation alone does not establish them.

## Second batch: streaming and media storage

The streamer master switch now has independent controls for chat-list names,
avatars, message/draft previews, and notification identities and text. These
operate on presentation data; they do not rename peers or rewrite messages.
Changing notification masking clears existing notifications so an old preview
does not remain on screen. The list controls do not conceal an opened chat,
profile, message-search results, or the operating system's own UI.

Vault settings now select private chats, groups, channels, bots, archived chats,
photos, video/GIF, voice/round messages, and other files. The existing scopes
remain enabled by default for compatibility; automatic saving itself stays off
by default. Files are saved when already available, including at download
completion. This is not a background download or history-synchronization tool.
Outgoing, synthetic and ephemeral media are excluded.

File-size, retention and total-size controls are exposed in settings. Cleanup
runs during successful saves at most once per minute or through the explicit
cleanup action. The chat menu can protect that account/chat's vault files from
TTL and quota cleanup. Legacy filenames without an account prefix are protected
conservatively across accounts. Protected files may exceed the storage quota.
Manual full clearing remains separate and asks for confirmation including
protected files.

New filenames include the account, peer, message and document identity to avoid
collisions. Document names are sanitized and copies use atomic replacement;
Windows still prefers hardlinks when possible. The vault is a plaintext folder,
not an encrypted archive. Unknown files, subdirectories and symlinks are excluded
from automatic cleanup. All new settings and protected-chat selections are
included in configuration backup.

`test_wicked_vault_cleanup` exercises TTL expiry, quota eviction order, protected
files, legacy filenames, account separation, unmanaged files and unlimited mode
using temporary directories. The GitHub Debug workflow runs it before publishing.
Streamer masking and filter behavior still require runtime verification in the
built client; these filesystem tests do not cover those UI paths.
