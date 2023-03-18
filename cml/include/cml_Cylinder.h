#pragma once


#include <dpl_GeneralException.h>
#include "cml_TriangleMesh.h"


namespace cml
{
	class	Cylinder
	{
	public: // data
		dpl::ReadOnly<float, Cylinder> radius;
		dpl::ReadOnly<float, Cylinder> height;

	public: // lifecycle
		CLASS_CTOR			Cylinder()
			: radius(0.f)
			, height(0.f)
		{

		}

		CLASS_CTOR			Cylinder(					const float		RADIUS, 
														const float		HEIGHT)
			: radius(RADIUS)
			, height(HEIGHT)
		{
			validate_radius(RADIUS);
			validate_height(HEIGHT);
		}

	public: // functions
		inline void			set_radius(					const float		NEW_RADIUS)
		{
			validate_radius(NEW_RADIUS);
			radius = NEW_RADIUS;
		}

		inline void			set_height(					const float		NEW_HEIGHT)
		{
			validate_height(NEW_HEIGHT);
			height = NEW_HEIGHT;
		}

		inline float		calculate_volume() const
		{
			return height*glm::pi<float>()*radius*radius;
		}

		TriangleMesh		to_TriangleMesh(			const uint32_t	SLICES) const;

		TriangleMesh		to_TriangleMesh_with_arrow(	const uint32_t	SLICES) const;

	private: // functions
		inline void			validate_radius(			const float		RADIUS) const
		{
#ifdef _DEBUG
			if(RADIUS < 0.f)
				throw dpl::GeneralException(this, __LINE__, "Cylinder radius cannot be negative.");
#endif
		}

		inline void			validate_height(			const float		HEIGHT) const
		{
#ifdef _DEBUG
			if (HEIGHT < 0.f)
				throw dpl::GeneralException(this, __LINE__, "Cylinder height cannot be negative.");
#endif // _DEBUG
		}
	};
}