#include <iostream>
#include <chrono>
#include "gap_buffer_persistent.h"

namespace pobj = pmem::obj;

using namespace std;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

int main(int argc, char *argv[])
{
    size_t n = 1;

    if (argc == 2)
    {
        n = atoi(argv[1]);
    }
    else
    {
        cout << "usage: ./gb_test n" << endl;
        return 0;
    }

    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, std::milli> duration_sec;

    pobj::pool<GapBuffer::root> pop;
    int input, len_str, offset;
    string file_path, insert_str, out_path;

    file_path = "gap_buffer_in.txt";
    out_path = "gap_buffer_out.txt";

    if (access((file_path + "_pers").c_str(), F_OK) != 0)
    {
        pop = pmem::obj::pool<GapBuffer::root>::create(file_path + "_pers", DEFAULT_LAYOUT, PMEMOBJ_MIN_POOL);
    }
    else
    {
        pop = pmem::obj::pool<GapBuffer::root>::open(file_path + "_pers", DEFAULT_LAYOUT);
    }

    auto r = pop.root();

    pobj::transaction::run(pop, [&]
                           { r->root_gap_buffer = pobj::make_persistent<GapBuffer::gap_buffer>(); });

    GapBuffer::create(pop, file_path);

    start = high_resolution_clock::now();
    for (size_t i = 0; i < n; i++)
    {
        GapBuffer::insert(pop, "a", 0);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "insert time:" << duration_sec.count() << endl;

    start = high_resolution_clock::now();
    for (size_t i = 0; i < n; i++)
    {
        GapBuffer::deleteCharacter(pop, 0);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "remove time:" << duration_sec.count() << endl;

    GapBuffer::close(pop, file_path);

    return 0;
}
