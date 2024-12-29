#pragma once

#include <string>
#include <vector>

/* Used to define the type of tokens for our programming language */
enum class TokenType {
    exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    let,
    eq,
    plus,
    star,
    minus,
    fslash,
    open_curly,
    close_curly,
    if_,
    elif,
    else_,
    colon,
    for_
};

/* Used for converting a token to a string for an easy usage for further operations */
inline std::string to_string(const TokenType type)
{
    switch (type) {
    case TokenType::exit:
        return "`exit`";
    case TokenType::int_lit:
        return "int literal";
    case TokenType::semi:
        return "`;`";
    case TokenType::open_paren:
        return "`(`";
    case TokenType::close_paren:
        return "`)`";
    case TokenType::ident:
        return "identifier";
    case TokenType::let:
        return "`let`";
    case TokenType::eq:
        return "`=`";
    case TokenType::plus:
        return "`+`";
    case TokenType::star:
        return "`*`";
    case TokenType::minus:
        return "`-`";
    case TokenType::fslash:
        return "`/`";
    case TokenType::open_curly:
        return "`{`";
    case TokenType::close_curly:
        return "`}`";
    case TokenType::if_:
        return "`if`";
    case TokenType::elif:
        return "`elif`";
    case TokenType::else_:
        return "`else`";
    case TokenType::colon:
        return "`:`";
    case TokenType::for_:
        return "`for`";
    }
    assert(false);
}

/* Used for defining the precedence of the binary operators in order to know
** in which order the operations will execute
*/
inline std::optional<int> bin_prec(const TokenType type)
{
    switch (type) {
    /* The - and + will execute last in the order they appear */
    case TokenType::minus:
    case TokenType::plus:
        return 0;
    /* The / and * will execute first in the order they appear */
    case TokenType::fslash:
    case TokenType::star:
        return 1;
    default:
        return {};
    }
}

struct Token {
    TokenType type; // The type of the token
    int line; // the line in the file where the token is located
    std::optional<std::string> value {}; // the value of the token if it is an identifier or an integer literal
};

class Tokenizer {
public:
    explicit Tokenizer(std::string src) // the constructor of the class
        : m_src(std::move(src)) // move is used in order to avoid copying for nothing the string
    {
    }

    std::vector<Token> tokenize()
    {
        std::vector<Token> tokens; //here we retain the tokens
        std::string buf; // here we build the tokens temporarily
        int line_count = 1; // the line in the file where the token is located
        while (peek().has_value()) {
            if (std::isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buf.push_back(consume());
                }
                if (buf == "exit") {
                    tokens.push_back({ TokenType::exit, line_count });
                    buf.clear();
                }
                else if (buf == "let") {
                    tokens.push_back({ TokenType::let, line_count });
                    buf.clear();
                }
                else if (buf == "if") {
                    tokens.push_back({ TokenType::if_, line_count });
                    buf.clear();
                }
                else if (buf == "elif") {
                    tokens.push_back({ TokenType::elif, line_count });
                    buf.clear();
                }
                else if (buf == "else") {
                    tokens.push_back({ TokenType::else_, line_count });
                    buf.clear();
                }
                else if (buf == "for") {
                    tokens.push_back({ TokenType::for_, line_count });
                    buf.clear();
                }
                else {
                    tokens.push_back({ TokenType::ident, line_count, buf });
                    buf.clear();
                }
            }
            else if (std::isdigit(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                tokens.push_back({ TokenType::int_lit, line_count, buf });
                buf.clear();
            }
            else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/') {
                consume();
                consume();
                while (peek().has_value() && peek().value() != '\n') {
                    consume();
                }
            }
            else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*') {
                consume();
                consume();
                while (peek().has_value()) {
                    if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/') {
                        break;
                    }
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
            }
            else if (peek().value() == '(') {
                consume();
                tokens.push_back({ TokenType::open_paren, line_count });
            }
            else if (peek().value() == ')') {
                consume();
                tokens.push_back({ TokenType::close_paren, line_count });
            }
            else if (peek().value() == ';') {
                consume();
                tokens.push_back({ TokenType::semi, line_count });
            }
            else if (peek().value() == '=') {
                consume();
                tokens.push_back({ TokenType::eq, line_count });
            }
            else if (peek().value() == '+') {
                consume();
                tokens.push_back({ TokenType::plus, line_count });
            }
            else if (peek().value() == '*') {
                consume();
                tokens.push_back({ TokenType::star, line_count });
            }
            else if (peek().value() == '-') {
                consume();
                tokens.push_back({ TokenType::minus, line_count });
            }
            else if (peek().value() == '/') {
                consume();
                tokens.push_back({ TokenType::fslash, line_count });
            }
            else if (peek().value() == '{') {
                consume();
                tokens.push_back({ TokenType::open_curly, line_count });
            }
            else if (peek().value() == '}') {
                consume();
                tokens.push_back({ TokenType::close_curly, line_count });
            }
            else if (peek().value() == ':') {
                consume();
                tokens.push_back({ TokenType::colon, line_count });
            }
            else if (peek().value() == '\n') {
                consume();
                line_count++;
            }
            else if (std::isspace(peek().value())) {
                consume();
            }
            else {
                std::cerr << "Invalid token" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;
    }

private:
    [[nodiscard]] std::optional<char> peek(const size_t offset = 0) const
    {    // here we check if the index is greater than the length of the string
        if (m_index + offset >= m_src.length()) {
            return {};
        }
        // here we return the character at the index + offset
        return m_src.at(m_index + offset);
    }
    // here we consume the character at the index by returning it
    char consume()
    {
        return m_src.at(m_index++);
    }

    const std::string m_src;
    size_t m_index = 0;
};
