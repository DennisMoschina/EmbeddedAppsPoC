# Base directory for micropython_embed
set(EMBED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/micropython_embed)

target_compile_definitions(app PRIVATE asm=__asm__)

# Include the base directory for correct include resolution
# This ensures py/... and other similar includes resolve correctly
target_include_directories(app PRIVATE
    ${EMBED_DIR}   # Adds micropython_embed/ so py/... resolves
)

# Compiler flags
target_compile_options(app PRIVATE
    -Wall
    -Og
    -fno-common
)

# Collect all source files within the micropython_embed directory
file(GLOB_RECURSE EMBED_SOURCES
    ${EMBED_DIR}/*/*.c
    ${EMBED_DIR}/*/*/*.c
)

# Add all sources to the embed library
target_sources(app PRIVATE
    ${EMBED_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/microPython.c
)