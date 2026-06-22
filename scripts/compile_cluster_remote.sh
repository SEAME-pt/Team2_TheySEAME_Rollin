#!/bin/bash

################################################################################
# Cluster Qt Application Remote Compilation Script
# 
# This script mounts the remote development folder via sshfs and compiles the
# Cluster Qt application on the remote machine.
#
# Prerequisites:
#   - sshfs installed: sudo apt-get install sshfs
#   - SSH key authentication configured for team2@100.103.187.55
#   - Remote Qt 6.7.3 installed at ~/Qt/6.7.3/gcc_64/
#
# Usage:
#   ./scripts/compile_cluster_remote.sh
#   ./scripts/compile_cluster_remote.sh --only-mount
#   ./scripts/compile_cluster_remote.sh --only-compile
#   ./scripts/compile_cluster_remote.sh --unmount
################################################################################

set -e

# Configuration
REMOTE_HOST="team2@100.103.187.55"
REMOTE_PATH="/home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin"
LOCAL_MOUNT_POINT="$HOME/remote-elrepo"
LOCAL_PROJECT_DIR="$HOME/SEA_ME/ElRepo"
CLUSTER_APP_DIR="apps/Cluster/qtApp"
QT_CMAKE="~/Qt/6.7.3/gcc_64/bin/qt-cmake"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Functions
print_header() {
    echo -e "${BLUE}==============================================================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}==============================================================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}→ $1${NC}"
}

mount_remote() {
    print_header "Mounting Remote Folder via sshfs"
    
    # Create mount point if it doesn't exist
    if [ ! -d "$LOCAL_MOUNT_POINT" ]; then
        print_info "Creating mount point: $LOCAL_MOUNT_POINT"
        mkdir -p "$LOCAL_MOUNT_POINT"
    fi
    
    # Check if already mounted
    if mount | grep -q "$LOCAL_MOUNT_POINT"; then
        print_success "Remote folder already mounted at $LOCAL_MOUNT_POINT"
        return 0
    fi
    
    # Mount via sshfs
    print_info "Mounting $REMOTE_HOST:$REMOTE_PATH to $LOCAL_MOUNT_POINT"
    if sshfs "$REMOTE_HOST:$REMOTE_PATH" "$LOCAL_MOUNT_POINT" -o allow_other,follow_symlinks; then
        print_success "Successfully mounted remote folder"
        return 0
    else
        print_error "Failed to mount remote folder"
        return 1
    fi
}

unmount_remote() {
    print_header "Unmounting Remote Folder"
    
    if ! mount | grep -q "$LOCAL_MOUNT_POINT"; then
        print_success "Remote folder not mounted"
        return 0
    fi
    
    print_info "Unmounting $LOCAL_MOUNT_POINT"
    if fusermount -u "$LOCAL_MOUNT_POINT"; then
        print_success "Successfully unmounted remote folder"
        return 0
    else
        print_error "Failed to unmount remote folder"
        return 1
    fi
}

compile_remote() {
    print_header "Compiling Cluster Qt Application on Remote Machine"
    
    print_info "Remote compilation command via SSH"
    ssh "$REMOTE_HOST" << 'REMOTE_SCRIPT'
set -e
REMOTE_PATH="/home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin"
QT_CMAKE="$HOME/Qt/6.7.3/gcc_64/bin/qt-cmake"
CLUSTER_APP_DIR="apps/Cluster/qtApp"
    export PATH="$HOME/Qt/6.7.3/gcc_64/bin:$PATH"

cd "$REMOTE_PATH/$CLUSTER_APP_DIR"

echo "Creating build directory..."
rm -rf build
mkdir -p build
cd build

echo "Running qt-cmake..."
$QT_CMAKE ..

echo "Building project..."
cmake --build . --parallel $(nproc)

echo "Build completed successfully!"
ls -lh qtAppExec
ls -lh ../qml/3d-assets/carro/LowPolyFiatUNO.* || true
REMOTE_SCRIPT
    
    print_success "Remote compilation completed"
}

sync_local() {
    print_header "Syncing Project Files to Remote"
    
    print_info "Syncing QML and source files"
    
    local local_qml_path="$LOCAL_PROJECT_DIR/$CLUSTER_APP_DIR/qml"
    local local_src_path="$LOCAL_PROJECT_DIR/$CLUSTER_APP_DIR/src"
    local local_cmake_file="$LOCAL_PROJECT_DIR/$CLUSTER_APP_DIR/CMakeLists.txt"
    local local_root_qrc_file="$LOCAL_PROJECT_DIR/$CLUSTER_APP_DIR/qml.qrc"
    local local_main_file="$LOCAL_PROJECT_DIR/$CLUSTER_APP_DIR/main.cpp"
    local local_qt_conf_file="$LOCAL_PROJECT_DIR/$CLUSTER_APP_DIR/qt.conf"
    local local_libbkuksa_path="$LOCAL_PROJECT_DIR/libs/libkuksa"
    local remote_app_path="$REMOTE_PATH/$CLUSTER_APP_DIR"
    local remote_libbkuksa_path="$REMOTE_PATH/libs/libkuksa"

    # Sync build configuration files
    print_info "Copying build configuration files..."
    if [ -f "$local_cmake_file" ]; then
        scp "$local_cmake_file" "$REMOTE_HOST:$remote_app_path/" 2>/dev/null || \
            print_info "CMakeLists sync completed"
    fi
    if [ -f "$local_main_file" ]; then
        scp "$local_main_file" "$REMOTE_HOST:$remote_app_path/" 2>/dev/null || \
            print_info "main.cpp sync completed"
    fi
    if [ -f "$local_qt_conf_file" ]; then
        scp "$local_qt_conf_file" "$REMOTE_HOST:$remote_app_path/" 2>/dev/null || \
            print_info "qt.conf sync completed"
    fi
    if [ -f "$local_root_qrc_file" ]; then
        scp "$local_root_qrc_file" "$REMOTE_HOST:$remote_app_path/" 2>/dev/null || \
            print_info "qml.qrc sync completed"
    fi
    
    # Sync QML files (critical for UI updates)
    if [ -d "$local_qml_path" ]; then
        print_info "Copying QML files..."
        scp -r "$local_qml_path"/* "$REMOTE_HOST:$remote_app_path/qml/" 2>/dev/null || \
            print_info "QML sync completed"
    fi
    
    # Sync source files (C++ code)
    if [ -d "$local_src_path" ]; then
        print_info "Copying source files..."
        scp -r "$local_src_path"/* "$REMOTE_HOST:$remote_app_path/src/" 2>/dev/null || \
            print_info "Source sync completed"
    fi

    # Sync shared kuksa library sources/headers used by qtApp
    if [ -d "$local_libbkuksa_path" ]; then
        print_info "Copying libkuksa files..."
        scp -r "$local_libbkuksa_path"/* "$REMOTE_HOST:$remote_libbkuksa_path/" 2>/dev/null || \
            print_info "libkuksa sync completed"
    fi
    
    print_info "Syncing proto headers from local to remote"
    
    local local_proto_path="$LOCAL_PROJECT_DIR/$CLUSTER_APP_DIR/../../middleware/kuksa/val/v2"
    local remote_proto_path="$REMOTE_PATH/apps/middleware/kuksa/val/v2"
    
    if [ ! -d "$local_proto_path" ]; then
        print_error "Proto source path not found: $local_proto_path"
        return 1
    fi
    
    print_info "Copying .pb.h, .pb.cc, .grpc.pb.h, .grpc.pb.cc files"
    
    scp -r "$local_proto_path"/*.{pb.h,pb.cc,grpc.pb.h,grpc.pb.cc} \
        "$REMOTE_HOST:$remote_proto_path/" 2>/dev/null || \
        print_info "Proto files already synced or partially synced"
    
    print_success "Project file sync completed"
}

show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  (no args)        Mount remote and compile"
    echo "  --only-mount     Only mount remote folder"
    echo "  --only-compile   Only compile (assumes already mounted)"
    echo "  --unmount        Unmount remote folder"
    echo "  --help           Show this help message"
}

# Main execution
main() {
    case "${1:-}" in
        --help)
            show_usage
            exit 0
            ;;
        --only-mount)
            mount_remote
            ;;
        --only-compile)
            compile_remote
            ;;
        --unmount)
            unmount_remote
            ;;
        "")
            # Default: mount and compile
            mount_remote
            sync_local
            compile_remote
            print_success "All tasks completed!"
            echo ""
            echo "Binary location: $LOCAL_MOUNT_POINT/$CLUSTER_APP_DIR/build/qtAppExec"
            echo ""
            echo "To run the application:"
            echo "  ssh $REMOTE_HOST 'QSG_INFO=1 QSG_RHI_BACKEND=opengl QT_QPA_PLATFORM=wayland $REMOTE_PATH/$CLUSTER_APP_DIR/build/qtAppExec'"
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
}

# Run main
main "$@"
