//
// Created by medek on 12.4.17.
//

#include "SketchTable.h"

#include <random>

SketchTable::SketchTable(std::mt19937 & g) {
    table = std::array<uint32_t, MODULAR_MAX>();

    // generate permutation of table indexes
    std::array<uint8_t, INDEX_COUNT> indexes = std::array<uint8_t, INDEX_COUNT>();
    for (unsigned i = 0; i <= UINT8_MAX; ++i) {
        indexes[i] = static_cast<uint8_t>(i);
    }
    std::shuffle(indexes.begin(), indexes.end(), g);

    // create table for forward and reverse hashing
    values_map = std::array<uint8_t, INDEX_COUNT>(indexes);
    for(unsigned j = 0; j <= UINT8_MAX; j++) {
        values_map[j] /= 32;
        reverse_hashing_map[values_map[j]].push_back(static_cast<uint8_t>(j));
    }
}

SketchTable::SketchTable(const std::array<uint8_t, 256> & values_map,
                         const std::array<std::vector<uint8_t>, 8> & reverse_hashing_map) :
                                values_map(values_map),
                                reverse_hashing_map(reverse_hashing_map) {
    table = std::array<uint32_t, MODULAR_MAX>();
}



void SketchTable::insert_size(uint32_t src, uint32_t dst, uint32_t size = 1) {
    src = mangle(src);
    uint32_t result = hash_ip(src);
    dst = mangle(dst);
    result |= (hash_ip(dst) << 12);
    table[result] += size;
}

const std::vector<uint8_t> &SketchTable::get_reverse_by_word(uint32_t word, unsigned word_index) const {
    return reverse_hashing_map[get_word_of_hash(word, word_index)];
}

std::set<uint32_t>
SketchTable::in_what_heavy_buckets_is_this_word(uint8_t word, unsigned word_index, const std::vector<uint32_t> &buckets) {
    std::set<uint32_t> result;
    uint8_t word_hash = modular_hash(word);
    for (uint32_t bucket : buckets) {
        if (get_word_of_hash(bucket, word_index) == word_hash) {
            result.insert(bucket);
        }
    }
    return result;
}

void SketchTable::print_statistics() {
    int32_t max = 0;
    uint64_t sum = 0;
    for (uint64_t i = 0; i < MODULAR_MAX; i++) {
        sum += table[i];
        if (table[i] > max) {
            max = table[i];
        }
    }
    float avg = sum/table.size();
    float dispersion = 0;
    int32_t threshold = max/2;
    unsigned above = 0;
    for (uint32_t & val : table) {
        dispersion += abs(static_cast<int32_t>((avg - val)));
        if (val > threshold) {
            above++;
        }
    }
    dispersion /= table.size();

    std::cout << "heavy buckets: " << above << std::endl;
    std::cout << "TIME_MAX value: " << max << std::endl;
    std::cout << "avg value: " << avg << std::endl;
    std::cout << "dispersion: " << dispersion << std::endl;
}

void SketchTable::calculate_buckets() {
    if(!heavy_buckets.empty()) {
        return;
    }
//     value, index
    std::multimap<uint32_t, uint32_t> buckets;

    for (uint32_t i = 0; i < MODULAR_MAX; i++) {
        uint32_t table_val = table[i];
        if (table_val > 0) {
            if (buckets.size() < BUCKET_COUNT || table_val >= (*buckets.begin()).first) {
                buckets.insert({table_val, i});
            }
            if (buckets.size() > BUCKET_COUNT) {
                buckets.erase(buckets.begin());
            }
        }
    }
    for (auto x : buckets) {
        heavy_buckets.push_back(x.second);
    }
}

//void SketchTable::calculate_buckets() {
//    if(!heavy_buckets.empty()) {
//        return;
//    }
//    for (uint32_t i = 0; i < MODULAR_MAX; i++) {
//        if(table[i] > 2){
//            heavy_buckets.push_back(i);
//        }
//    }
//}

uint16_t SketchTable::hash_ip(uint32_t value) {
    uint16_t result = 0;
    for (unsigned i = 0; i < 4; ++i) {
        uint8_t word = modular_hash(get_word_of_val(value, i));
        result |= (word << (i * HASH_WORD_LEN));
    }
    return result;
}


uint8_t SketchTable::modular_hash(uint8_t value) const {
    return values_map[value];
}


const std::vector<uint32_t> &SketchTable::get_buckets() const {
    return heavy_buckets;
}

void SketchTable::print_table() {
    for (int i = 0; i < table.size(); ++i) {
        if(table[i] != 0) {
            std::cout << i << ": " << table[i] << std::endl;
        }
    }
}

//SketchTable * SketchTable::operator+(const SketchTable &another) {
//    auto * result = new SketchTable(this->values_map, this->reverse_hashing_map);
//    for (uint32_t i = 0; i < MODULAR_MAX; ++i) {
//        result->table[i] = table[i] + another.table[i];
//    }
//    return result;
//}
