#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

using namespace std;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

/**
 * This inspiration for this class has been taken from the
 * concepts and the code that has been described here:
 * https://www.geeksforgeeks.org/gap-buffer-data-structure/
 * All the functionalities mentioned here have been implemented.
 */

class GapBuffer
{

public:
    vector<char> buffer;
    int gapSize = 10;
    int gapLeft = 0;
    int gapRight = gapSize - gapLeft - 1;
    int size = 10;

    GapBuffer()
    {
        gapSize = 10;
        gapLeft = 0;
        gapRight = gapSize - gapLeft - 1;
        size = 10;

        for (int i = 0; i < size; i++)
        {
            buffer.push_back('_');
        }
    }

    /**
     *  This function moves is used to grow the gap
     *  at index position and return the vector
     */
    void grow(int k, int position)
    {

        // char copy[size];
        vector<char> copy(size);

        // The characters of the buffer after 'position'
        // are copied to the copy array
        for (int i = position; i < size; i++)
        {
            copy[i - position] = buffer[i];
        }

        // A gap of 'k' is inserted from the 'position' index
        // The gap is represented by '_'
        for (int i = 0; i < k; i++)
        {
            buffer.insert(buffer.begin() + i + position, '_');
        }

        // The remaining array is inserted
        for (int i = 0; i < k + position; i++)
        {
            buffer.insert(buffer.begin() + position + i + k, copy[i]);
        }

        size += k;
        gapRight += k;
    }

    /**
     *  This function moves the gap to the left, in the vector
     */
    void left(int position)
    {
        // Moves the gap left, character by character
        while (position < gapLeft)
        {
            gapLeft--;
            gapRight--;
            buffer.at(gapRight + 1) = buffer[gapLeft];
            buffer.at(gapLeft) = '_';
        }
    }

    /**
     *  This function moves the gap to the right, in the vector
     */
    void right(int position)
    {
        // Moves the gap right, character by character
        while (position > gapLeft)
        {
            gapLeft++;
            gapRight++;
            buffer.at(gapLeft - 1) = buffer[gapRight];
            buffer.at(gapRight) = '_';
        }
    }

    /**
     *  This function controls the movement of the gap
     *  by checking its position to the point of insertion
     */
    void moveCursor(int position)
    {
        if (position < gapLeft)
        {
            left(position);
        }
        else
        {
            right(position);
        }
    }

    /**
     *  This function inserts the 'input' string to the
     *  buffer at the point 'position'
     */
    void insert(string input, int position)
    {

        int i = 0, len = input.length();

        if (position != gapLeft)
        {
            moveCursor(position);
        }

        while (i < len)
        {

            // If the gap is empty, we need to grow the gap
            if (gapRight == gapLeft)
            {
                int k = 10;
                grow(k, position);
            }

            // Insert the character in the gap and move the gap
            buffer.at(gapLeft) = input[i];
            gapLeft++;
            i++;
            position++;
        }
    }

    /**
     *  This function deletes the character at the 'position' index
     */
    void deleteCharacter(int position)
    {

        if (gapLeft != position + 1)
        {
            moveCursor(position + 1);
        }
        gapLeft--;
        buffer.at(gapLeft) = '_';
    }

    void close(string file_path) {
        std::string text (buffer.begin(), buffer.end());
        ofstream out_file;
        out_file.open(file_path);
        // Write the string to the file.
        out_file << text;
        out_file.close();
    }
};

static void evaluate(GapBuffer gapBuffer, string file_name, int n){
    high_resolution_clock::time_point start, start1, start2;
    high_resolution_clock::time_point end, end1, end2;
    duration<double, std::milli> duration_sec, save_duration_sec, total_duration_sec = std::chrono::milliseconds::zero();
    std::string item_name;
    std::ifstream nameFileout;
    string line;
    int count = 0;

    nameFileout.open("input_eval.txt");
    start = high_resolution_clock::now();
    while((count < (-1)*n) && (nameFileout >> line))
    {
        start1 = high_resolution_clock::now();
        gapBuffer.insert(line, count);
        end1 = high_resolution_clock::now();
        total_duration_sec += std::chrono::duration_cast<duration<double, std::milli>>(end1 - start1);

        start2 = high_resolution_clock::now();
        gapBuffer.close(file_name + "_vol_test.txt");
        end2 = high_resolution_clock::now();
        save_duration_sec += std::chrono::duration_cast<duration<double, std::milli>>(end2 - start2);
        count++;
    }    
    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);

    cout << "Total insert and save time (in ms):" << duration_sec.count() << endl;
    cout << "Average character insert latency (in ms):" << total_duration_sec.count()/count << endl;
    cout << "Average Save to file latency (in ms):" << save_duration_sec.count()/count << endl;

    FILE *fpt;
    fpt = fopen("volatile_gb_word.csv", "a+");
    fprintf(fpt, "%d, %f, %f, %f\n", count, duration_sec.count(), total_duration_sec.count()/count, save_duration_sec.count()/count);
    fclose(fpt);
}

static void evaluate_typing_simul_1min(GapBuffer gapBuffer, string file_name, int n){
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
        gapBuffer.insert(string(1, ch), count);
        end1 = high_resolution_clock::now();
        total_duration_sec += std::chrono::duration_cast<duration<double, std::milli>>(end1 - start1);

        start2 = high_resolution_clock::now();
        gapBuffer.close(file_name + "_vol_test.txt");
        end2 = high_resolution_clock::now();
        save_duration_sec += std::chrono::duration_cast<duration<double, std::milli>>(end2 - start2);
        count++;
    }
    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);

    cout << "Total insert and save time (in ms):" << duration_sec.count() << endl;
    cout << "Average character insert latency (in ms):" << total_duration_sec.count()/count << endl;
    cout << "Average Save to file latency (in ms):" << save_duration_sec.count()/count << endl;

    FILE *fpt;
    fpt = fopen("volatile_gb_char.csv", "a+");
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
    string file_name = "init_read";

    GapBuffer gapBuffer;

    if(n < 0){        
        cout<<endl<<"n = "<<(-1)*n<<endl;
        cout<<"Gapbuffer volatile version\nInsert words evaluation mode\n";
        evaluate(gapBuffer, file_name, n);

        cout<<"Gapbuffer volatile version\nTyping simulation character wise evaluation mode\n";
        evaluate_typing_simul_1min(gapBuffer, file_name, n);
        exit(0);
    }

    start = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        gapBuffer.insert("a", 0);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "insert time:" << duration_sec.count() << " ms" << endl;

    start = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        gapBuffer.deleteCharacter(0);
    }
    end = high_resolution_clock::now();
    duration_sec =
        std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "remove time:" << duration_sec.count() << " ms" << endl;

    return 0;
}