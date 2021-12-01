#include <iostream>

#include "piece_table_persistent.h"

namespace pobj = pmem::obj;

using namespace std;

int main (int argc, char *argv[]) {
	pobj::pool<PieceTable::root> pop;
	int ip, len_str, offset;
	string file_path, insert_str, out_path;

	// cout<<"Enter file name: ";
	// cin>>file_path;
	file_path = "eg.txt";
	out_path = "egout.txt";
	if (access((file_path + "_pers").c_str(), F_OK) != 0) {
		pop = pmem::obj::pool<PieceTable::root>::create(file_path + "_pers", DEFAULT_LAYOUT, PMEMOBJ_MIN_POOL);				
	}
	else {
		pop = pmem::obj::pool<PieceTable::root>::open(file_path + "_pers", DEFAULT_LAYOUT);
	}

	while(1){
		cout<<"1-> Create piece table\t2-> Insert\t3-> SeekFwd\t4-> Remove\t5-> Rewind\t6-> Print\t";
		cout<<"7-> Write to outfile\t8-> Quit\n";
		cin>>ip;

		if(ip == 1){
			pobj::transaction::run(pop, [&]{
				(pop.root())->root_piece_table = pobj::make_persistent<PieceTable::piece_table>();				
			});

			PieceTable::create(pop, file_path);
		}
		else if(ip == 2){
			cout<<"Enter string to be inserted: ";
			cin>>insert_str;
			PieceTable::insert(pop, insert_str);
		}
		else if(ip == 3){
			cout<<"Enter offset: ";
			cin>>offset;
			PieceTable::seek(pop, 0, PieceTable::FWD);
		}
		else if(ip == 4){
			cout<<"Enter length of string to be removed: ";
			cin>>len_str;	
			PieceTable::remove(pop, len_str);
		}
		else if(ip == 5){
			PieceTable::rewind(pop);
		}
		else if(ip == 6){
			PieceTable::print_table(pop);
		}
		else if(ip == 7){
			PieceTable::close(pop, out_path);
		}
		else if(ip == 8){
			break;
		}
		else{
			cout<<"Invalid choice\n";
		}
	}
	return 0;
}
