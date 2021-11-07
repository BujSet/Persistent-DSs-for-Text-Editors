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

            void grow(int k, int position) {
                
                char copy[size];
                for (int i = position; i < size; i++) {
                    copy[i - position] = buffer[i];
                }

                for (int i = 0; i < k; i++) {
                    buffer.insert(buffer.begin() + i + position, '_');
                }

                for (int i = 0; i < k + position; i++) {
                    buffer.insert(buffer.begin()+ position + i + k, copy[i]);
                }

                size += k;
                gap_right+=k;
            }
            
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

            void move_cursor(int position) {
                if (position < gap_left) {
                    left(position);
                }
                else {
                    right(position);
                }
            }

            void insert(string input_string, int position) {
                
                int i, len = input.length();
                i = 0;

                if (position != gap_left) {
                    move_cursor(position);
                }

                while (i < len) {
                    if (gap_right == gap_left) {
                        int k = 10;
                        grow(k, position);
                    }
                    buffer.at(gap_left) = input[i];
                    gap_left++;
                    i++;
                    position++;
                }
            }

            void delete_character(int position) {
                
                if (gap_left != position + 1) {
                    move_cursor(position + 1);
                }
                gap_left--;
                buffer.at(gap_left) = '_';
            }       

}