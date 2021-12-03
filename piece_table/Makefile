
build:
	@g++ -o pt piece_table.cpp pt_driver.cpp -Wall
	@g++ -o piece_table_persistent piece_table_persistent.cpp pt_driver_persistent.cpp -lpmemobj -lpmem -pthread
clean:
	@touch pt && rm -f pt

