/*******************************************************************************
 * Author : RKhuwq
 * Email : sohunjug@hotmail.com
 * Last modified : 2013-09-21 00:49
 * Filename : MemPool.h
 * Description :
 * *****************************************************************************/

#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

#include <new>

#ifndef _MEMORY_BLOCK_SIZE_ 
#define _MEMORY_BLOCK_SIZE_ 2048
#endif

template <class Type, const int m_nMemBlockSize = _MEMORY_BLOCK_SIZE_, const int m_nItemSize = 4>
class MemoryPool
{
    typedef unsigned char BYTE;

public:
    MemoryPool()
        : m_pMemBlockHeader(NULL), m_pFreeNodeHeader(NULL)
    {
    }
    ~MemoryPool()
    {
    }

    void* mallocc()
    {
        if (m_pFreeNodeHeader == NULL)
        {
            int nCount = m_nMemBlockSize / m_nItemSize;
            MemBlock *pNewBlock = new MemBlock;
            pNewBlock->data[0].pPrev = NULL;
            for (int i = 1; i < nCount; ++i)
                pNewBlock->data[i].pPrev = &pNewBlock->data[i - 1];
            m_pFreeNodeHeader = &pNewBlock->data[nCount - 1];
            pNewBlock->pPrev = m_pMemBlockHeader;
            m_pMemBlockHeader = pNewBlock;
        }
        void *pFreeNode = m_pFreeNodeHeader;
        m_pFreeNodeHeader = m_pFreeNodeHeader->pPrev;
        return pFreeNode;
    }

    void free(void *p)
    {
        FreeNode *pNode = (FreeNode *)p;
        pNode->pPrev = m_pFreeNodeHeader;
        m_pFreeNodeHeader = pNode;
    }
private:

    typedef struct _FreeNode
    {
        _FreeNode *pPrev;
        BYTE data[m_nItemSize - sizeof(void*)];
    }FreeNode;

    typedef struct _MemBlock
    {
        _MemBlock *pPrev;
        FreeNode data[m_nMemBlockSize / m_nItemSize];
    }MemBlock;

    MemBlock *m_pMemBlockHeader;
    FreeNode *m_pFreeNodeHeader;


};

#endif
