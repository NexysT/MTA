#!/usr/bin/env bash
set -euo pipefail
EXPECTED="39d4a4f1b0b5c51ca792dc45f134c695e9083ea6ca266d61a2c3e4e085fcb56d"
FILE="${1:-./net.so}"
if [[ ! -f "$FILE" ]]; then echo "net.so not found: $FILE" >&2; exit 1; fi
ACTUAL="$(sha256sum "$FILE" | awk '{print $1}')"
echo "SHA-256: $ACTUAL"
if [[ "$ACTUAL" == "$EXPECTED" ]]; then
  echo "Compatible tested net.so fingerprint."
  exit 0
fi
echo "WARNING: different net.so. Port and test the encoder offset before using this native module." >&2
exit 2
