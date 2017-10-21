#ifndef NOPE_DTS_PARSER_ERROR_SYNTAX_HPP_
# define NOPE_DTS_PARSER_ERROR_SYNTAX_HPP_

# include <exception>
# include <string>

namespace nope::dts::parser::error
{
	class Syntax : public std::exception
	{
	public:
		Syntax(std::string const &message) noexcept;

		~Syntax() noexcept override;

		const char *what() const noexcept override;

	private:
		std::string m_message;
	};
}

#endif // !NOPE_DTS_PARSER_ERROR_SYNTAX_HPP_