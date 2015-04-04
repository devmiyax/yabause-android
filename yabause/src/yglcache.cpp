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

  return 1;
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
    GLuint _vertexBuffer;
    void * _pMapBuffer;
    int _initsize;

    unsigned char * m_pMemBlock;                //The address of memory pool.
    unsigned long m_ulBlockSize;

    void * prepos;

    unsigned long currentpos;
    unsigned long tc_startpos;
    unsigned long va_startpos;

public:
    CVboPool(unsigned long size);
    ~CVboPool();

    int alloc(unsigned long size, void ** vpos, void ** tcpos, void ** vapos  ); //Allocate memory unit
    int expand( unsigned long addsize,void ** vpos, void **tcpos, void **vapos  );                                   //Free memory unit

    void unMap();
    void reMap();
	GLuint getVboId(){ return _vertexBuffer; }
    intptr_t getOffset( void* address ); //{ return address-(intptr_t)m_pMemBlock; }
};

CVboPool::CVboPool(unsigned long size )
{
    m_ulBlockSize = size * ( sizeof(int)*2 + sizeof(float)*4 + sizeof(float)*4 ) ;
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_ulBlockSize,NULL,GL_DYNAMIC_DRAW);
    m_pMemBlock = (unsigned char *)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_ulBlockSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    currentpos = 0;

    tc_startpos = size * ( sizeof(int)*2);
    va_startpos = tc_startpos + (size*sizeof(float)*4) ;

}

void CVboPool::unMap()
{
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glUnmapBuffer(GL_ARRAY_BUFFER);

}

void CVboPool::reMap()
{
    void *p;
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    p = glMapBufferRange(GL_ARRAY_BUFFER, 0, m_ulBlockSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    if( p != m_pMemBlock )
    {
        printf("???\n");
    }
    currentpos = 0;
    prepos = NULL;
}


CVboPool::~CVboPool()
{
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDeleteBuffers(1,&_vertexBuffer);
}

int CVboPool::alloc(unsigned long size, void ** vpos, void ** tcpos, void ** vapos  )
{
    if( (currentpos* ( sizeof(int)))  >= tc_startpos ||
        tc_startpos + (currentpos*sizeof(float)*2)  >= va_startpos ||
        va_startpos + (currentpos*sizeof(float)*2) >= m_ulBlockSize )
    {
        printf("bad alloc %X,%X,%d\n",prepos,*vpos,size);
        return -1;
    }

    *vpos = m_pMemBlock + (currentpos* ( sizeof(int)*2)) ;
    *tcpos = m_pMemBlock + tc_startpos + (currentpos*sizeof(float)*4) ;
    *vapos = m_pMemBlock + va_startpos + (currentpos*sizeof(float)*4) ;
    prepos = *vpos;
    currentpos += size;
//    printf("alloc %X,%X,%X,%X,%d,%d\n",*vpos,*tcpos,*vapos,prepos,size,currentpos);
    return 0;
}

int CVboPool::expand( unsigned long addsize,void ** vpos, void **tcpos, void **vapos  )
{
    if( (currentpos += addsize) >= tc_startpos || tc_startpos + currentpos + addsize >= va_startpos || va_startpos + currentpos + addsize >= m_ulBlockSize )
    {

        printf("bad expand %X,%X,%d\n",prepos,*vpos,addsize);
        return -1;
    }

    // OverWitten!
    if( *vpos != prepos )
    {
        int a=0;
        printf("bad expand %X,%X,%d\n",prepos,*vpos,addsize);
    }else{
        currentpos += addsize;
//        printf("expand %X,%X,%d,%d\n",prepos,*vpos,addsize,currentpos);
    }
}

intptr_t CVboPool::getOffset( void* address )
{
//    printf("getOffset %X-%X=%X\n",address,m_pMemBlock,(intptr_t)address-(intptr_t)m_pMemBlock);
    return (intptr_t)address-(intptr_t)m_pMemBlock;
}

CVboPool * g_pool;

int YglInitVertexBuffer( int initsize ) {
    g_pool = new CVboPool(initsize);
}

void YglDeleteVertexBuffer()
{
    delete g_pool;
}

int YglUnMapVertexBuffer() {
    g_pool->unMap();
    //printf("================= unMap ====================\n");
}

int YglMapVertexBuffer() {
    g_pool->reMap();
    //printf("================= reMap ====================\n");
}

int YglGetVertexBuffer( int size, void ** vpos, void **tcpos, void **vapos )
{
    return g_pool->alloc(size,vpos,tcpos,vapos);
}

int YglExpandVertexBuffer( int addsize, void ** vpos, void **tcpos, void **vapos )
{
    g_pool->expand(addsize,vpos,tcpos,vapos);
}

int YglUserDirectVertexBuffer()
{
	 glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int YglUserVertexBuffer()
{
	 glBindBuffer(GL_ARRAY_BUFFER, g_pool->getVboId() );
}

intptr_t YglGetOffset( void* address )
{
    return g_pool->getOffset(address);
}


}
