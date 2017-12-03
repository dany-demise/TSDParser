#include "stdafx.h"

namespace nope::dts::parser
{
	/// <summary>
	/// Initializes a new instance of the <see cref="Tokenizer"/> class.
	/// </summary>
	/// <param name="filename">The filename.</param>
	Tokenizer::Tokenizer(std::string_view filename) :
		m_filename(filename),
		m_input((std::istreambuf_iterator<char>(std::ifstream(m_filename))),
			std::istreambuf_iterator<char>()),
		m_token(),
		m_cursor(0)
	{
		for (std::size_t cursor = 0; cursor < m_input.size();)
		{
			Token token;
			char cur = m_input[cursor];
			char peek = m_input[cursor + 1];

			if (std::isspace(cur))
			{
				token = this->parseSpace(cursor);
			}
			else if (cur == '/' && peek == '/')
			{
				token = this->parseLineComment(cursor);
			}
			else if (cur == '/' && peek == '*')
			{
				token = this->parseBlockComment(cursor);
			}
			else if (std::isalpha(cur) || cur == '_' || cur == '$')
			{
				token = this->parseId(cursor);
				this->filterKeyword(token);
			}
			else if (cur == '"' || cur == '\'')
			{
				token = this->parseString(cursor);
			}
			else if (std::isdigit(cur))
			{
				token = this->parseNumber(cursor);
			}
			else
			{
				token = this->parsePunctuation(cursor);
			}

			m_token.push_back(token);
		}
		m_token.emplace_back(TokenType::END_OF_FILE);
	}

	/// <summary>
	/// Finalizes an instance of the <see cref="Tokenizer"/> class.
	/// </summary>
	/// <returns></returns>
	Tokenizer::~Tokenizer() noexcept
	{
	}

	/// <summary>
	/// Peeks a token at the specified lookahead.
	/// </summary>
	/// <param name="lookAhead">Lookahead.</param>
	/// <returns>The token found at this lookahead.</returns>
	Token Tokenizer::peek(std::uint32_t lookAhead, bool ignoreNewline)
	{
		if (this->eof())
		{
			return m_token.back();
		}

		std::size_t cursor = m_cursor;
		TokenType type = m_token[cursor].type;

		while (type != TokenType::END_OF_FILE)
		{
			if (type == TokenType::LINE_COMMENT || type == TokenType::BLOCK_COMMENT ||
				(type == TokenType::P_NEWLINE && ignoreNewline) || --lookAhead != 0)
			{
				++cursor;
			}
			else
			{
				break;
			}
		}

		return m_token[cursor];
	}

	/// <summary>
	/// Get the next token
	/// </summary>
	/// <returns>Next token in the input</returns>
	Token Tokenizer::next(bool ignoreNewline)
	{
		if (this->eof())
		{
			return m_token.back();
		}

		TokenType type = m_token[m_cursor].type;

		while (type != TokenType::END_OF_FILE)
		{
			if (type == TokenType::LINE_COMMENT || type == TokenType::BLOCK_COMMENT ||
				(type == TokenType::P_NEWLINE && ignoreNewline))
			{
				++m_cursor;
			}
			else
			{
				break;
			}
		}

		return m_token[m_cursor++];
	}

	bool Tokenizer::nextIf(Token & token, TokenType type, std::uint32_t lookAhead, bool ignoreNewline)
	{
		if (this->peek(lookAhead, ignoreNewline).type == type)
		{
			token << this->next(ignoreNewline);
			return true;
		}
		return false;
	}

	bool Tokenizer::nextIf(Token & token, std::function<bool()> func)
	{
		bool res = func();

		if (res)
		{
			token << this->next();
		}

		return res;
	}

	/// <summary>
	/// Check if it reached the end of the input.
	/// </summary>
	/// <returns></returns>
	bool Tokenizer::eof() const
	{
		return m_cursor <= m_token.size();
	}

	/// <summary>
	/// Throw an error with the specified message.
	/// </summary>
	/// <param name="message">The message.</param>
	void Tokenizer::error(std::string_view message, std::size_t line, std::size_t col) const
	{
		std::string location = m_filename + ':' + std::to_string(line)
			+ ':' + std::to_string(col);

		throw error::Syntax(location + " Error: " + std::string(message));
	}

	void Tokenizer::error(std::string_view message) const
	{
		auto[line, col] = this->getCursorPosition(m_cursor);

		this->error(message, line, col);
	}

	bool Tokenizer::_eof(std::size_t cursor) const
	{
		return cursor >= m_input.size();
	}

	/// <summary>
	/// Get the remaining input character number.
	/// </summary>
	std::size_t Tokenizer::remain(std::size_t cursor) const
	{
		if (m_input.size() < cursor)
			return 0;
		return m_input.size() - cursor;
	}

	Token Tokenizer::parseSpace(std::size_t &cursor)
	{
		std::size_t begin = cursor;

		if (m_input[cursor] == '\n')
		{
			++cursor;
			return Token(TokenType::P_NEWLINE, std::string_view(&m_input[begin], 1));
		}

		while (!this->_eof(cursor) &&
			std::isspace(m_input[cursor]) && m_input[cursor] != '\n')
		{
			++cursor;
		}

		return Token(TokenType::BLANK, std::string_view(&m_input[begin], cursor - begin));
	}

	Token Tokenizer::parseLineComment(std::size_t &cursor)
	{
		std::size_t begin = cursor;

		while (!this->_eof(cursor) && m_input[cursor] != '\n')
			++cursor;

		return Token(TokenType::LINE_COMMENT, std::string_view(&m_input[begin], cursor - begin));
	}

	Token Tokenizer::parseBlockComment(std::size_t &cursor)
	{
		std::size_t begin = cursor;

		while (cursor - begin < 4 || m_input[cursor - 1] != '/' || m_input[cursor - 2] != '*')
		{
			if (this->_eof(cursor))
			{
				auto [line, col] = this->getCursorPosition(m_token.size());

				this->error("Dit you forgot to close the block comment ?", line, col);
			}
			++cursor;
		}

		return Token(TokenType::BLOCK_COMMENT, std::string_view(&m_input[begin], cursor - begin));
	}

	Token Tokenizer::parseId(std::size_t &cursor)
	{
		std::size_t begin = cursor;

		while (!this->_eof(cursor) &&
				(std::isalnum(m_input[cursor]) ||
				m_input[cursor] == '_' ||
				m_input[cursor] == '$'))
		{
			++cursor;
		}

		return Token(TokenType::ID, std::string_view(&m_input[begin], cursor - begin));
	}

	void Tokenizer::filterKeyword(Token & token) const
	{
		std::pair<const char *, TokenType> val[] = {
			{ "class", TokenType::KW_CLASS },
			{ "interface", TokenType::KW_INTERFACE },
			{ "constructor", TokenType::KW_CONSTRUCTOR },
			{ "const", TokenType::KW_CONST },
			{ "enum", TokenType::KW_ENUM },
			{ "export", TokenType::KW_EXPORT },
			{ "extends", TokenType::KW_EXTENDS },
			{ "function", TokenType::KW_FUNCTION },
			{ "import", TokenType::KW_IMPORT },
			{ "in", TokenType::KW_IN },
			{ "typeof", TokenType::KW_TYPEOF },
			{ "type", TokenType::KW_TYPE },
			{ "keyof", TokenType::KW_KEYOF },
			{ "var", TokenType::KW_VAR },
			{ "let", TokenType::KW_VAR },
			{ "implements", TokenType::KW_IMPLEMENTS },
			{ "private", TokenType::KW_VISIBILITY },
			{ "protected", TokenType::KW_VISIBILITY },
			{ "public", TokenType::KW_VISIBILITY },
			{ "static", TokenType::KW_STATIC },
			{ "readonly", TokenType::KW_READONLY },
			{ "as", TokenType::KW_AS },
			{ "is", TokenType::KW_IS },
			{ "from", TokenType::KW_FROM },
			{ "declare", TokenType::KW_DECLARE },
			{ "module", TokenType::KW_MODULE },
			{ "namespace", TokenType::KW_MODULE },
			{ "require", TokenType::KW_REQUIRE }
		};

		for (auto const &v : val)
		{
			if (token.value == v.first)
			{
				token.type = v.second;
				return ;
			}
		}
	}

	Token Tokenizer::parseString(std::size_t &cursor)
	{
		char quote = m_input[cursor];
		std::size_t begin = cursor++;

		bool esc = false;

		while (!this->_eof(cursor))
		{
			if (!esc && m_input[cursor] == quote)
			{
				cursor++;
				break;
			}
			if (!esc && m_input[cursor] == '\n')
			{
				this->error("Unexpected newline");
			}
			if (!esc && m_input[cursor] == '\\')
			{
				esc = true;
			}
			else
			{
				esc = false;
			}
			++cursor;
		}

		return Token(TokenType::STRING_LITERAL, std::string_view(&m_input[begin], cursor - begin));
	}

	Token Tokenizer::parseNumber(std::size_t &cursor)
	{
		std::size_t begin = cursor;

		while (!this->_eof(cursor) && std::isdigit(m_input[cursor]))
		{
			++cursor;
		}

		return Token(TokenType::NUMBER, std::string_view(&m_input[begin], cursor - begin));
	}

	Token Tokenizer::parsePunctuation(std::size_t &cursor)
	{
		std::size_t begin = cursor;

		std::pair<const char *, TokenType> val[] = {
			{ "=>", TokenType::P_ARROW },				// Keep the arrow before the equal token
			{ "...", TokenType::P_SPREAD },
			{ ":", TokenType::P_COLON },
			{ ";", TokenType::P_SEMICOLON },
			{ "\n", TokenType::P_NEWLINE },
			{ ".", TokenType::P_DOT },
			{ ",", TokenType::P_COMMA },
			{ "/", TokenType::P_SLASH },
			{ "\\", TokenType::P_ANTISLASH },
			{ "?", TokenType::P_QUESTION },
			{ "*", TokenType::P_STAR },
			{ "(", TokenType::P_OPEN_PAR },
			{ ")", TokenType::P_CLOSE_PAR },
			{ "{", TokenType::P_OPEN_BRACE },
			{ "}", TokenType::P_CLOSE_BRACE },
			{ "[", TokenType::P_OPEN_BRACKET },
			{ "]", TokenType::P_CLOSE_BRACKET },
			{ "=", TokenType::P_EQUAL },
			{ "|", TokenType::P_VERTICAL_BAR },
			{ "<", TokenType::P_GREATER_THAN },
			{ ">", TokenType::P_LESS_THAN },
			{ "&", TokenType::P_AMPERSAND }
		};

		for (auto const &v : val)
		{
			if (std::size_t len = strlen(v.first); m_input.substr(begin, len) == v.first)
			{
				cursor += len;
				return Token(v.second, std::string_view(&m_input[begin], len));
			}
		}
		return Token(TokenType::UNKNOWN);
	}

	std::pair<std::size_t, std::size_t> Tokenizer::getCursorPosition(std::size_t index) const
	{
		std::size_t line = 1;
		std::size_t col = 1;

		for (std::size_t i = 0; i < index; ++i)
		{
			if (m_token[i].type == TokenType::P_NEWLINE)
			{
				++line;
				col = 1;
			}
			else if (m_token[i].type == TokenType::BLOCK_COMMENT)
			{
				for (std::size_t j = 0; j < m_token[i].value.size(); ++j)
				{
					if (m_token[i].value[j] == '\n')
					{
						++line;
						col = 1;
					}
					else
					{
						++col;
					}
				}
			}
			else
			{
				++col;
			}
		}

		return std::make_pair(line, col);
	}
}