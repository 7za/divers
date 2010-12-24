heap_pool_dbg: clean
	gcc heap_pool.c -o heap_pool -lrt -Wall -W -Wextra -DDBG -ggdb --coverage

heap_pool: clean
	gcc heap_pool.c -o heap_pool -lrt -Wall -W -Wextra

clean:
	rm -f heap_pool
