#include <iostream>
#include "piece_table.h"

using namespace std;

int main (int argc, char *argv[]) {
	if (argc != 2) {
		cout << "[usage]: <file_to_edit>\n";
		return 1;
	}
	PieceTable::PT *T = (PieceTable::PT *)malloc(sizeof(PieceTable::PT));

	string file_name = "init_read";
	PieceTable::open(T, file_name + ".txt");
	PieceTable::seek(T, 9, PieceTable::FWD);
	PieceTable::seek(T, 2, PieceTable::BWD);
	PieceTable::seek(T, PieceTable::get_cursor_pos(T), PieceTable::BWD);
	PieceTable::rewind(T);

	PieceTable::insert(T, "World ");

	PieceTable::seek(T, PieceTable::get_cursor_pos(T), PieceTable::BWD);

	PieceTable::insert(T, "Hello ");

	PieceTable::rewind(T);
	PieceTable::seek(T, 12, PieceTable::FWD);
	PieceTable::seek(T, 12, PieceTable::BWD);
	//TODO seeking backwards past beginning file results in seg fault, need to fix that
	PieceTable::seek(T, 6, PieceTable::FWD);
	PieceTable::seek(T, 2, PieceTable::FWD);
	PieceTable::remove(T, 2);
	PieceTable::rewind(T);
	PieceTable::seek(T, 4, PieceTable::FWD);
	PieceTable::remove(T, 7);
	PieceTable::rewind(T);

	PieceTable::remove(T, 5);

	PieceTable::close(T, file_name + "_vol.txt");
	free(T);
	return 0;
}
