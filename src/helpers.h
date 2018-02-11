//
// Created by med on 16.5.17.
//

#ifndef SKETCH_HELPERS_H
#define SKETCH_HELPERS_H

/**
* rotate_right
* cyclic bitwise shift
*/
inline uint32_t rotate_right(uint32_t v, unsigned r){
    return (v>>r) | (v<<(32-r));
}

/**
* rotate_left
* cyclic bitwise shift
*/
inline uint32_t rotate_left(uint32_t v, unsigned r){
    return (v<<r) | (v>>(32-r));
}


/**
* fast_atoi
* fast atoi for unsigned integers
*/
template<typename T> T fast_atoi(const char * str){
    T val = 0;
    while(*str) {
        val = static_cast<T>(val * 10 + (*str++ - '0'));
    }
    return val;
}


/**
 * print_binary
 * @param value that is to be printed in binary
 * **/
template<typename T> void print_binary(T value)
{
    std::cout << std::bitset<sizeof(T) * 8>(value) << std::endl;
}

/**
 * print_simple_container
 * formatted print of container
 * @param value any container of primitive values that can be iterated
 * **/
template<typename T> void print_simple_container(T & container)
{
    std::cout << "contained values: ";
    for(auto item : container) {
        std::cout << static_cast<uint64_t>(item) << ' ';
    }
    std::cout << std::endl;
}

template<typename T> void print_zero_nonzero_count(T &container)
{
    unsigned counter = 0;
    unsigned zero_counter = 0;
    for(auto item : container) {
        if(item){
            counter++;
        }
        else{
            zero_counter++;
        }
    }
    std::cout << "zeros : " << zero_counter << " non zeros: " << counter << std::endl;
}

template<typename T> void print_simple_map(T & map)
{
    for(auto item : map) {
        std::cout << "key: " << static_cast<uint64_t>(item.first) << " value: " << static_cast<uint64_t>(item.second) << std::endl;
    }
}

inline uint32_t binary_to_decimal(const char * binary){
    std::bitset<32> x (binary);
    return static_cast<uint32_t>(x.to_ulong());
}

#endif //SKETCH_HELPERS_H
