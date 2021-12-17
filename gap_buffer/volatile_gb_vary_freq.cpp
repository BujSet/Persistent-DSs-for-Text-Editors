#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>

#define NUM_ITERS 1000

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
};

int main(int argc, char *argv[]) {
    size_t n = 1;

    if (argc == 2) {
        n = atoi(argv[1]);
    } else {
        cout << "usage: ./gb_vary_freq n" << endl;
        return 0;
    }

    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, std::milli> duration_sec;

    ifstream in_file;
    string file_path = "gb_vary_freq.txt";
    in_file.open(file_path);
		
    if(!in_file) {
        cout<<"Can't locate file!\n";
    }

    stringstream strStream;
    strStream << in_file.rdbuf();
    string fileContents = strStream.str();
    cout << "File contents: " << fileContents << endl;
    vector<char> char_vector (fileContents.begin(), fileContents.end()); 

    GapBuffer gapBuffer;

    start = high_resolution_clock::now();
    for (size_t i = 0; i < NUM_ITERS; i++)
    {
        gapBuffer.insert("a", 0);

        if (i % n == 0) {
            std::string text (gapBuffer.buffer.begin(), gapBuffer.buffer.end());
            // cout << "String before saving: " << text << endl;
            ofstream out_file;
	        out_file.open(file_path);
            // Write the string to the file.
            out_file << text;
	        out_file.close();
        }
    }
    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << "insert time:" << duration_sec.count() << " ms" << endl;

    return 0;
}