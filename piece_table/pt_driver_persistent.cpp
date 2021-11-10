#include <iostream>

#include "piece_table_persistent.h"

using namespace std;

int main (int argc, char *argv[]) {
	pmem::obj::pool<PieceTable::root> pop;
	auto r = pop.root();
	int ip;
	char fileName[20], insertStr[50];

	while(1){
		printf("1-> Create piece table\t2-> Insert\t3-> Seek\t4-> Remove\t5-> Rewind\n");
		cin>>ip;

		if(ip == 1){
			cout<<"Enter file name: ";
			cin>>fileName;

			if (access(fileName, F_OK) != 0) {
				pop = pmem::obj::pool<PieceTable::root>::create(fileName, DEFAULT_LAYOUT, PMEMOBJ_MIN_POOL);		
				
				PieceTable::create(pop, fileName);
			}
			else {
				pop = pmem::obj::pool<PieceTable::root>::open(fileName, DEFAULT_LAYOUT);
			}
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
