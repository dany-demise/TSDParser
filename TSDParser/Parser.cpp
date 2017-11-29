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
		bool needEndOfLine = false;

		if (m_input.peek().type == TokenType::KW_INTERFACE)
		{
			elem << this->parseClass();
		}
		else if (m_input.peek().type == TokenType::KW_TYPE)
		{
			elem << this->parseTypeDef();
			needEndOfLine = true;
		}
		else
		{

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
			case TokenType::KW_VAR:
				elem << this->parseGlobalVariable();
				needEndOfLine = true;
				break;
			case TokenType::KW_FUNCTION:
				elem << this->parseGlobalFunction();
				needEndOfLine = true;
				break;
			case TokenType::KW_CLASS:
				elem << this->parseClass();
				break;
			case TokenType::KW_TYPE:
				elem << this->parseTypeDef();
				needEndOfLine = true;
				break;
			default:
				m_input.error("Unexpected element");
				break;
			}
		}

		if (needEndOfLine)
		{
			elem << m_input.next(false);

			if (elem.last().type != TokenType::P_SEMICOLON &&
				elem.last().type != TokenType::P_NEWLINE)
			{
				m_input.error("Expected a ';' or a newline at the end of the declaration");
			}
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
		case TokenType::KW_INTERFACE:
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

	Token Parser::parseGlobalVariable()
	{
		Token var(TokenType::GlobalVariable);

		var << m_input.next();

		if (var[0].type != TokenType::KW_VAR)
		{
			m_input.error("Expected 'var' keyword for file level variable declaration");
		}

		var << this->parseVariable();

		var << m_input.next(false);

		if (var.last().type != TokenType::P_SEMICOLON &&
			var.last().type != TokenType::P_NEWLINE)
		{
			m_input.error("Expected a ';' or a newline at the end of the declaration");
		}

		return var;
	}

	Token Parser::parseGlobalFunction()
	{
		Token func(TokenType::GlobalFunction);

		func << m_input.next();

		if (func[0].type != TokenType::KW_FUNCTION)
		{
			m_input.error("Expected 'function' keyword for file level function declaration");
		}

		func << this->parseFunction();

		return func;
	}

	Token Parser::parseClass()
	{
		Token clas(TokenType::Class);

		clas << m_input.next();
		clas << m_input.next();

		if (clas[0].type != TokenType::KW_CLASS && clas[0].type != TokenType::KW_INTERFACE)
		{
			m_input.error("Expected 'class' or 'interface' keyword");
		}

		if (clas[1].type != TokenType::ID)
		{
			m_input.error("Expected an identifier as the class's name");
		}

		if (m_input.peek().type == TokenType::P_GREATER_THAN)
		{
			clas << this->parseGenericParameterPack();
		}

		clas << m_input.next();

		if (clas.last().type == TokenType::KW_EXTENDS)
		{
			bool endExtend = false;

			while (endExtend == false)
			{
				clas << this->parseDotId();
				clas << m_input.next();
				
				if (clas.last().type == TokenType::P_GREATER_THAN)
				{
					bool endGeneric = false;

					while (endGeneric == false)
					{
						clas << this->parseUnionType();
						clas << m_input.next();

						if (clas.last().type == TokenType::P_LESS_THAN)
						{
							endGeneric = true;
						}
						else if (clas.last().type != TokenType::P_COMMA)
						{
							m_input.error("Expected a comma after a generic's type parameter");
						}
					}
					clas << m_input.next();
				}

				if (clas.last().type != TokenType::P_COMMA)
				{
					endExtend = true;
				}
			}
		}

		if (clas.last().type != TokenType::P_OPEN_BRACE)
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

		if (m_input.peek().type == TokenType::P_OPEN_PAR)
		{
			elem << this->parseObjectCallable();
		}
		else if (m_input.peek().type == TokenType::P_OPEN_BRACKET)
		{
			if (m_input.peek().type == TokenType::KW_READONLY)
			{
				elem << m_input.next();
			}

			elem << this->parseMapObject();
		}
		else
		{
			bool isReadonly = false;
			Token peek[4] = {
				m_input.peek(0),
				m_input.peek(1),
				m_input.peek(2),
				m_input.peek(3)
			};
			int index = 0;

			auto check = [](Token const &token, TokenType type, Token const &peek) {
				return token.type == type &&
					peek.type != TokenType::P_QUESTION &&
					peek.type != TokenType::P_COLON &&
					peek.type != TokenType::P_OPEN_PAR &&
					peek.type != TokenType::P_GREATER_THAN;
			};
			
			if (check(peek[index], TokenType::KW_VISIBILITY, peek[index + 1]))
			{
				elem << m_input.next();
				++index;
			}

			if (check(peek[index], TokenType::KW_STATIC, peek[index + 1]))
			{
				elem << m_input.next();
				++index;
			}

			if (check(peek[index], TokenType::KW_READONLY, peek[index + 1]))
			{
				elem << m_input.next();
				isReadonly = true;
			}

			if (m_input.peek(1).type == TokenType::P_OPEN_PAR ||
				m_input.peek(1).type == TokenType::P_GREATER_THAN)
			{
				if (isReadonly)
				{
					m_input.error("'readonly' can only be affected to properties, and not methods");
				}
				elem << this->parseFunction();
			}
			else
			{
				elem << this->parseVariable();
			}
		}

		if (m_input.peek(0, false).type != TokenType::P_SEMICOLON &&
			m_input.peek(0, false).type != TokenType::P_NEWLINE &&
			m_input.peek(0, false).type != TokenType::P_CLOSE_BRACE)
		{
			m_input.error("Expected a ';' or a newline at the end of the declaration");
		}

			
		if (m_input.peek(0, false).type != TokenType::P_CLOSE_BRACE)
		{
			elem << m_input.next(false);
		}

		return elem;
	}

	Token Parser::parseGenericParameterPack()
	{
		Token gen(TokenType::GenericParameterPack);
		bool end = false;
		bool hasDefault = false;

		gen << m_input.next();

		if (gen[0].type != TokenType::P_GREATER_THAN)
		{
			m_input.error("Expected a '<' as generic's type declaration");
		}

		while (end == false)
		{
			gen << this->parseGenericParameter();

			gen << m_input.next();

			if (gen.last().type == TokenType::P_LESS_THAN)
			{
				end = true;
			}
			else if (gen.last().type != TokenType::P_COMMA)
			{
				m_input.error("Expected a comma after a generic's type");
			}
		}

		return gen;
	}

	Token Parser::parseGenericParameter()
	{
		Token param(TokenType::GenericParameter);

		param << m_input.next();

		if (param.last().type != TokenType::ID)
		{
			m_input.error("Expected an identifier as generic's type");
		}

		if (m_input.peek().type == TokenType::KW_EXTENDS)
		{
			param << m_input.next();
			param << this->parseUnionType();
		}
		
		if (m_input.peek().type == TokenType::P_EQUAL)
		{
			param << m_input.next();
			param << this->parseUnionType();
		}

		return param;
	}

	Token Parser::parseObjectCallable()
	{
		Token func(TokenType::ObjectCallable);

		func << m_input.next();

		if (func[0].type != TokenType::P_OPEN_PAR)
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

		func << this->parseUnionType();

		return func;
	}

	Token Parser::parseFunction()
	{
		Token func(TokenType::Function);

		func << this->parseElementKey();
		
		if (m_input.peek().type == TokenType::P_GREATER_THAN)
		{
			func << this->parseGenericParameterPack();
		}
		
		func << m_input.next();

		if (func.last().type != TokenType::P_OPEN_PAR)
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

		if (m_input.peek(1).type == TokenType::KW_IS)
		{
			func << this->parseFunctionTypePredicate();
		}
		else
		{
			func << this->parseUnionType();
		}

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
		Token t(TokenType::Parameter);

		// TODO: uncomment this and implement parseAssignation()
		/*if (m_input.peek(1).type == TokenType::P_EQUAL)
		{
			return this->parseAssignation();
		}*/

		if (m_input.peek().type == TokenType::P_SPREAD)
		{
			t << m_input.next();
		}

		t << this->parseVariable();

		return t;
	}

	Token Parser::parseMapObject()
	{
		Token obj(TokenType::MapObject);

		obj << m_input.next();
		obj << m_input.next();
		obj << m_input.next();
		obj << m_input.next();
		obj << m_input.next();
		obj << m_input.next();

		if (obj[0].type != TokenType::P_OPEN_BRACKET)
		{
			m_input.error("Expected a '[' at the beggining of a map property");
		}
		if (obj[1].type != TokenType::ID)
		{
			m_input.error("Expected a an identifier as the key name");
		}
		if (obj[2].type != TokenType::P_COLON)
		{
			m_input.error("Expected a semicolon ':' after the key name");
		}
		if (obj[3].type != TokenType::ID)
		{
			m_input.error("Expected a key type (string or number)");
		}
		if (obj[4].type != TokenType::P_CLOSE_BRACKET)
		{
			m_input.error("Expected a ']' at the end of a map key declaration");
		}
		if (obj[5].type != TokenType::P_COLON)
		{
			m_input.error("Expected a semicolon ':' after the map key declaration");
		}

		obj << this->parseUnionType();

		return obj;
	}

	Token Parser::parseVariable()
	{
		Token var(TokenType::Variable);

		var << this->parseElementKey();

		var << m_input.next();

		if (var[1].type == TokenType::P_QUESTION)
		{
			var << m_input.next();
		}

		if (var.last().type != TokenType::P_COLON)
		{
			m_input.error("Expected a ':' after the variable's name");
		}

		var << this->parseUnionType();

		return var;
	}

	Token Parser::parseTypeDef()
	{
		Token def(TokenType::TypeDef);

		def << m_input.next();
		def << m_input.next();
		def << m_input.next();

		if (def[0].type != TokenType::KW_TYPE)
		{
			m_input.error("Expected 'type' keyword in alias type declaration");
		}

		if (def[1].isKeyword() && !def[1].isReserved())
		{
			def[1].type = TokenType::ID;
		}

		if (def[1].type != TokenType::ID)
		{
			m_input.error("Expected type alias name");
		}

		if (def[2].type != TokenType::P_EQUAL)
		{
			m_input.error("Expected equal symbol '=' to declare the alias type");
		}

		def << this->parseUnionType();

		return def;
	}

	Token Parser::parseFunctionTypePredicate()
	{
		Token pred(TokenType::FunctionTypePredicate);

		pred << m_input.next();
		pred << m_input.next();

		if (pred[0].type != TokenType::ID)
		{
			m_input.error("Expected an identifier at the beggining of a function type predicate");
		}

		if (pred[1].type != TokenType::KW_IS)
		{
			m_input.error("Expected the keyword 'is' in a function type predicate");
		}

		pred << this->parseUnionType();

		return pred;
	}

	Token Parser::parseUnionType()
	{
		Token unionType(TokenType::UnionType);
		bool end = false;

		while (end == false)
		{
			unionType << this->parseType();

			if (m_input.peek(0, false).type == TokenType::P_VERTICAL_BAR)
			{
				unionType << m_input.next();
			}
			else
			{
				end = true;
			}
		}

		if (unionType.child.size() == 1)
		{
			return unionType.child[0];
		}
		else
		{
			return unionType;
		}
	}

	Token Parser::parseType()
	{
		Token type(TokenType::Type);
		Token peek = m_input.peek();

		if (peek.type == TokenType::STRING_LITERAL ||
			peek.type == TokenType::NUMBER)
		{
			return m_input.next();
		}
		else if (peek.type == TokenType::P_OPEN_BRACE)
		{
			return this->parseAnonymousType();
		}
		else if (peek.type == TokenType::P_OPEN_PAR)
		{
			return this->parseLambdaType();
		}

		if (peek.type == TokenType::KW_TYPEOF ||
			peek.type == TokenType::KW_KEYOF)
		{
			type << m_input.next();
		}

		type << this->parseDotId();

		peek = m_input.peek();

		if (peek.type == TokenType::P_GREATER_THAN)
		{
			type << m_input.next();

			bool end = false;

			while (end == false)
			{
				type << this->parseUnionType();

				if (m_input.peek().type == TokenType::P_COMMA)
				{
					type << m_input.next();
				}
				else
				{
					end = true;
				}
			}

			if (m_input.peek().type != TokenType::P_LESS_THAN)
			{
				m_input.error("Expected a '>' at the end of the generic");
			}
			type << m_input.next();
		}

		while (m_input.peek().type == TokenType::P_OPEN_BRACKET)
		{
			type << m_input.next();
			type << m_input.next();

			if (type.last().type == TokenType::ID)
			{
				type << m_input.next();
			}

			if (type.last().type != TokenType::P_CLOSE_BRACKET)
			{
				m_input.error("Expected a ']' at the end of the array");
			}
		}

		return type;
	}

	Token Parser::parseLambdaType()
	{
		Token lambda(TokenType::LambdaType);

		lambda << m_input.next();

		if (lambda[0].type != TokenType::P_OPEN_PAR)
		{
			m_input.error("Expected a '(' at the beggining of the lambda parameters declaration");
		}

		if (m_input.peek().type != TokenType::P_CLOSE_PAR)
		{
			lambda << this->parseParameterPack();
		}

		lambda << m_input.next();

		if (lambda.last().type != TokenType::P_CLOSE_PAR)
		{
			m_input.error("Expected a ')' at the end of the lambda parameters declaration");
		}

		lambda << m_input.next();

		if (lambda.last().type != TokenType::P_ARROW)
		{
			m_input.error("Expected the arrow symbol '=>' before lambda return type declaration");
		}

		lambda << this->parseUnionType();

		return lambda;
	}

	Token Parser::parseAnonymousType()
	{
		Token anon(TokenType::AnonymousType);

		anon << m_input.next();

		if (anon[0].type != TokenType::P_OPEN_BRACE)
		{
			m_input.error("Expected a '{' for anonymous type declaration");
		}

		while (m_input.peek().type != TokenType::P_CLOSE_BRACE)
		{
			anon << this->parseClassElement();
		}

		anon << m_input.next();

		return anon;
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

	Token Parser::parseElementKey()
	{
		Token elem(TokenType::ElementKey);

		elem << m_input.next();

		if (elem[0].isKeyword())
		{
			elem[0].type = TokenType::ID;
		}

		if (elem[0].type != TokenType::ID &&
			elem[0].type != TokenType::NUMBER &&
			elem[0].type != TokenType::STRING_LITERAL)
		{
			m_input.error("Expected a key (identifier, string literal or number)");
		}

		return elem;
	}
}
