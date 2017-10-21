#ifndef NOPE_TSDCLASS_HPP_
# define NOPE_TSDCLASS_HPP_

# include <string>
# include <vector>
# include <optional>
# include "TSDMethod.hpp"
# include "TSDProperty.hpp"

namespace nope
{
	class TSDClass
	{
	public:
		TSDClass() = delete;
		TSDClass(std::string const &name);
		TSDClass(TSDClass const &that) = delete;
		TSDClass(TSDClass &&that) = delete;

		~TSDClass() noexcept;

		TSDClass &operator=(TSDClass const &that) = delete;
		TSDClass &operator=(TSDClass &&that) = delete;

		std::string const &name() const;

		std::vector<TSDMethod> const &methods() const;
		std::vector<TSDProperty> const &properties() const;

	private:
		std::string m_fullName;
		std::string m_id;
		std::vector<TSDMethod> m_method;
		std::vector<TSDProperty> m_property;
	};
}

#endif // !NOPE_TSDCLASS_HPP_