#include <cstring>
#include <libabyss/common/ringbuffer.h>

namespace LibAbyss {

RingBuffer::RingBuffer(uint32_t bufferSize)
    : _bufferSize(bufferSize), _buffer(bufferSize), _readPosition(0), _writePosition(0), _remainingToRead(0), _mutex() {}

void RingBuffer::PushData(std::span<const uint8_t> data) {
    std::lock_guard<std::mutex> guard(_mutex);

    // Determine the bytes we have left to write before we hit the read position
    const auto remainingSize = _bufferSize - _remainingToRead;

    // Grab the amount of data we are going to write
    const auto toWrite = data.size();

    // Determine the amount of overflow (if we are writing past the current read position)
    if (toWrite > remainingSize)
        throw std::runtime_error("RingBuffer overflow");

    // Start with the current write position
    auto writePos = _writePosition;

    // Wrap write position
    while (writePos >= _bufferSize)
        writePos -= _bufferSize;

    // Write all the bytes
    for (auto i = 0; i < toWrite; i++) {
        _buffer[writePos++] = data[i];

        // Wrap the writer index around to the beginning if we reached the end
        while (writePos >= _bufferSize)
            writePos -= _bufferSize;
    }

    // Add data to the data we can now read
    _remainingToRead += toWrite;

    // Update write position
    _writePosition = writePos;

    // Wrap the read position
    while (_readPosition >= _bufferSize)
        _readPosition -= _bufferSize;
}
void RingBuffer::ReadData(std::span<uint8_t> outBuffer) {
    std::lock_guard<std::mutex> guard(_mutex);

    const auto toRead = std::min(_remainingToRead, (uint32_t)outBuffer.size());
    memset(outBuffer.data(), 0, outBuffer.size());
    if (outBuffer.empty() || toRead == 0) {
        return;
    }

    auto readPos = _readPosition;

    while (readPos >= _bufferSize)
        readPos -= _bufferSize;

    for (auto i = 0; i < toRead; i++) {
        outBuffer[i] = _buffer[readPos++];

        while (readPos >= _bufferSize)
            readPos -= _bufferSize;
    }

    _readPosition = readPos;
    _remainingToRead -= toRead;
}
void RingBuffer::Reset() {
    std::lock_guard<std::mutex> guard(_mutex);

    _readPosition = 0;
    _writePosition = 0;
    _remainingToRead = 0;
}
uint32_t RingBuffer::Available() { return _remainingToRead; }

} // namespace LibAbyss
