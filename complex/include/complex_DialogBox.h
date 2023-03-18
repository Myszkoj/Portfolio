#pragma once


#include "complex_PopupWindow.h"


namespace complex
{
	class DialogBox : public PopupWindow
	{
	public: // subtypes
		using	Callback	= std::function<void()>;

		struct	Option
		{
			dpl::ReadOnly<std::string,	Option> buttonText;
			dpl::ReadOnly<Callback,		Option> callback;

			CLASS_CTOR	Option(	const char*		BUTTON_TEXT,
								const Callback	ON_CLICK)
				: buttonText(BUTTON_TEXT)
				, callback(ON_CLICK)
			{

			}
		};

		using	Options		= std::vector<Option>;

		enum	Alignment
		{
			LEFT_ALIGNMENT,
			RIGHT_ALIGNMENT
		};

	public: // data
		dpl::ReadOnly<std::string,	DialogBox> message;
		dpl::ReadOnly<Options,		DialogBox> options;
		dpl::ReadOnly<Alignment,	DialogBox> alignment;
		dpl::ReadOnly<float,		DialogBox> maxButtonWidth;

	public: // lifecycle
		CLASS_CTOR		DialogBox(			const std::string&		NAME,
											const std::string&		MESSAGE,
											const float				MAX_BUTTON_WIDTH	= 200.f,
											const Alignment			ALIGNMENT			= RIGHT_ALIGNMENT);

		CLASS_CTOR		DialogBox(			const std::string&		NAME,
											const float				MAX_BUTTON_WIDTH	= 200.f,
											const Alignment			ALIGNMENT			= RIGHT_ALIGNMENT);

	public: // functions
		inline void		set_message(		const char*				NEW_MESSAGE)
		{
			*message = NEW_MESSAGE;
		}

		inline void		set_message(		const std::string&		NEW_MESSAGE)
		{
			*message = NEW_MESSAGE;
		}

		inline void		add_option(			const char*				BUTTON_TEXT,
											const Callback			ON_CLICK = nullptr)
		{
			options->emplace_back(BUTTON_TEXT, ON_CLICK);
		}
	};
}