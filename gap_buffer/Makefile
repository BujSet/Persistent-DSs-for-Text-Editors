
build:
	@g++ -o gb_test gb_test.cpp -Wall
	@g++ -o gb_driver_persistent gap_buffer_persistent.cpp gb_driver_persistent.cpp -lpmemobj -lpmem -pthread
	@g++ -o gb_test_persistent gap_buffer_persistent.cpp gb_test_persistent.cpp -lpmemobj -lpmem -pthread
clean:
	@rm -f gb_test gb_test_persistent