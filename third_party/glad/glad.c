#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct gladGLversionStruct GLVersion;

static int has_ext(const char* ext) {
#if defined(GL_VERSION_3_0) || defined(GL_ES_VERSION_3_0)
  if (GLVersion.major < 3 || glGetStringi == NULL) {
#endif
    const char* extensions;
    const char* loc;
    const char* terminator;
    extensions = (const char*)glGetString(GL_EXTENSIONS);
    if (extensions == NULL || ext == NULL) {
      return 0;
    }

    while (1) {
      loc = strstr(extensions, ext);
      if (loc == NULL) {
        return 0;
      }

      terminator = loc + strlen(ext);
      if ((loc == extensions || *(loc - 1) == ' ') &&
          (*terminator == ' ' || *terminator == '\0')) {
        return 1;
      }
      extensions = terminator;
    }
#if defined(GL_VERSION_3_0) || defined(GL_ES_VERSION_3_0)
  } else {
    GLint num_exts, index;

    glGetIntegerv(GL_NUM_EXTENSIONS, &num_exts);
    for (index = 0; index < num_exts; index++) {
      if (strcmp((const char*)glGetStringi(GL_EXTENSIONS, index), ext) == 0) {
        return 1;
      }
    }
  }
#endif

  return 0;
}
int GLAD_GL_VERSION_1_0;
int GLAD_GL_VERSION_1_1;
int GLAD_GL_VERSION_1_2;
int GLAD_GL_VERSION_1_3;
int GLAD_GL_VERSION_1_4;
int GLAD_GL_VERSION_1_5;
int GLAD_GL_VERSION_2_0;
int GLAD_GL_VERSION_2_1;
int GLAD_GL_VERSION_3_0;
int GLAD_GL_VERSION_3_1;
int GLAD_GL_VERSION_3_2;
PFNGLCOPYTEXIMAGE1DPROC glad_glCopyTexImage1D;
PFNGLVERTEXATTRIBI3UIPROC glad_glVertexAttribI3ui;
PFNGLWINDOWPOS2SPROC glad_glWindowPos2s;
PFNGLWINDOWPOS2IPROC glad_glWindowPos2i;
PFNGLWINDOWPOS2FPROC glad_glWindowPos2f;
PFNGLWINDOWPOS2DPROC glad_glWindowPos2d;
PFNGLVERTEX2FVPROC glad_glVertex2fv;
PFNGLINDEXIPROC glad_glIndexi;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_glFramebufferRenderbuffer;
PFNGLRECTDVPROC glad_glRectdv;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glad_glCompressedTexSubImage3D;
PFNGLEVALCOORD2DPROC glad_glEvalCoord2d;
PFNGLEVALCOORD2FPROC glad_glEvalCoord2f;
PFNGLINDEXDPROC glad_glIndexd;
PFNGLVERTEXATTRIB1SVPROC glad_glVertexAttrib1sv;
PFNGLINDEXFPROC glad_glIndexf;
PFNGLLINEWIDTHPROC glad_glLineWidth;
PFNGLGETINTEGERI_VPROC glad_glGetIntegeri_v;
PFNGLGETMAPFVPROC glad_glGetMapfv;
PFNGLINDEXSPROC glad_glIndexs;
PFNGLCOMPILESHADERPROC glad_glCompileShader;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glad_glGetTransformFeedbackVarying;
PFNGLWINDOWPOS2IVPROC glad_glWindowPos2iv;
PFNGLINDEXFVPROC glad_glIndexfv;
PFNGLFOGIVPROC glad_glFogiv;
PFNGLSTENCILMASKSEPARATEPROC glad_glStencilMaskSeparate;
PFNGLRASTERPOS2FVPROC glad_glRasterPos2fv;
PFNGLLIGHTMODELIVPROC glad_glLightModeliv;
PFNGLCOLOR4UIPROC glad_glColor4ui;
PFNGLSECONDARYCOLOR3FVPROC glad_glSecondaryColor3fv;
PFNGLFOGFVPROC glad_glFogfv;
PFNGLENABLEIPROC glad_glEnablei;
PFNGLVERTEX4IVPROC glad_glVertex4iv;
PFNGLEVALCOORD1FVPROC glad_glEvalCoord1fv;
PFNGLWINDOWPOS2SVPROC glad_glWindowPos2sv;
PFNGLCREATESHADERPROC glad_glCreateShader;
PFNGLISBUFFERPROC glad_glIsBuffer;
PFNGLGETMULTISAMPLEFVPROC glad_glGetMultisamplefv;
PFNGLGENRENDERBUFFERSPROC glad_glGenRenderbuffers;
PFNGLCOPYTEXSUBIMAGE2DPROC glad_glCopyTexSubImage2D;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glad_glCompressedTexImage2D;
PFNGLVERTEXATTRIB1FPROC glad_glVertexAttrib1f;
PFNGLBLENDFUNCSEPARATEPROC glad_glBlendFuncSeparate;
PFNGLVERTEX4FVPROC glad_glVertex4fv;
PFNGLBINDTEXTUREPROC glad_glBindTexture;
PFNGLVERTEXATTRIB1SPROC glad_glVertexAttrib1s;
PFNGLTEXCOORD2FVPROC glad_glTexCoord2fv;
PFNGLSAMPLEMASKIPROC glad_glSampleMaski;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glad_glDrawRangeElementsBaseVertex;
PFNGLTEXCOORD4FVPROC glad_glTexCoord4fv;
PFNGLUNIFORMMATRIX3X2FVPROC glad_glUniformMatrix3x2fv;
PFNGLPOINTSIZEPROC glad_glPointSize;
PFNGLVERTEXATTRIB2DVPROC glad_glVertexAttrib2dv;
PFNGLDELETEPROGRAMPROC glad_glDeleteProgram;
PFNGLCOLOR4BVPROC glad_glColor4bv;
PFNGLRASTERPOS2FPROC glad_glRasterPos2f;
PFNGLRASTERPOS2DPROC glad_glRasterPos2d;
PFNGLLOADIDENTITYPROC glad_glLoadIdentity;
PFNGLRASTERPOS2IPROC glad_glRasterPos2i;
PFNGLRENDERBUFFERSTORAGEPROC glad_glRenderbufferStorage;
PFNGLUNIFORMMATRIX4X3FVPROC glad_glUniformMatrix4x3fv;
PFNGLCOLOR3BPROC glad_glColor3b;
PFNGLCLEARBUFFERFVPROC glad_glClearBufferfv;
PFNGLEDGEFLAGPROC glad_glEdgeFlag;
PFNGLVERTEX3DPROC glad_glVertex3d;
PFNGLVERTEX3FPROC glad_glVertex3f;
PFNGLVERTEX3IPROC glad_glVertex3i;
PFNGLCOLOR3IPROC glad_glColor3i;
PFNGLUNIFORM3FPROC glad_glUniform3f;
PFNGLVERTEXATTRIB4UBVPROC glad_glVertexAttrib4ubv;
PFNGLCOLOR3SPROC glad_glColor3s;
PFNGLVERTEX3SPROC glad_glVertex3s;
PFNGLCOLORMASKIPROC glad_glColorMaski;
PFNGLCLEARBUFFERFIPROC glad_glClearBufferfi;
PFNGLTEXCOORD1IVPROC glad_glTexCoord1iv;
PFNGLBLITFRAMEBUFFERPROC glad_glBlitFramebuffer;
PFNGLVERTEXATTRIB3FPROC glad_glVertexAttrib3f;
PFNGLVERTEX2IVPROC glad_glVertex2iv;
PFNGLCOLOR3SVPROC glad_glColor3sv;
PFNGLGETVERTEXATTRIBDVPROC glad_glGetVertexAttribdv;
PFNGLUNIFORMMATRIX3X4FVPROC glad_glUniformMatrix3x4fv;
PFNGLNORMALPOINTERPROC glad_glNormalPointer;
PFNGLVERTEX4SVPROC glad_glVertex4sv;
PFNGLPASSTHROUGHPROC glad_glPassThrough;
PFNGLFOGIPROC glad_glFogi;
PFNGLBEGINPROC glad_glBegin;
PFNGLEVALCOORD2DVPROC glad_glEvalCoord2dv;
PFNGLCOLOR3UBVPROC glad_glColor3ubv;
PFNGLVERTEXPOINTERPROC glad_glVertexPointer;
PFNGLSECONDARYCOLOR3UIVPROC glad_glSecondaryColor3uiv;
PFNGLDELETEFRAMEBUFFERSPROC glad_glDeleteFramebuffers;
PFNGLDRAWARRAYSPROC glad_glDrawArrays;
PFNGLUNIFORM1UIPROC glad_glUniform1ui;
PFNGLMULTITEXCOORD1DPROC glad_glMultiTexCoord1d;
PFNGLMULTITEXCOORD1FPROC glad_glMultiTexCoord1f;
PFNGLLIGHTFVPROC glad_glLightfv;
PFNGLVERTEXATTRIB3DPROC glad_glVertexAttrib3d;
PFNGLCLEARPROC glad_glClear;
PFNGLMULTITEXCOORD1IPROC glad_glMultiTexCoord1i;
PFNGLGETACTIVEUNIFORMNAMEPROC glad_glGetActiveUniformName;
PFNGLMULTITEXCOORD1SPROC glad_glMultiTexCoord1s;
PFNGLISENABLEDPROC glad_glIsEnabled;
PFNGLSTENCILOPPROC glad_glStencilOp;
PFNGLGETQUERYOBJECTUIVPROC glad_glGetQueryObjectuiv;
PFNGLFRAMEBUFFERTEXTURE2DPROC glad_glFramebufferTexture2D;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC
glad_glGetFramebufferAttachmentParameteriv;
PFNGLTRANSLATEFPROC glad_glTranslatef;
PFNGLVERTEXATTRIB4NUBPROC glad_glVertexAttrib4Nub;
PFNGLTRANSLATEDPROC glad_glTranslated;
PFNGLTEXCOORD3SVPROC glad_glTexCoord3sv;
PFNGLGETFRAGDATALOCATIONPROC glad_glGetFragDataLocation;
PFNGLTEXIMAGE1DPROC glad_glTexImage1D;
PFNGLTEXPARAMETERIVPROC glad_glTexParameteriv;
PFNGLSECONDARYCOLOR3BVPROC glad_glSecondaryColor3bv;
PFNGLGETMATERIALFVPROC glad_glGetMaterialfv;
PFNGLGETTEXIMAGEPROC glad_glGetTexImage;
PFNGLFOGCOORDFVPROC glad_glFogCoordfv;
PFNGLPIXELMAPUIVPROC glad_glPixelMapuiv;
PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog;
PFNGLGENFRAMEBUFFERSPROC glad_glGenFramebuffers;
PFNGLINDEXSVPROC glad_glIndexsv;
PFNGLGETATTACHEDSHADERSPROC glad_glGetAttachedShaders;
PFNGLISRENDERBUFFERPROC glad_glIsRenderbuffer;
PFNGLVERTEX3IVPROC glad_glVertex3iv;
PFNGLBITMAPPROC glad_glBitmap;
PFNGLMATERIALIPROC glad_glMateriali;
PFNGLISVERTEXARRAYPROC glad_glIsVertexArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glad_glDisableVertexAttribArray;
PFNGLGETQUERYIVPROC glad_glGetQueryiv;
PFNGLTEXCOORD4FPROC glad_glTexCoord4f;
PFNGLTEXCOORD4DPROC glad_glTexCoord4d;
PFNGLTEXCOORD4IPROC glad_glTexCoord4i;
PFNGLMATERIALFPROC glad_glMaterialf;
PFNGLTEXCOORD4SPROC glad_glTexCoord4s;
PFNGLGETUNIFORMINDICESPROC glad_glGetUniformIndices;
PFNGLISSHADERPROC glad_glIsShader;
PFNGLMULTITEXCOORD2SPROC glad_glMultiTexCoord2s;
PFNGLVERTEXATTRIBI4UBVPROC glad_glVertexAttribI4ubv;
PFNGLVERTEX3DVPROC glad_glVertex3dv;
PFNGLGETINTEGER64VPROC glad_glGetInteger64v;
PFNGLPOINTPARAMETERIVPROC glad_glPointParameteriv;
PFNGLENABLEPROC glad_glEnable;
PFNGLGETACTIVEUNIFORMSIVPROC glad_glGetActiveUniformsiv;
PFNGLCOLOR4FVPROC glad_glColor4fv;
PFNGLTEXCOORD1FVPROC glad_glTexCoord1fv;
PFNGLTEXCOORD2SVPROC glad_glTexCoord2sv;
PFNGLVERTEXATTRIB4DVPROC glad_glVertexAttrib4dv;
PFNGLMULTITEXCOORD1DVPROC glad_glMultiTexCoord1dv;
PFNGLMULTITEXCOORD2IPROC glad_glMultiTexCoord2i;
PFNGLTEXCOORD3FVPROC glad_glTexCoord3fv;
PFNGLSECONDARYCOLOR3USVPROC glad_glSecondaryColor3usv;
PFNGLTEXGENFPROC glad_glTexGenf;
PFNGLGETPOINTERVPROC glad_glGetPointerv;
PFNGLPOLYGONOFFSETPROC glad_glPolygonOffset;
PFNGLGETUNIFORMUIVPROC glad_glGetUniformuiv;
PFNGLNORMAL3FVPROC glad_glNormal3fv;
PFNGLSECONDARYCOLOR3SPROC glad_glSecondaryColor3s;
PFNGLDEPTHRANGEPROC glad_glDepthRange;
PFNGLFRUSTUMPROC glad_glFrustum;
PFNGLMULTITEXCOORD4SVPROC glad_glMultiTexCoord4sv;
PFNGLDRAWBUFFERPROC glad_glDrawBuffer;
PFNGLPUSHMATRIXPROC glad_glPushMatrix;
PFNGLRASTERPOS3FVPROC glad_glRasterPos3fv;
PFNGLORTHOPROC glad_glOrtho;
PFNGLDRAWELEMENTSINSTANCEDPROC glad_glDrawElementsInstanced;
PFNGLWINDOWPOS3SVPROC glad_glWindowPos3sv;
PFNGLCLEARINDEXPROC glad_glClearIndex;
PFNGLMAP1DPROC glad_glMap1d;
PFNGLMAP1FPROC glad_glMap1f;
PFNGLFLUSHPROC glad_glFlush;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glad_glGetRenderbufferParameteriv;
PFNGLINDEXIVPROC glad_glIndexiv;
PFNGLRASTERPOS3SVPROC glad_glRasterPos3sv;
PFNGLGETVERTEXATTRIBPOINTERVPROC glad_glGetVertexAttribPointerv;
PFNGLPIXELZOOMPROC glad_glPixelZoom;
PFNGLFENCESYNCPROC glad_glFenceSync;
PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays;
PFNGLVERTEXATTRIB3SVPROC glad_glVertexAttrib3sv;
PFNGLBEGINCONDITIONALRENDERPROC glad_glBeginConditionalRender;
PFNGLDRAWELEMENTSBASEVERTEXPROC glad_glDrawElementsBaseVertex;
PFNGLGETTEXLEVELPARAMETERIVPROC glad_glGetTexLevelParameteriv;
PFNGLLIGHTIPROC glad_glLighti;
PFNGLLIGHTFPROC glad_glLightf;
PFNGLGETATTRIBLOCATIONPROC glad_glGetAttribLocation;
PFNGLSTENCILFUNCSEPARATEPROC glad_glStencilFuncSeparate;
PFNGLCLAMPCOLORPROC glad_glClampColor;
PFNGLUNIFORM4IVPROC glad_glUniform4iv;
PFNGLCLEARSTENCILPROC glad_glClearStencil;
PFNGLMULTITEXCOORD3FVPROC glad_glMultiTexCoord3fv;
PFNGLGETPIXELMAPUIVPROC glad_glGetPixelMapuiv;
PFNGLGENTEXTURESPROC glad_glGenTextures;
PFNGLTEXCOORD4IVPROC glad_glTexCoord4iv;
PFNGLGETTEXPARAMETERIUIVPROC glad_glGetTexParameterIuiv;
PFNGLINDEXPOINTERPROC glad_glIndexPointer;
PFNGLVERTEXATTRIB4NBVPROC glad_glVertexAttrib4Nbv;
PFNGLISSYNCPROC glad_glIsSync;
PFNGLVERTEX2FPROC glad_glVertex2f;
PFNGLVERTEX2DPROC glad_glVertex2d;
PFNGLDELETERENDERBUFFERSPROC glad_glDeleteRenderbuffers;
PFNGLUNIFORM2IPROC glad_glUniform2i;
PFNGLMAPGRID2DPROC glad_glMapGrid2d;
PFNGLMAPGRID2FPROC glad_glMapGrid2f;
PFNGLVERTEX2IPROC glad_glVertex2i;
PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glad_glFramebufferTextureLayer;
PFNGLVERTEX2SPROC glad_glVertex2s;
PFNGLNORMAL3BVPROC glad_glNormal3bv;
PFNGLVERTEXATTRIB4NUIVPROC glad_glVertexAttrib4Nuiv;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glad_glFlushMappedBufferRange;
PFNGLSECONDARYCOLOR3SVPROC glad_glSecondaryColor3sv;
PFNGLVERTEX3SVPROC glad_glVertex3sv;
PFNGLGENQUERIESPROC glad_glGenQueries;
PFNGLGETPIXELMAPFVPROC glad_glGetPixelMapfv;
PFNGLTEXENVFPROC glad_glTexEnvf;
PFNGLTEXSUBIMAGE3DPROC glad_glTexSubImage3D;
PFNGLGETINTEGER64I_VPROC glad_glGetInteger64i_v;
PFNGLFOGCOORDDPROC glad_glFogCoordd;
PFNGLFOGCOORDFPROC glad_glFogCoordf;
PFNGLCOPYTEXIMAGE2DPROC glad_glCopyTexImage2D;
PFNGLTEXENVIPROC glad_glTexEnvi;
PFNGLMULTITEXCOORD1IVPROC glad_glMultiTexCoord1iv;
PFNGLISENABLEDIPROC glad_glIsEnabledi;
PFNGLVERTEXATTRIBI2IPROC glad_glVertexAttribI2i;
PFNGLMULTITEXCOORD2DVPROC glad_glMultiTexCoord2dv;
PFNGLUNIFORM2IVPROC glad_glUniform2iv;
PFNGLVERTEXATTRIB1FVPROC glad_glVertexAttrib1fv;
PFNGLUNIFORM4UIVPROC glad_glUniform4uiv;
PFNGLMATRIXMODEPROC glad_glMatrixMode;
PFNGLFEEDBACKBUFFERPROC glad_glFeedbackBuffer;
PFNGLGETMAPIVPROC glad_glGetMapiv;
PFNGLFRAMEBUFFERTEXTURE1DPROC glad_glFramebufferTexture1D;
PFNGLGETSHADERIVPROC glad_glGetShaderiv;
PFNGLMULTITEXCOORD2DPROC glad_glMultiTexCoord2d;
PFNGLMULTITEXCOORD2FPROC glad_glMultiTexCoord2f;
PFNGLBINDFRAGDATALOCATIONPROC glad_glBindFragDataLocation;
PFNGLPRIORITIZETEXTURESPROC glad_glPrioritizeTextures;
PFNGLCALLLISTPROC glad_glCallList;
PFNGLSECONDARYCOLOR3UBVPROC glad_glSecondaryColor3ubv;
PFNGLGETDOUBLEVPROC glad_glGetDoublev;
PFNGLMULTITEXCOORD3IVPROC glad_glMultiTexCoord3iv;
PFNGLVERTEXATTRIB1DPROC glad_glVertexAttrib1d;
PFNGLLIGHTMODELFPROC glad_glLightModelf;
PFNGLGETUNIFORMIVPROC glad_glGetUniformiv;
PFNGLVERTEX2SVPROC glad_glVertex2sv;
PFNGLLIGHTMODELIPROC glad_glLightModeli;
PFNGLWINDOWPOS3IVPROC glad_glWindowPos3iv;
PFNGLUNIFORM3FVPROC glad_glUniform3fv;
PFNGLPIXELSTOREIPROC glad_glPixelStorei;
PFNGLCALLLISTSPROC glad_glCallLists;
PFNGLMAPBUFFERPROC glad_glMapBuffer;
PFNGLSECONDARYCOLOR3DPROC glad_glSecondaryColor3d;
PFNGLTEXCOORD3IPROC glad_glTexCoord3i;
PFNGLMULTITEXCOORD4FVPROC glad_glMultiTexCoord4fv;
PFNGLRASTERPOS3IPROC glad_glRasterPos3i;
PFNGLSECONDARYCOLOR3BPROC glad_glSecondaryColor3b;
PFNGLRASTERPOS3DPROC glad_glRasterPos3d;
PFNGLRASTERPOS3FPROC glad_glRasterPos3f;
PFNGLCOMPRESSEDTEXIMAGE3DPROC glad_glCompressedTexImage3D;
PFNGLTEXCOORD3FPROC glad_glTexCoord3f;
PFNGLDELETESYNCPROC glad_glDeleteSync;
PFNGLTEXCOORD3DPROC glad_glTexCoord3d;
PFNGLTEXIMAGE2DMULTISAMPLEPROC glad_glTexImage2DMultisample;
PFNGLGETVERTEXATTRIBIVPROC glad_glGetVertexAttribiv;
PFNGLMULTIDRAWELEMENTSPROC glad_glMultiDrawElements;
PFNGLVERTEXATTRIB3FVPROC glad_glVertexAttrib3fv;
PFNGLTEXCOORD3SPROC glad_glTexCoord3s;
PFNGLUNIFORM3IVPROC glad_glUniform3iv;
PFNGLRASTERPOS3SPROC glad_glRasterPos3s;
PFNGLPOLYGONMODEPROC glad_glPolygonMode;
PFNGLDRAWBUFFERSPROC glad_glDrawBuffers;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glad_glGetActiveUniformBlockiv;
PFNGLARETEXTURESRESIDENTPROC glad_glAreTexturesResident;
PFNGLISLISTPROC glad_glIsList;
PFNGLRASTERPOS2SVPROC glad_glRasterPos2sv;
PFNGLRASTERPOS4SVPROC glad_glRasterPos4sv;
PFNGLCOLOR4SPROC glad_glColor4s;
PFNGLUSEPROGRAMPROC glad_glUseProgram;
PFNGLLINESTIPPLEPROC glad_glLineStipple;
PFNGLMULTITEXCOORD1SVPROC glad_glMultiTexCoord1sv;
PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog;
PFNGLGETBUFFERPARAMETERIVPROC glad_glGetBufferParameteriv;
PFNGLMULTITEXCOORD2IVPROC glad_glMultiTexCoord2iv;
PFNGLUNIFORMMATRIX2X4FVPROC glad_glUniformMatrix2x4fv;
PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray;
PFNGLCOLOR4BPROC glad_glColor4b;
PFNGLSECONDARYCOLOR3FPROC glad_glSecondaryColor3f;
PFNGLCOLOR4FPROC glad_glColor4f;
PFNGLCOLOR4DPROC glad_glColor4d;
PFNGLCOLOR4IPROC glad_glColor4i;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glad_glMultiDrawElementsBaseVertex;
PFNGLRASTERPOS3IVPROC glad_glRasterPos3iv;
PFNGLVERTEX2DVPROC glad_glVertex2dv;
PFNGLTEXCOORD4SVPROC glad_glTexCoord4sv;
PFNGLUNIFORM2UIVPROC glad_glUniform2uiv;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glad_glCompressedTexSubImage1D;
PFNGLFINISHPROC glad_glFinish;
PFNGLGETBOOLEANVPROC glad_glGetBooleanv;
PFNGLDELETESHADERPROC glad_glDeleteShader;
PFNGLDRAWELEMENTSPROC glad_glDrawElements;
PFNGLRASTERPOS2SPROC glad_glRasterPos2s;
PFNGLGETMAPDVPROC glad_glGetMapdv;
PFNGLVERTEXATTRIB4NSVPROC glad_glVertexAttrib4Nsv;
PFNGLMATERIALFVPROC glad_glMaterialfv;
PFNGLVIEWPORTPROC glad_glViewport;
PFNGLUNIFORM1UIVPROC glad_glUniform1uiv;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glad_glTransformFeedbackVaryings;
PFNGLINDEXDVPROC glad_glIndexdv;
PFNGLCOPYTEXSUBIMAGE3DPROC glad_glCopyTexSubImage3D;
PFNGLTEXCOORD3IVPROC glad_glTexCoord3iv;
PFNGLVERTEXATTRIBI3IPROC glad_glVertexAttribI3i;
PFNGLCLEARDEPTHPROC glad_glClearDepth;
PFNGLVERTEXATTRIBI4USVPROC glad_glVertexAttribI4usv;
PFNGLTEXPARAMETERFPROC glad_glTexParameterf;
PFNGLTEXPARAMETERIPROC glad_glTexParameteri;
PFNGLGETSHADERSOURCEPROC glad_glGetShaderSource;
PFNGLTEXBUFFERPROC glad_glTexBuffer;
PFNGLPOPNAMEPROC glad_glPopName;
PFNGLVALIDATEPROGRAMPROC glad_glValidateProgram;
PFNGLPIXELSTOREFPROC glad_glPixelStoref;
PFNGLUNIFORM3UIVPROC glad_glUniform3uiv;
PFNGLRASTERPOS4FVPROC glad_glRasterPos4fv;
PFNGLEVALCOORD1DVPROC glad_glEvalCoord1dv;
PFNGLRECTIPROC glad_glRecti;
PFNGLCOLOR4UBPROC glad_glColor4ub;
PFNGLMULTTRANSPOSEMATRIXFPROC glad_glMultTransposeMatrixf;
PFNGLRECTFPROC glad_glRectf;
PFNGLRECTDPROC glad_glRectd;
PFNGLNORMAL3SVPROC glad_glNormal3sv;
PFNGLNEWLISTPROC glad_glNewList;
PFNGLCOLOR4USPROC glad_glColor4us;
PFNGLLINKPROGRAMPROC glad_glLinkProgram;
PFNGLHINTPROC glad_glHint;
PFNGLRECTSPROC glad_glRects;
PFNGLTEXCOORD2DVPROC glad_glTexCoord2dv;
PFNGLRASTERPOS4IVPROC glad_glRasterPos4iv;
PFNGLGETSTRINGPROC glad_glGetString;
PFNGLEDGEFLAGVPROC glad_glEdgeFlagv;
PFNGLDETACHSHADERPROC glad_glDetachShader;
PFNGLSCALEFPROC glad_glScalef;
PFNGLENDQUERYPROC glad_glEndQuery;
PFNGLSCALEDPROC glad_glScaled;
PFNGLEDGEFLAGPOINTERPROC glad_glEdgeFlagPointer;
PFNGLCOPYPIXELSPROC glad_glCopyPixels;
PFNGLVERTEXATTRIBI2UIPROC glad_glVertexAttribI2ui;
PFNGLPOPATTRIBPROC glad_glPopAttrib;
PFNGLDELETETEXTURESPROC glad_glDeleteTextures;
PFNGLSTENCILOPSEPARATEPROC glad_glStencilOpSeparate;
PFNGLDELETEQUERIESPROC glad_glDeleteQueries;
PFNGLVERTEXATTRIB4FPROC glad_glVertexAttrib4f;
PFNGLVERTEXATTRIB4DPROC glad_glVertexAttrib4d;
PFNGLINITNAMESPROC glad_glInitNames;
PFNGLGETBUFFERPARAMETERI64VPROC glad_glGetBufferParameteri64v;
PFNGLCOLOR3DVPROC glad_glColor3dv;
PFNGLVERTEXATTRIBI1IPROC glad_glVertexAttribI1i;
PFNGLGETTEXPARAMETERIVPROC glad_glGetTexParameteriv;
PFNGLWAITSYNCPROC glad_glWaitSync;
PFNGLVERTEXATTRIB4SPROC glad_glVertexAttrib4s;
PFNGLCOLORMATERIALPROC glad_glColorMaterial;
PFNGLSAMPLECOVERAGEPROC glad_glSampleCoverage;
PFNGLUNIFORM1FPROC glad_glUniform1f;
PFNGLGETVERTEXATTRIBFVPROC glad_glGetVertexAttribfv;
PFNGLRENDERMODEPROC glad_glRenderMode;
PFNGLGETCOMPRESSEDTEXIMAGEPROC glad_glGetCompressedTexImage;
PFNGLWINDOWPOS2DVPROC glad_glWindowPos2dv;
PFNGLUNIFORM1IPROC glad_glUniform1i;
PFNGLGETACTIVEATTRIBPROC glad_glGetActiveAttrib;
PFNGLUNIFORM3IPROC glad_glUniform3i;
PFNGLPIXELTRANSFERIPROC glad_glPixelTransferi;
PFNGLTEXSUBIMAGE2DPROC glad_glTexSubImage2D;
PFNGLDISABLEPROC glad_glDisable;
PFNGLLOGICOPPROC glad_glLogicOp;
PFNGLEVALPOINT2PROC glad_glEvalPoint2;
PFNGLPIXELTRANSFERFPROC glad_glPixelTransferf;
PFNGLSECONDARYCOLOR3IPROC glad_glSecondaryColor3i;
PFNGLUNIFORM4UIPROC glad_glUniform4ui;
PFNGLCOLOR3FPROC glad_glColor3f;
PFNGLBINDFRAMEBUFFERPROC glad_glBindFramebuffer;
PFNGLGETTEXENVFVPROC glad_glGetTexEnvfv;
PFNGLRECTFVPROC glad_glRectfv;
PFNGLCULLFACEPROC glad_glCullFace;
PFNGLGETLIGHTFVPROC glad_glGetLightfv;
PFNGLCOLOR3DPROC glad_glColor3d;
PFNGLTEXGENDPROC glad_glTexGend;
PFNGLTEXGENIPROC glad_glTexGeni;
PFNGLMULTITEXCOORD3SPROC glad_glMultiTexCoord3s;
PFNGLGETSTRINGIPROC glad_glGetStringi;
PFNGLMULTITEXCOORD3IPROC glad_glMultiTexCoord3i;
PFNGLMULTITEXCOORD3FPROC glad_glMultiTexCoord3f;
PFNGLMULTITEXCOORD3DPROC glad_glMultiTexCoord3d;
PFNGLATTACHSHADERPROC glad_glAttachShader;
PFNGLFOGCOORDDVPROC glad_glFogCoorddv;
PFNGLUNIFORMMATRIX2X3FVPROC glad_glUniformMatrix2x3fv;
PFNGLGETTEXGENFVPROC glad_glGetTexGenfv;
PFNGLFOGCOORDPOINTERPROC glad_glFogCoordPointer;
PFNGLPROVOKINGVERTEXPROC glad_glProvokingVertex;
PFNGLFRAMEBUFFERTEXTURE3DPROC glad_glFramebufferTexture3D;
PFNGLTEXGENIVPROC glad_glTexGeniv;
PFNGLRASTERPOS2DVPROC glad_glRasterPos2dv;
PFNGLSECONDARYCOLOR3DVPROC glad_glSecondaryColor3dv;
PFNGLCLIENTACTIVETEXTUREPROC glad_glClientActiveTexture;
PFNGLVERTEXATTRIBI4SVPROC glad_glVertexAttribI4sv;
PFNGLSECONDARYCOLOR3USPROC glad_glSecondaryColor3us;
PFNGLTEXENVFVPROC glad_glTexEnvfv;
PFNGLREADBUFFERPROC glad_glReadBuffer;
PFNGLTEXPARAMETERIUIVPROC glad_glTexParameterIuiv;
PFNGLDRAWARRAYSINSTANCEDPROC glad_glDrawArraysInstanced;
PFNGLGENERATEMIPMAPPROC glad_glGenerateMipmap;
PFNGLWINDOWPOS3FVPROC glad_glWindowPos3fv;
PFNGLLIGHTMODELFVPROC glad_glLightModelfv;
PFNGLDELETELISTSPROC glad_glDeleteLists;
PFNGLGETCLIPPLANEPROC glad_glGetClipPlane;
PFNGLVERTEX4DVPROC glad_glVertex4dv;
PFNGLTEXCOORD2DPROC glad_glTexCoord2d;
PFNGLPOPMATRIXPROC glad_glPopMatrix;
PFNGLTEXCOORD2FPROC glad_glTexCoord2f;
PFNGLCOLOR4IVPROC glad_glColor4iv;
PFNGLINDEXUBVPROC glad_glIndexubv;
PFNGLUNMAPBUFFERPROC glad_glUnmapBuffer;
PFNGLTEXCOORD2IPROC glad_glTexCoord2i;
PFNGLRASTERPOS4DPROC glad_glRasterPos4d;
PFNGLRASTERPOS4FPROC glad_glRasterPos4f;
PFNGLVERTEXATTRIB3SPROC glad_glVertexAttrib3s;
PFNGLTEXCOORD2SPROC glad_glTexCoord2s;
PFNGLBINDRENDERBUFFERPROC glad_glBindRenderbuffer;
PFNGLVERTEX3FVPROC glad_glVertex3fv;
PFNGLTEXCOORD4DVPROC glad_glTexCoord4dv;
PFNGLMATERIALIVPROC glad_glMaterialiv;
PFNGLISPROGRAMPROC glad_glIsProgram;
PFNGLVERTEXATTRIB4BVPROC glad_glVertexAttrib4bv;
PFNGLVERTEX4SPROC glad_glVertex4s;
PFNGLVERTEXATTRIB4FVPROC glad_glVertexAttrib4fv;
PFNGLNORMAL3DVPROC glad_glNormal3dv;
PFNGLUNIFORM4IPROC glad_glUniform4i;
PFNGLACTIVETEXTUREPROC glad_glActiveTexture;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;
PFNGLROTATEDPROC glad_glRotated;
PFNGLROTATEFPROC glad_glRotatef;
PFNGLVERTEX4IPROC glad_glVertex4i;
PFNGLREADPIXELSPROC glad_glReadPixels;
PFNGLVERTEXATTRIBI3IVPROC glad_glVertexAttribI3iv;
PFNGLLOADNAMEPROC glad_glLoadName;
PFNGLUNIFORM4FPROC glad_glUniform4f;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glad_glRenderbufferStorageMultisample;
PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays;
PFNGLSHADEMODELPROC glad_glShadeModel;
PFNGLMAPGRID1DPROC glad_glMapGrid1d;
PFNGLGETUNIFORMFVPROC glad_glGetUniformfv;
PFNGLMAPGRID1FPROC glad_glMapGrid1f;
PFNGLDISABLECLIENTSTATEPROC glad_glDisableClientState;
PFNGLMULTITEXCOORD3SVPROC glad_glMultiTexCoord3sv;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glad_glDrawElementsInstancedBaseVertex;
PFNGLSECONDARYCOLORPOINTERPROC glad_glSecondaryColorPointer;
PFNGLALPHAFUNCPROC glad_glAlphaFunc;
PFNGLUNIFORM1IVPROC glad_glUniform1iv;
PFNGLMULTITEXCOORD4IVPROC glad_glMultiTexCoord4iv;
PFNGLGETQUERYOBJECTIVPROC glad_glGetQueryObjectiv;
PFNGLSTENCILFUNCPROC glad_glStencilFunc;
PFNGLMULTITEXCOORD1FVPROC glad_glMultiTexCoord1fv;
PFNGLUNIFORMBLOCKBINDINGPROC glad_glUniformBlockBinding;
PFNGLCOLOR4UIVPROC glad_glColor4uiv;
PFNGLRECTIVPROC glad_glRectiv;
PFNGLRASTERPOS3DVPROC glad_glRasterPos3dv;
PFNGLEVALMESH2PROC glad_glEvalMesh2;
PFNGLEVALMESH1PROC glad_glEvalMesh1;
PFNGLTEXCOORDPOINTERPROC glad_glTexCoordPointer;
PFNGLVERTEXATTRIB4NUBVPROC glad_glVertexAttrib4Nubv;
PFNGLVERTEXATTRIBI4IVPROC glad_glVertexAttribI4iv;
PFNGLEVALCOORD2FVPROC glad_glEvalCoord2fv;
PFNGLCOLOR4UBVPROC glad_glColor4ubv;
PFNGLLOADTRANSPOSEMATRIXDPROC glad_glLoadTransposeMatrixd;
PFNGLLOADTRANSPOSEMATRIXFPROC glad_glLoadTransposeMatrixf;
PFNGLVERTEXATTRIBI4IPROC glad_glVertexAttribI4i;
PFNGLRASTERPOS2IVPROC glad_glRasterPos2iv;
PFNGLGETBUFFERSUBDATAPROC glad_glGetBufferSubData;
PFNGLTEXENVIVPROC glad_glTexEnviv;
PFNGLBLENDEQUATIONSEPARATEPROC glad_glBlendEquationSeparate;
PFNGLVERTEXATTRIBI1UIPROC glad_glVertexAttribI1ui;
PFNGLGENBUFFERSPROC glad_glGenBuffers;
PFNGLSELECTBUFFERPROC glad_glSelectBuffer;
PFNGLVERTEXATTRIB2SVPROC glad_glVertexAttrib2sv;
PFNGLPUSHATTRIBPROC glad_glPushAttrib;
PFNGLVERTEXATTRIBIPOINTERPROC glad_glVertexAttribIPointer;
PFNGLBLENDFUNCPROC glad_glBlendFunc;
PFNGLCREATEPROGRAMPROC glad_glCreateProgram;
PFNGLTEXIMAGE3DPROC glad_glTexImage3D;
PFNGLISFRAMEBUFFERPROC glad_glIsFramebuffer;
PFNGLLIGHTIVPROC glad_glLightiv;
PFNGLPRIMITIVERESTARTINDEXPROC glad_glPrimitiveRestartIndex;
PFNGLTEXGENFVPROC glad_glTexGenfv;
PFNGLENDPROC glad_glEnd;
PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers;
PFNGLSCISSORPROC glad_glScissor;
PFNGLCLIPPLANEPROC glad_glClipPlane;
PFNGLPUSHNAMEPROC glad_glPushName;
PFNGLTEXGENDVPROC glad_glTexGendv;
PFNGLINDEXUBPROC glad_glIndexub;
PFNGLSECONDARYCOLOR3IVPROC glad_glSecondaryColor3iv;
PFNGLRASTERPOS4IPROC glad_glRasterPos4i;
PFNGLMULTTRANSPOSEMATRIXDPROC glad_glMultTransposeMatrixd;
PFNGLCLEARCOLORPROC glad_glClearColor;
PFNGLVERTEXATTRIB4UIVPROC glad_glVertexAttrib4uiv;
PFNGLNORMAL3SPROC glad_glNormal3s;
PFNGLVERTEXATTRIB4NIVPROC glad_glVertexAttrib4Niv;
PFNGLCLEARBUFFERIVPROC glad_glClearBufferiv;
PFNGLPOINTPARAMETERIPROC glad_glPointParameteri;
PFNGLBLENDCOLORPROC glad_glBlendColor;
PFNGLWINDOWPOS3DPROC glad_glWindowPos3d;
PFNGLVERTEXATTRIBI2UIVPROC glad_glVertexAttribI2uiv;
PFNGLUNIFORM3UIPROC glad_glUniform3ui;
PFNGLCOLOR4DVPROC glad_glColor4dv;
PFNGLVERTEXATTRIBI4UIVPROC glad_glVertexAttribI4uiv;
PFNGLPOINTPARAMETERFVPROC glad_glPointParameterfv;
PFNGLUNIFORM2FVPROC glad_glUniform2fv;
PFNGLSECONDARYCOLOR3UBPROC glad_glSecondaryColor3ub;
PFNGLSECONDARYCOLOR3UIPROC glad_glSecondaryColor3ui;
PFNGLTEXCOORD3DVPROC glad_glTexCoord3dv;
PFNGLBINDBUFFERRANGEPROC glad_glBindBufferRange;
PFNGLNORMAL3IVPROC glad_glNormal3iv;
PFNGLWINDOWPOS3SPROC glad_glWindowPos3s;
PFNGLPOINTPARAMETERFPROC glad_glPointParameterf;
PFNGLGETVERTEXATTRIBIUIVPROC glad_glGetVertexAttribIuiv;
PFNGLWINDOWPOS3IPROC glad_glWindowPos3i;
PFNGLMULTITEXCOORD4SPROC glad_glMultiTexCoord4s;
PFNGLWINDOWPOS3FPROC glad_glWindowPos3f;
PFNGLCOLOR3USPROC glad_glColor3us;
PFNGLCOLOR3UIVPROC glad_glColor3uiv;
PFNGLVERTEXATTRIB4NUSVPROC glad_glVertexAttrib4Nusv;
PFNGLGETLIGHTIVPROC glad_glGetLightiv;
PFNGLDEPTHFUNCPROC glad_glDepthFunc;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glad_glCompressedTexSubImage2D;
PFNGLLISTBASEPROC glad_glListBase;
PFNGLMULTITEXCOORD4FPROC glad_glMultiTexCoord4f;
PFNGLCOLOR3UBPROC glad_glColor3ub;
PFNGLMULTITEXCOORD4DPROC glad_glMultiTexCoord4d;
PFNGLVERTEXATTRIBI4BVPROC glad_glVertexAttribI4bv;
PFNGLGETTEXPARAMETERFVPROC glad_glGetTexParameterfv;
PFNGLCOLOR3UIPROC glad_glColor3ui;
PFNGLMULTITEXCOORD4IPROC glad_glMultiTexCoord4i;
PFNGLGETPOLYGONSTIPPLEPROC glad_glGetPolygonStipple;
PFNGLCLIENTWAITSYNCPROC glad_glClientWaitSync;
PFNGLVERTEXATTRIBI4UIPROC glad_glVertexAttribI4ui;
PFNGLMULTITEXCOORD4DVPROC glad_glMultiTexCoord4dv;
PFNGLCOLORMASKPROC glad_glColorMask;
PFNGLTEXPARAMETERIIVPROC glad_glTexParameterIiv;
PFNGLBLENDEQUATIONPROC glad_glBlendEquation;
PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation;
PFNGLRASTERPOS4SPROC glad_glRasterPos4s;
PFNGLENDTRANSFORMFEEDBACKPROC glad_glEndTransformFeedback;
PFNGLVERTEXATTRIB4USVPROC glad_glVertexAttrib4usv;
PFNGLMULTITEXCOORD3DVPROC glad_glMultiTexCoord3dv;
PFNGLCOLOR4SVPROC glad_glColor4sv;
PFNGLPOPCLIENTATTRIBPROC glad_glPopClientAttrib;
PFNGLBEGINTRANSFORMFEEDBACKPROC glad_glBeginTransformFeedback;
PFNGLFOGFPROC glad_glFogf;
PFNGLVERTEXATTRIBI1IVPROC glad_glVertexAttribI1iv;
PFNGLCOLOR3IVPROC glad_glColor3iv;
PFNGLCOMPRESSEDTEXIMAGE1DPROC glad_glCompressedTexImage1D;
PFNGLCOPYTEXSUBIMAGE1DPROC glad_glCopyTexSubImage1D;
PFNGLTEXCOORD1IPROC glad_glTexCoord1i;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_glCheckFramebufferStatus;
PFNGLTEXCOORD1DPROC glad_glTexCoord1d;
PFNGLTEXCOORD1FPROC glad_glTexCoord1f;
PFNGLENDCONDITIONALRENDERPROC glad_glEndConditionalRender;
PFNGLENABLECLIENTSTATEPROC glad_glEnableClientState;
PFNGLBINDATTRIBLOCATIONPROC glad_glBindAttribLocation;
PFNGLUNIFORMMATRIX4X2FVPROC glad_glUniformMatrix4x2fv;
PFNGLMULTITEXCOORD2SVPROC glad_glMultiTexCoord2sv;
PFNGLVERTEXATTRIB1DVPROC glad_glVertexAttrib1dv;
PFNGLDRAWRANGEELEMENTSPROC glad_glDrawRangeElements;
PFNGLTEXCOORD1SPROC glad_glTexCoord1s;
PFNGLBINDBUFFERBASEPROC glad_glBindBufferBase;
PFNGLBUFFERSUBDATAPROC glad_glBufferSubData;
PFNGLVERTEXATTRIB4IVPROC glad_glVertexAttrib4iv;
PFNGLGENLISTSPROC glad_glGenLists;
PFNGLCOLOR3BVPROC glad_glColor3bv;
PFNGLMAPBUFFERRANGEPROC glad_glMapBufferRange;
PFNGLFRAMEBUFFERTEXTUREPROC glad_glFramebufferTexture;
PFNGLGETTEXGENDVPROC glad_glGetTexGendv;
PFNGLMULTIDRAWARRAYSPROC glad_glMultiDrawArrays;
PFNGLENDLISTPROC glad_glEndList;
PFNGLUNIFORM2UIPROC glad_glUniform2ui;
PFNGLVERTEXATTRIBI2IVPROC glad_glVertexAttribI2iv;
PFNGLCOLOR3USVPROC glad_glColor3usv;
PFNGLWINDOWPOS2FVPROC glad_glWindowPos2fv;
PFNGLDISABLEIPROC glad_glDisablei;
PFNGLINDEXMASKPROC glad_glIndexMask;
PFNGLPUSHCLIENTATTRIBPROC glad_glPushClientAttrib;
PFNGLSHADERSOURCEPROC glad_glShaderSource;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glad_glGetActiveUniformBlockName;
PFNGLVERTEXATTRIBI3UIVPROC glad_glVertexAttribI3uiv;
PFNGLCLEARACCUMPROC glad_glClearAccum;
PFNGLGETSYNCIVPROC glad_glGetSynciv;
PFNGLUNIFORM2FPROC glad_glUniform2f;
PFNGLBEGINQUERYPROC glad_glBeginQuery;
PFNGLGETUNIFORMBLOCKINDEXPROC glad_glGetUniformBlockIndex;
PFNGLBINDBUFFERPROC glad_glBindBuffer;
PFNGLMAP2DPROC glad_glMap2d;
PFNGLMAP2FPROC glad_glMap2f;
PFNGLVERTEX4DPROC glad_glVertex4d;
PFNGLUNIFORMMATRIX2FVPROC glad_glUniformMatrix2fv;
PFNGLTEXCOORD1SVPROC glad_glTexCoord1sv;
PFNGLBUFFERDATAPROC glad_glBufferData;
PFNGLEVALPOINT1PROC glad_glEvalPoint1;
PFNGLGETTEXPARAMETERIIVPROC glad_glGetTexParameterIiv;
PFNGLTEXCOORD1DVPROC glad_glTexCoord1dv;
PFNGLGETERRORPROC glad_glGetError;
PFNGLGETTEXENVIVPROC glad_glGetTexEnviv;
PFNGLGETPROGRAMIVPROC glad_glGetProgramiv;
PFNGLGETFLOATVPROC glad_glGetFloatv;
PFNGLTEXSUBIMAGE1DPROC glad_glTexSubImage1D;
PFNGLMULTITEXCOORD2FVPROC glad_glMultiTexCoord2fv;
PFNGLVERTEXATTRIB2FVPROC glad_glVertexAttrib2fv;
PFNGLEVALCOORD1DPROC glad_glEvalCoord1d;
PFNGLGETTEXLEVELPARAMETERFVPROC glad_glGetTexLevelParameterfv;
PFNGLEVALCOORD1FPROC glad_glEvalCoord1f;
PFNGLPIXELMAPFVPROC glad_glPixelMapfv;
PFNGLGETPIXELMAPUSVPROC glad_glGetPixelMapusv;
PFNGLGETINTEGERVPROC glad_glGetIntegerv;
PFNGLACCUMPROC glad_glAccum;
PFNGLGETBUFFERPOINTERVPROC glad_glGetBufferPointerv;
PFNGLGETVERTEXATTRIBIIVPROC glad_glGetVertexAttribIiv;
PFNGLRASTERPOS4DVPROC glad_glRasterPos4dv;
PFNGLTEXCOORD2IVPROC glad_glTexCoord2iv;
PFNGLISQUERYPROC glad_glIsQuery;
PFNGLVERTEXATTRIB4SVPROC glad_glVertexAttrib4sv;
PFNGLWINDOWPOS3DVPROC glad_glWindowPos3dv;
PFNGLTEXIMAGE2DPROC glad_glTexImage2D;
PFNGLSTENCILMASKPROC glad_glStencilMask;
PFNGLDRAWPIXELSPROC glad_glDrawPixels;
PFNGLMULTMATRIXDPROC glad_glMultMatrixd;
PFNGLMULTMATRIXFPROC glad_glMultMatrixf;
PFNGLISTEXTUREPROC glad_glIsTexture;
PFNGLGETMATERIALIVPROC glad_glGetMaterialiv;
PFNGLUNIFORM1FVPROC glad_glUniform1fv;
PFNGLLOADMATRIXFPROC glad_glLoadMatrixf;
PFNGLLOADMATRIXDPROC glad_glLoadMatrixd;
PFNGLTEXPARAMETERFVPROC glad_glTexParameterfv;
PFNGLUNIFORMMATRIX3FVPROC glad_glUniformMatrix3fv;
PFNGLVERTEX4FPROC glad_glVertex4f;
PFNGLRECTSVPROC glad_glRectsv;
PFNGLCOLOR4USVPROC glad_glColor4usv;
PFNGLPOLYGONSTIPPLEPROC glad_glPolygonStipple;
PFNGLINTERLEAVEDARRAYSPROC glad_glInterleavedArrays;
PFNGLNORMAL3IPROC glad_glNormal3i;
PFNGLNORMAL3FPROC glad_glNormal3f;
PFNGLNORMAL3DPROC glad_glNormal3d;
PFNGLNORMAL3BPROC glad_glNormal3b;
PFNGLPIXELMAPUSVPROC glad_glPixelMapusv;
PFNGLGETTEXGENIVPROC glad_glGetTexGeniv;
PFNGLARRAYELEMENTPROC glad_glArrayElement;
PFNGLCOPYBUFFERSUBDATAPROC glad_glCopyBufferSubData;
PFNGLVERTEXATTRIBI1UIVPROC glad_glVertexAttribI1uiv;
PFNGLVERTEXATTRIB2DPROC glad_glVertexAttrib2d;
PFNGLVERTEXATTRIB2FPROC glad_glVertexAttrib2f;
PFNGLVERTEXATTRIB3DVPROC glad_glVertexAttrib3dv;
PFNGLDEPTHMASKPROC glad_glDepthMask;
PFNGLVERTEXATTRIB2SPROC glad_glVertexAttrib2s;
PFNGLCOLOR3FVPROC glad_glColor3fv;
PFNGLTEXIMAGE3DMULTISAMPLEPROC glad_glTexImage3DMultisample;
PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv;
PFNGLUNIFORM4FVPROC glad_glUniform4fv;
PFNGLGETACTIVEUNIFORMPROC glad_glGetActiveUniform;
PFNGLCOLORPOINTERPROC glad_glColorPointer;
PFNGLFRONTFACEPROC glad_glFrontFace;
PFNGLGETBOOLEANI_VPROC glad_glGetBooleani_v;
PFNGLCLEARBUFFERUIVPROC glad_glClearBufferuiv;
int GLAD_GL_ARB_robustness;
int GLAD_GL_ARB_multisample;
int GLAD_GL_EXT_separate_specular_color;
PFNGLSAMPLECOVERAGEARBPROC glad_glSampleCoverageARB;
PFNGLGETGRAPHICSRESETSTATUSARBPROC glad_glGetGraphicsResetStatusARB;
PFNGLGETNTEXIMAGEARBPROC glad_glGetnTexImageARB;
PFNGLREADNPIXELSARBPROC glad_glReadnPixelsARB;
PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC glad_glGetnCompressedTexImageARB;
PFNGLGETNUNIFORMFVARBPROC glad_glGetnUniformfvARB;
PFNGLGETNUNIFORMIVARBPROC glad_glGetnUniformivARB;
PFNGLGETNUNIFORMUIVARBPROC glad_glGetnUniformuivARB;
PFNGLGETNUNIFORMDVARBPROC glad_glGetnUniformdvARB;
PFNGLGETNMAPDVARBPROC glad_glGetnMapdvARB;
PFNGLGETNMAPFVARBPROC glad_glGetnMapfvARB;
PFNGLGETNMAPIVARBPROC glad_glGetnMapivARB;
PFNGLGETNPIXELMAPFVARBPROC glad_glGetnPixelMapfvARB;
PFNGLGETNPIXELMAPUIVARBPROC glad_glGetnPixelMapuivARB;
PFNGLGETNPIXELMAPUSVARBPROC glad_glGetnPixelMapusvARB;
PFNGLGETNPOLYGONSTIPPLEARBPROC glad_glGetnPolygonStippleARB;
PFNGLGETNCOLORTABLEARBPROC glad_glGetnColorTableARB;
PFNGLGETNCONVOLUTIONFILTERARBPROC glad_glGetnConvolutionFilterARB;
PFNGLGETNSEPARABLEFILTERARBPROC glad_glGetnSeparableFilterARB;
PFNGLGETNHISTOGRAMARBPROC glad_glGetnHistogramARB;
PFNGLGETNMINMAXARBPROC glad_glGetnMinmaxARB;
static void load_GL_VERSION_1_0(GLADloadproc load) {
  if (!GLAD_GL_VERSION_1_0)
    return;
  glad_glCullFace = (PFNGLCULLFACEPROC)load("glCullFace");
  glad_glFrontFace = (PFNGLFRONTFACEPROC)load("glFrontFace");
  glad_glHint = (PFNGLHINTPROC)load("glHint");
  glad_glLineWidth = (PFNGLLINEWIDTHPROC)load("glLineWidth");
  glad_glPointSize = (PFNGLPOINTSIZEPROC)load("glPointSize");
  glad_glPolygonMode = (PFNGLPOLYGONMODEPROC)load("glPolygonMode");
  glad_glScissor = (PFNGLSCISSORPROC)load("glScissor");
  glad_glTexParameterf = (PFNGLTEXPARAMETERFPROC)load("glTexParameterf");
  glad_glTexParameterfv = (PFNGLTEXPARAMETERFVPROC)load("glTexParameterfv");
  glad_glTexParameteri = (PFNGLTEXPARAMETERIPROC)load("glTexParameteri");
  glad_glTexParameteriv = (PFNGLTEXPARAMETERIVPROC)load("glTexParameteriv");
  glad_glTexImage1D = (PFNGLTEXIMAGE1DPROC)load("glTexImage1D");
  glad_glTexImage2D = (PFNGLTEXIMAGE2DPROC)load("glTexImage2D");
  glad_glDrawBuffer = (PFNGLDRAWBUFFERPROC)load("glDrawBuffer");
  glad_glClear = (PFNGLCLEARPROC)load("glClear");
  glad_glClearColor = (PFNGLCLEARCOLORPROC)load("glClearColor");
  glad_glClearStencil = (PFNGLCLEARSTENCILPROC)load("glClearStencil");
  glad_glClearDepth = (PFNGLCLEARDEPTHPROC)load("glClearDepth");
  glad_glStencilMask = (PFNGLSTENCILMASKPROC)load("glStencilMask");
  glad_glColorMask = (PFNGLCOLORMASKPROC)load("glColorMask");
  glad_glDepthMask = (PFNGLDEPTHMASKPROC)load("glDepthMask");
  glad_glDisable = (PFNGLDISABLEPROC)load("glDisable");
  glad_glEnable = (PFNGLENABLEPROC)load("glEnable");
  glad_glFinish = (PFNGLFINISHPROC)load("glFinish");
  glad_glFlush = (PFNGLFLUSHPROC)load("glFlush");
  glad_glBlendFunc = (PFNGLBLENDFUNCPROC)load("glBlendFunc");
  glad_glLogicOp = (PFNGLLOGICOPPROC)load("glLogicOp");
  glad_glStencilFunc = (PFNGLSTENCILFUNCPROC)load("glStencilFunc");
  glad_glStencilOp = (PFNGLSTENCILOPPROC)load("glStencilOp");
  glad_glDepthFunc = (PFNGLDEPTHFUNCPROC)load("glDepthFunc");
  glad_glPixelStoref = (PFNGLPIXELSTOREFPROC)load("glPixelStoref");
  glad_glPixelStorei = (PFNGLPIXELSTOREIPROC)load("glPixelStorei");
  glad_glReadBuffer = (PFNGLREADBUFFERPROC)load("glReadBuffer");
  glad_glReadPixels = (PFNGLREADPIXELSPROC)load("glReadPixels");
  glad_glGetBooleanv = (PFNGLGETBOOLEANVPROC)load("glGetBooleanv");
  glad_glGetDoublev = (PFNGLGETDOUBLEVPROC)load("glGetDoublev");
  glad_glGetError = (PFNGLGETERRORPROC)load("glGetError");
  glad_glGetFloatv = (PFNGLGETFLOATVPROC)load("glGetFloatv");
  glad_glGetIntegerv = (PFNGLGETINTEGERVPROC)load("glGetIntegerv");
  glad_glGetString = (PFNGLGETSTRINGPROC)load("glGetString");
  glad_glGetTexImage = (PFNGLGETTEXIMAGEPROC)load("glGetTexImage");
  glad_glGetTexParameterfv =
      (PFNGLGETTEXPARAMETERFVPROC)load("glGetTexParameterfv");
  glad_glGetTexParameteriv =
      (PFNGLGETTEXPARAMETERIVPROC)load("glGetTexParameteriv");
  glad_glGetTexLevelParameterfv =
      (PFNGLGETTEXLEVELPARAMETERFVPROC)load("glGetTexLevelParameterfv");
  glad_glGetTexLevelParameteriv =
      (PFNGLGETTEXLEVELPARAMETERIVPROC)load("glGetTexLevelParameteriv");
  glad_glIsEnabled = (PFNGLISENABLEDPROC)load("glIsEnabled");
  glad_glDepthRange = (PFNGLDEPTHRANGEPROC)load("glDepthRange");
  glad_glViewport = (PFNGLVIEWPORTPROC)load("glViewport");
  glad_glNewList = (PFNGLNEWLISTPROC)load("glNewList");
  glad_glEndList = (PFNGLENDLISTPROC)load("glEndList");
  glad_glCallList = (PFNGLCALLLISTPROC)load("glCallList");
  glad_glCallLists = (PFNGLCALLLISTSPROC)load("glCallLists");
  glad_glDeleteLists = (PFNGLDELETELISTSPROC)load("glDeleteLists");
  glad_glGenLists = (PFNGLGENLISTSPROC)load("glGenLists");
  glad_glListBase = (PFNGLLISTBASEPROC)load("glListBase");
  glad_glBegin = (PFNGLBEGINPROC)load("glBegin");
  glad_glBitmap = (PFNGLBITMAPPROC)load("glBitmap");
  glad_glColor3b = (PFNGLCOLOR3BPROC)load("glColor3b");
  glad_glColor3bv = (PFNGLCOLOR3BVPROC)load("glColor3bv");
  glad_glColor3d = (PFNGLCOLOR3DPROC)load("glColor3d");
  glad_glColor3dv = (PFNGLCOLOR3DVPROC)load("glColor3dv");
  glad_glColor3f = (PFNGLCOLOR3FPROC)load("glColor3f");
  glad_glColor3fv = (PFNGLCOLOR3FVPROC)load("glColor3fv");
  glad_glColor3i = (PFNGLCOLOR3IPROC)load("glColor3i");
  glad_glColor3iv = (PFNGLCOLOR3IVPROC)load("glColor3iv");
  glad_glColor3s = (PFNGLCOLOR3SPROC)load("glColor3s");
  glad_glColor3sv = (PFNGLCOLOR3SVPROC)load("glColor3sv");
  glad_glColor3ub = (PFNGLCOLOR3UBPROC)load("glColor3ub");
  glad_glColor3ubv = (PFNGLCOLOR3UBVPROC)load("glColor3ubv");
  glad_glColor3ui = (PFNGLCOLOR3UIPROC)load("glColor3ui");
  glad_glColor3uiv = (PFNGLCOLOR3UIVPROC)load("glColor3uiv");
  glad_glColor3us = (PFNGLCOLOR3USPROC)load("glColor3us");
  glad_glColor3usv = (PFNGLCOLOR3USVPROC)load("glColor3usv");
  glad_glColor4b = (PFNGLCOLOR4BPROC)load("glColor4b");
  glad_glColor4bv = (PFNGLCOLOR4BVPROC)load("glColor4bv");
  glad_glColor4d = (PFNGLCOLOR4DPROC)load("glColor4d");
  glad_glColor4dv = (PFNGLCOLOR4DVPROC)load("glColor4dv");
  glad_glColor4f = (PFNGLCOLOR4FPROC)load("glColor4f");
  glad_glColor4fv = (PFNGLCOLOR4FVPROC)load("glColor4fv");
  glad_glColor4i = (PFNGLCOLOR4IPROC)load("glColor4i");
  glad_glColor4iv = (PFNGLCOLOR4IVPROC)load("glColor4iv");
  glad_glColor4s = (PFNGLCOLOR4SPROC)load("glColor4s");
  glad_glColor4sv = (PFNGLCOLOR4SVPROC)load("glColor4sv");
  glad_glColor4ub = (PFNGLCOLOR4UBPROC)load("glColor4ub");
  glad_glColor4ubv = (PFNGLCOLOR4UBVPROC)load("glColor4ubv");
  glad_glColor4ui = (PFNGLCOLOR4UIPROC)load("glColor4ui");
  glad_glColor4uiv = (PFNGLCOLOR4UIVPROC)load("glColor4uiv");
  glad_glColor4us = (PFNGLCOLOR4USPROC)load("glColor4us");
  glad_glColor4usv = (PFNGLCOLOR4USVPROC)load("glColor4usv");
  glad_glEdgeFlag = (PFNGLEDGEFLAGPROC)load("glEdgeFlag");
  glad_glEdgeFlagv = (PFNGLEDGEFLAGVPROC)load("glEdgeFlagv");
  glad_glEnd = (PFNGLENDPROC)load("glEnd");
  glad_glIndexd = (PFNGLINDEXDPROC)load("glIndexd");
  glad_glIndexdv = (PFNGLINDEXDVPROC)load("glIndexdv");
  glad_glIndexf = (PFNGLINDEXFPROC)load("glIndexf");
  glad_glIndexfv = (PFNGLINDEXFVPROC)load("glIndexfv");
  glad_glIndexi = (PFNGLINDEXIPROC)load("glIndexi");
  glad_glIndexiv = (PFNGLINDEXIVPROC)load("glIndexiv");
  glad_glIndexs = (PFNGLINDEXSPROC)load("glIndexs");
  glad_glIndexsv = (PFNGLINDEXSVPROC)load("glIndexsv");
  glad_glNormal3b = (PFNGLNORMAL3BPROC)load("glNormal3b");
  glad_glNormal3bv = (PFNGLNORMAL3BVPROC)load("glNormal3bv");
  glad_glNormal3d = (PFNGLNORMAL3DPROC)load("glNormal3d");
  glad_glNormal3dv = (PFNGLNORMAL3DVPROC)load("glNormal3dv");
  glad_glNormal3f = (PFNGLNORMAL3FPROC)load("glNormal3f");
  glad_glNormal3fv = (PFNGLNORMAL3FVPROC)load("glNormal3fv");
  glad_glNormal3i = (PFNGLNORMAL3IPROC)load("glNormal3i");
  glad_glNormal3iv = (PFNGLNORMAL3IVPROC)load("glNormal3iv");
  glad_glNormal3s = (PFNGLNORMAL3SPROC)load("glNormal3s");
  glad_glNormal3sv = (PFNGLNORMAL3SVPROC)load("glNormal3sv");
  glad_glRasterPos2d = (PFNGLRASTERPOS2DPROC)load("glRasterPos2d");
  glad_glRasterPos2dv = (PFNGLRASTERPOS2DVPROC)load("glRasterPos2dv");
  glad_glRasterPos2f = (PFNGLRASTERPOS2FPROC)load("glRasterPos2f");
  glad_glRasterPos2fv = (PFNGLRASTERPOS2FVPROC)load("glRasterPos2fv");
  glad_glRasterPos2i = (PFNGLRASTERPOS2IPROC)load("glRasterPos2i");
  glad_glRasterPos2iv = (PFNGLRASTERPOS2IVPROC)load("glRasterPos2iv");
  glad_glRasterPos2s = (PFNGLRASTERPOS2SPROC)load("glRasterPos2s");
  glad_glRasterPos2sv = (PFNGLRASTERPOS2SVPROC)load("glRasterPos2sv");
  glad_glRasterPos3d = (PFNGLRASTERPOS3DPROC)load("glRasterPos3d");
  glad_glRasterPos3dv = (PFNGLRASTERPOS3DVPROC)load("glRasterPos3dv");
  glad_glRasterPos3f = (PFNGLRASTERPOS3FPROC)load("glRasterPos3f");
  glad_glRasterPos3fv = (PFNGLRASTERPOS3FVPROC)load("glRasterPos3fv");
  glad_glRasterPos3i = (PFNGLRASTERPOS3IPROC)load("glRasterPos3i");
  glad_glRasterPos3iv = (PFNGLRASTERPOS3IVPROC)load("glRasterPos3iv");
  glad_glRasterPos3s = (PFNGLRASTERPOS3SPROC)load("glRasterPos3s");
  glad_glRasterPos3sv = (PFNGLRASTERPOS3SVPROC)load("glRasterPos3sv");
  glad_glRasterPos4d = (PFNGLRASTERPOS4DPROC)load("glRasterPos4d");
  glad_glRasterPos4dv = (PFNGLRASTERPOS4DVPROC)load("glRasterPos4dv");
  glad_glRasterPos4f = (PFNGLRASTERPOS4FPROC)load("glRasterPos4f");
  glad_glRasterPos4fv = (PFNGLRASTERPOS4FVPROC)load("glRasterPos4fv");
  glad_glRasterPos4i = (PFNGLRASTERPOS4IPROC)load("glRasterPos4i");
  glad_glRasterPos4iv = (PFNGLRASTERPOS4IVPROC)load("glRasterPos4iv");
  glad_glRasterPos4s = (PFNGLRASTERPOS4SPROC)load("glRasterPos4s");
  glad_glRasterPos4sv = (PFNGLRASTERPOS4SVPROC)load("glRasterPos4sv");
  glad_glRectd = (PFNGLRECTDPROC)load("glRectd");
  glad_glRectdv = (PFNGLRECTDVPROC)load("glRectdv");
  glad_glRectf = (PFNGLRECTFPROC)load("glRectf");
  glad_glRectfv = (PFNGLRECTFVPROC)load("glRectfv");
  glad_glRecti = (PFNGLRECTIPROC)load("glRecti");
  glad_glRectiv = (PFNGLRECTIVPROC)load("glRectiv");
  glad_glRects = (PFNGLRECTSPROC)load("glRects");
  glad_glRectsv = (PFNGLRECTSVPROC)load("glRectsv");
  glad_glTexCoord1d = (PFNGLTEXCOORD1DPROC)load("glTexCoord1d");
  glad_glTexCoord1dv = (PFNGLTEXCOORD1DVPROC)load("glTexCoord1dv");
  glad_glTexCoord1f = (PFNGLTEXCOORD1FPROC)load("glTexCoord1f");
  glad_glTexCoord1fv = (PFNGLTEXCOORD1FVPROC)load("glTexCoord1fv");
  glad_glTexCoord1i = (PFNGLTEXCOORD1IPROC)load("glTexCoord1i");
  glad_glTexCoord1iv = (PFNGLTEXCOORD1IVPROC)load("glTexCoord1iv");
  glad_glTexCoord1s = (PFNGLTEXCOORD1SPROC)load("glTexCoord1s");
  glad_glTexCoord1sv = (PFNGLTEXCOORD1SVPROC)load("glTexCoord1sv");
  glad_glTexCoord2d = (PFNGLTEXCOORD2DPROC)load("glTexCoord2d");
  glad_glTexCoord2dv = (PFNGLTEXCOORD2DVPROC)load("glTexCoord2dv");
  glad_glTexCoord2f = (PFNGLTEXCOORD2FPROC)load("glTexCoord2f");
  glad_glTexCoord2fv = (PFNGLTEXCOORD2FVPROC)load("glTexCoord2fv");
  glad_glTexCoord2i = (PFNGLTEXCOORD2IPROC)load("glTexCoord2i");
  glad_glTexCoord2iv = (PFNGLTEXCOORD2IVPROC)load("glTexCoord2iv");
  glad_glTexCoord2s = (PFNGLTEXCOORD2SPROC)load("glTexCoord2s");
  glad_glTexCoord2sv = (PFNGLTEXCOORD2SVPROC)load("glTexCoord2sv");
  glad_glTexCoord3d = (PFNGLTEXCOORD3DPROC)load("glTexCoord3d");
  glad_glTexCoord3dv = (PFNGLTEXCOORD3DVPROC)load("glTexCoord3dv");
  glad_glTexCoord3f = (PFNGLTEXCOORD3FPROC)load("glTexCoord3f");
  glad_glTexCoord3fv = (PFNGLTEXCOORD3FVPROC)load("glTexCoord3fv");
  glad_glTexCoord3i = (PFNGLTEXCOORD3IPROC)load("glTexCoord3i");
  glad_glTexCoord3iv = (PFNGLTEXCOORD3IVPROC)load("glTexCoord3iv");
  glad_glTexCoord3s = (PFNGLTEXCOORD3SPROC)load("glTexCoord3s");
  glad_glTexCoord3sv = (PFNGLTEXCOORD3SVPROC)load("glTexCoord3sv");
  glad_glTexCoord4d = (PFNGLTEXCOORD4DPROC)load("glTexCoord4d");
  glad_glTexCoord4dv = (PFNGLTEXCOORD4DVPROC)load("glTexCoord4dv");
  glad_glTexCoord4f = (PFNGLTEXCOORD4FPROC)load("glTexCoord4f");
  glad_glTexCoord4fv = (PFNGLTEXCOORD4FVPROC)load("glTexCoord4fv");
  glad_glTexCoord4i = (PFNGLTEXCOORD4IPROC)load("glTexCoord4i");
  glad_glTexCoord4iv = (PFNGLTEXCOORD4IVPROC)load("glTexCoord4iv");
  glad_glTexCoord4s = (PFNGLTEXCOORD4SPROC)load("glTexCoord4s");
  glad_glTexCoord4sv = (PFNGLTEXCOORD4SVPROC)load("glTexCoord4sv");
  glad_glVertex2d = (PFNGLVERTEX2DPROC)load("glVertex2d");
  glad_glVertex2dv = (PFNGLVERTEX2DVPROC)load("glVertex2dv");
  glad_glVertex2f = (PFNGLVERTEX2FPROC)load("glVertex2f");
  glad_glVertex2fv = (PFNGLVERTEX2FVPROC)load("glVertex2fv");
  glad_glVertex2i = (PFNGLVERTEX2IPROC)load("glVertex2i");
  glad_glVertex2iv = (PFNGLVERTEX2IVPROC)load("glVertex2iv");
  glad_glVertex2s = (PFNGLVERTEX2SPROC)load("glVertex2s");
  glad_glVertex2sv = (PFNGLVERTEX2SVPROC)load("glVertex2sv");
  glad_glVertex3d = (PFNGLVERTEX3DPROC)load("glVertex3d");
  glad_glVertex3dv = (PFNGLVERTEX3DVPROC)load("glVertex3dv");
  glad_glVertex3f = (PFNGLVERTEX3FPROC)load("glVertex3f");
  glad_glVertex3fv = (PFNGLVERTEX3FVPROC)load("glVertex3fv");
  glad_glVertex3i = (PFNGLVERTEX3IPROC)load("glVertex3i");
  glad_glVertex3iv = (PFNGLVERTEX3IVPROC)load("glVertex3iv");
  glad_glVertex3s = (PFNGLVERTEX3SPROC)load("glVertex3s");
  glad_glVertex3sv = (PFNGLVERTEX3SVPROC)load("glVertex3sv");
  glad_glVertex4d = (PFNGLVERTEX4DPROC)load("glVertex4d");
  glad_glVertex4dv = (PFNGLVERTEX4DVPROC)load("glVertex4dv");
  glad_glVertex4f = (PFNGLVERTEX4FPROC)load("glVertex4f");
  glad_glVertex4fv = (PFNGLVERTEX4FVPROC)load("glVertex4fv");
  glad_glVertex4i = (PFNGLVERTEX4IPROC)load("glVertex4i");
  glad_glVertex4iv = (PFNGLVERTEX4IVPROC)load("glVertex4iv");
  glad_glVertex4s = (PFNGLVERTEX4SPROC)load("glVertex4s");
  glad_glVertex4sv = (PFNGLVERTEX4SVPROC)load("glVertex4sv");
  glad_glClipPlane = (PFNGLCLIPPLANEPROC)load("glClipPlane");
  glad_glColorMaterial = (PFNGLCOLORMATERIALPROC)load("glColorMaterial");
  glad_glFogf = (PFNGLFOGFPROC)load("glFogf");
  glad_glFogfv = (PFNGLFOGFVPROC)load("glFogfv");
  glad_glFogi = (PFNGLFOGIPROC)load("glFogi");
  glad_glFogiv = (PFNGLFOGIVPROC)load("glFogiv");
  glad_glLightf = (PFNGLLIGHTFPROC)load("glLightf");
  glad_glLightfv = (PFNGLLIGHTFVPROC)load("glLightfv");
  glad_glLighti = (PFNGLLIGHTIPROC)load("glLighti");
  glad_glLightiv = (PFNGLLIGHTIVPROC)load("glLightiv");
  glad_glLightModelf = (PFNGLLIGHTMODELFPROC)load("glLightModelf");
  glad_glLightModelfv = (PFNGLLIGHTMODELFVPROC)load("glLightModelfv");
  glad_glLightModeli = (PFNGLLIGHTMODELIPROC)load("glLightModeli");
  glad_glLightModeliv = (PFNGLLIGHTMODELIVPROC)load("glLightModeliv");
  glad_glLineStipple = (PFNGLLINESTIPPLEPROC)load("glLineStipple");
  glad_glMaterialf = (PFNGLMATERIALFPROC)load("glMaterialf");
  glad_glMaterialfv = (PFNGLMATERIALFVPROC)load("glMaterialfv");
  glad_glMateriali = (PFNGLMATERIALIPROC)load("glMateriali");
  glad_glMaterialiv = (PFNGLMATERIALIVPROC)load("glMaterialiv");
  glad_glPolygonStipple = (PFNGLPOLYGONSTIPPLEPROC)load("glPolygonStipple");
  glad_glShadeModel = (PFNGLSHADEMODELPROC)load("glShadeModel");
  glad_glTexEnvf = (PFNGLTEXENVFPROC)load("glTexEnvf");
  glad_glTexEnvfv = (PFNGLTEXENVFVPROC)load("glTexEnvfv");
  glad_glTexEnvi = (PFNGLTEXENVIPROC)load("glTexEnvi");
  glad_glTexEnviv = (PFNGLTEXENVIVPROC)load("glTexEnviv");
  glad_glTexGend = (PFNGLTEXGENDPROC)load("glTexGend");
  glad_glTexGendv = (PFNGLTEXGENDVPROC)load("glTexGendv");
  glad_glTexGenf = (PFNGLTEXGENFPROC)load("glTexGenf");
  glad_glTexGenfv = (PFNGLTEXGENFVPROC)load("glTexGenfv");
  glad_glTexGeni = (PFNGLTEXGENIPROC)load("glTexGeni");
  glad_glTexGeniv = (PFNGLTEXGENIVPROC)load("glTexGeniv");
  glad_glFeedbackBuffer = (PFNGLFEEDBACKBUFFERPROC)load("glFeedbackBuffer");
  glad_glSelectBuffer = (PFNGLSELECTBUFFERPROC)load("glSelectBuffer");
  glad_glRenderMode = (PFNGLRENDERMODEPROC)load("glRenderMode");
  glad_glInitNames = (PFNGLINITNAMESPROC)load("glInitNames");
  glad_glLoadName = (PFNGLLOADNAMEPROC)load("glLoadName");
  glad_glPassThrough = (PFNGLPASSTHROUGHPROC)load("glPassThrough");
  glad_glPopName = (PFNGLPOPNAMEPROC)load("glPopName");
  glad_glPushName = (PFNGLPUSHNAMEPROC)load("glPushName");
  glad_glClearAccum = (PFNGLCLEARACCUMPROC)load("glClearAccum");
  glad_glClearIndex = (PFNGLCLEARINDEXPROC)load("glClearIndex");
  glad_glIndexMask = (PFNGLINDEXMASKPROC)load("glIndexMask");
  glad_glAccum = (PFNGLACCUMPROC)load("glAccum");
  glad_glPopAttrib = (PFNGLPOPATTRIBPROC)load("glPopAttrib");
  glad_glPushAttrib = (PFNGLPUSHATTRIBPROC)load("glPushAttrib");
  glad_glMap1d = (PFNGLMAP1DPROC)load("glMap1d");
  glad_glMap1f = (PFNGLMAP1FPROC)load("glMap1f");
  glad_glMap2d = (PFNGLMAP2DPROC)load("glMap2d");
  glad_glMap2f = (PFNGLMAP2FPROC)load("glMap2f");
  glad_glMapGrid1d = (PFNGLMAPGRID1DPROC)load("glMapGrid1d");
  glad_glMapGrid1f = (PFNGLMAPGRID1FPROC)load("glMapGrid1f");
  glad_glMapGrid2d = (PFNGLMAPGRID2DPROC)load("glMapGrid2d");
  glad_glMapGrid2f = (PFNGLMAPGRID2FPROC)load("glMapGrid2f");
  glad_glEvalCoord1d = (PFNGLEVALCOORD1DPROC)load("glEvalCoord1d");
  glad_glEvalCoord1dv = (PFNGLEVALCOORD1DVPROC)load("glEvalCoord1dv");
  glad_glEvalCoord1f = (PFNGLEVALCOORD1FPROC)load("glEvalCoord1f");
  glad_glEvalCoord1fv = (PFNGLEVALCOORD1FVPROC)load("glEvalCoord1fv");
  glad_glEvalCoord2d = (PFNGLEVALCOORD2DPROC)load("glEvalCoord2d");
  glad_glEvalCoord2dv = (PFNGLEVALCOORD2DVPROC)load("glEvalCoord2dv");
  glad_glEvalCoord2f = (PFNGLEVALCOORD2FPROC)load("glEvalCoord2f");
  glad_glEvalCoord2fv = (PFNGLEVALCOORD2FVPROC)load("glEvalCoord2fv");
  glad_glEvalMesh1 = (PFNGLEVALMESH1PROC)load("glEvalMesh1");
  glad_glEvalPoint1 = (PFNGLEVALPOINT1PROC)load("glEvalPoint1");
  glad_glEvalMesh2 = (PFNGLEVALMESH2PROC)load("glEvalMesh2");
  glad_glEvalPoint2 = (PFNGLEVALPOINT2PROC)load("glEvalPoint2");
  glad_glAlphaFunc = (PFNGLALPHAFUNCPROC)load("glAlphaFunc");
  glad_glPixelZoom = (PFNGLPIXELZOOMPROC)load("glPixelZoom");
  glad_glPixelTransferf = (PFNGLPIXELTRANSFERFPROC)load("glPixelTransferf");
  glad_glPixelTransferi = (PFNGLPIXELTRANSFERIPROC)load("glPixelTransferi");
  glad_glPixelMapfv = (PFNGLPIXELMAPFVPROC)load("glPixelMapfv");
  glad_glPixelMapuiv = (PFNGLPIXELMAPUIVPROC)load("glPixelMapuiv");
  glad_glPixelMapusv = (PFNGLPIXELMAPUSVPROC)load("glPixelMapusv");
  glad_glCopyPixels = (PFNGLCOPYPIXELSPROC)load("glCopyPixels");
  glad_glDrawPixels = (PFNGLDRAWPIXELSPROC)load("glDrawPixels");
  glad_glGetClipPlane = (PFNGLGETCLIPPLANEPROC)load("glGetClipPlane");
  glad_glGetLightfv = (PFNGLGETLIGHTFVPROC)load("glGetLightfv");
  glad_glGetLightiv = (PFNGLGETLIGHTIVPROC)load("glGetLightiv");
  glad_glGetMapdv = (PFNGLGETMAPDVPROC)load("glGetMapdv");
  glad_glGetMapfv = (PFNGLGETMAPFVPROC)load("glGetMapfv");
  glad_glGetMapiv = (PFNGLGETMAPIVPROC)load("glGetMapiv");
  glad_glGetMaterialfv = (PFNGLGETMATERIALFVPROC)load("glGetMaterialfv");
  glad_glGetMaterialiv = (PFNGLGETMATERIALIVPROC)load("glGetMaterialiv");
  glad_glGetPixelMapfv = (PFNGLGETPIXELMAPFVPROC)load("glGetPixelMapfv");
  glad_glGetPixelMapuiv = (PFNGLGETPIXELMAPUIVPROC)load("glGetPixelMapuiv");
  glad_glGetPixelMapusv = (PFNGLGETPIXELMAPUSVPROC)load("glGetPixelMapusv");
  glad_glGetPolygonStipple =
      (PFNGLGETPOLYGONSTIPPLEPROC)load("glGetPolygonStipple");
  glad_glGetTexEnvfv = (PFNGLGETTEXENVFVPROC)load("glGetTexEnvfv");
  glad_glGetTexEnviv = (PFNGLGETTEXENVIVPROC)load("glGetTexEnviv");
  glad_glGetTexGendv = (PFNGLGETTEXGENDVPROC)load("glGetTexGendv");
  glad_glGetTexGenfv = (PFNGLGETTEXGENFVPROC)load("glGetTexGenfv");
  glad_glGetTexGeniv = (PFNGLGETTEXGENIVPROC)load("glGetTexGeniv");
  glad_glIsList = (PFNGLISLISTPROC)load("glIsList");
  glad_glFrustum = (PFNGLFRUSTUMPROC)load("glFrustum");
  glad_glLoadIdentity = (PFNGLLOADIDENTITYPROC)load("glLoadIdentity");
  glad_glLoadMatrixf = (PFNGLLOADMATRIXFPROC)load("glLoadMatrixf");
  glad_glLoadMatrixd = (PFNGLLOADMATRIXDPROC)load("glLoadMatrixd");
  glad_glMatrixMode = (PFNGLMATRIXMODEPROC)load("glMatrixMode");
  glad_glMultMatrixf = (PFNGLMULTMATRIXFPROC)load("glMultMatrixf");
  glad_glMultMatrixd = (PFNGLMULTMATRIXDPROC)load("glMultMatrixd");
  glad_glOrtho = (PFNGLORTHOPROC)load("glOrtho");
  glad_glPopMatrix = (PFNGLPOPMATRIXPROC)load("glPopMatrix");
  glad_glPushMatrix = (PFNGLPUSHMATRIXPROC)load("glPushMatrix");
  glad_glRotated = (PFNGLROTATEDPROC)load("glRotated");
  glad_glRotatef = (PFNGLROTATEFPROC)load("glRotatef");
  glad_glScaled = (PFNGLSCALEDPROC)load("glScaled");
  glad_glScalef = (PFNGLSCALEFPROC)load("glScalef");
  glad_glTranslated = (PFNGLTRANSLATEDPROC)load("glTranslated");
  glad_glTranslatef = (PFNGLTRANSLATEFPROC)load("glTranslatef");
}
static void load_GL_VERSION_1_1(GLADloadproc load) {
  if (!GLAD_GL_VERSION_1_1)
    return;
  glad_glDrawArrays = (PFNGLDRAWARRAYSPROC)load("glDrawArrays");
  glad_glDrawElements = (PFNGLDRAWELEMENTSPROC)load("glDrawElements");
  glad_glGetPointerv = (PFNGLGETPOINTERVPROC)load("glGetPointerv");
  glad_glPolygonOffset = (PFNGLPOLYGONOFFSETPROC)load("glPolygonOffset");
  glad_glCopyTexImage1D = (PFNGLCOPYTEXIMAGE1DPROC)load("glCopyTexImage1D");
  glad_glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC)load("glCopyTexImage2D");
  glad_glCopyTexSubImage1D =
      (PFNGLCOPYTEXSUBIMAGE1DPROC)load("glCopyTexSubImage1D");
  glad_glCopyTexSubImage2D =
      (PFNGLCOPYTEXSUBIMAGE2DPROC)load("glCopyTexSubImage2D");
  glad_glTexSubImage1D = (PFNGLTEXSUBIMAGE1DPROC)load("glTexSubImage1D");
  glad_glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)load("glTexSubImage2D");
  glad_glBindTexture = (PFNGLBINDTEXTUREPROC)load("glBindTexture");
  glad_glDeleteTextures = (PFNGLDELETETEXTURESPROC)load("glDeleteTextures");
  glad_glGenTextures = (PFNGLGENTEXTURESPROC)load("glGenTextures");
  glad_glIsTexture = (PFNGLISTEXTUREPROC)load("glIsTexture");
  glad_glArrayElement = (PFNGLARRAYELEMENTPROC)load("glArrayElement");
  glad_glColorPointer = (PFNGLCOLORPOINTERPROC)load("glColorPointer");
  glad_glDisableClientState =
      (PFNGLDISABLECLIENTSTATEPROC)load("glDisableClientState");
  glad_glEdgeFlagPointer = (PFNGLEDGEFLAGPOINTERPROC)load("glEdgeFlagPointer");
  glad_glEnableClientState =
      (PFNGLENABLECLIENTSTATEPROC)load("glEnableClientState");
  glad_glIndexPointer = (PFNGLINDEXPOINTERPROC)load("glIndexPointer");
  glad_glInterleavedArrays =
      (PFNGLINTERLEAVEDARRAYSPROC)load("glInterleavedArrays");
  glad_glNormalPointer = (PFNGLNORMALPOINTERPROC)load("glNormalPointer");
  glad_glTexCoordPointer = (PFNGLTEXCOORDPOINTERPROC)load("glTexCoordPointer");
  glad_glVertexPointer = (PFNGLVERTEXPOINTERPROC)load("glVertexPointer");
  glad_glAreTexturesResident =
      (PFNGLARETEXTURESRESIDENTPROC)load("glAreTexturesResident");
  glad_glPrioritizeTextures =
      (PFNGLPRIORITIZETEXTURESPROC)load("glPrioritizeTextures");
  glad_glIndexub = (PFNGLINDEXUBPROC)load("glIndexub");
  glad_glIndexubv = (PFNGLINDEXUBVPROC)load("glIndexubv");
  glad_glPopClientAttrib = (PFNGLPOPCLIENTATTRIBPROC)load("glPopClientAttrib");
  glad_glPushClientAttrib =
      (PFNGLPUSHCLIENTATTRIBPROC)load("glPushClientAttrib");
}
static void load_GL_VERSION_1_2(GLADloadproc load) {
  if (!GLAD_GL_VERSION_1_2)
    return;
  glad_glDrawRangeElements =
      (PFNGLDRAWRANGEELEMENTSPROC)load("glDrawRangeElements");
  glad_glTexImage3D = (PFNGLTEXIMAGE3DPROC)load("glTexImage3D");
  glad_glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)load("glTexSubImage3D");
  glad_glCopyTexSubImage3D =
      (PFNGLCOPYTEXSUBIMAGE3DPROC)load("glCopyTexSubImage3D");
}
static void load_GL_VERSION_1_3(GLADloadproc load) {
  if (!GLAD_GL_VERSION_1_3)
    return;
  glad_glActiveTexture = (PFNGLACTIVETEXTUREPROC)load("glActiveTexture");
  glad_glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)load("glSampleCoverage");
  glad_glCompressedTexImage3D =
      (PFNGLCOMPRESSEDTEXIMAGE3DPROC)load("glCompressedTexImage3D");
  glad_glCompressedTexImage2D =
      (PFNGLCOMPRESSEDTEXIMAGE2DPROC)load("glCompressedTexImage2D");
  glad_glCompressedTexImage1D =
      (PFNGLCOMPRESSEDTEXIMAGE1DPROC)load("glCompressedTexImage1D");
  glad_glCompressedTexSubImage3D =
      (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)load("glCompressedTexSubImage3D");
  glad_glCompressedTexSubImage2D =
      (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)load("glCompressedTexSubImage2D");
  glad_glCompressedTexSubImage1D =
      (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC)load("glCompressedTexSubImage1D");
  glad_glGetCompressedTexImage =
      (PFNGLGETCOMPRESSEDTEXIMAGEPROC)load("glGetCompressedTexImage");
  glad_glClientActiveTexture =
      (PFNGLCLIENTACTIVETEXTUREPROC)load("glClientActiveTexture");
  glad_glMultiTexCoord1d = (PFNGLMULTITEXCOORD1DPROC)load("glMultiTexCoord1d");
  glad_glMultiTexCoord1dv =
      (PFNGLMULTITEXCOORD1DVPROC)load("glMultiTexCoord1dv");
  glad_glMultiTexCoord1f = (PFNGLMULTITEXCOORD1FPROC)load("glMultiTexCoord1f");
  glad_glMultiTexCoord1fv =
      (PFNGLMULTITEXCOORD1FVPROC)load("glMultiTexCoord1fv");
  glad_glMultiTexCoord1i = (PFNGLMULTITEXCOORD1IPROC)load("glMultiTexCoord1i");
  glad_glMultiTexCoord1iv =
      (PFNGLMULTITEXCOORD1IVPROC)load("glMultiTexCoord1iv");
  glad_glMultiTexCoord1s = (PFNGLMULTITEXCOORD1SPROC)load("glMultiTexCoord1s");
  glad_glMultiTexCoord1sv =
      (PFNGLMULTITEXCOORD1SVPROC)load("glMultiTexCoord1sv");
  glad_glMultiTexCoord2d = (PFNGLMULTITEXCOORD2DPROC)load("glMultiTexCoord2d");
  glad_glMultiTexCoord2dv =
      (PFNGLMULTITEXCOORD2DVPROC)load("glMultiTexCoord2dv");
  glad_glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)load("glMultiTexCoord2f");
  glad_glMultiTexCoord2fv =
      (PFNGLMULTITEXCOORD2FVPROC)load("glMultiTexCoord2fv");
  glad_glMultiTexCoord2i = (PFNGLMULTITEXCOORD2IPROC)load("glMultiTexCoord2i");
  glad_glMultiTexCoord2iv =
      (PFNGLMULTITEXCOORD2IVPROC)load("glMultiTexCoord2iv");
  glad_glMultiTexCoord2s = (PFNGLMULTITEXCOORD2SPROC)load("glMultiTexCoord2s");
  glad_glMultiTexCoord2sv =
      (PFNGLMULTITEXCOORD2SVPROC)load("glMultiTexCoord2sv");
  glad_glMultiTexCoord3d = (PFNGLMULTITEXCOORD3DPROC)load("glMultiTexCoord3d");
  glad_glMultiTexCoord3dv =
      (PFNGLMULTITEXCOORD3DVPROC)load("glMultiTexCoord3dv");
  glad_glMultiTexCoord3f = (PFNGLMULTITEXCOORD3FPROC)load("glMultiTexCoord3f");
  glad_glMultiTexCoord3fv =
      (PFNGLMULTITEXCOORD3FVPROC)load("glMultiTexCoord3fv");
  glad_glMultiTexCoord3i = (PFNGLMULTITEXCOORD3IPROC)load("glMultiTexCoord3i");
  glad_glMultiTexCoord3iv =
      (PFNGLMULTITEXCOORD3IVPROC)load("glMultiTexCoord3iv");
  glad_glMultiTexCoord3s = (PFNGLMULTITEXCOORD3SPROC)load("glMultiTexCoord3s");
  glad_glMultiTexCoord3sv =
      (PFNGLMULTITEXCOORD3SVPROC)load("glMultiTexCoord3sv");
  glad_glMultiTexCoord4d = (PFNGLMULTITEXCOORD4DPROC)load("glMultiTexCoord4d");
  glad_glMultiTexCoord4dv =
      (PFNGLMULTITEXCOORD4DVPROC)load("glMultiTexCoord4dv");
  glad_glMultiTexCoord4f = (PFNGLMULTITEXCOORD4FPROC)load("glMultiTexCoord4f");
  glad_glMultiTexCoord4fv =
      (PFNGLMULTITEXCOORD4FVPROC)load("glMultiTexCoord4fv");
  glad_glMultiTexCoord4i = (PFNGLMULTITEXCOORD4IPROC)load("glMultiTexCoord4i");
  glad_glMultiTexCoord4iv =
      (PFNGLMULTITEXCOORD4IVPROC)load("glMultiTexCoord4iv");
  glad_glMultiTexCoord4s = (PFNGLMULTITEXCOORD4SPROC)load("glMultiTexCoord4s");
  glad_glMultiTexCoord4sv =
      (PFNGLMULTITEXCOORD4SVPROC)load("glMultiTexCoord4sv");
  glad_glLoadTransposeMatrixf =
      (PFNGLLOADTRANSPOSEMATRIXFPROC)load("glLoadTransposeMatrixf");
  glad_glLoadTransposeMatrixd =
      (PFNGLLOADTRANSPOSEMATRIXDPROC)load("glLoadTransposeMatrixd");
  glad_glMultTransposeMatrixf =
      (PFNGLMULTTRANSPOSEMATRIXFPROC)load("glMultTransposeMatrixf");
  glad_glMultTransposeMatrixd =
      (PFNGLMULTTRANSPOSEMATRIXDPROC)load("glMultTransposeMatrixd");
}
static void load_GL_VERSION_1_4(GLADloadproc load) {
  if (!GLAD_GL_VERSION_1_4)
    return;
  glad_glBlendFuncSeparate =
      (PFNGLBLENDFUNCSEPARATEPROC)load("glBlendFuncSeparate");
  glad_glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC)load("glMultiDrawArrays");
  glad_glMultiDrawElements =
      (PFNGLMULTIDRAWELEMENTSPROC)load("glMultiDrawElements");
  glad_glPointParameterf = (PFNGLPOINTPARAMETERFPROC)load("glPointParameterf");
  glad_glPointParameterfv =
      (PFNGLPOINTPARAMETERFVPROC)load("glPointParameterfv");
  glad_glPointParameteri = (PFNGLPOINTPARAMETERIPROC)load("glPointParameteri");
  glad_glPointParameteriv =
      (PFNGLPOINTPARAMETERIVPROC)load("glPointParameteriv");
  glad_glFogCoordf = (PFNGLFOGCOORDFPROC)load("glFogCoordf");
  glad_glFogCoordfv = (PFNGLFOGCOORDFVPROC)load("glFogCoordfv");
  glad_glFogCoordd = (PFNGLFOGCOORDDPROC)load("glFogCoordd");
  glad_glFogCoorddv = (PFNGLFOGCOORDDVPROC)load("glFogCoorddv");
  glad_glFogCoordPointer = (PFNGLFOGCOORDPOINTERPROC)load("glFogCoordPointer");
  glad_glSecondaryColor3b =
      (PFNGLSECONDARYCOLOR3BPROC)load("glSecondaryColor3b");
  glad_glSecondaryColor3bv =
      (PFNGLSECONDARYCOLOR3BVPROC)load("glSecondaryColor3bv");
  glad_glSecondaryColor3d =
      (PFNGLSECONDARYCOLOR3DPROC)load("glSecondaryColor3d");
  glad_glSecondaryColor3dv =
      (PFNGLSECONDARYCOLOR3DVPROC)load("glSecondaryColor3dv");
  glad_glSecondaryColor3f =
      (PFNGLSECONDARYCOLOR3FPROC)load("glSecondaryColor3f");
  glad_glSecondaryColor3fv =
      (PFNGLSECONDARYCOLOR3FVPROC)load("glSecondaryColor3fv");
  glad_glSecondaryColor3i =
      (PFNGLSECONDARYCOLOR3IPROC)load("glSecondaryColor3i");
  glad_glSecondaryColor3iv =
      (PFNGLSECONDARYCOLOR3IVPROC)load("glSecondaryColor3iv");
  glad_glSecondaryColor3s =
      (PFNGLSECONDARYCOLOR3SPROC)load("glSecondaryColor3s");
  glad_glSecondaryColor3sv =
      (PFNGLSECONDARYCOLOR3SVPROC)load("glSecondaryColor3sv");
  glad_glSecondaryColor3ub =
      (PFNGLSECONDARYCOLOR3UBPROC)load("glSecondaryColor3ub");
  glad_glSecondaryColor3ubv =
      (PFNGLSECONDARYCOLOR3UBVPROC)load("glSecondaryColor3ubv");
  glad_glSecondaryColor3ui =
      (PFNGLSECONDARYCOLOR3UIPROC)load("glSecondaryColor3ui");
  glad_glSecondaryColor3uiv =
      (PFNGLSECONDARYCOLOR3UIVPROC)load("glSecondaryColor3uiv");
  glad_glSecondaryColor3us =
      (PFNGLSECONDARYCOLOR3USPROC)load("glSecondaryColor3us");
  glad_glSecondaryColor3usv =
      (PFNGLSECONDARYCOLOR3USVPROC)load("glSecondaryColor3usv");
  glad_glSecondaryColorPointer =
      (PFNGLSECONDARYCOLORPOINTERPROC)load("glSecondaryColorPointer");
  glad_glWindowPos2d = (PFNGLWINDOWPOS2DPROC)load("glWindowPos2d");
  glad_glWindowPos2dv = (PFNGLWINDOWPOS2DVPROC)load("glWindowPos2dv");
  glad_glWindowPos2f = (PFNGLWINDOWPOS2FPROC)load("glWindowPos2f");
  glad_glWindowPos2fv = (PFNGLWINDOWPOS2FVPROC)load("glWindowPos2fv");
  glad_glWindowPos2i = (PFNGLWINDOWPOS2IPROC)load("glWindowPos2i");
  glad_glWindowPos2iv = (PFNGLWINDOWPOS2IVPROC)load("glWindowPos2iv");
  glad_glWindowPos2s = (PFNGLWINDOWPOS2SPROC)load("glWindowPos2s");
  glad_glWindowPos2sv = (PFNGLWINDOWPOS2SVPROC)load("glWindowPos2sv");
  glad_glWindowPos3d = (PFNGLWINDOWPOS3DPROC)load("glWindowPos3d");
  glad_glWindowPos3dv = (PFNGLWINDOWPOS3DVPROC)load("glWindowPos3dv");
  glad_glWindowPos3f = (PFNGLWINDOWPOS3FPROC)load("glWindowPos3f");
  glad_glWindowPos3fv = (PFNGLWINDOWPOS3FVPROC)load("glWindowPos3fv");
  glad_glWindowPos3i = (PFNGLWINDOWPOS3IPROC)load("glWindowPos3i");
  glad_glWindowPos3iv = (PFNGLWINDOWPOS3IVPROC)load("glWindowPos3iv");
  glad_glWindowPos3s = (PFNGLWINDOWPOS3SPROC)load("glWindowPos3s");
  glad_glWindowPos3sv = (PFNGLWINDOWPOS3SVPROC)load("glWindowPos3sv");
  glad_glBlendColor = (PFNGLBLENDCOLORPROC)load("glBlendColor");
  glad_glBlendEquation = (PFNGLBLENDEQUATIONPROC)load("glBlendEquation");
}
static void load_GL_VERSION_1_5(GLADloadproc load) {
  if (!GLAD_GL_VERSION_1_5)
    return;
  glad_glGenQueries = (PFNGLGENQUERIESPROC)load("glGenQueries");
  glad_glDeleteQueries = (PFNGLDELETEQUERIESPROC)load("glDeleteQueries");
  glad_glIsQuery = (PFNGLISQUERYPROC)load("glIsQuery");
  glad_glBeginQuery = (PFNGLBEGINQUERYPROC)load("glBeginQuery");
  glad_glEndQuery = (PFNGLENDQUERYPROC)load("glEndQuery");
  glad_glGetQueryiv = (PFNGLGETQUERYIVPROC)load("glGetQueryiv");
  glad_glGetQueryObjectiv =
      (PFNGLGETQUERYOBJECTIVPROC)load("glGetQueryObjectiv");
  glad_glGetQueryObjectuiv =
      (PFNGLGETQUERYOBJECTUIVPROC)load("glGetQueryObjectuiv");
  glad_glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
  glad_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)load("glDeleteBuffers");
  glad_glGenBuffers = (PFNGLGENBUFFERSPROC)load("glGenBuffers");
  glad_glIsBuffer = (PFNGLISBUFFERPROC)load("glIsBuffer");
  glad_glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
  glad_glBufferSubData = (PFNGLBUFFERSUBDATAPROC)load("glBufferSubData");
  glad_glGetBufferSubData =
      (PFNGLGETBUFFERSUBDATAPROC)load("glGetBufferSubData");
  glad_glMapBuffer = (PFNGLMAPBUFFERPROC)load("glMapBuffer");
  glad_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)load("glUnmapBuffer");
  glad_glGetBufferParameteriv =
      (PFNGLGETBUFFERPARAMETERIVPROC)load("glGetBufferParameteriv");
  glad_glGetBufferPointerv =
      (PFNGLGETBUFFERPOINTERVPROC)load("glGetBufferPointerv");
}
static void load_GL_VERSION_2_0(GLADloadproc load) {
  if (!GLAD_GL_VERSION_2_0)
    return;
  glad_glBlendEquationSeparate =
      (PFNGLBLENDEQUATIONSEPARATEPROC)load("glBlendEquationSeparate");
  glad_glDrawBuffers = (PFNGLDRAWBUFFERSPROC)load("glDrawBuffers");
  glad_glStencilOpSeparate =
      (PFNGLSTENCILOPSEPARATEPROC)load("glStencilOpSeparate");
  glad_glStencilFuncSeparate =
      (PFNGLSTENCILFUNCSEPARATEPROC)load("glStencilFuncSeparate");
  glad_glStencilMaskSeparate =
      (PFNGLSTENCILMASKSEPARATEPROC)load("glStencilMaskSeparate");
  glad_glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
  glad_glBindAttribLocation =
      (PFNGLBINDATTRIBLOCATIONPROC)load("glBindAttribLocation");
  glad_glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
  glad_glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
  glad_glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
  glad_glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load("glDeleteProgram");
  glad_glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
  glad_glDetachShader = (PFNGLDETACHSHADERPROC)load("glDetachShader");
  glad_glDisableVertexAttribArray =
      (PFNGLDISABLEVERTEXATTRIBARRAYPROC)load("glDisableVertexAttribArray");
  glad_glEnableVertexAttribArray =
      (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
  glad_glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)load("glGetActiveAttrib");
  glad_glGetActiveUniform =
      (PFNGLGETACTIVEUNIFORMPROC)load("glGetActiveUniform");
  glad_glGetAttachedShaders =
      (PFNGLGETATTACHEDSHADERSPROC)load("glGetAttachedShaders");
  glad_glGetAttribLocation =
      (PFNGLGETATTRIBLOCATIONPROC)load("glGetAttribLocation");
  glad_glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load("glGetProgramiv");
  glad_glGetProgramInfoLog =
      (PFNGLGETPROGRAMINFOLOGPROC)load("glGetProgramInfoLog");
  glad_glGetShaderiv = (PFNGLGETSHADERIVPROC)load("glGetShaderiv");
  glad_glGetShaderInfoLog =
      (PFNGLGETSHADERINFOLOGPROC)load("glGetShaderInfoLog");
  glad_glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)load("glGetShaderSource");
  glad_glGetUniformLocation =
      (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");
  glad_glGetUniformfv = (PFNGLGETUNIFORMFVPROC)load("glGetUniformfv");
  glad_glGetUniformiv = (PFNGLGETUNIFORMIVPROC)load("glGetUniformiv");
  glad_glGetVertexAttribdv =
      (PFNGLGETVERTEXATTRIBDVPROC)load("glGetVertexAttribdv");
  glad_glGetVertexAttribfv =
      (PFNGLGETVERTEXATTRIBFVPROC)load("glGetVertexAttribfv");
  glad_glGetVertexAttribiv =
      (PFNGLGETVERTEXATTRIBIVPROC)load("glGetVertexAttribiv");
  glad_glGetVertexAttribPointerv =
      (PFNGLGETVERTEXATTRIBPOINTERVPROC)load("glGetVertexAttribPointerv");
  glad_glIsProgram = (PFNGLISPROGRAMPROC)load("glIsProgram");
  glad_glIsShader = (PFNGLISSHADERPROC)load("glIsShader");
  glad_glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
  glad_glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
  glad_glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");
  glad_glUniform1f = (PFNGLUNIFORM1FPROC)load("glUniform1f");
  glad_glUniform2f = (PFNGLUNIFORM2FPROC)load("glUniform2f");
  glad_glUniform3f = (PFNGLUNIFORM3FPROC)load("glUniform3f");
  glad_glUniform4f = (PFNGLUNIFORM4FPROC)load("glUniform4f");
  glad_glUniform1i = (PFNGLUNIFORM1IPROC)load("glUniform1i");
  glad_glUniform2i = (PFNGLUNIFORM2IPROC)load("glUniform2i");
  glad_glUniform3i = (PFNGLUNIFORM3IPROC)load("glUniform3i");
  glad_glUniform4i = (PFNGLUNIFORM4IPROC)load("glUniform4i");
  glad_glUniform1fv = (PFNGLUNIFORM1FVPROC)load("glUniform1fv");
  glad_glUniform2fv = (PFNGLUNIFORM2FVPROC)load("glUniform2fv");
  glad_glUniform3fv = (PFNGLUNIFORM3FVPROC)load("glUniform3fv");
  glad_glUniform4fv = (PFNGLUNIFORM4FVPROC)load("glUniform4fv");
  glad_glUniform1iv = (PFNGLUNIFORM1IVPROC)load("glUniform1iv");
  glad_glUniform2iv = (PFNGLUNIFORM2IVPROC)load("glUniform2iv");
  glad_glUniform3iv = (PFNGLUNIFORM3IVPROC)load("glUniform3iv");
  glad_glUniform4iv = (PFNGLUNIFORM4IVPROC)load("glUniform4iv");
  glad_glUniformMatrix2fv =
      (PFNGLUNIFORMMATRIX2FVPROC)load("glUniformMatrix2fv");
  glad_glUniformMatrix3fv =
      (PFNGLUNIFORMMATRIX3FVPROC)load("glUniformMatrix3fv");
  glad_glUniformMatrix4fv =
      (PFNGLUNIFORMMATRIX4FVPROC)load("glUniformMatrix4fv");
  glad_glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)load("glValidateProgram");
  glad_glVertexAttrib1d = (PFNGLVERTEXATTRIB1DPROC)load("glVertexAttrib1d");
  glad_glVertexAttrib1dv = (PFNGLVERTEXATTRIB1DVPROC)load("glVertexAttrib1dv");
  glad_glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)load("glVertexAttrib1f");
  glad_glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)load("glVertexAttrib1fv");
  glad_glVertexAttrib1s = (PFNGLVERTEXATTRIB1SPROC)load("glVertexAttrib1s");
  glad_glVertexAttrib1sv = (PFNGLVERTEXATTRIB1SVPROC)load("glVertexAttrib1sv");
  glad_glVertexAttrib2d = (PFNGLVERTEXATTRIB2DPROC)load("glVertexAttrib2d");
  glad_glVertexAttrib2dv = (PFNGLVERTEXATTRIB2DVPROC)load("glVertexAttrib2dv");
  glad_glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)load("glVertexAttrib2f");
  glad_glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)load("glVertexAttrib2fv");
  glad_glVertexAttrib2s = (PFNGLVERTEXATTRIB2SPROC)load("glVertexAttrib2s");
  glad_glVertexAttrib2sv = (PFNGLVERTEXATTRIB2SVPROC)load("glVertexAttrib2sv");
  glad_glVertexAttrib3d = (PFNGLVERTEXATTRIB3DPROC)load("glVertexAttrib3d");
  glad_glVertexAttrib3dv = (PFNGLVERTEXATTRIB3DVPROC)load("glVertexAttrib3dv");
  glad_glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)load("glVertexAttrib3f");
  glad_glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)load("glVertexAttrib3fv");
  glad_glVertexAttrib3s = (PFNGLVERTEXATTRIB3SPROC)load("glVertexAttrib3s");
  glad_glVertexAttrib3sv = (PFNGLVERTEXATTRIB3SVPROC)load("glVertexAttrib3sv");
  glad_glVertexAttrib4Nbv =
      (PFNGLVERTEXATTRIB4NBVPROC)load("glVertexAttrib4Nbv");
  glad_glVertexAttrib4Niv =
      (PFNGLVERTEXATTRIB4NIVPROC)load("glVertexAttrib4Niv");
  glad_glVertexAttrib4Nsv =
      (PFNGLVERTEXATTRIB4NSVPROC)load("glVertexAttrib4Nsv");
  glad_glVertexAttrib4Nub =
      (PFNGLVERTEXATTRIB4NUBPROC)load("glVertexAttrib4Nub");
  glad_glVertexAttrib4Nubv =
      (PFNGLVERTEXATTRIB4NUBVPROC)load("glVertexAttrib4Nubv");
  glad_glVertexAttrib4Nuiv =
      (PFNGLVERTEXATTRIB4NUIVPROC)load("glVertexAttrib4Nuiv");
  glad_glVertexAttrib4Nusv =
      (PFNGLVERTEXATTRIB4NUSVPROC)load("glVertexAttrib4Nusv");
  glad_glVertexAttrib4bv = (PFNGLVERTEXATTRIB4BVPROC)load("glVertexAttrib4bv");
  glad_glVertexAttrib4d = (PFNGLVERTEXATTRIB4DPROC)load("glVertexAttrib4d");
  glad_glVertexAttrib4dv = (PFNGLVERTEXATTRIB4DVPROC)load("glVertexAttrib4dv");
  glad_glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)load("glVertexAttrib4f");
  glad_glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)load("glVertexAttrib4fv");
  glad_glVertexAttrib4iv = (PFNGLVERTEXATTRIB4IVPROC)load("glVertexAttrib4iv");
  glad_glVertexAttrib4s = (PFNGLVERTEXATTRIB4SPROC)load("glVertexAttrib4s");
  glad_glVertexAttrib4sv = (PFNGLVERTEXATTRIB4SVPROC)load("glVertexAttrib4sv");
  glad_glVertexAttrib4ubv =
      (PFNGLVERTEXATTRIB4UBVPROC)load("glVertexAttrib4ubv");
  glad_glVertexAttrib4uiv =
      (PFNGLVERTEXATTRIB4UIVPROC)load("glVertexAttrib4uiv");
  glad_glVertexAttrib4usv =
      (PFNGLVERTEXATTRIB4USVPROC)load("glVertexAttrib4usv");
  glad_glVertexAttribPointer =
      (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");
}
static void load_GL_VERSION_2_1(GLADloadproc load) {
  if (!GLAD_GL_VERSION_2_1)
    return;
  glad_glUniformMatrix2x3fv =
      (PFNGLUNIFORMMATRIX2X3FVPROC)load("glUniformMatrix2x3fv");
  glad_glUniformMatrix3x2fv =
      (PFNGLUNIFORMMATRIX3X2FVPROC)load("glUniformMatrix3x2fv");
  glad_glUniformMatrix2x4fv =
      (PFNGLUNIFORMMATRIX2X4FVPROC)load("glUniformMatrix2x4fv");
  glad_glUniformMatrix4x2fv =
      (PFNGLUNIFORMMATRIX4X2FVPROC)load("glUniformMatrix4x2fv");
  glad_glUniformMatrix3x4fv =
      (PFNGLUNIFORMMATRIX3X4FVPROC)load("glUniformMatrix3x4fv");
  glad_glUniformMatrix4x3fv =
      (PFNGLUNIFORMMATRIX4X3FVPROC)load("glUniformMatrix4x3fv");
}
static void load_GL_VERSION_3_0(GLADloadproc load) {
  if (!GLAD_GL_VERSION_3_0)
    return;
  glad_glColorMaski = (PFNGLCOLORMASKIPROC)load("glColorMaski");
  glad_glGetBooleani_v = (PFNGLGETBOOLEANI_VPROC)load("glGetBooleani_v");
  glad_glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)load("glGetIntegeri_v");
  glad_glEnablei = (PFNGLENABLEIPROC)load("glEnablei");
  glad_glDisablei = (PFNGLDISABLEIPROC)load("glDisablei");
  glad_glIsEnabledi = (PFNGLISENABLEDIPROC)load("glIsEnabledi");
  glad_glBeginTransformFeedback =
      (PFNGLBEGINTRANSFORMFEEDBACKPROC)load("glBeginTransformFeedback");
  glad_glEndTransformFeedback =
      (PFNGLENDTRANSFORMFEEDBACKPROC)load("glEndTransformFeedback");
  glad_glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)load("glBindBufferRange");
  glad_glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)load("glBindBufferBase");
  glad_glTransformFeedbackVaryings =
      (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)load("glTransformFeedbackVaryings");
  glad_glGetTransformFeedbackVarying =
      (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)load(
          "glGetTransformFeedbackVarying");
  glad_glClampColor = (PFNGLCLAMPCOLORPROC)load("glClampColor");
  glad_glBeginConditionalRender =
      (PFNGLBEGINCONDITIONALRENDERPROC)load("glBeginConditionalRender");
  glad_glEndConditionalRender =
      (PFNGLENDCONDITIONALRENDERPROC)load("glEndConditionalRender");
  glad_glVertexAttribIPointer =
      (PFNGLVERTEXATTRIBIPOINTERPROC)load("glVertexAttribIPointer");
  glad_glGetVertexAttribIiv =
      (PFNGLGETVERTEXATTRIBIIVPROC)load("glGetVertexAttribIiv");
  glad_glGetVertexAttribIuiv =
      (PFNGLGETVERTEXATTRIBIUIVPROC)load("glGetVertexAttribIuiv");
  glad_glVertexAttribI1i = (PFNGLVERTEXATTRIBI1IPROC)load("glVertexAttribI1i");
  glad_glVertexAttribI2i = (PFNGLVERTEXATTRIBI2IPROC)load("glVertexAttribI2i");
  glad_glVertexAttribI3i = (PFNGLVERTEXATTRIBI3IPROC)load("glVertexAttribI3i");
  glad_glVertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC)load("glVertexAttribI4i");
  glad_glVertexAttribI1ui =
      (PFNGLVERTEXATTRIBI1UIPROC)load("glVertexAttribI1ui");
  glad_glVertexAttribI2ui =
      (PFNGLVERTEXATTRIBI2UIPROC)load("glVertexAttribI2ui");
  glad_glVertexAttribI3ui =
      (PFNGLVERTEXATTRIBI3UIPROC)load("glVertexAttribI3ui");
  glad_glVertexAttribI4ui =
      (PFNGLVERTEXATTRIBI4UIPROC)load("glVertexAttribI4ui");
  glad_glVertexAttribI1iv =
      (PFNGLVERTEXATTRIBI1IVPROC)load("glVertexAttribI1iv");
  glad_glVertexAttribI2iv =
      (PFNGLVERTEXATTRIBI2IVPROC)load("glVertexAttribI2iv");
  glad_glVertexAttribI3iv =
      (PFNGLVERTEXATTRIBI3IVPROC)load("glVertexAttribI3iv");
  glad_glVertexAttribI4iv =
      (PFNGLVERTEXATTRIBI4IVPROC)load("glVertexAttribI4iv");
  glad_glVertexAttribI1uiv =
      (PFNGLVERTEXATTRIBI1UIVPROC)load("glVertexAttribI1uiv");
  glad_glVertexAttribI2uiv =
      (PFNGLVERTEXATTRIBI2UIVPROC)load("glVertexAttribI2uiv");
  glad_glVertexAttribI3uiv =
      (PFNGLVERTEXATTRIBI3UIVPROC)load("glVertexAttribI3uiv");
  glad_glVertexAttribI4uiv =
      (PFNGLVERTEXATTRIBI4UIVPROC)load("glVertexAttribI4uiv");
  glad_glVertexAttribI4bv =
      (PFNGLVERTEXATTRIBI4BVPROC)load("glVertexAttribI4bv");
  glad_glVertexAttribI4sv =
      (PFNGLVERTEXATTRIBI4SVPROC)load("glVertexAttribI4sv");
  glad_glVertexAttribI4ubv =
      (PFNGLVERTEXATTRIBI4UBVPROC)load("glVertexAttribI4ubv");
  glad_glVertexAttribI4usv =
      (PFNGLVERTEXATTRIBI4USVPROC)load("glVertexAttribI4usv");
  glad_glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC)load("glGetUniformuiv");
  glad_glBindFragDataLocation =
      (PFNGLBINDFRAGDATALOCATIONPROC)load("glBindFragDataLocation");
  glad_glGetFragDataLocation =
      (PFNGLGETFRAGDATALOCATIONPROC)load("glGetFragDataLocation");
  glad_glUniform1ui = (PFNGLUNIFORM1UIPROC)load("glUniform1ui");
  glad_glUniform2ui = (PFNGLUNIFORM2UIPROC)load("glUniform2ui");
  glad_glUniform3ui = (PFNGLUNIFORM3UIPROC)load("glUniform3ui");
  glad_glUniform4ui = (PFNGLUNIFORM4UIPROC)load("glUniform4ui");
  glad_glUniform1uiv = (PFNGLUNIFORM1UIVPROC)load("glUniform1uiv");
  glad_glUniform2uiv = (PFNGLUNIFORM2UIVPROC)load("glUniform2uiv");
  glad_glUniform3uiv = (PFNGLUNIFORM3UIVPROC)load("glUniform3uiv");
  glad_glUniform4uiv = (PFNGLUNIFORM4UIVPROC)load("glUniform4uiv");
  glad_glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC)load("glTexParameterIiv");
  glad_glTexParameterIuiv =
      (PFNGLTEXPARAMETERIUIVPROC)load("glTexParameterIuiv");
  glad_glGetTexParameterIiv =
      (PFNGLGETTEXPARAMETERIIVPROC)load("glGetTexParameterIiv");
  glad_glGetTexParameterIuiv =
      (PFNGLGETTEXPARAMETERIUIVPROC)load("glGetTexParameterIuiv");
  glad_glClearBufferiv = (PFNGLCLEARBUFFERIVPROC)load("glClearBufferiv");
  glad_glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC)load("glClearBufferuiv");
  glad_glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)load("glClearBufferfv");
  glad_glClearBufferfi = (PFNGLCLEARBUFFERFIPROC)load("glClearBufferfi");
  glad_glGetStringi = (PFNGLGETSTRINGIPROC)load("glGetStringi");
  glad_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)load("glIsRenderbuffer");
  glad_glBindRenderbuffer =
      (PFNGLBINDRENDERBUFFERPROC)load("glBindRenderbuffer");
  glad_glDeleteRenderbuffers =
      (PFNGLDELETERENDERBUFFERSPROC)load("glDeleteRenderbuffers");
  glad_glGenRenderbuffers =
      (PFNGLGENRENDERBUFFERSPROC)load("glGenRenderbuffers");
  glad_glRenderbufferStorage =
      (PFNGLRENDERBUFFERSTORAGEPROC)load("glRenderbufferStorage");
  glad_glGetRenderbufferParameteriv =
      (PFNGLGETRENDERBUFFERPARAMETERIVPROC)load("glGetRenderbufferParameteriv");
  glad_glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)load("glIsFramebuffer");
  glad_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)load("glBindFramebuffer");
  glad_glDeleteFramebuffers =
      (PFNGLDELETEFRAMEBUFFERSPROC)load("glDeleteFramebuffers");
  glad_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)load("glGenFramebuffers");
  glad_glCheckFramebufferStatus =
      (PFNGLCHECKFRAMEBUFFERSTATUSPROC)load("glCheckFramebufferStatus");
  glad_glFramebufferTexture1D =
      (PFNGLFRAMEBUFFERTEXTURE1DPROC)load("glFramebufferTexture1D");
  glad_glFramebufferTexture2D =
      (PFNGLFRAMEBUFFERTEXTURE2DPROC)load("glFramebufferTexture2D");
  glad_glFramebufferTexture3D =
      (PFNGLFRAMEBUFFERTEXTURE3DPROC)load("glFramebufferTexture3D");
  glad_glFramebufferRenderbuffer =
      (PFNGLFRAMEBUFFERRENDERBUFFERPROC)load("glFramebufferRenderbuffer");
  glad_glGetFramebufferAttachmentParameteriv =
      (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)load(
          "glGetFramebufferAttachmentParameteriv");
  glad_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)load("glGenerateMipmap");
  glad_glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)load("glBlitFramebuffer");
  glad_glRenderbufferStorageMultisample =
      (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)load(
          "glRenderbufferStorageMultisample");
  glad_glFramebufferTextureLayer =
      (PFNGLFRAMEBUFFERTEXTURELAYERPROC)load("glFramebufferTextureLayer");
  glad_glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)load("glMapBufferRange");
  glad_glFlushMappedBufferRange =
      (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)load("glFlushMappedBufferRange");
  glad_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
  glad_glDeleteVertexArrays =
      (PFNGLDELETEVERTEXARRAYSPROC)load("glDeleteVertexArrays");
  glad_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
  glad_glIsVertexArray = (PFNGLISVERTEXARRAYPROC)load("glIsVertexArray");
}
static void load_GL_VERSION_3_1(GLADloadproc load) {
  if (!GLAD_GL_VERSION_3_1)
    return;
  glad_glDrawArraysInstanced =
      (PFNGLDRAWARRAYSINSTANCEDPROC)load("glDrawArraysInstanced");
  glad_glDrawElementsInstanced =
      (PFNGLDRAWELEMENTSINSTANCEDPROC)load("glDrawElementsInstanced");
  glad_glTexBuffer = (PFNGLTEXBUFFERPROC)load("glTexBuffer");
  glad_glPrimitiveRestartIndex =
      (PFNGLPRIMITIVERESTARTINDEXPROC)load("glPrimitiveRestartIndex");
  glad_glCopyBufferSubData =
      (PFNGLCOPYBUFFERSUBDATAPROC)load("glCopyBufferSubData");
  glad_glGetUniformIndices =
      (PFNGLGETUNIFORMINDICESPROC)load("glGetUniformIndices");
  glad_glGetActiveUniformsiv =
      (PFNGLGETACTIVEUNIFORMSIVPROC)load("glGetActiveUniformsiv");
  glad_glGetActiveUniformName =
      (PFNGLGETACTIVEUNIFORMNAMEPROC)load("glGetActiveUniformName");
  glad_glGetUniformBlockIndex =
      (PFNGLGETUNIFORMBLOCKINDEXPROC)load("glGetUniformBlockIndex");
  glad_glGetActiveUniformBlockiv =
      (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)load("glGetActiveUniformBlockiv");
  glad_glGetActiveUniformBlockName =
      (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)load("glGetActiveUniformBlockName");
  glad_glUniformBlockBinding =
      (PFNGLUNIFORMBLOCKBINDINGPROC)load("glUniformBlockBinding");
  glad_glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)load("glBindBufferRange");
  glad_glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)load("glBindBufferBase");
  glad_glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)load("glGetIntegeri_v");
}
static void load_GL_VERSION_3_2(GLADloadproc load) {
  if (!GLAD_GL_VERSION_3_2)
    return;
  glad_glDrawElementsBaseVertex =
      (PFNGLDRAWELEMENTSBASEVERTEXPROC)load("glDrawElementsBaseVertex");
  glad_glDrawRangeElementsBaseVertex =
      (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC)load(
          "glDrawRangeElementsBaseVertex");
  glad_glDrawElementsInstancedBaseVertex =
      (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)load(
          "glDrawElementsInstancedBaseVertex");
  glad_glMultiDrawElementsBaseVertex =
      (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC)load(
          "glMultiDrawElementsBaseVertex");
  glad_glProvokingVertex = (PFNGLPROVOKINGVERTEXPROC)load("glProvokingVertex");
  glad_glFenceSync = (PFNGLFENCESYNCPROC)load("glFenceSync");
  glad_glIsSync = (PFNGLISSYNCPROC)load("glIsSync");
  glad_glDeleteSync = (PFNGLDELETESYNCPROC)load("glDeleteSync");
  glad_glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)load("glClientWaitSync");
  glad_glWaitSync = (PFNGLWAITSYNCPROC)load("glWaitSync");
  glad_glGetInteger64v = (PFNGLGETINTEGER64VPROC)load("glGetInteger64v");
  glad_glGetSynciv = (PFNGLGETSYNCIVPROC)load("glGetSynciv");
  glad_glGetInteger64i_v = (PFNGLGETINTEGER64I_VPROC)load("glGetInteger64i_v");
  glad_glGetBufferParameteri64v =
      (PFNGLGETBUFFERPARAMETERI64VPROC)load("glGetBufferParameteri64v");
  glad_glFramebufferTexture =
      (PFNGLFRAMEBUFFERTEXTUREPROC)load("glFramebufferTexture");
  glad_glTexImage2DMultisample =
      (PFNGLTEXIMAGE2DMULTISAMPLEPROC)load("glTexImage2DMultisample");
  glad_glTexImage3DMultisample =
      (PFNGLTEXIMAGE3DMULTISAMPLEPROC)load("glTexImage3DMultisample");
  glad_glGetMultisamplefv =
      (PFNGLGETMULTISAMPLEFVPROC)load("glGetMultisamplefv");
  glad_glSampleMaski = (PFNGLSAMPLEMASKIPROC)load("glSampleMaski");
}
static void load_GL_ARB_multisample(GLADloadproc load) {
  if (!GLAD_GL_ARB_multisample)
    return;
  glad_glSampleCoverageARB =
      (PFNGLSAMPLECOVERAGEARBPROC)load("glSampleCoverageARB");
}
static void load_GL_ARB_robustness(GLADloadproc load) {
  if (!GLAD_GL_ARB_robustness)
    return;
  glad_glGetGraphicsResetStatusARB =
      (PFNGLGETGRAPHICSRESETSTATUSARBPROC)load("glGetGraphicsResetStatusARB");
  glad_glGetnTexImageARB = (PFNGLGETNTEXIMAGEARBPROC)load("glGetnTexImageARB");
  glad_glReadnPixelsARB = (PFNGLREADNPIXELSARBPROC)load("glReadnPixelsARB");
  glad_glGetnCompressedTexImageARB =
      (PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC)load("glGetnCompressedTexImageARB");
  glad_glGetnUniformfvARB =
      (PFNGLGETNUNIFORMFVARBPROC)load("glGetnUniformfvARB");
  glad_glGetnUniformivARB =
      (PFNGLGETNUNIFORMIVARBPROC)load("glGetnUniformivARB");
  glad_glGetnUniformuivARB =
      (PFNGLGETNUNIFORMUIVARBPROC)load("glGetnUniformuivARB");
  glad_glGetnUniformdvARB =
      (PFNGLGETNUNIFORMDVARBPROC)load("glGetnUniformdvARB");
  glad_glGetnMapdvARB = (PFNGLGETNMAPDVARBPROC)load("glGetnMapdvARB");
  glad_glGetnMapfvARB = (PFNGLGETNMAPFVARBPROC)load("glGetnMapfvARB");
  glad_glGetnMapivARB = (PFNGLGETNMAPIVARBPROC)load("glGetnMapivARB");
  glad_glGetnPixelMapfvARB =
      (PFNGLGETNPIXELMAPFVARBPROC)load("glGetnPixelMapfvARB");
  glad_glGetnPixelMapuivARB =
      (PFNGLGETNPIXELMAPUIVARBPROC)load("glGetnPixelMapuivARB");
  glad_glGetnPixelMapusvARB =
      (PFNGLGETNPIXELMAPUSVARBPROC)load("glGetnPixelMapusvARB");
  glad_glGetnPolygonStippleARB =
      (PFNGLGETNPOLYGONSTIPPLEARBPROC)load("glGetnPolygonStippleARB");
  glad_glGetnColorTableARB =
      (PFNGLGETNCOLORTABLEARBPROC)load("glGetnColorTableARB");
  glad_glGetnConvolutionFilterARB =
      (PFNGLGETNCONVOLUTIONFILTERARBPROC)load("glGetnConvolutionFilterARB");
  glad_glGetnSeparableFilterARB =
      (PFNGLGETNSEPARABLEFILTERARBPROC)load("glGetnSeparableFilterARB");
  glad_glGetnHistogramARB =
      (PFNGLGETNHISTOGRAMARBPROC)load("glGetnHistogramARB");
  glad_glGetnMinmaxARB = (PFNGLGETNMINMAXARBPROC)load("glGetnMinmaxARB");
}
static void find_extensionsGL(void) {
  GLAD_GL_EXT_separate_specular_color =
      has_ext("GL_EXT_separate_specular_color");
  GLAD_GL_ARB_multisample = has_ext("GL_ARB_multisample");
  GLAD_GL_ARB_robustness = has_ext("GL_ARB_robustness");
}

static void find_coreGL(void) {
  /* Thank you @elmindreda
   * https://github.com/elmindreda/greg/blob/master/templates/greg.c.in#L176
   * https://github.com/glfw/glfw/blob/master/src/context.c#L36
   */
  int i, major, minor;

  const char* version;
  const char* prefixes[] = {"OpenGL ES-CM ", "OpenGL ES-CL ", "OpenGL ES ",
                            NULL};

  version = (const char*)glGetString(GL_VERSION);
  if (!version)
    return;

  for (i = 0; prefixes[i]; i++) {
    const size_t length = strlen(prefixes[i]);
    if (strncmp(version, prefixes[i], length) == 0) {
      version += length;
      break;
    }
  }

/* PR #18 */
#ifdef _MSC_VER
  sscanf_s(version, "%d.%d", &major, &minor);
#else
  sscanf(version, "%d.%d", &major, &minor);
#endif

  GLVersion.major = major;
  GLVersion.minor = minor;
  GLAD_GL_VERSION_1_0 = (major == 1 && minor >= 0) || major > 1;
  GLAD_GL_VERSION_1_1 = (major == 1 && minor >= 1) || major > 1;
  GLAD_GL_VERSION_1_2 = (major == 1 && minor >= 2) || major > 1;
  GLAD_GL_VERSION_1_3 = (major == 1 && minor >= 3) || major > 1;
  GLAD_GL_VERSION_1_4 = (major == 1 && minor >= 4) || major > 1;
  GLAD_GL_VERSION_1_5 = (major == 1 && minor >= 5) || major > 1;
  GLAD_GL_VERSION_2_0 = (major == 2 && minor >= 0) || major > 2;
  GLAD_GL_VERSION_2_1 = (major == 2 && minor >= 1) || major > 2;
  GLAD_GL_VERSION_3_0 = (major == 3 && minor >= 0) || major > 3;
  GLAD_GL_VERSION_3_1 = (major == 3 && minor >= 1) || major > 3;
  GLAD_GL_VERSION_3_2 = (major == 3 && minor >= 2) || major > 3;
}

int gladLoadGLLoader(GLADloadproc load) {
  GLVersion.major = 0;
  GLVersion.minor = 0;
  glGetString = (PFNGLGETSTRINGPROC)load("glGetString");
  if (glGetString == NULL)
    return 0;
  if (glGetString(GL_VERSION) == NULL)
    return 0;
  find_coreGL();
  load_GL_VERSION_1_0(load);
  load_GL_VERSION_1_1(load);
  load_GL_VERSION_1_2(load);
  load_GL_VERSION_1_3(load);
  load_GL_VERSION_1_4(load);
  load_GL_VERSION_1_5(load);
  load_GL_VERSION_2_0(load);
  load_GL_VERSION_2_1(load);
  load_GL_VERSION_3_0(load);
  load_GL_VERSION_3_1(load);
  load_GL_VERSION_3_2(load);

  find_extensionsGL();
  load_GL_ARB_multisample(load);
  load_GL_ARB_robustness(load);
  return GLVersion.major != 0 || GLVersion.minor != 0;
}
