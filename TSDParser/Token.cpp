#include "stdafx.h"

namespace nope::dts::parser
{
	Token::Token(TokenType t, std::string_view val) :
		type(t),
		value(val),
		child()
	{
	}

	bool Token::operator==(Token const & that) const
	{
		return type == that.type && value == that.value;
	}

	bool Token::operator!=(Token const & that) const
	{
		return !(*this == that);
	}

	bool Token::isTerminal() const
	{
		switch (type)
		{
		case TokenType::END_OF_FILE:
		case TokenType::ID:
		case TokenType::KW_CLASS:
		case TokenType::KW_CONST:
		case TokenType::KW_ENUM:
		case TokenType::KW_EXPORT:
		case TokenType::KW_EXTENDS:
		case TokenType::KW_FUNCTION:
		case TokenType::KW_IMPORT:
		case TokenType::KW_IN:
		case TokenType::KW_TYPEOF:
		case TokenType::KW_VAR:
		case TokenType::KW_IMPLEMENTS:
		case TokenType::KW_VISIBILITY:
		case TokenType::KW_STATIC:
		case TokenType::KW_AS:
		case TokenType::KW_DECLARE:
		case TokenType::KW_FROM:
		case TokenType::KW_MODULE:
		case TokenType::KW_REQUIRE:
		case TokenType::P_COLON:
		case TokenType::P_SEMICOLON:
		case TokenType::P_DOT:
		case TokenType::P_COMMA:
		case TokenType::P_SLASH:
		case TokenType::P_ANTISLASH:
		case TokenType::P_QUESTION:
		case TokenType::P_STAR:
		case TokenType::P_OPEN_PAR:
		case TokenType::P_CLOSE_PAR:
		case TokenType::P_OPEN_BRACE:
		case TokenType::P_CLOSE_BRACE:
		case TokenType::P_OPEN_BRACKET:
		case TokenType::P_CLOSE_BRACKET:
		case TokenType::P_EQUAL:
		case TokenType::STRING_LITERAL:
		case TokenType::NUMBER:
			return true;
		default:
			return false;
		}
	}

	bool Token::operator<(Token const & that) const
	{
		if (type < that.type)
		{
			return true;
		}
		if (type == that.type)
		{
			return value < that.value;
		}
		return false;
	}

	Token & Token::operator<<(Token &&children)
	{
		std::cout << "Push " << children.type << " => " << children.value << std::endl;
		child.push_back(std::move(children));

		return *this;
	}

	Token const & Token::operator[](std::size_t i) const
	{
		return child[i];
	}

	Token & Token::operator[](std::size_t i)
	{
		return child[i];
	}

	std::size_t Token::size() const
	{
		return child.size();
	}

	Token const & Token::last() const
	{
		return child.back();
	}

	Token & Token::last()
	{
		return child.back();
	}

	std::string Token::json() const
	{
		std::stringstream ss;

		ss << "{\"type\":\"" << this->type << "\", ";
		
		if (this->isTerminal())
		{
			ss << "\"value\":\"" << this->value << "\"}";
		}
		else
		{
			
			ss << "\"child\":[";

			for (int i = 0; i < child.size(); ++i)
			{
				if (i != 0)
				{
					ss << ", ";
				}
				ss << child[i].json();
			}
			ss << "]}";
		}
		return ss.str();
	}

	std::string Token::code() const
	{

		if (this->isTerminal())
		{
			return std::string(this->value);
		}

		std::stringstream ss;

		for (int i = 0; i < child.size(); ++i)
		{
			if (i != 0)
			{
				ss << ' ';
			}
			ss << child[i].code();
		}

		return ss.str();
	}

	bool operator<(TokenType l, TokenType r)
	{
		return static_cast<int>(l) < static_cast<int>(r);
	}

	std::ostream & operator<<(std::ostream & os, TokenType t)
	{
		std::string_view s;

		switch (t)
		{
		case TokenType::UNKNOWN:
			s = "UNKNOWN";
			break;
		case TokenType::END_OF_FILE:
			s = "EOF";
			break;
		case TokenType::ID:
			s = "ID";
			break;
		case TokenType::KW_CLASS:
			s = "KW_CLASS";
			break;
		case TokenType::KW_CONST:
			s = "KW_CONST";
			break;
		case TokenType::KW_ENUM:
			s = "KW_ENUM";
			break;
		case TokenType::KW_EXPORT:
			s = "KW_EXPORT";
			break;
		case TokenType::KW_EXTENDS:
			s = "KW_EXTENDS";
			break;
		case TokenType::KW_FUNCTION:
			s = "KW_FUNCTION";
			break;
		case TokenType::KW_IMPORT:
			s = "KW_IMPORT";
			break;
		case TokenType::KW_IN:
			s = "KW_IN";
			break;
		case TokenType::KW_TYPEOF:
			s = "KW_TYPEOF";
			break;
		case TokenType::KW_VAR:
			s = "KW_VAR";
			break;
		case TokenType::KW_IMPLEMENTS:
			s = "KW_IMPLEMENTS";
			break;
		case TokenType::KW_VISIBILITY:
			s = "KW_VISIBILITY";
			break;
		case TokenType::KW_STATIC:
			s = "KW_STATIC";
			break;
		case TokenType::KW_AS:
			s = "KW_AS";
			break;
		case TokenType::KW_DECLARE:
			s = "KW_DECLARE";
			break;
		case TokenType::KW_FROM:
			s = "KW_FROM";
			break;
		case TokenType::KW_MODULE:
			s = "KW_MODULE";
			break;
		case TokenType::KW_REQUIRE:
			s = "KW_REQUIRE";
			break;
		case TokenType::P_COLON:
			s = "P_COLON";
			break;
		case TokenType::P_SEMICOLON:
			s = "P_SEMICOLON";
			break;
		case TokenType::P_DOT:
			s = "P_DOT";
			break;
		case TokenType::P_COMMA:
			s = "P_COMMA";
			break;
		case TokenType::P_SLASH:
			s = "P_SLASH";
			break;
		case TokenType::P_ANTISLASH:
			s = "P_ANTISLASH";
			break;
		case TokenType::P_QUESTION:
			s = "P_QUESTION";
			break;
		case TokenType::P_STAR:
			s = "P_STAR";
			break;
		case TokenType::P_OPEN_PAR:
			s = "P_OPEN_PAR";
			break;
		case TokenType::P_CLOSE_PAR:
			s = "P_CLOSE_PAR";
			break;
		case TokenType::P_OPEN_BRACE:
			s = "P_OPEN_BRACE";
			break;
		case TokenType::P_CLOSE_BRACE:
			s = "P_CLOSE_BRACE";
			break;
		case TokenType::P_OPEN_BRACKET:
			s = "P_OPEN_BRACKET";
			break;
		case TokenType::P_CLOSE_BRACKET:
			s = "P_CLOSE_BRACKET";
			break;
		case TokenType::P_EQUAL:
			s = "P_EQUAL";
			break;
		case TokenType::STRING_LITERAL:
			s = "STRING_LITERAL";
			break;
		case TokenType::NUMBER:
			s = "NUMBER";
			break;
		case TokenType::DotId:
			s = "DotId";
			break;
		case TokenType::Variable:
			s = "Variable";
			break;
		case TokenType::ParameterPack:
			s = "ParameterPack";
			break;
		case TokenType::Function:
			s = "Function";
			break;
		case TokenType::Property:
			s = "Property";
			break;
		case TokenType::Class:
			s = "Class";
			break;
		case TokenType::Namespace:
			s = "Namespace";
			break;
		case TokenType::File:
			s = "File";
			break;
		}

		return os << s;
	}
}