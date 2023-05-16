if(UNIX)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    add_compile_definitions(
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
        -D_CRT_SECURE_NO_WARNINGS
    )
endif()

add_subdirectory(simple)
