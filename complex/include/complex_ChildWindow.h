#pragma once


#include "complex_Dockable.h"
#include "complex_Window.h"


namespace complex
{
	class ChildWindow	: public Dockable
						, public Window
	{
	protected: // lifecycle
		CLASS_CTOR			ChildWindow(		const std::string&			NAME,
												const ImVec2&				REQUIRED_SIZE = ImVec2(-1.f, -1.f));

		CLASS_CTOR			ChildWindow(		const ChildWindow&			OTHER) = delete;

		CLASS_CTOR			ChildWindow(		ChildWindow&&				other) noexcept = default;

		ChildWindow&		operator=(			const ChildWindow&			OTHER) = delete;

		ChildWindow&		operator=(			ChildWindow&&				other) noexcept = default;

	public: // Window implementation
		virtual void		update() override;

	private: // Dockable implementation
		virtual const char*	get_target_name() const final override
		{
			return Window::name().c_str();
		}
	};
}