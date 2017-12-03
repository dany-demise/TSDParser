#ifndef NOPE_DTS_PARSER_PARSER_HPP_
# define NOPE_DTS_PARSER_PARSER_HPP_

# include <string_view>
# include "Token.hpp"
# include "Tokenizer.hpp"

namespace nope::dts::parser
{
	class Parser
	{
	public:
		Parser() = delete;
		Parser(std::string_view filename);
		Parser(Parser const &that) = delete;
		Parser(Parser &&that) = default;
		
		~Parser() noexcept;

		Parser &operator=(Parser const &that) = delete;
		Parser &operator=(Parser &&that) = default;

		void parse();

		Token const &ast() const;

	private:
		Token parseFile();
		Token parseFileElement();
		Token parseImport();
		Token parseExport();
		Token parseNamespace();
		Token parseNamespaceElement();
		Token parseGlobalVariable();
		Token parseGlobalFunction();
		Token parseClass();
		Token parseClassElement();
		Token parseGenericParameterPack();
		Token parseGenericParameter();
		Token parseObjectCallable();
		Token parseFunction();
		Token parseConstructor();
		Token parseParameterPack();
		Token parseParameter();
		Token parseMapObject();
		Token parseVariable();
		Token parseTypeDef();
		Token parseFunctionTypePredicate();
		Token parseTypeParenthesis();
		Token parseTypeGroup();
		Token parseUnionType();
		Token parseType();
		Token parseLambdaType();
		Token parseAnonymousType();
		Token parseDotId();
		Token parseElementKey();

		void checkEndOfLine(Token &token);

		inline void Parser::nextAndCheck(Token & token, TokenType type, std::string_view msg)
		{
			token << m_input.next();

			this->checkToken(token.last(), type, msg);
		}

		inline void Parser::nextAndCheck(Token & token, std::initializer_list<TokenType> types, std::string_view msg)
		{
			token << m_input.next();

			this->checkToken(token.last(), types, msg);
		}

		inline void Parser::checkToken(Token const & token, TokenType type, std::string_view msg) const
		{
			if (token.type != type)
			{
				m_input.error(msg);
			}
		}

		inline void Parser::checkToken(Token const & token, std::initializer_list<TokenType> types, std::string_view msg) const
		{
			for (auto type : types)
			{
				if (token.type == type)
				{
					return;
				}
			}

			m_input.error(msg);
		}

		Tokenizer m_input;
		Token m_ast;
	};
}

#endif // !NOPE_DTS_PARSER_PARSER_HPP_