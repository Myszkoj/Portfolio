#include "..//include/cml_Rectangle.h"


namespace cml
{
	TriangleMesh			Rectangle::to_TriangleMesh(		const uint32_t		W_SUBDIVISIONS,
															const uint32_t		H_SUBDIVISIONS) const
	{
		const uint32_t x_vertices = 1 + (1 << W_SUBDIVISIONS);
		const uint32_t y_vertices = 1 + (1 << H_SUBDIVISIONS);

		TriangleMesh	output;
						output.reserve_vertices(x_vertices * y_vertices);
						output.reserve_indices((x_vertices - 1)*(y_vertices - 1)*6);

		const float dX = width() / static_cast<float>(x_vertices - 1);
		const float dY = height() / static_cast<float>(y_vertices - 1);
		const float xOffset = -width()/2.f;
		const float yOffset = -height()/2.f;

		for(uint32_t y = 0; y < y_vertices; ++y)
		{
			for(uint32_t x = 0; x < x_vertices; ++x)
			{
				if(y < y_vertices - 1 && x < x_vertices - 1)
				{
					const uint32_t i = output.get_numVertices();
					const uint32_t j = i + x_vertices;

					output.add_index(i);
					output.add_index(j + 1);
					output.add_index(j);

					output.add_index(i);
					output.add_index(i + 1);
					output.add_index(j + 1);
				}

				output.add_vertex(Vec3(x*dX + xOffset, y*dY + yOffset, 0.f));
			}
		}

		return output;
	}
}