
#include <new>
#include <vector>

#undef max
#undef min
/// @brief 定长内存池实现
class CFixSizeMemoryPool
{
     typedef unsigned char byte_t;
public:
     CFixSizeMemoryPool(size_t nodeSize, size_t iniNodeCount, size_t nodeCountInBlock)
         : m_BlockCount(std::max((size_t)1U, nodeCountInBlock))
         , m_pFreeHead(NULL)
     {
         // 保证每个Block至少包含一个Node
         m_NodeSize = std::max(sizeof(byte_t*), nodeSize);
         // 分配初始内存
         if(iniNodeCount != 0)
         {
              const size_t c = iniNodeCount / m_BlockCount + (iniNodeCount % m_BlockCount != 0);
              for(size_t i = 0; i < c; i++)
                   AddNewBlock();
         }
     }
     ~CFixSizeMemoryPool()
     {
         for(std::vector<byte_t*>::const_iterator it = m_BlockAddr.begin(); it != m_BlockAddr.end(); ++it)
              delete[] (byte_t*)(*it);
         m_BlockAddr.clear();
     }
public:
     /// @brief 原生的内存分配。如果用于对象，请注意调用构造函数
     void* RawAlloc()
     {
         if(m_pFreeHead == NULL)
              AddNewBlock();
         byte_t* ret = m_pFreeHead;
         m_pFreeHead = *(byte_t**)m_pFreeHead;
         return ret;
     }
     template<typename T>
     T* Alloc()
     {
         return ::new(RawAlloc()) T();
     }
     template<typename T>
     void Free(T*& p)
     {
         byte_t* bp = (byte_t*)p;
         // 调用析构函数
         p->~T();
         // 将内存回收到内存池
         if(m_pFreeHead )
              *(byte_t**)m_pFreeHead = bp;
         m_pFreeHead = bp;
         p = NULL;
     }
public:
     /// @brief 该内存池可分配的每个节点的大小
     size_t NodeSize()const{return m_NodeSize;}
private:
     /// @brief 创建一个新的Block
     void AddNewBlock()
     {
         // 分配内存
         byte_t* pBuf = new byte_t[m_NodeSize * m_BlockCount];
         // 登记该次分配
         m_BlockAddr.push_back(pBuf);
         // 将新内存串成空闲链表
         byte_t* p = pBuf;
         for(size_t i = 0; i < m_BlockCount - 1; i++, p += m_NodeSize)
              *(byte_t**)p = (p + m_NodeSize);
         *(byte_t**)p = m_pFreeHead;
         // 更新空闲链头指针
         m_pFreeHead = pBuf;
     }
private:
     size_t m_NodeSize;                   ///< 每个内存Node的大小（字节）
     size_t m_BlockCount;                 ///< 每个Block中包含的Node的个数
 
     byte_t* m_pFreeHead;                 ///< 未使用内存的头指针
     std::vector<byte_t*> m_BlockAddr;    ///< 该池占用的所有Block的头指针
};
