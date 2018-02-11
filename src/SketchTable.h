//
// Created by medek on 7.3.17.
//

#ifndef SKETCH_SKETCHTABLE_H
#define SKETCH_SKETCHTABLE_H

#include <array>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <cmath>
#include <stdint.h>
#include <random>
#include "misc_functions.h"

/**
 * class SketchTable
 * stores how many times given value has been stored. each index of table is hash of value
 * and element on that index is count of insertions of values that hash to index.
*/
class SketchTable
{
//    friend class Sketch;
//    friend class SketchRange;
//protected:
public:
    std::array<uint32_t, MODULAR_MAX> table;
    std::array<uint8_t, INDEX_COUNT> values_map; // array used directly for hashing. hash of x is values[x]
    std::array<std::vector<uint8_t>, 8> reverse_hashing_map; // 2d array for fast reverse hashing
    std::vector<uint32_t> heavy_buckets;

    /**
     * SketchTable
     * generates maps for hashing and reverse hashing
    */
    SketchTable(std::mt19937 & g);

    /**
     * SketchTable
     * creates table with given values map and reverse hashing map
    */
    SketchTable(const std::array<uint8_t, 256> &values_map,
                const std::array<std::vector<uint8_t>, 8> &reverse_hashing_map);

    /**
     * insert
     * creates complete hash by mangling value and concatenating modular hashes of value words.
     * appends @param size at value hash. Default value for size is 1.
    */
    void insert_size(uint32_t src, uint32_t dst, uint32_t size);

    /**
     * get_reverse_by_word
     * returns all possible reverse modular hashes of word on given index
    */
    const std::vector<uint8_t> & get_reverse_by_word(uint32_t word, unsigned word_index) const;

    /**
     * print_table
     * prints all non-zero values in table
    */
    void print_table();

    /**
     * in_what_heavy_bucket_is_this_word
     * returns all buckets, that contain on given index hash of given word.
    */
    std::set<uint32_t> in_what_heavy_buckets_is_this_word(uint8_t word, unsigned word_index,
                                                          const std::vector<uint32_t> &buckets);

    /**
     * print_statistics
     * debug function for testing collision rate and dispersion of hash function
    */
    void print_statistics();

    /**
     * calculate_buckets
     * saves 5 hashes with most occurrences in table
    */
    void calculate_buckets();

    /**
     * get_buckets
     * returns heavy buckets of the table
    */
    const std::vector<uint32_t> & get_buckets() const;

    /**
     * hash
     * computes modular hash of every byte of value and concatenates them
    */
    uint16_t hash_ip(uint32_t value);

    /**
    * modular_hash
    * hashes a value of size 1 byte using values_map attribute
    */
    uint8_t modular_hash(uint8_t value) const;

//    SketchTable * operator+(const SketchTable &another);
};

#endif //SKETCH_SKETCHTABLE_H
