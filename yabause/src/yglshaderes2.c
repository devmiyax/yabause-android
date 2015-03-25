/*  Copyright 2005-2006 Guillaume Duhamel
    Copyright 2005-2006 Theo Berkau
    Copyright 2011 Shinya Miyamoto

    This file is part of Yabause.

    Yabause is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Yabause is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yabause; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifdef __ANDROID__
#include <stdlib.h>
#include <math.h>
#include "ygl.h"
#include "yui.h"
#include "vidshared.h"

#define printf xprintf

extern float vdp1wratio;
extern float vdp1hratio;
extern int GlHeight;
extern int GlWidth;

static void Ygl_printShaderError( GLuint shader )
{
  GLsizei bufSize;

  glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &bufSize);

  if (bufSize > 1) {
    GLchar *infoLog;

    infoLog = (GLchar *)malloc(bufSize);
    if (infoLog != NULL) {
      GLsizei length;
      glGetShaderInfoLog(shader, bufSize, &length, infoLog);
      printf("Shaderlog:\n%s\n", infoLog);
      free(infoLog);
    }
  }
}

static GLuint _prgid[PG_MAX] ={0};

/*------------------------------------------------------------------------------------
 *  Normal Draw
 * ----------------------------------------------------------------------------------*/
const GLchar Yglprg_normal_v[] =
      "uniform mat4 u_mvpMatrix;    \n"
      "uniform mat4 u_texMatrix;    \n"
      "attribute vec4 a_position;   \n"
      "attribute vec4 a_texcoord;   \n"
      "varying   vec4 v_texcoord;     \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position*u_mvpMatrix; \n"
      "   v_texcoord  = a_texcoord/*u_texMatrix*/; \n"
      "   v_texcoord.s  = v_texcoord.s/2048.0; \n"
      "   v_texcoord.t  = v_texcoord.t/1024.0; \n"
      "} ";
const GLchar * pYglprg_normal_v[] = {Yglprg_normal_v, NULL};

const GLchar Yglprg_normal_f[] =
      "precision highp float;                            \n"
      "varying vec4 v_texcoord;                            \n"
      "uniform sampler2D s_texture;                        \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  vec2 addr = v_texcoord.st;                        \n"
      "  vec4 txcol = texture2D( s_texture, addr );         \n"
      "  if(txcol.a > 0.0)\n                                 "
      "     gl_FragColor = txcol;\n                         "
      "  else \n                                            "
      "     discard;\n                                      "
      "}                                                   \n";
const GLchar * pYglprg_normal_f[] = {Yglprg_normal_f, NULL};

int Ygl_uniformNormal(void * p )
{
   YglProgram * prg;
   prg = p;
   glEnableVertexAttribArray(prg->vertexp);
   glEnableVertexAttribArray(prg->texcoordp);
   glUniform1i(glGetUniformLocation(_prgid[PG_NORMAL], (const GLchar *)"s_texture"), 0);
   return 0;
}

int Ygl_cleanupNormal(void * p )
{
   YglProgram * prg;
   prg = p;
   return 0;
}

int ShaderDrawTest()
{

   GLuint vertexp         = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_position");
   GLuint texcoordp       = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_texcoord");
   GLuint mtxModelView    = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_mvpMatrix");
   GLuint mtxTexture      = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_texMatrix");

   GLfloat vec[]={ 0.0f,0.0f,-0.5f, 100.0f,  0.0f,-0.5f,100.0f,100.0f,-0.5f,
                   0.0f,0.0f,-0.5f, 100.0f,100.0f,-0.5f,  0.0f,100.0f,-0.5f };

/*
   GLfloat vec[]={ 0.0f,0.0f,-0.5f,
                   -1.0f,1.0f,-0.5f,
                   1.0f,1.0f,-0.5f,
                   0.0f,0.0f,-0.5f,
                   -1.0f,-1.0f,-0.5f,
                   1.0f,-1.0f,-0.5f,
   };
*/
   GLfloat tex[]={ 0.0f,0.0f, 2048.0f,0.0f,2048.0f,1024.0f,0.0f,0.0f,2048.0f,1024.0f,0.0f,1024.0f };

//   GLfloat tex[]={ 0.0f,0.0f,1.0f,0.0f,1.0f,1.0f,
//                   0.0f,0.0f,1.0f,1.0f,0.0f,1.0f };

   YglMatrix mtx;
   YglMatrix mtxt;
   GLuint id = glGetUniformLocation(_prgid[PG_NORMAL], (const GLchar *)"s_texture");

   YglLoadIdentity(&mtx);
   YglLoadIdentity(&mtxt);

   YglOrtho(&mtx,0.0f,100.0f,100.0f,0.0f,1.0f,0.0f);

   glUseProgram(_prgid[PG_NORMAL]);
   glUniform1i(id, 0);

   glEnableVertexAttribArray(vertexp);
   glEnableVertexAttribArray(texcoordp);

   glUniformMatrix4fv( mtxModelView, 1, GL_FALSE, (GLfloat*) &_Ygl->mtxModelView/*mtx*/.m[0][0] );
   glUniformMatrix4fv( mtxTexture, 1, GL_FALSE, (GLfloat*) &_Ygl->mtxTexture.m[0][0] );

   glVertexAttribPointer(vertexp,3, GL_FLOAT,GL_FALSE, 0, (const GLvoid*) vec );
   glVertexAttribPointer(texcoordp,2, GL_FLOAT,GL_FALSE, 0, (const GLvoid*)tex );

   glDrawArrays(GL_TRIANGLES, 0, 6);

   return 0;

}

/*------------------------------------------------------------------------------------
 *  Window Operation
 * ----------------------------------------------------------------------------------*/
const GLchar Yglprg_window_v[] =
      "uniform mat4 u_mvpMatrix;    \n"
      "attribute vec4 a_position;   \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position*u_mvpMatrix; \n"
      "} ";
const GLchar * pYglprg_window_v[] = {Yglprg_window_v, NULL};

const GLchar Yglprg_window_f[] =
      "precision highp float;                            \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  gl_FragColor = vec4( 1.0,1.0,1.0,1.0 );\n"
      "}                                                   \n";
const GLchar * pYglprg_window_f[] = {Yglprg_window_f, NULL};

int Ygl_uniformWindow(void * p )
{
   YglProgram * prg;
   prg = p;
   glUseProgram(prg->prgid );
   glEnableVertexAttribArray(prg->vertexp);

   return 0;
}

int Ygl_cleanupWindow(void * p )
{
   YglProgram * prg;
   prg = p;
   return 0;
}

/*------------------------------------------------------------------------------------
 *  VDP1 Normal Draw
 * ----------------------------------------------------------------------------------*/
const GLchar Yglprg_vdp1_normal_v[] =
      "uniform mat4 u_mvpMatrix;    \n"
      "uniform mat4 u_texMatrix;    \n"
      "attribute vec4 a_position;   \n"
      "attribute vec4 a_texcoord;   \n"
      "varying   vec4 v_texcoord;     \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position*u_mvpMatrix; \n"
      "   v_texcoord  = a_texcoord/*u_texMatrix*/; \n"
      "   v_texcoord.s  = v_texcoord.s/2048.0; \n"
      "   v_texcoord.t  = v_texcoord.t/1024.0; \n"
      "} ";
const GLchar * pYglprg_vdp1_normal_v[] = {Yglprg_vdp1_normal_v, NULL};

const GLchar Yglprg_vpd1_normal_f[] =
      "precision highp float;                            \n"
      "varying vec4 v_texcoord;                            \n"
      "uniform sampler2D s_texture;                        \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  vec2 addr = v_texcoord.st;                        \n"
      "  addr.s = addr.s / (v_texcoord.q);                 \n"
      "  addr.t = addr.t / (v_texcoord.q);                 \n"
      "  gl_FragColor = texture2D( s_texture, addr );      \n"
      "  if( gl_FragColor.a == 0.0 ) discard;                \n"
      "}                                                   \n";
const GLchar * pYglprg_vdp1_normal_f[] = {Yglprg_vpd1_normal_f, NULL};

int Ygl_uniformVdp1Normal(void * p )
{
   YglProgram * prg;
   prg = p;
   glEnableVertexAttribArray(prg->vertexp);
   glEnableVertexAttribArray(prg->texcoordp);
   glUniform1i(glGetUniformLocation(_prgid[PG_NORMAL], (const GLchar *)"s_texture"), 0);
   return 0;
}

int Ygl_cleanupVdp1Normal(void * p )
{
   YglProgram * prg;
   prg = p;
   return 0;
}



/*------------------------------------------------------------------------------------
 *  VDP1 GlowShading Operation
 * ----------------------------------------------------------------------------------*/
const GLchar Yglprg_vdp1_gouraudshading_v[] =
      "uniform mat4 u_mvpMatrix;                \n"
      "uniform mat4 u_texMatrix;                \n"
      "attribute vec4 a_position;               \n"
      "attribute vec4 a_texcoord;               \n"
      "attribute vec4 a_grcolor;                \n"
      "varying   vec4 v_texcoord;               \n"
      "varying   vec4 v_vtxcolor;               \n"
      "void main() {                            \n"
      "   v_vtxcolor  = a_grcolor;              \n"
      "   v_texcoord  = a_texcoord/*u_texMatrix*/; \n"
      "   v_texcoord.s  = v_texcoord.s/2048.0; \n"
      "   v_texcoord.t  = v_texcoord.t/1024.0; \n"
      "   gl_Position = a_position*u_mvpMatrix; \n"
      "}\n";
const GLchar * pYglprg_vdp1_gouraudshading_v[] = {Yglprg_vdp1_gouraudshading_v, NULL};

const GLchar Yglprg_vdp1_gouraudshading_f[] =
      "precision highp float;                                                 \n"
      "uniform sampler2D u_sprite;                                              \n"
      "varying vec4 v_texcoord;                                                 \n"
      "varying vec4 v_vtxcolor;                                                 \n"
      "void main() {                                                            \n"
      "  vec2 addr = v_texcoord.st;                                             \n"
      "  addr.s = addr.s / (v_texcoord.q);                                      \n"
      "  addr.t = addr.t / (v_texcoord.q);                                      \n"
      "  vec4 spriteColor = texture2D(u_sprite,addr);                           \n"
      "  if( spriteColor.a == 0.0 ) discard;                                      \n"
      "  gl_FragColor   = spriteColor;                                          \n"
      "  gl_FragColor  += vec4(v_vtxcolor.r,v_vtxcolor.g,v_vtxcolor.b,0.0);     \n"
      "  gl_FragColor.a = spriteColor.a;                                        \n"
      "}\n";
const GLchar * pYglprg_vdp1_gouraudshading_f[] = {Yglprg_vdp1_gouraudshading_f, NULL};

int Ygl_uniformGlowShading(void * p )
{
   YglProgram * prg;
   prg = p;
   glEnableVertexAttribArray(prg->vertexp);
   glEnableVertexAttribArray(prg->texcoordp);
   glUniform1i(glGetUniformLocation(prg->prgid, (const GLchar *)"u_sprite"), 0);
   if( prg->vertexAttribute != NULL )
   {
      glEnableVertexAttribArray(prg->vaid);
      glVertexAttribPointer(prg->vaid,4, GL_FLOAT, GL_FALSE, 0, prg->vertexAttribute);
   }
   return 0;
}

int Ygl_cleanupGlowShading(void * p )
{
   YglProgram * prg;
   prg = p;
   glDisableVertexAttribArray(prg->vaid);
   return 0;
}

/*------------------------------------------------------------------------------------
 *  VDP1 GlowShading and Half Trans Operation
 * ----------------------------------------------------------------------------------*/
static int id_sprite;
static int id_fbo;
static int id_fbowidth;
static int id_fboheight;

const GLchar Yglprg_vdp1_gouraudshading_hf_v[] =
      "uniform mat4 u_mvpMatrix;                \n"
      "uniform mat4 u_texMatrix;                \n"
      "attribute vec4 a_position;               \n"
      "attribute vec4 a_texcoord;               \n"
      "attribute vec4 a_grcolor;                \n"
      "varying   vec4 v_texcoord;               \n"
      "varying   vec4 v_vtxcolor;               \n"
      "void main() {                            \n"
      "   v_vtxcolor  = a_grcolor;              \n"
      "   v_texcoord  = a_texcoord/*u_texMatrix*/; \n"
      "   v_texcoord.s  = v_texcoord.s/2048.0; \n"
      "   v_texcoord.t  = v_texcoord.t/1024.0; \n"
      "   gl_Position = a_position*u_mvpMatrix; \n"
      "}\n";
const GLchar * pYglprg_vdp1_gouraudshading_hf_v[] = {Yglprg_vdp1_gouraudshading_hf_v, NULL};

const GLchar Yglprg_vdp1_gouraudshading_hf_f[] =
      "precision highp float;                                                                     \n"
      "uniform sampler2D u_sprite;                                                                  \n"
      "uniform sampler2D u_fbo;                                                                     \n"
      "uniform int u_fbowidth;                                                                      \n"
      "uniform int u_fbohegiht;                                                                     \n"
      "varying vec4 v_texcoord;                                                                     \n"
      "varying vec4 v_vtxcolor;                                                                     \n"
      "void main() {                                                                                \n"
      "  vec2 addr = v_texcoord.st;                                                                 \n"
      "  vec2 faddr = vec2( gl_FragCoord.x/float(u_fbowidth), gl_FragCoord.y/float(u_fbohegiht));   \n"
      "  addr.s = addr.s / (v_texcoord.q);                                                          \n"
      "  addr.t = addr.t / (v_texcoord.q);                                                          \n"
      "  vec4 spriteColor = texture2D(u_sprite,addr);                                               \n"
      "  if( spriteColor.a == 0.0 ) discard;                                                          \n"
      "  vec4 fboColor    = texture2D(u_fbo,faddr);                                                 \n"
      "  spriteColor += vec4(v_vtxcolor.r,v_vtxcolor.g,v_vtxcolor.b,0.0);                           \n"
      "  if( fboColor.a > 0.0 && spriteColor.a > 0.0 )                                              \n"
      "  {                                                                                          \n"
      "    gl_FragColor = spriteColor*0.5 + fboColor*0.5;                                           \n"
      "    gl_FragColor.a = fboColor.a;                                                             \n"
      "  }else{                                                                                     \n"
      "    gl_FragColor = spriteColor;                                                              \n"
      "  }                                                                                          \n"
      "}\n";
const GLchar * pYglprg_vdp1_gouraudshading_hf_f[] = {Yglprg_vdp1_gouraudshading_hf_f, NULL};

int Ygl_uniformGlowShadingHalfTrans(void * p )
{
   YglProgram * prg;
   prg = p;
   glEnableVertexAttribArray(prg->vertexp);
   glEnableVertexAttribArray(prg->texcoordp);
   if( prg->vertexAttribute != NULL )
   {
      glEnableVertexAttribArray(prg->vaid);
      glVertexAttribPointer(prg->vaid,4, GL_FLOAT, GL_FALSE, 0, prg->vertexAttribute);
   }

   glUniform1i(id_sprite, 0);
   glUniform1i(id_fbo, 1);
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D,_Ygl->vdp1FrameBuff[_Ygl->drawframe]);
   glUniform1i(id_fbowidth, GlWidth);
   glUniform1i(id_fboheight, GlHeight);
   glActiveTexture(GL_TEXTURE0);
   return 0;
}

int Ygl_cleanupGlowShadingHalfTrans(void * p )
{
   YglProgram * prg;
   prg = p;
   glDisableVertexAttribArray(prg->vaid);
   return 0;
}


/*------------------------------------------------------------------------------------
 *  VDP1 Half Trans Operation
 * ----------------------------------------------------------------------------------*/
static int id_hf_sprite;
static int id_hf_fbo;
static int id_hf_fbowidth;
static int id_hf_fboheight;

const GLchar Yglprg_vdp1_halftrans_v[] =
      "uniform mat4 u_mvpMatrix;                \n"
      "uniform mat4 u_texMatrix;                \n"
      "attribute vec4 a_position;               \n"
      "attribute vec4 a_texcoord;               \n"
      "varying   vec4 v_texcoord;               \n"
      "void main() {                            \n"
      "   v_texcoord  = a_texcoord/*u_texMatrix*/; \n"
      "   v_texcoord.s  = v_texcoord.s/2048.0; \n"
      "   v_texcoord.t  = v_texcoord.t/1024.0; \n"
      "   gl_Position = a_position*u_mvpMatrix; \n"
      "}\n";
const GLchar * pYglprg_vdp1_halftrans_v[] = {Yglprg_vdp1_halftrans_v, NULL};

const GLchar Yglprg_vdp1_halftrans_f[] =
      "precision highp float;                                                                     \n"
      "uniform sampler2D u_sprite;                                                                  \n"
      "uniform sampler2D u_fbo;                                                                     \n"
      "uniform int u_fbowidth;                                                                      \n"
      "uniform int u_fbohegiht;                                                                     \n"
      "varying vec4 v_texcoord;                                                                     \n"
      "void main() {                                                                                \n"
      "  vec2 addr = v_texcoord.st;                                                                 \n"
      "  vec2 faddr = vec2( gl_FragCoord.x/float(u_fbowidth), gl_FragCoord.y/float(u_fbohegiht));   \n"
      "  addr.s = addr.s / (v_texcoord.q);                                                          \n"
      "  addr.t = addr.t / (v_texcoord.q);                                                          \n"
      "  vec4 spriteColor = texture2D(u_sprite,addr);                                               \n"
      "  if( spriteColor.a == 0.0 ) discard;                                                          \n"
      "  vec4 fboColor    = texture2D(u_fbo,faddr);                                                 \n"
      "  if( fboColor.a > 0.0 && spriteColor.a > 0.0 )                                              \n"
      "  {                                                                                          \n"
      "    gl_FragColor = spriteColor*0.5 + fboColor*0.5;                                           \n"
      "    gl_FragColor.a = fboColor.a;                                                             \n"
      "  }else{                                                                                     \n"
      "    gl_FragColor = spriteColor;                                                              \n"
      "  }                                                                                          \n"
      "}\n";
const GLchar * pYglprg_vdp1_halftrans_f[] = {Yglprg_vdp1_halftrans_f, NULL};

int Ygl_uniformHalfTrans(void * p )
{
   YglProgram * prg;
   prg = p;

   glEnableVertexAttribArray(prg->vertexp);
   glEnableVertexAttribArray(prg->texcoordp);

   glUniform1i(id_hf_sprite, 0);
   glUniform1i(id_hf_fbo, 1);
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D,_Ygl->vdp1FrameBuff[_Ygl->drawframe]);
   glUniform1i(id_hf_fbowidth, GlWidth);
   glUniform1i(id_hf_fboheight, GlHeight);
   glActiveTexture(GL_TEXTURE0);
   return 0;
}

int Ygl_cleanupHalfTrans(void * p )
{
   YglProgram * prg;
   prg = p;
   return 0;
}

/*------------------------------------------------------------------------------------
 *  VDP1 UserClip Operation
 * ----------------------------------------------------------------------------------*/
int Ygl_uniformStartUserClip(void * p )
{
   YglProgram * prg;
   prg = p;

   glEnableVertexAttribArray(prg->vertexp);
   glEnableVertexAttribArray(prg->texcoordp);

   if( prg->ux1 != -1 )
   {
      GLint vertices[12];
      glColorMask( GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE );
      glStencilMask(0xffffffff);
      glClearStencil(0);
      glClear(GL_STENCIL_BUFFER_BIT);
      glEnable(GL_STENCIL_TEST);
      glStencilFunc(GL_ALWAYS,0x1,0x01);
      glStencilOp(GL_REPLACE,GL_REPLACE,GL_REPLACE);
      glDisable(GL_TEXTURE_2D);

      // render
      vertices[0] = (int)((float)prg->ux1 * vdp1wratio);
      vertices[1] = (int)((float)prg->uy1 * vdp1hratio);
      vertices[2] = (int)((float)(prg->ux2+1) * vdp1wratio);
      vertices[3] = (int)((float)prg->uy1 * vdp1hratio);
      vertices[4] = (int)((float)(prg->ux2+1) * vdp1wratio);
      vertices[5] = (int)((float)(prg->uy2+1) * vdp1hratio);

      vertices[6] = (int)((float)prg->ux1 * vdp1wratio);
      vertices[7] = (int)((float)prg->uy1 * vdp1hratio);
      vertices[8] = (int)((float)(prg->ux2+1) * vdp1wratio);
      vertices[9] = (int)((float)(prg->uy2+1) * vdp1hratio);
      vertices[10] = (int)((float)prg->ux1 * vdp1wratio);
      vertices[11] = (int)((float)(prg->uy2+1) * vdp1hratio);

      glUniformMatrix4fv( prg->mtxModelView, 1, GL_FALSE, (GLfloat*) &_Ygl->mtxModelView );
      glUniformMatrix4fv( prg->mtxTexture, 1, GL_FALSE, (GLfloat*) &_Ygl->mtxTexture.m[0][0] );
      glVertexAttribPointer(prg->vertexp,2, GL_INT,GL_FALSE, 0, (GLvoid*)vertices );

      glDrawArrays(GL_TRIANGLES, 0, 6);

      glColorMask( GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE );
      glStencilFunc(GL_ALWAYS,0,0x0);
      glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
      glDisable(GL_STENCIL_TEST);
      glEnable(GL_TEXTURE_2D);
   }

   glEnable(GL_STENCIL_TEST);
   glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
   if( prg->uClipMode == 0x02 )
   {
      glStencilFunc(GL_EQUAL,0x1,0xFF);
   }else if( prg->uClipMode == 0x03 )
   {
      glStencilFunc(GL_EQUAL,0x0,0xFF);
   }else{
      glStencilFunc(GL_ALWAYS,0,0xFF);
   }

   return 0;
}

int Ygl_cleanupStartUserClip(void * p ){return 0;}

int Ygl_uniformEndUserClip(void * p )
{
   YglProgram * prg;
   prg = p;
   glDisable(GL_STENCIL_TEST);
   glStencilFunc(GL_ALWAYS,0,0xFF);
   return 0;
}

int Ygl_cleanupEndUserClip(void * p ){return 0;}


int Ygl_uniformStartVDP2Window(void * p )
{
   YglProgram * prg;
   prg = p;


   glEnable(GL_STENCIL_TEST);
   glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);

   glEnableVertexAttribArray(prg->vertexp);
   glEnableVertexAttribArray(prg->texcoordp);


   if( prg->bwin0 && !prg->bwin1 )
   {
      if( prg->logwin0 )
      {
         glStencilFunc(GL_EQUAL,0x01,0x01);
      }else{
         glStencilFunc(GL_NOTEQUAL,0x01,0x01);
      }
   }else if( !prg->bwin0 && prg->bwin1 ) {

      if( prg->logwin1 )
      {
         glStencilFunc(GL_EQUAL,0x02,0x02);
      }else{
         glStencilFunc(GL_NOTEQUAL,0x02,0x02);
      }
   }else if( prg->bwin0 && prg->bwin1 ) {
       // and
      if( prg->winmode == 0x0 )
      {
         glStencilFunc(GL_EQUAL,0x03,0x03);

      // OR
      }else if( prg->winmode == 0x01 )
      {
          glStencilFunc(GL_LEQUAL,0x01,0x03);

      }
   }

   return 0;
}

int Ygl_cleanupStartVDP2Window(void * p ){return 0;}

int Ygl_uniformEndVDP2Window(void * p )
{
   YglProgram * prg;
   prg = p;
   glDisable(GL_STENCIL_TEST);
   glStencilFunc(GL_ALWAYS,0,0xFF);
   return 0;
}

int Ygl_cleanupEndVDP2Window(void * p ){return 0;}


/*------------------------------------------------------------------------------------
 *  VDP2 Draw Frame buffer Operation
 * ----------------------------------------------------------------------------------*/
static int idvdp1FrameBuffer;
static int idfrom;
static int idto;
static int idcoloroffset;

const GLchar Yglprg_vdp1_drawfb_v[] =
      "#version 300 es \n"
      "uniform mat4 u_mvpMatrix;                \n"
      "layout (location = 0) in vec4 a_position;               \n"
      "layout (location = 1) in vec2 a_texcoord;               \n"
      "out vec2 v_texcoord;                 \n"
      "void main() {                            \n"
      "   v_texcoord  = a_texcoord;             \n"
      "   gl_Position = a_position*u_mvpMatrix; \n"
      "}\n";
const GLchar * pYglprg_vdp2_drawfb_v[] = {Yglprg_vdp1_drawfb_v, NULL};

const GLchar Yglprg_vdp2_drawfb_f[] =
      "#version 300 es \n"
      "precision highp float;                             \n"
      "in vec2 v_texcoord;                             \n"
      "uniform sampler2D s_vdp1FrameBuffer;                 \n"
      "uniform float u_from;                                  \n"
      "uniform float u_to;                                    \n"
      "uniform vec4 u_coloroffset;                            \n"
      "out vec4 fragColor;            \n"
      "void main()                                          \n"
      "{                                                    \n"
      "  vec2 addr = v_texcoord;                         \n"
      "  vec4 fbColor = texture(s_vdp1FrameBuffer,addr);  \n"
      "  int additional = int(fbColor.a * 255.0);           \n"
      "  float alpha = float((additional/8)*8)/255.0;  \n"
      "  float depth = clamp( float(additional&0x07)/10.0, 0.0, 1.0); \n"
      "  if( depth < u_from || depth > u_to ){ discard;return;} \n"
      "  if( alpha > 0.0){ \n"
      "     fragColor = fbColor;                            \n"
      "     fragColor.r = clamp( fragColor.r+u_coloroffset.r, 0.0, 1.0);\n"
      "     fragColor.g = clamp( fragColor.g+u_coloroffset.r, 0.0, 1.0);\n"
      "     fragColor.b = clamp( fragColor.b+u_coloroffset.r, 0.0, 1.0);\n"
      "     fragColor.a = alpha;\n"
      "     gl_FragDepth = depth;\n"
      "  } else { \n"
      "     discard;\n"
      "  }\n"
      "}                                                    \n";



#if 0
const GLchar Yglprg_vdp2_drawfb_f[] = \
"uniform sampler2D vdp1FrameBuffer;\n" \
"uniform float from; \n" \
"uniform float to; \n" \
"uniform vec4 coloroffset;\n" \
"void main() {\n" \
"  vec2 addr = gl_TexCoord[0].st;\n" \
"  vec4 fbColor = texture2D(vdp1FrameBuffer,addr);\n" \
"  int additional = int(fbColor.a * 255.0);\n" \
"   float alpha = float(int(additional/8)*8)/255.0; \n" \
"   float depth = (fbColor.a-alpha)*255.0/10.0 + 0.05; \n" \
"  if( depth < from || depth > to ){ discard; return; }\n" \
"  gl_FragColor = fbColor; \n" \
"  gl_FragColor += vec4(coloroffset.r,coloroffset.g,coloroffset.b,0.0);\n" \
"  gl_FragColor.a = alpha;\n" \
"  gl_FragDepth = depth;\n" \
"}\n";
#endif

const GLchar * pYglprg_vdp2_drawfb_f[] = {Yglprg_vdp2_drawfb_f, NULL};

int Ygl_uniformVDP2DrawFramebuffer( void * p, float from, float to , float * offsetcol )
{
   YglProgram * prg;
   prg = p;

   glUseProgram(_prgid[PG_VDP2_DRAWFRAMEBUFF]);
   glUniform1i(idvdp1FrameBuffer, 0);
   glActiveTexture(GL_TEXTURE0);
   glUniform1f(idfrom,from);
   glUniform1f(idto,to);
   glUniform4f(idcoloroffset,offsetcol[0],offsetcol[1],offsetcol[2],offsetcol[3]);
   glEnableVertexAttribArray(prg->vertexp);
   glEnableVertexAttribArray(prg->texcoordp);
}


/*------------------------------------------------------------------------------------
 *  VDP2 Add Blend operaiotn
 * ----------------------------------------------------------------------------------*/
int Ygl_uniformAddBlend(void * p )
{
   glBlendFunc(GL_ONE,GL_ONE);
   return 0;
}

int Ygl_cleanupAddBlend(void * p )
{
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   return 0;
}


int YglGetProgramId( int prg )
{
   return _prgid[prg];
}

int YglInitShader( int id, const GLchar * vertex[], const GLchar * frag[] )
{
    GLint compiled,linked;
    GLuint vshader;
    GLuint fshader;

   _prgid[id] = glCreateProgram();
    if (_prgid[id] == 0 ) return -1;

    vshader = glCreateShader(GL_VERTEX_SHADER);
    fshader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vshader, 1, vertex, NULL);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
       printf( "Compile error in vertex shader.\n");
       Ygl_printShaderError(vshader);
       _prgid[id] = 0;
       return -1;
    }

    glShaderSource(fshader, 1, frag, NULL);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
       printf( "Compile error in fragment shader.\n");
       Ygl_printShaderError(fshader);
       _prgid[id] = 0;
       return -1;
     }

    glAttachShader(_prgid[id], vshader);
    glAttachShader(_prgid[id], fshader);
    glLinkProgram(_prgid[id]);
    glGetProgramiv(_prgid[id], GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
       printf("Link error..\n");
       Ygl_printShaderError(_prgid[id]);
       _prgid[id] = 0;
       return -1;
    }
    return 0;
}

int YglProgramInit()
{

   GLint compiled,linked;

   printf("PG_NORMAL\n");
   //
   if( YglInitShader( PG_NORMAL, pYglprg_normal_v, pYglprg_normal_f ) != 0 )
      return -1;

   //
   _prgid[PG_VFP1_STARTUSERCLIP] = _prgid[PG_NORMAL];
   _prgid[PG_VFP1_ENDUSERCLIP] = _prgid[PG_NORMAL];
   _prgid[PG_VDP2_ADDBLEND] = _prgid[PG_NORMAL];

   printf("PG_VDP1_NORMAL\n");
   //
   if( YglInitShader( PG_VDP1_NORMAL, pYglprg_vdp1_normal_v, pYglprg_vdp1_normal_f ) != 0 )
      return -1;

   printf("PG_VFP1_GOURAUDSAHDING\n");

   //
   if( YglInitShader( PG_VFP1_GOURAUDSAHDING, pYglprg_vdp1_gouraudshading_v, pYglprg_vdp1_gouraudshading_f ) != 0 )
      return -1;

   printf("PG_VDP2_DRAWFRAMEBUFF --START--\n");

   //
   if( YglInitShader( PG_VDP2_DRAWFRAMEBUFF, pYglprg_vdp2_drawfb_v, pYglprg_vdp2_drawfb_f ) != 0 )
      return -1;

   printf("PG_VDP2_DRAWFRAMEBUFF --END--\n");

   idvdp1FrameBuffer = glGetUniformLocation(_prgid[PG_VDP2_DRAWFRAMEBUFF], (const GLchar *)"s_vdp1FrameBuffer");
   idfrom = glGetUniformLocation(_prgid[PG_VDP2_DRAWFRAMEBUFF], (const GLchar *)"u_from");
   idto   = glGetUniformLocation(_prgid[PG_VDP2_DRAWFRAMEBUFF], (const GLchar *)"u_to");
   idcoloroffset = glGetUniformLocation(_prgid[PG_VDP2_DRAWFRAMEBUFF], (const GLchar *)"u_coloroffset");



   _Ygl->renderfb.prgid=_prgid[PG_VDP2_DRAWFRAMEBUFF];
   _Ygl->renderfb.setupUniform    = Ygl_uniformNormal;
   _Ygl->renderfb.cleanupUniform  = Ygl_cleanupNormal;
   _Ygl->renderfb.vertexp         = glGetAttribLocation(_prgid[PG_VDP2_DRAWFRAMEBUFF],(const GLchar *)"a_position");
   _Ygl->renderfb.texcoordp       = glGetAttribLocation(_prgid[PG_VDP2_DRAWFRAMEBUFF],(const GLchar *)"a_texcoord");
   _Ygl->renderfb.mtxModelView    = glGetUniformLocation(_prgid[PG_VDP2_DRAWFRAMEBUFF],(const GLchar *)"u_mvpMatrix");

   printf("PG_VFP1_HALFTRANS\n");

   //
   if( YglInitShader( PG_VFP1_HALFTRANS, pYglprg_vdp1_halftrans_v, pYglprg_vdp1_halftrans_f ) != 0 )
      return -1;

   id_hf_sprite = glGetUniformLocation(_prgid[PG_VFP1_HALFTRANS], (const GLchar *)"u_sprite");
   id_hf_fbo = glGetUniformLocation(_prgid[PG_VFP1_HALFTRANS], (const GLchar *)"u_fbo");
   id_hf_fbowidth = glGetUniformLocation(_prgid[PG_VFP1_HALFTRANS], (const GLchar *)"u_fbowidth");
   id_hf_fboheight = glGetUniformLocation(_prgid[PG_VFP1_HALFTRANS], (const GLchar *)"u_fbohegiht");

   printf("PG_VFP1_GOURAUDSAHDING_HALFTRANS\n");

   if( YglInitShader( PG_VFP1_GOURAUDSAHDING_HALFTRANS, pYglprg_vdp1_gouraudshading_hf_v, pYglprg_vdp1_gouraudshading_hf_f ) != 0 )
      return -1;

   id_sprite = glGetUniformLocation(_prgid[PG_VFP1_GOURAUDSAHDING_HALFTRANS], (const GLchar *)"u_sprite");
   id_fbo = glGetUniformLocation(_prgid[PG_VFP1_GOURAUDSAHDING_HALFTRANS], (const GLchar *)"u_fbo");
   id_fbowidth = glGetUniformLocation(_prgid[PG_VFP1_GOURAUDSAHDING_HALFTRANS], (const GLchar *)"u_fbowidth");
   id_fboheight = glGetUniformLocation(_prgid[PG_VFP1_GOURAUDSAHDING_HALFTRANS], (const GLchar *)"u_fbohegiht");

   printf("PG_WINDOW\n");
   //
   if( YglInitShader( PG_WINDOW, pYglprg_window_v, pYglprg_window_f ) != 0 )
      return -1;

   _Ygl->windowpg.prgid=_prgid[PG_WINDOW];
   _Ygl->windowpg.setupUniform    = Ygl_uniformNormal;
   _Ygl->windowpg.cleanupUniform  = Ygl_cleanupNormal;
   _Ygl->windowpg.vertexp         = glGetAttribLocation(_prgid[PG_WINDOW],(const GLchar *)"a_position");
   _Ygl->windowpg.mtxModelView    = glGetUniformLocation(_prgid[PG_WINDOW],(const GLchar *)"u_mvpMatrix");

   return 0;
}



int YglProgramChange( YglLevel * level, int prgid )
{
   GLuint id;
   YglProgram* tmp;
   YglProgram* current;
   level->prgcurrent++;

   if( level->prgcurrent >= level->prgcount)
   {
      level->prgcount++;
      tmp = (YglProgram*)malloc(sizeof(YglProgram)*level->prgcount);
      if( tmp == NULL ) return -1;
      memset(tmp,0,sizeof(YglProgram)*level->prgcount);
      memcpy(tmp,level->prg,sizeof(YglProgram)*(level->prgcount-1));
      level->prg = tmp;

      level->prg[level->prgcurrent].currentQuad = 0;
      level->prg[level->prgcurrent].maxQuad = 12 * 64;
      if ((level->prg[level->prgcurrent].quads = (int *) malloc(level->prg[level->prgcurrent].maxQuad * sizeof(int))) == NULL)
         return -1;

      if ((level->prg[level->prgcurrent].textcoords = (float *) malloc(level->prg[level->prgcurrent].maxQuad * sizeof(float) * 2)) == NULL)
         return -1;

       if ((level->prg[level->prgcurrent].vertexAttribute = (float *) malloc(level->prg[level->prgcurrent].maxQuad * sizeof(float)*2)) == NULL)
         return -1;
   }

   current = &level->prg[level->prgcurrent];
   level->prg[level->prgcurrent].prgid=prgid;
   level->prg[level->prgcurrent].prg=_prgid[prgid];

   if( prgid == PG_NORMAL )
   {
      current->setupUniform    = Ygl_uniformNormal;
      current->cleanupUniform  = Ygl_cleanupNormal;
      current->vertexp         = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_position");
      current->texcoordp       = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_texcoord");
      current->mtxModelView    = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_mvpMatrix");
      current->mtxTexture      = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_texMatrix");
      id = glGetUniformLocation(_prgid[PG_NORMAL], (const GLchar *)"s_texture");
      glUniform1i(id, 0);
   }else if( prgid == PG_VDP1_NORMAL )
   {
      current->setupUniform    = Ygl_uniformVdp1Normal;
      current->cleanupUniform  = Ygl_cleanupVdp1Normal;
      current->vertexp         = glGetAttribLocation(_prgid[PG_VDP1_NORMAL],(const GLchar *)"a_position");
      current->texcoordp       = glGetAttribLocation(_prgid[PG_VDP1_NORMAL],(const GLchar *)"a_texcoord");
      current->mtxModelView    = glGetUniformLocation(_prgid[PG_VDP1_NORMAL],(const GLchar *)"u_mvpMatrix");
      current->mtxTexture      = glGetUniformLocation(_prgid[PG_VDP1_NORMAL],(const GLchar *)"u_texMatrix");
      id = glGetUniformLocation(_prgid[PG_NORMAL], (const GLchar *)"s_texture");
      glUniform1i(id, 0);

   }else if( prgid == PG_VFP1_GOURAUDSAHDING )
   {
      level->prg[level->prgcurrent].setupUniform = Ygl_uniformGlowShading;
      level->prg[level->prgcurrent].cleanupUniform = Ygl_cleanupGlowShading;
      id = glGetUniformLocation(_prgid[PG_VFP1_GOURAUDSAHDING], (const GLchar *)"u_sprite");
      glUniform1i(id, 0);
      level->prg[level->prgcurrent].vaid = 0;
      level->prg[level->prgcurrent].vaid = glGetAttribLocation(_prgid[PG_VFP1_GOURAUDSAHDING],(const GLchar *)"a_grcolor");
      current->vertexp         = glGetAttribLocation(_prgid[PG_VFP1_GOURAUDSAHDING],(const GLchar *)"a_position");
      current->texcoordp       = glGetAttribLocation(_prgid[PG_VFP1_GOURAUDSAHDING],(const GLchar *)"a_texcoord");
      current->mtxModelView    = glGetUniformLocation(_prgid[PG_VFP1_GOURAUDSAHDING],(const GLchar *)"u_mvpMatrix");
      current->mtxTexture      = glGetUniformLocation(_prgid[PG_VFP1_GOURAUDSAHDING],(const GLchar *)"u_texMatrix");
   }
   else if( prgid == PG_VFP1_STARTUSERCLIP )
   {
      level->prg[level->prgcurrent].setupUniform = Ygl_uniformStartUserClip;
      level->prg[level->prgcurrent].cleanupUniform = Ygl_cleanupStartUserClip;
      current->vertexp         = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_position");
      current->texcoordp       = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_texcoord");
      current->mtxModelView    = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_mvpMatrix");
      current->mtxTexture      = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_texMatrix");
   }
   else if( prgid == PG_VFP1_ENDUSERCLIP )
   {
      level->prg[level->prgcurrent].setupUniform = Ygl_uniformEndUserClip;
      level->prg[level->prgcurrent].cleanupUniform = Ygl_cleanupEndUserClip;
      current->vertexp         = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_position");
      current->texcoordp       = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_texcoord");
      current->mtxModelView    = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_mvpMatrix");
      current->mtxTexture      = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_texMatrix");
   }
   else if( prgid == PG_VFP1_HALFTRANS )
   {
      GLuint id;
      level->prg[level->prgcurrent].setupUniform = Ygl_uniformHalfTrans;
      level->prg[level->prgcurrent].cleanupUniform = Ygl_cleanupHalfTrans;
      current->vertexp         = glGetAttribLocation(_prgid[PG_VFP1_HALFTRANS],(const GLchar *)"a_position");
      current->texcoordp       = glGetAttribLocation(_prgid[PG_VFP1_HALFTRANS],(const GLchar *)"a_texcoord");
      current->mtxModelView    = glGetUniformLocation(_prgid[PG_VFP1_HALFTRANS],(const GLchar *)"u_mvpMatrix");
      current->mtxTexture      = glGetUniformLocation(_prgid[PG_VFP1_HALFTRANS],(const GLchar *)"u_texMatrix");
   }
   else if( prgid == PG_VFP1_GOURAUDSAHDING_HALFTRANS )
   {
      GLuint id;
      level->prg[level->prgcurrent].setupUniform = Ygl_uniformGlowShadingHalfTrans;
      level->prg[level->prgcurrent].cleanupUniform = Ygl_cleanupGlowShadingHalfTrans;
      level->prg[level->prgcurrent].vaid = glGetAttribLocation(_prgid[PG_VFP1_GOURAUDSAHDING_HALFTRANS],(const GLchar *)"a_grcolor");
      current->vertexp         = glGetAttribLocation(_prgid[PG_VFP1_GOURAUDSAHDING_HALFTRANS],(const GLchar *)"a_position");
      current->texcoordp       = glGetAttribLocation(_prgid[PG_VFP1_GOURAUDSAHDING_HALFTRANS],(const GLchar *)"a_texcoord");
      current->mtxModelView    = glGetUniformLocation(_prgid[PG_VFP1_GOURAUDSAHDING_HALFTRANS],(const GLchar *)"u_mvpMatrix");
      current->mtxTexture      = glGetUniformLocation(_prgid[PG_VFP1_GOURAUDSAHDING_HALFTRANS],(const GLchar *)"u_texMatrix");


   }else if( prgid == PG_VDP2_ADDBLEND )
   {
      level->prg[level->prgcurrent].setupUniform = Ygl_uniformAddBlend;
      level->prg[level->prgcurrent].cleanupUniform = Ygl_cleanupAddBlend;
      current->vertexp         = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_position");
      current->texcoordp       = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_texcoord");
      current->mtxModelView    = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_mvpMatrix");
      current->mtxTexture      = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_texMatrix");
   }else if( prgid == PG_VDP2_STARTWINDOW )
   {
      level->prg[level->prgcurrent].setupUniform = Ygl_uniformStartVDP2Window;
      level->prg[level->prgcurrent].cleanupUniform = Ygl_cleanupStartVDP2Window;
      current->vertexp         = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_position");
      current->texcoordp       = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_texcoord");
      current->mtxModelView    = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_mvpMatrix");
      current->mtxTexture      = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_texMatrix");
   }else if( prgid == PG_VDP2_ENDWINDOW )
   {
      level->prg[level->prgcurrent].setupUniform = Ygl_uniformEndVDP2Window;
      level->prg[level->prgcurrent].cleanupUniform = Ygl_cleanupEndVDP2Window;
      current->vertexp         = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_position");
      current->texcoordp       = glGetAttribLocation(_prgid[PG_NORMAL],(const GLchar *)"a_texcoord");
      current->mtxModelView    = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_mvpMatrix");
      current->mtxTexture      = glGetUniformLocation(_prgid[PG_NORMAL],(const GLchar *)"u_texMatrix");
   }else{
      level->prg[level->prgcurrent].setupUniform = NULL;
      level->prg[level->prgcurrent].cleanupUniform = NULL;
   }
   return 0;

}

#endif
