extern "C" {
#include "ygl.h"
#include "yui.h"
#include "vidshared.h"
}


#include <ext/hash_map>
using __gnu_cxx::hash_map;

hash_map< u32 , YglCache > g_TexHash;

extern "C" {

int YglIsCached(u32 addr, YglCache * c ) {

  hash_map< u32 , YglCache >::iterator pos =  g_TexHash.find(addr);
  if( pos == g_TexHash.end() )
  {
      return 0;
  }

  c->x=pos->second.x;
  c->y=pos->second.y;

  return 0;
}

//////////////////////////////////////////////////////////////////////////////

void YglCacheAdd(u32 addr, YglCache * c) {

   g_TexHash[addr] = *c;
}

//////////////////////////////////////////////////////////////////////////////

void YglCacheReset(void) {
   g_TexHash.clear();
}

//////////////////////////////////////////////////////////////////////////////



class CVboPool
{
private:
    //The purpose of the structure`s definition is that we can operate linkedlist conveniently
    struct _Unit                     //The type of the node of linkedlist.
    {
        struct _Unit *pPrev, *pNext;
    };

    GLuint _vertexBuffer;
    void * _pMapBuffer;
    int _initsize;

    void* m_pMemBlock;                //The address of memory pool.

    //Manage all unit with two linkedlist.
    struct _Unit*    m_pAllocatedMemBlock; //Head pointer to Allocated linkedlist.
    struct _Unit*    m_pFreeMemBlock;      //Head pointer to Free linkedlist.

    unsigned long    m_ulUnitNum; //Memory unit size. There are much unit in memory pool.
    unsigned long    m_ulUnitSize; //Memory unit size. There are much unit in memory pool.
    unsigned long    m_ulBlockSize;//Memory pool size. Memory pool is make of memory unit.

public:
    CVboPool(unsigned long lUnitNum = 50, unsigned long lUnitSize = 1024);
    ~CVboPool();

    void* Alloc(unsigned long ulSize, bool bUseMemPool = true); //Allocate memory unit
    void Free( void* p );                                   //Free memory unit

    void unMap();
    void reMap();
};

CVboPool::CVboPool(unsigned long ulUnitNum,unsigned long ulUnitSize) :
    m_pMemBlock(NULL), m_pAllocatedMemBlock(NULL), m_pFreeMemBlock(NULL),
    m_ulBlockSize(ulUnitNum * (ulUnitSize+sizeof(struct _Unit))),
    m_ulUnitSize(ulUnitSize)
{
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_ulBlockSize,NULL,GL_STREAM_DRAW);
    m_pMemBlock = glMapBufferRange(GL_ARRAY_BUFFER, 0, m_ulBlockSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    m_ulUnitNum = ulUnitNum;
    //m_pMemBlock = malloc(m_ulBlockSize);     //Allocate a memory block.

    if(NULL != m_pMemBlock)
    {
        for(unsigned long i=0; i<ulUnitNum; i++)  //Link all mem unit . Create linked list.
        {
            struct _Unit *pCurUnit = (struct _Unit *)( (char *)m_pMemBlock + i*(ulUnitSize+sizeof(struct _Unit)) );

            pCurUnit->pPrev = NULL;
            pCurUnit->pNext = m_pFreeMemBlock;    //Insert the new unit at head.

            if(NULL != m_pFreeMemBlock)
            {
                m_pFreeMemBlock->pPrev = pCurUnit;
            }
            m_pFreeMemBlock = pCurUnit;
        }
    }
}

void CVboPool::unMap()
{
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glUnmapBuffer(GL_ARRAY_BUFFER);

}

void CVboPool::reMap()
{
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    m_pMemBlock = glMapBufferRange(GL_ARRAY_BUFFER, 0, m_ulBlockSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    if(NULL != m_pMemBlock)
    {
        for(unsigned long i=0; i<m_ulUnitNum; i++)  //Link all mem unit . Create linked list.
        {
            struct _Unit *pCurUnit = (struct _Unit *)( (char *)m_pMemBlock + i*(m_ulUnitSize+sizeof(struct _Unit)) );

            pCurUnit->pPrev = NULL;
            pCurUnit->pNext = m_pFreeMemBlock;    //Insert the new unit at head.

            if(NULL != m_pFreeMemBlock)
            {
                m_pFreeMemBlock->pPrev = pCurUnit;
            }
            m_pFreeMemBlock = pCurUnit;
        }
    }
}


CVboPool::~CVboPool()
{
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDeleteBuffers(1,&_vertexBuffer);
}

void* CVboPool::Alloc(unsigned long ulSize, bool bUseMemPool)
{
    if(    ulSize > m_ulUnitSize || false == bUseMemPool ||
        NULL == m_pMemBlock   || NULL == m_pFreeMemBlock)
    {
        return NULL; //malloc(ulSize);
    }

    //Now FreeList isn`t empty
    struct _Unit *pCurUnit = m_pFreeMemBlock;
    m_pFreeMemBlock = pCurUnit->pNext;            //Get a unit from free linkedlist.
    if(NULL != m_pFreeMemBlock)
    {
        m_pFreeMemBlock->pPrev = NULL;
    }

    pCurUnit->pNext = m_pAllocatedMemBlock;

    if(NULL != m_pAllocatedMemBlock)
    {
        m_pAllocatedMemBlock->pPrev = pCurUnit;
    }
    m_pAllocatedMemBlock = pCurUnit;

    return (void *)((char *)pCurUnit + sizeof(struct _Unit) );
}

void CVboPool::Free( void* p )
{
    if(m_pMemBlock<p && p<(void *)((char *)m_pMemBlock + m_ulBlockSize) )
    {
        struct _Unit *pCurUnit = (struct _Unit *)((char *)p - sizeof(struct _Unit) );

        m_pAllocatedMemBlock = pCurUnit->pNext;
        if(NULL != m_pAllocatedMemBlock)
        {
            m_pAllocatedMemBlock->pPrev = NULL;
        }

        pCurUnit->pNext = m_pFreeMemBlock;
        if(NULL != m_pFreeMemBlock)
        {
             m_pFreeMemBlock->pPrev = pCurUnit;
        }

        m_pFreeMemBlock = pCurUnit;
    }
    else
    {
        //free(p);
    }
}

CVboPool * g_pool;

int YglInitVertexBuffer( int initsize ) {
    g_pool = new CVboPool( 1024, initsize/1024 );
}

void YglDeleteVertexBuffer()
{
    delete g_pool;
}

int YglUnMapVertexBuffer() {
    g_pool->unMap();
}

int YglMapVertexBuffer() {
    g_pool->reMap();
}

void * YglGetVertexBuffer( int size)
{
    return g_pool->Alloc(size);
}

int YglFreeVertexBuffer( void * p)
{
    g_pool->Free(p);
}


}
