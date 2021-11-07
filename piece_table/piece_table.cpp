#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <assert.h>

#include "piece_table.h"

using namespace std;

void PieceTable::init(PieceTable::PT *T, string file_path) {
	if (!T) {
		cout << "Must allocate T first!\n";
		return;
	}

	ifstream in_file;
	in_file.open(file_path);
	if (!in_file.is_open()){
		cout << "Unable to open input file " << file_path << " for read.\n";
		return;
	}
	PieceTable::P *P=(PieceTable::P *)malloc(sizeof(PieceTable::P));
	if (!P) {
		cout << "Unable to malloc initial piece!\n";
		return;
	}
	stringstream strStream;
    	strStream << in_file.rdbuf(); //read the file
    	T->original = strStream.str();
	P->src = ORIGINAL;
	P->start = 0;
	P->len = T->original.length();
	T->pieces.push_back(P);
}

string PieceTable::stitch(PieceTable::PT *T) {
	if (!T) {
		cout << "Unable to read null piece table!\n";
		return "";
	}
	string ret;
	//cout << "stitch called when there are " << T->pieces.size() <<" pieces\n";
	for (const auto& P: T->pieces) {
		assert(P);
		if (P->src == PieceTable::ORIGINAL) {
			//cout << "Reading " << P->len << " bytes from offset " << P->start << " from original buffer\n";
			ret.append(T->original, P->start, P->len);
		} else {
			//cout << "Reading " << P->len << " bytes from offset " << P->start << " from add buffer\n";
			assert(P->src == PieceTable::ADD);
			ret.append(T->add, P->start, P->len);
		}
	}
	return ret;
}

void PieceTable::insert(PieceTable::PT *T, string s, size_t offset) {
	//cout << "Inserting " << s << " to position " << offset << "\n";
	if (!T) {
		cout << "Unable to insert string to null piece table!\n";
		return;
	}
	// First we create a piece to represent this addition
	PieceTable::P *P = (PieceTable::P *)malloc(sizeof(PieceTable::P));
	if (!P) {
		cout << "Unable to malloc new piece for insertion!\n";
		return;
	}
	P->src = PieceTable::ADD;
	P->start = T->add.length();
	P->len = s.length();
	//cout << "P->start=" << P->start << "\n";
	//cout << "P->len=" << P->len << "\n";

	// Next we place the string into the add buffer
	T->add.append(s);

	// Now we need to find where to insert the piece we created earlier
	if (offset == 0) {
		// Special edge of adding to the beginning of the text
		T->pieces.insert(T->pieces.begin(), P);
		return;
	}

	size_t prev_char_cnt = 0;
	size_t char_cnt = 0;
	size_t ins_idx = 0;
	bool split_piece = true;
	for (size_t i = 0; i < T->pieces.size(); i++) {
		ins_idx = i;
		char_cnt += T->pieces[i]->len;
		//cout << "Char count in loop is=" << char_cnt << "\n";
		if (offset == char_cnt) {
			ins_idx++;
			split_piece = false;
			break;
		}
		if (offset < char_cnt) {
			break;
		}
		prev_char_cnt = char_cnt;
	}

	if (!split_piece) {
		// next edge case is where we insert on existing piece boundaries
		//cout << "Inserting to index " << ins_idx << "\n";
		T->pieces.insert(T->pieces.begin() + ins_idx, P);
		return;
	}
	
	// Now we need to split a piece 
	PieceTable::P *to_split = T->pieces[ins_idx];
	//cout << "to_split->len=" << to_split->len << "\n";
	PieceTable::P *prev = (PieceTable::P *)malloc(sizeof(PieceTable::P));
	PieceTable::P *post = (PieceTable::P *)malloc(sizeof(PieceTable::P));
	if (!prev || !post) {
		cout << "Unable to split piece where insertion belongs!\n";
		return;
	}
	prev->src = to_split->src;
	prev->start = to_split->start;
	prev->len = offset - prev_char_cnt;
	//cout << "prev->start=" << prev->start << "\n";
	//cout << "prev->len=" << prev->len << "\n";
	post->src = to_split->src;
	post->start = prev->start + prev->len;
	post->len = to_split->len - prev->len;
	//cout << "post->start=" << prev->start << "\n";
	//cout << "post->len=" << post->len << "\n";

	T->pieces.erase(T->pieces.begin() + ins_idx);
	T->pieces.insert(T->pieces.begin() + ins_idx, post);
	T->pieces.insert(T->pieces.begin() + ins_idx, P);
	T->pieces.insert(T->pieces.begin() + ins_idx, prev);

	free(to_split);
}

