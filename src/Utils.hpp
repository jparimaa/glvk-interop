#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstdio>

const int c_windowWidth = 1600;
const int c_windowHeight = 1200;

#define CHECK(f)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(f))                                                          \
        {                                                                  \
            printf("Abort. %s failed at %s:%d\n", #f, __FILE__, __LINE__); \
            abort();                                                       \
        }                                                                  \
    } while (false)

template<typename T>
uint32_t ui32Size(const T& container)
{
    return static_cast<uint32_t>(container.size());
}