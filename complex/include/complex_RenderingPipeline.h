#pragma once


#include <dpl_Chain.h>
#include <dpl_Variation.h>
#include <dpl_ThreadPool.h>
#include <dpl_ResourceControl.h>
#include <glw_SLProgram.h>
#include <glw_FrameBuffer.h>
#include "wapp_System.h"
#include "wapp_Camera.h"


namespace wapp
{
	class Application;
	class RenderingPipeline;


	class RenderTarget	: public Camera
						, public glw::FrameBuffer
						, public dpl::Link<RenderingPipeline, RenderTarget>
	{
	public: // relations
		friend RenderingPipeline;

	private: // subtypes
		using MyPipeline	= dpl::Link<RenderingPipeline, RenderTarget>;
		using MyCamera		= dpl::Association<RenderTarget, Camera>;

	public: // lifecycle
		/* CTOR */			RenderTarget() = default;

		/* CTOR */			RenderTarget(		const RenderTarget&		OTHER) = delete;

		/* CTOR */			RenderTarget(		RenderTarget&&			other) noexcept = default;

		RenderTarget&		operator=(			const RenderTarget&		OTHER) = delete;

		RenderTarget&		operator=(			RenderTarget&&			other) noexcept = default;

	private: // functions used by rendering pipeline
		inline void			update()
		{
			Camera::update(resolution().x, resolution().y);
			if(this->is_collapsed()) return;
			FrameBuffer::clear();
		}
	};


	class Renderer : public dpl::Link<RenderingPipeline, Renderer>
	{
	public: // relations
		friend RenderingPipeline;

	private: // subtypes
		using MyBase = dpl::Link<RenderingPipeline, Renderer>;

	protected: // lifecycle
		/* CTOR */		Renderer() = default;
	};


	template<typename T>
	class RendereringSystem;


	/*
		Stores/updates render targets and invokes renderers.

		Order of render targets is ignored. Removing one may cause other to be moved in memory.
		Order of renderers is preserved. Removing one in the middle will not change order of the others.
	*/
	class RenderingPipeline : public dpl::Chain<RenderingPipeline, Renderer>
							, public dpl::Chain<RenderingPipeline, RenderTarget>
	{
	public: // relations
		friend Application;

		template<typename>
		friend class RendereringSystem;

	private: // subtypes
		using MyRenderers		= dpl::Chain<RenderingPipeline, Renderer>;
		using MyRenderTargets	= dpl::Chain<RenderingPipeline, RenderTarget>;

	public: // lifecycle
		/* CTOR */				RenderingPipeline() = default;

	public: // Renderer function
		inline void				add_renderer(			Renderer&				renderer)
		{
			MyRenderers::attach_back(renderer);
		}

	public: // RenderTarget function
		inline uint32_t			get_numRenderTargets() const
		{
			return MyRenderTargets::size();
		}

		inline bool				add_render_target(		RenderTarget&			target)
		{
			return MyRenderTargets::attach_back(target);
		}

		inline bool				remove_render_target(	RenderTarget&			target)
		{
			return MyRenderTargets::detach_link(target);
		}

		inline bool				remove_all_render_targets()
		{
			return MyRenderTargets::remove_all_links();
		}

	private: // functions
		inline void				update()
		{
			MyRenderTargets::iterate_forward([](RenderTarget& target)
			{
				target.update();
			});
		}

		inline void				release()
		{
			no_except([&](){MyRenderers::remove_all_links();});
			no_except([&](){MyRenderTargets::remove_all_links();});
		}
	};


	template<typename T>
	class RendereringSystem : public Renderer
							, public System<T>
	{
	public: // relations
		friend RenderingPipeline;

	private: // subtypes
		using MyBase = System<T>;

	public: // subtypes
		using Binding = typename MyBase::Binding;

	protected: // lifecycle
		/* CTOR */		RendereringSystem(	const Binding&		BINDING)
			: MyBase(BINDING)
		{

		}

	private: // interface
		/*	
			Called before rendering starts.
		*/
		virtual void	on_begin(			Project&			project,
											dpl::ThreadPool&	threadPool){}

		/*
			Called only when target is active(has camera and viewport is not collapsed).
		*/
		virtual void	on_render(			Project&			project,
											dpl::ThreadPool&	threadPool,
											RenderTarget&		target) = 0;

		/*
			Called when rendering ends.
		*/
		virtual void	on_end(				Project&			project,
											dpl::ThreadPool&	threadPool){}

	private: // implementation
		virtual void	on_update(			Project&			project,
											dpl::ThreadPool&	threadPool) final override
		{
			if(RenderingPipeline* pipeline = Renderer::get_chain())
			{
				on_begin(project, threadPool);
				pipeline->iterate_forward([&](RenderTarget&	target)
				{
					if(!target.is_collapsed()) on_render(project, threadPool, target);
				});
				on_end(project, threadPool);
			}
		}
	};
}