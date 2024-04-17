add_executable(ring_buffer_test ./RingBufferTest.c ../src/common/RingBuffer.c)
target_link_libraries(ring_buffer_test)
target_compile_definitions(ring_buffer_test PRIVATE NO_LOGGING)

add_test(NAME ring_buffer_test COMMAND $<TARGET_FILE:ring_buffer_test>)
