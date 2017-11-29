#ifndef NOPE_DTS_PARSER_TOKEN_HPP_
# define NOPE_DTS_PARSER_TOKEN_HPP_

# include <string_view>
# include <vector>

namespace nope::dts::parser
{
	enum class TokenType
	{
		UNKNOWN,
		END_OF_FILE,
		ID,
		KW_CLASS,
		KW_INTERFACE,
		KW_CONST,
		KW_ENUM,
		KW_EXPORT,
		KW_EXTENDS,
		KW_FUNCTION,
		KW_IMPORT,
		KW_IN,
		KW_TYPEOF,
		KW_VAR,
		KW_IMPLEMENTS,
		KW_VISIBILITY,
		KW_STATIC,
		KW_READONLY,
		KW_AS,
		KW_DECLARE,
		KW_FROM,
		KW_MODULE,
		KW_REQUIRE,
		P_ARROW,
		P_COLON,
		P_SEMICOLON,
		P_NEWLINE,
		P_DOT,
		P_COMMA,
		P_SLASH,
		P_ANTISLASH,
		P_QUESTION,
		P_STAR,
		P_OPEN_PAR,
		P_CLOSE_PAR,
		P_OPEN_BRACE,
		P_CLOSE_BRACE,
		P_OPEN_BRACKET,
		P_CLOSE_BRACKET,
		P_EQUAL,
		P_VERTICAL_BAR,
		P_GREATER_THAN,
		P_LESS_THAN,
		STRING_LITERAL,
		NUMBER,
		ElementKey,
		DotId,
		Type,
		AnonymousType,
		LambdaType,
		UnionType,
		Variable,
		ParameterPack,
		ObjectCallable,
		Function,
		Property,
		GenericParameter,
		ClassElement,
		Class,
		GlobalFunction,
		GlobalVariable,
		NamespaceElement,
		Namespace,
		FileElement,
		File
	};

	struct Token
	{
		Token() = default;
		Token(TokenType t, std::string_view val = "");
		Token(Token const &that) = default;
		Token(Token &&that) = default;

		~Token() noexcept = default;

		Token &operator=(Token const &that) = default;
		Token &operator=(Token &&that) = default;

		bool operator==(Token const &that) const;
		bool operator!=(Token const &that) const;

		bool isTerminal() const;

		bool operator<(Token const &that) const;

		TokenType type;
		std::string_view value;
		std::vector<Token> child;

		Token &operator<<(Token &&children);

		Token const &operator[](std::size_t i) const;
		Token &operator[](std::size_t i);

		std::size_t size() const;

		Token const &last() const;
		Token &last();

		std::string json() const;
		std::string code() const;
		std::string xml() const;
	};

	bool operator<(TokenType l, TokenType r);
	std::ostream &operator<<(std::ostream &os, TokenType t);
}

#endif // !NOPE_DTS_PARSER_TOKEN_HPP_