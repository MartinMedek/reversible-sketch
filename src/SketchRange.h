//
// Created by med on 27.4.17.
//

#ifndef SKETCH_SKETCHRANGE_H
#define SKETCH_SKETCHRANGE_H

#include <array>
#include <queue>
#include "Sketch.h"
#include "misc_functions.h"
const uint32_t DIF_THRESHOLD = 500;


class SketchRange {
public:
    Sketch * image;
    std::list<Sketch> sketchMap;
    std::array<std::map<uint32_t, int>, TABLE_COUNT> potentials;
    std::array<std::array<uint32_t, MODULAR_MAX>, TABLE_COUNT> * floating_sums;
    uint64_t frame_size;
    uint64_t max_time;
    int avg_window;
    int flow_counter;
    int sketch_counter;

    SketchRange(uint64_t frame_size, int avg_window) : max_time(0),
                                                       flow_counter(0),
                                                       sketch_counter(0),
                                                       frame_size(frame_size),
                                                       avg_window(avg_window)
    {
        image = new Sketch();
        floating_sums = new std::array<std::array<uint32_t, MODULAR_MAX>, TABLE_COUNT>();
    }

    void insert(ParsedData & data)
    {
        // if sketchMap is empty, initialize it
        if (sketchMap.empty()) {
            add_sketch();
            max_time = data.timestamp+frame_size;
        }
        else { // if there is too much sketches, compute averages and delete last element.
            while (data.timestamp >= max_time) {
                if(sketchMap.size() > avg_window){
                    step_forward();
                }
                std::cout << "printing avgs\n";
                for (unsigned long i = 0; i < TABLE_COUNT; ++i) {
                    std::cout << i << ": ";
                    print_zero_nonzero_count(floating_sums->at(i));
                }
                add_sketch();
                std::cout << "flowcounter: " << flow_counter << std::endl;
                flow_counter = 0;
            }
        }
        sketchMap.front().insert_size(data.src_ip, data.dst_ip, data.data_size);
        flow_counter++;
    }

    void print_front_sketch(){
        std::cout << "printing front sketch\n";
        for (unsigned long i = 0; i < TABLE_COUNT; ++i) {
            std::cout << i << ":\n";
            print_zero_nonzero_count(sketchMap.front().tables[i]->table);
        }
    }

    void add_sketch()
    {
        sketchMap.emplace_front(*image);
        sketch_counter++;
        max_time += frame_size;

    }

    void init_float_avg(unsigned table_index, uint32_t index)
    {
        // remove this when not debugging
        if((*floating_sums)[table_index][index] != 0){
            std::cout << "floating sum not 0 when initializing\n";
            exit(1);
        }

        auto current_sketch = sketchMap.rbegin();
        for (int i = 0; i < avg_window; ++i) {
            (*floating_sums)[table_index][index] += current_sketch->tables[table_index]->table[index];
            current_sketch++;
        }
    }

    void extend_float_avg(unsigned table_index, uint32_t index)
    {
        // remove last element from window
        (*floating_sums)[table_index][index] -=
                sketchMap.rbegin()->tables[table_index]->table[index];
        // push element to window
        (*floating_sums)[table_index][index] +=
                (*++sketchMap.begin()).tables[table_index]->table[index];
    }

    int get_diff(uint32_t one, uint32_t two)
    {
        return abs(static_cast<int>(one) - static_cast<int>(two));
    }

    void detect_const(unsigned table_index, uint32_t index)
    {
        uint32_t avg = (*floating_sums)[table_index][index] / avg_window;
        uint32_t last_val = sketchMap.front().tables[table_index]->table[index];
        int diff = get_diff(avg, last_val);
//        if(new_val > 0 && (*floating_sums)[table_index][index]){
//            std::cout << "NEW VAL: " << new_val << " AVG: " << (*floating_sums)[table_index][index] << std::endl;
//        }

        if(diff <= avg && last_val > 1000) {
            potentials[table_index][index]++;
            std::cout << "NEW VAL: " << last_val << " AVG: " << (*floating_sums)[table_index][index]/avg_window << std::endl;
//            std::cout << "const detected: " << last_val << "\n";
            image->tables[table_index]->table[index]++;
        }
    }

    void step_forward()
    {
        std::cout << "---------------------starting step forward---------------------------\n";
        if (sketch_counter <= avg_window) {
            std::cout << "stepping forward when sketch_counter is not bigger than avg window\n";
            return;
        }
        clock_t begin = clock();
        for (uint32_t tab_index = 0; tab_index < TABLE_COUNT; ++tab_index) {
            for (unsigned index = 0; index < MODULAR_MAX; ++index) {
                if (sketch_counter == avg_window+1) {
                    init_float_avg(tab_index, index);
                    detect_const(tab_index, index);
                }
                else {
                    extend_float_avg(tab_index, index);
                    detect_const(tab_index, index);
                }
            }
        }
        sketchMap.pop_back();


        clock_t end = clock();
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
        std::cout << "elapsed time: " << elapsed_secs << std::endl;
    }

    virtual ~SketchRange()
    {
        delete(image);
        delete(floating_sums);
    }
};


#endif //SKETCH_SKETCHRANGE_H
