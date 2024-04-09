/**
 * @file bitstream.hpp
 * @authors @GersonFeDutra
 * @brief Data Structure to hold a stream of booleans as an array.
 * @version 0.1
 * @date 2024-03-15
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <stdbool.h>

/** @brief Ceiling Division:
 * Returns the smallest multiple N of y such that x <= y * N. */
#define ceil_div(x, y) (((x) + (y) - 1) / (y))

/** @brief 8 bits in a byte */
#define BITS 8
/** @brief Word length:
 * Number of bits in a bool
 */
#define WORD_LEN sizeof(bool) * BITS

class BitSlice {
  private:
    bool slice;
  public:
    BitSlice(bool value, int idx) {
        slice = (value) ? ((bool)01) << idx : 0;
    }
    bool get() {
      return slice;
    }
};

class BitStream {
  private:
    bool *stream;
  public:
    BitStream(size_t len) {
    	stream = new bool[ceil_div(len, WORD_LEN)];
    }
	~BitStream() {
		delete[] stream;
	}
    bool operator[](int index) {
        return stream[index / WORD_LEN] & (((bool)01) << (index % WORD_LEN));
    }
    bool set(int index, bool value) {
        BitSlice bit(value, index);
        stream[index / WORD_LEN] |= bit.get();
        return bit.get();
    }
};
