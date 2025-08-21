#include "MQOParser.h"

namespace imp {

// TODO(Waleed): Check if its valid integer we are parsing when we are using std::stoi

MQOParser::MQOParser(std::filesystem::path const& filePath)
{
    m_file.open(filePath, std::ios::in | std::ios::binary);
}

bool MQOParser::Parse(MQOFile& mqoFile)
{
    if (!NextLine() || m_currentLine != "Metasequoia Document") {
        return SetError("Not a valid Metasequoia file");
    }

    if (!NextLine() || m_currentLine.find("Format Text Ver") != 0) {
        return SetError("Unsupported MQO format version");
    }

    while (NextLine()) {
        if (m_currentLine.empty() || m_currentLine == "Eof") {
            continue;
        }

        if (m_currentLine.find("Scene ") == 0) {
            MQOScene scene;
            if (!ParseScene(scene)) {
                return false;
            }
            mqoFile.SetScene(scene);
            continue;
        }

        if (m_currentLine.find("Material ") != std::string::npos) {
            std::vector<MQOMaterial> materials;
            if (!ParseMaterials(materials)) {
                return false;
            }

            for (auto const& material : materials) {
                mqoFile.AddMaterial(material);
            }
            continue;
        }

        if (m_currentLine.find("Object ") == 0) {
            MQOObject object;
            if (!ParseObject(object)) {
                return false;
            }
            mqoFile.AddObject(object);
            continue;
        }

        if (m_currentLine.find('{') != std::string::npos) {
            m_position = m_currentLine.find('{') + 1;
            SkipToMatchingBrace();
        }
    }

    return true;
}

bool MQOParser::ParseScene(MQOScene& scene)
{
    if (m_currentLine.find('{') == std::string::npos) {
        if (!NextLine() || m_currentLine.find('{') == std::string::npos) {
            return SetError("Expected opening brace for Scene section");
        }
    }

    while (NextLine()) {
        if (m_currentLine.find('}') != std::string::npos) {
            return true;
        }

        m_position = 0;
        SkipWhitespace();

        if (m_currentLine.find("pos ") == 0) {
            m_position = 4;
            Token token;

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.posX = std::stof(token.value);

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.posY = std::stof(token.value);

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.posZ = std::stof(token.value);
        } else if (m_currentLine.find("lookat ") == 0) {
            m_position = 7;
            Token token;

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.lookatX = std::stof(token.value);

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.lookatY = std::stof(token.value);

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.lookatZ = std::stof(token.value);
        } else if (m_currentLine.find("head ") == 0) {
            m_position = 5;
            Token token;

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.head = std::stof(token.value);
        } else if (m_currentLine.find("pich ") == 0) {
            m_position = 5;
            Token token;

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.pitch = std::stof(token.value);
        } else if (m_currentLine.find("ortho ") == 0) {
            m_position = 6;
            Token token;

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.ortho = std::stoi(token.value);
        } else if (m_currentLine.find("zoom2 ") == 0) {
            m_position = 6;
            Token token;

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.zoom2 = std::stof(token.value);
        } else if (m_currentLine.find("amb ") == 0) {
            m_position = 4;
            Token token;

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.ambR = std::stof(token.value);

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.ambG = std::stof(token.value);

            if (!Expect(TokenType::NUMBER, token)) {
                continue;
            }
            scene.ambB = std::stof(token.value);
        }
    }

    return SetError("Unexpected end of file while parsing Scene section");
}

bool MQOParser::ParseMaterials(std::vector<MQOMaterial>& materials)
{
    Token token;
    if (!Expect(TokenType::IDENTIFIER, token) || token.value != "Material") {
        return SetError("Expected 'Material' keyword");
    }
    Token count;
    if (!Expect(TokenType::NUMBER, count)) {
        return SetError("Expected material count");
    }
    Token lbrace;
    if (!Expect(TokenType::SYMBOL, lbrace) || !lbrace.IsSymbol('{')) {
        return SetError("Expected opening brace for Material section");
    }
    int32_t materialCount = std::stoi(count.value);
    materials.reserve(materialCount);

    while (NextLine()) {
        if (m_currentLine.find('}') != std::string::npos) {
            return true;
        }

        m_position = 0;
        if (MQOMaterial material; ParseMaterial(material)) {
            materials.push_back(material);
        }
    }

    return SetError("Unexpected end of file while parsing Materials section");
}

bool MQOParser::ParseObject(MQOObject& object)
{
    size_t firstQuote = m_currentLine.find('"');
    size_t lastQuote = m_currentLine.find('"', firstQuote + 1);
    if (firstQuote != std::string::npos && lastQuote != std::string::npos) {
        object.name = m_currentLine.substr(firstQuote + 1, lastQuote - firstQuote - 1);
    }

    if (m_currentLine.find('{') == std::string::npos) {
        if (!NextLine() || m_currentLine.find('{') == std::string::npos) {
            return SetError("Expected opening brace for Object section");
        }
    }

    while (NextLine()) {
        if (m_currentLine == "}" || m_currentLine == "\t}") {
            return true;
        }

        m_position = 0;
        SkipWhitespace();

        if (m_currentLine.find("\tdepth ") == 0) {
            m_position = 7;
            if (Token token; Expect(TokenType::NUMBER, token)) {
                object.depth = std::stoi(token.value);
            }
        } else if (m_currentLine.find("\tfolding ") == 0) {
            m_position = 10;
            if (Token token; Expect(TokenType::NUMBER, token)) {
                object.folding = std::stoi(token.value);
            }
        } else if (m_currentLine.find("\tscale ") == 0) {
            m_position = 8;
            Token token;

            if (Expect(TokenType::NUMBER, token)) {
                object.scaleX = std::stof(token.value);
            }
            if (Expect(TokenType::NUMBER, token)) {
                object.scaleY = std::stof(token.value);
            }
            if (Expect(TokenType::NUMBER, token)) {
                object.scaleZ = std::stof(token.value);
            }
        } else if (m_currentLine.find("\trotation ") == 0) {
            m_position = 11;
            Token token;

            if (Expect(TokenType::NUMBER, token)) {
                object.rotationX = std::stof(token.value);
            }
            if (Expect(TokenType::NUMBER, token)) {
                object.rotationY = std::stof(token.value);
            }
            if (Expect(TokenType::NUMBER, token)) {
                object.rotationZ = std::stof(token.value);
            }
        } else if (m_currentLine.find("\ttranslation ") == 0) {
            m_position = 14;
            Token token;

            if (Expect(TokenType::NUMBER, token)) {
                object.translationX = std::stof(token.value);
            }
            if (Expect(TokenType::NUMBER, token)) {
                object.translationY = std::stof(token.value);
            }
            if (Expect(TokenType::NUMBER, token)) {
                object.translationZ = std::stof(token.value);
            }
        } else if (m_currentLine.find("\tvisible ") == 0) {
            m_position = 10;
            Token token;
            if (Expect(TokenType::NUMBER, token)) {
                object.visible = std::stoi(token.value);
            }
        } else if (m_currentLine.find("\tlocking ") == 0) {
            m_position = 10;
            Token token;
            if (Expect(TokenType::NUMBER, token)) {
                object.locking = std::stoi(token.value);
            }
        } else if (m_currentLine.find("\tshading ") == 0) {
            m_position = 10;
            Token token;
            if (Expect(TokenType::NUMBER, token)) {
                object.shading = std::stoi(token.value);
            }
        } else if (m_currentLine.find("\tfacet ") == 0) {
            m_position = 8;
            Token token;
            if (Expect(TokenType::NUMBER, token)) {
                object.facet = std::stof(token.value);
            }
        } else if (m_currentLine.find("\tcolor ") == 0) {
            m_position = 8;

            Token token;
            if (Expect(TokenType::NUMBER, token)) {
                object.colorR = std::stof(token.value);
            }
            if (Expect(TokenType::NUMBER, token)) {
                object.colorG = std::stof(token.value);
            }
            if (Expect(TokenType::NUMBER, token)) {
                object.colorB = std::stof(token.value);
            }
        } else if (m_currentLine.find("\tcolor_type ") == 0) {
            m_position = 13;
            if (Token token; Expect(TokenType::NUMBER, token)) {
                object.colorType = std::stoi(token.value);
            }
        } else if (m_currentLine.find("\tvertex ") == 0) {
            if (!ParseVertices(object.vertices)) {
                return false;
            }
        } else if (m_currentLine.find("\tface ") == 0) {
            if (!ParseFaces(object.faces)) {
                return false;
            }
        } else if (m_currentLine.find("\tvertexattr") != std::string::npos) {
            if (m_currentLine.find('{') != std::string::npos) {
                if (!ParseVertexAttr(object)) {
                    return false;
                }
            }
        } else if (m_currentLine.find('{') != std::string::npos) {
            m_position = m_currentLine.find('{') + 1;
            SkipToMatchingBrace();
        }
    }
    return SetError("Unexpected end of file while parsing Object section");
}

bool MQOParser::ParseVertices(std::vector<MQOVertex>& vertices)
{
    size_t countPos = m_currentLine.find(' ');
    if (countPos != std::string::npos) {
        size_t bracePos = m_currentLine.find('{', countPos);
        if (bracePos != std::string::npos) {
            std::string countStr = m_currentLine.substr(countPos, bracePos - countPos);
            int32_t vertexCount = std::stoi(countStr);
            vertices.reserve(vertexCount);
        }
    }

    while (NextLine()) {
        if (m_currentLine == "\t}" || m_currentLine.find("\t}") != std::string::npos) {
            return true;
        }

        m_position = 0;
        SkipWhitespace();
        if (MQOVertex vertex; ParseVertex(vertex)) {
            vertices.push_back(vertex);
        }
    }
    return SetError("Unexpected end of file while parsing Vertex section");
}

bool MQOParser::ParseFaces(std::vector<MQOFace>& faces)
{
    size_t countPos = m_currentLine.find(' ');
    if (countPos != std::string::npos) {
        size_t bracePos = m_currentLine.find('{', countPos);
        if (bracePos != std::string::npos) {
            std::string countStr = m_currentLine.substr(countPos, bracePos - countPos);
            int32_t faceCount = std::stoi(countStr);
            faces.reserve(faceCount);
        }
    }

    while (NextLine()) {
        if (m_currentLine == "\t}" || m_currentLine.find("\t}") != std::string::npos) {
            return true;
        }

        m_position = 0;
        SkipWhitespace();
        if (MQOFace face; ParseFace(face)) {
            faces.push_back(face);
        }
    }
    return SetError("Unexpected end of file while parsing Face section");
}

bool MQOParser::ParseVertexAttr(MQOObject& object)
{
    if (m_currentLine.find('{') != std::string::npos) {
        m_position = m_currentLine.find('{') + 1;
    }

    int32_t braceLevel = 1;
    bool inWeitSection = false;

    while (true) {
        if (m_position >= m_currentLine.length()) {
            if (!NextLine()) {
                return SetError("Unexpected end of file while parsing VertexAttr section");
            }
            continue;
        }

        SkipWhitespace();

        if (m_position < m_currentLine.length() && m_currentLine[m_position] == '}') {
            m_position++;
            braceLevel--;
            if (braceLevel == 0) {
                return true;
            }
            if (inWeitSection) {
                inWeitSection = false;
            }
            continue;
        }

        if (!inWeitSection) {
            if (m_position + 4 <= m_currentLine.length() && m_currentLine.substr(m_position, 4) == "weit") {
                m_position += 4;
                SkipWhitespace();
                if (m_position < m_currentLine.length() && m_currentLine[m_position] == '{') {
                    m_position++;
                    braceLevel++;
                    inWeitSection = true;
                }
            } else {
                if (Token token = NextToken(); token.type == TokenType::IDENTIFIER) {
                    token = NextToken();
                    if (token.IsSymbol('{')) {
                        braceLevel++;
                        int32_t sectionBraceLevel = 1;
                        while (sectionBraceLevel > 0) {
                            if (m_position >= m_currentLine.length()) {
                                if (!NextLine()) {
                                    return SetError("Unexpected end of file while parsing unknown section");
                                }
                                continue;
                            }
                            char c = m_currentLine[m_position++];
                            if (c == '{') {
                                sectionBraceLevel++;
                                braceLevel++;
                            } else if (c == '}') {
                                sectionBraceLevel--;
                                braceLevel--;
                            }
                        }
                    }
                } else {
                    m_position = m_currentLine.length();
                }
            }
        } else {
            Token indexToken = NextToken();
            if (indexToken.type == TokenType::NUMBER) {
                Token weightToken = NextToken();
                if (weightToken.type == TokenType::NUMBER) {
                    int32_t index = std::stoi(indexToken.value);
                    float weight = std::stof(weightToken.value);
                    if (index >= 0 && index < object.vertices.size()) {
                        object.vertices[index].weit = weight;
                    }
                }
            } else if (indexToken.IsSymbol('}')) {
                m_position--;
                inWeitSection = false;
            } else {
                m_position = m_currentLine.length();
            }
        }
    }
}

bool MQOParser::ParseVertex(MQOVertex& vertex)
{
    Token token;
    if (!Expect(TokenType::NUMBER, token)) {
        return false;
    }
    vertex.x = std::stof(token.value);

    if (!Expect(TokenType::NUMBER, token)) {
        return false;
    }
    vertex.y = std::stof(token.value);

    if (!Expect(TokenType::NUMBER, token)) {
        return false;
    }
    vertex.z = std::stof(token.value);
    return true;
}

bool MQOParser::ParseFace(MQOFace& face)
{
    Token token;
    if (!Expect(TokenType::NUMBER, token)) {
        return false;
    }

    int32_t vertexCount = std::stoi(token.value);
    if (vertexCount != 3) {
        SetError("Only triangles are supported. Found " + std::to_string(vertexCount) + "-sided polygon.");
        return false;
    }

    SkipWhitespace();
    if (!Expect(TokenType::IDENTIFIER, token) || token.value != "V") {
        return false;
    }

    SkipWhitespace();
    token = NextToken();
    if (!token.IsSymbol('(')) {
        return false;
    }

    std::vector<int32_t> vertices;
    for (int32_t i = 0; i < vertexCount; i++) {
        SkipWhitespace();
        if (!Expect(TokenType::NUMBER, token)) {
            return false;
        }
        vertices.push_back(std::stoi(token.value));
        SkipWhitespace();
    }

    face.v1 = vertices[2];
    face.v2 = vertices[1];
    face.v3 = vertices[0];

    SkipWhitespace();
    token = NextToken();
    if (!token.IsSymbol(')')) {
        return false;
    }

    SkipWhitespace();
    if (!Expect(TokenType::IDENTIFIER, token) || token.value != "M") {
        return false;
    }

    SkipWhitespace();
    token = NextToken();
    if (!token.IsSymbol('(')) {
        return false;
    }

    SkipWhitespace();
    if (!Expect(TokenType::NUMBER, token)) {
        return false;
    }
    face.materialIndex = std::stoi(token.value);

    SkipWhitespace();

    token = NextToken();
    if (!token.IsSymbol(')')) {
        return false; // NOLINT(*-simplify-boolean-expr)
    }
    return true;
}

bool MQOParser::ParseMaterial(MQOMaterial& material)
{
    Token token;
    if (!Expect(TokenType::STRING, token)) {
        return false;
    }
    material.name = token.value;

    while (m_position < m_currentLine.length()) {
        SkipWhitespace();
        if (m_position >= m_currentLine.length()) {
            break;
        }
        if (!Expect(TokenType::IDENTIFIER, token)) {
            bool foundIdentifier = false;
            while (m_position < m_currentLine.length()) {
                char c = m_currentLine[m_position];
                if (std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_') {
                    foundIdentifier = true;
                    break;
                }
                m_position++;
            }

            if (!foundIdentifier) {
                break;
            }
            if (!Expect(TokenType::IDENTIFIER, token)) {
                break;
            }
        }

        std::string property = token.value;
        SkipWhitespace();
        token = NextToken();
        if (!token.IsSymbol('(')) {
            continue;
        }
        if (property == "shader") {
            SkipWhitespace();
            if (!Expect(TokenType::NUMBER, token)) {
                SkipToClosingParenthesis();
                continue;
            }
            material.shader = std::stoi(token.value);
        } else if (property == "col") {
            SkipWhitespace();
            if (!Expect(TokenType::NUMBER, token)) {
                SkipToClosingParenthesis();
                continue;
            }
            material.r = std::stof(token.value);

            SkipWhitespace();
            if (!Expect(TokenType::NUMBER, token)) {
                SkipToClosingParenthesis();
                continue;
            }
            material.g = std::stof(token.value);

            SkipWhitespace();
            if (!Expect(TokenType::NUMBER, token)) {
                SkipToClosingParenthesis();
                continue;
            }
            material.b = std::stof(token.value);

            SkipWhitespace();
            if (!Expect(TokenType::NUMBER, token)) {
                SkipToClosingParenthesis();
                continue;
            }
            material.alpha = std::stof(token.value);
        } else if (property == "dif") {
            SkipWhitespace();
            if (!Expect(TokenType::NUMBER, token)) {
                SkipToClosingParenthesis();
                continue;
            }
            material.diffuse = std::stof(token.value);
        } else if (property == "amb") {
            SkipWhitespace();
            if (!Expect(TokenType::NUMBER, token)) {
                SkipToClosingParenthesis();
                continue;
            }
            material.ambient = std::stof(token.value);
        } else if (property == "emi") {
            SkipWhitespace();
            if (!Expect(TokenType::NUMBER, token)) {
                SkipToClosingParenthesis();
                continue;
            }
            material.emission = std::stof(token.value);
        } else if (property == "spc") {
            SkipWhitespace();
            if (!Expect(TokenType::NUMBER, token)) {
                SkipToClosingParenthesis();
                continue;
            }
            material.specular = std::stof(token.value);
        } else if (property == "power") {
            SkipWhitespace();
            if (!Expect(TokenType::NUMBER, token)) {
                SkipToClosingParenthesis();
                continue;
            }
            material.power = std::stof(token.value);
        } else {
            SkipToClosingParenthesis();
        }
    }

    return true;
}

void MQOParser::SkipWhitespace()
{
    while (m_position < m_currentLine.length() && std::isspace(static_cast<unsigned char>(m_currentLine[m_position])) != 0) {
        m_position++;
    }
}

bool MQOParser::NextLine()
{
    if (std::getline(m_file, m_currentLine)) {
        m_position = 0;
        while (!m_currentLine.empty() && (m_currentLine.back() == '\r' || m_currentLine.back() == '\n')) {
            m_currentLine.pop_back();
        }
        SkipWhitespace();
        return true;
    }
    return false;
}

Token MQOParser::NextToken()
{
    if (m_position >= m_currentLine.length()) {
        return { TokenType::ENDOFFILE, "" };
    }

    SkipWhitespace();

    if (m_position >= m_currentLine.length()) {
        return { TokenType::ENDOFFILE, "" };
    }

    char c = m_currentLine[m_position];
    if (c == '"') {
        m_position++;
        std::string value;
        while (m_position < m_currentLine.length() && m_currentLine[m_position] != '"') {
            value += m_currentLine[m_position++];
        }
        if (m_position < m_currentLine.length() && m_currentLine[m_position] == '"') {
            m_position++;
        }
        return { TokenType::STRING, value };
    }

    if (c == '{' || c == '}' || c == '(' || c == ')') {
        m_position++;
        return { TokenType::SYMBOL, std::string(1, c) };
    }

    if (std::isdigit(static_cast<unsigned char>(c)) != 0 || c == '-' || c == '.') {
        std::string value;
        if (c == '-') {
            value += c;
            m_position++;
            if (m_position >= m_currentLine.length() || std::isdigit(static_cast<unsigned char>(m_currentLine[m_position])) != 0 || m_currentLine[m_position] == '.') {
                return { TokenType::SYMBOL, "-" };
            }
        }
        while (m_position < m_currentLine.length() && std::isdigit(static_cast<unsigned char>(m_currentLine[m_position])) != 0) {
            value += m_currentLine[m_position++];
        }
        if (m_position < m_currentLine.length() && m_currentLine[m_position] == '.') {
            value += m_currentLine[m_position++];
            while (m_position < m_currentLine.length() && std::isdigit(static_cast<unsigned char>(m_currentLine[m_position])) != 0) {
                value += m_currentLine[m_position++];
            }
        }

        if (m_position < m_currentLine.length() && (m_currentLine[m_position] == 'e' || m_currentLine[m_position] == 'E')) {
            value += m_currentLine[m_position++];
            if (m_position < m_currentLine.length() && (m_currentLine[m_position] == '+' || m_currentLine[m_position] == '-')) {
                value += m_currentLine[m_position++];
            }
            while (m_position < m_currentLine.length() && std::isdigit(static_cast<unsigned char>(m_currentLine[m_position])) != 0) {
                value += m_currentLine[m_position++];
            }
        }

        return { TokenType::NUMBER, value };
    }

    if (std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_') {
        std::string value;
        while (m_position < m_currentLine.length() && (std::isalnum(static_cast<unsigned char>(m_currentLine[m_position])) != 0 || m_currentLine[m_position] == '_')) {
            value += m_currentLine[m_position++];
        }
        return { TokenType::IDENTIFIER, value };
    }
    m_position++;
    return NextToken();
}

bool MQOParser::Expect(TokenType type, Token& token)
{
    token = NextToken();
    return token.type == type;
}

void MQOParser::SkipToClosingBrace()
{
    SkipToMatchingBrace();
}

void MQOParser::SkipToClosingParenthesis()
{
    int32_t parenLevel = 1;
    while (m_position < m_currentLine.length() && parenLevel > 0) {
        char c = m_currentLine[m_position++];
        if (c == '(') {
            parenLevel++;
        } else if (c == ')') {
            parenLevel--;
        }
    }
}

bool MQOParser::SkipToMatchingBrace()
{
    int32_t braceLevel = 1;
    while (braceLevel > 0) {
        if (m_position >= m_currentLine.length()) {
            if (!NextLine()) {
                return false;
            }
            continue;
        }

        char c = m_currentLine[m_position++];
        if (c == '{') {
            braceLevel++;
        } else if (c == '}') {
            braceLevel--;
        }
    }
    return true;
}

bool MQOParser::SetError(std::string const& message)
{
    m_errorMessage = message;
    return false;
}
}
