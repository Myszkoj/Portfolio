#pragma once


#include "glw_FragmentLayer.h"


namespace glw
{
	class DepthStencilLayer : public FragmentLayer
	{
	public: // data
		dpl::ReadOnly<GLclampd,	DepthStencilLayer> depth;
		dpl::ReadOnly<GLint,	DepthStencilLayer> stencil;

	public: // lifecycle
		CLASS_CTOR		DepthStencilLayer(	const Binding&	BINDING)
			: FragmentLayer(BINDING, Attachment::DEPTH_STENCIL_BUFFER, GL_DEPTH32F_STENCIL8)
			, depth(1.0)
			, stencil(0)
		{

		}

	public: // functions
		inline void		set_depth(			const GLclampd	NEW_DEPTH)
		{
			*depth = glm::clamp(NEW_DEPTH, 0.0, 1.0);
		}

		inline void		set_stencil(		const GLint		NEW_STENCIL)
		{
			*stencil = glm::clamp(NEW_STENCIL, 0, 255);
		}

	private: // Layer implementation
		virtual void	clear() final override
		{
#ifdef _DEBUG
		glClearDepth(depth());
		glClearStencil(stencil());
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#else
		// Shorter alternative, but does not generate important errors.
		glClearBufferfi(GL_DEPTH_STENCIL, GL_NONE, depth(), stencil());
#endif // _DEBUG
		}
	};
}