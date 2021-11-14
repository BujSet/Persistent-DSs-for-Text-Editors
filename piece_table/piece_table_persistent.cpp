#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <vector>

#include "piece_table_persistent.h"

using namespace std;

void PieceTable::create(pobj::pool<PieceTable::root> pop, string file_path) {
	auto r = pop.root();
	pobj::persistent_ptr<piece_table> ptable;
	pobj::persistent_ptr<piece> p;
	pobj::persistent_ptr<cursor> c;
	ifstream in_file;		

	if (access(file_path.c_str(), F_OK) != 0) {
		pop = pmem::obj::pool<PieceTable::root>::create(file_path, DEFAULT_LAYOUT, PMEMOBJ_MIN_POOL);		
		
		pobj::transaction::run(pop, [&]{
			r->root_piece_table = pobj::make_persistent<PieceTable::piece_table>();
			(r->root_piece_table)->cursor_pt = pobj::make_persistent<PieceTable::cursor>(0);
		});
	}
	else {
		pop = pmem::obj::pool<PieceTable::root>::open(file_path, DEFAULT_LAYOUT);
	}

	ptable = r->root_piece_table;
	c = ptable->cursor_pt;
	PieceTable::piece_vector_type &pvector = *(ptable->pieces);

	pobj::transaction::run(pop, [&]{
		p = pobj::make_persistent<PieceTable::piece>();

		in_file.open(file_path);
		stringstream strStream;
		strStream << in_file.rdbuf();
		
		ptable->original = strStream.str();

		p->src = ORIGINAL;
		p->start = 0;
		p->len = ptable->original.length();
		pvector.push_back(*p);

		c->pos = 0;
		c->piece_idx = 0;
		c->piece_offset = 0;
		ptable->cursor_pt = c;
	});
}

string PieceTable::stitch(pobj::pool<PieceTable::root> pop) {
	auto r = pop.root();
	if (r->root_piece_table == NULL) {
		cout << "Unable to read null piece table!\n";
		return "";
	}

	pobj::persistent_ptr<piece_table> ptable = r->root_piece_table;
	pobj::persistent_ptr<piece> p;
	PieceTable::piece_vector_type &pvector = *(ptable->pieces);
	string ret;

	pobj::transaction::run(pop, [&]{
		for (int i = 0; i < pvector.size(); i++) {
			p = &pvector[i];
			assert(p);
			if (p->src == PieceTable::ORIGINAL) {
				ret.append(ptable->original.c_str(), p->start, p->len);
			} 
			else {
				assert(p->src == PieceTable::ADD);
				ret.append(ptable->add.c_str(), p->start, p->len);
			}
		}
	});
	return ret;
}

void PieceTable::insert(pobj::pool<PieceTable::root> pop, string s) {
	auto r = pop.root();
	if (r->root_piece_table == NULL) {
		cout << "Unable to read null piece table!\n";
		return;
	}
	
	pobj::persistent_ptr<piece_table> ptable = r->root_piece_table;
	pobj::persistent_ptr<piece> p, piece, post;
	pobj::persistent_ptr<cursor> c;
	PieceTable::piece_vector_type &pvector = *(ptable->pieces);

	pobj::transaction::run(pop, [&]{
		// First we create a piece to represent this addition
		p = pobj::make_persistent<PieceTable::piece>();
		if (!p) {
			cout << "Unable to malloc new piece for insertion!\n";
			return;
		}
		p->src = PieceTable::ADD;
		p->start = ptable->add.length();
		p->len = s.length();

		// Next we place the string into the add buffer
		ptable->add.append(s);
		
		c = ptable->cursor_pt;
		piece = &pvector[c->piece_idx];

		if (c->piece_offset == 0) {
			// Need to insert the piece before cursor piece
			pvector.insert(pvector.begin() + c->piece_idx, p);
			c->pos += p->len;
			c->piece_idx++;
		} 
		else if (c->piece_offset == piece->len - 1) {
			// Need to insert the piece after cursor piece
			c->piece_idx++;
			pvector.insert(pvector.begin() + c->piece_idx, p);
			c->pos += piece->len;
		} 
		else {
			// Need to split the piece at cursor
			post = pobj::make_persistent<PieceTable::piece>();
			
			post->src = piece->src;
			post->start = piece->start + c->piece_offset;
			post->len = piece->len - c->piece_offset;
			piece->len = c->piece_offset;
			c->piece_idx++;
			c->pos += p->len;
			pvector.insert(pvector.begin() + c->piece_idx, post);
			pvector.insert(pvector.begin() + c->piece_idx, p);
			c->piece_idx++;
			c->piece_offset = 0;
		}
	});

}

void PieceTable::remove(pobj::pool<PieceTable::root> pop, size_t len) {
	auto r = pop.root();
	if (r->root_piece_table == NULL) {
		cout << "Unable to read null piece table!\n";
		return;
	}
	if (len == 0) {
		cout << "Nothing to insert!\n";
		return;
	}

	pobj::persistent_ptr<piece_table> ptable = r->root_piece_table;
	pobj::persistent_ptr<piece> piece, post;
	pobj::persistent_ptr<cursor> c;
	PieceTable::piece_vector_type &pvector = *(ptable->pieces);

	pobj::transaction::run(pop, [&]{
		size_t bytes_to_remove = len;

		while (bytes_to_remove) {
			c = ptable->cursor_pt;
			piece = &pvector[c->piece_idx];

			if (c->piece_offset == 0) {
				if (bytes_to_remove >= piece->len) {
					bytes_to_remove -= piece->len;
					pvector.erase(pvector.begin() + c->piece_idx);
					pobj::delete_persistent<PieceTable::piece>(piece);
				} else {
					piece->start += bytes_to_remove; 
					piece->len -= bytes_to_remove;
					bytes_to_remove = 0;
				}
			} 
			else { 
				// Need to break up the cursor piece
				if (bytes_to_remove > (piece->len - c->piece_offset)) {
					bytes_to_remove -= (piece->len - c->piece_offset);
					piece->len = c->piece_offset;
					c->piece_idx++;
					c->piece_offset = 0;
				} else {
					if (bytes_to_remove == (piece->len - c->piece_offset)) {
						piece->len -= bytes_to_remove;
						bytes_to_remove = 0;
					}
					else {
						post = pobj::make_persistent<PieceTable::piece>();

						post->src = piece->src;
						post->start = piece->start + bytes_to_remove + c->piece_offset;
						post->len = piece->len - post->start;
						piece->len = c->piece_offset;
						bytes_to_remove = 0;
						c->piece_idx++;
						c->piece_offset = 0;
						pvector.insert(pvector.begin()+c->piece_idx, post);
					}
				}
			}
		}
	});
}

int PieceTable::get_cursor_pos(pobj::pool<PieceTable::root> pop) {
	auto r = pop.root();
	if (r->root_piece_table == NULL) {
		cout << "Cannot find cursor on a null table\n";
		return 0;
	}
	return (r->root_piece_table)->cursor_pt->pos;
}

void PieceTable::print_cursor(pobj::pool<PieceTable::root> pop) {
	auto r = pop.root();
	pobj::persistent_ptr<piece_table> ptable = r->root_piece_table;
	pobj::persistent_ptr<cursor> c = ptable->cursor_pt;

	pobj::transaction::run(pop, [&]{
		cout << "Cursor:={pos=" << c->pos << ", piece_idx=" << c->piece_idx << ", piece_offset=" 
		<< c->piece_offset << "}\n";
	});
}

void PieceTable::seek(pobj::pool<PieceTable::root> pop, size_t offset, PieceTable::SeekDir dir) {
	if (offset == 0) {
		return;
	}

	auto r = pop.root();
	if (r->root_piece_table == NULL) {
		cout << "Cannot move cursor on null piece table!\n";
		return;
	}

	pobj::persistent_ptr<piece_table> ptable = r->root_piece_table;
	pobj::persistent_ptr<piece> piece;
	pobj::persistent_ptr<cursor> c = ptable->cursor_pt;
	PieceTable::piece_vector_type &pvector = *(ptable->pieces);

	pobj::transaction::run(pop, [&]{
		size_t bytes_moved = 0, bytes_to_move;

		while (bytes_moved < offset) {
			bytes_to_move = offset - bytes_moved;
			piece = &pvector[c->piece_idx];
			if (dir==PieceTable::FWD) {
				if ((piece->len - c->piece_offset) > bytes_to_move) {
					c->pos += bytes_to_move;
					c->piece_offset += bytes_to_move;
					break;
				}
				assert((piece->len - c->piece_offset) <= bytes_to_move);
				c->pos += piece->len - c->piece_offset;
				c->piece_idx++;
				c->piece_offset = 0;
				bytes_moved += piece->len - c->piece_offset;
			} 
			else {
				assert(dir == PieceTable::BWD);
				if (c->piece_offset >= bytes_to_move) {
					c->pos -= bytes_to_move;
					c->piece_offset -= bytes_to_move;
					break;
				}
				assert(c->piece_offset <= bytes_to_move);
				c->pos -= c->piece_offset;
				c->piece_idx--;
				bytes_moved += c->piece_offset;
				c->piece_offset = pvector[c->piece_idx].len;
			}
		}	
	});
}

void PieceTable::rewind(pobj::pool<PieceTable::root> pop) {
	auto r = pop.root();
	if (r->root_piece_table == NULL) {
		cout << "Unbale to get character from null piece table\n";
		return;
	}

	pobj::persistent_ptr<piece_table> ptable = r->root_piece_table;

	ptable->cursor_pt->pos = 0;
	ptable->cursor_pt->piece_idx = 0;
	ptable->cursor_pt->piece_offset = 0;
}

void PieceTable::close(pobj::pool<PieceTable::root> pop, string file_path) {
	auto r = pop.root();
	if (r->root_piece_table == NULL) {
		cout << "Unable to close null piece table!\n";
		return;
	}

	pobj::persistent_ptr<piece_table> ptable = r->root_piece_table;
	pobj::persistent_ptr<piece> piece;
	pobj::persistent_ptr<cursor> c = ptable->cursor_pt;
	PieceTable::piece_vector_type &pvector = *(ptable->pieces);

	string text = PieceTable::stitch(pop);
	ofstream out_file;
	out_file.open(file_path);
	out_file << text;
	out_file.close();

	pobj::transaction::run(pop, [&]{
		for (int i = 0; i < pvector.size(); i++) {
			pobj::delete_persistent<PieceTable::piece>(&pvector[i]);

		}
		pobj::delete_persistent<PieceTable::cursor>(ptable->cursor_pt);
	});
}

void PieceTable::print_table(pobj::pool<PieceTable::root> pop) {
	auto r = pop.root();
	if (r->root_piece_table == NULL) {
		cout << "unable to print null piece table\n";
		return;
	}

	pobj::persistent_ptr<piece_table> ptable = r->root_piece_table;
	pobj::persistent_ptr<piece> piece;
	PieceTable::piece_vector_type &pvector = *(ptable->pieces);
	char c;

	pobj::transaction::run(pop, [&]{
		cout << "/---------- Printing piece table -----------\\\n";

		for (size_t i = 0; i < pvector.size(); i++) {
			piece = &pvector[i];
			if (piece->src == PieceTable::ORIGINAL) {
				c = ptable->original.at( piece->start);
			} 
			else {
				c = ptable->add.at(piece->start);
			}
			cout << "\tP[" << i << "]:={" << piece->src << ", start=" << piece->start << ", len=" << piece->len << ", c="<< c<<"}\n";
		}
		PieceTable::print_cursor(pop);
		piece = &pvector[ptable->cursor_pt->piece_idx];
		if (piece->src == PieceTable::ORIGINAL) {
			c = ptable->original.at(piece->start + ptable->cursor_pt->piece_offset);
		} else {
			c = ptable->add.at(piece->start + ptable->cursor_pt->piece_offset);
		}

		cout << "Char at cursor =" << c << "\n"; 
		cout << "\\------------- End piece table -------------/\n";
	});

}
