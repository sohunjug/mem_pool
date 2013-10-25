/*******************************************************************************
 * Author : huwq
 * Email : sohunjug@hotmail.com
 * Last modified : 2013-09-18 14:45
 * Filename : AutoFreeAlloc.h
 * Description : This file is similar to the stack memory management, 
 *               garbage collection of memory.
 *               You must not remove this notice, or any other, from the file.
 * *****************************************************************************/

#ifndef _AUTO_FREE_ALLOC_H_
#define _AUTO_FREE_ALLOC_H_

#include <new>
#include <stdlib.h>

#ifndef _MEMORY_BLOCK_SIZE_ 
#define _MEMORY_BLOCK_SIZE_ 2048
#endif

typedef void* (*pmalloc)(size_t);
typedef void  (*pfree)(void*);
typedef void __FnDestructor(void* data);
typedef __FnDestructor* DestructorType;

struct ArrayDestructHeader
{
	size_t count;
};

template <class Type>
struct ConstructorTraits
{
	static Type* constructArray(Type* array, size_t count)
	{
		for (size_t i = 0; i < count; ++i)
			new(array + i) Type;
		return array;
	}
};

template <class Type>
struct DestructorTraits
{
    static void destruct(void* data)
    {
        ((Type*)data)->~Type();
    }

    static void destructArray(void* array)
    {
        ArrayDestructHeader* hdr = (ArrayDestructHeader*)array;
        Type* data = (Type*)(hdr + 1);
        for (Type* dataEnd = data + hdr->count; data != dataEnd; ++data)
        	data->~Type();
    }
};

#define ALLOC_NEW(alloc,Type)                       ::new((alloc).Malloc(sizeof(Type), DestructorTraits<Type>::destruct)) Type
#define ALLOC_NEW_ARRAY(alloc, Type, count)         __newArray(alloc, (Type*)0, (count), DestructorTraits<Type>::destructArray

template <class AllocT, class Type>
inline Type* __newArray(AllocT& alloc, Type* zero, size_t count, DestructorType fn)
{
	ArrayDestructHeader* hdr = (ArrayDestructHeader*)alloc.Malloc(sizeof(ArrayDestructHeader) + sizeof(ArrayDestructHeader)*count, fn);
	hdr->count = count;

	return ConstructorTraits<Type>::constructArray((Type*)(hdr+1), count);
}

template <class _Alloc, int _MemBlockSize = _MEMORY_BLOCK_SIZE_>
class Alloc
{
    public:
        enum { MemBlockSize = _MemBlockSize };
        enum { HeaderSize = sizeof(void*) };
        enum { BlockSize = MemBlockSize - HeaderSize };
        enum { IsAutoFreeAlloctor = 1 };

    private:
        struct _MemBlock
        {
            _MemBlock* pPrev;
            char buffer[BlockSize];
        };

        struct _DestroyNode
        {
            _DestroyNode* pPrev;
            DestructorType fnDestroy;
        };

        char* m_begin;
        char* m_end;
        _DestroyNode* m_destroyChain;
        _Alloc* m_alloc;
        pmalloc _malloc;
        pfree _free;

    private:
        _MemBlock* _ChainHeader() const
        {
            return (_MemBlock*)(m_begin - HeaderSize);
        }

    public:

        Alloc() : m_destroyChain(NULL)
    {
        m_begin = m_end = (char*)HeaderSize;
        _malloc = malloc;
        _free = free;
    }

        Alloc(_Alloc* alloc) : m_destroyChain(NULL), m_alloc(alloc)
    {
        m_begin = m_end = (char*)HeaderSize;
        _malloc = m_alloc->Malloc;
        _free = m_alloc->Free;
    }

        Alloc(pmalloc _malloc, pfree _free) : m_destroyChain(NULL), _malloc(_malloc), _free(_free)
    {
        m_begin = m_end = (char*)HeaderSize;
    }

        ~Alloc()
        {
            clear();
        }

        void* Malloc(size_t cb)
        {
            if ((size_t)(m_end - m_begin) < cb)
            {
                if (cb >= BlockSize)
                {
                    _MemBlock* pHeader = _ChainHeader();
                    _MemBlock* pNew = (_MemBlock*)_malloc(HeaderSize + cb);
                    if (pHeader)
                    {
                        pNew->pPrev = pHeader->pPrev;
                        pHeader->pPrev = pNew;
                    }
                    else
                    {
                        m_end = m_begin = pNew->buffer;
                        pNew->pPrev = NULL;
                    }
                    return pNew->buffer;
                }
                else
                {
                    _MemBlock* pNew = (_MemBlock*)_malloc(sizeof(_MemBlock));
                    pNew->pPrev = _ChainHeader();
                    m_begin = pNew->buffer;
                    m_end = m_begin + BlockSize;
                }
            }
            return m_end -= cb;
        }

        void* Malloc(size_t cb, DestructorType fn)
        {
            _DestroyNode* pNode = (_DestroyNode*)Malloc(sizeof(_DestroyNode) + cb);
            pNode->fnDestroy = fn;
            pNode->pPrev = m_destroyChain;
            m_destroyChain = pNode;
            return pNode + 1;
        }

        /*template <class Type>
            class _Malloc
            {
                static void* __Malloc()
                {
                    return (::new(Malloc(sizeof(Type), DestructorTraits<Type>::destruct)) Type);
                }
            };
        template <class Type>
            void* Mallocc(Type *cb)
            {
                cb = (Type*)this->Malloc(sizeof(Type), DestructorTraits<Type>::destruct);
                ::new(cb) Type;
                return cb;
            }*/

        void clear()
        {
            while (m_destroyChain)
            {
                m_destroyChain->fnDestroy(m_destroyChain + 1);
                m_destroyChain = m_destroyChain->pPrev;
            }
            _MemBlock* pHeader = _ChainHeader();
            while (pHeader)
            {
                _MemBlock* pTemp = pHeader->pPrev;
                _free(pHeader);
                pHeader = pTemp;
            }
            m_begin = m_end = (char*)HeaderSize;
        }

};

#endif
