add_executable(example-ping-pong-server server.c shared.h)
target_link_libraries(example-ping-pong-server PRIVATE enet)

add_executable(example-ping-pong-client client.c shared.h)
target_link_libraries(example-ping-pong-client PRIVATE enet)
