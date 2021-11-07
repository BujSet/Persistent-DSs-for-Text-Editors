#include <string>
#include <vector>
using namespace std;

#ifndef __PIECE_TABLE_H__
#define __PIECE_TABLE_H__

namespace PieceTable {


	enum SourceBuffer {ORIGINAL, ADD};

	enum SeekDir {FWD=1, BWD=-1};

	typedef struct cursor_t {
		size_t pos;
		size_t piece_idx;
		size_t piece_offset;
	} Cursor;

	typedef struct piece {
		SourceBuffer src;
		size_t start;
		size_t len;
	} P;

	typedef struct piece_table_t {
    		string original;
		string add;
		vector<P*> pieces;
		Cursor *cursor;
	} PT;



	void open(PT *T, string file_path);

	string stitch(PT *T);

	/* Inserts string of character to piece table */
	void insert(PT *T, string s, size_t offset);

	/* Deletes character from piece table */
	void remove(PT *T, size_t offset, size_t len);

	int get_cursor_pos(PT *T);

	void seek(PT *T, size_t offset, SeekDir dir);

	char getc(PT *T);

	void close(PT *T, string file_path);
}

#endif /* __PIECE_TABLE_H__ */
