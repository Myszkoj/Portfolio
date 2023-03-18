#pragma once


#include "complex_Dockable.h"
#include "complex_Window.h"


namespace complex
{
	class Dockspace : public Dockable
					, public Window
	{
	private: // data
		dpl::ReadOnly<ImVec2,			Dockspace>	offset;
		dpl::ReadOnly<ImGuiID,			Dockspace>	dockspaceID;
		dpl::ReadOnly<bool,				Dockspace>	bNeedsRedock;

	public: // lifecycle
		CLASS_CTOR			Dockspace(			const std::string&			NAME			= "MainDockspace",
												const ImVec2&				REQUIRED_SIZE	= ImVec2(-1.f, -1.f));

		CLASS_CTOR			Dockspace(			const Window&				OTHER) = delete;

		CLASS_CTOR			Dockspace(			Dockspace&&					other) noexcept = default;

		Dockspace&			operator=(			const Dockspace&			OTHER) = delete;

		Dockspace&			operator=(			Dockspace&&					other) noexcept = default;

	public: // Widget implementation
		inline void			set_offset(			const ImVec2&				OFFSET)
		{
			offset = OFFSET;
		}

		virtual void		update() final override;

	private: // Dockable implementation
		virtual const char*	get_target_name() const final override
		{
			return name().c_str();
		}
	};
}