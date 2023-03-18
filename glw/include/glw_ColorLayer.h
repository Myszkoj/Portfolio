#pragma once


#include <variant>
#include <dpl_NamedType.h>
#include "glw_FragmentLayer.h"
#include "glw_Colors.h"
#include "glw_SLTransfers.h"


namespace glw
{
	template<typename glslT, typename T>
	class ColorLayer;


	class ColorComponent
	{
	public: // relations
		template<typename, typename> 
		friend class ColorLayer;

	private: // data
		static dpl::ReadOnly<uint32_t, ColorComponent> numTypes;

	private: // functions
		static uint32_t	register_component()
		{			
			//GLint MAX_IMAGE_TYPES = 0; 
			//glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MAX_IMAGE_TYPES);
			//if(numTypes() >= MAX_IMAGE_TYPES)
			//	throw Crash(__FILE__, __LINE__, "Too many image layer types. Max supported image layers: "  + std::to_string(MAX_IMAGE_TYPES));
				
			return (*numTypes)++;
		}
	};


	template<typename glsl_T, typename T>
	class ColorLayer	: public FragmentLayer
						, public ColorComponent
	{
	public: // relations
		friend	FrameBuffer;

	public: // subtypes
		enum	ClearType
		{
			INTEGER_CLEAR,
			UNSIGNED_INTEGER_CLEAR,
			FLOAT_CLEAR
		};

		using	glslT		= glsl_T;
		using	DataT		= typename glslT::DataT;
		using	ClearColor	= std::variant<std::monostate, glm::tvec4<int32_t>, glm::tvec4<uint32_t>, glm::tvec4<float>>;
		using	Translator	= typename glslT::Format;
		using	Binding		= FragmentLayer::Binding;

	public: // constants
		static const GLenum GL_FORMAT		= Translator::FORMAT;
		static const GLenum GL_COMPONENT	= Translator::BASE;
		static const GLenum GL_DATA_TYPE	= Translator::GL;

	public: // Attachments
		static FragData<glslT>	BUFFER;

	public: // data
		dpl::ReadOnly<ClearType,	ColorLayer> clearType;
		dpl::ReadOnly<ClearColor,	ColorLayer> clearColor;
			
	public: // lifecycle
		CLASS_CTOR		ColorLayer(			const Binding&				BINDING,
											const GLenum				INTERNAL_FORMAT)
			: FragmentLayer(BINDING, static_cast<Attachment>(GL_COLOR_ATTACHMENT0 + BUFFER.location()), INTERNAL_FORMAT)
			, clearType(ClearType::FLOAT_CLEAR)
		{
			if(isSignedIntColor(internalFormat()))
			{
				clearColor->emplace<IVec4>(0, 0, 0, 0);
				clearType = ClearType::INTEGER_CLEAR;
			}
			else if(isUnsignedIntColor(internalFormat()))
			{
				clearColor->emplace<UVec4>(0u, 0u, 0u, 0u);
				clearType = ClearType::UNSIGNED_INTEGER_CLEAR;
			}
			else if(isFloatColor(internalFormat()) || isUNormColor(internalFormat()))
			{
				clearColor->emplace<Vec4>(0.f, 0.f, 0.f, 0.f);
				clearType = ClearType::FLOAT_CLEAR;
			}
			else
			{
				throw dpl::GeneralException(this, __LINE__, "Signed, normalized colors are not supported yet.");
			}
		}

	public: // functions
		inline void		set_clear_color(	const glm::tvec4<int32_t>&	NEW_COLOR)
		{
			access_clear_color<glm::tvec4<int32_t>>() = NEW_COLOR;
		}

		inline void		set_clear_color(	const glm::tvec4<uint32_t>&	NEW_COLOR)
		{
			access_clear_color<glm::tvec4<uint32_t>>() = NEW_COLOR;
		}

		inline void		set_clear_color(	const glm::tvec4<float>&	NEW_COLOR)
		{
			access_clear_color<glm::tvec4<float>>() = NEW_COLOR;
		}

		inline void		set_clear_color(	const RGBA&					NEW_COLOR)
		{
			access_clear_color<glm::tvec4<float>>() = NEW_COLOR.normalize();
		}

	private: // functions
		template<typename T>
		inline T&		access_clear_color()
		{
			if(!std::holds_alternative<T>(clearColor()))
				throw dpl::GeneralException(this, __LINE__, "Invalid clear color.");

			return std::get<T>(*clearColor);
		}

	private: // Layer implementation
		virtual void	clear() final override
		{
			switch(clearType())
			{
			case ClearType::INTEGER_CLEAR:			glClearBufferiv(GL_COLOR,	BUFFER.location(), &std::get<IVec4>(clearColor())[0]);	break;
			case ClearType::UNSIGNED_INTEGER_CLEAR:	glClearBufferuiv(GL_COLOR,	BUFFER.location(), &std::get<UVec4>(clearColor())[0]);	break;
			case ClearType::FLOAT_CLEAR:			glClearBufferfv(GL_COLOR,	BUFFER.location(), &std::get<Vec4>(clearColor())[0]);	break;

			default: throw Crash(this, __LINE__, "Invalid clear type.");
			}
		}
	};


	template<typename glslT, typename T>
	FragData<glslT>		ColorLayer<glslT, T>::BUFFER(ColorComponent::register_component(), dpl::undecorate_type_name<T>());
}