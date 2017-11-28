#include "stdafx.h"

namespace nope::dts::parser
{
	/// <summary>
	/// Initializes a new instance of the <see cref="Tokenizer"/> class.
	/// </summary>
	/// <param name="filename">The filename.</param>
	Tokenizer::Tokenizer(std::string_view filename) :
		m_filename(filename),
		m_realLine(1),
		m_realCol(1),
		m_realCursor(0),
		m_realLastSkipNewlineCount(0),
		m_line(&m_realLine),
		m_col(&m_realCol),
		m_cursor(&m_realCursor),
		m_lastSkipNewlineCount(&m_realLastSkipNewlineCount),
		m_peekLine(1),
		m_peekCol(1),
		m_peekCursor(0),
		m_peekLastSkipNewlineCount(0),
		m_input((std::istreambuf_iterator<char>(std::ifstream(m_filename))),
			std::istreambuf_iterator<char>())
	{
		this->skip();
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
	Token Tokenizer::peek(std::uint32_t lookAhead, bool skipNewline)
	{
		this->peekMode(true);

		for (std::uint32_t i = 0; i < lookAhead; ++i)
		{
			this->next(skipNewline);
		}

		Token token = this->next(skipNewline);

		this->peekMode(false);

		return token;
	}

	/// <summary>
	/// Get the next token
	/// </summary>
	/// <returns>Next token in the input</returns>
	Token Tokenizer::next(bool skipNewline)
	{
		Token token;

		if (skipNewline == false && *m_lastSkipNewlineCount > 0)
		{
			(*m_lastSkipNewlineCount)--;
			token.type = TokenType::P_NEWLINE;
			token.value = "\n";

			return token;
		}

		if (this->eof())
		{
			token.type = TokenType::END_OF_FILE;
			return token;
		}
		
		char cur = m_input[*m_cursor];
		
		if (std::isalpha(cur) || cur == '_')
		{
			token = this->parseId();
			this->filterKeyword(token);
		}
		else if (cur == '"')
		{
			token = this->parseString();
		}
		else if (std::isdigit(cur))
		{
			token = this->parseNumber();
		}
		else
		{
			token = this->parsePunctuation();
		}

		this->skip();

		return token;
	}

	/// <summary>
	/// Check if it reached the end of the input.
	/// </summary>
	/// <returns></returns>
	bool Tokenizer::eof() const
	{
		return *m_cursor >= m_input.size();
	}

	/// <summary>
	/// Throw an error with the specified message.
	/// </summary>
	/// <param name="message">The message.</param>
	void Tokenizer::error(std::string const & message) const
	{
		std::string location = m_filename + ':' + std::to_string(*m_line)
			+ ':' + std::to_string(*m_col);

		throw error::Syntax(location + " Error: " + message);
	}

	/// <summary>
	/// Get the remaining input character number.
	/// </summary>
	std::size_t Tokenizer::remain() const
	{
		if (m_input.size() > *m_cursor)
			return 0;
		return m_input.size() - *m_cursor;
	}

	/// <summary>
	/// Skips useless input (space or comment).
	/// </summary>
	void Tokenizer::skip()
	{
		std::size_t lastCursorPos = *m_cursor + 1;
		
		*m_lastSkipNewlineCount = 0;

		while (*m_cursor != lastCursorPos)
		{
			lastCursorPos = *m_cursor;
			this->skipBlank();
			this->skipLineComment();
			this->skipBlockComment();
		}
	}

	/// <summary>
	/// Skips a line comment.
	/// </summary>
	void Tokenizer::skipLineComment()
	{
		if (this->remain() > 2 && m_input[*m_cursor] == '/' &&
			m_input[*m_cursor + 1] == '/')
		{
			while (this->eof() == false && m_input[*m_cursor] != '\n')
			{
				(*m_cursor)++;
				(*m_col)++;
			}

			if (this->eof() == false)
			{
				(*m_line)++;
				*m_col = 1;
				(*m_lastSkipNewlineCount)++;
			}
		}
	}

	/// <summary>
	/// Skips a block comment.
	/// </summary>
	void Tokenizer::skipBlockComment()
	{
		std::uint32_t beginLine = *m_line;
		std::uint32_t beginCol = *m_col;

		if (this->remain() > 2 && m_input[*m_cursor] == '/' &&
			m_input[*m_cursor + 1] == '*')
		{
			*m_cursor += 3;

			while (m_input[*m_cursor - 1] != '*' || m_input[*m_cursor] != '/')
			{
				if (this->eof())
				{
					this->error("Unexpected end of file. Did you forget to " \
						"close your multiline comment starting at " \
						"line " + std::to_string(beginLine) +
						" column " + std::to_string(beginCol) + " ?");
				}

				if (m_input[*m_cursor] == '\n')
				{
					(*m_line)++;
					*m_col = 1;
				}
				else
				{
					(*m_col)++;
				}
				(*m_cursor)++;
			}
			(*m_cursor)++;
		}
	}

	/// <summary>
	/// Skips blank characters.
	/// </summary>
	void Tokenizer::skipBlank()
	{
		while (this->eof() == false && std::isspace(m_input[*m_cursor]))
		{
			if (m_input[*m_cursor] == '\n')
			{
				(*m_lastSkipNewlineCount)++;
				(*m_line)++;
				*m_col = 1;
			}
			(*m_cursor)++;
		}
	}

	Token Tokenizer::parseId()
	{
		std::size_t begin = *m_cursor;

		while (this->eof() == false &&
			(std::isalnum(m_input[*m_cursor]) || m_input[*m_cursor] == '_'))
		{
			++(*m_cursor);
			++(*m_col);
		}

		return Token(TokenType::ID, std::string_view(&m_input[begin], *m_cursor - begin));
	}

	void Tokenizer::filterKeyword(Token & token) const
	{
		std::pair<const char *, TokenType> val[] = {
			{ "class", TokenType::KW_CLASS },
			{ "interface", TokenType::KW_INTERFACE },
			{ "const", TokenType::KW_CONST },
			{ "enum", TokenType::KW_ENUM },
			{ "export", TokenType::KW_EXPORT },
			{ "extends", TokenType::KW_EXTENDS },
			{ "function", TokenType::KW_FUNCTION },
			{ "import", TokenType::KW_IMPORT },
			{ "in", TokenType::KW_IN },
			{ "typeof", TokenType::KW_TYPEOF },
			{ "var", TokenType::KW_VAR },
			{ "let", TokenType::KW_VAR },
			{ "implements", TokenType::KW_IMPLEMENTS },
			{ "private", TokenType::KW_VISIBILITY },
			{ "protected", TokenType::KW_VISIBILITY },
			{ "public", TokenType::KW_VISIBILITY },
			{ "static", TokenType::KW_STATIC },
			{ "readonly", TokenType::KW_READONLY },
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

	Token Tokenizer::parseString()
	{
		std::size_t begin = ++(*m_cursor);

		bool esc = false;

		while (this->eof() == false)
		{
			if (!esc && m_input[*m_cursor] == '"')
			{
				(*m_cursor)++;
				break;
			}
			if (!esc && m_input[*m_cursor] == '\n')
			{
				this->error("Unexpected newline");
			}
			if (!esc && m_input[*m_cursor] == '\\')
			{
				esc = true;
			}
			else
			{
				esc = false;
			}
			++(*m_cursor);
		}

		return Token(TokenType::STRING_LITERAL, std::string_view(&m_input[begin], *m_cursor - begin - 1));
	}

	Token Tokenizer::parseNumber()
	{
		std::size_t begin = *m_cursor;

		while (this->eof() == false && std::isdigit(m_input[*m_cursor]))
		{
			++(*m_cursor);
			++(*m_col);
		}

		return Token(TokenType::NUMBER, std::string_view(&m_input[begin], *m_cursor - begin));
	}

	Token Tokenizer::parsePunctuation()
	{
		std::size_t begin = *m_cursor;

		std::pair<const char *, TokenType> val[] = {
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
			{ ">", TokenType::P_LESS_THAN }
		};

		for (auto const &v : val)
		{
			if (std::size_t len = strlen(v.first); m_input.substr(begin, len) == v.first)
			{
				*m_cursor += len;
				return Token(v.second, std::string_view(&m_input[begin], len));
			}
		}
		return Token(TokenType::UNKNOWN);
	}

	void Tokenizer::peekMode(bool mode)
	{
		if (mode)
		{
			m_peekCursor = m_realCursor;
			m_peekLine = m_realLine;
			m_peekCol = m_realCol;
			m_peekLastSkipNewlineCount = m_realLastSkipNewlineCount;
			m_cursor = &m_peekCursor;
			m_line = &m_peekLine;
			m_col = &m_peekCol;
			m_lastSkipNewlineCount = &m_peekLastSkipNewlineCount;
		}
		else
		{
			m_cursor = &m_realCursor;
			m_line = &m_realLine;
			m_col = &m_realCol;
			m_lastSkipNewlineCount = &m_realLastSkipNewlineCount;
		}
	}
}