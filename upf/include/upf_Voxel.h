#pragma once


#include "upf_utilities.h"

#pragma warning( disable : 26812 ) // unscoped enum

namespace upf
{
	class	Voxel
	{
	public: // constants
		static const uint32_t	NUM_COORDINATE_BITS	= 20;
		static const uint32_t	NUM_USER_FLAGS		= 4;	// Number of flags reserved for the user.
		static const uint32_t	NUM_VOXELS_PER_AXIS	= (1 << NUM_COORDINATE_BITS);
		static const int32_t	MAX_COORD			= NUM_VOXELS_PER_AXIS - 1;
		static const uint32_t	ORIGIN_OFFSET		= NUM_VOXELS_PER_AXIS / 2u; // Offset to the coordinate that contains center of the point(0, 0, 0).
		static const uint32_t	VALID_COORDS_MIN	= 1;
		static const uint32_t	VALID_COORDS_MAX	= MAX_COORD-1;
		static const Voxel		NULL_VOXEL; // Has all coordinates set to MAX_COORD and all flags to false.

	public: // subtypes
		enum	Flags
		{
			FLAG_A = (1<<0),
			FLAG_B = (1<<1),
			FLAG_C = (1<<2),
			FLAG_D = (1<<3)
		};

		enum	FlagCombination
		{
			FLAG_AB = FLAG_A | FLAG_B,
			FLAG_AC = FLAG_A | FLAG_C,
			FLAG_AD = FLAG_A | FLAG_D,
			FLAG_BC = FLAG_B | FLAG_C,
			FLAG_BD = FLAG_B | FLAG_D,
			FLAG_CD = FLAG_C | FLAG_D,

			FLAG_ABC = FLAG_A | FLAG_B | FLAG_C,
			FLAG_ACD = FLAG_A | FLAG_C | FLAG_D,
			FLAG_ABD = FLAG_A | FLAG_B | FLAG_D,
			FLAG_BCD = FLAG_B | FLAG_C | FLAG_D,

			FLAG_ABCD = FLAG_A | FLAG_B | FLAG_C | FLAG_D
		};

		using	Size = cml::HVSize;

		enum	Coordinate
		{
			HX,
			HY,
			V
		};

		enum	Operation
		{
			MIN_OF,
			MAX_OF
		};

	public: // data
		uint64_t hx		: NUM_COORDINATE_BITS;
		uint64_t hy		: NUM_COORDINATE_BITS;
		uint64_t v		: NUM_COORDINATE_BITS;
		uint64_t flags	: NUM_USER_FLAGS;

	public: // lifecycle
		CLASS_CTOR				Voxel()
			: hx(0)
			, hy(0)
			, v(0)
			, flags(0)
		{
				
		}

		CLASS_CTOR				Voxel(						const uint32_t					HORIZONTAL_X,
															const uint32_t					HORIZONTAL_Y,
															const uint32_t					VERTICAL)
			: hx(HORIZONTAL_X)
			, hy(HORIZONTAL_Y)
			, v(VERTICAL)
			, flags(0)
		{
			validate_coordinate(HORIZONTAL_X);
			validate_coordinate(HORIZONTAL_Y);
			validate_coordinate(VERTICAL);
		}

		CLASS_CTOR				Voxel(						const cml::HVPoint&				PRECISE_POSITION,
															const Size&						VOXEL_SIZE)
			: hx(from_precise(PRECISE_POSITION.h.x, VOXEL_SIZE.horizontal))
			, hy(from_precise(PRECISE_POSITION.h.y, VOXEL_SIZE.horizontal))
			, v(from_precise(PRECISE_POSITION.v, VOXEL_SIZE.vertical))
			, flags(0)
		{

		}

		CLASS_CTOR				Voxel(						const Operation					OPERATION,
															const Voxel*					VOXELS,
															const uint32_t					NUM_VOXELS)
			: Voxel()
		{
			(OPERATION == MIN_OF) ? min_of(VOXELS, NUM_VOXELS) : max_of(VOXELS, NUM_VOXELS);
		}

	public: // operators
		inline operator			const uint64_t() const
		{
			return reinterpret_cast<const uint64_t&>(*this);
		}

		inline bool				operator==(					const Voxel&					OTHER) const
		{
			return reinterpret_cast<const uint64_t&>(*this) == reinterpret_cast<const uint64_t&>(OTHER);
		}
		
		inline bool				operator!=(					const Voxel&					OTHER) const
		{
			return reinterpret_cast<const uint64_t&>(*this) != reinterpret_cast<const uint64_t&>(OTHER);
		}

	public: // coordinates functions
		inline uint64_t			to_coords_mask() const
		{
			return static_cast<const uint64_t&>(*this) & NULL_VOXEL;//  (uint64_t(v) << 40) | (uint64_t(hy) << 20) | uint64_t(hx);
		}

		inline void				copy_coords(				const Voxel&					OTHER)
		{
			hx	= OTHER.hx;
			hy	= OTHER.hy;
			v	= OTHER.v;
		}

		inline void				set_Horizontal(				const glm::vec2&				PRECISE_POSITION,
															const Size&						VOXEL_SIZE)
		{
			hx = from_precise(PRECISE_POSITION.x, VOXEL_SIZE.horizontal);
			hy = from_precise(PRECISE_POSITION.y, VOXEL_SIZE.horizontal);
		}

		inline void				set_Vertical(				const float						PRECISE_POSITION,
															const Size&						VOXEL_SIZE)
		{
			v = from_precise(PRECISE_POSITION, VOXEL_SIZE.vertical);
		}

		inline void				reset_coords(				const cml::HVPoint&				PRECISE,
															const Size&						VOXEL_SIZE)
		{
			set_Horizontal(PRECISE.h,	VOXEL_SIZE);
			set_Vertical(PRECISE.v,		VOXEL_SIZE);
		}

		inline void				minimize(					const Voxel&					OTHER)
		{
			if(OTHER.hx < hx) hx = OTHER.hx;
			if(OTHER.hy < hy) hy = OTHER.hy;
			if(OTHER.v < v) v = OTHER.v;
		}

		inline void				maximize(					const Voxel&					OTHER)
		{
			if(OTHER.hx > hx) hx = OTHER.hx;
			if(OTHER.hy > hy) hy = OTHER.hy;
			if(OTHER.v > v) v = OTHER.v;
		}

		inline void				min_of(						const Voxel*					VOXELS,
															const uint32_t					NUM_VOXELS)
		{
			*this = VOXELS[0];
			for(uint32_t index = 1; index < NUM_VOXELS; ++index)
			{
				minimize(VOXELS[index]);
			}
		}

		inline void				max_of(						const Voxel*					VOXELS,
															const uint32_t					NUM_VOXELS)
		{
			*this = VOXELS[0];
			for(uint32_t index = 1; index < NUM_VOXELS; ++index)
			{
				maximize(VOXELS[index]);
			}
		}

		inline cml::HVPoint		to_HVPoint(					const Size&						VOXEL_SIZE) const
		{
			return cml::HVPoint(to_precise(hx,	VOXEL_SIZE.horizontal),
								to_precise(hy,	VOXEL_SIZE.horizontal),
								to_precise(v,	VOXEL_SIZE.vertical));
		}

		inline void				clamp(						const uint32_t					MIN_VALUE,
															const uint32_t					MAX_VALUE)
		{
			hx	= glm::clamp(uint32_t(hx),	MIN_VALUE, MAX_VALUE);
			hy	= glm::clamp(uint32_t(hy),	MIN_VALUE, MAX_VALUE);
			v	= glm::clamp(uint32_t(v),	MIN_VALUE, MAX_VALUE);
		}

		inline uint32_t			get_coord(					const Coordinate				COORDINATE) const
		{
			switch(COORDINATE)
			{
			case HX:	return hx;
			case HY:	return hy;
			case V:		return v;

#ifdef _DEBUG
			default: throw dpl::GeneralException(this, __LINE__, "Invalid COORDINATE");
#endif // !_DEBUG
			}
		}

	public: // flags functions
		inline void				set_flag(					const Flags						FLAG,
															const bool						bSTATE)
		{
			bSTATE	? flags |= FLAG 
					: flags &= ~FLAG;
		}

		inline void				set_flags(					const FlagCombination			COMBINATION,
															const bool						bSTATE)
		{
			bSTATE	? flags |= COMBINATION 
					: flags &= ~COMBINATION;
		}

		inline void				set_flags(					const Voxel&					OTHER)
		{
			flags = OTHER.flags;
		}

		inline bool				get_flag(					const Flags						FLAG) const
		{
			return (flags & FLAG) == FLAG;
		}

		inline bool				has_flag_combination(		const FlagCombination			COMBINATION) const
		{
			return (flags & COMBINATION) == COMBINATION;
		}

		inline bool				has_any_flag_of(			const FlagCombination			COMBINATION) const
		{
			return (flags & COMBINATION) > 0;
		}

		inline bool				same_flag_combination(		const Voxel&					OTHER,
															const Flags						COMBINATION) const
		{
			return (flags & COMBINATION) == (OTHER.flags & COMBINATION);
		}

		inline bool				same_flag_combination(		const Voxel&					OTHER,
															const FlagCombination			COMBINATION = FLAG_ABCD) const
		{
			return (flags & COMBINATION) == (OTHER.flags & COMBINATION);
		}

	public: // helpers
		static inline float		calculate_relative_position(const float						POSITION,
															const float						STRIDE)
		{
			return wrap_value(fmod(POSITION, STRIDE), STRIDE);
		}

	private: // functions
		inline void				validate_coordinate(		const uint32_t					COORDINATE) const
		{
#ifdef _DEBUG
			if(COORDINATE > MAX_COORD)
				throw dpl::GeneralException(this, __LINE__, "Invalid coordinate.");
#endif // _DEBUG
		}

		inline static float		to_precise(					const uint32_t					VOXEL_COORDS,
															const float						STRIDE)
		{
			return VOXEL_COORDS * STRIDE - ORIGIN_OFFSET * STRIDE;
		}

		inline static uint32_t	from_precise(				const float						PRECISE_COORD,
															const float						STRIDE)
		{
			const int32_t ORIGIN = int32_t(PRECISE_COORD / STRIDE);
			return glm::clamp(ORIGIN_OFFSET + ((PRECISE_COORD < 0.f)? ORIGIN-1 : ORIGIN), VALID_COORDS_MIN, VALID_COORDS_MAX);
		}

		static inline float		wrap_value(					const float						VALUE,
															const float						RANGE)
		{
			return (VALUE < 0.f) ? VALUE + RANGE : VALUE;
		}
	};
}


namespace std 
{
	template<>
	struct hash<upf::Voxel>
	{
		inline std::size_t operator()(const upf::Voxel& KEY) const
		{
			return hash<uint64_t>()(KEY);
		}
	};
}