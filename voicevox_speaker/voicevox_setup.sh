#!/usr/bin/env bash
set -e 

cd external/

VOICEVOX_VERSION=0.16.0
ARCH="$(uname -m)"
OS="$(uname -s)"

if [[ "$OS" == "Linux" ]]; then
    if [[ "$ARCH" == "x86_64" ]]; then
        DOWNLOADER="download-linux-x64"
    elif [[ "$ARCH" == "arm64" ]]; then
        DOWNLOADER="download-linux-arm64"
    else
        echo "Unsupported architecture: $VOICEVOX_PLATFORM"
        exit 1
    fi
else
    echo "Unsupported OS: $OS"
    exit 1
fi

DOWNLOADER_URL="https://github.com/VOICEVOX/voicevox_core/releases/download/${VOICEVOX_VERSION}/${DOWNLOADER}"

# Download downloader if not already present
if [ ! -f "$DOWNLOADER" ]; then
    echo "Downloading VOICEVOX downloader for $OS $ARCH..."
    curl -L -o "$DOWNLOADER" "$DOWNLOADER_URL"
    chmod +x "$DOWNLOADER"
fi

echo "Running VOICEVOX downloader (you must accept the license)"
./"$DOWNLOADER"
echo "Download success!"