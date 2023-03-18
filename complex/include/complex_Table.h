#pragma once


#include <functional>
#include "complex_Widget.h"


namespace complex
{
	/*
		Additional table flag. Makes table headers visible.
	*/
	const uint32_t ImGuiTableFlags_ShowHeaders = (1 << 31);

	class Table : public Widget
	{
	public: // subtypes
		using RowID	= uint32_t;
		
		class Column
		{
		public: // subtypes
			using Content = std::function<void(RowID)>;

		public: // data
			dpl::ReadOnly<std::string,				Column> name;
			dpl::ReadOnly<Content,					Column> content;
			dpl::ReadOnly<ImGuiTableColumnFlags,	Column> flags; // ImGuiTableColumnFlags_None

		public: // lifecycle
			CLASS_CTOR Column(	const std::string&			NAME,
								const Content				CONTENT,
								const ImGuiTableColumnFlags FLAGS)
				: name(NAME)
				, content(CONTENT)
				, flags(FLAGS)
			{

			}
		};

		using Columns = std::vector<Column>;

	public: // data
		dpl::ReadOnly<ImGuiTableFlags,	Table> flags;
		dpl::ReadOnly<Columns,			Table> columns;
		dpl::ReadOnly<uint32_t,			Table> numRows;

	public: // lifecycle
		CLASS_CTOR			Table(				const std::string&			NAME,
												const ImGuiTableFlags		FLAGS,
												const ImVec2&				REQUIRED_SIZE = ImVec2(0.f, 0.f))
			: Widget(NAME, REQUIRED_SIZE)
			, flags(FLAGS)
			, numRows(0)
		{

		}

	public: // functions
		inline void			add_column(			const std::string&			COLUMN_NAME,
												const Column::Content		COLUMN_CONTENT,
												const ImGuiTableColumnFlags COLUMN_FLAGS = ImGuiTableColumnFlags_None)
		{
			if(COLUMN_CONTENT)
			{
				columns->emplace_back(COLUMN_NAME, COLUMN_CONTENT, COLUMN_FLAGS);
			}
		}

		inline void			set_rows(			const uint32_t				NUM_ROWS)
		{
			*numRows	= NUM_ROWS;
		}

		void				update();
	};
}