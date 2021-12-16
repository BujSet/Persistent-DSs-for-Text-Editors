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

#ifndef __GAP_BUFFER_H__
#define __GAP_BUFFER_H__

#define DEFAULT_LAYOUT "DEFAULT_LAYOUT"

namespace GapBuffer {

	using char_vector_type = pobj::vector<char>;

	typedef struct gap_buffer {
		pobj::p<size_t> gap_size;
		pobj::p<size_t> gap_left;
		pobj::p<size_t> gap_right;
		pobj::p<size_t> size;
		
    	pobj::persistent_ptr<char_vector_type> buffer;

	} gap_buffer;

	typedef struct root {
		pobj::persistent_ptr<gap_buffer> root_gap_buffer;
	} root;


	void create(pobj::pool<GapBuffer::root> pop, string file_path);

	void initValues(pobj::persistent_ptr<GapBuffer::gap_buffer> gBuffer);

	void insert(pobj::pool<GapBuffer::root> pop, string input, int position);

	void deleteCharacter(pobj::pool<GapBuffer::root> pop, int position);

	void moveCursor(pobj::pool<GapBuffer::root> pop, int position);

	void left(pobj::pool<GapBuffer::root> pop, int position);

	void right(pobj::pool<GapBuffer::root> pop, int position);

	void grow(pobj::pool<GapBuffer::root> pop, int k, int position); 

	void print_buffer(pobj::pool<GapBuffer::root> pop);

	void close(pobj::pool<GapBuffer::root> pop, string file_path);
}

#endif /* __GAP_BUFFER_H__ */
