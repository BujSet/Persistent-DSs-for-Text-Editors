
build:
	@g++ -o gb_test gb_test.cpp -Wall
	@g++ -o gb_driver_persistent gap_buffer_persistent.cpp gb_driver_persistent.cpp -lpmemobj -lpmem -pthread -Wall
	@g++ -o gb_test_persistent gap_buffer_persistent.cpp gb_test_persistent.cpp -lpmemobj -lpmem -pthread -Wall
clean:
	@rm -f gb_test gb_test_persistent
	@rm -f init_read_pers init_read_pers.txt init_read_pers_test.txt init_read_pool_test gap_buffer_in.txt_pers gb_driver_persistent
	@rm -f volatile_gb_char.csv volatile_gb_word.csv persistent_gb_char.csv persistent_gb_word.csv
