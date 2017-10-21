#include "stdafx.h"

namespace nope
{
	TSDFile::TSDFile(std::string const &name) :
		m_tracker(name),
		m_module()
	{
	}

	TSDFile::~TSDFile() noexcept
	{
	}

	std::string TSDFile::json() const
	{
		return "THIS IS NOT IMPLEMENTED";
	}

	void TSDFile::parse()
	{
		char c;

		while (m_tracker.peek(c))
		{
			// Parse comment
			if (c == '/')
			{ 
				TSDComment::parse(m_tracker);
			}
			// Parse module
			else if (std::isalpha(c))
			{
				m_module.emplace_back(m_tracker);
			}
			// If not space, generate an error
			else if (std::isspace(c) == false)
			{
				m_tracker.error(std::string("Unexpected '") + c + "'");
			}
		}
	}
}