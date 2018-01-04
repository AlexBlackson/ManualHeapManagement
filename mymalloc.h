#define MALLOC my_malloc
#define FREE my_free
#define DUMP_HEAP() dump_heap()

void *my_malloc(int size);
void dump_heap();
void my_free(void *data);