#ifndef NOPE_DTS_PARSER_TOKENIZER_HPP_
# define NOPE_DTS_PARSER_TOKENIZER_HPP_

# include <string>
# include <cinttypes>
# include "Token.hpp"

namespace nope::dts::parser
{
	class Tokenizer
	{
	public:
		Tokenizer() = delete;
		/// <summary>
		/// Initializes a new instance of the <see cref="Tokenizer"/> class.
		/// </summary>
		/// <param name="filename">The filename.</param>
		Tokenizer(std::string const &filename);
		Tokenizer(Tokenizer const &that) = delete;
		Tokenizer(Tokenizer &&that) = delete;
		
		~Tokenizer() noexcept;

		Tokenizer &operator=(Tokenizer const &that) = delete;
		Tokenizer &operator=(Tokenizer &&that) = delete;

		Token peek(std::uint32_t lookAhead = 0) const;
		Token next();

		void error(std::string const &message) const;
	private:
		std::uint32_t m_line;
		std::uint32_t m_col;

		std::int32_t m_cursor;
		std::string m_input;
	};
}

#endif // !NOPE_DTS_PARSER_TOKENIZER_HPP_