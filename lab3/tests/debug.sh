#!/usr/bin/env bash
# Показывает что реально выдаёт бинарник на каждом тесте.
# expected.txt в репо нет — сравнение с эталоном недоступно,
# только хэш совпадает/нет.

set -euo pipefail

BINARY="${1:-./path_processor}"
TESTS_DIR="$(dirname "$0")"

for test_dir in "$TESTS_DIR"/*/; do
    name=$(basename "$test_dir")
    expected_hash_file="$test_dir/expected.sha256"

    echo "──────────────────────────────────────"
    echo "TEST: $name"
    echo ""

    echo "  input.txt:"
    sed 's/^/    /' "$test_dir/input.txt"
    echo ""

    out=$(mktemp)

    if ! "$BINARY" "$test_dir" "$test_dir/input.txt" "$out" 2>&1 | sed 's/^/  [binary] /'; then
        echo "  ERROR: binary exited with non-zero status"
        rm "$out"
        continue
    fi

    echo ""
    echo "  output ($( wc -l < "$out" | tr -d ' ') lines):"
    sed 's/^/    /' "$out"

    if [ -f "$expected_hash_file" ]; then
        expected_hash=$(cat "$expected_hash_file")
        if command -v sha256sum > /dev/null 2>&1; then
            actual_hash=$(sha256sum "$out" | awk '{print $1}')
        else
            actual_hash=$(shasum -a 256 "$out" | awk '{print $1}')
        fi
        echo ""
        echo "  expected sha256: $expected_hash"
        echo "  actual   sha256: $actual_hash"
        if [ "$actual_hash" = "$expected_hash" ]; then
            echo "  → PASS"
        else
            echo "  → FAIL"
        fi
    fi

    rm "$out"
    echo ""
done
