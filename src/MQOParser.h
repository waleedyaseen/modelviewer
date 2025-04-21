#pragma once

#include "MQOFile.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace imp {
enum class TokenType : uint8_t {
    IDENTIFIER,
    NUMBER,
    STRING,
    SYMBOL,
    ENDOFFILE
};

struct Token {
    TokenType type;
    std::string value;

    bool IsSymbol(char symbol) const
    {
        return type == TokenType::SYMBOL && value.length() == 1 && value[0] == symbol;
    }
};
class MQOParser {
public:
    explicit MQOParser(std::filesystem::path const& filePath);

    bool Parse(MQOFile& mqoFile);

    bool Good() const
    {
        return m_file.good();
    }

    std::string GetErrorMessage() const
    {
        return m_errorMessage;
    }

private:
    bool ParseScene(MQOScene& scene);
    bool ParseMaterials(std::vector<MQOMaterial>& materials);
    bool ParseObject(MQOObject& object);
    bool ParseVertices(std::vector<MQOVertex>& vertices);
    bool ParseFaces(std::vector<MQOFace>& faces);
    bool ParseVertexAttr(MQOObject& object);
    bool ParseMaterial(MQOMaterial& material);
    bool ParseVertex(MQOVertex& vertex);
    bool ParseFace(MQOFace& face);

    void SkipWhitespace();
    bool NextLine();
    Token NextToken();
    bool Expect(TokenType type, Token& token);
    void SkipToClosingBrace();
    void SkipToClosingParenthesis();
    bool SkipToMatchingBrace();

    bool SetError(std::string const& message);

    std::ifstream m_file;
    std::string m_currentLine;
    size_t m_position = 0;
    std::string m_errorMessage;
};
}
