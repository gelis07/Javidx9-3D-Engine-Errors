
#include "CreateTriangle.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


const char* VertexShaderSource = 
"#version 330 core\n"
"\n"
"layout(location=0) in vec4 position;"
"\n"
"uniform mat4 u_MVP;"
"\n"
"void main()\n"
"{\n"
"   gl_Position = u_MVP * position;\n"
"};\n";

const char* fragmentShaderSource = 
"#version 330 core\n"
"\n"
"layout(location=0) out vec4 color;"
"\n"
"uniform vec4 u_Color;\n"
"void main()\n"
"{\n"
"   color = u_Color;\n"
"};\n";


Triangles::Triangles(){
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    glCompileShader(fragmentShader);
    

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgram, vertexShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    Shader = shaderProgram;
}
void Triangles::CreateTriangle(const std::array<Pos2D,3> &positions, Color color)
{
    Colors.push_back(color);
    unsigned int VertexBuffer;
    unsigned int VertexArray;
    glGenBuffers(1, &VertexBuffer);
    glGenVertexArrays(1, &VertexArray);
    VertexArrays.push_back(VertexArray);
    VertexBuffers.push_back(VertexBuffer);



    glBindVertexArray(VertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    // glBufferData(GL_ARRAY_BUFFER, 6*sizeof(float), positions.data(), GL_STATIC_DRAW);
      glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2,GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    glEnableVertexAttribArray(0);

    int location = glGetUniformLocation(Shader, "u_Color");
    ColorLocations.push_back(location);
}