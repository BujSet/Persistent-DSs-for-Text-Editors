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
	PieceTable::insert(T, "Hello ", 0);
	string text =  PieceTable::stitch(T);
	cout << text;
	PieceTable::insert(T, " World", text.length() - 1);
	text =  PieceTable::stitch(T);
	cout << text;
	PieceTable::insert(T, " INSERTION", 11);
	text =  PieceTable::stitch(T);
	cout << text;
	PieceTable::remove(T, 21, 6); // Remove " ipsum" from text
	text =  PieceTable::stitch(T);
	cout << text;
	cout << text.length();
	PieceTable::remove(T, text.length() - 6, 6); // Remove " World" from text
	text =  PieceTable::stitch(T);
	cout << text;
	PieceTable::close(T, "hello.txt");
	free(T);
	return 0;
}
