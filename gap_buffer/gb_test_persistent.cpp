#include <iostream>
#include <fstream>
#include <chrono>

#include "gap_buffer_persistent.h"

namespace pobj = pmem::obj;

using namespace std;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

static void evaluate(pobj::pool<GapBuffer::root> pop, string file_name, int n){
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
        GapBuffer::insert(pop, line, count);
        end1 = high_resolution_clock::now();
        total_duration_sec += std::chrono::duration_cast<duration<double, std::milli>>(end1 - start1);
        count++;
    }    
    start2 = high_resolution_clock::now();
    GapBuffer::close(pop, file_name + "_pers_test.txt");
    end2 = high_resolution_clock::now();
    save_duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end2 - start2);

    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);

    cout << "Total insert and save time (in ms):" << duration_sec.count() << endl;
    cout << "Average character insert latency (in ms):" << total_duration_sec.count()/count << endl;
    cout << "Average Save to file latency (in ms):" << save_duration_sec.count()/count << endl;

    FILE *fpt;
    fpt = fopen("persistent_gb_word.csv", "a+");
    fprintf(fpt, "%d, %f, %f, %f\n", count, duration_sec.count(), total_duration_sec.count()/count, save_duration_sec.count()/count);
    fclose(fpt);
}

static void evaluate_typing_simul_1min(pobj::pool<GapBuffer::root> pop, string file_name, int n){
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
        GapBuffer::insert(pop, string(1, ch), count);
        end1 = high_resolution_clock::now();
        total_duration_sec += std::chrono::duration_cast<duration<double, std::milli>>(end1 - start1);
        count++;
    }
    start2 = high_resolution_clock::now();
    GapBuffer::close(pop, file_name + "_pers_test.txt");
    end2 = high_resolution_clock::now();
    save_duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end2 - start2);

    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);

    cout << "Total insert and save time (in ms):" << duration_sec.count() << endl;
    cout << "Average character insert latency (in ms):" << total_duration_sec.count()/count << endl;
    cout << "Average Save to file latency (in ms):" << save_duration_sec.count()/count << endl;

    FILE *fpt;
    fpt = fopen("persistent_gb_char.csv", "a+");
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
        cout << "usage: ./gb_test n" << endl;
        return 0;
    }

    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, std::milli> duration_sec;

    pobj::pool<GapBuffer::root> pop;
    string file_name, insert_str;

    file_name = "init_read";

    if (access((file_name + "_pers").c_str(), F_OK) != 0)
    {
        pop = pmem::obj::pool<GapBuffer::root>::create(file_name + "_pers", DEFAULT_LAYOUT, PMEMOBJ_MIN_POOL);
    }
    else
    {
        pop = pmem::obj::pool<GapBuffer::root>::open(file_name + "_pers", DEFAULT_LAYOUT);
    }

    auto r = pop.root();

    pobj::transaction::run(pop, [&]
                           { r->root_gap_buffer = pobj::make_persistent<GapBuffer::gap_buffer>(); });

    GapBuffer::create(pop, file_name + ".txt");

    if(n < 0){  
        cout<<endl<<"n = "<<(-1)*n<<endl;    
        cout<<"Gapbuffer persistent version\nInsert words evaluation mode\n";
        evaluate(pop, file_name, n);

        cout<<"Gapbuffer persistent version\nTyping simulation character wise evaluation mode\n";
        evaluate_typing_simul_1min(pop, file_name, n);
        exit(0);
    }

    start = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        start = high_resolution_clock::now();
        GapBuffer::insert(pop, "a", i);
        end = high_resolution_clock::now();
        duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
        cout << "iteration: " << i << " time: " << duration_sec.count() << " ms" << endl;
    }
    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "insert time:" << duration_sec.count() << " ms" << endl;

    start = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        start = high_resolution_clock::now();
        GapBuffer::deleteCharacter(pop, 0);
        end = high_resolution_clock::now();
        duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
        cout << "iteration: " << i << " time: " << duration_sec.count() << " ms" << endl;
    }
    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "remove time:" << duration_sec.count() << " ms" << endl;

    GapBuffer::close(pop, file_name + "_pers_test.txt");

    return 0;
}
