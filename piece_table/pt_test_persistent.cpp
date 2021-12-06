#include <iostream>
#include <chrono>
#include "piece_table_persistent.h"

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
        cout << "usage: ./pt_test_persistent n" << endl;
        return 0;
    }

    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, std::milli> duration_sec;

    pobj::pool<PieceTable::root> pop;
    int ip, len_str, offset;
    string file_path, insert_str, out_path;

    // cout<<"Enter file name: ";
    // cin>>file_path;
    file_path = "eg.txt";
    out_path = "egout.txt";
    if (access((file_path + "_pers").c_str(), F_OK) != 0)
    {
        pop = pmem::obj::pool<PieceTable::root>::create(file_path + "_pers", DEFAULT_LAYOUT, PMEMOBJ_MIN_POOL);
    }
    else
    {
        pop = pmem::obj::pool<PieceTable::root>::open(file_path + "_pers", DEFAULT_LAYOUT);
    }

    pobj::transaction::run(pop, [&]
                           { (pop.root())->root_piece_table = pobj::make_persistent<PieceTable::piece_table>(); });

    PieceTable::create(pop, file_path);

    start = high_resolution_clock::now();
    for (size_t i = 0; i < n; i++)
    {
        PieceTable::insert(pop, "a");
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "insert time:" << duration_sec.count() << endl;

    start = high_resolution_clock::now();
    for (size_t i = 0; i < n; i++)
    {
        PieceTable::rewind(pop);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "rewind time:" << duration_sec.count() << endl;

    start = high_resolution_clock::now();
    for (size_t i = 0; i < n; i++)
    {
        PieceTable::rewind(pop);
        PieceTable::remove(pop, 1);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "remove time:" << duration_sec.count() << endl;

    PieceTable::close(pop, file_path);

    return 0;
}
