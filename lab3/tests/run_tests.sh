#!/usr/bin/env bash
set -euo pipefail

BINARY="${1:-./path_processor}"
TESTS_DIR="$(dirname "$0")"
PASS=0
FAIL=0

sha256_hash() {
    if command -v sha256sum > /dev/null 2>&1; then
        sha256sum "$1" | awk '{print $1}'
    else
        shasum -a 256 "$1" | awk '{print $1}'
    fi
}

for test_dir in "$TESTS_DIR"/*/; do
    name=$(basename "$test_dir")
    expected_hash_file="$test_dir/expected.sha256"

    if [ ! -f "$expected_hash_file" ]; then
        echo "SKIP $name (no expected.sha256)"
        continue
    fi

    expected_hash=$(cat "$expected_hash_file")
    out=$(mktemp)
    binary_log=$(mktemp)

    if ! "$BINARY" "$test_dir" "$test_dir/input.txt" "$out" > "$binary_log" 2>&1; then
        echo "FAIL $name (binary exited with error)"
        echo "  --- binary output ---"
        sed 's/^/  /' "$binary_log"
        echo "  --------------------"
        FAIL=$((FAIL + 1))
        rm -f "$out" "$binary_log"
        continue
    fi

    actual_hash=$(sha256_hash "$out")
    rm -f "$out" "$binary_log"

    if [ "$actual_hash" = "$expected_hash" ]; then
        echo "PASS $name"
        PASS=$((PASS + 1))
    else
        echo "FAIL $name (hash mismatch: expected $expected_hash, got $actual_hash)"
        FAIL=$((FAIL + 1))
    fi
done

echo ""
echo "Results: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
