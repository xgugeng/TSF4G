#include "core/tlibc_hash.h"
#include "core/tlibc_timer.h"
#include "core/tlibc_mempool.h"
#include "core/tlibc_unzip.h"
#include "core/tlibc_util.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>

#include <sys/time.h>

#define HASH_KEY_LENGTH 128
typedef struct _test_hash_data_t
{
	tlibc_hash_head_t hash_head;
	char key[HASH_KEY_LENGTH];
	int data;
}test_hash_data_t;

#define hash_bucket_size 1024

static void test_hash()
{
	tlibc_hash_bucket_t buckets[hash_bucket_size];//定义桶的大小
	tlibc_hash_t hash_table;//定义hash表
	test_hash_data_t d0, d1, d2;
	tlibc_hash_head_t *pos;
	const tlibc_hash_head_t *pos_const;
	const test_hash_data_t *data;
	const tlibc_list_head_t *iter;
	const tlibc_list_head_t *iter_bucket;

	//准备数据
	d0.data = 0;
	snprintf(d0.key, HASH_KEY_LENGTH, "d0");

	d1.data = 10;
	snprintf(d1.key, HASH_KEY_LENGTH, "d1");

	d2.data = 200;
	snprintf(d2.key, HASH_KEY_LENGTH, "d2");

	//buckets内存不可释放掉
	tlibc_hash_init(&hash_table, buckets, hash_bucket_size);

	//insert
	//插入数据之后, key指向的内存不可以释放掉
	tlibc_hash_insert(&hash_table, d0.key, (uint32_t)strlen(d0.key), &d0.hash_head);
	tlibc_hash_insert(&hash_table, d1.key, (uint32_t)strlen(d1.key), &d1.hash_head);
	tlibc_hash_insert(&hash_table, d2.key, (uint32_t)strlen(d2.key), &d2.hash_head);

	pos_const = tlibc_hash_find_const(&hash_table, "d0", 2);
	if(pos_const != NULL)
	{
		data = TLIBC_CONTAINER_OF(pos_const, const test_hash_data_t, hash_head);
		printf("find d0 data: %d\n", data->data);
	}
	

	//遍历所有数据
	printf("\n");
	for(iter_bucket = hash_table.used_bucket_list.next; iter_bucket != &hash_table.used_bucket_list; iter_bucket = iter_bucket->next)
	{
		const tlibc_hash_bucket_t *bucket = TLIBC_CONTAINER_OF(iter_bucket, const tlibc_hash_bucket_t, used_bucket_list);
		for(iter = bucket->data_list.next; iter != &bucket->data_list; iter = iter->next)
		{
			data = TLIBC_CONTAINER_OF(iter, const test_hash_data_t, hash_head);
			printf("key %s, data %d\n", data->key, data->data);
		}
	}
	
	
	//删除
	printf("\n");
	pos = tlibc_hash_find(&hash_table, "d2", 2);
	if(pos != NULL)
	{
		data = TLIBC_CONTAINER_OF(pos, const test_hash_data_t, hash_head);
		printf("find d2 data: %d\n", data->data);


		tlibc_hash_remove(&hash_table, pos);
		pos_const = tlibc_hash_find_const(&hash_table, "d2", 2);
		if(pos_const == NULL)
		{
			data = TLIBC_CONTAINER_OF(pos_const, const test_hash_data_t, hash_head);
			printf("can not find d2\n");
		}
	}

	//清空hash表
	printf("\n");
	tlibc_hash_clear(&hash_table);
	pos_const = tlibc_hash_find_const(&hash_table, "d0", 2);
	if(pos_const == NULL)
	{
		printf("can not find d0\n");
	}
}

typedef struct _timer_data_t
{
	tlibc_timer_entry_t timer_entry;
	int data;	
}timer_data_t;

timer_data_t timer_db;
tlibc_timer_t timer;

#define TIMER_INTERVAL_MS 1000
static time_t get_current_ms()
{
	struct timeval tv;	
	gettimeofday(&tv, NULL);

	return tv.tv_sec*1000 + tv.tv_usec/1000;
}


static void timer_callback(const tlibc_timer_entry_t* header)
{
	timer_data_t *self = TLIBC_CONTAINER_OF(header, timer_data_t, timer_entry);
	printf("hello world %d!\n", self->data);
	timer_db.timer_entry.expires = tlibc_timer_jiffies(&timer) + TIMER_INTERVAL_MS;
	tlibc_timer_push(&timer, &timer_db.timer_entry);
}
time_t start_ms;
static void test_timer()
{	
	uint32_t count = 0;
	time_t start_ms = get_current_ms();

	tlibc_timer_init(&timer);
	timer_db.data = 123456;

	TIMER_ENTRY_BUILD(&timer_db.timer_entry, tlibc_timer_jiffies(&timer), timer_callback);	
	tlibc_timer_push(&timer, &timer_db.timer_entry);
	
	for(;;)
	{
		time_t current_ms = get_current_ms() - start_ms;
		int busy = FALSE;
		while(tlibc_timer_jiffies(&timer) <= current_ms)
		{
			if(tlibc_timer_tick(&timer) == E_TLIBC_NOERROR)
			{
				busy = TRUE;
			}
		}
		
		if(busy)
		{
			count = 0;
		}
		else
		{
			++count;
			if(count > 50)
			{
				sleep(1);
			}
		}
	}
}

typedef struct _unit_t
{
	int data;
	tlibc_mempool_entry_t entry;	
}unit_t;

#define MAX_UNIT_NUM 128
unit_t mem[MAX_UNIT_NUM];

static void test_mempool()
{
	tlibc_mempool_t mp;
	size_t i, j;
	unit_t *data_list[MAX_UNIT_NUM];
	size_t data_list_num = 0;
	int total;
	tlibc_list_head_t *iter;

	tlibc_mempool_init(&mp, unit_t, entry, (char*)mem, sizeof(unit_t), MAX_UNIT_NUM);

	data_list_num = 0;

	for(i = 0; i < MAX_UNIT_NUM; ++i)
	{
		if(rand() % 100 < 70)
		{
			assert(!tlibc_mempool_empty(&mp));
			assert(!tlibc_mempool_over(&mp));
			tlibc_mempool_alloc(&mp, unit_t, entry, data_list[data_list_num]);
			assert(data_list[data_list_num] != NULL);
			data_list[data_list_num]->data = (int)i;
			++data_list_num;
		}
		else if(data_list_num > 0)
		{
			int pos = rand() % (int)data_list_num;
			size_t id = tlibc_mempool_ptr2id(&mp, data_list[pos]);
			unit_t *addr = NULL;
			addr = (unit_t*)tlibc_mempool_id2ptr(&mp, id);
			assert(addr == data_list[pos]);

			tlibc_mempool_free(&mp, unit_t, entry, data_list[pos]);
			addr = (unit_t*)tlibc_mempool_id2ptr(&mp, id);
			assert(!tlibc_mempool_ptr_test(addr, entry, addr->entry.sn));

			for(j = (size_t)pos; j < data_list_num; ++j)
			{
				data_list[j] = data_list[j + 1];
			}
			--data_list_num;
		}		
	}
	
	total = 0;
	//遍历所有元素
	for(iter = mp.mempool_entry.used_list.next; iter != &mp.mempool_entry.used_list; iter = iter->next)
	{
		unit_t *unit, *data;
		size_t data_id, id;

		unit = TLIBC_CONTAINER_OF(iter, unit_t, entry.used_list);
		assert(tlibc_mempool_ptr_test(unit, entry, unit->entry.sn));
		id = tlibc_mempool_ptr2id(&mp, unit);
		assert(tlibc_mempool_id_test(&mp, id));
		data = (unit_t*)tlibc_mempool_id2ptr(&mp, id);
		data_id = tlibc_mempool_ptr2id(&mp, data);

		if(data_id != id)
		{
			assert(0);
		}

		if(unit != data)
		{
			assert(0);
		}

		printf("%d\n", unit->data);
		++total;
	}
	assert(total == data_list_num);
	assert(total == mp.used_list_num);
}

static void test_unzip()
{
	void *buff;
	uint32_t size_buf;
	tlibc_error_code_t err;
	tlibc_unzip_s uf;
	err = tlibc_unzip_init(&uf, "d:/1.xlsx");
	//unz64_s uf = unzOpen("d:/1.zip");

	err = tlibc_unzip_locate(&uf, "xl/workbook.xml");
	err = tlibc_unzip_open_current_file(&uf);
	size_buf = uf.cur_file_info.uncompressed_size;
	buff = malloc(size_buf);
	err = tlibc_read_current_file(&uf,buff, &size_buf);
	//buff里面装着xml， 解析下就好了……
	free(buff);
	err = tlibc_unzip_close_current_file (&uf);
	tlibc_unzip_fini(&uf);
}

int main()
{
	size_t t = 0x12345678;
	tlibc_size_to_little(t);
	tlibc_little_to_size(t);
	assert(t == 0x12345678);

	test_hash();

	test_timer();

	test_mempool();

	test_unzip();

	return 0;
}
