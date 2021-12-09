#include <iostream>
#include <fstream>
#include <chrono>
#include "piece_table_persistent.h"

namespace pobj = pmem::obj;

using namespace std;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

void evaluate(pobj::pool<PieceTable::root> pop, string file_path){
    high_resolution_clock::time_point start, start1;
    high_resolution_clock::time_point end, end1;
    duration<double, std::milli> duration_sec;
    std::string item_name;
    std::ifstream nameFileout;

    nameFileout.open("input_eval.txt");
    string line;
    start = high_resolution_clock::now();
    // while(std::getline(nameFileout, line))
    while(nameFileout >> line)
    {
        // start1 = high_resolution_clock::now();
        PieceTable::insert(pop, line);
        // end1 = high_resolution_clock::now();
        // duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end1 - start1);
        // cout << "1 word insert time:" << duration_sec.count() << endl;
    }    
    PieceTable::close(pop, file_path + "_pers_test.txt");
    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "insert time:" << duration_sec.count() << endl;
}

void evaluate_charlevel(pobj::pool<PieceTable::root> pop, string file_path){
    high_resolution_clock::time_point start, start1;
    high_resolution_clock::time_point end, end1;
    duration<double, std::milli> duration_sec;
    std::string item_name;
    std::ifstream nameFileout;
    char ch;

    nameFileout.open("input_eval.txt");
    string line;
    start = high_resolution_clock::now();
    while(nameFileout >> noskipws >> ch) {
        start1 = high_resolution_clock::now();
        PieceTable::insert(pop, string(1, ch));
        end = high_resolution_clock::now();     
    }
    PieceTable::close(pop, file_path + "_pers_test.txt");
    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "insert time:" << duration_sec.count() << endl;
}

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
    file_path = "eg";
    out_path = "egout.txt";
    if (access((file_path + "_pool_test").c_str(), F_OK) != 0)
    {
        pop = pmem::obj::pool<PieceTable::root>::create(file_path + "_pool_test", DEFAULT_LAYOUT, PMEMOBJ_MIN_POOL);
    }
    else
    {
        pop = pmem::obj::pool<PieceTable::root>::open(file_path + "_pool_test", DEFAULT_LAYOUT);
    }

    pobj::transaction::run(pop, [&]
                           { (pop.root())->root_piece_table = pobj::make_persistent<PieceTable::piece_table>(); });

    PieceTable::create(pop, file_path + "_test.txt");

    if(n == -1){
        cout<<"Piece table persistent version\nInsert metric evaluation mode\n";
        evaluate(pop, file_path);
        exit(0);
    }
    else if(n == -2){
        cout<<"Piece table persistent version\nCharlevel Insert metric evaluation mode\n";
        evaluate_charlevel(pop, file_path);
        exit(0);
    }

    start = high_resolution_clock::now();
    for (size_t i = 0; i < n; i++)
    {
        start = high_resolution_clock::now();
        PieceTable::insert(pop, "a");
        end = high_resolution_clock::now();
        duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
        cout << duration_sec.count() << endl;
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

    PieceTable::close(pop, file_path + "_pers_test.txt");

    return 0;
}
