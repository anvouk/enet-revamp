if(UNIX)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(simple)
add_subdirectory(ping-pong)
