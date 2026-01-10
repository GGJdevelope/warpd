#!/bin/sh

export KEYCHAIN_NAME=warpd-temp-keychain
export KEYCHAIN_PASSWORD=

# Allow passing binary path as argument, default to ../bin/warpd
BINARY_PATH="${1:-../bin/warpd-bin}"

cd "$(dirname "$0")"

security create-keychain -p "${KEYCHAIN_PASSWORD}" "${KEYCHAIN_NAME}" 2>/dev/null || true

# Add keychain to search path
security list-keychains -d user -s "${KEYCHAIN_NAME}" 2>/dev/null || true

security import warpd.p12 -k "${KEYCHAIN_NAME}" -P "" -T /usr/bin/codesign 2>/dev/null || true
security import warpd.cer -k "${KEYCHAIN_NAME}" -T /usr/bin/codesign 2>/dev/null || true
security unlock-keychain -p ${KEYCHAIN_PASSWORD} ${KEYCHAIN_NAME} 2>/dev/null || true

# Suppress codesign modal password prompt
security set-key-partition-list -S apple-tool:,apple: -s -k "$KEYCHAIN_PASSWORD" -D "${identity}" -t private ${KEYCHAIN_NAME} > /dev/null 2>&1 || true

codesign --force --deep --keychain "${KEYCHAIN_NAME}" -s warpd "$BINARY_PATH" 2>/dev/null || codesign --force --deep -s - "$BINARY_PATH" || true

security delete-keychain ${KEYCHAIN_NAME} 2>/dev/null || true
