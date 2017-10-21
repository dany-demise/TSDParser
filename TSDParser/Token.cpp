#include "stdafx.h"

namespace nope::dts::parser
{
	Token::Token(TokenType t, std::string_view val) :
		type(t),
		value(val),
		child()
	{
	}
}