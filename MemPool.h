#include <new>
#include <stdlib.h>
#include "Alloc.h"
#include <stdarg.h>
#include <stdio.h>

template <class __malloc, class __free>
class MemPool
{
    private:
        __malloc *_malloc;
        __free *_free;
        typedef struct __MemBlock {
            size_t size;
            char* addr;
            int status;
            DestructorType fn;
            __MemBlock* pre;
            __MemBlock* next;
        }_MemBlock;

        typedef struct __BlockHeader {
            _MemBlock* block;
            char content;
        }_BlockHeader;

        typedef struct __MemHeader {
            size_t whole_size;
            size_t used_size;
            size_t block_count;
            size_t free_block_size;
            _MemBlock* used_block;
            _MemBlock* free_block;
            _MemBlock* leisure_block;
        }_MemHeader;

    private:
        const size_t whole_size;
        _MemHeader* header;
        char* block_header;
        char* block_end;

    public:
        MemPool(int size, __malloc &_malloc, __free &_free) 
            : _malloc(_malloc), _free(_free), whole_size(size)
        {
            header = (_MemHeader*)(*_malloc)(sizeof(_MemHeader) + whole_size);
            block_header = block_end = NULL;
        }

        ~MemPool()
        {
            clear();
            (*free)(header);
        }

    public:

        void Alloc()
        {
            if (block_header == NULL)
            {
                header->whole_size = whole_size;
                header->used_size = 0;
                header->block_count = 0;
                header->free_block_size = 0;
                header->used_block = NULL;
                header->free_block = NULL;
                header->leisure_block = NULL;
                block_header = (char*)(header + 1);
                block_end = block_header + whole_size;
            }
        }

        template <class Type>
            Type* New(size_t size)
            {
                Type* pAddr = Malloc<Type>(size);
                if (pAddr == NULL)
                    return NULL;
                get_block_header(pAddr)->block->fn = DestructorTraits<Type>::destruct;
                return ::new(pAddr) Type;
            }

        template <class Type>
            Type* Malloc(size_t size)
            {
                Alloc();
                if (header->free_block)
                    if (header->free_block->pre)
                        printf("1\n");
                if (header->used_block)
                    if (header->used_block->pre)
                        printf("0\n");
                if (header->leisure_block)
                    if (header->leisure_block->pre)
                        printf("2\n");

                size += sizeof(void*);
                char* pAddr;
                int count = 0;
                _MemBlock *temp, *theBlock = NULL, *addBlock;
                if (!  ((size < header->whole_size - header->used_size) 
                            || (size < block_end - block_header - (header->block_count + 1) * sizeof(_MemBlock))
                            || size < header->free_block_size
                            || ((sizeof(_MemBlock) > block_end - block_header - (header->block_count + 1) * sizeof(_MemBlock)) 
                                || !header->leisure_block)))
                    return NULL;

                if (header->leisure_block)
                    addBlock = header->leisure_block;
                else if ((size_t)(block_end - block_header) > (header->block_count + count) * sizeof(_MemBlock) + sizeof(_MemBlock))
                {
                    count ++;
                    header->block_count += count; 
                    addBlock = (_MemBlock*)(block_end - sizeof(_MemBlock) * header->block_count);
                }
                else return NULL;

                if ((size_t)(block_end - block_header) > (header->block_count + count) * sizeof(_MemBlock) + size)
                {
                    theBlock = temp = addBlock;
                    temp->addr = block_header;
                    ((_BlockHeader*)block_header)->block = temp;
                    block_header += size;
                }
                else
                {
                    clear_up();
                    temp = header->free_block;
                    while(temp)
                    {
                        if (temp->size < size)
                        {
                            temp = temp->next;
                            continue;
                        }
                        else if (temp->size == size)
                        {
                            break;
                        }
                        else 
                        {
                            theBlock = temp;
                            temp = addBlock;
                            temp->addr = theBlock->addr;
                            theBlock->size -= size;
                            theBlock->addr += size;
                            break;
                        }
                    }
                }
                if (!temp)
                    return NULL;
                else
                    get_out_block(temp);

                temp->size = size;
                temp->status = 1;
                temp->pre  = NULL;
                temp->next = header->used_block;
                temp->fn = NULL;
                if (header->used_block)
                    header->used_block->pre = temp;
                header->used_block = temp;
                header->used_size += size;
                pAddr = &((_BlockHeader*)temp->addr)->content;

                return (Type*)pAddr;
            }

        template <class Type>
            void Free(Type* &p)
            {
                if (!p)
                    return;
                _BlockHeader* temp = get_block_header(p);
                if (temp->block == NULL
                        || (_BlockHeader*)temp->block->addr != temp 
                        || temp->block->status == 0)
                    return;
                get_out_block(temp->block);
                temp->block->fn(p);
                temp->block->status = 0;
                temp->block->next = header->free_block;
                if (header->free_block)
                    header->free_block->pre = temp->block;
                header->free_block = temp->block;
                header->used_size -= temp->block->size;
                p = NULL;
            }

        void clear()
        {
            _MemBlock* head = header->used_block;
            while(head)
            {
                if (head->fn)
                    head->fn(head->addr + sizeof(_MemBlock*));
                head = head->next;
            }
            block_header = NULL;
        }

        size_t count()
        {
            return header->block_count;
        }

    private:

        _BlockHeader* get_block_header(void* pAddr)
        {
            return (_BlockHeader*)((char*)pAddr - sizeof(_MemBlock*));
        }

        void get_out_block(_MemBlock* &p)
        {
            if (!p->pre)
            {
                if (header->used_block == p)
                    header->used_block = p->next;
                else if (header->free_block == p)
                    header->free_block = p->next;
                else if (header->leisure_block == p)
                    header->leisure_block = p->next;
            }
            else 
            {
                p->pre->next = p->next;
            }

            if (p->next)
            {
                p->next->pre = p->pre;
            }
            p->pre = NULL;
            p->next = NULL;
        }

        void clear_up()
        {
            _MemBlock* head = header->free_block, *temp;
            while (head)
            {
                if (head->status != 0)
                {
                    printf("error\n");
                    break;
                }
                if ((char*)head == block_end - header->block_count * sizeof(_MemBlock))
                {
                    temp = head;
                    head = head->next;
                    get_out_block(temp);
                    header->block_count --;
                    continue;
                }
                if ((head->addr + head->size) == block_header)
                {
                    temp = head;
                    head = head->next;
                    get_out_block(temp);
                    temp->next = header->leisure_block;
                    if (header->leisure_block)
                        header->leisure_block->pre = temp;
                    header->leisure_block = temp;
                    head->size += temp->size;
                    block_header -= temp->size;
                    continue;
                }
                else if (((_BlockHeader*)(head->addr + head->size))->block->status == 0)
                {
                    get_out_block(((_BlockHeader*)(head->addr + head->size))->block);
                    ((_BlockHeader*)(head->addr + head->size))->block->next = header->leisure_block;
                    if (header->leisure_block)
                        header->leisure_block->pre = ((_BlockHeader*)(head->addr + head->size))->block;
                    header->leisure_block = ((_BlockHeader*)(head->addr + head->size))->block;
                    head->size += ((_BlockHeader*)(head->addr + head->size))->block->size;
                    continue;
                }
                if (head->size > header->free_block_size)
                    header->free_block_size = head->size;
                head = head->next;
            }
        }
};

typedef MemPool<void*(size_t), void(void*)> MemPoolSys;
