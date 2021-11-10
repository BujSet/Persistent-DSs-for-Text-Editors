#include <unistd.h>
#include <iostream>
#include <libpmemobj.h>
#include <libpmemobj++/transaction.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/p.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/container/string.hpp>

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

	typedef struct cursor_t {
		cursor_t(int posarg) {
			pos = posarg;
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

	typedef struct piece_table_t {
    	string_type original;
		string_type add;
		pobj::persistent_ptr<piece_vector_type> pieces;
		pobj::persistent_ptr<cursor> cursorPieceTable;
	} pieceTable;

	typedef struct root {
		pobj::persistent_ptr<pieceTable> rootPieceTable;
		pobj::persistent_ptr<cursor> rootCursor;
	} root;


	void create(pobj::pool<root> pop, string file_path);

	string stitch(pobj::pool<root> pop);

	void insert(pobj::pool<root> pop, string s);

	void remove(pobj::pool<root> pop, size_t len);

	int get_cursor_pos(pobj::pool<root> pop);

	void print_cursor(pobj::pool<root> pop);

	void seek(pobj::pool<root> pop, size_t offset, SeekDir dir);

	void rewind(pobj::pool<root> pop);

	void print_table(pobj::pool<root> pop);

	void close(pobj::pool<root> pop, string file_path);
}

#endif /* __PIECE_TABLE_H__ */
