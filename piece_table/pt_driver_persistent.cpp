#include <iostream>

#include "piece_table_persistent.h"

namespace pobj = pmem::obj;

using namespace std;

int main (int argc, char *argv[]) {
	pobj::pool<PieceTable::root> pop;
	auto r = pop.root();
	int ip;
	string fileName, insertStr;

	while(1){
		printf("1-> Create piece table\t2-> Insert\t3-> Seek\t4-> Remove\t5-> Rewind\n");
		cin>>ip;

		if(ip == 1){
			cout<<"Enter file name: ";
			cin>>fileName;
			PieceTable::create(pop, fileName);
		}
		else if(ip == 2){
			cout<<"Enter string to be inserted: ";
			cin>>insertStr;
			PieceTable::insert(pop, insertStr);
		}
		else if(ip == 3){
			PieceTable::seek(pop, 0, PieceTable::FWD);
		}
		else if(ip == 4){			
			PieceTable::remove(pop, 0);
		}
		else if(ip == 5){
			PieceTable::rewind(pop);
		}
		else if(ip == 6){
			PieceTable::print_table(pop);
		}
		else if(ip == 7){
			break;
		}
		else{
			cout<<"Invalid choice\n";
		}
	}
	return 0;
}
