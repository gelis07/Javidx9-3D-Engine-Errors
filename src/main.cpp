#define GLEW_STATIC
#include <GL/glew.h>
#include "CreateTriangle.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
#include <array>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Engine.h"
#include <crtdbg.h>

int main(void)
{
    GLFWwindow* window;
    ScreenCoord coord;
    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(coord.width, coord.height, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    if(glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;
    Engine engine(&window);
    glm::mat4 proj = glm::ortho(0.0f, coord.width, 0.0f, coord.height, -1.0f, 1.0f);
    float time = 0.02f;
    float Speed = 8.0f;
    std::array<float,3> offset = {1, 0, 0};

    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark();
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {   
 
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
            std::cout << "OpenGL Error: " << error << std::endl;

        engine.OnUpdate(time, offset, Speed);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplGlfwGL3_NewFrame();
        for (size_t i = 0; i < engine.TriangleRenderer.VertexArrays.size(); i++)
        {
            glUseProgram(engine.TriangleRenderer.Shader);
            glUniform4f(engine.TriangleRenderer.ColorLocations[i], engine.TriangleRenderer.Colors[i].r, engine.TriangleRenderer.Colors[i].g, engine.TriangleRenderer.Colors[i].b, engine.TriangleRenderer.Colors[i].a);
            int location = glGetUniformLocation(engine.TriangleRenderer.Shader, "u_MVP");
            if(location == -1)
            {
                std::cout << "Error uniform not found" << std::endl;
            }else{
                glUniformMatrix4fv(location, 1,GL_FALSE, &proj[0][0]);
            }

            glBindVertexArray(engine.TriangleRenderer.VertexArrays[i]);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glDeleteVertexArrays(1, &engine.TriangleRenderer.VertexArrays[i]);
            glDeleteBuffers(1, &engine.TriangleRenderer.VertexBuffers[i]);
        }
        {
            ImGui::SetNextWindowSize(ImVec2(350, 100));
            ImGui::Begin("Transform", NULL, ImGuiWindowFlags_NoResize);
            ImGui::SliderFloat("Rotation Speed", &time, 0.0f, 0.1f);
            ImGui::SliderFloat("Speed", &Speed, 0.0f, 50.0f);
            ImGui::SliderFloat3("Position", &offset[0], -100.0f, 100.0f);
            ImGui::End();
        }
        /* Swap front and back buffers */
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
        engine.TriangleRenderer.ColorLocations.clear();
        engine.TriangleRenderer.Colors.clear();
        engine.TriangleRenderer.VertexArrays.clear();
        engine.TriangleRenderer.VertexBuffers.clear();

    }

    glfwTerminate();
    return 0;
}