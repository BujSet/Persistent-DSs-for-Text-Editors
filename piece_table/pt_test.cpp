#include <iostream>
#include <fstream>
#include <chrono>
#include "piece_table.h"

using namespace std;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

static void evaluate(PieceTable::PT *T, string file_path){
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
    cout << "Insert and save time (in ms):" << duration_sec.count() << endl;
}

static void evaluate_typing_simul_1min(PieceTable::PT *T, string file_path){
    high_resolution_clock::time_point start, start1, start2;
    high_resolution_clock::time_point end, end1, end2;
    duration<double, std::milli> duration_sec, save_duration_sec = std::chrono::milliseconds::zero(), total_duration_sec = std::chrono::milliseconds::zero();
    std::string item_name;
    std::ifstream nameFileout;
    char ch;
    int count = 0;
    string line;

    nameFileout.open("input_eval.txt");
    start = high_resolution_clock::now();
    while((count < 480) && (nameFileout >> noskipws >> ch)) {
        start1 = high_resolution_clock::now();
        PieceTable::insert(T, string(1, ch));
        end1 = high_resolution_clock::now();
        total_duration_sec += std::chrono::duration_cast<duration<double, std::milli>>(end1 - start1);

        start2 = high_resolution_clock::now();
        PieceTable::close(T, file_path + "_vol_test.txt");
        end2 = high_resolution_clock::now();
        save_duration_sec += std::chrono::duration_cast<duration<double, std::milli>>(end2 - start2);
        // cout << "Char insert and save latency (in ms):" << duration_sec.count() << endl;
        count++;
    }
    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    
    cout << "Insert and save time (in ms):" << duration_sec.count() << endl;
    cout << "Average character insert latency (in ms):" << duration_sec.count()/count << endl;
    cout << "Average Save to file latency (in ms):" << save_duration_sec.count() << endl;
}

int main(int argc, char *argv[])
{
    int n = 1;

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
    string file_path = "init_read";

    PieceTable::PT *T = (PieceTable::PT *)malloc(sizeof(PieceTable::PT));
    PieceTable::open(T, file_path + ".txt");

    if(n == -1){
        cout<<"Piece table volatile version\nInsert metric evaluation mode\n";
        evaluate(T, file_path);
        exit(0);
    }
    else if(n == -2){
        cout<<"Piece table volatile version\nCharlevel Insert metric evaluation mode\n";
        evaluate_typing_simul_1min(T, file_path);
        exit(0);
    }

    start = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        PieceTable::insert(T, "a");
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "insert time (in ms):" << duration_sec.count() << endl;

    start = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        PieceTable::rewind(T);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "rewind time (in ms):" << duration_sec.count() << endl;

    start = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        PieceTable::rewind(T);
        PieceTable::remove(T, 1);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "remove time (in ms):" << duration_sec.count() << endl;

    PieceTable::close(T, file_path + "_vol_test.txt");
    free(T);
    return 0;
}
