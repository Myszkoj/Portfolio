#include "..//include/glw_Colors.h"
#include <sstream>

#pragma warning( disable : 26451)
#pragma warning( disable : 26812)

namespace glw
{
	inline std::string	get_colorCode_info()
	{
		return "Invalid palette hash code. Palette hash: P#RxGyBz, where x, y and z represent number of different values in given color channel.";
	}



	CLASS_CTOR			/*RGB::RGB(	const std::string&	colorCode,
									ColorCodeType		colorCodeType)
		: r(0), g(0), b(0)
	{
		if(colorCodeType == ColorCodeType::eHEX_COLOR)
		{
			uint32_t hexValue;

			std::stringstream ss;
			ss << std::hex << colorCode;			
			if(ss.fail())
				throw dpl::GeneralException(this, __LINE__, "Invalid hex value: " + colorCode);

			ss >> hexValue;
			if(ss.fail())
				throw dpl::GeneralException(this, __LINE__, "Unable to convert hex value: " + colorCode);

			r = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
			g = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
			b = ((hexValue) & 0xFF);        // Extract the BB byte
		}
		else
		{
			if(colorCode.size() < 8)
				throw dpl::GeneralException(this, __LINE__, get_colorCode_info());

			uint32_t Rlevels = 0;
			uint32_t Glevels = 0;
			uint32_t Blevels = 0;

			Parser parser(colorCode.data() + 2, colorCode.data() + colorCode.size() -1);

			{// Red
				if(*parser.carriage() != 'R')
					throw dpl::GeneralException(this, __LINE__, get_colorCode_info());

				if(!parser.skip_char())
					throw dpl::GeneralException(this, __LINE__, get_colorCode_info());

				if(!parser.parse_while('G'))
					throw dpl::GeneralException(this, __LINE__, get_colorCode_info());

				std::stringstream ss(parser.string());

				if((ss >> Rlevels).fail())
					throw GeneralException(this, __LINE__, get_colorCode_info());

				if(Rlevels > 255)
					throw GeneralException(this, __LINE__, "Invalid palette hash code. Too many steps: " + std::to_string(Rlevels));
			}

			{// Green
				if(!parser.skip_char())
					throw GeneralException(this, __LINE__, get_colorCode_info());

				if(!parser.parse_while('B'))
					throw GeneralException(this, __LINE__, get_colorCode_info());

				std::stringstream ss(parser.string());

				if((ss >> Glevels).fail())
					throw GeneralException(this, __LINE__, get_colorCode_info());

				if(Glevels > 255)
					throw GeneralException(this, __LINE__, "Invalid palette hash code. Too many steps: " + std::to_string(Glevels));
			}

			{// Blue
				if(!parser.skip_char())
					throw GeneralException(this, __LINE__, get_colorCode_info());

				if(!parser.parse_rest())
					throw GeneralException(this, __LINE__, get_colorCode_info());

				std::stringstream ss(parser.string());

				if((ss >> Blevels).fail())
					throw GeneralException(this, __LINE__, get_colorCode_info());

				if(Blevels > 255)
					throw GeneralException(this, __LINE__, "Invalid palette hash code. Too many steps: " + std::to_string(Blevels));
			}

			r = Rlevels;
			g = Glevels;
			b = Blevels;
		}
	}
	*/

	std::string			RGB::to_hash() const
	{
		uint32_t num = uint32_t(r) << 16 | uint32_t(g) << 8 | uint32_t(b);

		std::stringstream stream;
		stream << std::hex << num;
		return std::string( stream.str() );
	}

	RGB::Palette		RGB::generate_palette()
	{
		if (r < 2 || g < 2 || b < 2)
			throw dpl::GeneralException(__LINE__, "Fail to generate RGB palette. You cannot set step value below 2.");

		Palette palette(r*g*b);

		uint32_t Rdivisor = r - 1;
		uint32_t Gdivisor = g - 1;
		uint32_t Bdivisor = b - 1;

		uint32_t Rs = 255 / Rdivisor;
		uint32_t Gs = 255 / Gdivisor;
		uint32_t Bs = 255 / Bdivisor;

		RGB* current = palette.data();

		for (uint32_t RID = 0; RID < r; ++RID)
		{			
			for (uint32_t GID = 0; GID < g; ++GID)
			{
				for (uint32_t BID = 0; BID < b; ++BID)
				{
					current->r = static_cast<uint8_t>((RID + 1 == r) ? 255 : RID * Rs);
					current->g = static_cast<uint8_t>((GID + 1 == g) ? 255 : GID * Gs);
					current->b = static_cast<uint8_t>((BID + 1 == b) ? 255 : BID * Bs);

					++current;
				}
			}
		}

		return palette;
	}



	CLASS_CTOR			RGBA::RGBA(				const std::string&	hexColor)
		: r(0), g(0), b(0), a(0)
	{
		uint32_t hexValue;

		std::stringstream ss;
		ss << std::hex << hexColor;			
		if(ss.fail())
			throw dpl::GeneralException(this, __LINE__, "Invalid hex value: " + hexColor);

		ss >> hexValue;
		if(ss.fail())
			throw dpl::GeneralException(this, __LINE__, "Unable to convert hex value: " + hexColor);

		r = ((hexValue >> 24) & 0xFF);  // Extract the RR byte
		g = ((hexValue >> 16) & 0xFF);  // Extract the GG byte
		b = ((hexValue >> 8) & 0xFF);   // Extract the BB byte
		a = ((hexValue) & 0xFF);		// Extract the AA byte
	}

	std::string			RGBA::to_hash() const
	{
		uint32_t num = uint32_t(r) << 24 | uint32_t(g) << 16 | uint32_t(b) << 8 | uint32_t(a);

		std::stringstream stream;
		stream << "0x" << std::hex << num;
		return std::string( stream.str() );
	}

	RGBA::Palette		RGBA::generate_palette()
	{
		if (r < 2 || g < 2 || b < 2)
			throw dpl::GeneralException(__LINE__, "Fail to generate RGB palette. You cannot set step value below 2.");

		Palette palette(r*g*b);

		uint32_t Rdivisor = r - 1;
		uint32_t Gdivisor = g - 1;
		uint32_t Bdivisor = b - 1;

		uint32_t Rs = 255 / Rdivisor;
		uint32_t Gs = 255 / Gdivisor;
		uint32_t Bs = 255 / Bdivisor;

		RGBA* current = palette.data();

		for (uint32_t RID = 0; RID < r; ++RID)
		{			
			for (uint32_t GID = 0; GID < g; ++GID)
			{
				for (uint32_t BID = 0; BID < b; ++BID)
				{
					current->r = static_cast<uint8_t>((RID + 1 == r) ? 255 : RID * Rs);
					current->g = static_cast<uint8_t>((GID + 1 == g) ? 255 : GID * Gs);
					current->b = static_cast<uint8_t>((BID + 1 == b) ? 255 : BID * Bs);
					current->a = a;

					++current;
				}
			}
		}

		return palette;
	}


	/*
	void				fill_palette(				const std::string&	colorCode,
													RGB::Palette&		palette)
	{
		if(colorCode[0] == 'P')
		{
			RGB::Palette tmp = RGB(colorCode, RGB::ePALETTE_HASH).generate_palette();

			palette.insert(palette.end(), tmp.begin(), tmp.end());
		}
		else if(colorCode[0] == '0')
		{
			palette.push_back(RGB(colorCode, RGB::eHEX_COLOR));
		}
		else
		{
			throw GeneralException(__FILE__, __LINE__, "Unknown color code: " + colorCode + " \n" + get_colorCode_info());
		}
	}
	*/

	unsigned long		get_nearest_palette_color(	unsigned char		r,
													unsigned char		g, 
													unsigned char		b, 
													unsigned long		steps)
	{
		unsigned long divisor = steps - 1;
		unsigned long S = 255 / divisor;
		unsigned long halfS = S / 2;

		unsigned long rs = (r == 255) ? divisor : (static_cast<long>(r) + halfS) / S;
		unsigned long gs = (g == 255) ? divisor : (static_cast<long>(g) + halfS) / S;
		unsigned long bs = (b == 255) ? divisor : (static_cast<long>(b) + halfS) / S;

		return rs * steps * steps + gs * steps + bs;
	}
}