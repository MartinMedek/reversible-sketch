//
// Created by med on 27.3.17.
//

#ifndef SKETCH_BIV_H
#define SKETCH_BIV_H

#include <array>
#include <set>
#include <cstdint>
#include "misc_functions.h"

typedef std::array<std::set<uint32_t>, TABLE_COUNT> BucketMap; // map of all buckets, that all words of BIV hash to.

/**
 * Bucket Index Vector
 * BIV is used for computing original values inserted to sketch from heavy buckets.
 *
 * If vector contains WORD_COUNT values and at least BucketMap in at least (TABLE_COUNT-R) tables is't empty,
 * value chain can be considered reverse hash of heavy buckets in Sketch
 */
class BIV{
    /** word_map stores vector of words, where, once word_map is filled, concatenation of all words is the original reverse hashed value. **/
    std::array<uint64_t, WORD_COUNT> word_map{};

    /** bucket_map is map of buckets, that for all words of word map have on its index its word. **/
    BucketMap bucket_map;
public:
    BIV() = default;

    BIV(const BIV & another){
        for (unsigned i = 0; i < WORD_COUNT; ++i) {
            word_map[i] = another.word_map[i];
        }
        for (int j = 0; j < TABLE_COUNT; ++j) {
            bucket_map[j].insert(another.bucket_map[j].begin(), another.bucket_map[j].end());
        }
    }

    const std::set<uint32_t> & get_buckets_at(unsigned index) const {
        return bucket_map[index];
    }

    void set_buckets_at(unsigned index, std::set<uint32_t> & vec) {
        bucket_map[index] = vec;
    }

    uint64_t get_word_at(unsigned index) const {
        return word_map[index];
    }

    void set_word_at(unsigned index, uint64_t val) {
        word_map[index] = val;
    }

    /** concatenate words from word map to reconstruct original value inserted to sketch **/
    uint64_t get_image_from_vector() const {
        uint64_t result = 0;
        for (int i = 0; i < WORD_COUNT; ++i) {
            result |= (word_map[i] << (i*8));
        }
        return result;
    }
};

#endif //SKETCH_BIV_H
