#include "stdafx.h"

namespace nope::dts::parser
{
	Parser::Parser(std::string_view filename) :
		m_input(filename),
		m_ast(TokenType::UNKNOWN)
	{
	}

	Parser::~Parser() noexcept
	{
	}

	void Parser::parse()
	{
		m_ast = this->parseFile();
	}

	Token const & Parser::ast() const
	{
		return m_ast;
	}

	Token Parser::parseFile()
	{
		Token file(TokenType::File);

		while (m_input.peek().type != TokenType::END_OF_FILE)
		{
			file << this->parseFileElement();
		}

		return file;
	}

	Token Parser::parseFileElement()
	{
		Token elem(TokenType::FileElement);

		elem << m_input.next();

		if (elem[0].type != TokenType::KW_DECLARE)
		{
			m_input.error("Expected a 'declare' keyword at global namespace level");
		}

		switch (m_input.peek().type)
		{
		case TokenType::KW_MODULE:
			elem << this->parseNamespace();
			break;
		default:
			m_input.error("Expected a namespace");
			break;
		}

		return elem;
	}

	Token Parser::parseNamespace()
	{
		Token ns(TokenType::Namespace);

		ns << m_input.next();
		ns << this->parseDotId();
		ns << m_input.next();

		if (ns[0].type != TokenType::KW_MODULE)
		{
			m_input.error("Expected 'namespace' or 'module' keyword, got ");
		}

		if (ns[2].type != TokenType::P_OPEN_BRACE)
		{
			m_input.error("Expected a '{' for namespace declaration");
		}

		while (m_input.peek().type != TokenType::P_CLOSE_BRACE)
		{
			ns << this->parseNamespaceElement();
		}

		ns << m_input.next();

		return ns;
	}

	Token Parser::parseNamespaceElement()
	{
		Token elem(TokenType::NamespaceElement);

		if (m_input.peek().type == TokenType::KW_EXPORT)
		{
			elem << m_input.next();
		}

		switch (m_input.peek().type)
		{
		case TokenType::KW_CLASS:
			elem << this->parseClass();
			break;
		case TokenType::KW_MODULE:
			elem << this->parseNamespace();
			break;
		default:
			m_input.error("Expected a class or namespace");
			break;
		}

		return elem;
	}

	Token Parser::parseClass()
	{
		Token clas(TokenType::Class);

		clas << m_input.next();
		clas << m_input.next();
		clas << m_input.next();

		if (clas[0].type != TokenType::KW_CLASS)
		{
			m_input.error("Expected 'class' or 'interface' keyword");
		}

		if (clas[1].type != TokenType::ID)
		{
			m_input.error("Expected an identifier as the class's name");
		}

		if (clas[2].type != TokenType::P_OPEN_BRACE)
		{
			m_input.error("Expected a '{' for class declaration");
		}

		while (m_input.peek().type != TokenType::P_CLOSE_BRACE)
		{
			clas << this->parseClassElement();
		}

		clas << m_input.next();

		return clas;
	}

	Token Parser::parseClassElement()
	{
		Token elem(TokenType::ClassElement);

		if (m_input.peek().type == TokenType::KW_VISIBILITY)
		{
			elem << m_input.next();
		}

		if (m_input.peek().type == TokenType::KW_STATIC)
		{
			elem << m_input.next();
		}

		if (m_input.peek(1).type == TokenType::P_OPEN_PAR)
		{
			elem << this->parseFunction();
		}
		else
		{
			elem << this->parseVariable();
		}

		elem << m_input.next();

		if (elem.last().type != TokenType::P_SEMICOLON)
		{
			m_input.error("Expected a ';' at the end of the declaration");
		}

		return elem;
	}

	Token Parser::parseFunction()
	{
		Token func(TokenType::Function);

		func << m_input.next();
		func << m_input.next();

		if (func[0].type != TokenType::ID)
		{
			m_input.error("Expected an identifier as function's name");
		}
		if (func[1].type != TokenType::P_OPEN_PAR)
		{
			m_input.error("Expected an opening parenthesis after the function's name");
		}

		if (m_input.peek().type != TokenType::P_CLOSE_PAR)
		{
			func << this->parseParameterPack();
		}

		func << m_input.next();

		if (func.last().type != TokenType::P_CLOSE_PAR)
		{
			m_input.error("Expected a closing parenthesis after parameters declaration");
		}

		func << m_input.next();

		if (func.last().type != TokenType::P_COLON)
		{
			m_input.error("Expected a ':' before the function return type");
		}

		func << this->parseType();

		return func;
	}

	Token Parser::parseParameterPack()
	{
		Token pack(TokenType::ParameterPack);

		pack << this->parseParameter();

		while (m_input.peek().type == TokenType::P_COMMA)
		{
			pack << m_input.next();
			pack << this->parseParameter();
		}

		return pack;
	}

	Token Parser::parseParameter()
	{
		Token t(TokenType::UNKNOWN);

		// TODO: uncomment this and implement parseAssignation()
		/*if (m_input.peek(1).type == TokenType::P_EQUAL)
		{
			return this->parseAssignation();
		}*/

		return this->parseVariable();
	}

	Token Parser::parseVariable()
	{
		Token var(TokenType::Variable);

		var << m_input.next();
		var << m_input.next();

		if (var[0].type != TokenType::ID)
		{
			m_input.error("Expected an identifier as variable's name");
		}

		if (var[1].type == TokenType::P_QUESTION)
		{
			var << m_input.next();
		}

		if (var.last().type != TokenType::P_COLON)
		{
			m_input.error("Expected a ':' after the variable's name");
		}

		var << this->parseType();

		return var;
	}

	Token Parser::parseType()
	{
		Token type(TokenType::Type);

		Token peek = m_input.peek();

		if (peek.type == TokenType::KW_TYPEOF)
		{
			type << m_input.next();
		}

		type = this->parseDotId();

		return type;
	}

	Token Parser::parseDotId()
	{
		Token dotId(TokenType::DotId);

		dotId << m_input.next();

		if (dotId[0].type != TokenType::ID)
		{
			m_input.error("Expected an identifier");
		}

		while (m_input.peek(0).type == TokenType::P_DOT && m_input.peek(1).type == TokenType::ID)
		{
			dotId << m_input.next();
			dotId << m_input.next();
		}

		return dotId;
	}
}
