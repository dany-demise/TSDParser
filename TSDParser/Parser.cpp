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
		else if (m_input.peek().type == TokenType::KW_IMPORT)
		{
			elem << this->parseImport();
			needEndOfLine = true;
		}
		else if (m_input.peek().type == TokenType::KW_EXPORT &&
			m_input.peek(1).type == TokenType::P_EQUAL)
		{
			elem << this->parseExport();
			needEndOfLine = true;
		}
		else
		{
			this->nextAndCheck(elem, { TokenType::KW_DECLARE, TokenType::KW_EXPORT },
				"Expected a 'declare' or 'export' keyword at global namespace level");

			switch (m_input.peek().type)
			{

			case TokenType::KW_MODULE:
				elem << this->parseNamespace();
				break;
			case TokenType::ID:
				if (m_input.peek().value == "global")
				{
					elem << this->parseNamespace();
				}
				else
				{
					m_input.error("Unexpected element");
				}
				break;
			case TokenType::KW_VAR:
			case TokenType::KW_CONST:
				elem << this->parseGlobalVariable();
				needEndOfLine = true;
				break;
			case TokenType::KW_FUNCTION:
				elem << this->parseGlobalFunction();
				needEndOfLine = true;
				break;
			case TokenType::KW_CLASS:
			case TokenType::KW_INTERFACE:
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
			this->checkEndOfLine(elem);
		}

		return elem;
	}

	Token Parser::parseImport()
	{
		Token import(TokenType::Import);

		this->nextAndCheck(import, TokenType::KW_IMPORT,
			"Expected the 'import' keyword for and import declaration");

		if (m_input.nextIf(import, TokenType::STRING_LITERAL))
		{
			return import;
		}

		if (m_input.nextIf(import, TokenType::P_OPEN_BRACE))
		{
			this->nextAndCheck(import, TokenType::ID,
				"Expected an identifier as import name");

			if (m_input.nextIf(import, TokenType::KW_AS))
			{
				this->nextAndCheck(import, TokenType::ID,
					"Expected an identifier as import alias name");
			}

			this->nextAndCheck(import, TokenType::P_CLOSE_BRACE,
				"Expected a closing brace '}'");
		}
		else
		{
			this->nextAndCheck(import, { TokenType::ID, TokenType::P_STAR },
				"Expected an identifier or a '*' as import name");

			if (m_input.nextIf(import, TokenType::KW_AS))
			{
				this->nextAndCheck(import, TokenType::ID,
					"Expected an identifier as import alias name");
			}

			this->nextAndCheck(import, TokenType::P_CLOSE_BRACE,
				"Expected a closing brace '}'");
		}

		this->nextAndCheck(import, TokenType::KW_FROM,
			"Expected the keyword 'from' in import declaration");

		this->nextAndCheck(import, TokenType::STRING_LITERAL,
			"Expected a string a imported from file");

		return (import);
	}

	Token Parser::parseExport()
	{
		Token exp(TokenType::Export);

		this->nextAndCheck(exp, TokenType::KW_EXPORT,
			"Expected 'export' keyword");

		this->nextAndCheck(exp, TokenType::P_EQUAL,
			"Expected an equal sign '='");

		exp << this->parseDotId();

		return exp;
	}

	Token Parser::parseNamespace()
	{
		Token ns(TokenType::Namespace);


		this->nextAndCheck(ns, { TokenType::KW_MODULE, TokenType::ID },
			"Expected 'namespace' or 'module' keyword");

		if (ns[0].type == TokenType::ID && ns[0].value != "global")
		{
			m_input.error("Unexpected identifier");
		}

		if (ns[0].type != TokenType::ID)
		{
			ns << this->parseDotId();
		}

		this->nextAndCheck(ns, TokenType::P_OPEN_BRACE,
			"Expected a '{' at the beggining of a namespace declaration");

		while (m_input.peek().type != TokenType::P_CLOSE_BRACE)
		{
			ns << this->parseNamespaceElement();
		}

		// We know it's a P_CLOSE_BRACE
		ns << m_input.next();

		return ns;
	}

	Token Parser::parseNamespaceElement()
	{
		Token elem(TokenType::NamespaceElement);

		m_input.nextIf(elem, TokenType::KW_EXPORT);

		switch (m_input.peek().type)
		{
		case TokenType::KW_CLASS:
		case TokenType::KW_INTERFACE:
			elem << this->parseClass();
			break;
		case TokenType::KW_MODULE:
			elem << this->parseNamespace();
			break;
		case TokenType::KW_TYPE:
			elem << this->parseTypeDef();
			this->checkEndOfLine(elem);
			break;
		case TokenType::KW_FUNCTION:
			elem << this->parseGlobalFunction();
			this->checkEndOfLine(elem);
			break;
		case TokenType::KW_VAR:
		case TokenType::KW_CONST:
			elem << this->parseGlobalVariable();
			this->checkEndOfLine(elem);
			break;
		default:
			m_input.error("Expected a class, a namespace or a type");
			break;
		}

		return elem;
	}

	Token Parser::parseGlobalVariable()
	{
		Token var(TokenType::GlobalVariable);

		this->nextAndCheck(var, { TokenType::KW_VAR, TokenType::KW_CONST },
			"Expected 'var' or 'const' keyword for file level variable declaration");

		var << this->parseVariable();

		this->checkEndOfLine(var);

		return var;
	}

	Token Parser::parseGlobalFunction()
	{
		Token func(TokenType::GlobalFunction);

		this->nextAndCheck(func, TokenType::KW_FUNCTION,
			"Expected 'function' keyword for file level function declaration");

		func << this->parseFunction();

		return func;
	}

	Token Parser::parseClass()
	{
		Token clas(TokenType::Class);

		this->nextAndCheck(clas, { TokenType::KW_CLASS, TokenType::KW_INTERFACE },
			"Expected 'class' or 'interface' keyword");

		this->nextAndCheck(clas, TokenType::ID,
			"Expected an identifier as the class's name");

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
						else
						{
							this->checkToken(clas.last(), TokenType::P_COMMA,
								"Expected a comma after a generic's type parameter");
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

		this->checkToken(clas.last(), TokenType::P_OPEN_BRACE,
			"Expected a '{' for class declaration");

		while (m_input.peek().type != TokenType::P_CLOSE_BRACE)
		{
			clas << this->parseClassElement();
		}

		// We are sure it's a P_CLOSE_BRACE
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
			m_input.nextIf(elem, TokenType::KW_READONLY);

			elem << this->parseMapObject();
		}
		else if (m_input.peek().type == TokenType::KW_CONSTRUCTOR)
		{
			elem << this->parseConstructor();
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

			auto *input = &m_input;

			auto check = [&elem, &peek, &index, input](TokenType type) {
				if (peek[index].type == type &&
					peek[index + 1].type != TokenType::P_QUESTION &&
					peek[index + 1].type != TokenType::P_COLON &&
					peek[index + 1].type != TokenType::P_OPEN_PAR &&
					peek[index + 1].type != TokenType::P_GREATER_THAN)
				{
					elem << input->next();
					++index;
					return true;
				}
				return false;
			};
			
			check(TokenType::KW_VISIBILITY);
			check(TokenType::KW_STATIC);
			isReadonly = check(TokenType::KW_READONLY);

			if (m_input.peek(1).type == TokenType::P_OPEN_PAR ||
				m_input.peek(1).type == TokenType::P_GREATER_THAN ||
				(m_input.peek(1).type == TokenType::P_QUESTION &&
				m_input.peek(2).type == TokenType::P_OPEN_PAR))
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

		if (m_input.peek(0, true).type != TokenType::P_CLOSE_BRACE)
		{
			this->checkEndOfLine(elem);
		}

		return elem;
	}

	Token Parser::parseGenericParameterPack()
	{
		Token gen(TokenType::GenericParameterPack);
		bool end = false;
		bool hasDefault = false;

		this->nextAndCheck(gen, TokenType::P_GREATER_THAN,
			"Expected a '<' as generic's type declaration");

		while (end == false)
		{
			gen << this->parseGenericParameter();

			gen << m_input.next();

			if (gen.last().type == TokenType::P_LESS_THAN)
			{
				end = true;
			}
			else
			{
				this->checkToken(gen.last(), TokenType::P_COMMA,
					"Expected a comma after a generic's type");
			}
		}

		return gen;
	}

	Token Parser::parseGenericParameter()
	{
		Token param(TokenType::GenericParameter);

		this->nextAndCheck(param, TokenType::ID,
			"Expected an identifier as generic's type");

		if (m_input.nextIf(param, TokenType::KW_EXTENDS))
		{
			param << this->parseUnionType();
		}

		if (m_input.nextIf(param, TokenType::P_EQUAL))
		{
			param << this->parseUnionType();
		}

		return param;
	}

	Token Parser::parseObjectCallable()
	{
		Token func(TokenType::ObjectCallable);

		this->nextAndCheck(func, TokenType::P_OPEN_PAR,
			"Expected an opening parenthesis after the function's name");

		if (m_input.peek().type != TokenType::P_CLOSE_PAR)
		{
			func << this->parseParameterPack();
		}

		this->nextAndCheck(func, TokenType::P_CLOSE_PAR,
			"Expected a closing parenthesis after parameters declaration");

		this->nextAndCheck(func, TokenType::P_COLON,
			"Expected a ':' before the function return type");

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
		else
		{
			m_input.nextIf(func, TokenType::P_QUESTION);
		}

		this->nextAndCheck(func, TokenType::P_OPEN_PAR,
			"Expected an opening parenthesis after the function's name");

		if (m_input.peek().type != TokenType::P_CLOSE_PAR)
		{
			func << this->parseParameterPack();
		}

		this->nextAndCheck(func, TokenType::P_CLOSE_PAR,
			"Expected a closing parenthesis after parameters declaration");

		this->nextAndCheck(func, TokenType::P_COLON,
			"Expected a ':' before the function return type");

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

	Token Parser::parseConstructor()
	{
		Token constructor(TokenType::Constructor);

		this->nextAndCheck(constructor, TokenType::KW_CONSTRUCTOR,
			"Expected the 'constructor' keyword as a constructor name");

		this->nextAndCheck(constructor, TokenType::P_OPEN_PAR,
			"Expected an opening parenthesis after the constructor's name");

		if (m_input.peek().type != TokenType::P_CLOSE_PAR)
		{
			constructor << this->parseParameterPack();
		}

		this->nextAndCheck(constructor, TokenType::P_CLOSE_PAR,
			"Expected a closing parenthesis after parameters declaration");

		return constructor;
	}

	Token Parser::parseParameterPack()
	{
		Token pack(TokenType::ParameterPack);

		pack << this->parseParameter();

		while (m_input.nextIf(pack, TokenType::P_COMMA))
		{
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

		m_input.nextIf(t, TokenType::P_SPREAD);

		t << this->parseVariable();

		return t;
	}

	Token Parser::parseMapObject()
	{
		Token obj(TokenType::MapObject);

		this->nextAndCheck(obj, TokenType::P_OPEN_BRACKET,
			"Expected a '[' at the beggining of a map property");
		this->nextAndCheck(obj, TokenType::ID,
			"Expected a an identifier as the key name");
		this->nextAndCheck(obj, TokenType::P_COLON,
			"Expected a semicolon ':' after the key name");
		this->nextAndCheck(obj, TokenType::ID,
			"Expected a key type(string or number)");
		this->nextAndCheck(obj, TokenType::P_CLOSE_BRACKET,
			"Expected a ']' at the end of a map key declaration");
		this->nextAndCheck(obj, TokenType::P_COLON,
			"Expected a semicolon ':' after the map key declaration");

		obj << this->parseUnionType();

		return obj;
	}

	Token Parser::parseVariable()
	{
		Token var(TokenType::Variable);

		var << this->parseElementKey();

		m_input.nextIf(var, TokenType::P_QUESTION);

		this->nextAndCheck(var, TokenType::P_COLON,
			"Expected a ':' after the variable's name");

		var << this->parseUnionType();

		return var;
	}

	Token Parser::parseTypeDef()
	{
		Token def(TokenType::TypeDef);

		
		this->nextAndCheck(def, TokenType::KW_TYPE,
			"Expected 'type' keyword in alias type declaration");

		def << m_input.next();
		if (def[1].isKeyword() && !def[1].isReserved())
		{
			def[1].type = TokenType::ID;
		}
		this->checkToken(def[1], TokenType::ID, "Expected type alias name");

		if (m_input.peek().type == TokenType::P_GREATER_THAN)
		{
			def << this->parseGenericParameterPack();
		}

		this->nextAndCheck(def, TokenType::P_EQUAL,
			"Expected equal symbol '=' to declare the alias type");

		def << this->parseUnionType();

		return def;
	}

	Token Parser::parseFunctionTypePredicate()
	{
		Token pred(TokenType::FunctionTypePredicate);

		this->nextAndCheck(pred, TokenType::ID,
			"Expected an identifier at the beggining of a function type predicate");

		this->nextAndCheck(pred, TokenType::KW_IS,
			"Expected the keyword 'is' in a function type predicate");

		pred << this->parseUnionType();

		return pred;
	}

	Token Parser::parseTypeParenthesis()
	{
		int level = 1;
		int i;

		this->checkToken(m_input.peek(), TokenType::P_OPEN_PAR,
			"Expected a parenthesis '('");

		for (i = 1; level > 0; ++i)
		{
			Token peek = m_input.peek(i);

			if (peek.type == TokenType::P_OPEN_PAR)
			{
				level++;
			}
			else if (peek.type == TokenType::P_CLOSE_PAR)
			{
				level--;
			}
			else if (peek.type == TokenType::END_OF_FILE)
			{
				m_input.error("Unheaven number of parenthesis");
			}
		}

		if (m_input.peek(i).type == TokenType::P_ARROW)
		{
			return this->parseLambdaType();
		}
		else
		{
			return this->parseTypeGroup();
		}
	}

	Token Parser::parseTypeGroup()
	{
		Token group(TokenType::TypeGroup);

		this->nextAndCheck(group, TokenType::P_OPEN_PAR,
			"Expected parenthesis '(' before a type group");

		group << this->parseUnionType();

		this->nextAndCheck(group, TokenType::P_CLOSE_PAR,
			"Expected parenthesis ')' after a type group");

		while (m_input.nextIf(group, TokenType::P_OPEN_BRACKET))
		{
			m_input.nextIf(group, TokenType::ID);

			this->nextAndCheck(group, TokenType::P_CLOSE_BRACKET,
				"Expected a ']' at the end of the array");
		}

		return group;
	}

	Token Parser::parseUnionType()
	{
		Token unionType(TokenType::UnionType);
		bool end = false;

		while (end == false)
		{
			unionType << this->parseType();

			if (!m_input.nextIf(unionType, TokenType::P_VERTICAL_BAR))
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
			return this->parseTypeParenthesis();
		}

		if (peek.type == TokenType::KW_TYPEOF ||
			peek.type == TokenType::KW_KEYOF)
		{
			type << m_input.next();
		}

		type << this->parseDotId();

		if (m_input.nextIf(type, TokenType::P_GREATER_THAN))
		{
			bool end = false;

			while (end == false)
			{
				type << this->parseUnionType();

				if (!m_input.nextIf(type, TokenType::P_COMMA))
				{
					end = true;
				}
			}

			this->checkToken(m_input.peek(), TokenType::P_LESS_THAN,
				"Expected a '>' at the end of the generic");
			
			type << m_input.next();
		}

		while (m_input.nextIf(type, TokenType::P_OPEN_BRACKET))
		{
			m_input.nextIf(type, TokenType::ID);

			this->nextAndCheck(type, TokenType::P_CLOSE_BRACKET,
				"Expected a ']' at the end of the array");
		}

		return type;
	}

	Token Parser::parseLambdaType()
	{
		Token lambda(TokenType::LambdaType);

		this->nextAndCheck(lambda, TokenType::P_OPEN_PAR, 
			"Expected a '(' at the beggining of the lambda parameters declaration");

		if (m_input.peek().type != TokenType::P_CLOSE_PAR)
		{
			lambda << this->parseParameterPack();
		}

		this->nextAndCheck(lambda, TokenType::P_CLOSE_PAR,
			"Expected a ')' at the end of the lambda parameters declaration");

		this->nextAndCheck(lambda, TokenType::P_ARROW, 
			"Expected the arrow symbol '=>' before lambda return type declaration");

		lambda << this->parseUnionType();

		return lambda;
	}

	Token Parser::parseAnonymousType()
	{
		Token anon(TokenType::AnonymousType);

		this->nextAndCheck(anon, TokenType::P_OPEN_BRACE,
			"Expected a '{' for anonymous type declaration");

		while (m_input.peek().type != TokenType::P_CLOSE_BRACE)
		{
			anon << this->parseClassElement();
		}

		// We know it's a P_CLOSE_BRACE
		anon << m_input.next();

		return anon;
	}

	Token Parser::parseDotId()
	{
		Token dotId(TokenType::DotId);

		this->nextAndCheck(dotId, TokenType::ID, "Expected an identifier");

		while (m_input.nextIf(dotId, TokenType::P_DOT))
		{
			this->nextAndCheck(dotId, TokenType::ID, "Expected an identifier");
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

		this->checkToken(elem[0], { TokenType::ID, TokenType::NUMBER, TokenType::STRING_LITERAL },
			"Expected a key (identifier, string literal or number)");

		return elem;
	}

	void Parser::checkEndOfLine(Token & token)
	{
		token << m_input.next(true);

		this->checkToken(token.last(), { TokenType::P_SEMICOLON, TokenType::P_NEWLINE },
			"Expected a ';' or a newline at the end of the declaration");
	}
}
