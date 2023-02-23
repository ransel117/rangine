/*
* MIT License
*
* Copyright (c) 2023 Ransel117
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#ifndef RANGINE_H
#define RANGINE_H

#if defined(__GNUC__) || defined(__clang__)
/* GCC-compatible compiler (gcc, clang) */
#define RANGINE_INLINE static inline __attribute__((always_inline))
#elif defined(_MSC_VER)
/* Microsoft */
#define RANGINE_INLINE static inline __forceinline
#else
/* Unknown */
#define RANGINE_INLINE static inline
#endif

#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include <ranmath/ranmath.h>

/* ---------------- TYPES ---------------- */
typedef struct {
    vec2 pos;
    vec2 size;
} AABB;
typedef struct {
    vec2 start;
    vec2 end;
    f32 width;
} Line;
typedef struct {
    char *data;
    size_t len;
    bool is_valid;
} File;

/* ----------------- METHODS ----------------- */
#define RG_ERROR_RETURN(R, ...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); return R; } while(0);
#define RG_ERROR_EXIT(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); exit(1); } while(0);

RANGINE_INLINE SDL_Window *rg_init(const char*, const u32, const u32);
RANGINE_INLINE SDL_Window *rg_init_window(const char*, const u32, const u32);
RANGINE_INLINE u32  rg_shader_create(const char*, const char*);
RANGINE_INLINE void rg_shader_init(u32*, const f32, const f32);
RANGINE_INLINE void rg_color_texture_init(u32*);
RANGINE_INLINE void rg_line_init(u32*, u32*);
RANGINE_INLINE void rg_aabb_init(u32*, u32*, u32*);
RANGINE_INLINE void rg_render_begin(void);
RANGINE_INLINE void rg_render_end(SDL_Window*);
RANGINE_INLINE void rg_line_draw(Line, vec4);
RANGINE_INLINE void rg_aabb_draw(AABB, vec4);
RANGINE_INLINE void rg_aabb_line_draw(AABB, const f32, vec4);
RANGINE_INLINE i32  rg_file_write(const void*, const size_t, const char*);
RANGINE_INLINE File rg_file_read(const char*);
RANGINE_INLINE i32  rg_exit(SDL_Window*);

static const char* default_shaders[2] = {
    "./shaders/shader_default.vert",
    "./shaders/shader_default.frag",
};

#endif /* RANGINE_H */

#ifdef RANGINE_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <glad/gl.h>
#include <SDL2/SDL.h>

#define RANMATH_IMPLEMENTATION
#include <ranmath/ranmath.h>

static u32 m_shader_default;
static u32 m_texture_color;
static u32 m_vao_line;
static u32 m_vbo_line;
static u32 m_vao_aabb;
static u32 m_vbo_aabb;
static u32 m_ebo_aabb;

RANGINE_INLINE SDL_Window *rg_init(const char* title, const u32 width, const u32 height) {
    SDL_Window *m_window = rg_init_window(title, width, height);

    rg_shader_init(&m_shader_default, (f32)width/3, (f32)height/3);
    rg_color_texture_init(&m_texture_color);
    rg_line_init(&m_vao_line, &m_vbo_line);
    rg_aabb_init(&m_vao_aabb, &m_vbo_aabb, &m_ebo_aabb);

    return m_window;
}
RANGINE_INLINE SDL_Window *rg_init_window(const char *title, const u32 width, const u32 height) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        RG_ERROR_EXIT("Could not init SDL: %s", SDL_GetError());
    }

    SDL_Window *m_window = SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    width,
    height,
    SDL_WINDOW_OPENGL
    );

    if (!m_window) {
        RG_ERROR_EXIT("Failed to init window: %s\n", SDL_GetError());
    }

    SDL_GL_CreateContext(m_window);

    if (gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0) {
        RG_ERROR_EXIT("Failed to load GL: %s", SDL_GetError());
    }

    printf("OpenGL Loaded\n");
    printf("Vendor:  %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version: %s\n", glGetString(GL_VERSION));

    return m_window;
}
RANGINE_INLINE u32 rg_shader_create(const char *path_vert, const char *path_frag) {
    int success;
    char log[512];

    File file_vertex = rg_file_read(path_vert);
    if (!file_vertex.is_valid) {
        RG_ERROR_EXIT("Error reading shader: %s\n", path_vert);
    }

    u32 shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader_vertex, 1, (const char *const*)&file_vertex.data, NULL);
    glCompileShader(shader_vertex);
    glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader_vertex, 512, NULL, log);
        RG_ERROR_EXIT("Error compiling vertex shader. %s\n", log);
    }

    File file_fragment = rg_file_read(path_frag);
    if (!file_fragment.is_valid) {
        RG_ERROR_EXIT("Error reading shader: %s\n", path_frag);
    }

    u32 shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader_fragment, 1, (const char *const*)&file_fragment.data, NULL);
    glCompileShader(shader_fragment);
    glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader_fragment, 512, NULL, log);
        RG_ERROR_EXIT("Error compiling fragment shader. %s\n", log);
    }

    u32 shader = glCreateProgram();
    glAttachShader(shader, shader_vertex);
    glAttachShader(shader, shader_fragment);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader, 512, NULL, log);
        RG_ERROR_EXIT("Error linking shader. %s\n", log);
    }

    free(file_vertex.data);
    free(file_fragment.data);

    return shader;
}
RANGINE_INLINE void rg_shader_init(u32 *shader_default, const f32 render_width, const f32 render_height) {
    RM_MAT4_CVT projection;

    projection.m = rm_mat4_ortho(0, render_width, 0, render_height, -2, 2);

    *shader_default = rg_shader_create(default_shaders[0], default_shaders[1]);

    glUseProgram(*shader_default);
    glUniformMatrix4fv(glGetUniformLocation(*shader_default, "projection"), 1, GL_FALSE, &projection.f[0][0]);
}
RANGINE_INLINE void rg_color_texture_init(u32 *texture) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    u8 solid_white[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, solid_white);

    glBindTexture(GL_TEXTURE_2D, 0);
}
RANGINE_INLINE void rg_line_init(u32 *vao, u32 *vbo) {
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(f32), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}
RANGINE_INLINE void rg_aabb_init(u32 *vao, u32 *vbo, u32 *ebo) {
    /* x, y, z, u, v */
    f32 vertices[20] = {
        0.5, 0.5, 0, 0, 0,
        0.5, -0.5, 0, 0, 1,
        -0.5, -0.5, 0, 1, 1,
        -0.5, 0.5, 0, 1, 0
    };

    u32 indices[6] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glGenBuffers(1, ebo);

    glBindVertexArray(*vao);

    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    /* xyz */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), NULL);
    glEnableVertexAttribArray(0);

    /* uv */
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

RANGINE_INLINE void rg_render_begin(void) {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}
RANGINE_INLINE void rg_render_end(SDL_Window *window) {
    SDL_GL_SwapWindow(window);
}

RANGINE_INLINE void rg_line_draw(Line l, vec4 color) {
    f32 x, y;
    RM_VEC4_CVT line_color;
    RM_MAT4_CVT line_model;

    x = l.end.x - l.start.x;
    y = l.end.y - l.start.y;
    f32 line[6] = {0, 0, 0, x, y, 0};

    line_color.v = rm_vec4_copy(color);

    line_model.m = rm_mat4_translate(l.start.x, l.start.y, 0);

    glUseProgram(m_shader_default);
    glLineWidth(l.width);

    glUniformMatrix4fv(glGetUniformLocation(m_shader_default, "model"), 1, GL_FALSE, &line_model.f[0][0]);
    glUniform4fv(glGetUniformLocation(m_shader_default, "color"), 1, line_color.f);

    glBindVertexArray(m_vao_line);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_line);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line), line);

    glBindTexture(GL_TEXTURE_2D, m_texture_color);
    glDrawArrays(GL_LINES, 0, 2);

    glBindVertexArray(0);
}
RANGINE_INLINE void rg_aabb_draw(AABB r, vec4 color) {
    RM_VEC4_CVT aabb_color;
    RM_MAT4_CVT aabb_model;

    aabb_color.v = rm_vec4_copy(color);

    aabb_model.m = rm_mat4_translate(r.pos.x, r.pos.y, 0);
    aabb_model.m = rm_mat4_scale_aniso(aabb_model.m, r.size.x, r.size.y, 1);

    glUseProgram(m_shader_default);

    glUniformMatrix4fv(glGetUniformLocation(m_shader_default, "model"), 1, GL_FALSE, &aabb_model.f[0][0]);
    glUniform4fv(glGetUniformLocation(m_shader_default, "color"), 1, aabb_color.f);

    glBindVertexArray(m_vao_aabb);

    glBindTexture(GL_TEXTURE_2D, m_texture_color);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
}
RANGINE_INLINE void rg_aabb_line_draw(AABB r, const f32 width, vec4 color) {
    vec2 a, b, hsize;
    Line l1, l2, l3, l4;

    hsize = rm_vec2_scale(r.size, 0.5);
    a = rm_vec2_add(r.pos, hsize);
    b = rm_vec2_sub(r.pos, hsize);

    l1 = (Line){
        rm_vec2_copy(b),
        (vec2){a.x, b.y},
        width,
    };
    l2 = (Line){
        rm_vec2_copy(l1.end),
        rm_vec2_copy(a),
        width,
    };
    l3 = (Line){
        rm_vec2_copy(l2.end),
        (vec2){b.x, a.y},
        width
    };
    l4 = (Line){
        rm_vec2_copy(l3.end),
        rm_vec2_copy(l1.start),
        width
    };

    rg_line_draw(l1, color);
    rg_line_draw(l2, color);
    rg_line_draw(l3, color);
    rg_line_draw(l4, color);
}

#define RG_READ_CHUNK_SIZE 2097152
#define RG_READ_ERROR_GENERAL "Error reading file: %s. errno: %d"
#define RG_READ_ERROR_MEMORY "Not enough free memory to read file: %s"

RANGINE_INLINE File rg_file_read(const char *path) {
    File file;
    FILE *fp;
    char *data, *tmp;
    size_t used, size, n;

    file.is_valid = false;
    data = NULL;
    used = 0;
    size = 0;

    fp = fopen(path, "rb");
    if (!fp || ferror(fp)) {
        RG_ERROR_RETURN(file, RG_READ_ERROR_GENERAL, path, errno);
    }

    while (true) {
        if (used + RG_READ_CHUNK_SIZE + 1 > size) {
            size = used + RG_READ_CHUNK_SIZE + 1;
            if (size <= used) {
                free(data);
                RG_ERROR_RETURN(file, "Input file too large: %s", path);
            }

            tmp = realloc(data, size);
            if (!tmp) {
                free(data);
                RG_ERROR_RETURN(file, RG_READ_ERROR_MEMORY, path);
            }
            data = tmp;
        }

        n = fread(data + used, 1, RG_READ_CHUNK_SIZE, fp);
        if (n == 0) {
            break;
        }

        used += n;
    }

    if (ferror(fp)) {
        free(data);
        RG_ERROR_RETURN(file, RG_READ_ERROR_GENERAL, path, errno);
    }

    tmp = realloc(data, used + 1);
    if (!tmp) {
        free(data);
        RG_ERROR_RETURN(file, RG_READ_ERROR_MEMORY, path);
    }

    data = tmp;
    data[used] = 0;

    file.data = data;
    file.len = used;
    file.is_valid = true;

    return file;
}
RANGINE_INLINE i32 rg_file_write(const void *buffer, const size_t size, const char *path) {
    FILE *fp;
    size_t chunks_written;

    fp = fopen(path, "wb");
    if (!fp || ferror(fp)) {
        RG_ERROR_RETURN(1, "Cannot write file: %s.", path);
    }

    chunks_written = fwrite(buffer, size, 1, fp);
    fclose(fp);
    if (chunks_written != 1) {
        RG_ERROR_RETURN(1, "Write error. Expected 1 chunk, got %zu.", chunks_written);
    }

    return 0;
}

RANGINE_INLINE i32 rg_exit(SDL_Window *window) {
    glDeleteVertexArrays(1, &m_vao_line);
    glDeleteVertexArrays(1, &m_vao_aabb);
    glDeleteBuffers(1, &m_vbo_line);
    glDeleteBuffers(1, &m_vbo_aabb);
    glDeleteBuffers(1, &m_ebo_aabb);
    glDeleteProgram(m_shader_default);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

#endif /* RANGINE_IMPLEMENTATION */
