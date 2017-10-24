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
		Token parseNamespace();
		Token parseNamespaceElement();
		//token parseglobalvariable();
		//token parseglobalfunction();
		Token parseClass();
		Token parseClassElement();
		Token parseFunction();
		Token parseParameterPack();
		Token parseParameter();
		Token parseVariable();
		Token parseType();
		Token parseDotId();

		Tokenizer m_input;
		Token m_ast;
	};
}

#endif // !NOPE_DTS_PARSER_PARSER_HPP_