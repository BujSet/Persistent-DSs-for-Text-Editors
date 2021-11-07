#include <iostream>
#include <vector>

using namespace std;

class GapBuffer {

    public:
            vector<char> buffer;
            int gap_size = 10;
            int gap_left = 0;
            int gap_right = gap_size - gap_left - 1;
            int size = 10;

            void grow(int k, int position);
            
            void left(int position) {
                while (position < gap_left) {
                    gap_left--;
                    gap_right--;
                    buffer.at(gap_right + 1) = buffer[gap_left];
                    buffer.at(gap_left) = '_';
                }
            }

            void right(int position) {
                while (position > gap_left) {
                    gap_left++;
                    gap_right++;
                    buffer.at(gap_left - 1) = buffer[gap_right];
                    buffer.at(gap_right) = '_';
                }                
            }

            void move_cursor(int position);

            void insert(string input_string, int position);

            void delete_character(int position);        

}