#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <vector>

#include "gap_buffer_persistent.h"

using namespace std;

void GapBuffer::create(pobj::pool<GapBuffer::root> pop, string file_path) {
    auto r = pop.root();
	pobj::persistent_ptr<GapBuffer::gap_buffer> gBuffer;
	ifstream in_file;

	pobj::transaction::run(pop, [&] {
        gBuffer = r->root_gap_buffer;	
		// gBuffer->buffer = pobj::make_persistent<GapBuffer::char_vector_type>();
    
		GapBuffer::char_vector_type &cvector = *(gBuffer->buffer);

		in_file.open(file_path);
		
		if(!in_file){
		  	cout<<"Can't locate file!\n";
			return;
		}

		stringstream strStream;
		strStream << in_file.rdbuf();
        // TODO: Need to decide if I want to add just the text in the buffer to the file or some metadata as well?
		auto temp = pobj::make_persistent<GapBuffer::char_vector_type>(strStream.str().c_str(), strlen(strStream.str().c_str()));
		gBuffer->buffer = temp;

        // TODO: Need to see how to calculate these values on the go {Iterate through the string or character vector?}
        // gBuffer-> gap_size = 
        // gBuffer-> gap_left = 
        // gBuffer-> gap_right = 
        
        gBuffer-> size = gBuffer->buffer->size();

	});
}

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
		cout << "Cannot move left on null gap buffer!\n";
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
		cout << "Cannot move right on null gap buffer!\n";
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

void GapBuffer::print_buffer(pobj::pool<GapBuffer::root> pop) {
	
    auto r = pop.root();
	
    if (r->root_gap_buffer == NULL) {
		cout << "Unable to print null gap buffer\n";
		return;
	}

	pobj::persistent_ptr<GapBuffer::gap_buffer> root_gap_buffer = r->root_gap_buffer;
	PieceTable::piece piece;	
	char c;

	pobj::transaction::run(pop, [&]{
		
        cout << "<---------- Printing Gap Buffer ----------->\n";
		GapBuffer::char_vector_type &char_vector = *(root_gap_buffer->buffer);

		cout<<"ptable->original: "<<(ptable->original)->c_str()<<"\n";
		cout<<"ptable->add: "<<(ptable->add)->c_str()<<"\n";
		
        for (size_t i = 0; i < char_vector.size(); i++) {
            cout << "At position : " << i << " character: " << char_vector[i] << endl;
		}

        cout << "Gap Size: " << root_gap_buffer->gap_size << endl;
        cout << "Gap Left: " << root_gap_buffer->gap_left << endl; 
        cout << "Gap Right: " << root_gap_buffer->gap_right << endl;
        cout << "Total Size: " << root_gap_buffer->size << endl;
	});

}

void GapBuffer::close(pobj::pool<GapBuffer::root> pop, string file_path) {
	auto r = pop.root();
	
    if (r->root_gap_buffer == NULL) {
		cout << "Unable to close null gap buffer!\n";
		return;
	}

	pobj::persistent_ptr<PieceTable::gap_buffer> gbuffer = r->root_gap_buffer;
    
    // string text = convert the vector of characters to a string;
	ofstream out_file;
	out_file.open(file_path);
	// Write the string to the file.
    // out_file << text;
	out_file.close();

	pobj::transaction::run(pop, [&]{
        
		/*
            Check if need to delete something while closing?
            PieceTable::piece_vector_type &pvector = *(ptable->pieces);

            for (int i = 0; i < pvector.size(); i++) {
                pobj::delete_persistent<PieceTable::piece>(&pvector[i]);
            }
        */
	});
}

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