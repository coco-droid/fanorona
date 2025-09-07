#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="build"
EXECUTABLE="fanorona"
DEBUG_MODE=false
CLEAN_BUILD=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            DEBUG_MODE=true
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -d, --debug    Build in debug mode"
            echo "  -c, --clean    Clean build directory before building"
            echo "  -h, --help     Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if required libraries are installed
check_dependencies() {
    print_status "Checking dependencies..."
    
    # Check for SDL2
    if ! pkg-config --exists sdl2; then
        print_error "SDL2 not found. Install with: sudo apt-get install libsdl2-dev"
        exit 1
    fi
    
    # Check for SDL2_ttf
    if ! pkg-config --exists SDL2_ttf; then
        print_error "SDL2_ttf not found. Install with: sudo apt-get install libsdl2-ttf-dev"
        exit 1
    fi
    
    # Check for SDL2_image
    if ! pkg-config --exists SDL2_image; then
        print_error "SDL2_image not found. Install with: sudo apt-get install libsdl2-image-dev"
        exit 1
    fi
    
    # Check for SDL2_mixer (optional, for audio)
    if ! pkg-config --exists SDL2_mixer; then
        print_warning "SDL2_mixer not found. Audio will be disabled. Install with: sudo apt-get install libsdl2-mixer-dev"
    fi
    
    print_success "Dependencies check completed"
}

# Create build directory
setup_build_dir() {
    if [[ "$CLEAN_BUILD" == true ]] && [[ -d "$BUILD_DIR" ]]; then
        print_status "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi
    
    if [[ ! -d "$BUILD_DIR" ]]; then
        print_status "Creating build directory..."
        mkdir -p "$BUILD_DIR"
    fi
}

# Build the project
build_project() {
    print_status "Building Fanorona game..."
    
    # Compiler and flags
    CC="gcc"
    
    # Base flags
    CFLAGS="-std=c99 -Wall -Wextra -Wpedantic"
    
    # Debug or release flags
    if [[ "$DEBUG_MODE" == true ]]; then
        CFLAGS="$CFLAGS -g -O0 -DDEBUG"
        print_status "Building in DEBUG mode"
    else
        CFLAGS="$CFLAGS -O2 -DNDEBUG"
        print_status "Building in RELEASE mode"
    fi
    
    # SDL2 flags
    SDL_CFLAGS=$(pkg-config --cflags sdl2 SDL2_ttf SDL2_image)
    SDL_LIBS=$(pkg-config --libs sdl2 SDL2_ttf SDL2_image)
    
    # Add SDL2_mixer if available
    if pkg-config --exists SDL2_mixer; then
        SDL_CFLAGS="$SDL_CFLAGS $(pkg-config --cflags SDL2_mixer)"
        SDL_LIBS="$SDL_LIBS $(pkg-config --libs SDL2_mixer)"
        CFLAGS="$CFLAGS -DHAVE_SDL_MIXER"
    fi
    
    # Math library
    LIBS="-lm $SDL_LIBS"
    
    # Include directories
    INCLUDES="-Isrc"
    
    # Source files
    SOURCES=(
        "src/main.c"
        "src/core/sdl_init.c"
        "src/core/timer.c"
        "src/core/config.c"
        "src/window/window_manager.c"  # Add this line
        "src/layer/layer.c"
        "src/layer/layer_manager.c"
        "src/layer/dirty_rect.c"
        "src/layer/render_target.c"
        "src/ui/widget.c"
        "src/ui/button.c"
        "src/ui/pieces_widget.c"
        "src/ui/animation.c"
        "src/scenes/scene.c"
        "src/scenes/game_scene.c"
        "src/scenes/menu_scene.c"
        "src/engine/fanorona.c"
        "src/engine/game_state.c"
        "src/event/event_dispatcher.c"
        "src/event/coordinate_utils.c"
        "src/event/hitbox.c"
        "src/net/p2p.c"
        "src/audio/audio.c"
        "src/ai/minimax.c"
        "src/ai/gnn_inference.c"
        "src/analyzer/postgame.c"
    )
    
    # Filter existing source files
    EXISTING_SOURCES=()
    for src in "${SOURCES[@]}"; do
        if [[ -f "$src" ]]; then
            EXISTING_SOURCES+=("$src")
        else
            print_warning "Source file not found: $src"
        fi
    done
    
    # Build command
    BUILD_CMD="$CC $CFLAGS $INCLUDES $SDL_CFLAGS ${EXISTING_SOURCES[*]} -o $BUILD_DIR/$EXECUTABLE $LIBS"
    
    print_status "Executing: $BUILD_CMD"
    
    # Execute build
    if $BUILD_CMD 2>&1 | tee "$BUILD_DIR/build.log"; then
        print_success "Build completed successfully!"
    else
        print_error "Build failed! Check $BUILD_DIR/build.log for details."
        exit 1
    fi
}

# Run the game
run_game() {
    if [[ -f "$BUILD_DIR/$EXECUTABLE" ]]; then
        print_status "Starting Fanorona game..."
        
        # Copy the icon to the build directory if it exists
        if [[ -f "icone.png" ]]; then
            cp "icone.png" "$BUILD_DIR/"
            print_status "Icon copied to build directory"
        elif [[ -f "assets/icone.png" ]]; then
            mkdir -p "$BUILD_DIR/assets"
            cp "assets/icone.png" "$BUILD_DIR/assets/"
            cp "assets/icone.png" "$BUILD_DIR/"
            print_status "Icon copied from assets to build directory"
        else
            print_warning "Icon file not found. Creating a simple test icon..."
            # Create a simple 32x32 PNG icon if none exists
            convert -size 32x32 xc:blue "$BUILD_DIR/icone.png" 2>/dev/null || print_warning "Could not create test icon (imagemagick not installed)"
        fi
        
        cd "$BUILD_DIR"
        ./"$EXECUTABLE"
        cd ..
    else
        print_error "Executable not found: $BUILD_DIR/$EXECUTABLE"
        exit 1
    fi
}

# Main execution
main() {
    print_status "Fanorona Game Build Script"
    echo "=========================="
    
    check_dependencies
    setup_build_dir
    build_project
    
    print_success "Build process completed!"
    
    # Ask if user wants to run the game
    read -p "Do you want to run the game now? [y/N]: " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        run_game
    else
        print_status "Game executable is ready at: $BUILD_DIR/$EXECUTABLE"
    fi
}

# Run main function
main "$@"
