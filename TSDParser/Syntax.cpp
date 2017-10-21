#include "stdafx.h"

namespace nope::dts::parser::error
{
	Syntax::Syntax(std::string const &message) noexcept :
		m_message(message)
	{
	}

	Syntax::~Syntax() noexcept
	{
	}

	const char *Syntax::what() const noexcept
	{
		return m_message.c_str();
	}
}