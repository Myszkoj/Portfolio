#include "..//include/cml_TriangleMesh.h"
#include <set>
#include <map>
#include <unordered_map>
#include <array>
#include <memory>
#include <dpl_GeneralException.h>
#include <poly2tri/poly2tri.h>

#pragma warning (disable: 26451)

namespace cml
{
	using	Contour			= std::vector<uint32_t>;
	using	ContourArray	= std::vector<Contour>;

	uint32_t			estimate_numTriangles(	const Contour&						BORDER,
												const ContourArray&					HOLES)
	{
		/*
			NUM_TRIANGLES = N-2 + M*2 - T
			N - ilosæ wierzcho³ków w poligonie
			M - iloœæ wierzcho³ków w dziurach
			T - iloœæ trójk¹tów w dziurach
		*/
			
		const uint32_t NUM_BASE_VERTICES	= static_cast<uint32_t>(BORDER.size());
		const uint32_t NUM_BASE_TRIANGLES	= NUM_BASE_VERTICES - 2;

		uint32_t numVertices		= NUM_BASE_VERTICES;
		uint32_t numHoleTriangles	= 0;

		for(auto& iHole : HOLES)
		{
			const uint32_t NUM_HOLE_VERTICES = static_cast<uint32_t>(iHole.size());
			if(NUM_HOLE_VERTICES < 3)
				continue;

			numVertices			+= NUM_HOLE_VERTICES;
			numHoleTriangles	+= NUM_HOLE_VERTICES - 2;
		}

		return NUM_BASE_TRIANGLES + numVertices - numHoleTriangles;
	}

	Contour				to_contour(				const TriangleMesh::Vertices2D&		VERTICES,
												const uint64_t						BEGIN,
												const uint64_t						END)
		{
			Contour contour;
					contour.reserve(END-BEGIN);

			for(uint64_t index = BEGIN; index < END; ++index)
			{
				contour.push_back(static_cast<uint32_t>(index));
			}

			return contour;
		}

	inline void			validate_contour(		const Contour&						CONTOUR)
	{
		if(CONTOUR.size() < 3)
			throw dpl::GeneralException(__FILE__, __LINE__, "Contour must have at least 3 vertices.");
	}


	namespace Triangulation
	{
		class	OrientedEdge
		{
		public: // data
			dpl::ReadOnly<uint32_t,		OrientedEdge> beginID;
			dpl::ReadOnly<uint32_t,		OrientedEdge> endID;
			dpl::ReadOnly<Orientation,	OrientedEdge> orientation;

		public: // lifecycle
			CLASS_CTOR			OrientedEdge(	const uint32_t		BEGIN_ID, 
												const uint32_t		END_ID, 
												const Orientation 	ORIENTATION)
				: beginID(BEGIN_ID)
				, endID(END_ID)
				, orientation(ORIENTATION)
			{

			}

		public: // functions
			inline bool			operator==(		const OrientedEdge&	OTHER) const
			{
				return OTHER.beginID == beginID && OTHER.endID == endID;
			}

			inline bool			operator!=(		const OrientedEdge&	OTHER) const
			{
				return OTHER.beginID != beginID || OTHER.endID != endID;
			}

			inline const Vec2&	begin(			const Vec2*			VERTEX_BUFFER) const
			{
				return VERTEX_BUFFER[beginID];
			}

			inline const Vec2&	end(			const Vec2*			VERTEX_BUFFER) const
			{
				return VERTEX_BUFFER[endID];
			}

			/*
			inline void			flip()
			{
				std::swap(*beginID, *endID);
				orientation = (orientation == Orientation::CW) ? Orientation::CCW : Orientation::CW;
			}
			*/
		};

		/*
			This class contains information about edges connected to the vertex.

			There are 3 types of nodes

			Node with both vectors starting on node's position.
				A
			   /
			  T
			   \
			    B
			--------------------------------> +X
			Node with two vectors ending on node's position. 
			 A
			  \
			   T
			  /
			 B
			--------------------------------> +X
			Node with one vector starting and second ending on node's position.
			    A
			   /
		      T	
			 /
			B
			--------------------------------> +X
			where A and B are edges connected to adjacent KNodes


			- nodes can be sorted with < operator using pointer to vertex
			- nodes can generate edges
		*/
		class	TNode
		{
		public: // data
			dpl::ReadOnly<Orientation,	TNode>	orientation;
			dpl::ReadOnly<uint32_t,		TNode>	vertexID;
			dpl::ReadOnly<uint32_t,		TNode>	prevID;
			dpl::ReadOnly<uint32_t,		TNode>	nextID;		
			dpl::ReadOnly<const Vec2*,	TNode>	vertices;

		public: // lifecycle
			CLASS_CTOR				TNode(			const Orientation	ORIENTATION,
													const uint32_t		VERTEX_ID,
													const uint32_t		PREV_ID,
													const uint32_t		NEXT_ID,
													const Vec2*			VERTEX_BUFFER)
				: orientation(ORIENTATION)
				, vertexID(VERTEX_ID)
				, prevID(PREV_ID)
				, nextID(NEXT_ID)
				, vertices(VERTEX_BUFFER)
			{

			}

		public: // functions
			inline const Vec2&		vertex() const
			{
				return vertices()[vertexID];
			}

			inline const Vec2&		previous() const
			{
				return vertices()[prevID];
			}

			inline const Vec2&		next() const
			{
				return vertices()[nextID];
			}

			inline bool				operator<(		const TNode&		OTHER) const
			{
				return less(this->vertex(), OTHER.vertex());
			}

			inline bool				is_IN_end() const
			{
				return less(previous(), vertex());
			}

			inline bool				is_OUT_end() const
			{
				return less(next(), vertex());
			}

			inline OrientedEdge		calculate_IN() const
			{
				return OrientedEdge(prevID, vertexID, orientation);
			}

			inline OrientedEdge		calculate_OUT() const
			{
				return OrientedEdge(vertexID, nextID, orientation);
			}

		private: // functions
			inline bool less(const Vec2& LHS, const Vec2& RHS) const
			{
				if(LHS.x > RHS.x)	return false;
				if(LHS.x < RHS.x)	return true;
				return LHS.y < RHS.y;
			}
		};

		class	KNode : public Vec2
		{
		public: // inherited
			using Vec2::Vec2;

		public: // functions
			inline bool		operator<(		const KNode&		OTHER) const
			{
				if(this->x > OTHER.x)	return false;
				if(this->x < OTHER.x)	return true;
				return this->y < OTHER.y;
			}
		};

		
		using	ContourList		= std::list<Contour>;
		using	Triangle		= std::array<uint32_t, 3>;
		using	Triangles		= std::vector<Triangle>;
		using	SortedNodes		= std::set<TNode>;

		struct	Result
		{
			SortedNodes		nodes;
			ContourArray	monotones;
			Triangles		triangles;
		};
	}

	namespace Triangulation2
	{
		using	Contour			= std::vector<uint32_t>;
		using	ContourArray	= std::vector<Contour>;

		/*
			This class contains information about edges connected to the vertex.

			There are 3 types of nodes

			Node with both vectors starting on node's position.
				A
			   /
			  T
			   \
			    B
			--------------------------------> +X
			Node with two vectors ending on node's position. 
			 A
			  \
			   T
			  /
			 B
			--------------------------------> +X
			Node with one vector starting and second ending on node's position.
			    A
			   /
		      T	
			 /
			B
			--------------------------------> +X
			where A and B are edges connected to adjacent KNodes


			- nodes can be sorted with < operator using pointer to vertex
			- nodes can generate edges
		*/

		class	OrientedEdge
		{
		public: // data
			dpl::ReadOnly<uint32_t,		OrientedEdge>	beginID;
			dpl::ReadOnly<uint32_t,		OrientedEdge>	endID;
			dpl::ReadOnly<Orientation,	OrientedEdge>	orientation;
			dpl::ReadOnly<const Vec2*,	OrientedEdge>	vertices;

		public: // lifecycle
			CLASS_CTOR			OrientedEdge(	const uint32_t		BEGIN_ID, 
												const uint32_t		END_ID, 
												const Orientation 	ORIENTATION,
												const Vec2*			VERTEX_BUFFER)
				: beginID(BEGIN_ID)
				, endID(END_ID)
				, orientation(ORIENTATION)
				, vertices(VERTEX_BUFFER)
			{

			}

		public: // functions
			inline bool			operator==(		const OrientedEdge&	OTHER) const
			{
				return OTHER.beginID == beginID && OTHER.endID == endID;
			}

			inline bool			operator!=(		const OrientedEdge&	OTHER) const
			{
				return OTHER.beginID != beginID || OTHER.endID != endID;
			}

			inline bool			operator<(		const OrientedEdge&	OTHER) const
			{
				return less(this->begin(), OTHER.begin());
			}

			inline const Vec2&	begin() const
			{
				return vertices()[beginID];
			}

			inline const Vec2&	end() const
			{
				return vertices()[endID];
			}

			inline uint32_t		get_monotone_beginID() const
			{
				return less(begin(), end()) ? beginID() : endID();
			}

			inline uint32_t		get_monotone_endID() const
			{
				return less(begin(), end()) ? endID() : beginID();
			}

			bool				point_between(	const Vec2&			POINT) const
			{
				const Vec2& BEGIN	= begin();
				const Vec2& END		= end();

				if(less(BEGIN, END))
				{
					const Vec2& LEFT	= BEGIN;
					const Vec2& RIGHT	= END;

					return less(LEFT, POINT) && less(POINT, RIGHT);
				}

				const Vec2& LEFT	= END;
				const Vec2& RIGHT	= BEGIN;

				return less(LEFT, POINT) && less(POINT, RIGHT);
			}

			/*
			inline bool			point_left(		const Vec2&			POINT) const
			{
				return less(POINT, glm::min(begin(), end()));
			}

			inline bool			point_right(	const Vec2&			POINT) const
			{
				return less(glm::max(begin(), end()), POINT);
			}
			*/
		private: // functions
			inline bool			less(			const Vec2& LHS, const Vec2& RHS) const
			{
				if(LHS.x > RHS.x)	return false;
				if(LHS.x < RHS.x)	return true;
				return LHS.y < RHS.y;
			}

			inline bool			less_equal(		const Vec2& LHS, const Vec2& RHS) const
			{
				if(LHS.x > RHS.x)	return false;
				if(LHS.x < RHS.x)	return true;
				return LHS.y <= RHS.y;
			}
		};

		using	Triangle		= std::array<uint32_t, 3>;
		using	Triangles		= std::vector<Triangle>;
		using	SortedEdges		= std::map<OrientedEdge, std::unique_ptr<Contour>>; // Maps edges with potential monotone polygons.
		using	Intersections	= std::unordered_map<float, std::vector<SortedEdges::iterator>>; // All edges that intersect given X coordinate.
	}
}

namespace std
{
	template <>
	struct hash<cml::Triangulation::OrientedEdge>
	{
		static size_t	calculate(	const cml::Triangulation::OrientedEdge& EDGE)
		{
			if(EDGE.beginID > EDGE.endID)
				return (hash<uint32_t>()(EDGE.beginID) ^ hash<uint32_t>()(EDGE.endID) << 1);

			return (hash<uint32_t>()(EDGE.endID) ^ hash<uint32_t>()(EDGE.beginID) << 1);
		}

		size_t			operator()(	const cml::Triangulation::OrientedEdge& link) const
		{
			return calculate(link);
		}
	};

	template <>
	struct hash<cml::Triangulation2::OrientedEdge>
	{
		static size_t	calculate(	const cml::Triangulation2::OrientedEdge& EDGE)
		{
			if(EDGE.beginID > EDGE.endID)
				return (hash<uint32_t>()(EDGE.beginID) ^ hash<uint32_t>()(EDGE.endID) << 1);

			return (hash<uint32_t>()(EDGE.endID) ^ hash<uint32_t>()(EDGE.beginID) << 1);
		}

		size_t			operator()(	const cml::Triangulation2::OrientedEdge& link) const
		{
			return calculate(link);
		}
	};
}

namespace cml
{
	namespace Triangulation
	{
		Orientation			calculate_winding(		const uint32_t*						CONTOUR,
													const uint64_t						CONTOUR_SIZE,
													const Vec2*							VERTEX_BUFFER)
		{
			float signedArea = 0.f;

			for(uint64_t i = CONTOUR_SIZE -1, j = 0; j < CONTOUR_SIZE; i = j++)
			{
				const auto& FIRST_VERTEX	= VERTEX_BUFFER[CONTOUR[i]];
				const auto& SECOND_VERTEX	= VERTEX_BUFFER[CONTOUR[j]];

				signedArea += FIRST_VERTEX.x * SECOND_VERTEX.y - SECOND_VERTEX.x * FIRST_VERTEX.y;
			}

			return signedArea < 0.f ? Orientation::CW : Orientation::CCW;
		}

		void				contour_to_TNodes(		const Contour&						CONTOUR,
													const Orientation					TARGET_ORIENTATION,
													const Vec2*							VERTEX_BUFFER,
													SortedNodes&						output)
		{
			validate_contour(CONTOUR);

			const uint32_t		NUM_VERTICES	= static_cast<uint32_t>(CONTOUR.size());
			const Orientation	ORIENTATION		= calculate_winding(CONTOUR.data(), NUM_VERTICES, VERTEX_BUFFER);

			for (uint32_t i = NUM_VERTICES - 1, j = 0; j < NUM_VERTICES; i = j++)
			{
				const uint32_t	CURRENT_ID	= CONTOUR.at(j);
				uint32_t		prevID		= CONTOUR.at(i);
				uint32_t		nextID		= CONTOUR.at((j+1)%NUM_VERTICES);

				if(ORIENTATION != TARGET_ORIENTATION) std::swap(prevID, nextID);

				auto result = output.emplace(TARGET_ORIENTATION, CONTOUR.at(j), prevID, nextID, VERTEX_BUFFER);
				if(result.second == false)
					throw dpl::GeneralException(__FILE__, __LINE__, "No two, arbitrary vertices of the polygon and its holes can share the same X and Y coordinate.");
			}
		}

		ContourList			TNodes_to_monotones(	const SortedNodes&					NODES,
													const Vec2*							VERTEX_BUFFER)
		{
			ContourList output;

			// Pointer to Contour forces output to be a list instead of a vector.
			std::unordered_map<OrientedEdge, Contour*>	openPolygons;  // open map of edges that are used to generate monotone polygons

			// Triangulation starts here.
			// We process each T-Node from left to right and try to assign it to the monotone polygon.
			for(auto& iNode : NODES)
			{
				const OrientedEdge IN  = iNode.calculate_IN();
				const OrientedEdge OUT = iNode.calculate_OUT();

				// Distance to closest edge above the node.
				const OrientedEdge* edgeAbove		= nullptr;
				float				aboveDistance	= std::numeric_limits<float>::max();

				// Distance to closest edge below the node.
				const OrientedEdge* edgeBelow		= nullptr;
				float				belowDistance	= std::numeric_limits<float>::max();

				// Each edge, no matter if we can add it to the polygon or not, can override edges above or below.
				// We will check on which side of the edge is the current TNode and based on that we will decide 
				// whether or not it will be added. 
				for(auto& iMonotone : openPolygons)
				{
					// edge cannot be connected to KNode
					if(iMonotone.first != IN && iMonotone.first != OUT)
					{
						const Vec2& EDGE_START	= iMonotone.first.begin(VERTEX_BUFFER);
						const Vec2& EDGE_END	= iMonotone.first.end(VERTEX_BUFFER);

						//if(right_side(EDGE_START, EDGE_END, iNode.vertex()))
						{
							// This is the distance between edge Y coordinate and the TNode at the TNode.x position
							// note that if distance is negative edge is above TNode.
							if(auto SIGNED_DISTANCE = signed_Y_distance(EDGE_START, EDGE_END, iNode.vertex()))
							{
								const float DISTANCE = abs(SIGNED_DISTANCE.value());
								if(SIGNED_DISTANCE < 0.f)
								{
									if(DISTANCE < aboveDistance)
									{
										aboveDistance	= DISTANCE;
										edgeAbove		= &iMonotone.first;
									}
								}
								else 
								{
									if(DISTANCE < belowDistance)
									{
										belowDistance	= DISTANCE;
										edgeBelow		= &iMonotone.first;
									}
								}
							}
						}	
					}
				}

				if(edgeBelow)
				{
					if(right_side(edgeBelow->begin(VERTEX_BUFFER), edgeBelow->end(VERTEX_BUFFER), iNode.vertex()))
						openPolygons[*edgeBelow]->push_back(iNode.vertexID());
				}
				if(edgeAbove && (edgeAbove != edgeBelow || edgeBelow == nullptr))
				{	
					if(right_side(edgeAbove->begin(VERTEX_BUFFER), edgeAbove->end(VERTEX_BUFFER), iNode.vertex()))
						openPolygons[*edgeAbove]->push_back(iNode.vertexID());
				}


				{auto& pIN	= openPolygons[IN];

					if(pIN == nullptr) pIN = &output.emplace_back();
					pIN->push_back(iNode.vertexID());
				}

				{auto& pOUT	= openPolygons[OUT];

					if(pOUT == nullptr) pOUT = &output.emplace_back();
					pOUT->push_back(iNode.vertexID());
				}
		
				// Close the polygon(remove polygon from the open list).
				if(iNode.is_IN_end())	openPolygons.erase(IN);
				if(iNode.is_OUT_end())	openPolygons.erase(OUT);
			}

			if(openPolygons.size() != 0)
				throw dpl::GeneralException(__FILE__, __LINE__, "Triangulation could not be completed. Number of edges that was not processed: " + std::to_string(openPolygons.size()));

			uint32_t numInvalid = 0;

			// Erase contours with less than 3 vertices.
			for(auto iContour = output.begin(); iContour != output.end();)
			{
				if(iContour->size() < 3)
				{
					++numInvalid;
					iContour = output.erase(iContour);
				}
				else
				{
					++iContour;
				}
			}

			if(numInvalid > 0)
			{
				int breakpoint = 0;
			}

			return output;
		}

		Triangle			create_triangle(		std::list<OrientedEdge>::iterator	current, 
													const Orientation					TARGET_ORIENTATION,
													const Vec2*							VERTEX_BUFFER)
		{
			Triangle triangle{current->beginID(), current->endID(), (++current)->endID()};

			if(calculate_winding(triangle.data(), 3, VERTEX_BUFFER) != TARGET_ORIENTATION)
			{
				std::swap(triangle[1], triangle[2]);
			}

			return triangle;
		}

		void				perform_triangulation(	const Contour&						MONOTONE,
													const Orientation					TARGET_ORIENTATION,
													const Vec2*							VERTEX_BUFFER,
													Triangles&							output)
		{
			validate_contour(MONOTONE);

			const uint32_t		NUM_VERTICES	= static_cast<uint32_t>(MONOTONE.size());
			const Orientation	ORIENTATION		= calculate_winding(MONOTONE.data(), NUM_VERTICES, VERTEX_BUFFER);

			std::list<OrientedEdge> openList; // list of edges to process

			for(uint32_t index = 0; index < NUM_VERTICES; ++index)
			{
				openList.emplace_back(MONOTONE[index], MONOTONE[(index+1)%NUM_VERTICES], ORIENTATION);
			}

			auto current = openList.begin();
			
			while(openList.size() > 3)
			{
				auto next = std::next(current);

				while(1)
				{
					const bool bRIGHT = right_side(	current->begin(VERTEX_BUFFER), 
													current->end(VERTEX_BUFFER), 
													next->end(VERTEX_BUFFER));

					if(ORIENTATION == Orientation::CW ? !bRIGHT : bRIGHT)
						current = next++; // based on how monotone polygon was created we will never go out of bounds
					else
						break;
				}

				output.emplace_back(create_triangle(current, TARGET_ORIENTATION, VERTEX_BUFFER));

				/*
					-> [newCurrent] -> [oldCurrent] -> [oldNext] ->
							(insert)       (erase[1])     (erase[2])
				*/
				current = openList.insert(current, OrientedEdge(current->beginID(), next->endID(), ORIENTATION));
				openList.erase(openList.erase(std::next(current)));

				// now is an interesting part, if we skip this step, we may generate invalid polygon,
				// we must try to decrement current iterator in order to combine some previous edge with newly generated one
				// otherwise we will reach the point in which we have to loop whole triangulation process ...
				if(current != openList.begin())
					--current;
			}

			// Generate last triangle from remaining 3 edges.
			output.emplace_back(create_triangle(openList.begin(), TARGET_ORIENTATION, VERTEX_BUFFER));
		}

		Result				execute(				const Contour&						BORDER,
													const ContourArray&					HOLES,
													const Orientation					TARGET_ORIENTATION,
													const Vec2*							VERTEX_BUFFER)
		{
			Result	result;
					result.triangles.reserve(estimate_numTriangles(BORDER, HOLES));
					
			contour_to_TNodes(BORDER, Orientation::CW, VERTEX_BUFFER, result.nodes);
			for (auto& iHole : HOLES)
			{
				contour_to_TNodes(iHole, Orientation::CCW, VERTEX_BUFFER, result.nodes);
			}

			auto monotones = TNodes_to_monotones(result.nodes, VERTEX_BUFFER);
			for(auto& iMonotone : monotones)
			{
				perform_triangulation(iMonotone, TARGET_ORIENTATION, VERTEX_BUFFER, result.triangles);
			}

			return result;
		}
	}

	namespace Triangulation2
	{
		Orientation			calculate_winding(		const uint32_t*						CONTOUR,
													const uint64_t						CONTOUR_SIZE,
													const Vec2*							VERTEX_BUFFER)
		{
			float signedArea = 0.f;

			for(uint64_t i = CONTOUR_SIZE -1, j = 0; j < CONTOUR_SIZE; i = j++)
			{
				const auto& FIRST_VERTEX	= VERTEX_BUFFER[CONTOUR[i]];
				const auto& SECOND_VERTEX	= VERTEX_BUFFER[CONTOUR[j]];

				signedArea += FIRST_VERTEX.x * SECOND_VERTEX.y - SECOND_VERTEX.x * FIRST_VERTEX.y;
			}

			return signedArea < 0.f ? Orientation::CW : Orientation::CCW;
		}

		void				contour_to_edges(		const Contour&						CONTOUR,
													const Orientation					TARGET_ORIENTATION,
													const Vec2*							VERTEX_BUFFER,
													SortedEdges&						edges)
		{
			validate_contour(CONTOUR);

			const uint32_t		NUM_VERTICES	= static_cast<uint32_t>(CONTOUR.size());
			const Orientation	ORIENTATION		= calculate_winding(CONTOUR.data(), NUM_VERTICES, VERTEX_BUFFER);

			for (uint32_t i = NUM_VERTICES - 1, j = 0; j < NUM_VERTICES; i = j++)
			{			
				uint32_t	beginID	= CONTOUR.at(i);
				uint32_t	endID	= CONTOUR.at(j);

				if(ORIENTATION != TARGET_ORIENTATION) std::swap(beginID, endID);

				auto result = edges.emplace(OrientedEdge(beginID, endID, TARGET_ORIENTATION, VERTEX_BUFFER), nullptr);
				if(result.second == false)
					throw dpl::GeneralException(__FILE__, __LINE__, "No two, arbitrary vertices of the polygon and its holes can share the same X and Y coordinate.");
			}
		}

		Triangle			create_triangle(		std::list<OrientedEdge>::iterator	current, 
													const Orientation					TARGET_ORIENTATION,
													const Vec2*							VERTEX_BUFFER)
		{
			Triangle triangle{current->beginID(), current->endID(), (++current)->endID()};

			if(calculate_winding(triangle.data(), 3, VERTEX_BUFFER) != TARGET_ORIENTATION)
			{
				std::swap(triangle[1], triangle[2]);
			}

			return triangle;
		}

		void				perform_triangulation(	const Contour&						MONOTONE,
													const Orientation					TARGET_ORIENTATION,
													const Vec2*							VERTEX_BUFFER,
													Triangles&							output)
		{
			validate_contour(MONOTONE);

			const uint32_t		NUM_VERTICES	= static_cast<uint32_t>(MONOTONE.size());
			const Orientation	ORIENTATION		= calculate_winding(MONOTONE.data(), NUM_VERTICES, VERTEX_BUFFER);

			std::list<OrientedEdge> openList; // list of edges to process

			for(uint32_t index = 0; index < NUM_VERTICES; ++index)
			{
				openList.emplace_back(MONOTONE[index], MONOTONE[(index+1)%NUM_VERTICES], ORIENTATION, VERTEX_BUFFER);
			}

			auto current = openList.begin();
			
			while(openList.size() > 3)
			{
				auto next = std::next(current);

				while(1)
				{
					const bool bRIGHT = right_side(	current->begin(), 
													current->end(), 
													next->end());

					if(ORIENTATION == Orientation::CW ? !bRIGHT : bRIGHT)
						current = next++; // based on how monotone polygon was created we will never go out of bounds
					else
						break;
				}

				output.emplace_back(create_triangle(current, TARGET_ORIENTATION, VERTEX_BUFFER));

				/*
					-> [newCurrent] -> [oldCurrent] -> [oldNext] ->
							(insert)       (erase[1])     (erase[2])
				*/
				current = openList.insert(current, OrientedEdge(current->beginID(), next->endID(), ORIENTATION, VERTEX_BUFFER));
				openList.erase(openList.erase(std::next(current)));

				// now is an interesting part, if we skip this step, we may generate invalid polygon,
				// we must try to decrement current iterator in order to combine some previous edge with newly generated one
				// otherwise we will reach the point in which we have to loop whole triangulation process ...
				if(current != openList.begin())
					--current;
			}

			// Generate last triangle from remaining 3 edges.
			output.emplace_back(create_triangle(openList.begin(), TARGET_ORIENTATION, VERTEX_BUFFER));
		}

		Triangles			execute(				const Contour&						BORDER,
													const ContourArray&					HOLES,
													const Orientation					TARGET_ORIENTATION,
													const Vec2*							VERTEX_BUFFER)
		{
			Triangles	triangles;
						triangles.reserve(estimate_numTriangles(BORDER, HOLES));

			SortedEdges	edges;
					
			contour_to_edges(BORDER, Orientation::CW, VERTEX_BUFFER, edges);
			for (auto& iHole : HOLES)
			{
				contour_to_edges(iHole, Orientation::CCW, VERTEX_BUFFER, edges);
			}

			Intersections	intersections;
							intersections.reserve(edges.size());

			// Find all unique X coordinates.
			for(auto& iEdge : edges) intersections[iEdge.first.begin().x];

			// Find all intersections(O(n^2), very slow).
			for(auto iEdge = edges.begin(); iEdge != edges.end(); ++iEdge)
			{
				const Vec2& BEGIN	= iEdge->first.begin();
				const Vec2& END		= iEdge->first.end();

				const float MIN = glm::min(BEGIN.x, END.x);
				const float MAX = glm::max(BEGIN.x, END.x);

				for(auto& iIntersection : intersections)
				{
					if(MIN <= iIntersection.first 
					&& MAX >= iIntersection.first)
					{
						iIntersection.second.push_back(iEdge);
					}
				}
			}

			for(auto it = edges.begin(); it != edges.end(); ++it)
			{
				const uint32_t	VERTEX_ID	= it->first.beginID();
				const Vec2&		VERTEX		= it->first.begin();

				// Distance to closest edge above the node.
				SortedEdges::iterator	edgeAbove		= edges.end();
				float					distanceAbove	= std::numeric_limits<float>::max();
				float					dotAbove		= -1.f;

				// Distance to closest edge below the node.
				SortedEdges::iterator	edgeBelow		= edges.end();
				float					distanceBelow	= std::numeric_limits<float>::max();
				float					dotBelow		= -1.f;

				// Find closest edges.
				const auto& POTENTIAL_MONOTONES = intersections[VERTEX.x];
				for(auto& iBASE : POTENTIAL_MONOTONES)
				{
					if(iBASE->first.point_between(VERTEX))
					{
						const Vec2& EDGE_START	= iBASE->first.begin();
						const Vec2& EDGE_END	= iBASE->first.end();

						if(auto SIGNED_DISTANCE = signed_Y_distance(EDGE_START, EDGE_END, VERTEX))
						{
							const float DISTANCE = abs(SIGNED_DISTANCE.value());

							if(SIGNED_DISTANCE < 0.f && DISTANCE < distanceAbove)
							{
								const float DOT = glm::dot(Vec2(0.f, -1.f), EDGE_START-EDGE_END);

								if(DISTANCE < distanceAbove)
								{
									distanceAbove	= DISTANCE;
									dotAbove		= DOT;
									edgeAbove		= iBASE;
								}
								else if(DISTANCE == distanceAbove)
								{
									if(DOT > dotAbove)
									{
										distanceAbove	= DISTANCE;
										dotAbove		= DOT;
										edgeAbove		= iBASE;
									}
								}
							}
							else if(SIGNED_DISTANCE >= 0.f) 
							{
								const float DOT = glm::dot(Vec2(0.f, 1.f), EDGE_START-EDGE_END);

								if(DISTANCE < distanceBelow)
								{
									distanceBelow	= DISTANCE;
									dotBelow		= DOT;
									edgeBelow		= iBASE;
								}
								if(DISTANCE == distanceBelow)
								{
									if(DOT > dotBelow)
									{
										distanceBelow	= DISTANCE;
										dotBelow		= DOT;
										edgeBelow		= iBASE;
									}
								}
							}
						}
					}
				}

				if(edgeBelow != edges.end())
				{
					const Vec2& EDGE_START	= edgeBelow->first.begin();
					const Vec2& EDGE_END	= edgeBelow->first.end();

					if(right_side(EDGE_START, EDGE_END, VERTEX))
					{
						auto& monotone = edgeBelow->second;
						if(!monotone)
						{
							monotone = std::make_unique<Contour>();
							monotone->push_back(edgeBelow->first.get_monotone_beginID());
						}

						monotone->push_back(VERTEX_ID);
					}
				}
				if((edgeAbove != edges.end()) && (edgeAbove != edgeBelow || edgeBelow == edges.end()))
				{	
					const Vec2& EDGE_START	= edgeAbove->first.begin();
					const Vec2& EDGE_END	= edgeAbove->first.end();

					if(right_side(EDGE_START, EDGE_END, VERTEX))
					{
						auto& monotone = edgeAbove->second;
						if(!monotone)
						{
							monotone = std::make_unique<Contour>();
							monotone->push_back(edgeAbove->first.get_monotone_beginID());
						}

						monotone->push_back(VERTEX_ID);
					}
				}			
			}

			// Erase contours with less than 3 vertices.
			for(auto& it : edges)
			{
				if(it.second)
				{
					if(it.second->size() < 2)
					{
						it.second.reset();
					}
					else // Close monotone and go to the next.
					{
						const auto& EDGE = it.first;
						it.second->push_back(EDGE.get_monotone_endID());

						perform_triangulation(*it.second, TARGET_ORIENTATION, VERTEX_BUFFER, triangles);
					}
				}
			}

			return triangles;
		}
	}

	void						fill(			std::vector<p2t::Point>&	output,
												const std::vector<Vec2>&	POLYGON,
												const Orientation			TARGET_ORIENTATION)
	{
		output.reserve(output.size() + POLYGON.size());

		const uint32_t		NUM_POINTS	= static_cast<uint32_t>(POLYGON.size());
		const Orientation	ORIENTATION = calculate_polygon_orientation(POLYGON.data(), NUM_POINTS);

		if(ORIENTATION == TARGET_ORIENTATION)
		{
			for(auto& iVertex : POLYGON)
			{
				output.emplace_back(iVertex.x, iVertex.y);
			}
		}
		else // Inverse
		{
			for(uint32_t index = 0; index < NUM_POINTS;++index)
			{
				const auto& VERTEX = POLYGON[NUM_POINTS-1-index];
				output.emplace_back(VERTEX.x, VERTEX.y);
			}
		}
	}

	std::vector<p2t::Point*>	to_contour(		std::vector<p2t::Point>&	vertices,
												const uint64_t				BEGIN,
												const uint64_t				END)
	{
		std::vector<p2t::Point*>	contour;
									contour.reserve(END-BEGIN);

		for(uint64_t index = BEGIN; index < END; ++index)
		{
			contour.push_back(&vertices[index]);
		}

		return contour;
	}

//=====> TriangleMesh -> public functions
	void		TriangleMesh::triangulate(		const CoordinateSystem& RPS,
												const uint32_t			X_2D_INDEX,
												const uint32_t			Y_2D_INDEX,
												const Vertices2D&		BORDER_POLYGON,
												const Vertices2DArray&	HOLE_POLYGONS,
												const Orientation		TARGET_ORIENTATION)
	{
		std::vector<p2t::Point>	vertices2D;
		
		fill(vertices2D, BORDER_POLYGON, Orientation::CW);

		for(auto& iHole : HOLE_POLYGONS)
		{
			fill(vertices2D, *iHole, Orientation::CW);
		}

		p2t::CDT cdt(to_contour(vertices2D, 0, BORDER_POLYGON.size()));

		uint64_t offset = BORDER_POLYGON.size();
		for(auto& iHole : HOLE_POLYGONS)
		{
			cdt.AddHole(to_contour(vertices2D, offset, offset + iHole->size()));
			offset += iHole->size();
		}

		cdt.Triangulate();
		
		// Transform 2D vertices into 3D RPS space.
		vertices = Vertices(vertices2D.size());
		for(uint64_t vertexID = 0; vertexID < vertices2D.size(); ++vertexID)
		{
			const auto& p2tPoint = vertices2D[vertexID];
			const Vec2 VERTEX_2D(p2tPoint.x, p2tPoint.y);
			(*vertices)[vertexID] = RPS.unproject_point(VERTEX_2D, X_2D_INDEX, Y_2D_INDEX);;
		}

		const auto TRIANGLES = cdt.GetTriangles();

		indices->clear();
		indices->reserve(TRIANGLES.size() * 3);

		const auto* ARRAY_START = reinterpret_cast<const p2t::Point*>(vertices2D.data());

		for(uint64_t triangleID = 0; triangleID < TRIANGLES.size(); ++triangleID)
		{
			const auto&	TRIANGLE = TRIANGLES[triangleID];

			if(TRIANGLE->IsInterior())
			{
				const uint64_t OFFSET = triangleID * 3;

				indices->push_back(static_cast<uint32_t>(TRIANGLE->GetPoint(0) - ARRAY_START));
				indices->push_back(static_cast<uint32_t>(TRIANGLE->GetPoint(1) - ARRAY_START));
				indices->push_back(static_cast<uint32_t>(TRIANGLE->GetPoint(2) - ARRAY_START));
			}
			else
			{
				int breakpoint = 0;
			}
		}
	}

	void		TriangleMesh::validate_index_count() const
	{
		if(get_numIndices() % 3 != 0)
			throw dpl::GeneralException(this, __LINE__, "Number of indices must be divisible by 3.");
	}

	void		TriangleMesh::validate_indices() const
	{
		validate_index_count();

		const uint32_t NUM_VERTICES = get_numVertices();
		for(auto& index : indices())
		{
			if(index >= NUM_VERTICES)
				throw dpl::GeneralException(this, __LINE__, "TriangleMesh contains at least one invalid index.");
		}
	}

	void		TriangleMesh::extend(			const TriangleMesh&		OTHER,
												const Mat4&				OTHER_TRANSFORMATION)
	{
		vertices->reserve(get_numVertices() + OTHER.get_numVertices());
		indices->reserve(get_numIndices() + OTHER.get_numIndices());

		const uint32_t VERTEX_OFFSET = get_numVertices();

		for(auto& OTHER_VERTEX : OTHER.vertices())
		{
			vertices->push_back(OTHER_TRANSFORMATION * Vec4(OTHER_VERTEX, 1.f));
		}

		for(auto& OTHER_INDEX : OTHER.indices())
		{
			indices->push_back(OTHER_INDEX + VERTEX_OFFSET);
		}
	}

	void		TriangleMesh::flip()
	{
		validate_index_count();

		const uint32_t NUM_TRIANGLES = get_numIndices() / 3;

		// Swap second and third vertex index of each triangle.
		for(uint32_t triangleID = 0; triangleID < NUM_TRIANGLES; ++triangleID)
		{
			const uint32_t OFFSET = triangleID * 3;
			std::swap((*indices)[OFFSET+1], (*indices)[OFFSET+2]);
		}
	}

	void		TriangleMesh::generate_normals(	Vec3*					output) const
	{
		validate_indices();

		const uint32_t NUM_VERTICES		= get_numVertices();
		const uint32_t NUM_TRIANGLES	= get_numIndices() / 3;

		for(uint32_t triangleID = 0; triangleID < NUM_TRIANGLES; ++triangleID)
		{
			const uint32_t OFFSET = triangleID * 3;
			const uint32_t AID = indices()[OFFSET+0];
			const uint32_t BID = indices()[OFFSET+1];
			const uint32_t CID = indices()[OFFSET+2];

			auto& A = vertices()[AID];
			auto& B = vertices()[BID];
			auto& C = vertices()[CID];

			const Vec3 AB	= glm::normalize(B-A);
			const Vec3 AC	= glm::normalize(C-A);
			const Vec3 N	= glm::normalize(glm::cross(AB, AC));

			output[AID] += N;
			output[BID] += N;
			output[CID] += N;
		}

		for(uint32_t vertexID = 0; vertexID < NUM_VERTICES; ++vertexID)
		{
			auto&	normal = output[vertexID];
					normal = glm::normalize(normal);
		}
	}
}