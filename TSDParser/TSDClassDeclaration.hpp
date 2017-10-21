#ifndef NOPE_TSDCLASSDECLARATION_HPP_
# define NOPE_TSDCLASSDECLARATION_HPP_

# include "TSDClass.hpp"

namespace nope
{
	class TSDClassDeclaration
	{
	public:
		TSDClassDeclaration();
		TSDClassDeclaration(TSDClassDeclaration const &that) = delete;
		TSDClassDeclaration(TSDClassDeclaration &&that) = delete;

		TSDClassDeclaration() noexcept;

		TSDClassDeclaration &operator=(TSDClassDeclaration const &that) = delete;
		TSDClassDeclaration &operator=(TSDClassDeclaration &&that) = delete;

		void export(bool value);
		bool isExported() const;

		TSDClass &classItself();
		TSDClass const &classItself() const;

	private:
		TSDClass m_class;
		bool m_exported;
	};
}

#endif // !NOPE_TSDCLASSDECLARATION_HPP_