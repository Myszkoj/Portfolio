#include "..//include/cml_Cuboid.h"
#include "..//include/cml_Rectangle.h"


namespace cml
{
	TriangleMesh		Cuboid::to_TriangleMesh(	const uint32_t	W_SUBDIVISIONS,
													const uint32_t	H_SUBDIVISIONS,
													const uint32_t	D_SUBDIVISIONS) const
	{
		const Rectangle XY(width(), height());
		const Rectangle XZ(width(), depth());
		const Rectangle YZ(height(), depth());

		const TriangleMesh XY_MESH = XY.to_TriangleMesh(W_SUBDIVISIONS, H_SUBDIVISIONS);
		const TriangleMesh XZ_MESH = XZ.to_TriangleMesh(W_SUBDIVISIONS, D_SUBDIVISIONS);
		const TriangleMesh YZ_MESH = YZ.to_TriangleMesh(H_SUBDIVISIONS, D_SUBDIVISIONS);

		const Mat4 PLUS_XY	= glm::translate(CoordinateSystem::global_Z() * halfDepth());
		const Mat4 MINUS_XY	= glm::translate(-CoordinateSystem::global_Z() * halfDepth()) * glm::rotate(glm::pi<float>(), CoordinateSystem::global_X());

		const Mat4 PLUS_XZ	= glm::translate(CoordinateSystem::global_Y() * halfHeight())	* glm::rotate(glm::pi<float>()/2.f, -CoordinateSystem::global_X());
		const Mat4 MINUS_XZ	= glm::translate(-CoordinateSystem::global_Y() * halfHeight()) * glm::rotate(glm::pi<float>()/2.f, CoordinateSystem::global_X());

		const Mat4 PLUS_YZ	= glm::translate(CoordinateSystem::global_X() * halfWidth()) * glm::rotate(glm::pi<float>()/2.f, CoordinateSystem::global_Y());
		const Mat4 MINUS_YZ	= glm::translate(-CoordinateSystem::global_X() * halfWidth()) * glm::rotate(glm::pi<float>()/2.f, -CoordinateSystem::global_Y());

		TriangleMesh	output;
						output.reserve_vertices(2 * (XY_MESH.get_numVertices() + XZ_MESH.get_numVertices() + YZ_MESH.get_numVertices()));
						output.reserve_indices(2 * (XY_MESH.get_numIndices() + XZ_MESH.get_numIndices() + YZ_MESH.get_numIndices()));
						output.extend(XY_MESH, PLUS_XY);
						output.extend(XY_MESH, MINUS_XY);
						output.extend(XZ_MESH, PLUS_XZ);
						output.extend(XZ_MESH, MINUS_XZ);
						output.extend(YZ_MESH, PLUS_YZ);
						output.extend(YZ_MESH, MINUS_YZ);

		return output;
	}
}