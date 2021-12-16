#include <unistd.h>
#include <iostream>
#include <libpmemobj.h>
#include <libpmemobj++/transaction.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/p.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/container/string.hpp>
#include <valgrind/pmemcheck.h>

#include <string>
#include <vector>

using namespace std;
namespace pobj = pmem::obj;

#ifndef __PIECE_TABLE_H__
#define __PIECE_TABLE_H__

#define DEFAULT_LAYOUT "DEFAULT_LAYOUT"

namespace PieceTable {
	enum SourceBuffer {ORIGINAL, ADD};
	enum SeekDir {FWD=1, BWD=-1};

	using string_type = pobj::string;

	typedef struct cursor {
		cursor(int posarg) {
			pos = posarg;
		}

		cursor(){			
		}

		pobj::p<size_t> pos;
		pobj::p<size_t> piece_idx;
		pobj::p<size_t> piece_offset;
	} cursor;

	typedef struct piece {
		pobj::p<SourceBuffer> src;
		pobj::p<size_t> start;
		pobj::p<size_t> len;
	} piece;

	using piece_vector_type = pobj::vector<piece>;

	typedef struct piece_table {
    	pobj::persistent_ptr<string_type> original;
		pobj::persistent_ptr<string_type> add;
		pobj::persistent_ptr<piece_vector_type> pieces;
		pobj::persistent_ptr<cursor> cursor_pt;
		pobj::p<int> add_start;
	} piece_table;

	typedef struct root {
		pobj::persistent_ptr<piece_table> root_piece_table;
	} root;


	void create(pobj::pool<PieceTable::root> pop, string file_path);

	string stitch(pobj::pool<PieceTable::root> pop);

	void insert(pobj::pool<PieceTable::root> pop, string s);

	void remove(pobj::pool<PieceTable::root> pop, size_t bytes_to_remove);

	int get_cursor_pos(pobj::pool<PieceTable::root> pop);

	void print_cursor(pobj::pool<PieceTable::root> pop);

	void seek(pobj::pool<PieceTable::root> pop, size_t offset, SeekDir dir);

	void rewind(pobj::pool<PieceTable::root> pop);

	void print_table(pobj::pool<PieceTable::root> pop);

	void close(pobj::pool<PieceTable::root> pop, string file_path);
}

#endif /* __PIECE_TABLE_H__ */
