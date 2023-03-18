#pragma once


#include "complex_Window.h"


namespace complex
{
	class PopupWindow : public Window
	{
	public: // subtypes
		enum Type
		{
			CONTEXT,
			MODAL
		};

		enum State
		{
			CLOSED,
			OPEN_REQUESTED,
			OPEN
		};

	private: // data
		dpl::ReadOnly<Type,		PopupWindow> type;
		dpl::ReadOnly<State,	PopupWindow> state;

	protected: // lifecycle
		CLASS_CTOR			PopupWindow(		const Type					TYPE,
												const std::string&			NAME,
												const ImVec2&				REQUIRED_SIZE = ImVec2(-1.f, -1.f));

	public: // functions
		inline bool			is_open() const
		{
			return state() == State::OPEN;
		}

		inline bool			reopen_over_item(	const ImGuiPopupFlags		FLAGS = ImGuiPopupFlags_MouseButtonRight)
		{
			return ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) ? reopen(FLAGS) : false;
		}

		/*
			Opens popup.
			Returns false if popup was already open.
		*/
		bool				reopen(				const ImGuiPopupFlags		FLAGS = ImGuiPopupFlags_MouseButtonRight);

	public: // implementation
		virtual void		open() final override
		{
			if(state == State::CLOSED) state = State::OPEN_REQUESTED;
			Window::open();
		}

		virtual void		close() final override
		{
			if(state == State::OPEN_REQUESTED) state = State::CLOSED;
			Window::close();
		}

		virtual void		update() override;

	private: // interface
		virtual void		on_closed(){}
	};
}