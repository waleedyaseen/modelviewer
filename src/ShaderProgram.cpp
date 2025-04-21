#include "ShaderProgram.h"

#include "Dialogs.h"

#include <glad/glad.h>
#include <glm/mat4x4.hpp>

namespace imp {
ShaderProgram::ShaderProgram()
    : m_id(0)
{
    // Do nothing.
}

ShaderProgram::~ShaderProgram()
{
    if (m_id != 0) {
        glDeleteProgram(m_id);
        m_id = 0;
    }
}

bool ShaderProgram::Create(char const* vertexSource, char const* fragmentSource)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        glDeleteShader(vertexShader);
        ShowFatalDialog("Error", "Vertex shader compilation failed");
        return false;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        ShowFatalDialog("Error", "Fragment shader compilation failed");
        return false;
    }

    m_id = glCreateProgram();
    glAttachShader(m_id, vertexShader);
    glAttachShader(m_id, fragmentShader);
    glLinkProgram(m_id);

    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        ShowFatalDialog("Error", "Shader program linking failed");
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return true;
}

void ShaderProgram::Bind() const
{
    glUseProgram(m_id);
}

void ShaderProgram::SetUniform(char const* name, int value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void ShaderProgram::SetUniform(char const* name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void ShaderProgram::SetUniform(char const* name, glm::vec3 const& value)
{
    glUniform3fv(GetUniformLocation(name), 1, &value[0]);
}

void ShaderProgram::SetUniform(char const* name, glm::vec4 const& value)
{
    glUniform4fv(GetUniformLocation(name), 1, &value[0]);
}

void ShaderProgram::SetUniform(char const* name, glm::mat4 const& value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

GLint ShaderProgram::GetUniformLocation(char const* name)
{
    if (auto const it = m_uniformLocations.find(name); it != m_uniformLocations.end()) {
        return it->second;
    }
    GLint const location = glGetUniformLocation(m_id, name);
    m_uniformLocations[name] = location;
    return location;
}
}
