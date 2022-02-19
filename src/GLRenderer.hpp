#pragma once

class GLFWwindow;

class GLRenderer final
{
public:
    GLRenderer();
    ~GLRenderer();

    bool render();

private:
    void createWindow();

    GLFWwindow* m_window;
};
