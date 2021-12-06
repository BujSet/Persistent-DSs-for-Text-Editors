
build:
	@g++ -o pt piece_table.cpp pt_driver.cpp -Wall
	@g++ -o pt_test piece_table.cpp pt_test.cpp -Wall
	@g++ -o piece_table_persistent piece_table_persistent.cpp pt_driver_persistent.cpp -lpmemobj -lpmem -pthread
	@g++ -o pt_test_persistent piece_table_persistent.cpp pt_test_persistent.cpp -lpmemobj -lpmem -pthread
clean:
	@touch pt && rm -f pt pt_test piece_table_persistent pt_test_persistent

