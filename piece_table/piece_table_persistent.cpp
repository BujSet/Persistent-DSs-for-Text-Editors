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
	pobj::persistent_ptr<PieceTable::piece_table> ptable;
	PieceTable::piece p;
	pobj::persistent_ptr<PieceTable::cursor> c;
	ifstream in_file;

	pobj::transaction::run(pop, [&]{
		ptable = r->root_piece_table;		
		(ptable->pieces) = pobj::make_persistent<PieceTable::piece_vector_type>();
		PieceTable::piece_vector_type &pvector = *(ptable->pieces);

		in_file.open(file_path);
		
		if(!in_file){
		  	cout<<"Can't locate file!\n";
			return;
		}

		stringstream strStream;
		strStream << in_file.rdbuf();
		auto t1 = pobj::make_persistent<string_type>(strStream.str().c_str(), strlen(strStream.str().c_str()));
		ptable->original = t1;
		string tmp;
		for(int i=0;i<5000;i++){
			tmp+="-";
		}
		auto t2 = pobj::make_persistent<string_type>(tmp.c_str(), strlen(tmp.c_str()));
		ptable->add = t2;

		p.src = ORIGINAL;
		p.start = 0;
		p.len = ptable->original->size();
		pvector.push_back(p);

		ptable->add_start = 0;

		c = pobj::make_persistent<PieceTable::cursor>();
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

	pobj::persistent_ptr<PieceTable::piece_table> ptable = r->root_piece_table;
	pobj::persistent_ptr<PieceTable::piece> p;	
	string ret;

	pobj::transaction::run(pop, [&]{
		PieceTable::piece_vector_type &pvector = *(ptable->pieces);

		for (int i = 0; i < pvector.size(); i++) {
			p = &pvector[i];
			assert(p);
			if (p->src == PieceTable::ORIGINAL) {
				ret.append(ptable->original->c_str(), p->start, p->len);
			} 
			else {
				assert(p->src == PieceTable::ADD);
				ret.append(ptable->add->c_str(), p->start, p->len);
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
	
	pobj::persistent_ptr<PieceTable::piece_table> ptable = r->root_piece_table;
	PieceTable::piece p, piece, post;
	pobj::persistent_ptr<PieceTable::cursor> c;	

	pobj::transaction::run(pop, [&]{
		PieceTable::piece_vector_type &pvector = *(ptable->pieces);
		
		// First we create a piece to represent this addition
		p.src = PieceTable::ADD;
		p.start = ptable->add_start;
		p.len = s.size();
		ptable->add_start = s.size();

		// Next we place the string into the add buffer
		(ptable->add)->replace(p.start, p.len, s.c_str());
		// (ptable->add)->append(s.c_str());
		
		c = ptable->cursor_pt;
		piece = pvector[c->piece_idx];

		if (c->piece_offset == 0) {
			// Need to insert the piece before cursor piece
			pvector.insert(pvector.begin() + c->piece_idx, p);
			pvector[c->piece_idx] = p;
			c->pos += p.len;
			c->piece_idx++;
		} 
		else if (c->piece_offset == piece.len - 1) {
			// Need to insert the piece after cursor piece
			c->piece_idx++;
			pvector.insert(pvector.begin() + c->piece_idx, p);
			pvector[c->piece_idx] = p;
			c->pos += piece.len;
		} 
		else {
			// Need to split the piece at cursor
			post.src = piece.src;
			post.start = piece.start + c->piece_offset;
			post.len = piece.len - c->piece_offset;
			piece.len = c->piece_offset;
			c->piece_idx++;
			c->pos += p.len;
			pvector.insert(pvector.begin() + c->piece_idx, post);
			pvector.insert(pvector.begin() + c->piece_idx, p);
			pvector[c->piece_idx] = post;
			pvector[c->piece_idx] = p;
			c->piece_idx++;
			c->piece_offset = 0;
		}
	});

}

void PieceTable::remove(pobj::pool<PieceTable::root> pop, size_t bytes_to_remove) {
	auto r = pop.root();
	if (r->root_piece_table == NULL) {
		cout << "Unable to read null piece table!\n";
		return;
	}
	if (bytes_to_remove == 0) {
		cout << "Nothing to remove!\n";
		return;
	}

	pobj::persistent_ptr<PieceTable::piece_table> ptable = r->root_piece_table;
	pobj::persistent_ptr<PieceTable::piece> piece, post; 
	pobj::persistent_ptr<PieceTable::cursor> c;

	pobj::transaction::run(pop, [&]{
		PieceTable::piece_vector_type &pvector = *(ptable->pieces);

		while (bytes_to_remove) {
			c = ptable->cursor_pt;
			piece = &pvector[c->piece_idx];

			if (c->piece_offset == 0) {
				if (bytes_to_remove >= piece->len) {
					bytes_to_remove -= piece->len;
					pvector.erase(pvector.begin() + c->piece_idx);
					pobj::delete_persistent<PieceTable::piece>(piece);
					c->piece_idx--;
					c->piece_offset = 0;
					if(c->piece_idx < 0){
						cout<<"No more pieces left for cursor to move!\n";
						break;
					}
				}
				else {
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
				} 
				else {
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
						pvector.insert(pvector.begin() + c->piece_idx, *post);
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
	pobj::persistent_ptr<PieceTable::piece_table> ptable = r->root_piece_table;
	pobj::persistent_ptr<PieceTable::cursor> c = ptable->cursor_pt;

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

	pobj::persistent_ptr<PieceTable::piece_table> ptable = r->root_piece_table;
	PieceTable::piece piece;
	pobj::persistent_ptr<PieceTable::cursor> c = ptable->cursor_pt;
	size_t bytes_moved = 0, bytes_to_move;

	pobj::transaction::run(pop, [&]{		
		PieceTable::piece_vector_type &pvector = *(ptable->pieces);

		while (bytes_moved < offset) {
			bytes_to_move = offset - bytes_moved;
			piece = pvector[c->piece_idx];
			if (dir==PieceTable::FWD) {
				if ((piece.len - c->piece_offset) > bytes_to_move) {
					c->pos += bytes_to_move;
					c->piece_offset += bytes_to_move;
					break;
				}
				assert((piece.len - c->piece_offset) <= bytes_to_move);
				c->pos += piece.len - c->piece_offset;
				c->piece_idx++;
				c->piece_offset = 0;
				bytes_moved += piece.len - c->piece_offset;
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
		cout << "Unable to get character from null piece table\n";
		return;
	}

	pobj::persistent_ptr<PieceTable::piece_table> ptable = r->root_piece_table;

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

	pobj::persistent_ptr<PieceTable::piece_table> ptable = r->root_piece_table;
	pobj::persistent_ptr<PieceTable::piece> piece;
	pobj::persistent_ptr<PieceTable::cursor> c = ptable->cursor_pt;	

	string text = PieceTable::stitch(pop);
	ofstream out_file;
	out_file.open(file_path);
	out_file << text;
	out_file.close();

	pobj::transaction::run(pop, [&]{
		PieceTable::piece_vector_type &pvector = *(ptable->pieces);

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

	pobj::persistent_ptr<PieceTable::piece_table> ptable = r->root_piece_table;
	PieceTable::piece piece;	
	char c;

	pobj::transaction::run(pop, [&]{
		cout << "/---------- Printing piece table -----------\\\n";
		PieceTable::piece_vector_type &pvector = *(ptable->pieces);

		cout<<"ptable->original: "<<(ptable->original)->c_str()<<"\n";
		cout<<"ptable->add: "<<(ptable->add)->c_str()<<"\n";
		for (size_t i = 0; i < pvector.size(); i++) {
			piece = pvector[i];
			if (piece.src == PieceTable::ORIGINAL) {
				c = ptable->original->at(piece.start);
			} 
			else {
				c = ptable->add->at(piece.start);
			}
			cout << "\tPiece[" << i << "]:={" << piece.src << ", start=" << piece.start << 
			", len=" << piece.len << ", c="<< c<<"}\n";
		}
		// PieceTable::print_cursor(pop);
		
		// piece = pvector[ptable->cursor_pt->piece_idx];
		// if (piece.src == PieceTable::ORIGINAL) {
		// 	c = ptable->original->at(piece.start + ptable->cursor_pt->piece_offset);
		// } 
		// else {
		// 	c = ptable->add->at(piece.start + ptable->cursor_pt->piece_offset);
		// }

		// cout << "Char at cursor =" << c << "\n"; 
		// cout << "\\------------- End piece table -------------/\n";
	});
}
