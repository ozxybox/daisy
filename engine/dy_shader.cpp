#include "dy_shader.h"
#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


// GL uniform types as enums
// This makes dealing with uniforms nicer

enum
{
    DY_UNIFORM_FLOAT1,
    DY_UNIFORM_FLOAT2,
    DY_UNIFORM_FLOAT3,
    DY_UNIFORM_FLOAT4,

    DY_UNIFORM_INT1,
    DY_UNIFORM_INT2,
    DY_UNIFORM_INT3,
    DY_UNIFORM_INT4,

    DY_UNIFORM_MAT2,
    DY_UNIFORM_MAT3,
    DY_UNIFORM_MAT4,
};
const unsigned int dy_uniform_type_size[] =
{
    /*float1*/ sizeof(float) * 1,
    /*float2*/ sizeof(float) * 2,
    /*float3*/ sizeof(float) * 3,
    /*float4*/ sizeof(float) * 4,

    /*int1*/   sizeof(int)   * 1,
    /*int2*/   sizeof(int)   * 2,
    /*int3*/   sizeof(int)   * 3,
    /*int4*/   sizeof(int)   * 4,

    /*mat2*/   sizeof(float) * 2 * 2,
    /*mat3*/   sizeof(float) * 3 * 3,
    /*mat4*/   sizeof(float) * 4 * 4,
};


typedef struct dy_parameter
{
    int type;
    const char* name;
    void* data;
    int size;
} dy_parameter;

// This should be the same as DY_SHADERPARAM
#define PARAMETER(type, name) {DY_UNIFORM_ ## type, "u_" #name, 0},
static dy_parameter dy_shader_params[DY_SHADERPARAM_COUNT] = {
    PARAMETER(MAT4, model)
    PARAMETER(MAT4, view)
    PARAMETER(MAT4, projection)
};
#undef PARAMETER

typedef struct dy_shaderdata
{
    GLuint program;

    GLint uniforms[DY_SHADERPARAM_COUNT];
} dy_shaderdata;


static void* dy_shader_param_data;
static dy_shaderdata* dy_shader_current;


// Internal functions //

static void dy_check_error_(GLuint shader)
{
    int infoLogLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0)
    {
        char vertexError[1024];
        if (infoLogLength > 1024)
            infoLogLength = 1024;
        glGetShaderInfoLog(shader, infoLogLength, NULL, &vertexError[0]);
        printf("[shader] Error:\n%s", vertexError);
    }
}
static GLuint dy_compile_shader_(GLenum shaderType, const char* src)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    dy_check_error_(shader);
    return shader;
}
static void dy_apply_parameter_(int idx)
{
    if(!dy_shader_current) return;

    GLint uniform = dy_shader_current->uniforms[idx];
    if (uniform < 0) return;

    void* mem = dy_shader_params[idx].data;
    GLfloat* memf = reinterpret_cast<GLfloat*>(mem);
    GLint* memi = reinterpret_cast<GLint*>(mem);
    switch (dy_shader_params[idx].type)
    {
    case DY_UNIFORM_FLOAT1: glUniform1fv(uniform, 1, memf); break;
    case DY_UNIFORM_FLOAT2: glUniform2fv(uniform, 1, memf); break;
    case DY_UNIFORM_FLOAT3: glUniform3fv(uniform, 1, memf); break;
    case DY_UNIFORM_FLOAT4: glUniform4fv(uniform, 1, memf); break;
    
    case DY_UNIFORM_INT1: glUniform1iv(uniform, 1, memi); break;
    case DY_UNIFORM_INT2: glUniform2iv(uniform, 1, memi); break;
    case DY_UNIFORM_INT3: glUniform3iv(uniform, 1, memi); break;
    case DY_UNIFORM_INT4: glUniform4iv(uniform, 1, memi); break;

    case DY_UNIFORM_MAT2: glUniformMatrix2fv(uniform, 1, 0, memf); break;
    case DY_UNIFORM_MAT3: glUniformMatrix3fv(uniform, 1, 0, memf); break;
    case DY_UNIFORM_MAT4: glUniformMatrix4fv(uniform, 1, 0, memf); break;
    }
}
static void dy_apply_all_parameters_()
{
    for (int i = 0; i < DY_SHADERPARAM_COUNT; i++)
        dy_apply_parameter_(i);
}



// Public facing functions //

void dy_shader_init()
{
    dy_shader_current = 0;

    // Prep uniform data

    // Create a block for the uni data to live in
    int sz = 0;
    for (int i = 0; i < DY_SHADERPARAM_COUNT; i++)
        sz += dy_uniform_type_size[dy_shader_params[i].type];
    dy_shader_param_data = calloc(1, sz);

    // Assign uni data pointers
    char* unidata = (char*)dy_shader_param_data;
    for (int i = 0; i < DY_SHADERPARAM_COUNT; i++)
    {
        dy_shader_params[i].data = unidata;
        unidata += dy_uniform_type_size[dy_shader_params[i].type];
    }

}

dy_shader* dy_shader_create(const char* vs, const char* fs)
{
    dy_shaderdata* data = reinterpret_cast<dy_shaderdata*>(malloc(sizeof(dy_shaderdata)));

    // Compile vertex and fragment shaders
    GLuint vertexShader = dy_compile_shader_(GL_VERTEX_SHADER, vs);
    GLuint fragmentShader = dy_compile_shader_(GL_FRAGMENT_SHADER, fs);

    // Attach them to the program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // Setup the vertex attributes
    glBindAttribLocation(program, 0, "a_pos");
    glBindAttribLocation(program, 1, "a_color");
    glBindAttribLocation(program, 2, "a_uv");

    // Link!
    glLinkProgram(program);
    data->program = program;

    // Get all uniforms for this shader
    for (int i = 0; i < DY_SHADERPARAM_COUNT; i++)
        data->uniforms[i] = glGetUniformLocation(program, dy_shader_params[i].name);

    // Clean up the scraps
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return data;
}
void dy_shader_destroy(dy_shader* shader)
{
    dy_shaderdata* data = reinterpret_cast<dy_shaderdata*>(shader);

    glDeleteProgram(data->program);
    free(shader);
}

void dy_shader_bind(dy_shader* shader)
{
    dy_shaderdata* data = reinterpret_cast<dy_shaderdata*>(shader);

    dy_shader_current = data;

    glUseProgram(data->program);
    dy_apply_all_parameters_();
}
void dy_shader_set(int parameter, void* data)
{
    assert(parameter >= 0 && parameter < DY_SHADERPARAM_COUNT);

    dy_parameter* uni = &dy_shader_params[parameter];

    // Copy it in so we can apply it elsewhere later
    memcpy(uni->data, data, dy_uniform_type_size[uni->type]);

    // Apply it
    dy_apply_parameter_(parameter);
}

 