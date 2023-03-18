#pragma once


#include "glw_VAO.h"
#include <dpl_Values.h>


// declarations
namespace glw
{
	struct	DrawVertexPack;
	struct	DrawElementPack;

	using	OnDraw				= std::function<void()>;
	using	OnDrawVertexPack	= std::function<void(const DrawVertexPack&)>;
	using	OnDrawElementPack	= std::function<void(const DrawElementPack&)>;
	using	ICount				= dpl::RangedValue<int32_t, 0, std::numeric_limits<int32_t>::max()>;
	using	UCount				= dpl::RangedValue<uint32_t, 0, std::numeric_limits<uint32_t>::max()>;
}

// commands
namespace glw
{
	struct	DrawVertexPack
	{
		VertexTopology::Primitive	primitiveType	= VertexTopology::POINTS;
		ICount						vertexOffset	= 0;
		ICount						numVertices		= 0;
		ICount						numInstances	= 1;
	};

	struct	DrawVertexPacks
	{
		DrawVertexPack*				packs			= nullptr;
		ICount						numPacks		= 0;
	};

	struct	DrawElementPack
	{
		ICount						indexOffset		= 0;
		ICount						numIndices		= 0;
		ICount						numInstances	= 1;
	};

	struct	DrawElementPacks
	{
		DrawElementPack*			packs			= nullptr;
		ICount						numPacks		= 0;
	};

	struct	DrawArraysIndirectCommand
	{
		UCount						count			= 0;
		UCount						primCount		= 0;
		UCount						first			= 0;
		UCount						baseInstance	= 0;
	};


	struct	DrawElementsIndirectCommand
	{
		UCount						primitiveCount	= 0;
		UCount						instanceCount	= 0;
		UCount						firstIndex		= 0;
		ICount						baseVertex		= 0;
		UCount						baseInstance	= 0;
	};
}

// vertex rendering
namespace glw
{
	template<typename VertexT>
	inline void		draw(		VAO&									vao,
								VertexBuffer<VertexT>&					vertices,
								const OnDraw&							ON_DRAW)
	{
		if(!ON_DRAW) return;
		vao.enclose_action([&]()
		{
			vertices.enclose_action([&]()
			{
				ON_DRAW();
			});
		});
	}

	template<typename VertexT>
	inline void		draw(		VAO&									vao,
								VertexBuffer<VertexT>&					vertices,
								const DrawVertexPack&					DRAW_COMMAND)
	{
		glw::draw<VertexT>(vao, vertices, [&]()
		{
			glDrawArraysInstanced(	DRAW_COMMAND.primitiveType, 
										DRAW_COMMAND.vertexOffset, 
										DRAW_COMMAND.numVertices, 
										DRAW_COMMAND.numInstances);
		});
	}

	template<typename VertexT>
	inline void		draw(		VAO&									vao,
								VertexBuffer<VertexT>&					vertices,
								const DrawVertexPacks&					DRAW_COMMANDS,
								const OnDrawVertexPack&					ON_DRAW = nullptr)
	{
		glw::draw<VertexT>(vao, vertices, [&]()
		{
			for(uint32_t index = 0; index < DRAW_COMMANDS.numPacks; ++index)
			{
				const auto& COMMAND = DRAW_COMMANDS.packs[index];
				if(ON_DRAW) ON_DRAW(COMMAND);
				glDrawArraysInstanced(	COMMAND.primitiveType, 
										COMMAND.vertexOffset, 
										COMMAND.numVertices, 
										COMMAND.numInstances);
			}
		});
	}
}

// element rendering
namespace glw
{
	template<typename VertexT>
	inline void		draw(		VAO&									vao,
								VertexBuffer<VertexT>&					vertices,
								VertexTopology&							topology,
								const OnDraw&							ON_DRAW)
	{
		if(!ON_DRAW) return;
		glw::draw<VertexT>(vao, vertices, [&]()
		{
			topology.enclose_action([&]()
			{
				ON_DRAW();
			});
		});
	}

	template<typename VertexT>
	inline void		draw(		VAO&									vao,
								VertexBuffer<VertexT>&					vertices,
								VertexTopology&							topology,
								const DrawElementPack&					DRAW_COMMAND)
	{
		glw::draw<VertexT>(vao, vertices, topology, [&]()
		{
			glDrawElementsInstanced(topology.primitive(), 
									DRAW_COMMAND.numIndices, 
									topology.INDEX_TYPE, 
									(const void*)(DRAW_COMMAND.indexOffset * sizeof(VertexTopology::INDEX_TYPE)), 
									DRAW_COMMAND.numInstances);
		});
	}

	template<typename VertexT>
	inline void		draw(		VAO&									vao,
								VertexBuffer<VertexT>&					vertices,
								VertexTopology&							topology,
								const DrawElementPacks&					DRAW_COMMANDS,
								const OnDrawElementPack&				ON_DRAW = nullptr)
	{
		glw::draw<VertexT>(vao, vertices, topology, [&]()
		{
			for(uint32_t index = 0; index < DRAW_COMMANDS.numPacks; ++index)
			{
				const auto& COMMAND = DRAW_COMMANDS.packs[index];
				if(ON_DRAW) ON_DRAW(COMMAND);
				glDrawElementsInstanced(topology.primitive(), 
										COMMAND.numIndices, 
										topology.INDEX_TYPE, 
										(const void*)(COMMAND.indexOffset * sizeof(VertexTopology::INDEX_TYPE)), 
										COMMAND.numInstances);
			}
		});
	}
}

// indirect rendering
namespace glw
{
	/*
		[TODO]: 
		- Design CommandBuffer class that stores all draw commands(derived from Buffer_of<T> and member variable TransferablePack<T> attached to that buffer).
		- Add CommandBuffer::update function that generates required commands.

		[PROBLEM]: Number of instances in each command
	*/

	template<typename VertexT>
	inline void		draw(		VAO&									vao,
								VertexBuffer<VertexT>&					vertices,
								const VertexTopology::Primitive			PRIMITIVE_TYPE,
								Buffer_of<DrawArraysIndirectCommand>&	commandBuffer)
	{
		glw::draw<VertexT>(vao, vertices, [&]()
		{
			commandBuffer.enclose_action([&]()
			{
				glMultiDrawArraysIndirect(PRIMITIVE_TYPE, nullptr, commandBuffer.size(), 0);
			});
		});
	}

	template<typename VertexT>
	inline void		draw(		VAO&									vao,
								VertexBuffer<VertexT>&					vertices,
								VertexTopology&							topology,
								Buffer_of<DrawElementsIndirectCommand>&	commandBuffer)
	{
		glw::draw<VertexT>(vao, vertices, topology, [&]()
		{
			commandBuffer.enclose_action([&]()
			{
				glMultiDrawElementsIndirect(topology.primitive, VertexTopology::INDEX_TYPE, nullptr, commandBuffer.size(), 0);
			});
		});
	}
}