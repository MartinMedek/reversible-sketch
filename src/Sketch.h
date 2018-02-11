//
// Created by medek on 7.3.17.
//

#ifndef SKETCH_SKETCH_TABLE_H
#define SKETCH_SKETCH_TABLE_H

#include <array>
#include <algorithm>
#include <list>
#include <cstdint>
#include "SketchTable.h"
#include "BIV.h"
#include "misc_functions.h"

class Sketch
{
public:
    std::array<SketchTable *, TABLE_COUNT> tables;
    std::list<uint64_t> reversed_hashes;
    std::array<std::map<uint8_t, HeavyBuckets>, WORD_COUNT> modular_potentials;

    /** simple constructor **/
    Sketch();

    /** create empty sketch with same hashing functions **/
    Sketch(const Sketch &sketch);

    virtual ~Sketch();

    /**
     * insert
     * inserts given value to all tables
     * @param src Value to be inserted
    */
    void insert(uint32_t src, uint32_t dst);

    /**
     * insert
     * inserts given value to all tables
     * @param src Value to be inserted
    */
    void insert_size(uint32_t src, uint32_t dst, uint32_t size);

    /**
     * compute_all_biv
     * initializes all possible BIVs and extends them in all possible ways. Should compute all reverse hashes
    */
    void compute_reverse_hashes();

    /**
     * print_reverse_hashes
     * if reverse hashes of heavy buckets were computed, prints them
    */
    void print_reverse_hashes();

    /**
     * compute_modular_potentials
     * saves modular potentials on given word index
     * @param index Word index of potentials
    */
    void compute_modular_potentials(unsigned word_index);

    /**
     * get_modular_potentials
     * @param index Word index of potentials
     * @return modular potentials on given word index
    */
    const std::map<uint8_t, HeavyBuckets> & get_modular_potentials(unsigned index) const;

    /**
     * get_buckets
     * @param index Index of given table
     * @return heavy buckets of given table
    */
    const std::vector<uint32_t> & get_buckets(unsigned index) const;

    /**
     * compute_buckets
     * in every hash table calls compute_buckets method
    */
    void compute_buckets();

    /** debug outputs **/
    void print_buckets();

    /** debug outputs **/
    void print_statistics();

    /**
     * init_biv
     * first step in creating bucket index vector. Creates BIV with length 1 and given word of index 0
    */
    BIV init_biv(uint8_t word);

    /**
     * init_table_stats
     * computes all heavy buckets and modular potentials
    */
    void init_table_stats();

    /**
     * extend_biv_recursive
     * extends given BIV by 1, intersecting its bucket maps for all modular potentials on given index.
     * Recursively calls itself with incremented index. Values from finished vectors are stored at reverse_hashes
    */
    void extend_biv_recursive(const BIV & biv, unsigned word_index);

//    Sketch operator+(const Sketch &another);
};

#endif //SKETCH_SKETCH_TABLE_H
