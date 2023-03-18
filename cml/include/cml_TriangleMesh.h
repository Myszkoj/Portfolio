#pragma once


#include <vector>
#include <dpl_ReadOnly.h>
#include "cml_utilities.h"
#include "cml_CoordinateSystem.h"

#pragma warning( push )
#pragma warning( disable : 26451) // Arithmetic overflow.

namespace cml
{
	class TriangleMesh
	{
	public: // subtypes
		using	Vertices	= std::vector<Vec3>;
		using	Indices		= std::vector<uint32_t>;
		using	Normals		= std::vector<Vec3>;

		using	Vertices2D		= std::vector<Vec2>;
		using	Vertices2DArray	= std::vector<const Vertices2D*>;

	public: // data
		dpl::ReadOnly<Vertices,	TriangleMesh> vertices;
		dpl::ReadOnly<Indices,	TriangleMesh> indices;

	public: // functions
		void			triangulate(			const CoordinateSystem& RPS,
												const uint32_t			X_2D_INDEX,
												const uint32_t			Y_2D_INDEX,
												const Vertices2D&		BORDER_POLYGON,
												const Vertices2DArray&	HOLE_POLYGONS,
												const Orientation		TARGET_ORIENTATION);

		inline void		reset()
		{
			vertices->resize(0); vertices->shrink_to_fit();
			indices->resize(0); indices->shrink_to_fit();
		}

		inline uint32_t get_numVertices() const
		{
			return static_cast<uint32_t>(vertices().size());
		}

		inline uint32_t get_numIndices() const
		{
			return static_cast<uint32_t>(indices().size());
		}

		void			validate_index_count() const;

		void			validate_indices() const;

		inline void		reserve_vertices(		const uint32_t			AMOUNT)
		{
			vertices->reserve(AMOUNT);
		}

		inline void		reserve_indices(		const uint32_t			AMOUNT)
		{
			indices->reserve(AMOUNT);
		}

		/*
			Adds vertex and returns its index.
		*/
		inline uint32_t	add_vertex(				const Vec3&				NEW_VERTEX)
		{
			vertices->push_back(NEW_VERTEX);
			return static_cast<uint32_t>(vertices().size()-1);
		}

		inline void		add_index(				const uint32_t			NEW_INDEX)
		{
			indices->push_back(NEW_INDEX);
		}

		void			extend(					const TriangleMesh&		OTHER,
												const Mat4&				OTHER_TRANSFORMATION);

		void			flip();

		void			generate_normals(		Vec3*					output) const;

		inline Normals	generate_normals() const
		{
			Normals output(get_numVertices(), Vec3(0.f, 0.f, 0.f));			
			generate_normals(output.data());
			return output;
		}
	};
}

#pragma warning( pop ) 