//
// Created by med on 21. 3. 2017.
//
#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "SketchTable.h"
#include "BIV.h"
#include "Sketch.h"

static std::random_device rd;
static std::mt19937 sample_mt (rd());

TEST_CASE("Test table constructor"){
    auto * t = new SketchTable(sample_mt);
    for (unsigned i = 0; i < 256; ++i) {
        auto rhm = t->reverse_hashing_map[t->values_map[i]];
        REQUIRE(std::find(rhm.begin(), rhm.end(), i) != rhm.end());
    }
    for (unsigned i = 0; i < 8; ++i) {
        auto rhm = t->reverse_hashing_map[i];
        for (int j = 0; j < 32; ++j) {
            uint8_t num = rhm[j];

            for (int k = 0; k < 8; ++k) {
                if(i == k) {
                    continue;
                }
                auto x = t->reverse_hashing_map[k];
                REQUIRE(std::find(x.begin(), x.end(), num) == x.end());
            }
        }
    }
}

TEST_CASE("Get word of hash"){
    std::bitset<16> x ("0000111010001110");
    uint16_t num = (uint16_t) x.to_ulong();
    REQUIRE(get_word_of_hash(num, 0) == 6);
    REQUIRE(get_word_of_hash(num, 1) == 1);
    REQUIRE(get_word_of_hash(num, 2) == 2);
    REQUIRE(get_word_of_hash(num, 3) == 7);
}

TEST_CASE("biv constructor"){
    BIV biv = BIV();
    for (int i = 0; i < WORD_COUNT; ++i) {
        REQUIRE(biv.get_word_at(i) == 0);
    }
    for (int j = 0; j < TABLE_COUNT; ++j) {
        REQUIRE(biv.get_buckets_at(j).empty());
    }
}

TEST_CASE("modular hash"){
    auto * t = new SketchTable(sample_mt);
    for (int i = 0; i < 265; ++i) {
        auto val = static_cast<uint8_t>(i);
        REQUIRE(t->modular_hash(val) == t->values_map[val]);
    }
}

TEST_CASE("hash ip"){
    auto * t = new SketchTable(sample_mt);
    std::string strIp("1.2.3.255");
    uint32_t ip =  parse_ip(strIp);

    uint16_t hash = t->hash_ip(ip);
    for (uint32_t i = 0; i < 4; ++i) {
        CHECK(get_word_of_hash(hash, i) == t->modular_hash(get_word_of_val(ip, i)));
    }
}


TEST_CASE("mangle"){
    auto * t = new SketchTable(sample_mt);
    std::string strIp("1.2.3.255");
    uint32_t ip =  parse_ip(strIp);

    REQUIRE(mangle(ip) != ip);
    REQUIRE(mangle_reverse(mangle(ip)) == ip);
}

void test_insert(uint32_t src, uint32_t dst) {
    auto * t = new SketchTable(sample_mt);

    t->insert_size(src, dst, 2);

    uint32_t hash = 0;
    int counter = 0;
    for (unsigned i = 0; i < MODULAR_MAX; ++i) {
        if (t->table[i] == 2) {
            hash = i;
            counter++;
        }
    }
    REQUIRE(counter == 1);

    hash &= 0xfff;
    uint32_t mangled = mangle(src);
    REQUIRE(hash == t->hash_ip(mangled));

    uint8_t words[4];
    for (uint8_t j = 0; j < 4; ++j) {
        words[j] = get_word_of_val(mangled, j);
        CHECK(get_word_of_hash(hash, j) == t->modular_hash(words[j]));

        auto x = t->get_reverse_by_word(hash, j);
        CHECK(std::find(x.begin(), x.end(), words[j]) != x.end());
    }
}


TEST_CASE("insert"){
    std::string ip ("1.2.3.125");
    uint32_t ip_num = parse_ip(ip);
    test_insert(ip_num, ip_num * 3);
}

TEST_CASE("sketch_constructor"){
    auto * sketch = new Sketch();
    for (int i = 0; i < 3; ++i) {
        REQUIRE(sketch->tables[i]->values_map != sketch->tables[i+1]->values_map);
    }
}
