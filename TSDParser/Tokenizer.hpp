#ifndef NOPE_DTS_PARSER_TOKENIZER_HPP_
# define NOPE_DTS_PARSER_TOKENIZER_HPP_

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

		Token peek(std::uint32_t lookAhead = 0, bool skipNewline = true);
		Token next(bool skipNewline = true);
		bool eof() const;

		void error(std::string const &message) const;
	private:
		std::size_t remain() const;

		// Skip methods
		void skip();
		void skipLineComment();
		void skipBlockComment();
		void skipBlank();

		// Parsing methods
		Token parseId();
		void filterKeyword(Token &token) const;
		Token parseString();
		Token parseNumber();
		Token parsePunctuation();

		// Switch between normal and peek mode
		void peekMode(bool mode);

		std::string m_filename;

		std::uint32_t m_realLine;
		std::uint32_t m_realCol;
		std::size_t m_realCursor;
		std::uint32_t m_realLastSkipNewlineCount;

		std::uint32_t *m_line;
		std::uint32_t *m_col;
		std::size_t *m_cursor;
		std::uint32_t *m_lastSkipNewlineCount;

		std::uint32_t m_peekLine;
		std::uint32_t m_peekCol;
		std::size_t m_peekCursor;
		std::uint32_t m_peekLastSkipNewlineCount;

		std::string m_input;

	};
}

#endif // !NOPE_DTS_PARSER_TOKENIZER_HPP_