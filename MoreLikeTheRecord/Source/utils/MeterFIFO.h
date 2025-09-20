#pragma once
#include <JuceHeader.h>

namespace mojo {

/**
 * Lock-free FIFO for passing meter data from audio thread to UI thread
 * @tparam DataType The type of data to store in the FIFO
 * @tparam Size The size of the FIFO buffer (must be a power of 2)
 */
template<typename DataType, int Size>
class MeterFIFO {
public:
    MeterFIFO() {
        // Ensure Size is a power of 2
        static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");
    }
    
    /**
     * Push a new value to the FIFO
     * @param value The value to push
     * @return true if the push was successful, false if the FIFO is full
     */
    bool push(const DataType& value) noexcept {
        const int writePos = writeIndex.load();
        const int nextWritePos = (writePos + 1) & (Size - 1);
        
        if (nextWritePos != readIndex.load()) {
            buffer[writePos] = value;
            writeIndex.store(nextWritePos);
            return true;
        }
        
        return false; // FIFO is full
    }
    
    /**
     * Pop a value from the FIFO
     * @param value Reference to store the popped value
     * @return true if a value was popped, false if the FIFO is empty
     */
    bool pop(DataType& value) noexcept {
        const int readPos = readIndex.load();
        
        if (readPos == writeIndex.load()) {
            return false; // FIFO is empty
        }
        
        value = buffer[readPos];
        readIndex.store((readPos + 1) & (Size - 1));
        return true;
    }
    
    /**
     * Clear the FIFO
     */
    void clear() noexcept {
        readIndex.store(0);
        writeIndex.store(0);
    }
    
private:
    DataType buffer[Size];
    std::atomic<int> readIndex { 0 };
    std::atomic<int> writeIndex { 0 };
};

} // namespace mojo
