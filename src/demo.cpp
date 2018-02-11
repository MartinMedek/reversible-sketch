//
// Created by medek on 7. 3. 2017.
//
#include <iostream>
#include "Sketch.h"
#include <fstream>

#include "SketchRange.h"

const int TIME_MIN = 1345077870;
const int TIME_MAX = 1345078469;
const int TIME_DIFF = TIME_MAX-TIME_MIN;

const int FRAME_SIZE = 10;
const int FRAME_COUNT = (TIME_MAX-TIME_MIN)/FRAME_SIZE+1;

typedef unsigned long ulong;

void demo_sketch(){
    clock_t begin = clock();
    auto * s = new Sketch();
    ulong counter = 0;
    std::ifstream ifs ("b2.txt", std::ios::in);

    for (std::string line; std::getline(ifs, line); ) {
        if (line.empty()) {
            continue;
        }
        ParsedData data = parse_csv_line(line);
        if(data.port != SSH_NUM){
            continue;
        }
        s->insert_size(data.src_ip, data.dst_ip, 1);
        counter++;
    }
//    s.insert(1, 1);

    std::cout << "flow_counter: " << counter << std::endl;

    s->compute_reverse_hashes();

    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "elapsed time: " << elapsed_secs << std::endl;
}

void demo_sketchrange(){

    clock_t begin = clock();
    SketchRange r = SketchRange(10, 3);

    std::string att ("125.131.162.227");
    uint32_t attacker = parse_ip(att);

    uint64_t counter = 0;

    std::ifstream ifs ("eeee3", std::ios::in);
    for (std::string line; std::getline(ifs, line); ) {
        ParsedData data = parse_csv_line(line);
        if(data.port != SSH_NUM || data.timestamp <= TIME_MIN+360 || data.timestamp >= TIME_MAX-60){
            continue;
        }
//        if(data.src_ip != attacker /**  && data.src_ip != attacker **/){
//            continue;
//        }
        counter++;
        r.insert(data);
    }
    for (int i = 0; i < TABLE_COUNT; ++i) {
        std::cout << "I: " << i << std::endl;
        r.image->tables[i]->print_table();
    }
    r.image->compute_reverse_hashes();
    std::cout << "counter: " << counter << std::endl;
}


int main()
{
//demo_sketchrange();
    demo_sketch();
}