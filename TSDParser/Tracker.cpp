#include "stdafx.h"

namespace nope
{
	Tracker::Tracker(std::string const &filename) :
		m_filename(filename),
		m_content((std::istreambuf_iterator<char>(std::ifstream(filename))),
			std::istreambuf_iterator<char>()),
		m_cursor(0),
		m_line(0),
		m_column(0)
	{
	}

	Tracker::~Tracker() noexcept
	{
	}

	std::string const &Tracker::content() const
	{
		return m_content;
	}

	Tracker &Tracker::get(char &c)
	{
		if (m_cursor + 1 < m_content.size())
		{
			c = m_content[m_cursor];
			m_cursor++;
		}

		return *this;
	}

	Tracker const &Tracker::peek(char &c) const
	{
		if (*this)
		{
			c = m_content[m_cursor];
		}
		return *this;
	}

	Tracker &Tracker::peek(char &c)
	{
		if (*this)
		{
			c = m_content[m_cursor];
		}
		return *this;
	}

	void Tracker::unget()
	{
		if (m_cursor > 0)
		{
			m_cursor--;
		}
	}

	std::pair<std::uint32_t, std::uint32_t> Tracker::position() const
	{
		std::uint32_t line = 1;
		std::uint32_t col = 1;

		for (std::int32_t i = 0; i < m_cursor; ++i)
		{
			if (m_content[i] == '\n')
			{
				line++;
				col = 1;
			}
			else
			{
				col++;
			}
		}

		return std::make_pair(line, col);
	}

	Tracker::operator bool() const
	{
		return m_cursor < m_content.size();
	}

	void Tracker::error(std::string const &message) const
	{
		// TODO: implement with exeption instead of this ugly thing

		auto [line, col] = this->position(); 

		std::cerr << m_filename << ':' << line << ':' << col
			<< " Error: " << message << '\n' << m_content.substr(m_cursor - col, col)
			<< std::string(col - 1, ' ') << '^';

		std::exit(1);
	}
}