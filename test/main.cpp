//#include "../Alloc.h"
#include "../MemPool.h"
//#include "../AllocTest.h"
//#include <stdio.h>
#include <new>
#include <iostream>
#include <time.h>

int atime;

using namespace std;

class A
{
    public:
        A()
        {
            //aa = this;
            //printf("create\n");
            //if (atime % 100000 == 0)
            //    printf("create\n");
        }
        A(int a)
        {
            printf("create%d\n", a + 1);
        }
        ~A()
        {
            //printf("destory\n");
        }
        virtual void print()
        {
        }
        A* aa;
};

class B : public A
{
    public:
        B()
        {
            printf("create B\n");
        }
        ~B()
        {
            printf("destory B\n");
        }
        void print()
        {
            printf("%p   %p\n", aa, this);
        }
};

int main(int argc, char** argv)
{
    time_t start, end;
    atime = 0;

    int count = 100000;
    int ti = 100000;

    printf("Now test my memory pool!\n");

    start = time(NULL);
    MemPoolSys mem(20000 * count, malloc, free);

    end = time(NULL);
    printf("create memory_pool of %d: %.3f\n", 20000 * count, difftime(end, start));


    int k, m, n = 0, t = 0;

    start = time(NULL);
    A **aaa = mem.Malloc<A*>(sizeof(A*)*count);
    end = time(NULL);
    printf("create array of class A with %d: %.3f\n", count, difftime(end, start));
    A *aa;
    start = time(NULL);
    while (t < ti)
    {
        n = 0;
        for (int iLoop = 0; iLoop < count; iLoop++)
        {
            k = rand() % 2;
            if (k == 1 || aa == NULL)
            {
                aa = mem.New<A>(sizeof(A));
                aaa[n++] = aa;
            }
            if (n > 0)
            {
                m = rand() % n;
                mem.Free(aaa[m]);
            }
            if (n == count)
                break;

            //if (t == 554)
            //    printf("time:%d, %d, %ld\n", t, iLoop, mem.count());
            atime ++;
        }
        //printf("time:%d\n", t);
        t++;
        mem.clear();
    }
    end = time(NULL);
    printf("create or destory class A with random for %d and %d ti: %.3f\n", count, ti, difftime(end, start));



    atime = 0;
    printf("Now test system malloc!\n");
    start = time(NULL);
    aaa = (A**)malloc(sizeof(A*)*count);
    end = time(NULL);
    printf("create array of class A with %d: %.3f\n", count, difftime(end, start));
    n = 0;
    for (int iLoop = 0; iLoop < count; iLoop++)
    {
        aaa[n++] = NULL;
    }
    t = 0;
    start = time(NULL);
    while (t < ti)
    {
    n = 0;
        for (int iLoop = 0; iLoop < count; iLoop++)
        {
            k = rand() % 2;
            if (k == 1 || aa == NULL)
            {
                aa = new A;
                aaa[n++] = aa;
            }
            if (n > 0)
            {
                m = rand() % n;
                delete aaa[m];
                aaa[m] = NULL;
            }
            if (n == count)
                break;

            //printf("ti:%d, %ld\n", iLoop, mem.count());
            atime ++;
        }
        t++;
    n = 0;
    while (n < count)
    {
        if (aaa[n])
        {
            delete aaa[n];
            aaa[n] = NULL;
        }
        n ++;
    }
    }
    end = time(NULL);
    printf("create or destory class A with random for %d and %d ti: %.3f\n", count, ti, difftime(end, start));

    free(aaa);


    return 0;
}
