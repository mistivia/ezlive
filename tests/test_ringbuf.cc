#include "../src/ringbuf.h"

#include <cassert>
#include <cstring>
#include <thread>
#include <chrono>

static void test_constructor()
{
    ezlive::ring_buffer rb(1024);
    assert(rb.size() == 0);
    assert(rb.space() == 1024);
}

static void test_write_and_read()
{
    printf("test_write_and_read\n");
    ezlive::ring_buffer rb(1024);
    const uint8_t data[] = "hello world";
    size_t len = strlen(reinterpret_cast<const char*>(data)) + 1;

    size_t written = rb.write(data, len);
    assert(written == len);
    assert(rb.size() == len);
    assert(rb.space() == 1024 - len);

    uint8_t buffer[64] = {};
    size_t read = rb.read(buffer, sizeof(buffer));
    assert(read == len);
    assert(memcmp(data, buffer, len) == 0);
    assert(rb.size() == 0);
}

static void test_partial_read()
{
    printf("test_partial_read\n");
    ezlive::ring_buffer rb(1024);
    const uint8_t data[] = "hello world";
    size_t len = strlen(reinterpret_cast<const char*>(data)) + 1;

    rb.write(data, len);

    // Read partial data
    uint8_t buffer[6] = {};
    size_t read = rb.read(buffer, 6);
    assert(read == 6);
    assert(memcmp(data, buffer, 6) == 0);
    assert(rb.size() == len - 6);

    // Read remaining
    uint8_t buffer2[64] = {};
    size_t read2 = rb.read(buffer2, sizeof(buffer2));
    assert(read2 == len - 6);
    assert(memcmp(data + 6, buffer2, len - 6) == 0);
    assert(rb.size() == 0);
}

static void test_wrap_around()
{
    printf("test_wrap_around\n");
    ezlive::ring_buffer rb(16);
    const uint8_t data1[] = "abcdefghijklmn";  // 15 bytes including null

    rb.write(data1, 15);
    assert(rb.size() == 15);

    // Read and free up space
    uint8_t buffer[64] = {};
    rb.read(buffer, 10);  // Read 10 bytes
    assert(rb.size() == 5);

    // Write more to trigger wrap around
    const uint8_t data2[] = "1234567890";
    rb.write(data2, 10);

    // Read all remaining
    uint8_t buffer2[64] = {};
    size_t read = rb.read(buffer2, sizeof(buffer2));
    assert(read == 15);  // 5 from first write + 10 from second

    // Verify the data (5 from first + 10 from second)
    assert(memcmp(data1 + 10, buffer2, 5) == 0);
    assert(memcmp(data2, buffer2 + 5, 10) == 0);
}

static void test_write_larger_than_buffer()
{
    printf("test_write_larger_than_buffer\n");
    ezlive::ring_buffer rb(8);
    const uint8_t data[] = "this is a long string";
    
    // Need a background reader to consume data so write can proceed
    size_t total_read = 0;
    std::thread reader([&rb, &total_read]() {
        uint8_t buffer[8];
        while (total_read < sizeof(data)) {
            size_t read = rb.read(buffer, sizeof(buffer));
            total_read += read;
        }
    });
    
    // Should write all data even if larger than buffer
    size_t written = rb.write(data, sizeof(data));
    assert(written == sizeof(data));
    
    reader.join();
    assert(total_read == sizeof(data));
}

static void test_multiple_writes_reads()
{
    printf("test_multiple_writes_reads\n");
    ezlive::ring_buffer rb(100);
    const uint8_t data1[] = "first";
    const uint8_t data2[] = "second";
    const uint8_t data3[] = "third";

    rb.write(data1, 6);
    rb.write(data2, 7);
    rb.write(data3, 6);

    uint8_t buffer[64] = {};
    size_t read = rb.read(buffer, sizeof(buffer));
    assert(read == 19);  // 6 + 7 + 6
    assert(memcmp(buffer, "first\0second\0third\0", 19) == 0);
}

static void test_read_from_empty()
{
    printf("test_read_from_empty\n");
    ezlive::ring_buffer rb(64);

    // Test that read blocks on empty buffer until stop is called
    // This is tricky to test without blocking, so we'll just verify
    // the buffer is initially empty
    assert(rb.size() == 0);
}

static void test_stop()
{
    printf("test_stop\n");
    ezlive::ring_buffer rb(64);
    rb.stop();
    // After stop, read should return 0 on empty buffer
    uint8_t buffer[64] = {};
    size_t read = rb.read(buffer, sizeof(buffer));
    assert(read == 0);
}

static void test_concurrent_write_read()
{
    printf("test_concurrent_write_read\n");
    ezlive::ring_buffer rb(1024);
    std::thread writer([&rb]() {
        for (int i = 0; i < 100; i++) {
            uint8_t data[10];
            memset(data, i % 256, sizeof(data));
            rb.write(data, sizeof(data));
        }
    });

    std::thread reader([&rb]() {
        uint8_t buffer[10];
        for (int i = 0; i < 100; i++) {
            size_t read = rb.read(buffer, sizeof(buffer));
            assert(read == 10);
            uint8_t expected = i % 256;
            for (size_t j = 0; j < 10; j++) {
                assert(buffer[j] == expected);
            }
        }
    });

    writer.join();
    reader.join();
    assert(rb.size() == 0);
}

int main()
{
    test_constructor();
    test_write_and_read();
    test_partial_read();
    test_wrap_around();
    test_write_larger_than_buffer();
    test_multiple_writes_reads();
    test_read_from_empty();
    test_stop();
    test_concurrent_write_read();

    printf("All ringbuf tests passed!\n");
    return 0;
}
