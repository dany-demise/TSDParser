#ifndef NOPE_DTS_PARSER_TOKENIZER_HPP_
# define NOPE_DTS_PARSER_TOKENIZER_HPP_

# include <functional>
# include <string_view>
# include <cinttypes>
# include "Token.hpp"

namespace nope::dts::parser
{
	class Tokenizer
	{
	public:
		Tokenizer() = delete;
		Tokenizer(std::string_view filename);
		Tokenizer(Tokenizer const &that) = delete;
		Tokenizer(Tokenizer &&that) = delete;
		
		~Tokenizer() noexcept;

		Tokenizer &operator=(Tokenizer const &that) = delete;
		Tokenizer &operator=(Tokenizer &&that) = delete;

		Token peek(std::uint32_t lookAhead = 0, bool ignoreNewline = false);
		Token next(bool ignoreNewline = false);
		bool nextIf(Token &token, TokenType type, std::uint32_t lookAhead = 0, bool skipNewline = true);
		bool nextIf(Token &token, std::function<bool()> func);
		bool eof() const;

		void error(std::string_view message, std::size_t line, std::size_t col) const;
		void error(std::string_view message) const;
	private:
		bool _eof(std::size_t cursor) const;
		std::size_t remain(std::size_t cursor) const;

		// Parsing methods
		Token parseSpace(std::size_t &cursor);
		Token parseLineComment(std::size_t &cursor);
		Token parseBlockComment(std::size_t &cursor);
		Token parseId(std::size_t &cursor);
		void filterKeyword(Token &token) const;
		Token parseString(std::size_t &cursor);
		Token parseNumber(std::size_t &cursor);
		Token parsePunctuation(std::size_t &cursor);

		std::pair<std::size_t, std::size_t> getCursorPosition(std::size_t index) const;

		std::string m_filename;
		std::string m_input;
		std::vector<Token> m_token;

		std::size_t m_cursor;
	};
}

#endif // !NOPE_DTS_PARSER_TOKENIZER_HPP_