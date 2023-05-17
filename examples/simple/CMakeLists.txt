add_executable(example-simple-server server.c shared.h)
target_link_libraries(example-simple-server PRIVATE enet)

add_executable(example-simple-client client.c shared.h)
target_link_libraries(example-simple-client PRIVATE enet)
