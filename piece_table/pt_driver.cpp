#include <iostream>
#include "piece_table.h"

using namespace std;

int main (int argc, char *argv[]) {
	if (argc != 2) {
		cout << "[usage]: <file_to_edit>\n";
		return 1;
	}
	PieceTable::PT *T = (PieceTable::PT *)malloc(sizeof(PieceTable::PT));
	PieceTable::open(T, argv[1]);
	int cursor_pos = PieceTable::get_cursor_pos(T);
	cout << "Cursor initially set to " << cursor_pos << "\n";
	char c = PieceTable::getc(T);
	cout << "First char is " << c << "\n";
	cursor_pos = PieceTable::get_cursor_pos(T);
	cout << "Cursor moved to " << cursor_pos << "\n";
	PieceTable::seek(T, 9, PieceTable::FWD);
	cursor_pos = PieceTable::get_cursor_pos(T);
	cout << "Cursor moved to " << cursor_pos << "\n";
	c = PieceTable::getc(T);
	cout << "Next char is " << c << "\n";
	cursor_pos = PieceTable::get_cursor_pos(T);
	cout << "Cursor moved to " << cursor_pos << "\n";

	PieceTable::seek(T, 2, PieceTable::BWD);
	cursor_pos = PieceTable::get_cursor_pos(T);
	cout << "Cursor moved to " << cursor_pos << "\n";

	PieceTable::close(T, "hello.txt");
	free(T);
	return 0;
}
