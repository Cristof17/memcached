#include <libmemcached/memcached.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

char* generate_val(int size) {
	if (size < 0) {
		return NULL;
	}
	char *value = (char*)malloc((size) * sizeof(char));
	int i = 0;
	srand(time(0));
	for (i = 0; i < size; ++i) {
		char random_char = (rand() % 25);
		random_char += 97;
		value[i] = random_char;
		
	}
	value[size-1] = '\0';
	return value;
}

char* generate_key(int pos) {
	if (pos < 0) {
		return NULL;
	}
	char *key = (char *)malloc(11 * sizeof(char));
	sprintf(key, "key_%06d", pos);
	return key;
}

void write(memcached_st *memc, int keycnt, int size) {

	if (memc == NULL) {
		return;
	}
	if (keycnt < 0) {
		return;
	}
	if (size < 0) {
		return;
	}

	int i = 0;
	memcached_return_t error;

	for (i = 0; i < keycnt; ++i) {
		char *key = generate_key(i);
		size_t key_len = strlen(key);
		char *val = generate_val(size);
		size_t val_len = size;
		//fprintf(stdout, "key = %s value = %s\n",
				//key, val);
		error = 
		memcached_set(memc, key, key_len, val, 
				val_len, 0, 0);
		if (error != 0) {
			fprintf(stdout, "%s\n", memcached_strerror(memc, error));
			fprintf(stdout, "error<%s, %s>\n",
				       	key, val);
			memcached_set(memc, key, key_len, 
					val, val_len, 0, 0);
		}
	}
}

void read(memcached_st *memc, int keycnt, int batch) {

	if (memc == NULL) {
		return;
	}

	if (keycnt < 0) {
		return;
	}

	if (batch < 0) {
		return;
	}

	memcached_return_t error;
	int i = 0;

	char return_key[MEMCACHED_MAX_KEY];
	size_t return_key_len;
	size_t return_val_len;
	uint32_t flags;
	char **keys = (char **)
		calloc(keycnt, sizeof(char *));
	size_t *key_len = (size_t *)
		malloc(keycnt * sizeof(size_t));

	for (i = 0; i < keycnt; ++i) {
		char *key = generate_key(i);
		key_len[i] = strlen(key);
		keys[i] = key;
	}

	/*
	for (i = 0; i < keycnt; ++i) {
		fprintf(stdout, "%s\n", keys[i]);
	}
	*/


	int slices = keycnt/batch;
	for (i = 0; i < slices; ++i) {

		char *val;
		if (i == slices - 1) {
			error = memcached_mget(memc,
				(const char * const *)keys,
				key_len,
				batch);
		} else {
			error = memcached_mget(memc,
				(const char * const *)keys,
				key_len,
				batch + (keycnt % batch));
			char *val;
		}

		if (error != 0){ 
			fprintf(stdout, "%s\n", 
				memcached_strerror(memc, error));
		}

		while ((val = memcached_fetch(memc, return_key,
						&return_key_len,
						&return_val_len,
						&flags,
						&error))) {
			/*
			fprintf(stdout,
				"read key: %s, val: %s\n",
				return_key,
				val);
				*/
			free(val);
		};
	}

	free(keys);
	free(key_len);
}	

int main(int argc, char **argv) {

	memcached_st *memc = memcached_create(NULL);
	memcached_return_t error;
	error = memcached_server_add(memc, "127.0.0.1", 0);

	if (argc < 2) {
		fprintf(stderr, "./profiler write|read <keycnt> <size>\n");
		return -1;
	}

	int is_read = 0;
	int is_write = 0;
	long keycnt = 0;
	long keysize = 0;
	long batch = 0;
	if ((strcmp(argv[1], (const char *)"write") == 0)) {
		is_write = 1;
 		keycnt = atol(argv[2]);
		keysize = atol(argv[3]);
		fprintf(stdout, "write keycnt = %ld size = %ld",
			       	keycnt,keysize);
	} else if ((strcmp(argv[1], (const char *)"read") == 0)) {
		is_read = 1;
		keycnt = atol(argv[2]);
		batch = atol(argv[3]);
		fprintf(stdout, "read keycnt = %ld batch = %ld\n"
				,keycnt,batch);
	}

	//char *key = generate_key(10);
	//char *val = generate_val(10);
//	fprintf(stdout, "%s\n", key);
//	fprintf(stdout, "%s\n", val);

	struct timeval *tv = (struct timeval*)
		malloc(sizeof(struct timeval) * sizeof(char));
	struct timezone *tz = (struct timezone*)
		malloc(sizeof(struct timezone) * sizeof(char));

	memset(tv, 0, sizeof(struct timeval));
	memset(tz, 0, sizeof(struct timezone));
	gettimeofday(tv, tz);
	
	uint64_t before = (uint64_t)tv->tv_sec * 1000000 + tv->tv_usec;

	if (is_write) {
		write(memc, keycnt, keysize);
	} else if (is_read) {
		read(memc, keycnt, batch);
	}

	memset(tv, 0, sizeof(struct timeval));
	memset(tz, 0, sizeof(struct timezone));
	gettimeofday(tv, tz);

	uint64_t after = (uint64_t)tv->tv_sec * 1000000 + tv->tv_usec;

	uint64_t diff = after - before;
	
	fprintf(stdout, "duration %lu\n", diff);

	free(tv);
	free(tz);

	memcached_free(memc);

	return 0;
}
