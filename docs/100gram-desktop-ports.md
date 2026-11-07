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
