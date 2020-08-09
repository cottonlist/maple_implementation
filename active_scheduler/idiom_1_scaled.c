#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "trigger.h"
#include "instrumenter.h"

pthread_mutex_t lock;

int a = 0;
int b = 0;
int c = 0;
int d = 0;

void *
func1(void *arg)
{
	inst_begin(1001, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 1;
	inst_end(1001);

	inst_begin(1002, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 2;
	inst_end(1002);

	inst_begin(1003, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 2;
	inst_end(1003);

	inst_begin(1004, VAR, &b, MODE_READ, NULL, 0, 0);
	if (b == 1)
	{
		crash();
	}
	inst_end(1004);

	inst_begin(1005, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 1;
	inst_end(1005);

	inst_begin(1006, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 1;
	inst_end(1006);

	inst_begin(1007, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 2;
	inst_end(1007);

	inst_begin(1008, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 2;
	inst_end(1008);

	inst_begin(1009, VAR, &b, MODE_READ, NULL, 0, 0);
	if (b == 1)
	{
		crash();
	}
	inst_end(1009);

	inst_begin(1010, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 1;
	inst_end(1010);

	inst_begin(1011, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 1;
	inst_end(1011);

	inst_begin(1012, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 2;
	inst_end(1012);

	inst_begin(1013, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 2;
	inst_end(1013);

	inst_begin(1014, VAR, &b, MODE_READ, NULL, 0, 0);
	if (b == 1)
	{
		crash();
	}
	inst_end(1014);

	inst_begin(1015, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 1;
	inst_end(1015);

	inst_begin(1016, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 1;
	inst_end(1016);

	inst_begin(1017, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 2;
	inst_end(1017);

	inst_begin(1018, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 2;
	inst_end(1018);

	inst_begin(1019, VAR, &b, MODE_READ, NULL, 0, 0);
	if (b == 1)
	{
		crash();
	}
	inst_end(1019);

	inst_begin(1020, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 1;
	inst_end(1020);

	inst_begin(1021, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 1;
	inst_end(1021);

	inst_begin(1022, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 2;
	inst_end(1022);

	inst_begin(1023, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 2;
	inst_end(1023);

	inst_begin(1024, VAR, &b, MODE_READ, NULL, 0, 0);
	if (b == 1)
	{
		crash();
	}
	inst_end(1024);

	inst_begin(1025, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 1;
	inst_end(1025);

	// inst_begin(1026, VAR, &a, MODE_WRITE, NULL, 0, 0);
	// a = 1;
	// inst_end(1026);

	// inst_begin(1027, VAR, &a, MODE_WRITE, NULL, 0, 0);
	// a = 2;
	// inst_end(1027);

	// inst_begin(1028, VAR, &b, MODE_WRITE, NULL, 0, 0);
	// b = 2;
	// inst_end(1028);

	// inst_begin(1029, VAR, &b, MODE_READ, NULL, 0, 0);
	// if (b == 1)
	// {
	// 	crash();
	// }
	// inst_end(1029);

	// inst_begin(1030, VAR, &c, MODE_WRITE, NULL, 0, 0);
	// c = 1;
	// inst_end(1030);

	// inst_begin(1006, SYNC, (int *)&lock, MODE_READ, &lock, 0, 0);
	// pthread_mutex_lock(&lock);
	// inst_end(1006);

	// inst_begin(1007, VAR, &d, MODE_WRITE, &lock, 1, 0);
	// d = 1;
	// inst_end(1007);

	// inst_begin(1008, VAR, &d, MODE_WRITE, &lock, 0, 1);
	// d = 2;
	// inst_end(1008);

	// inst_begin(1009, SYNC, (int *)&lock, MODE_READ, &lock, 0, 0);
	// pthread_mutex_unlock(&lock);
	// inst_end(1009);

	// inst_begin(1001);
	// a = 1;
	// inst_end(1001);
	// inst_begin(1002);
	// a = 2;
	// inst_end(1002);
	return NULL;
}

void *
func2 (void *arg)
{
	inst_begin(2001, VAR, &a, MODE_READ, NULL, 0, 0);
	if(a == 1)
	{
		crash();
	}
	inst_end(2001);

	inst_begin(2002, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 1;
	inst_end(2002);

	inst_begin(2003, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 2;
	inst_end(2003);

	inst_begin(2004, VAR, &c, MODE_READ, NULL, 0, 0);
	if (c == 1)
	{
		crash();
	}
	inst_end(2004);

	inst_begin(2005, VAR, &a, MODE_READ, NULL, 0, 0);
	if(a == 1)
	{
		crash();
	}
	inst_end(2005);

	inst_begin(2006, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 1;
	inst_end(2006);

	inst_begin(2007, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 2;
	inst_end(2007);

	inst_begin(2008, VAR, &c, MODE_READ, NULL, 0, 0);
	if (c == 1)
	{
		crash();
	}
	inst_end(2008);

	inst_begin(2009, VAR, &a, MODE_READ, NULL, 0, 0);
	if(a == 1)
	{
		crash();
	}
	inst_end(2009);

	inst_begin(2010, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 1;
	inst_end(2010);

	inst_begin(2011, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 2;
	inst_end(2011);

	inst_begin(2012, VAR, &c, MODE_READ, NULL, 0, 0);
	if (c == 1)
	{
		crash();
	}
	inst_end(2012);

	inst_begin(2013, VAR, &a, MODE_READ, NULL, 0, 0);
	if(a == 1)
	{
		crash();
	}
	inst_end(2013);

	inst_begin(2014, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 1;
	inst_end(2014);

	inst_begin(2015, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 2;
	inst_end(2015);

	inst_begin(2016, VAR, &c, MODE_READ, NULL, 0, 0);
	if (c == 1)
	{
		crash();
	}
	inst_end(2016);

	inst_begin(2017, VAR, &a, MODE_READ, NULL, 0, 0);
	if(a == 1)
	{
		crash();
	}
	inst_end(2017);

	inst_begin(2018, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 1;
	inst_end(2018);

	inst_begin(2019, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 2;
	inst_end(2019);

	inst_begin(2020, VAR, &c, MODE_READ, NULL, 0, 0);
	if (c == 1)
	{
		crash();
	}
	inst_end(2020);

	inst_begin(2021, VAR, &a, MODE_READ, NULL, 0, 0);
	if(a == 1)
	{
		crash();
	}
	inst_end(2021);

	inst_begin(2022, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 1;
	inst_end(2022);

	inst_begin(2023, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 2;
	inst_end(2023);

	inst_begin(2024, VAR, &c, MODE_READ, NULL, 0, 0);
	if (c == 1)
	{
		crash();
	}
	inst_end(2024);

	// inst_begin(2005, SYNC, (int *)&lock, MODE_READ, &lock, 0, 0);
	// pthread_mutex_lock(&lock);
	// inst_end(2005);

	// inst_begin(2006, VAR, &d, MODE_READ, &lock, 1, 1);
	// if (d == 1)
	// {
	// 	crash();
	// }
	// inst_end(2006);

	// inst_begin(2007, SYNC, (int *)&lock, MODE_READ, &lock, 0, 0);
	// pthread_mutex_unlock(&lock);
	// inst_end(2007);

	// inst_begin(2001);
	// if (a==1) {
	// 	crash();
	// }
	// inst_end(2001);
	return NULL;
}

int
main(void)
{	

	pthread_mutex_init(&lock, NULL);
	// run profiler
	activate_profiler();
	pthread_t t1;
	pthread_t t2;
	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	create_exec_order(1);
	activate_scheduler();

	// run active scheduler
	test(t1, t2, func1, func2);

	pthread_mutex_destroy(&lock);

	exit(0);
}
