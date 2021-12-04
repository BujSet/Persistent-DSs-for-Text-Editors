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
        string fileContents = strStream.str();
        vector<char> char_vector (fileContents.begin(), fileContents.end()); 
        auto temp = pobj::make_persistent<GapBuffer::char_vector_type>(char_vector, char_vector.size());
		gBuffer->buffer = temp;

        GapBuffer::initValues(gBuffer);

	});
}

void GapBuffer::initValues(pobj::persistent_ptr<GapBuffer::gap_buffer> gBuffer) {
    
    gBuffer-> size = gBuffer->buffer->size();

    cout << "Total Size: " << gBuffer-> size << endl;

    int size = gBuffer->buffer->size();

    int gap_left, gap_right, gap_size;
    gap_left = gap_right = gap_size = 0

    for (size_t i = 0; i < size; i++) {
        if (gBuffer->buffer[i] == '_') {
            if (gap_left = 0) {
                gap_left = i;
            } else {
                gap_right = i;
            }
        }
    }
    gap_size = gap_right - gap_left + 1;

    cout << "Gap Left: " << gap_left << " Gap Right: " << gap_right << " Gap Size: " << gap_size << endl;

    gBuffer->left = gap_left;
    gBuffer->right = gap_right;
    gBuffer->size = gap_size;

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
    
    std::string text (gbuffer->buffer.begin(), gbuffer->buffer.end());
	ofstream out_file;
	out_file.open(file_path);
	// Write the string to the file.
    out_file << text;
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