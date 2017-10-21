#ifndef NOPE_TDSPROPERTY_HPP_
# define NOPE_TSDPROPERTY_HPP_

# include <string>
# include "TSDType.hpp"

namespace nope
{
	class TSDProperty
	{
	public:
		TSDProperty() = delete;
		TSDProperty(std::string const &name, TSDType const &type, bool isPublic = true, bool isStatic = false, bool isConst = false);
		TSDProperty(TSDProperty const &that) = delete;
		TSDProperty(TSDProperty &&that) = delete;

		~TSDProperty() noexcept;

		TSDProperty &operator=(TSDProperty const &that) = delete;
		TSDProperty &operator=(TSDProperty &&that) = delete;

		std::string const &name() const;
		TSDType const &type() const;
		bool isPublic() const;
		bool isStatic() const;
		bool isConst() const;

	private:
		std::string m_name;
		TSDType m_type;
		bool m_public;
		bool m_static;
		bool m_const;
	};
}

#endif // !NOPE_TDSPROPERTY_HPP_