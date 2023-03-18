#pragma once 


#include "glw_SLStage.h"


namespace glw
{
	/*
		look@ line 130 (GL_PROGRAM_SEPARABLE) <-- Idea to detach vertex shader from fragment shader.
		https://github.com/GPUOpen-LibrariesAndSDKs/SPARSEtextures/blob/master/sample/main.cpp
	*/
	class SLProgram	: public dpl::Resource<SLProgram>
					, public dpl::ResourceUser<VertexStage>
					, public dpl::ResourceUser<FragmentStage>
					, public dpl::ResourceUser<GeometryStage>
	{
	private: // subtypes
		using MyVSH	= dpl::ResourceUser<VertexStage>;
		using MyFSH	= dpl::ResourceUser<FragmentStage>;
		using MyGSH	= dpl::ResourceUser<GeometryStage>;

	public: // subtypes
		using Design = std::function<void(SLProgram&)>;

	private: // data
		static GLuint	sm_currentProgramID;
		GLuint			m_id;	
		bool			bNeedsRebuild;

	private: // lifecycle
		CLASS_CTOR					SLProgram(				const dpl::Ownership&		OWNERSHIP,
															const Binding& 				BINDING);

	public: // lifecycle
		CLASS_CTOR					SLProgram(				const dpl::Ownership&		OWNERSHIP,
															const Binding& 				BINDING,
															const Design&				DESIGN);

		CLASS_CTOR					SLProgram(				const dpl::Ownership&		OWNERSHIP,
															SLProgram&&					other) noexcept;

		CLASS_DTOR					~SLProgram();

		SLProgram&					operator=(				dpl::Swap<SLProgram>		other);

	private: // lifecycle
		CLASS_CTOR					SLProgram(				SLProgram&&					other) noexcept = delete;

		CLASS_CTOR					SLProgram(				const SLProgram&			OTHER) = delete;

		SLProgram&					operator=(				SLProgram&&					other) noexcept = delete;

		SLProgram&					operator=(				const SLProgram&			OTHER) = delete;

	public: // functions
		/*
			Returns false if program was already binded.
		*/
		bool						bind();

		/*
			Returns false if default program was already binded.
		*/
		static bool					bind_default();

		void						set_attribute_location(	const std::string&			NAME,
															const int32_t				location);

		int32_t						get_attribute_location(	const std::string&			NAME) const;

		void						get_attribute_list(		std::vector<std::string>&	names);

		void						set_fragData_location(	const std::string&			NAME,
															const int32_t				location);

		int32_t						get_fragData_location(	const std::string&			NAME) const;

		int32_t						get_uniform_location(	const std::string&			NAME) const;

		void						get_uniform_list(		std::vector<std::string>&	names);

	private: // functions
		void						generate();

		void						attach_shader(			SLStage&					shader);

		void						attach_shaders();

		void						link_shaders();

		void						release_glObj();

		std::string					get_program_info();

		void						rebuild();

		inline void					force_rebuild()
		{
			release_glObj();
			bNeedsRebuild = true;
		}

	private: // implementation
		virtual void				on_observe(				VertexStage&				shader) final override
		{
			force_rebuild();
		}

		virtual void				on_update(				VertexStage&				shader) final override
		{
			force_rebuild();
		}

		virtual void				on_subject_lost(		const VertexStage*			DUMMY,
															const uint32_t				INDEX) final override
		{
			force_rebuild();
		}

		virtual void				on_observe(				FragmentStage&				shader) final override
		{
			force_rebuild();
		}

		virtual void				on_update(				FragmentStage&				shader) final override
		{
			force_rebuild();
		}

		virtual void				on_subject_lost(		const FragmentStage*		DUMMY,
															const uint32_t				INDEX) final override
		{
			force_rebuild();
		}

		virtual void				on_observe(				GeometryStage&				shader) final override
		{
			force_rebuild();
		}

		virtual void				on_update(				GeometryStage&				shader) final override
		{
			force_rebuild();
		}

		virtual void				on_subject_lost(		const GeometryStage*		DUMMY,
															const uint32_t				INDEX) final override
		{
			force_rebuild();
		}
	};
}