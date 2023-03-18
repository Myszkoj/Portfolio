#pragma once


#include <string>
#include <optional>
#include <dpl_ReadOnly.h>
#include "cml_Ray.h"


namespace cml
{
	class Ray;
	class AABB;
	class OBB;
	class Sphere;
	class Cone;
	class Plane;
	class ConvexHull;



	/*
		https://graphics.stanford.edu/~mdfisher/Code/Engine/Plane.cpp.html
	*/
	class	Plane
	{
	public: // subtypes
		class LeastSquaresFitting
		{
		private: // data
			glm::vec3	m_sum	= glm::vec3(0.f, 0.f, 0.f);
			glm::vec2	m_sum2	= Vec2(0.f, 0.f);	
			float		m_xy	= 0.f;		
			float		m_yz	= 0.f;
			float		m_zx	= 0.f;
			uint32_t	m_count	= 0;

		public: // functions
			void					reset();

			void					add_point(	const glm::vec3& POINT);

			std::optional<Plane>	build_plane() const;

		private: // functions
			inline float			calculate_denom() const
			{
				return glm::determinant(glm::mat3(	m_sum2.x,	m_xy,		m_sum.x, 
													m_xy,		m_sum2.y,	m_sum.y, 
													m_sum.x,	m_sum.y,	float(m_count)));
			}

			inline float			calculate_nomP() const
			{
				return glm::determinant(glm::mat3(	m_zx,		m_xy,		m_sum.x, 
													m_yz,		m_sum2.y,	m_sum.y, 
													m_sum.z,	m_sum.y,	float(m_count)));
			}

			inline float			calculate_nomQ() const
			{
				return glm::determinant(glm::mat3(	m_sum2.x,	m_zx,		m_sum.x, 
													m_xy,		m_yz,		m_sum.y, 
													m_sum.x,	m_sum.z,	float(m_count)));
			}

			inline float			calculate_nomR() const
			{
				return glm::determinant(glm::mat3(	m_sum2.x,	m_xy,		m_zx, 
													m_xy,		m_sum2.y,	m_yz, 
													m_sum.x,	m_sum.y,	m_sum.z));
			}
		};

	public: // data
		dpl::ReadOnly<glm::vec3,	Plane>	normal;
		dpl::ReadOnly<float,		Plane>	distance;

	public: // lifecycle
		CLASS_CTOR						Plane()
			: normal(0.f, 0.f, 1.f)
			, distance(0.f)
		{

		}

		CLASS_CTOR						Plane(					const glm::vec3&		NORMAL, 
																const float				DISTANCE = 0.f)
			: normal(NORMAL)
			, distance(DISTANCE)
		{

		}

		CLASS_CTOR						Plane(					const glm::vec3&		ORIGIN, 
																const glm::vec3&		NORMAL)
			: normal(NORMAL)
			, distance(glm::dot(ORIGIN, NORMAL))
		{

		}

		// Generates plane from triangle with vertices in CCW order.
		CLASS_CTOR						Plane(					const glm::vec3&		POINT_A, 
																const glm::vec3&		POINT_B,
																const glm::vec3&		POINT_C)
			: normal(calculate_normal(POINT_A, POINT_B, POINT_C))
			, distance(calculate_dot(POINT_A, normal))
		{

		}

	public: // operators
		inline bool						operator==(				const Plane&			OTHER) const
		{
			return normal == OTHER.normal && distance == OTHER.distance;
		}

		inline bool						operator!=(				const Plane&			OTHER) const
		{
			return normal != OTHER.normal || distance != OTHER.distance;
		}

	public: // functions
		inline bool						is_similar(				const Plane&			OTHER,
																const float				THRESHOLD = 0.00001f) const
		{
			if(abs(glm::dot(normal(), OTHER.normal()) - 1.f) > THRESHOLD) return false;
			if(abs(distance() - OTHER.distance()) > THRESHOLD) return false;
			return true;
		}

		inline void						reset(					const glm::vec3&		NEW_NORMAL, 
																const float				NEW_DISTANCE)
		{
			set_normal(NEW_NORMAL);
			set_distance(NEW_DISTANCE);
		}

		inline void						reset(					const glm::vec3&		NEW_ORIGIN, 
																const glm::vec3&		NEW_NORMAL)
		{
			*normal		= NEW_NORMAL;
			*distance	= glm::dot(NEW_ORIGIN, NEW_NORMAL);
		}

		inline void						reset(					const glm::vec3&		POINT_A, 
																const glm::vec3&		POINT_B,
																const glm::vec3&		POINT_C)
		{
			*normal		= calculate_normal(POINT_A, POINT_B, POINT_C);
			*distance	= calculate_dot(POINT_A, normal());
		}

		inline void						set_normal(				const glm::vec3&		NEW_NORMAL)
		{
			*normal	= NEW_NORMAL;
		}

		inline void						set_distance(			float					NEW_DISTANCE)
		{
			this->distance = NEW_DISTANCE;
		}

		inline void						invert()
		{
			*normal *= -1.f;
		}

		inline void						move(					const glm::vec3&		dS)
		{
			*distance += calculate_dot(normal(), dS);
		}

		inline void						rotate(					const glm::vec3&		NEW_NORMAL)
		{
			const glm::vec3 V = normal() * distance();

			this->normal	= NEW_NORMAL;
			this->distance	= calculate_dot(NEW_NORMAL, V);
		}

		inline glm::vec3				calculate_offset() const
		{
			return normal() * distance();
		}

		void							set_average(			const Plane*			OTHER_PLANES,
																const uint32_t			NUM_PLANES);

		std::string						get_info() const;

	public: // tests
		/*
			Returns signed distance from the plane.
		*/
		inline float					point_distance(			const glm::vec3&		point) const
		{
			return calculate_dot(point, normal()) - distance();
		}

		inline glm::vec3				project_point(			const glm::vec3&		point) const
		{
			return point - point_distance(point) * normal();
		}

		inline bool						point_above(			const glm::vec3&		point) const
		{
			return point_distance(point) > 0.f;
		}

		inline bool						point_below(			const glm::vec3&		point) const
		{
			return point_distance(point) < 0.f;
		}

		std::optional<glm::vec3>		get_intersection(		const glm::vec3&		RAY_ORIGIN,
																const glm::vec3&		RAY_DIRECTION) const;

		inline std::optional<glm::vec3>	get_intersection(		const Ray&				RAY) const
		{
			return get_intersection(RAY.origin(), RAY.direction());
		}

		bool							intersects(				const Ray&				RAY) const;

		bool							intersects(				const glm::vec3&		point) const;

		bool							intersects(				const AABB&				aabb) const;

		bool							intersects(				const OBB&				obb) const;

		bool							intersects(				const Sphere&			sphere) const;

		bool							intersects(				const Cone&				other) const;

		bool							intersects(				const Plane&			other) const;

		bool							intersects(				const ConvexHull&		convexHull) const;
	};
}