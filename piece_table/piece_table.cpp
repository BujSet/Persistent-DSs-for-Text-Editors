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

void PieceTable::remove(PieceTable::PT *T, size_t offset, size_t len) {
	if (!T) {
		cout << "Unable to remove string from null piece table\n";
		return;
	}
	//cout << "Removing " << len << " bytes from offset " << offset << "\n";

	size_t char_cnt = 0;
	size_t prev_start_char_cnt = 0, prev_end_char_cnt = 0;
	size_t rmv_start_idx;
	bool start_on_boundary = false;
	for (size_t i = 0; i < T->pieces.size(); i++) {
		rmv_start_idx = i;
		//cout << "offset=" << offset << " char_cnt=" << char_cnt << "\n";
		if (offset == char_cnt) {
			start_on_boundary = true;
			break;
		}
		char_cnt += T->pieces[i]->len;
		if (offset < char_cnt) {
			break;
		}
		prev_start_char_cnt = char_cnt;
	}

	char_cnt = 0;
	size_t rmv_end_idx;
	bool end_on_boundary = false;
	for (size_t i = 0; i < T->pieces.size(); i++) {
		rmv_end_idx = i;
		if (offset + len == char_cnt) {
			end_on_boundary = true;
			break;
		}
		char_cnt += T->pieces[i]->len;
		if (offset + len < char_cnt) {
			break;
		}
		prev_end_char_cnt = char_cnt;
	}

	//cout << "Need to remove text from pieces " << rmv_start_idx << " to " << rmv_end_idx << "\n";
	//cout << "start_on_boundary=" << start_on_boundary << " end_on_boundary=" << end_on_boundary << "\n";
	vector<PieceTable::P*>::iterator it_start = T->pieces.begin() + rmv_start_idx;
	vector<PieceTable::P*>::iterator it_end  = T->pieces.begin() + rmv_end_idx;
	if (start_on_boundary && end_on_boundary && rmv_start_idx == rmv_end_idx) {
		// removing a singular piece
		PieceTable::P *delem = T->pieces[rmv_start_idx];
		T->pieces.erase(it_start);
		free(delem);
	} else if (start_on_boundary && end_on_boundary && rmv_start_idx != rmv_end_idx) {
		// Removing multiple pieces but without any splits
		vector<PieceTable::P*> delems;
		for (size_t i = 0; i < T->pieces.size(); i++) {
			if (rmv_start_idx <= i && i <= rmv_end_idx) {
				delems.push_back(T->pieces[i]);
			}
		}
		T->pieces.erase(it_start, it_end);
		for (PieceTable::P *piece: delems) {
			free(piece);
		}
	} else if (start_on_boundary && !end_on_boundary && rmv_start_idx == rmv_end_idx) {
		// Need to split a single piece
		PieceTable::P *to_split = T->pieces[rmv_start_idx];
		to_split->start += len;
		to_split->len = to_split->len - len;
	} else if (start_on_boundary && !end_on_boundary && rmv_start_idx != rmv_end_idx) {
		// Need to remove multiple pieces and split the last one
		PieceTable::P *to_split = T->pieces[rmv_end_idx];
		to_split->start = to_split->start + (offset - prev_start_char_cnt);
		to_split->len += len;
		it_end = T->pieces.begin() + (rmv_end_idx - 1); // Don't remove the one we are splitting
		vector<PieceTable::P*> delems;
		for (size_t i = 0; i < T->pieces.size(); i++) {
			if (rmv_start_idx <= i && i <= rmv_end_idx - 1) {
				delems.push_back(T->pieces[i]);
			}
		}
		T->pieces.erase(it_start, it_end);
		for (PieceTable::P *piece: delems) {
			free(piece);
		}
	} else if (!start_on_boundary && end_on_boundary && rmv_start_idx == rmv_end_idx) {
		// Need to split a single piece
		PieceTable::P *to_split = T->pieces[rmv_start_idx];
		to_split->len = to_split->len - len; 
	} else if (!start_on_boundary && end_on_boundary && rmv_start_idx != rmv_end_idx) {
		// Need to remove multiple pieces and split the first one
		PieceTable::P *to_split = T->pieces[rmv_start_idx];
		to_split->len = to_split->len - len;
		it_start = T->pieces.begin() + (rmv_start_idx + 1); // Don't remove the one we are splitting
		vector<PieceTable::P*> delems;
		for (size_t i = 0; i < T->pieces.size(); i++) {
			if (rmv_start_idx + 1 <= i && i <= rmv_end_idx) {
				delems.push_back(T->pieces[i]);
			}
		}
		T->pieces.erase(it_start, it_end);
		for (PieceTable::P *piece: delems) {
			free(piece);
		}
	} else if (!start_on_boundary && !end_on_boundary && rmv_start_idx == rmv_end_idx) {
		// Need to split single picece int two

		PieceTable::P *post = T->pieces[rmv_start_idx];
		PieceTable::P *prev = (PieceTable::P *)malloc(sizeof(PieceTable::P));
		prev->src = post->src;
		prev->start = post->start;
		prev->len = offset - prev_start_char_cnt;
		post->start = prev->start + prev->len + len;
		post->len = post->len - (prev->len + len);
		T->pieces.insert(it_start, prev);
	} else {
		assert(!start_on_boundary);
		assert(!end_on_boundary);
		assert(rmv_start_idx != rmv_end_idx);
		it_start = T->pieces.begin() + (rmv_start_idx + 1); // Don't remove the one we are splitting
		it_end = T->pieces.begin() + (rmv_end_idx - 1);     // Don't remove the one we are splitting
		vector<PieceTable::P*> delems;
		for (size_t i = 0; i < T->pieces.size(); i++) {
			if (rmv_start_idx + 1 <= i && i <= rmv_end_idx - 1) {
				delems.push_back(T->pieces[i]);
			}
		}
		if (rmv_start_idx  + 1 <= rmv_end_idx -1 ){
			T->pieces.erase(it_start, it_end);
		}
		for (PieceTable::P *piece: delems) {
			free(piece);
		}
		PieceTable::P *prev = T->pieces[rmv_start_idx];
		PieceTable::P *post = T->pieces[rmv_end_idx];
		prev->len = offset - prev_start_char_cnt;
		size_t old_post_start = post->start;
		post->start = old_post_start + (offset - prev_end_char_cnt);
		post->len = post->len - (post->start - old_post_start);
	}
}

int PieceTable::get_cursor_pos(PieceTable::PT *T) {
	if (!T) {
		cout << "Cannot find cursor on a null table\n";
		return -1;
	}
	return T->cursor->pos;
}

void PieceTable::seek(PieceTable::PT *T, size_t offset, PieceTable::SeekDir dir) {
	if (offset == 0) {
		return;
	}

	if (!T) {
		cout << "Cannot move cursor on null piece table!\n";
		return;
	}
	
	cout << "Attempting to move cursor from " << T->cursor->pos << " by " << offset << " bytes in dir " << dir << "\n"; 

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
			if (C->piece_offset > bytes_to_move) {
				C->pos -= bytes_to_move;
				C->piece_offset -= bytes_to_move;
				break; // we're done seeking to new position
			}
			assert(C->piece_offset <= bytes_to_move);
			C->pos -= C->piece_offset;
			C->piece_idx--;
			C->piece_offset = T->pieces[C->piece_idx]->len;
			bytes_moved += C->piece_offset;
		}
	}
	
}

char PieceTable::getc(PieceTable::PT *T) {
	if (!T) {
		cout << "Unbale to get character from null piece table\n";
		return -1;
	}
	PieceTable::Cursor *C = T->cursor;
	PieceTable::P *piece = T->pieces[C->piece_idx];
	char c;
	if (piece->src == PieceTable::ORIGINAL) {
		c = T->original.at(C->piece_offset);
	} else {
		assert(piece->src == PieceTable::ADD);
		c = T->add.at(C->piece_offset);
	}
	// Now move cursor
	PieceTable::seek(T, 1, PieceTable::FWD);
	return c;
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

	for (PieceTable::P *piece: T->pieces) {
		free(piece);
	}
	free(T->cursor);
}
