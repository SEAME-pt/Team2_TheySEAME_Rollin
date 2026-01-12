XX!/bin/bash

# Setup script for CMock + Ceedling + GTest testing environment
# This script installs dependencies and sets up the testing environment

set -e  # Exit on any error

echo "=== Setting up CMock + Ceedling + GTest Environment ==="

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check if gem is installed
gem_exists() {
    gem list "$1" -i >/dev/null 2>&1
}

# Install Ruby if not present
if ! command_exists ruby; then
    echo "Installing Ruby..."
    sudo apt-get update
    sudo apt-get install -y ruby ruby-dev build-essential
else
    echo "✓ Ruby is already installed: $(ruby --version)"
fi

# Install Ceedling gem if not present
if ! gem_exists ceedling; then
    echo "Installing Ceedling gem..."
    sudo gem install ceedling
else
    echo "✓ Ceedling gem is already installed: $(gem list ceedling)"
fi

# Install GoogleTest if not present
if ! pkg-config --exists gtest; then
    echo "Installing GoogleTest..."
    sudo apt-get install -y libgtest-dev cmake
    
    # Build GoogleTest (Ubuntu doesn't provide pre-built libraries)
    cd /usr/src/gtest
    sudo cmake .
    sudo make
    sudo cp lib/* /usr/lib/ 2>/dev/null || sudo cp *.a /usr/lib/
    cd - >/dev/null
else
    echo "✓ GoogleTest is already installed"
fi

echo ""
echo "=== Environment Setup Complete! ==="
echo ""
echo "Next steps:"
echo "1. Generate mock files: make generate-mocks"
echo "2. Run tests: make test"
echo "3. Or run both: make all"
echo ""
echo "For help: make help"