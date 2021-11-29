#include <iostream>
#include <vector>

#include "gap_buffer_persistent.h"

using namespace std;

void GapBuffer::create(pobj::pool<GapBuffer::root> pop, string file_path);

/**
*  This function inserts the 'input' string to the
*  buffer at the point 'position'
*/
void GapBuffer::insert(pobj::pool<GapBuffer::root> pop, string input, int position) {
    
    auto r = pop.root();
	if (r->root_gap_buffer == NULL) {
		cout << "Cannot insert in null gap buffer!\n";
		return;
	}    

    pobj::persistent_ptr<GapBuffer::gap_buffer> root_gap_buffer = r->root_gap_buffer;  
    int i = 0, len = input.length();

    pobj::transaction::run(pop, [&]{
        
        if (position != root_gap_buffer->gapLeft) {
            GapBuffer::moveCursor(pop, position);
        } 

        while (i < len) {

            // If the gap is empty, we need to grow the gap
            if (root_gap_buffer->gapRight == root_gap_buffer->gapLeft) {
                int k = 10;
                GapBuffer::grow(pop, k, position);
            }

            // Insert the character in the gap and move the gap
            root_gap_buffer->buffer.at(root_gap_buffer->gapLeft) = input[i];
            root_gap_buffer->gapLeft++;
            i++;
            position++;
        }  				
	});   
}

/**
*  This function deletes the character at the 'position' index
*/
void GapBuffer::deleteCharacter(pobj::pool<GapBuffer::root> pop, int position) {
    auto r = pop.root();
	if (r->root_gap_buffer == NULL) {
		cout << "Cannot delete on null gap buffer!\n";
		return;
	}    

    pobj::persistent_ptr<GapBuffer::gap_buffer> root_gap_buffer = r->root_gap_buffer;

    pobj::transaction::run(pop, [&]{
        
        if (root_gap_buffer->gapLeft != position + 1) {
            GapBuffer::moveCursor(pop, position + 1);
        }
        root_gap_buffer->gapLeft--;
        root_gap_buffer->buffer.at(root_gap_buffer->gapLeft) = '_';    				
	}); 

}

/**
*  This function controls the movement of the gap
*  by checking its position to the point of insertion 
*/
void GapBuffer::moveCursor(pobj::pool<GapBuffer::root> pop, int position) {

    auto r = pop.root();
	if (r->root_gap_buffer == NULL) {
		cout << "Cannot move cursor on null gap buffer!\n";
		return;
	}

    pobj::persistent_ptr<GapBuffer::gap_buffer> root_gap_buffer = r->root_gap_buffer;
    
    pobj::transaction::run(pop, [&]{
        
        if (position < root_gap_buffer->gapLeft) {
            GapBuffer::left(pop, position);
        } else {
            GapBuffer::right(pop, position);
        }       				
	});

}

/**
*  This function moves the gap to the left, in the vector 
*/
void GapBuffer::left(pobj::pool<GapBuffer::root> pop, int position) {
    auto r = pop.root();
	if (r->root_gap_buffer == NULL) {
		cout << "Cannot move right on null gap buffer!\n";
		return;
	}

    pobj::persistent_ptr<GapBuffer::gap_buffer> root_gap_buffer = r->root_gap_buffer;
    
    pobj::transaction::run(pop, [&]{
        
        // Moves the gap left, character by character
        while (position < root_gap_buffer->gapLeft) {
            root_gap_buffer->gapLeft--;
            root_gap_buffer->gapRight--;
            root_gap_buffer->buffer.at(root_gap_buffer->gapRight + 1) = root_gap_buffer->buffer[root_gap_buffer->gapLeft];
            root_gap_buffer->buffer.at(root_gap_buffer->gapLeft) = '_';
        }        		
			
	});
}

/**
*  This function moves the gap to the right, in the vector 
*/
void GapBuffer::right(pobj::pool<GapBuffer::root> pop, int position) {
    auto r = pop.root();
	if (r->root_gap_buffer == NULL) {
		cout << "Cannot move left on null gap buffer!\n";
		return;
	}

    pobj::persistent_ptr<GapBuffer::gap_buffer> root_gap_buffer = r->root_gap_buffer;
    
    pobj::transaction::run(pop, [&]{
    
        // Moves the gap right, character by character
        while (position > root_gap_buffer->gapLeft) {
            root_gap_buffer->gapLeft++;
            root_gap_buffer->gapRight++;
            root_gap_buffer->buffer.at(root_gap_buffer->gapLeft - 1) = root_gap_buffer->buffer[root_gap_buffer->gapRight];
            root_gap_buffer->buffer.at(root_gap_buffer->gapRight) = '_';
        }               		
			
	});
}

/**
*  This function moves is used to grow the gap 
*  at index position and return the vector 
*/
void GapBuffer::grow(pobj::pool<GapBuffer::root> pop, int k, int position) {

    auto r = pop.root();
	if (r->root_gap_buffer == NULL) {
		cout << "Cannot insert in null gap buffer!\n";
		return;
	}    

    pobj::persistent_ptr<GapBuffer::gap_buffer> root_gap_buffer = r->root_gap_buffer;  

    std::vector<char> copy_vector(root_gap_buffer->size);

    pobj::transaction::run(pop, [&]{  

        // The characters of the buffer after 'position' 
        // are copied to the copy array
        for (int i = position; i < size; i++) {
            copy_vector[i - position] = root_gap_buffer->buffer[i];
        }

        // A gap of 'k' is inserted from the 'position' index
        // The gap is represented by '_'
        for (int i = 0; i < k; i++) {
            root_gap_buffer->buffer.insert(root_gap_buffer->buffer.begin() + i + position, '_');
        }

        // The remaining array is inserted
        for (int i = 0; i < k + position; i++) {
            root_gap_buffer->buffer.insert(root_gap_buffer->buffer.begin()+ position + i + k, copy_vector[i]);
        }

        root_gap_buffer->size += k;
        root_gap_buffer->gapRight += k;            				
	}); 

} 

void GapBuffer::print_table(pobj::pool<GapBuffer::root> pop);

void GapBuffer::close(pobj::pool<GapBuffer::root> pop, string file_path);

class GapBuffer2 {

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