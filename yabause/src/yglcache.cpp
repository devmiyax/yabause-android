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

}
