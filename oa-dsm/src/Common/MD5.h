#ifndef CONSENSUS_MD5_H
#define CONSENSUS_MD5_H

#include <cstddef>

#include <iostream>
#include<string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include "MD5.h"

#define LEFT_ROTATE_SHIFT(x, n)     (((x) << (n)) | ((x) >> (32-(n))))
#define F(x, y, z)                  (((x) & (y)) | (~(x) & (z)))        // (X AND Y) OR ((NOT X) AND Z)
#define G(x, y, z)                  (((x) & (z)) | ((y) & ~(z)))        // (X AND Z)
#define H(x, y, z)                  ( (x) ^ (y) ^ (z))
#define I(x, y, z)                  ( (y) ^ ((x) | ~(z)))
#define A                           0x67452301
#define B                           0xefcdab89
#define C                           0x98badcfe
#define D                           0x10325476
#define HEX_NUM_CHAR_MAX            16
#define HEX_NUM_UPPER_CHAR_SET      "0123456789ABCDEF"
#define HEX_NUM_LOWER_CHAR_SET      "0123456789abcdef"
#define HEX_NUM_CHAR_SET            HEX_NUM_LOWER_CHAR_SET

using namespace std;

const static char msc_hexChars[] = HEX_NUM_CHAR_SET;

const unsigned int k[] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

const unsigned int s[] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
};

void mainLoop(unsigned int M[], unsigned int temp[]);

void DecToHex(unsigned int num, char *out);

struct MD5State {
    unsigned long long len;
    unsigned int temp[4];
    size_t ptr;
    char bytes_content[64];

    MD5State() : len(0), ptr(0) {
        temp[0] = A;
        temp[1] = B;
        temp[2] = C;
        temp[3] = D;
    }
};

template<typename T>
void UpdateMD5(const std::vector<T> &data, MD5State &state) {
    size_t data_ptr = 0;

    while (data_ptr < data.size()) {
        size_t remaining_space = 64 - state.ptr;
        size_t remaining_data = data.size() - data_ptr;
        size_t to_copy = std::min(remaining_space, remaining_data);

        std::memcpy(state.bytes_content + state.ptr, data.data() + data_ptr, to_copy);
        state.len += to_copy;
        state.ptr += to_copy;
        data_ptr += to_copy;

        if (state.ptr == 64) {
            unsigned int *content = reinterpret_cast<unsigned int *>(state.bytes_content);
            mainLoop(content, state.temp);
            state.ptr = 0;
        }
    }
}

std::string FinalizeMD5(MD5State &state);

/*template<typename T>
std::string computeMD5(const std::vector<T> &data, MD5State &state) {

    unsigned long long len = 0;
    //unsigned int temp[] = {A, B, C, D};

    //char bytes_content[64];
    size_t ptr = 0;
    while (true) {
        size_t end = min(ptr + 64, data.size());
        size_t to_read = end - ptr;
        std::memcpy(state.bytes_content, data.data() + ptr, 64);
        len += 1;
        unsigned int *content = reinterpret_cast<unsigned int *>(state.bytes_content);
        if (to_read < 64) {//ends, filling
            state.bytes_content[to_read] = 0x80;
            for (unsigned int i = to_read + 1; i < 56; ++i) {
                state.bytes_content[i] = 0x00;
            }

            if (to_read >= 56) {
                //fill
                for (unsigned int i = to_read + 1; i < 64; ++i) {
                    state.bytes_content[i] = 0x00;
                }
                mainLoop(content, state.temp);
                for (unsigned int i = 0; i < 56; ++i) {
                    state.bytes_content[i] = 0x00;
                }

            }
            unsigned long long *len_ptr = (unsigned long long *) (content + 14);
            *len_ptr = ((len - 1) * 64 + to_read) * 8;
        }

        mainLoop(content, state.temp);
        ptr += 64;
        if (ptr > data.size()) {
            break;
        }
    }

    char hex[32 + 1] = {0};
    for (int i = 0; i < 4; ++i) {
        DecToHex(state.temp[i], hex + (i << 3));
    }
    std::string md5 = std::string(hex);
    return md5;
}*/

#endif //CONSENSUS_MD5_H
