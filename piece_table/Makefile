
build:
	@g++ -o pt piece_table.cpp pt_driver.cpp -Wall
	@g++ -o pt_test piece_table.cpp pt_test.cpp -Wall
	@g++ -o pt_driver_persistent piece_table_persistent.cpp pt_driver_persistent.cpp -lpmemobj -lpmem -pthread -Wall
	@g++ -o pt_test_persistent piece_table_persistent.cpp pt_test_persistent.cpp -lpmemobj -lpmem -pthread -Wall
clean:
	@touch pt && rm -f pt pt_test pt_driver_persistent pt_test_persistent
	@rm -f init_read_pool init_read_pool_test 
	@rm -f init_read_vol.txt init_read_pers.txt init_read_vol_test.txt init_read_pers_test.txt
	@rm -f volatile_pt_char.csv volatile_pt_word.csv persistent_pt_char.csv persistent_pt_word.csv

