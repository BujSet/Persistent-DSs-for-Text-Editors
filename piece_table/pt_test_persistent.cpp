#include <iostream>
#include <fstream>
#include <chrono>

#include "piece_table_persistent.h"

namespace pobj = pmem::obj;

using namespace std;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

static void evaluate(pobj::pool<PieceTable::root> pop, string file_name, int n){
    high_resolution_clock::time_point start, start1, start2;
    high_resolution_clock::time_point end, end1, end2;
    duration<double, std::milli> duration_sec, save_duration_sec = std::chrono::milliseconds::zero(), total_duration_sec = std::chrono::milliseconds::zero();
    std::string item_name;
    std::ifstream nameFileout;
    string line;
    int count = 0;

    nameFileout.open("input_eval.txt");
    start = high_resolution_clock::now();
    while((count < (-1)*n) && (nameFileout >> line))
    {
        start1 = high_resolution_clock::now();
        PieceTable::insert(pop, line);
        end1 = high_resolution_clock::now();
        total_duration_sec += std::chrono::duration_cast<duration<double, std::milli>>(end1 - start1);
        count++;
    }    
    start2 = high_resolution_clock::now();
    PieceTable::close(pop, file_name + "_pers_test.txt");
    end2 = high_resolution_clock::now();
    save_duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end2 - start2);

    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);

    cout << "Total insert and save time (in ms):" << duration_sec.count() << endl;
    cout << "Average character insert latency (in ms):" << total_duration_sec.count()/count << endl;
    cout << "Average Save to file latency (in ms):" << save_duration_sec.count()/count << endl;

    FILE *fpt;
    fpt = fopen("persistent_pt_word.csv", "a+");
    fprintf(fpt, "%d, %f, %f, %f\n", count, duration_sec.count(), total_duration_sec.count()/count, save_duration_sec.count()/count);
    fclose(fpt);
}

static void evaluate_typing_simul_1min(pobj::pool<PieceTable::root> pop, string file_name, int n){
    high_resolution_clock::time_point start, start1, start2;
    high_resolution_clock::time_point end, end1, end2;
    duration<double, std::milli> duration_sec, save_duration_sec, total_duration_sec = std::chrono::milliseconds::zero();
    std::string item_name;
    std::ifstream nameFileout;
    char ch;
    int count = 0;
    string line;

    nameFileout.open("input_eval.txt");
    start = high_resolution_clock::now();
    while((count < (-1)*n) && (nameFileout >> noskipws >> ch)) {
        start1 = high_resolution_clock::now();
        PieceTable::insert(pop, string(1, ch));
        end1 = high_resolution_clock::now();
        total_duration_sec += std::chrono::duration_cast<duration<double, std::milli>>(end1 - start1);
        count++;
    }
    start2 = high_resolution_clock::now();
    PieceTable::close(pop, file_name + "_pers_test.txt");
    end2 = high_resolution_clock::now();
    save_duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end2 - start2);

    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    
    cout << "Total insert and save time (in ms):" << duration_sec.count() << endl;
    cout << "Average character insert latency (in ms):" << total_duration_sec.count()/count << endl;
    cout << "Average Save to file latency (in ms):" << save_duration_sec.count() << endl;

    FILE *fpt;
    fpt = fopen("persistent_pt_char.csv", "a+");
    fprintf(fpt, "%d, %f, %f, %f\n", count, duration_sec.count(), total_duration_sec.count()/count, save_duration_sec.count()/count);
    fclose(fpt);
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
        cout << "usage: ./pt_test_persistent n" << endl;
        return 0;
    }

    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, std::milli> duration_sec;

    pobj::pool<PieceTable::root> pop;
    string file_name, insert_str;

    file_name = "init_read";
    if (access((file_name + "_pool_test").c_str(), F_OK) != 0)
    {
        // do nothing
    }
    else
    {
        // Remove existing pool for new test
        string pool_name = string(file_name + string("_pool_test"));
        if(remove(pool_name.c_str()) != 0){
            cout<<"Unable to delete existing pool: "<<pool_name<<"\n";
            exit(0);
        }
    }

    pop = pmem::obj::pool<PieceTable::root>::create(file_name + "_pool_test", DEFAULT_LAYOUT, PMEMOBJ_MIN_POOL);	
    pobj::transaction::run(pop, [&]{
        (pop.root())->root_piece_table = pobj::make_persistent<PieceTable::piece_table>();				
    });

    PieceTable::create(pop, file_name + ".txt");

    if(n < 0){
        cout<<endl<<"n = "<<(-1)*n<<endl;
        cout<<"Piece table persistent version\nInsert words evaluation mode\n";
        evaluate(pop, file_name, n);

        cout<<"Piece table persistent version\nTyping simulation character wise evaluation mode\n";
        evaluate_typing_simul_1min(pop, file_name, n);
        exit(0);
    }

    start = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        PieceTable::insert(pop, "a");
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "insert time (in ms):" << duration_sec.count() << endl;

    start = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        PieceTable::rewind(pop);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "rewind time (in ms):" << duration_sec.count() << endl;

    start = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        PieceTable::rewind(pop);
        PieceTable::remove(pop, 1);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "remove time (in ms):" << duration_sec.count() << endl;

    PieceTable::close(pop, file_name + "_pers_test.txt");

    return 0;
}
