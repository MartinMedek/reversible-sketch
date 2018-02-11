//
// Created by medek on 7.3.17.
//

#ifndef MISC_FUNCTIONS_H
#define MISC_FUNCTIONS_H

#include <cstring>
#include <set>
#include <iostream>
#include <list>
#include <regex>
#include <stdint.h>
#include <array>
#include "helpers.h"

/** constants and masks **/
const uint64_t SRC_MASK = 0xffffffff;
const uint64_t DST_MASK = 0xffffffff00000000;
const uint64_t SRC_HASH_MASK = 0xfff;
const uint64_t DST_HASH_MASK = 0xfff000;
const uint32_t EVEN_MASK = 0xaaaaaaaa; /** used for mangling **/
const uint32_t ODD_MASK = 0x55555555; /** used for mangling **/
static const unsigned WORD_COUNT = 8; /** number of modular hashed words hash is build of **/
const uint32_t HASH_WORD_MASK[WORD_COUNT] =
        {0x7, 0x38, 0x1c0, 0xe00, 0x7000, 0x38000, 0x1c0000, 0xe00000}; /** used for getting words of hashes **/
const uint32_t BYTE_MASK[4] = {0xff, 0xff00, 0xff0000, 0xff000000}; /** used for getting words from ips **/
const uint32_t INDEX_COUNT = UINT8_MAX+1;
const unsigned HASH_WORD_LEN = 3;
static const uint32_t MODULAR_MAX = 16777216; /** size of SketchTable **/

const uint16_t SSH_NUM = 22;
const uint16_t TELNET_NUM = 23;
const uint16_t HTTP_NUM = 80;
const uint16_t HTTPS_NUM = 443;
const uint16_t RDP_NUM = 3389;

/** these can be changed **/
const unsigned R = 0; // maximal tolerance when building modular potentials and bucket index vectors
static const unsigned TABLE_COUNT = 5; // count of tables in sketch
static const unsigned BUCKET_COUNT = 1; // maximal count of buckets

struct ParsedData{
    uint32_t src_ip;
    uint32_t dst_ip;
    uint32_t timestamp;
    uint16_t port;
    uint32_t data_size;
};

typedef std::array<std::set<uint32_t>, TABLE_COUNT> HeavyBuckets;


/**
* mangle
* shuffle bit order to prevent clustering of ip addresses
*/
inline uint32_t mangle(uint32_t value)
{
    uint32_t evens = rotate_left(value & EVEN_MASK, 4);
    uint32_t odds = rotate_right(value & ODD_MASK, 12);
    return odds | evens;
}

/**
* mangle_reverse
* reverse the mangling process
*/
inline uint32_t mangle_reverse(uint32_t value)
{
    uint32_t evens = rotate_right(value & EVEN_MASK, 4);
    uint32_t odds = rotate_left(value & ODD_MASK, 12);
    return odds | evens;
}


/**
 * overload with char * input
 */
inline uint32_t parse_ip(char * input)
{
    uint32_t ip = 0;
    char * curr = strtok(input, ".");
    for (int i = 0; i < 4; ++i) {
        ip |= static_cast<uint32_t>(fast_atoi<uint8_t>(curr))<<((3 - i) * 8);
        curr = strtok(nullptr, ".");
    }
    return ip;
}
/**
* parse_ip
* parse ip from string containing only ip address
*/
inline uint32_t parse_ip(std::string & input)
{
    return parse_ip(const_cast<char *>(input.c_str()));
}

/**
 * parse_csv_line
 * parse src ip, dst port, data size and timestamp of flow
 *
 * nfdump -r <filename> -q -o csv "inet"
 * ts,te,td,sa,da,sp,dp,pr,flg,fwd,stos,ipkt,ibyt,opkt,obyt,in,out,sas,das,smk,dmk,dtos,dir,nh,nhb,svln,dvln,ismc,odmc,idmc,osmc,mpls1,mpls2,mpls3,mpls4,mpls5,mpls6,mpls7,mpls8,mpls9,mpls10,cl,sl,al,ra,eng,exid,tr
*/
inline ParsedData parse_csv_line(std::string &input)
{
    ParsedData result{};
    uint32_t ip = 0;
    result.timestamp = fast_atoi<uint32_t>(strtok(const_cast<char *>(input.c_str()), ","));

    strtok(nullptr, ","); // skip time end
    strtok(nullptr, ","); // skip time duration

    char * src_ip = strtok(nullptr, ",");
    char * dst_ip = strtok(nullptr, ","); // skip dst src_ip
    strtok(nullptr, ","); // skip src port
    result.port = fast_atoi<uint16_t>(strtok(nullptr, ",")); // read dst port

    strtok(nullptr, ","); // skip stuff
    strtok(nullptr, ","); // skip
    strtok(nullptr, ","); // skip
    strtok(nullptr, ","); // skip

    auto ipkt = fast_atoi<uint32_t>(strtok(nullptr, ",")); // incoming packets
    auto ibyt = fast_atoi<uint32_t>(strtok(nullptr, ",")); // incoming bytes
    auto opkt = fast_atoi<uint32_t>(strtok(nullptr, ",")); // outgoing packets
    auto obyt = fast_atoi<uint32_t>(strtok(nullptr, ",")); // outgoing bytes

    result.data_size = ibyt + obyt; // read data size
    result.dst_ip = parse_ip(dst_ip);
    result.src_ip = parse_ip(src_ip);
    return result;
}

/**
 * precompute_frames
 * function for creating time frame map.
 */
inline std::map<int,int> precompute_frames(int min, int max, int frames){
    int dif = max - min;
    std::map<int,int> result;
    for (int i = 0; i < frames; ++i) {
        min += dif/frames;
        result.insert({min,i});
    }
    return result;
}

/**
 * time_to_frame_index
 * chooses, to which time frame of frame map given time maps
 */
inline int time_to_frame_index(std::map<int,int> & frames, int time){
    return (*frames.upper_bound(time)).second;
}

/**
 * get_word_of_hash
 * returns ith word of hash (word of hash has 3 bits)
*/
inline uint8_t get_word_of_hash(uint32_t value, unsigned word_index)
{
    return static_cast<uint8_t>(
            (value & HASH_WORD_MASK[word_index]) >> (word_index * HASH_WORD_LEN));
}

/**
 * get_word_of_value
 * returns ith word (word has length 8 bits) of value
*/
inline uint8_t get_word_of_val(uint32_t value, unsigned word_index)
{
    if(word_index >= 4){
        std::cout << "yay\n";
        return 0;
    }
    uint32_t res = value & BYTE_MASK[word_index];
    res = res >> (word_index * 8);
    return static_cast<uint8_t>(res);
}

inline std::string ip_to_str(uint32_t val){
    std::string result;
    for (unsigned i = 0; i < 3; ++i) {
        (result += std::to_string(get_word_of_val(val, 3-i))) += '.';
    }
    result += std::to_string(get_word_of_val(val, 0));
    return result;
}


#endif //MISC_FUNCTIONS_H

