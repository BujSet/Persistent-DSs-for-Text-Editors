#include <iostream>
#include <vector>

using namespace std;

/**
 * This inspiration for this class has been taken from the 
 * concepts and the code that has been described here:
 * https://www.geeksforgeeks.org/gap-buffer-data-structure/
 * All the functionalities mentioned here have been implemented.
 */ 

class GapBuffer {

    public:
            vector<char> buffer;
            int gapSize = 10;
            int gapLeft = 0;
            int gapRight = gapSize - gapLeft - 1;
            int size = 10;

            GapBuffer() {
                gapSize = 10;
                gapLeft = 0;
                gapRight = gapSize - gapLeft - 1;
                size = 10;

                for (int i = 0; i < size; i++) {
                    buffer.push_back('_');
                }
            }

            /**
             *  This function is used to grow the gap 
             *  at index position and return the vector 
             */
            void grow(int k, int position) {
                
                vector<char> copy(size);

                // The characters of the buffer after 'position' 
                // are copied to the copy array
                for (int i = position; i < size; i++) {
                    copy[i - position] = buffer[i];
                }

                // A gap of 'k' is inserted from the 'position' index
                // The gap is represented by '_'
                for (int i = 0; i < k; i++) {
                    buffer.insert(buffer.begin() + i + position, '_');
                }

                // The remaining array is inserted
                for (int i = 0; i < k + position; i++) {
                    buffer.insert(buffer.begin()+ position + i + k, copy[i]);
                }

                size += k;
                gapRight += k;
            }
            
            /**
             *  This function moves the gap to the left, in the vector 
             */
            void left(int position) {
                while (position < gapLeft) {
                    gapLeft--;
                    gapRight--;
                    buffer.at(gapRight + 1) = buffer[gapLeft];
                    buffer.at(gapLeft) = '_';
                }
            }
            
            /**
             *  This function moves the gap to the right, in the vector 
             */
            void right(int position) {
                while (position > gapLeft) {
                    gapLeft++;
                    gapRight++;
                    buffer.at(gapLeft - 1) = buffer[gapRight];
                    buffer.at(gapRight) = '_';
                }                
            }

            /**
             *  This function controls the movement of the gap
             *  by checking its position relative to the point of insertion 
             */
            void moveCursor(int position) {
                if (position < gapLeft) {
                    left(position);
                }
                else {
                    right(position);
                }
            }

            /**
             *  This function inserts the 'input' string to the
             *  buffer at the point 'position'
             */
            void insert(string input, int position) {
                
                int i = 0, len = input.length();

                if (position != gapLeft) {
                    moveCursor(position);
                }

                while (i < len) {

                    // If the gap is empty, we need to grow the gap
                    if (gapRight == gapLeft) {
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
            void deleteCharacter(int position) {
                
                if (gapLeft != position + 1) {
                    moveCursor(position + 1);
                }
                gapLeft--;
                buffer.at(gapLeft) = '_';
            }       

};

int main() { 

    GapBuffer gapBuffer;    
  
    cout << "Initializing the gap buffer with size 10" << endl;
   
    for (int i = 0; i < 10; i++) { 
        cout << gapBuffer.buffer.at(i) << " "; 
    } 
  
    cout << endl; 
  
    // Inserting a string to buffer 
    string input = "GEEKSGEEKS"; 
    int position = 0; 
  
    gapBuffer.insert(input, position); 
  
    cout << endl; 
    cout << "Inserting a string to buffer: GEEKSGEEKS" << endl; 
    cout << "Output: "; 
    for (int i = 0; i < gapBuffer.size; i++) { 
        cout << gapBuffer.buffer[i]<<" "; 
    } 
  
    input = "FOR"; 
    position = 5; 
  
    gapBuffer.insert(input, position); 
  
    cout << endl; 
    cout << endl; 
      
    cout << "Inserting a string to buffer: FOR" << endl; 
    cout << "Output: "; 
    for (int i = 0; i < gapBuffer.size; i++) { 
        cout << gapBuffer.buffer[i]<<" "; 
    }
  
    return 0;
}