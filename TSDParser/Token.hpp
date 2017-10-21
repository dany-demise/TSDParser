#ifndef NOPE_DTS_PARSER_TOKEN_HPP_
# define NOPE_DTS_PARSER_TOKEN_HPP_

# include <string_view>
# include <vector>

namespace nope::dts::parser
{
	enum class TokenType
	{
		Unknown = -1,
		ID,
		KEYWORD,
		STRING,
		NUMBER,
		PUNCTUATION,
		OPERATOR,
		Variable,
		ParameterPack,
		Function,
		Property,
		Class,
		Namespace
	};

	struct Token
	{
		Token() = default;
		Token(TokenType t, std::string_view val);
		Token(Token const &that) = default;
		Token(Token &&that) = default;

		~Token() noexcept = default;

		Token &operator=(Token const &that) = default;
		Token &operator=(Token &&that) = default;

		TokenType type;
		std::string_view value;
		std::vector<Token> child;
	};
}

#endif // !NOPE_DTS_PARSER_TOKEN_HPP_