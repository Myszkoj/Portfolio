#include "..//include/cml_Cylinder.h"


namespace cml
{
	TriangleMesh			Cylinder::to_TriangleMesh(				const uint32_t	SLICES) const
	{
		const uint32_t BASE_INDICES = (SLICES-2)*3;
		const uint32_t HULL_INDICES = SLICES*6;

		const uint32_t NUM_BASE_VERTICES	= 2*SLICES;
		const uint32_t NUM_HULL_VERTICES	= 2*SLICES;

		TriangleMesh	output;
						output.reserve_vertices(NUM_BASE_VERTICES + NUM_HULL_VERTICES);
						output.reserve_indices(HULL_INDICES + 2*BASE_INDICES);

		const float DELTA_ANGLE = 2.f * glm::pi<float>() / float(SLICES);

		auto add_vertices = [&](const float VERTICAL_OFFSET)
		{
			for(size_t nSlice = 0; nSlice < SLICES; nSlice++)
			{
				const float	ANGLE = DELTA_ANGLE * float(nSlice);

				output.add_vertex(Vec3(	radius() * cos(ANGLE),
										VERTICAL_OFFSET,
										radius() * sin(ANGLE)));
			}
		};

		const float VERTICAL_OFFSET = height()/2.f;

		// Add base vertices.
		add_vertices(VERTICAL_OFFSET);
		add_vertices(-VERTICAL_OFFSET);

		// Add hull vertices.
		add_vertices(VERTICAL_OFFSET);
		add_vertices(-VERTICAL_OFFSET);

		// base triangles
		for(uint32_t i = 1; i < SLICES -1; ++i)
		{
			// top triangle
			output.add_index(i+1);
			output.add_index(0);
			output.add_index(i);	

			// bottom triangle
			output.add_index(i + SLICES + 1);
			output.add_index(i + SLICES);
			output.add_index(SLICES);		
		}

		const uint32_t hullOffset = 2*SLICES;

		// hull triangles
		for(uint32_t i = SLICES -1, j = 0; j < SLICES; i = j++)
		{
			output.add_index(hullOffset + i);
			output.add_index(hullOffset + i + SLICES);
			output.add_index(hullOffset + j);
				   
			output.add_index(hullOffset + j);
			output.add_index(hullOffset + i + SLICES);
			output.add_index(hullOffset + j + SLICES);
		}

		return output;
	}

	TriangleMesh			Cylinder::to_TriangleMesh_with_arrow(	const uint32_t	SLICES) const
	{
		const uint32_t NUM_BASE_VERTICES		= 2*SLICES + 1; // top base have vertex at the center
		const uint32_t NUM_HULL_VERTICES		= 2*SLICES;

		const uint32_t NUM_TOP_BASE_INDICES		= SLICES*3;
		const uint32_t NUM_BOTTOM_BASE_INDICES	= (SLICES-2)*3;

		const uint32_t HULL_INDICES				= SLICES*6;

		TriangleMesh	output;
						output.reserve_vertices(NUM_BASE_VERTICES + NUM_HULL_VERTICES);
						output.reserve_indices(HULL_INDICES + NUM_TOP_BASE_INDICES + NUM_BOTTOM_BASE_INDICES);
		
		auto add_ring_vertices = [&](const float VERTICAL_OFFSET)
		{
			const float DELTA_ANGLE = 2.f * glm::pi<float>() / float(SLICES);

			for(size_t nSlice = 0; nSlice < SLICES; nSlice++)
			{
				const float	ANGLE = DELTA_ANGLE * float(nSlice);

				output.add_vertex(	Vec3(	radius() * cos(ANGLE),
											VERTICAL_OFFSET,
											radius() * sin(ANGLE)));				
			}
		};

		// Add top center vertex.
		output.add_vertex(Vec3(0.f, height(), 0.f));

		// Add vertices of the top base.
		const uint32_t TOP_BASE_VERTICES_OFFSET		= 0;
		add_ring_vertices(height());
		const uint32_t NUM_TOP_BASE_VERTICES		= output.get_numVertices() - TOP_BASE_VERTICES_OFFSET;

		// Add vertices of the bottom base.
		const uint32_t BOTTOM_BASE_VERTICES_OFFSET	= output.get_numVertices();
		add_ring_vertices(0.f);
		const uint32_t NUM_BOTTOM_BASE_VERTICES		= output.get_numVertices() - BOTTOM_BASE_VERTICES_OFFSET;

		// Add top vertices of the hull.
		const uint32_t TOP_HULL_VERTICES_OFFSET		= output.get_numVertices();
		add_ring_vertices(height());
		const uint32_t NUM_TOP_HULL_VERTICES		= output.get_numVertices() - TOP_HULL_VERTICES_OFFSET;

		// Add bottom vertices of the hull.
		const uint32_t BOTTOM_HULL_VERTICES_OFFSET	= output.get_numVertices();
		add_ring_vertices(0.f);
		const uint32_t NUM_BOTTOM_HULL_VERTICES		= output.get_numVertices() - BOTTOM_HULL_VERTICES_OFFSET;

		// Arrow end points.
		const uint32_t div = SLICES / 3;		// offset from first vertex of the ring to one of the end points of the arrow
		const uint32_t p0 = 0;					// center of the ring
		const uint32_t p1 = 1;					// apex vertex
		const uint32_t p2 = div + 1;			// left vertex
		const uint32_t p3 = SLICES - div + 1;	// right vertex

		// Add arrow triangles(CW)
		output.add_index(p0);
		output.add_index(p1);	
		output.add_index(p2);
		output.add_index(p1);
		output.add_index(p0);
		output.add_index(p3);		

		// Top base triangles(CCW, between p1 and p2)
		for(uint32_t i = 2; i < p2; ++i)
		{
			output.add_index(i+1);
			output.add_index(i);
			output.add_index(p1);			
		}
		// Top base triangles(CCW, between p1 and p3)
		for(uint32_t i = SLICES; i > p3; --i)
		{
			output.add_index(i);
			output.add_index(i-1);
			output.add_index(p1);		
		}
		// Top base triangles(CCW, between p2 and p3)
		for(uint32_t i = p2; i < p3; ++i)
		{
			output.add_index(i);
			output.add_index(p0);
			output.add_index(i+1);		
		}

		// Bottom base triangles(CCW)
		for(uint32_t i = 1; i < SLICES -1; ++i)
		{
			output.add_index(i + SLICES + 1);
			output.add_index(i + SLICES + 2);		
			output.add_index(SLICES + 1);		
		}

		const uint32_t HULL_INDICES_OFFSET = output.get_numIndices();

		// Offset to first vertex of the hull.
		const uint32_t hullOffset = 2*SLICES + 1; // we add top base center

		// Hull triangles(CCW)
		for(uint32_t i = SLICES -1, j = 0; j < SLICES; i = j++)
		{
			output.add_index(hullOffset + i);
			output.add_index(hullOffset + j);
			output.add_index(hullOffset + i + SLICES);
			
			output.add_index(hullOffset + j);
			output.add_index(hullOffset + j + SLICES);
			output.add_index(hullOffset + i + SLICES);			
		}

		return output;
	}
}