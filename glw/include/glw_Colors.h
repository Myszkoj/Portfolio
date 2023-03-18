#pragma once


#include "glw_Utilities.h"


namespace glw
{
	using RGBf	= glm::vec3;
	using RGBAf = glm::vec4;


	class RGB
	{
	public: // types
		using Palette = std::vector<RGB>;

		enum ColorCodeType
		{
			eHEX_COLOR,
			ePALETTE_HASH
		};

	public: // data
		uint8_t r;
		uint8_t g;
		uint8_t b;

	public: // functions
		CLASS_CTOR		RGB()
			: r(0)
			, g(0)
			, b(0)
		{

		}

		CLASS_CTOR		RGB(		const int32_t		ENCODED)
			: r((ENCODED & 0x000000FF) >> 0)
			, g((ENCODED & 0x0000FF00) >> 8)
			, b((ENCODED & 0x00FF0000) >> 16)
		{

		}

		CLASS_CTOR		RGB(		const uint32_t		ENCODED)
			: r((ENCODED & 0x000000FF) >> 0)
			, g((ENCODED & 0x0000FF00) >> 8)
			, b((ENCODED & 0x00FF0000) >> 16)
		{

		}

		CLASS_CTOR		RGB(		const uint8_t		R,
									const uint8_t		G,
									const uint8_t		B)
			: r(R)
			, g(G)
			, b(B)
		{

		}

		/*
			Constructs RGB object from normalized components.
		*/
		CLASS_CTOR		RGB(		const RGBf&			COLOR)
			: r(static_cast<uint8_t>(COLOR.r * 255.f))
			, g(static_cast<uint8_t>(COLOR.g * 255.f))
			, b(static_cast<uint8_t>(COLOR.b * 255.f))
		{

		}

		//CLASS_CTOR		RGB(		const std::string&	colorCode,
		//							ColorCodeType		colorCodeType);

		inline RGB&		operator=(	const RGBf&			color)
		{
			r = static_cast<uint8_t>(color.r * 255.f);
			g = static_cast<uint8_t>(color.g * 255.f);
			b = static_cast<uint8_t>(color.b * 255.f);
			return *this;
		}

		inline RGBf		normalize() const
		{
			return RGBf(static_cast<float>(r)/255.f,
						static_cast<float>(g)/255.f,
						static_cast<float>(b)/255.f);
		}

		inline uint8_t	intensity() const
		{
			uint8_t ret = r;
			if(g > ret) ret = g;
			if(b > ret) ret = b;
			return ret;
		}

		inline int32_t	to_int() const
		{
			return int32_t((r << 0) | (g << 8) | (b << 16) | 0xFF000000); 
		}

		uint32_t		to_uint() const
		{
			return uint32_t((r << 0) | (g << 8) | (b << 16) | 0xFF000000); 
		}

		std::string		to_hash() const;

		Palette			generate_palette();
	};


	class RGBA
	{
	public: // types
		using Palette = std::vector<RGBA>;

	public: // data
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;

	public: // functions
		CLASS_CTOR		RGBA()
			: r(0)
			, g(0)
			, b(0)
			, a(0)
		{

		}

		CLASS_CTOR		RGBA(				const int32_t		ENCODED)
			: r((ENCODED & 0x000000FF) >> 0)
			, g((ENCODED & 0x0000FF00) >> 8)
			, b((ENCODED & 0x00FF0000) >> 16)
			, a((ENCODED & 0xFF000000) >> 24)
		{

		}

		CLASS_CTOR		RGBA(				const uint32_t		ENCODED)
			: r((ENCODED & 0x000000FF) >> 0)
			, g((ENCODED & 0x0000FF00) >> 8)
			, b((ENCODED & 0x00FF0000) >> 16)
			, a((ENCODED & 0xFF000000) >> 24)
		{

		}

		CLASS_CTOR		RGBA(				const uint8_t		R,
											const uint8_t		G,
											const uint8_t		B,
											const uint8_t		A)
			: r(R)
			, g(G)
			, b(B)
			, a(A)
		{

		}

		CLASS_CTOR		RGBA(				const RGB			_RGB,
											const uint8_t		A)
			: r(_RGB.r)
			, g(_RGB.g)
			, b(_RGB.b)
			, a(A)
		{

		}

		/*
			Constructs RGBA object from normalized components.
		*/
		CLASS_CTOR		RGBA(				const RGBAf&		COLOR)
			: r(static_cast<uint8_t>(COLOR.r * 255.f))
			, g(static_cast<uint8_t>(COLOR.g * 255.f))
			, b(static_cast<uint8_t>(COLOR.b * 255.f))
			, a(static_cast<uint8_t>(COLOR.a * 255.f))
		{

		}

		CLASS_CTOR		RGBA(				const std::string&	HEX_COLOR);

		inline bool		operator==(			const RGBA&			other) const
		{
			return r == other.r 
				&& g == other.g 
				&& b == other.b 
				&& a == other.a;
		}

		inline bool		operator!=(			const RGBA&			other) const
		{
			return r != other.r 
				|| g != other.g 
				|| b != other.b 
				|| a != other.a;
		}

		inline RGBA&	operator=(			const RGBAf&		color)
		{
			r = static_cast<uint8_t>(color.r * 255.f);
			g = static_cast<uint8_t>(color.g * 255.f);
			b = static_cast<uint8_t>(color.b * 255.f);
			a = static_cast<uint8_t>(color.a * 255.f);
			return *this;
		}

		inline RGBAf	normalize() const
		{
			return RGBAf(	static_cast<float>(r)/255.f,
							static_cast<float>(g)/255.f,
							static_cast<float>(b)/255.f,
							static_cast<float>(a)/255.f);
		}

		inline int32_t	to_int() const
		{
			return int32_t((r << 0) | (g << 8) | (b << 16) | (a << 24)); 
		}

		uint32_t		to_uint() const
		{
			return uint32_t((r << 0) | (g << 8) | (b << 16) | (a << 24)); 
		}

		std::string		to_hash() const;

		Palette			generate_palette();
	};


	//void			fill_palette(				const std::string&	colorCode,
	//											RGB::Palette&		palette);

	unsigned long	get_nearest_palette_color(	unsigned char		r, 
												unsigned char		g, 
												unsigned char		b, 
												unsigned long		steps);
}