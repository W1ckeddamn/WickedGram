# WickedGram Windows updates

Windows x64 CI builds use the signed v2 updater with a WickedGram root key.
They never query Telegram's HTTP or MTProto update feeds. Builds without a
configured canary counter keep automatic updates disabled.

Every successful `dev` build creates an immutable prerelease named
`dev-build-<counter>`. The counter is the workflow run number multiplied by
100, plus the run attempt. The release contains the portable ZIP and a signed
update package. Only after uploading the package does CI replace `update.json`
in the `wickedgram-dev` prerelease. Pull requests cannot publish updates or
access the signing key. Other branches and version tags do not update the dev
feed.

The client compares the upstream base version and the build counter, downloads
the package over HTTPS, and verifies its signature, target and version before
decompression. The native Updater installs it after the user chooses to restart.
The payload contains application executables; it does not include account data.
Existing Telegram update staging markers are rejected.

The first build containing this updater must be installed manually. Subsequent
dev builds update through the app. Tagged releases are still downloadable ZIPs;
there is no separate stable automatic-update channel yet.

## Signing keys

The public trust files live in `Telegram/Resources/update/`. CI reads the
Ed25519 package key from the `WICKEDGRAM_UPDATE_SIGNING_KEY` repository secret
and deletes its temporary PEM after packing. Never print or commit a private
key. The manifest root key stays offline; it is not needed for ordinary builds.

The initial root and package-key backups are in the maintainer's protected
`~/.wickedgram-update-keys/` directory. Preserve these backups securely. Root
rotation requires a deliberate migration of installed clients. Package-key
rotation requires a new root-signed manifest with a higher manifest version.

CI runs the update verification, unpacking and Windows updater tests. Packer
also verifies each signed package before it is published. A failed packing,
test or upload step must leave the public feed unchanged.
