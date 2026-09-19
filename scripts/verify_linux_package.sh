#!/usr/bin/env bash
set -euo pipefail

package_root="${1:?usage: verify_linux_package.sh <package-root> [forbidden-prefix]}"
forbidden_prefix="${2:-}"
app="$package_root/bin/dnd_tracker"

if [[ ! -x "$app" ]]; then
    echo "missing executable: $app" >&2
    exit 1
fi

# The build jobs export LD_LIBRARY_PATH so Qt's own tools can start. A release
# must not depend on that build-only environment, so inspect it with the
# variable removed.
ldd_output="$(env -u LD_LIBRARY_PATH ldd "$app")"
printf '%s\n' "$ldd_output"

if grep -q 'not found' <<<"$ldd_output"; then
    echo "Linux package has unresolved shared-library dependencies" >&2
    exit 1
fi

# Also reject accidental resolution from the Qt SDK used on the CI runner.
# Otherwise a broken archive can look healthy only because its build tree still
# exists when this check runs.
if [[ -n "$forbidden_prefix" ]] && grep -Fq "$forbidden_prefix" <<<"$ldd_output"; then
    echo "Linux package resolves libraries from the build SDK: $forbidden_prefix" >&2
    exit 1
fi

# If this Qt build uses ICU, every ICU dependency must resolve from the package.
# ldd may preserve paths such as bin/../lib, so compare canonical paths rather
# than their textual spelling. There is deliberately no hard-coded ICU major
# version here: when Qt moves to another ABI, the check follows it automatically.
package_lib="$(readlink -f -- "$package_root/lib")"
while IFS= read -r line; do
    [[ -z "$line" ]] && continue
    resolved="$(awk '{print $3}' <<<"$line")"
    resolved="$(readlink -f -- "$resolved")"
    if [[ "$resolved" != "$package_lib/"* ]]; then
        echo "ICU dependency escaped package: $line" >&2
        exit 1
    fi
done < <(grep -E '^[[:space:]]*libicu[^[:space:]]* => ' <<<"$ldd_output" || true)

echo "Linux package runtime dependencies are self-contained."
