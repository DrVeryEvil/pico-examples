# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

add_executable(blink_simple
        blink_simple.c
)

# pull in common dependencies
target_link_libraries(blink_simple pico_stdlib)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(blink_simple)

# call pico_set_program_url to set path to example on github, so users can find the source for an example via picotool
example_auto_set_url(blink_simple)
