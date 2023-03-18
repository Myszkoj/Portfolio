#pragma once


#include "upf_Voxel.h"

#pragma pack(push, 4)
namespace upf
{
	class Box
	{
	public: // relations
		friend Block;
		friend SpatialDivision;

	public: // constants
		static const auto		DISABLED		= Voxel::FLAG_A; // Box will be skipped during pair generation.
		static const auto		HX_GREATER		= Voxel::FLAG_B;
		static const auto		HY_GREATER		= Voxel::FLAG_C;
		static const auto		V_GREATER		= Voxel::FLAG_D;
		static const auto		DIRECTION_FLAGS	= Voxel::FLAG_BCD;
		static const uint32_t	NUM_ID_BITS		= 28;
		static const uint32_t	NUM_TYPE_BITS	= 4;
		static const uint32_t	MAX_ID			= (1<<NUM_ID_BITS) - 1;
		static const uint32_t	INVALID_ID		= MAX_ID;

		using ID = dpl::RangedValue<uint32_t, 0, MAX_ID>;

	public: // data
		cml::HVPoint	center;

	private: // data
		Voxel	voxel;

	public: // functions
		inline const Voxel& get_voxel() const
		{
			return voxel;
		}

		inline void		enable()
		{
			voxel.set_flag(DISABLED, false);
		}

		inline void		disable()
		{
			voxel.set_flag(DISABLED, true);
		}

		inline bool		is_enabled() const
		{
			return !voxel.get_flag(DISABLED);
		}

		inline bool		is_disabled() const
		{
			return voxel.get_flag(DISABLED);
		}

		inline void		update_voxel(	const cml::HVSize&	BOX_SIZE,
										const cml::HVSize&	HALF_BOX_SIZE)
		{
			voxel.reset_coords(center, BOX_SIZE);
			voxel.set_flag(HX_GREATER,	Voxel::calculate_relative_position(center.h.x,	BOX_SIZE.horizontal) > HALF_BOX_SIZE.horizontal);
			voxel.set_flag(HY_GREATER,	Voxel::calculate_relative_position(center.h.y,	BOX_SIZE.horizontal) > HALF_BOX_SIZE.horizontal);
			voxel.set_flag(V_GREATER,	Voxel::calculate_relative_position(center.v,	BOX_SIZE.vertical) > HALF_BOX_SIZE.vertical);
		}
	};
}
#pragma pack(pop)