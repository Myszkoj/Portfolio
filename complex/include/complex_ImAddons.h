#pragma once


#include <optional>
#include "complex_Utilities.h"

// Node Graph Editor:
// https://github.com/ocornut/imgui/issues/306

namespace ImGui
{
	class StyleVariable
	{
	public: // lifecycle
		CLASS_CTOR		StyleVariable(	const ImGuiStyleVar_	ID, 
										const float				VALUE)
		{
			ImGui::PushStyleVar(ID, VALUE);
		}

		CLASS_CTOR		StyleVariable(	const ImGuiStyleVar_	ID, 
										const ImVec2			VALUE)
		{
			ImGui::PushStyleVar(ID, VALUE);
		}

		CLASS_CTOR		StyleVariable(	const StyleVariable&	OTHER) = delete;

		CLASS_CTOR		StyleVariable(	StyleVariable&&			other) = delete;

		CLASS_DTOR		~StyleVariable()
		{
			ImGui::PopStyleVar();
		}

		StyleVariable&	operator=(		const StyleVariable&	OTHER) = delete;

		StyleVariable&	operator=(		StyleVariable&&			other) = delete;
	};

	class StyleColor
	{
	public: // lifecycle
		CLASS_CTOR		StyleColor(		const ImGuiCol_		ID, 
										const ImU32			COLOR)
		{
			ImGui::PushStyleColor(ID, COLOR);
		}

		CLASS_CTOR		StyleColor(		const ImGuiCol_		ID, 
										const ImVec4		COLOR)
		{
			ImGui::PushStyleColor(ID, COLOR);
		}

		CLASS_CTOR		StyleColor(		const StyleColor&	OTHER) = delete;

		CLASS_CTOR		StyleColor(		StyleColor&&		other) = delete;

		CLASS_DTOR		~StyleColor()
		{
			ImGui::PopStyleColor();
		}

		StyleColor&		operator=(		const StyleColor&	OTHER) = delete;

		StyleColor&		operator=(		StyleColor&&		other) = delete;
	};

	/*
		May disable items for the lifetime of this object.
	*/
	class DisableItems
	{
	private: // data
		dpl::ReadOnly<bool, DisableItems> bState; // If this is true all items will be disabled.

	public: // lifecycle
		CLASS_CTOR		DisableItems(	const bool			bSTATE)
			: bState(bSTATE)
		{
			if(bState)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
		}

		CLASS_CTOR		DisableItems(	const DisableItems&	OTHER) = delete;

		CLASS_CTOR		DisableItems(	DisableItems&&		other) = delete;

		CLASS_DTOR		~DisableItems()
		{
			if(bState)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}

		DisableItems&	operator=(		const DisableItems&	OTHER) = delete;

		DisableItems&	operator=(		DisableItems&&		other) = delete;
	};

	template<typename T>	struct TypeToImGuiTypeID{};
	template<>				struct TypeToImGuiTypeID<int8_t>{	static const ImGuiDataType_ ID = ImGuiDataType_S8;};
	template<>				struct TypeToImGuiTypeID<uint8_t>{	static const ImGuiDataType_ ID = ImGuiDataType_U8;};
	template<>				struct TypeToImGuiTypeID<int16_t>{	static const ImGuiDataType_ ID = ImGuiDataType_S16;};
	template<>				struct TypeToImGuiTypeID<uint16_t>{	static const ImGuiDataType_ ID = ImGuiDataType_U16;};
	template<>				struct TypeToImGuiTypeID<int32_t>{	static const ImGuiDataType_ ID = ImGuiDataType_S32;};
	template<>				struct TypeToImGuiTypeID<uint32_t>{	static const ImGuiDataType_ ID = ImGuiDataType_U32;};
	template<>				struct TypeToImGuiTypeID<float>{	static const ImGuiDataType_ ID = ImGuiDataType_Float;};
	template<>				struct TypeToImGuiTypeID<double>{	static const ImGuiDataType_ ID = ImGuiDataType_Double;};


	enum Change
	{
		ANY,
		FINAL
	};


	template<typename T>
	class ValueControl
	{
	public: // data
		dpl::ReadOnly<T,	ValueControl> initialValue; // Value before ImGui item was active.
		dpl::ReadOnly<bool,	ValueControl> bActive;		// Current state of the last ImGui item.

	public: // lifecycle
		CLASS_CTOR				ValueControl(	const T&	INITIAL_VALUE)
			: initialValue(INITIAL_VALUE)
			, bActive(false)
		{

		}

	public: // functions
		/*
			Returns bool when value is modified. Bool indicate if change is final or not.
		*/
		std::optional<Change>	update(			T&			value)
		{
			const T		PREV_VALUE = value;
			const bool	bAnyChange = on_update(value);

			if(ImGui::IsItemActive())
			{
				if(!bActive)
				{
					initialValue	= PREV_VALUE;
					bActive			= true;
				}
			}
			else if(bActive)
			{
				bActive = false;

				if(initialValue() != value)
				{
					return Change::FINAL;
				}
			}

			if(bAnyChange) return Change::ANY;
			
			return std::nullopt;
		}

	private: // functions
		/*
			Implement to update given value.
			Should return true if value was modified.
		*/
		virtual bool			on_update(		T&			value) = 0;
	};

	glm::vec3		transform_point(			const glm::mat4&	MVP,
												const glm::vec3&	POINT);

	ImVec2			world_to_screen(			const glm::vec3&	WORLD_POSITION, 
												const glm::mat4&	VIEW_PROJECTION, 
												const ImVec2		POSITION, 
												const ImVec2		SIZE);

	inline ImVec2	world_to_screen(			const glm::vec3&	WORLD_POSITION, 
												const glm::mat4&	VIEW_PROJECTION, 
												const ImRect		VIEW_SPACE)
	{
		return world_to_screen(WORLD_POSITION, VIEW_PROJECTION, VIEW_SPACE.Min, VIEW_SPACE.GetSize());
	}

	inline ImVec2	normalize(					const ImVec2&		VECTOR)
	{
		const float LENGTH = glm::length(reinterpret_cast<const glm::vec2&>(VECTOR));
		return LENGTH > 0.f ? VECTOR / LENGTH : VECTOR;
	}

	float			get_screen_space_distance(  const glm::vec3&	START, 
												const glm::vec3&	END,
												const glm::mat4&	VIEW_PROJECTION,
												const float			ASPECT_RATIO);

	inline float	GetSecondItemWidth(			const float					FIRST_ITEM_WIDTH)
	{
		return glm::max(0.f, CalcItemWidth() - FIRST_ITEM_WIDTH - GetStyle().ItemSpacing.x);
	}

	bool			Splitter(					bool						split_vertically, 
												float						thickness, 
												float*						size1, 
												float*						size2, 
												float						min_size1, 
												float						min_size2, 
												float						splitter_long_axis_size = -1.0f);

	void			TextCenter(					const std::string&			TEXT);

	void			ItemTooltip(				const char*					TEXT,
												const float					MAX_WIDTH = 128.f);

	template<typename UIntT, typename EnumT>
	inline bool		Checkbox(					const char*					LABEL,
												dpl::Mask<UIntT, EnumT>&	mask,
												const EnumT					BIT_INDEX)
	{
		bool bState = mask.at(BIT_INDEX);
		if(Checkbox(LABEL, &bState))
		{
			mask.set_at(BIT_INDEX, bState);
			return true;
		}

		return false;
	}

	inline bool		IsAnyItemHovered(			const bool					bONLY_THIS_FRAME)
	{
		return bONLY_THIS_FRAME ? GImGui->HoveredId != 0 : IsAnyItemHovered();
	}

	bool			Checkbutton(				const char*					label,
												bool*						bState,
												const ImVec2&				size_arg	= ImVec2(0.f, 0.f),
												ImGuiButtonFlags			flags		= ImGuiButtonFlags_None);

	bool			ColorEdit3(					const char*					LABEL, 
												glw::RGB&					color,
												ImGuiColorEditFlags			flags = 0);

	bool			ColorEdit4(					const char*					LABEL, 
												glw::RGBA&					color,
												ImGuiColorEditFlags			flags = 0);

	static const char*  XYZ_FORMAT[] = {"x:%.3f", "y %.3f", "z:%.3f"};
	static const char*  YPR_FORMAT[] = {"y:%.3f", "p:%.3f", "r:%.3f"};

	bool			Vec3Control(				glm::vec3&					value,
												const char*					LABEL, 
												const char*					ID,
												const char**				FORMAT,
												const ImGuiInputTextFlags_	FLAGS = ImGuiInputTextFlags_None);

	inline bool		DragUint(const char* label, uint32_t* v, float v_speed, uint32_t v_min, uint32_t v_max, const char* format, ImGuiSliderFlags flags)
	{
		return DragScalar(label, ImGuiDataType_U32, v, v_speed, &v_min, &v_max, format, flags);
	}

	inline bool		DragUint2(const char* label, uint32_t v[2], float v_speed, uint32_t v_min, uint32_t v_max, const char* format, ImGuiSliderFlags flags)
	{
		return DragScalarN(label, ImGuiDataType_U32, v, 2, v_speed, &v_min, &v_max, format, flags);
	}

	inline bool		DragUint3(const char* label, uint32_t v[3], float v_speed, uint32_t v_min, uint32_t v_max, const char* format, ImGuiSliderFlags flags)
	{
		return DragScalarN(label, ImGuiDataType_U32, v, 3, v_speed, &v_min, &v_max, format, flags);
	}

	inline bool		DragUint4(const char* label, uint32_t v[4], float v_speed, uint32_t v_min, uint32_t v_max, const char* format, ImGuiSliderFlags flags)
	{
		return DragScalarN(label, ImGuiDataType_U32, v, 4, v_speed, &v_min, &v_max, format, flags);
	}

	inline bool		InputUint(const char* label, uint32_t* v, uint32_t step, uint32_t step_fast, ImGuiInputTextFlags flags)
	{
		// Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
		const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
		return InputScalar(label, ImGuiDataType_U32, v, (step > 0) ? &step : nullptr, (step_fast > 0) ? &step_fast : nullptr, format, flags);
	}

	inline bool		InputUint2(const char* label, uint32_t v[2], ImGuiInputTextFlags flags)
	{
		return InputScalarN(label, ImGuiDataType_U32, v, 2, NULL, NULL, "%d", flags);
	}

	inline bool		InputUint3(const char* label, uint32_t v[3], ImGuiInputTextFlags flags)
	{
		return InputScalarN(label, ImGuiDataType_U32, v, 3, NULL, NULL, "%d", flags);
	}

	inline bool		InputUint4(const char* label, uint32_t v[4], ImGuiInputTextFlags flags)
	{
		return InputScalarN(label, ImGuiDataType_U32, v, 4, NULL, NULL, "%d", flags);
	}
}

// HV
namespace ImGui
{
	void			Horizontal();

	void			Vertical(			const bool		bNEW_LINE = true);
}

// group panel
namespace ImGui
{
	void			BeginGroupPanel(	const char*		NAME, 
										const ImVec2&	SIZE = ImVec2(-1.0f, -1.0f));

	void			EndGroupPanel(		const bool		bSTRETCH = false);
}

// help
namespace ImGui
{
	void			TooltipOverLastItem(const char*		desc);

	void			HelpMarker(			const char*		desc);
}

// prefixed input scalars
namespace ImGui
{
	/*
		Input scalars are initially blanc.
	*/
	const uint32_t ImGuiInputTextFlags_BlankScalar = 30;

	bool InputScalarExt(                const char*                     LABEL, 
										const ImGuiDataType             DATA_TYPE,
										void*                           p_data,
										const void*                     p_step,
										const void*                     p_step_fast, 
										const char*                     FORMAT,
										ImGuiInputTextFlags             flags = ImGuiInputTextFlags_None);

	bool PrefixedInputVector(           const char*                     LABEL,
										const ImGuiDataType             DATA_TYPE,
										void*                           p_data, 
										const uint32_t                  COMPONENTS, 
										const char**                    FORMATS,
										const float*                    WIDTH = nullptr, 
										ImGuiInputTextFlags             flags = ImGuiInputTextFlags_None);
}

// progress indicators
namespace ImGui
{
	void			BufferingBar(		const char*		label, 
										float			value,  
										const ImVec2&	size_arg, 
										const ImU32&	bg_col, 
										const ImU32&	fg_col);

	void			Spinner(			const char*		label, 
										float			radius, 
										int				thickness, 
										const ImU32&	color);

	inline void		Spinner(			const char*		label, 
										float			radius, 
										int				thickness, 
										const ImGuiCol_ COLOR_TYPE = ImGuiCol_SliderGrabActive)
	{
		Spinner(label, radius, thickness, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[COLOR_TYPE]));
	}

	void			DotSpinner(			const char*		label, 
										const float		indicator_radius, 
										const ImU32&	main_color, 
										const ImU32&	backdrop_color, 
										const int		circle_count, 
										const float		speed);
}

// ???
namespace ImGui
{
	IMGUI_API bool			SpinScaler(const char* label, ImGuiDataType data_type, void* data_ptr, const void* step, const void* step_fast, const char* format, ImGuiInputTextFlags flags);
	IMGUI_API bool			SpinInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
	IMGUI_API bool			SpinFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	IMGUI_API bool			SpinDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);
}

// sliders
namespace ImGui
{
	IMGUI_API bool          SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, float power, float thickness);
	IMGUI_API bool          SliderScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format, float power, float thickness);
	IMGUI_API bool          VSliderScalar(const char* label, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, float power, float thickness);

	IMGUI_API bool          SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, float power, float thickness);
	IMGUI_API bool          SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format, float power, float thickness);
	IMGUI_API bool          SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format, float power, float thickness);
	IMGUI_API bool          SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format, float power, float thickness);
	
	IMGUI_API bool          SliderAngle(const char* label, float* v_rad, float v_degrees_min, float v_degrees_max, const char* format, float thickness);
   
	IMGUI_API bool          SliderInt(const char* label, int* v, int v_min, int v_max, const char* format, float thickness);
	IMGUI_API bool          SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format, float thickness);
	IMGUI_API bool          SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format, float thickness);
	IMGUI_API bool          SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format, float thickness);
   
	inline bool				SliderUint(const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, const char* format, ImGuiSliderFlags flags)
	{
		return SliderScalar(label, ImGuiDataType_U32, v, &v_min, &v_max, format, flags);
	}
	inline bool				SliderUint2(const char* label, uint32_t v[2], uint32_t v_min, uint32_t v_max, const char* format, ImGuiSliderFlags flags)
	{
		return SliderScalarN(label, ImGuiDataType_U32, v, 2, &v_min, &v_max, format, flags);
	}
	inline bool				SliderUint3(const char* label, uint32_t v[3], uint32_t v_min, uint32_t v_max, const char* format, ImGuiSliderFlags flags)
	{
		return SliderScalarN(label, ImGuiDataType_U32, v, 3, &v_min, &v_max, format, flags);
	}
	inline bool				SliderUint4(const char* label, uint32_t v[4], uint32_t v_min, uint32_t v_max, const char* format, ImGuiSliderFlags flags)
	{
		return SliderScalarN(label, ImGuiDataType_U32, v, 4, &v_min, &v_max, format, flags);
	}

	IMGUI_API bool          VSliderFloat(const char* label, const ImVec2& size, float* v, float v_min, float v_max, const char* format, float power, float thickness);
	IMGUI_API bool          VSliderInt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* format, float thickness); 
}