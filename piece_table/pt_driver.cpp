#include <iostream>
#include "piece_table.h"

using namespace std;

int main (int argc, char *argv[]) {
	if (argc != 2) {
		cout << "[usage]: <file_to_edit>\n";
		return 1;
	}
	cout << "Initalizing piece table...";
	PieceTable::PT *T = (PieceTable::PT *)malloc(sizeof(PieceTable::PT));
	cout << "Success!\n";

	PieceTable::open(T, argv[1]);
	PieceTable::seek(T, 9, PieceTable::FWD);
	cout << "After seeking 9 forward\n";
	PieceTable::print_cursor(T->cursor);
	PieceTable::seek(T, 2, PieceTable::BWD);
	cout << "After seeking 2 backward\n";
	PieceTable::print_cursor(T->cursor);
	PieceTable::seek(T, PieceTable::get_cursor_pos(T), PieceTable::BWD);
	cout << "After seeking all the way backward\n";
	PieceTable::print_cursor(T->cursor);
	PieceTable::rewind(T);

	cout << "--------------------------About to start all the weird stuff----------------------\n";

	PieceTable::insert(T, "World ");
	cout << "After inserting the word World\n";
	PieceTable::print_table(T);

	PieceTable::seek(T, PieceTable::get_cursor_pos(T), PieceTable::BWD);
	cout << "After seeking back all the way to the beginning\n";
	PieceTable::print_table(T);

	PieceTable::insert(T, "Hello ");
	cout << "After inserting hello\n";
	PieceTable::print_table(T);

	PieceTable::rewind(T);
	cout << "After rewinding\n";
	PieceTable::print_table(T);
	PieceTable::seek(T, 12, PieceTable::FWD);
	cout << "After seeking 12 forward\n";
	PieceTable::print_table(T);
	PieceTable::seek(T, 12, PieceTable::BWD);
	cout << "After seeking 12 backward\n";
	PieceTable::print_table(T);
	//TODO seeking backwards past beginning file results in seg fault, need to fix that
	PieceTable::seek(T, 6, PieceTable::FWD);
	cout << "After seeking 6 forward\n";
	PieceTable::print_table(T);
	PieceTable::seek(T, 2, PieceTable::FWD);
	cout << "After seeking 2 forward\n";
	PieceTable::print_table(T);
	PieceTable::remove(T, 2);
	cout << "After removing 2 bytes past cursor\n";
	PieceTable::print_table(T);
	//PieceTable::seek(T, 1, PieceTable::FWD);
	//cout << "After seeking 1 forward\n";
	//PieceTable::print_table(T);
	PieceTable::rewind(T);
	cout << "After rewind\n";
	PieceTable::print_table(T);
	PieceTable::seek(T, 3, PieceTable::FWD);
	cout << "After seeking 3 forward\n";
	PieceTable::print_table(T);
	PieceTable::remove(T, 5);
	cout << "After removing 5 bytes past cursor\n";
	PieceTable::print_table(T);

	PieceTable::close(T, "hello.txt");
	free(T);
	return 0;
}
