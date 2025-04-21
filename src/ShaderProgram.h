#pragma once

#include <glm/fwd.hpp>
#include <unordered_map>

namespace imp {
class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();

    ShaderProgram(ShaderProgram const&) = delete;
    ShaderProgram& operator=(ShaderProgram const&) = delete;

    bool Create(char const* vertexSource, char const* fragmentSource);
    void Bind() const;

    void SetUniform(char const* name, int value);
    void SetUniform(char const* name, float value);
    void SetUniform(char const* name, glm::vec3 const& value);
    void SetUniform(char const* name, glm::vec4 const& value);
    void SetUniform(char const* name, glm::mat4 const& value);
    int32_t GetUniformLocation(char const* name);

    uint32_t GetId() const
    {
        return m_id;
    }

private:
    uint32_t m_id;
    std::unordered_map<char const*, int32_t> m_uniformLocations;
};
}
