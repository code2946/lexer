#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cctype>
#include <map>
#include <set>

using namespace std;

// Enum for Token Types
enum TokenType {
    KEYWORD,
    IDENTIFIER,
    OPERATOR,
    SYMBOL,
    INTEGER,
    FLOAT,
    STRING,
    CHARACTER,
    PREPROCESSOR,
    COMMENT,
    UNKNOWN,
    END_OF_FILE
};

// Structure to hold Token Info
struct Token {
    TokenType type;
    std::string value;
    int line;
};

class LexicalAnalyzer {
private:
    std::string source;
    size_t pos;
    int currentLine;

    std::set<std::string> keywords = {
        "int", "float", "char", "double", "if", "else", "while", "for",
        "return", "void", "switch", "case", "break", "continue", "class",
        "public", "private", "protected", "using", "namespace", "bool",
        "true", "false", "const", "static", "struct", "new", "delete"
    };

    // Preprocessor directives (come after a '#')
    std::set<std::string> preprocessorDirectives = {
        "include", "define", "ifdef", "ifndef", "endif", "pragma", "undef"
    };

    std::set<std::string> operators = {
        "+", "-", "*", "/", "%", "=", "==", "!=", "<", ">", "<=", ">=",
        "&&", "||", "!", "++", "--", "+=", "-=", "*=", "/=",
        "<<", ">>", "->", "&", "|", "^", "~", "<<=", ">>="
    };

    std::set<char> symbols = {
        '(', ')', '{', '}', '[', ']', ';', ',', '.', ':', '?'
    };

    void skipWhitespace() {
        while (pos < source.length() && isspace(static_cast<unsigned char>(source[pos]))) {
            if (source[pos] == '\n') currentLine++;
            pos++;
        }
    }

    bool isKeyword(const std::string& str) {
        return keywords.find(str) != keywords.end();
    }

    bool isOperator(const std::string& str) {
        return operators.find(str) != operators.end();
    }

public:
    LexicalAnalyzer(std::string src) : source(src), pos(0), currentLine(1) {}

    Token getNextToken() {
        skipWhitespace();

        if (pos >= source.length()) {
            return {END_OF_FILE, "", currentLine};
        }

        char currentChar = source[pos];

        // Handle Comments
        if (currentChar == '/' && pos + 1 < source.length()) {
            if (source[pos + 1] == '/') { // Single line comment
                int startLine = currentLine;
                pos += 2;
                std::string commentValue = "//";
                while (pos < source.length() && source[pos] != '\n') {
                    commentValue += source[pos++];
                }
                return {COMMENT, commentValue, startLine};
            } else if (source[pos + 1] == '*') { // Multi-line comment
                int startLine = currentLine;
                pos += 2;
                std::string commentValue = "/*";
                bool terminated = false;
                while (pos + 1 < source.length()) {
                    if (source[pos] == '*' && source[pos + 1] == '/') {
                        commentValue += "*/";
                        pos += 2;
                        terminated = true;
                        break;
                    }
                    if (source[pos] == '\n') currentLine++;
                    commentValue += source[pos++];
                }
                if (!terminated) {
                    // Unterminated comment: consume the rest and flag it
                    while (pos < source.length()) {
                        if (source[pos] == '\n') currentLine++;
                        commentValue += source[pos++];
                    }
                    return {UNKNOWN, "Unterminated comment: " + commentValue, startLine};
                }
                return {COMMENT, commentValue, startLine};
            }
        }

        // Handle Preprocessor Directives (#include, #define, etc.)
        if (currentChar == '#') {
            int startLine = currentLine;
            std::string directive = "#";
            pos++;
            // Skip spaces between # and the directive word
            while (pos < source.length() && (source[pos] == ' ' || source[pos] == '\t')) {
                pos++;
            }
            while (pos < source.length() && std::isalpha(static_cast<unsigned char>(source[pos]))) {
                directive += source[pos++];
            }
            return {PREPROCESSOR, directive, startLine};
        }

        // Handle Strings
        if (currentChar == '"') {
            int startLine = currentLine;
            std::string strVal = "\"";
            pos++;
            while (pos < source.length() && source[pos] != '"') {

                
                if (source[pos] == '\\' && pos + 1 < source.length()) {
                    // Preserve escape sequences like \" \n \t
                    strVal += source[pos++];
                    strVal += source[pos++];
                    continue;
                }
                if (source[pos] == '\n') currentLine++;
                strVal += source[pos++];
            }
            if (pos < source.length()) {
                strVal += "\"";
                pos++;
                return {STRING, strVal, startLine};
            }
            return {UNKNOWN, "Unterminated string: " + strVal, startLine};
        }

        // Handle Character Literals ('a', '\n', etc.)
        if (currentChar == '\'') {
            int startLine = currentLine;
            std::string charVal = "'";
            pos++;
            if (pos < source.length() && source[pos] == '\\' && pos + 1 < source.length()) {
                // Escape sequence
                charVal += source[pos++];
                charVal += source[pos++];
            } else if (pos < source.length() && source[pos] != '\'') {
                charVal += source[pos++];
            }
            if (pos < source.length() && source[pos] == '\'') {
                charVal += "'";
                pos++;
                return {CHARACTER, charVal, startLine};
            }
            return {UNKNOWN, "Unterminated char literal: " + charVal, startLine};
        }

        // Handle Identifiers and Keywords
        if (std::isalpha(static_cast<unsigned char>(currentChar)) || currentChar == '_') {
            std::string ident = "";
            while (pos < source.length() &&
                   (std::isalnum(static_cast<unsigned char>(source[pos])) || source[pos] == '_')) {
                ident += source[pos++];
            }
            if (isKeyword(ident)) {
                return {KEYWORD, ident, currentLine};
            }
            return {IDENTIFIER, ident, currentLine};
        }

        // Handle Numbers (Integers and Floats) — reject multiple dots
        if (std::isdigit(static_cast<unsigned char>(currentChar))) {
            std::string num = "";
            bool isFloat = false;
            bool malformed = false;
            while (pos < source.length() &&
                   (std::isdigit(static_cast<unsigned char>(source[pos])) || source[pos] == '.')) {
                if (source[pos] == '.') {
                    if (isFloat) {
                        // Second dot → malformed number
                        malformed = true;
                    }
                    isFloat = true;
                }
                num += source[pos++];
            }
            if (malformed) {
                return {UNKNOWN, "Malformed number: " + num, currentLine};
            }
            return {isFloat ? FLOAT : INTEGER, num, currentLine};
        }

        // Handle Operators — try 3-char, then 2-char, then 1-char (longest-match)
        if (pos + 2 < source.length()) {
            std::string tripleOp = source.substr(pos, 3);
            if (isOperator(tripleOp)) {
                pos += 3;
                return {OPERATOR, tripleOp, currentLine};
            }
        }
        std::string op(1, currentChar);
        if (pos + 1 < source.length()) {
            std::string doubleOp = op + source[pos + 1];
            if (isOperator(doubleOp)) {
                pos += 2;
                return {OPERATOR, doubleOp, currentLine};
            }
        }
        if (isOperator(op)) {
            pos++;
            return {OPERATOR, op, currentLine};
        }

        // Handle Symbols
        if (symbols.find(currentChar) != symbols.end()) {
            pos++;
            return {SYMBOL, std::string(1, currentChar), currentLine};
        }

        // Unknown character
        pos++;
        return {UNKNOWN, std::string(1, currentChar), currentLine};
    }

    std::string getTypeName(TokenType type) {
        switch (type) {
            case KEYWORD: return "KEYWORD";
            case IDENTIFIER: return "IDENTIFIER";
            case OPERATOR: return "OPERATOR";
            case SYMBOL: return "SYMBOL";
            case INTEGER: return "INTEGER";
            case FLOAT: return "FLOAT";
            case STRING: return "STRING";
            case CHARACTER: return "CHARACTER";
            case PREPROCESSOR: return "PREPROCESSOR";
            case COMMENT: return "COMMENT";
            case UNKNOWN: return "UNKNOWN";
            case END_OF_FILE: return "EOF";
            default: return "UNKNOWN";
        }
    }
};

int main(int argc, char* argv[]) {
    std::string filename = (argc > 1) ? argv[1] : "test_code.cpp";
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        // Create a default file if it doesn't exist for demonstration
        std::ofstream outfile(filename);
        outfile << "#include <iostream>\n"
                << "int main() {\n"
                << "    float pi = 3.14;\n"
                << "    int radius = 10;\n"
                << "    // Calculate area\n"
                << "    float area = pi * radius * radius;\n"
                << "    return 0;\n"
                << "}";
        outfile.close();
        file.open(filename);
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    LexicalAnalyzer lexer(content);
    Token token;

    std::cout << std::left << std::setw(15) << "TYPE"
              << std::setw(25) << "VALUE"
              << std::setw(10) << "LINE" << std::endl;
    std::cout << std::string(50, '-') << std::endl;

    do {
        token = lexer.getNextToken();
        if (token.type != END_OF_FILE) {
            std::cout << std::left << std::setw(15) << lexer.getTypeName(token.type)
                 << std::setw(25) << token.value
                 << std::setw(10) << token.line << std::endl;
        }
    } while (token.type != END_OF_FILE);

    return 0;
}
