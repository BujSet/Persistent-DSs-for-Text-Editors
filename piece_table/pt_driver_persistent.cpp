#include <iostream>
#include <fstream>
#include <cstdio>

#include "piece_table_persistent.h"

namespace pobj = pmem::obj;

using namespace std;

int main (int argc, char *argv[]) {
	pobj::pool<PieceTable::root> pop;
	int ip, len_str, offset;
	string file_name, insert_str;

	file_name = "init_read";
	if (access((file_name + "_pool").c_str(), F_OK) != 0) {
		cout<<"Created pool!\n";
		pop = pmem::obj::pool<PieceTable::root>::create(file_name + "_pool", DEFAULT_LAYOUT, PMEMOBJ_MIN_POOL);	
		pobj::transaction::run(pop, [&]{
			(pop.root())->root_piece_table = pobj::make_persistent<PieceTable::piece_table>();				
		});

		PieceTable::create(pop, file_name + ".txt");			
	}
	else {
		cout<<"Opened existing pool!\n";
		pop = pmem::obj::pool<PieceTable::root>::open(file_name + "_pool", DEFAULT_LAYOUT);
	}

	while(1){
		cout<<"1-> Reinitialize piece table pool\t 2-> Insert\t 3-> SeekFwd\t 4-> SeekBwd\t 5-> Remove\t 6-> Rewind\n";
		cout<<"7-> Print\t 8-> Write to outfile\t 9-> Quit\n";
		cin>>ip;

		if(ip == 1){
			string pool_name = string(file_name + string("_pool"));
			if(remove(pool_name.c_str()) != 0){
				cout<<"Unable to delete existing pool: "<<pool_name<<"\n";
				continue;
			}

			pop = pmem::obj::pool<PieceTable::root>::create(file_name + "_pool", DEFAULT_LAYOUT, PMEMOBJ_MIN_POOL);	
			pobj::transaction::run(pop, [&]{
				(pop.root())->root_piece_table = pobj::make_persistent<PieceTable::piece_table>();				
			});

			PieceTable::create(pop, file_name + ".txt");
		}
		else if(ip == 2){
			cout<<"Enter string to be inserted: ";
			cin.ignore();
			std::getline(std::cin, insert_str);
			PieceTable::insert(pop, insert_str);
		}
		else if(ip == 3){
			cout<<"Enter offset: ";
			cin>>offset;
			PieceTable::seek(pop, offset, PieceTable::FWD);
		}
		else if(ip == 4){
			cout<<"Enter offset: ";
			cin>>offset;
			PieceTable::seek(pop, offset, PieceTable::BWD);
		}
		else if(ip == 5){
			cout<<"Enter length of string to be removed: ";
			cin>>len_str;	
			PieceTable::remove(pop, len_str);
		}
		else if(ip == 6){
			PieceTable::rewind(pop);
		}
		else if(ip == 7){
			PieceTable::print_table(pop);
		}
		else if(ip == 8){
			PieceTable::close(pop, file_name + "_pers.txt");
		}
		else if(ip == 9){
			break;
		}
		else{
			cout<<"Invalid choice\n";
		}
	}
	return 0;
}
