/**
 * Copyright (C) 2015-2018 Think Silicon S.A. (https://think-silicon.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public v3
 * License as published by the Free Software Foundation;
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */

#include "texture2d_color.h"

static  openGL_mesh_t      mesh_screen_quad;
static  openGL_program_t   program;
static  openGL_camera_t    camera;
static  openGL_viewport_t  viewport;
static  openGL_rendering_t rendering;
static  openGL_shading_t   shading;
static  const char        *win_name;

bool InitGL()
{
// Print GPU specifications
    GpuViewer();

#ifdef BINARY_PROG
// Load Precompilde binary program
    if(!LoadProgramBinary(BINARY_PROGRAM_SHADER_NAME, 1, &program.mID))
        return false;
#else
// Load Vertex Shader
    if(!LoadShader(VERTEX_SHADER_NAME   , &program.mVertexShader  , GL_VERTEX_SHADER))
        return false;
// Load Fragment Shader
    if(!LoadShader(FRAGMENT_SHADER_NAME , &program.mFragmentShader, GL_FRAGMENT_SHADER))
        return false;
// Load Program Shader
    if(!LoadProgram(program.mVertexShader, program.mFragmentShader, &program.mID))
        return false;
#endif

// Free Shader Compiler Resources
    glReleaseShaderCompiler();

// Initialize Program
    InitProgram(&program);

// Initialize Mesh
    InitMesh      (&mesh_screen_quad, 2, 2, squad_vertex_buffer_data, sizeof(squad_vertex_buffer_data),
                                            squad_uv_buffer_data    , sizeof(squad_uv_buffer_data)    ,
                                            squad_color_buffer_data , sizeof(squad_color_buffer_data) ,
                                            squad_index_buffer_data , sizeof(squad_index_buffer_data) ,
                                            diffuse_textures, 1);

// Initialize Camera
    InitCamera    (&camera);

// Basic Rendering Setup, (a) Init Depth & (b) Init Background Color
    InitRendering (&rendering, COLOR_BLACK);

// Basic Shading Setup, (a) Init Shading Count & (b) Init uniform value
    InitShading   (&shading  , 1, 0.5f);

// Culling Setup
    glEnable      (GL_CULL_FACE);
    glCullFace    (GL_BACK);
    glFrontFace   (GL_CW);

// Depth Testing Setup
    glEnable      (GL_DEPTH_TEST);
    glDepthFunc   (GL_LEQUAL);
    glClearDepthf (rendering.mDepth);

// Set Clear Color
    glClearColor  (rendering.mBackgroundColor[0], rendering.mBackgroundColor[1], rendering.mBackgroundColor[2], rendering.mBackgroundColor[3]);

// Upload shader uniforms
    glUseProgram(program.mID);
    program.mLocationPos = glGetAttribLocation (program.mID, "v_posCoord_in");
    program.mLocationUV  = glGetAttribLocation (program.mID, "v_texCoord_in");
    glUniform1i(glGetUniformLocation(program.mID, "uniform_texture"), 0);

#ifdef INFO_DISPLAY
    printf("[Shading    Mode] [%s] [Total Time] [%d sec]\n", shading_titles[shading.mType], KILL_APP_PERIOD);
#endif
// Check for opengGL-relate Errors
    ASSERT_NO_GL_ERROR();

    return true;
}

void DrawGL()
{
  // Set Viewport
    glViewport(0, 0, viewport.mWidth, viewport.mHeight);

// Clear Depth & Color of Screen Buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Draw Scene
    DrawMesh(&program, &mesh_screen_quad);

// Check for opengGL-relate Errors
    ASSERT_NO_GL_ERROR();
}

void IdleGL(void)
{
    static float  mix_value         = 0.005f;
    static double totalTimeScript   = 0.0;

    double timePerFrame = GpuTimer(win_name);

    totalTimeScript   += timePerFrame;
    if(totalTimeScript >= (float)KILL_APP_PERIOD)
        KeyboardGL(ESC_KEY);

// Redraw
    eglutPostRedisplay();
}

void DestroyGL(void)
{
// Delete Program
    DeleteProgram (program.mID);
// Delete Mesh
    DeleteMesh    (&mesh_screen_quad);
}

void ReshapeGL(int width, int height)
{
// Set viewport
    SetViewport(&viewport, 0, 0, width, height);

// Check for opengGL-relate Errors
    ASSERT_NO_GL_ERROR();
}

void KeyboardGL(unsigned char key)
{
   if      (key == ESC_KEY) // escape key
   {
// Close app
     DestroyGL();
     if (_eglut->current)
        eglutDestroyWindow(_eglut->current->index);
     _eglutFini();

      exit(0);
   }
}

int main(int argc, char **argv)
{
    win_name = EXECUTABLE_NAME(argv[0]);

    eglutInitWindowSize (WIDTH, HEIGHT);
    eglutInitAPIMask    (EGLUT_OPENGL_ES2_BIT);
    eglutInit           (argc, (const char **)argv);

    int win = eglutCreateWindow(win_name);

    eglutIdleFunc       (IdleGL);
    eglutDisplayFunc    (DrawGL);
    eglutReshapeFunc    (ReshapeGL);
    eglutKeyboardFunc   (KeyboardGL);

    if(!InitGL()) {
        return 1;
    }

    if(SmokeTestsRunning()) {
        ReshapeGL(WIDTH, HEIGHT);
        char *fileName = EXECUTABLE_NAME(argv[0]);
        TakeScreenshot(fileName, DrawGL, WIDTH, HEIGHT);
        DestroyGL();
        eglutDestroyWindow(win);
        _eglutFini();

    } else {
        eglutMainLoop();
        DestroyGL();
    }

    return 0;
}
