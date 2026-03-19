#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DEPS_DIR="${SCRIPT_DIR}/../DanceUIDependencies"

SWIFT_VERSION="6.1.2"
SWIFT_TAG="swift-${SWIFT_VERSION}-RELEASE"
BOOST_VERSION="1.88.0"

# Skip Xcode version check to allow building with any Xcode version
export SKIP_XCODE_VERSION_CHECK=1

echo "=== DanceUIGraph Dependencies Setup ==="
echo "Dependencies directory: ${DEPS_DIR}"

mkdir -p "${DEPS_DIR}"

# ============ BOOST SETUP ============
setup_boost() {
    local BOOST_DIR="${DEPS_DIR}/boost"

    echo ""
    echo "=== Setting up Boost ${BOOST_VERSION} (header-only) ==="

    mkdir -p "${BOOST_DIR}"

    if [ ! -d "${BOOST_DIR}/boost" ]; then
        echo "Downloading Boost ${BOOST_VERSION}..."
        cd "${BOOST_DIR}"

        # Download from official Boost release
        curl -L -o boost.tar.gz \
            "https://github.com/boostorg/boost/releases/download/boost-${BOOST_VERSION}/boost-${BOOST_VERSION}-b2-nodocs.tar.gz"

        echo "Extracting Boost..."
        tar -xzf boost.tar.gz
        mv "boost-${BOOST_VERSION}/boost" boost
        rm -rf "boost-${BOOST_VERSION}" boost.tar.gz
    else
        echo "Boost directory already exists, skipping download."
    fi

    # Copy podspec
    echo "Installing boost.podspec..."
    cp "${SCRIPT_DIR}/Podspecs/boost.podspec" "${BOOST_DIR}/boost.podspec"

    echo "Boost ${BOOST_VERSION} setup complete!"
}

# ============ SWIFT SETUP ============
setup_swift() {
    local SWIFT_DIR="${DEPS_DIR}/swift"
    local SOURCE_DIR="${SWIFT_DIR}/Source"

    echo ""
    echo "=== Setting up Swift ${SWIFT_VERSION} ==="

    mkdir -p "${SOURCE_DIR}"

    # Clone Swift main repository
    if [ ! -d "${SOURCE_DIR}/swift" ]; then
        echo "Cloning Swift repository..."
        git clone --branch "${SWIFT_TAG}" \
            https://github.com/swiftlang/swift.git "${SOURCE_DIR}/swift"
    else
        echo "Swift repository already exists, skipping clone."
    fi

    # Run update-checkout to get all dependencies at correct versions
    echo "Running update-checkout for tag ${SWIFT_TAG}..."
    cd "${SOURCE_DIR}/swift"
    ./utils/update-checkout --clone --tag "${SWIFT_TAG}" \
        --skip-history --source-root "${SOURCE_DIR}"

    # Generate headers without full compilation
    # --skip-build: Skip compilation, only run CMake configuration
    # --skip-early-swift-driver: Required to avoid llbuild compilation (has SDK compatibility issues)
    if [ ! -d "${SOURCE_DIR}/build" ]; then
        echo "Running build-script to generate headers (no compilation)..."
        ./utils/build-script \
            --reconfigure \
            --skip-build \
            --skip-early-swift-driver \
            --release
    else
        echo "Build directory exists, skipping header generation."
    fi

    # Copy podspec
    echo "Installing swift.podspec..."
    cp "${SCRIPT_DIR}/Podspecs/swift.podspec" "${SWIFT_DIR}/swift.podspec"

    echo "Swift ${SWIFT_VERSION} setup complete!"
}

# ============ MAIN ============
# Run Boost first (quick download), then Swift (takes longer)
setup_boost
setup_swift

echo ""
echo "=== Setup Complete ==="
echo ""
echo "Add to your Podfile:"
echo "  pod 'swift', :path => '../DanceUIDependencies/swift'"
echo "  pod 'boost', :path => '../DanceUIDependencies/boost'"
