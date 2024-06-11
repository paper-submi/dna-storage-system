#include <iostream>
#include<string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include "MD5.h"

void mainLoop(unsigned int M[], unsigned int temp[]) {
    unsigned int f, g, tmp;
    unsigned int a = temp[0];
    unsigned int b = temp[1];
    unsigned int c = temp[2];
    unsigned int d = temp[3];
    for (unsigned int i = 0; i < 64; ++i) {
        if (i < 16) {
            f = F(b, c, d);
            g = i;
        } else if (i < 32) {
            f = G(b, c, d);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            f = H(b, c, d);
            g = (3 * i + 5) % 16;
        } else {
            f = I(b, c, d);
            g = (7 * i) % 16;
        }
        tmp = d;
        d = c;
        c = b;
        b = b + LEFT_ROTATE_SHIFT((a + f + k[i] + M[g]), s[i]);
        a = tmp;
    }
    temp[0] += a;
    temp[1] += b;
    temp[2] += c;
    temp[3] += d;
}

void DecToHex(unsigned int num, char *out) {
    out[8] = 0;
    for (int i = 0; i < 4; ++i) {
        unsigned int byte_val = (num >> (i << 3)) & 0xff;
        for (int j = 0; j < 2; ++j) {
            out[i << 1 | (1 - j)] = msc_hexChars[byte_val & 0x0f];
            byte_val >>= 4;
        }
    }
}

std::string FinalizeMD5(MD5State &state) {
    // Handle the last block and padding
    state.bytes_content[state.ptr] = 0x80;
    ++state.ptr;

    if (state.ptr > 56) {
        // Need an additional block
        std::memset(state.bytes_content + state.ptr, 0, 64 - state.ptr);
        unsigned int *content = reinterpret_cast<unsigned int *>(state.bytes_content);
        mainLoop(content, state.temp);

        state.ptr = 0;
    }

    std::memset(state.bytes_content + state.ptr, 0, 56 - state.ptr);
    unsigned long long totalBits = state.len * 8;
    std::memcpy(state.bytes_content + 56, &totalBits, 8);

    unsigned int *content = reinterpret_cast<unsigned int *>(state.bytes_content);
    mainLoop(content, state.temp);

    char hex[33] = {0};
    for (int i = 0; i < 4; ++i) {
        DecToHex(state.temp[i], hex + (i << 3));
    }
    return std::string(hex);
}
