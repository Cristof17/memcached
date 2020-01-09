run config with administrator rights
sudo ./config

compile profile.c with
	make

to write to memcached run:
	./profile write <keycount> <value_size>
to read from memcached run:
	./profile read <keycount> <read_batch_size>

to clean run
	make clean

