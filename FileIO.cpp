#include "FileIO.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include "Config.h"

bool readFile(const std::string& path, std::string& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::ostringstream buf;
    buf << in.rdbuf();
    out = buf.str();
    return true;
}

bool writeFile(const std::string& path, const std::string& content) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f << content;
    return static_cast<bool>(f);
}

std::string removeSpaces(const std::string& source) {
    std::string out;
    out.reserve(source.size());
    bool inString = false;
    for (char c : source) {
        if (c == '\n') {
            inString = false;  // strings cannot span lines
            if (config::KEEP_NEWLINES_IN_NOSPACES) out += '\n';
            continue;
        }
        if (c == '\r') continue;  // Windows line endings
        if (c == '"') inString = !inString;

        const bool blank = (c == ' ' || c == '\t' || c == '\f' || c == '\v');
        if (blank && (!inString || config::STRIP_SPACES_IN_STRINGS)) continue;
        out += c;
    }
    return out;
}

std::string buildReservedSymbolReport(const std::vector<Token>& tokens) {
    std::ostringstream os;
    os << std::left << std::setw(6) << "LINE" << std::setw(15) << "CATEGORY" << "LEXEME\n";
    os << std::setw(6) << "----" << std::setw(15) << "-------------" << "------\n";

    auto row = [&](int line, const char* category, const std::string& lexeme) {
        os << std::left << std::setw(6) << line << std::setw(15) << category << lexeme << '\n';
    };

    for (const Token& t : tokens) {
        if (isReservedWord(t.type)) {
            row(t.line, "RESERVED WORD", t.lexeme);
        } else if (isSymbol(t.type)) {
            row(t.line, "SYMBOL", t.lexeme);
        } else if (t.type == TokenType::StringLiteral) {
            // the two quote marks around a string are symbols too
            row(t.line, "SYMBOL", "\"");
            row(t.line, "SYMBOL", "\"");
        }
    }
    return os.str();
}
