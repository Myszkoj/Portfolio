#pragma once


#include "cml_Rectangle.h"


namespace cml
{
	class	TriangleMesh;


	class	Cuboid
	{
	public: // types
		enum Face
		{
			eLEFT,
			eRIGHT,
			eBOTTOM,
			eTOP,
			eBACK,
			eFRONT
		};

		enum Corner
		{
			eLEFT_BOTTOM_BACK,
			eLEFT_BOTTOM_FRONT,
			eLEFT_TOP_BACK,
			eLEFT_TOP_FRONT,
			eRIGHT_BOTTOM_BACK,
			eRIGHT_BOTTOM_FRONT,
			eRIGHT_TOP_BACK,
			eRIGHT_TOP_FRONT,
		};

	public: // data
		dpl::ReadOnly<Vec3, Cuboid> extents;

	public: // lifecycle
		CLASS_CTOR			Cuboid()
			: extents(0.f, 0.f, 0.f)
		{

		}

		CLASS_CTOR			Cuboid(				const float		WIDTH, 
												const float		HEIGHT, 
												const float		DEPTH)
			: extents(WIDTH/2.f, HEIGHT/2.f, DEPTH/2.f)
		{
			validate_width(WIDTH);
			validate_height(HEIGHT);
			validate_depth(DEPTH);
		}

		CLASS_CTOR			Cuboid(				const Vec3&		SIZE)
			: Cuboid(SIZE.x, SIZE.y, SIZE.z)
		{

		}

	public: // operators
		inline bool			operator==(			const Cuboid&	other) const
		{
			return this->extents() == other.extents();
		}

		inline bool			operator!=(			const Cuboid&	other) const
		{
			return this->extents() != other.extents();
		}

	public: // functions
		inline float		get_extent(			const uint32_t	AXIS_INDEX) const
		{
			return extents()[AXIS_INDEX];
		}

		inline void			set_extents(		const float		HALF_WIDTH, 
												const float		HALF_HEIGHT, 
												const float		HALF_DEPTH)
		{
			validate_width(HALF_WIDTH);
			validate_height(HALF_HEIGHT);
			validate_depth(HALF_DEPTH);

			extents->x	= HALF_WIDTH;
			extents->y	= HALF_HEIGHT;
			extents->z	= HALF_DEPTH;
		}

		inline void			set_extents(		const Vec3&		NEW_EXTENTS)
		{
			set_extents(NEW_EXTENTS.x, NEW_EXTENTS.y, NEW_EXTENTS.z);
		}

	public: // width/height/depth
		inline void			set_width(			const float		width)
		{
			validate_width(width);
			extents->x = width/2.f;
		}

		inline void			set_height(			const float		height)
		{
			validate_height(height);
			extents->y = height/2.f;
		}

		inline void			set_depth(			const float		depth)
		{
			validate_depth(depth);
			extents->z = depth/2.f;
		}

		inline float		width() const
		{
			return 2.f * halfWidth();
		}

		inline float		height() const
		{
			return 2.f * halfHeight();
		}

		inline float		depth() const
		{
			return 2.f * halfDepth();
		}

		inline float		halfWidth() const
		{
			return extents().x;
		}

		inline float		halfHeight() const
		{
			return extents().y;
		}

		inline float		halfDepth() const
		{
			return extents().z;
		}

	public: // size functions
		inline void			set_size(			const float		width, 
												const float		height, 
												const float		depth)
		{
			set_width(width);
			set_height(height);
			set_depth(depth);
		}

		inline void			set_size(			const Vec3&		size)
		{
			set_size(size.x, size.y, size.z);
		}

		inline Vec3			size() const
		{
			return Vec3(width(), height(), depth());
		}

	public: // scale functions
		inline void			scale(				const float		SCALAR)
		{
			(*extents) *= SCALAR;
		}

		inline void			scale(				const float		WIDTH_SCALAR, 
												const float		HEIGHT_SCALAR, 
												const float		DEPTH_SCALAR)
		{
			extents->x *= WIDTH_SCALAR;
			extents->y *= HEIGHT_SCALAR;
			extents->z *= DEPTH_SCALAR;
		}

		inline void			scale(				const Vec3&		FACTORS)
		{
			scale(FACTORS.x, FACTORS.y, FACTORS.z);
		}

	public: // conversion functions
		TriangleMesh		to_TriangleMesh(	const uint32_t	W_SUBDIVISIONS	= 0,
												const uint32_t	H_SUBDIVISIONS	= 0,
												const uint32_t	D_SUBDIVISIONS	= 0) const;

	public: // other functions
		inline float		range() const
		{
			return sqrt(halfWidth() * halfWidth() + halfHeight() * halfHeight() + halfDepth() * halfDepth());
		}

		inline float		volume() const
		{
			return 8.f * halfWidth() * halfHeight() * halfDepth();
		}

		inline float		surface_area() const
		{
			return 8.f * (halfWidth() * halfHeight() + halfWidth() * halfDepth() + halfHeight() * halfDepth());
		}

	private: // other functions
		inline void			validate_width(		const float		WIDTH) const
		{
#ifdef _DEBUG
			if (WIDTH < 0.f)
				throw dpl::GeneralException(this, __LINE__, "Width cannot be negative.");
#endif // _DEBUG
		}

		inline void			validate_height(	const float		HEIGHT) const
		{
#ifdef _DEBUG
			if (HEIGHT < 0.f)
				throw dpl::GeneralException(this, __LINE__, "Height cannot be negative.");
#endif // _DEBUG
		}

		inline void			validate_depth(		const float		DEPTH) const
		{
#ifdef _DEBUG
			if (DEPTH < 0.f)
				throw dpl::GeneralException(this, __LINE__, "Depth cannot be negative.");
#endif // _DEBUG

		}
	};
}