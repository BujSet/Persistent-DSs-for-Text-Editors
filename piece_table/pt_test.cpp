#include <iostream>
#include <fstream>
#include <chrono>
#include "piece_table.h"

using namespace std;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

void evaluate(PieceTable::PT *T, string file_path){
    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, std::milli> duration_sec;
    std::string item_name;
    std::ifstream nameFileout;

    nameFileout.open("input_eval.txt");
    string line;
    start = high_resolution_clock::now();
    while(std::getline(nameFileout, line))
    {
        PieceTable::insert(T, line);
        PieceTable::close(T, file_path + "_vol_test.txt");
    }
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
        cout << "usage: ./pt_test n" << endl;
        return 0;
    }

    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, std::milli> duration_sec;
    string file_path = "eg";

    PieceTable::PT *T = (PieceTable::PT *)malloc(sizeof(PieceTable::PT));
    PieceTable::open(T, file_path + ".txt");

    if(n == -1){
        cout<<"Piece table volatile version\nInsert metric evaluation mode\n";
        evaluate(T, file_path);
        exit(0);
    }

    start = high_resolution_clock::now();
    for (size_t i = 0; i < n; i++)
    {
        PieceTable::insert(T, "a");
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "insert time:" << duration_sec.count() << endl;

    start = high_resolution_clock::now();
    for (size_t i = 0; i < n; i++)
    {
        PieceTable::rewind(T);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "rewind time:" << duration_sec.count() << endl;

    start = high_resolution_clock::now();
    for (size_t i = 0; i < n; i++)
    {
        PieceTable::rewind(T);
        PieceTable::remove(T, 1);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "remove time:" << duration_sec.count() << endl;

    PieceTable::close(T, "hello.txt");
    free(T);
    return 0;
}
