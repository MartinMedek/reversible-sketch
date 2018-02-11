//
// Created by medek on 7.4.17.
//


//#include <omp.h>
#include <random>
#include "Sketch.h"


Sketch::Sketch() {
    std::random_device rd;
    std::mt19937 g(rd());
    for (int i = 0; i < TABLE_COUNT; ++i) {
        tables[i] = new SketchTable(g);
    }
}

Sketch::Sketch(const Sketch & sketch) {
    for (unsigned i = 0; i < TABLE_COUNT; ++i) {
        tables[i] = new SketchTable(sketch.tables[i]->values_map,
                                    sketch.tables[i]->reverse_hashing_map);
    }
}

Sketch::~Sketch() {
    for (int i = 0; i < TABLE_COUNT; ++i) {
        delete tables[i];
    }
}

void Sketch::insert(uint32_t src, uint32_t dst) {
    for (int i = 0; i < TABLE_COUNT; ++i) {
        tables[i]->insert_size(src, dst, 1);
    }
}

void Sketch::insert_size(uint32_t src, uint32_t dst, uint32_t size) {
    for (int i = 0; i < TABLE_COUNT; ++i) {
        tables[i]->insert_size(src, dst, size);
    }
}

void Sketch::compute_modular_potentials(unsigned word_index)
{
    if (!modular_potentials[word_index].empty()) { // if modular potentials were calculated, do nothing
        return;
    }
    std::map<uint8_t, unsigned> potentials; //the value shows in how many tables has the value hashed to heavy bucket
    for (unsigned i = 0; i < TABLE_COUNT; i++) {
        std::set<uint8_t> images; // set of all numbers, that hash to any heavy bucket at table i
        auto buckets = get_buckets(i);
        for (const uint32_t & bucket : buckets) {
            // add to images all possible reverse hashes of given at given index word of bucket
            const std::vector<uint8_t> bucket_images = tables[i]->get_reverse_by_word(bucket, word_index);
            images.insert(bucket_images.begin(), bucket_images.end());
        }
        for (uint8_t word : images) {
            potentials[word]++;
            // if word hashes to bucket in enough tables, it is modular potential
            if (potentials[word] >= (TABLE_COUNT-R)) {
                HeavyBuckets to_insert;
                for (unsigned j = 0; j < TABLE_COUNT; ++j) {
                    to_insert[j] = tables[j]->in_what_heavy_buckets_is_this_word(word, word_index, get_buckets(j));
                }
                modular_potentials[word_index].insert({word,to_insert});
            }
        }
    }
}

const std::map<uint8_t, HeavyBuckets> & Sketch::get_modular_potentials(unsigned index) const {
    return modular_potentials[index];
}

const std::vector<uint32_t> &Sketch::get_buckets(unsigned index) const {
    return tables[index]->get_buckets();
}

void Sketch::print_buckets() {
    for (unsigned i = 0; i < TABLE_COUNT; ++i) {
        std::cout << "TABLE " << i << ":" << std::endl;
        get_buckets(i);
        std::cout << std::endl;
    }
}

void Sketch::print_statistics() {
    for (int i = 0; i < TABLE_COUNT; ++i) {
        std::cout << "TABLE " << i << ":" << std::endl;
        tables[i]->print_statistics();
        std::cout << std::endl;
    }
}

void Sketch::print_reverse_hashes() {
    if(reversed_hashes.empty()){
        std::cout << "no reverse hashes found" << std::endl;
    }
    std::cout << "reverse hashes count: " << reversed_hashes.size() << std::endl;
    std::cout << "reverse hashes: " << std::endl;
    for (auto val : reversed_hashes) {
        uint32_t src = static_cast<uint32_t>(val & SRC_MASK);
        uint32_t dst = static_cast<uint32_t>((val & DST_MASK) >> 32);
        src = mangle_reverse(src);
        dst = mangle_reverse(dst);
        std::cout << "src: " << ip_to_str(src) << " dst: " << ip_to_str(dst) << std::endl;
    }

}

void Sketch::compute_buckets() {
    for (unsigned i = 0; i <TABLE_COUNT; ++i) {
        tables[i]->calculate_buckets();
    }
}

void Sketch::init_table_stats() {
    compute_buckets();
    for (unsigned i = 0; i < WORD_COUNT; ++i) {
        compute_modular_potentials(i);
    }
}

BIV Sketch::init_biv(uint8_t word) {
    BIV biv;
    for (unsigned i = 0; i < TABLE_COUNT; ++i) {
        biv.set_buckets_at(i, modular_potentials[0][word][i]);
    }
    biv.set_word_at(0, word);
    return biv;
}

void Sketch::extend_biv_recursive(const BIV & biv, unsigned word_index) {
    // check if vector is complete
    if (word_index == WORD_COUNT) {
        reversed_hashes.push_back(biv.get_image_from_vector());
        std::cout << "found reverse hash: " << reversed_hashes.size() << std::endl;
        return;
    }
    // function extends starting at word_index 1
    if (word_index < 1) {
        std::cout << "invalid word_index" << word_index << std::endl;
        return;
    }

    // extend for every modular potential
    for (auto potential : modular_potentials[word_index]) {
        BIV extended = BIV(biv);
        extended.set_word_at(word_index, potential.first);

        // intersect bucket maps
        unsigned counter = 0;
        for (unsigned table = 0; table < TABLE_COUNT; ++table) {
            std::set<uint32_t> intersection;
            std::set<uint32_t> & new_buckets = potential.second[table];
            std::set_intersection(biv.get_buckets_at(table).begin(), biv.get_buckets_at(table).end(),
                                  new_buckets.begin(), new_buckets.end(),
                                  std::inserter(intersection, intersection.begin()));
            if(intersection.empty()){
                std::cout << "end\n";
                counter++;
                // if bucketMap is empty in too many tables, vector doesn't lead to reverse hash
                if(counter>R){
                    break;
                }
            }
            extended.set_buckets_at(table, intersection);
        }
        // continue only if bucketMap is full enough (TABLE_COUNT-R tables filled)
        if (counter<=R) {
            extend_biv_recursive(extended, word_index+1);
        }
    }
}

void Sketch::compute_reverse_hashes() {
    // compute heavy buckets and modular potentials
    init_table_stats();
    // compute all BIVs with length 1 and recursively extend them
    for (auto x : modular_potentials[0]) {
        std::cout << "new modular potential\n";
        extend_biv_recursive(init_biv(x.first), 1);
    }
    print_reverse_hashes();
}

//Sketch Sketch::operator+(const Sketch &another) {
//    auto result = Sketch(another);
//    #pragma  omp parallel num_threads(TABLE_COUNT)
//    {
//        int i = omp_get_thread_num();
//        result.tables[i] = *tables[i] + *another.tables[i];
//    }
//    return result;
//}




