#include "stdafx.h"

namespace nope::dts::parser
{
	Tokenizer::Tokenizer(std::string const &filename) :
		m_line(1),
		m_col(1),
		m_cursor(0),
		m_input((std::istreambuf_iterator<char>(std::ifstream(filename))),
			std::istreambuf_iterator<char>())
	{
	}

	Tokenizer::~Tokenizer() noexcept
	{
	}
}