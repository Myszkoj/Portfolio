#include "..//include/complex_Table.h"


namespace complex
{
	void	Table::update()
	{
		const uint32_t NUM_COLUMNS = static_cast<uint32_t>(columns().size());
		if(NUM_COLUMNS > 0)
		{
			if (ImGui::BeginTable(name().c_str(), NUM_COLUMNS, flags, requiredSize()))
			{
				for(auto& iColumn : columns())
				{
					ImGui::TableSetupColumn(iColumn.name().c_str(), iColumn.flags());
				}

				if(flags() & ImGuiTableFlags_ShowHeaders)
				{
					ImGui::TableHeadersRow();
				}

				for(uint32_t rowID = 0; rowID < numRows(); ++rowID)
				{
					ImGui::TableNextRow();

					for(uint32_t columnID = 0; columnID < NUM_COLUMNS; ++columnID)
					{
						ImGui::TableSetColumnIndex(columnID);

						auto&	column = (*columns)[columnID];
								column.content()(rowID);
					}
				}

				ImGui::EndTable();
			}
		}
	}
}