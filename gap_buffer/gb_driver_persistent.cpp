#include <iostream>

#include "gap_buffer_persistent.h"

namespace pobj = pmem::obj;

using namespace std;

int main (int argc, char *argv[]) {
	pobj::pool<GapBuffer::root> pop;
	int input, offset;
	string file_path, insert_str, out_path;

	file_path = "gap_buffer_in.txt";
	out_path = "gap_buffer_out.txt";
	
	if (access((file_path + "_pers").c_str(), F_OK) != 0) {
		pop = pmem::obj::pool<GapBuffer::root>::create(file_path + "_pers", DEFAULT_LAYOUT, PMEMOBJ_MIN_POOL);				
	} else {
		pop = pmem::obj::pool<GapBuffer::root>::open(file_path + "_pers", DEFAULT_LAYOUT);
	}

	while(1){

		printf(" 1 -> Create Gap Buffer \n 2 -> Insert String \n 3 -> Delete Character \n 4 -> Move Cursor \n");
		printf(" 5 -> Print Contents \n 6 ->  Write to the output file\n 7 -> Quit \n");
		
		cin>>input;
		int cursor_position = 0;

		if(input == 1) {
			
			auto r = pop.root();
			
			pobj::transaction::run(pop, [&]{
				r->root_gap_buffer = pobj::make_persistent<GapBuffer::gap_buffer>();				
			});
			
			GapBuffer::create(pop, file_path);

		} else if(input == 2) {
			
			cout << "Enter string to be inserted: ";
			cin >> insert_str;
			
			// For testing purpose, we're using the 0th position here. Could be changed
			GapBuffer::insert(pop, insert_str, cursor_position);

		} else if(input == 3){
			
			cout << "Enter the position from which you need to delete the character: ";
			cin>>offset;

			// For testing purpose, we're not validating the input. Might need to handle boundary conditions
			GapBuffer::deleteCharacter(pop, offset);

		} else if(input == 4){
			
			cout << "Enter offset where you'd want the cursor to move to: ";
			cin >> cursor_position;
			
			// We will try to re-use this cursor position for insertion of the character as well.
			// Assumption is that the input will be valid, not validating the same.
			GapBuffer::moveCursor(pop, cursor_position);

		} else if(input == 5) {

			cout << "Printing the contents now: \n";
			GapBuffer::print_buffer(pop);

		} else if(input == 6) {
			cout << "Writing to the output file: " << file_path << endl;
			GapBuffer::close(pop, file_path);

		} else if(input == 7){
			break;
		} else {
			cout<<"Invalid choice\n";
		}
	}
	return 0;
}
