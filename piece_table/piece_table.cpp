#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <vector>

#include "piece_table.h"

using namespace std;

void PieceTable::open(PieceTable::PT *T, string file_path) {
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
	PieceTable::Cursor *C = (PieceTable::Cursor *)malloc(sizeof(PieceTable::Cursor));
	if (!C) {
		cout << "Unable to malloc cursor struct\n!";
		free(P);
		return;
	}

	stringstream strStream;
    	strStream << in_file.rdbuf(); //read the file
    	T->original = strStream.str();
	
	P->src = ORIGINAL;
	P->start = 0;
	P->len = T->original.length();

	T->pieces.push_back(P);

	C->pos = 0;
	C->piece_idx = 0;
	C->piece_offset = 0;
	T->cursor = C;
}

string PieceTable::stitch(PieceTable::PT *T) {
	if (!T) {
		cout << "Unable to read null piece table!\n";
		return "";
	}
	string ret;
	for (const auto& P: T->pieces) {
		assert(P);
		if (P->src == PieceTable::ORIGINAL) {
			ret.append(T->original, P->start, P->len);
		} else {
			assert(P->src == PieceTable::ADD);
			ret.append(T->add, P->start, P->len);
		}
	}
	return ret;
}

void PieceTable::insert(PieceTable::PT *T, string s) {
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

	// Next we place the string into the add buffer
	T->add.append(s);
	
	PieceTable::Cursor *C = T->cursor;
	PieceTable::P *piece = T->pieces[C->piece_idx];
	if (C->piece_offset == 0) {
		// Need to insert the piece before cursor piece
		T->pieces.insert(T->pieces.begin() + C->piece_idx, P);
		C->pos += P->len;
		C->piece_idx++;
	} else if (C->piece_offset == piece->len - 1) {
		// Need to insert the piece after cursor piece
		C->piece_idx++;
		T->pieces.insert(T->pieces.begin() + C->piece_idx, P);
		C->pos += P->len;
	} else {
		// Need to split the piece at cursor
		PieceTable::P *post = (PieceTable::P *)malloc(sizeof(PieceTable::P));
		post->src = piece->src;
		post->start = piece->start + C->piece_offset;
		post->len = piece->len - C->piece_offset;
		piece->len = C->piece_offset;
		C->piece_idx++;
		C->pos += P->len;
		T->pieces.insert(T->pieces.begin() + C->piece_idx, post);
		T->pieces.insert(T->pieces.begin() + C->piece_idx, P);
		C->piece_idx++;
		C->piece_offset = 0;
	}

}

void PieceTable::remove(PieceTable::PT *T, size_t len) {
	if (!T) {
		cout << "Unable to remove string from null piece table\n";
		return;
	}
	if (len == 0) {
		return;
	}

	size_t bytes_to_remove = len;
	PieceTable::Cursor *C;
	PieceTable::P *piece;
	while (bytes_to_remove) {
		C = T->cursor;
		piece = T->pieces[C->piece_idx];

		if (C->piece_offset == 0) {
			if (bytes_to_remove >= piece->len) {
				bytes_to_remove -= piece->len;
				T->pieces.erase(T->pieces.begin() + C->piece_idx);
				free(piece);
			} else {
				piece->start += bytes_to_remove; 
				piece->len -= bytes_to_remove;
				bytes_to_remove = 0;
			}
		} else { 
			// Need to break up the cursor piece
			if (bytes_to_remove > (piece->len - C->piece_offset)) {
				bytes_to_remove -= (piece->len - C->piece_offset);
				piece->len = C->piece_offset;
				C->piece_idx++;
				C->piece_offset = 0;
			} else {
				if (bytes_to_remove == (piece->len - C->piece_offset)) {
					piece->len -= bytes_to_remove;
					bytes_to_remove = 0;
				}
				else {
					PieceTable::P *post = (PieceTable::P *)malloc(sizeof(PieceTable::P));
					post->src = piece->src;
					post->start = piece->start + bytes_to_remove + C->piece_offset;
					post->len = piece->len - post->start;
					piece->len = C->piece_offset;
					bytes_to_remove = 0;
					C->piece_idx++;
					C->piece_offset = 0;
					T->pieces.insert(T->pieces.begin()+C->piece_idx, post);
				}
			}
		}
	}
}

int PieceTable::get_cursor_pos(PieceTable::PT *T) {
	if (!T) {
		cout << "Cannot find cursor on a null table\n";
		return -1;
	}
	return T->cursor->pos;
}

void PieceTable::print_cursor(PieceTable::Cursor *C) {
	if (!C) {
		cout << "Cannot print null cursor\n";
		return;
	}
	cout << "Cursor:={pos=" << C->pos << ", piece_idx=" << C->piece_idx << ", piece_offset=" << C->piece_offset << "}\n";
}

void PieceTable::seek(PieceTable::PT *T, size_t offset, PieceTable::SeekDir dir) {
	if (offset == 0) {
		return;
	}

	if (!T) {
		cout << "Cannot move cursor on null piece table!\n";
		return;
	}
	
	size_t bytes_moved = 0, bytes_to_move;
	PieceTable::Cursor *C = T->cursor;
	while (bytes_moved < offset) {
		bytes_to_move = offset - bytes_moved;
		PieceTable::P *piece = T->pieces[C->piece_idx];
		if (dir==PieceTable::FWD) {
			if ((piece->len - C->piece_offset) > bytes_to_move) {
				C->pos += bytes_to_move;
				C->piece_offset += bytes_to_move;
				break; // we're done seeking to new position
			}
			assert((piece->len - C->piece_offset) <= bytes_to_move);
			C->pos += piece->len - C->piece_offset;
			C->piece_idx++;
			C->piece_offset = 0;
			bytes_moved += piece->len - C->piece_offset;
		} else {
			assert(dir == PieceTable::BWD);
			if (C->piece_offset >= bytes_to_move) {
				C->pos -= bytes_to_move;
				C->piece_offset -= bytes_to_move;
				break; // we're done seeking to new position
			}
			assert(C->piece_offset <= bytes_to_move);
			C->pos -= C->piece_offset;
			C->piece_idx--;
			bytes_moved += C->piece_offset;
			C->piece_offset = T->pieces[C->piece_idx]->len;
		}
	}
	
}

void PieceTable::rewind(PieceTable::PT *T) {
	if (!T) {
		cout << "Unbale to get character from null piece table\n";
		return;
	}
	T->cursor->pos = 0;
	T->cursor->piece_idx = 0;
	T->cursor->piece_offset = 0;
}

void PieceTable::close(PieceTable::PT *T, string file_path) {
	if (!T) {
		cout << "Unable to close null piece table!\n";
		return;
	}
	string text = PieceTable::stitch(T);
	ofstream out_file;
	out_file.open(file_path);
	out_file << text;
	out_file.close();
	
	/*
	for (PieceTable::P *piece: T->pieces) {
		free(piece);
	}
	free(T->cursor);
	*/
}

void PieceTable::print_table(PieceTable::PT *T) {
	if (!T) {
		cout << "unable to print null piece table\n";
		return;
	}
	cout << "/---------- Printing piece table -----------\\\n";
	PieceTable::P *piece;
	char c;
	for (size_t i = 0; i < T->pieces.size(); i++) {
		piece = T->pieces[i];
		if (piece->src == PieceTable::ORIGINAL) {
			c = T->original.at( piece->start);
		} else {
			c = T->add.at(piece->start);
		}
		cout << "\tP[" << i << "]:={" << piece->src << ", start=" << piece->start << ", len=" << piece->len << ", c="<< c<<"}\n";
	}
	PieceTable::print_cursor(T->cursor);
	piece = T->pieces[T->cursor->piece_idx];
	if (piece->src == PieceTable::ORIGINAL) {
		c = T->original.at(piece->start + T->cursor->piece_offset);
	} else {
		c = T->add.at(piece->start + T->cursor->piece_offset);
	}

	cout << "Char at cursor =" << c << "\n"; 
	cout << "\\------------- End piece table -------------/\n";
	
}
