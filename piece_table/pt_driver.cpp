#include <iostream>
#include "piece_table.h"

using namespace std;

int main (int argc, char *argv[]) {
	if (argc != 2) {
		cout << "[usage]: <file_to_edit>\n";
		return 1;
	}
	PieceTable::PT *T = (PieceTable::PT *)malloc(sizeof(PieceTable::PT));
	PieceTable::init(T, argv[1]);
	//cout << T->pieces.size() << "\n";
	PieceTable::insert(T, "Hello ", 0);
	string text =  PieceTable::stitch(T);
	cout << text;
	//cout << T->pieces.size() << "\n";
	PieceTable::insert(T, " World", text.length() - 1);
	text =  PieceTable::stitch(T);
	cout << text;
	//cout << T->pieces.size() << "\n";
	PieceTable::insert(T, " INSERTION", 11);
	text =  PieceTable::stitch(T);
	cout << text;
	return 0;
}
