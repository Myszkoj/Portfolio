#pragma once


#include <dpl_StaticHolder.h>
#include "glw_Buffer.h"
#include "glw_VertexTopology.h"


namespace glw
{
	class VAO : private dpl::StaticHolder<GLuint, VAO>
	{
	private: // subtypes
		using	CurrentID	= dpl::StaticHolder<GLuint, VAO>;

	public: // subtypes
		using	Action		= std::function<void()>;

	private: // data
		dpl::ReadOnly<GLuint, VAO> ID;

	public: // lifecycle
		CLASS_CTOR			VAO()
			: ID(0)
		{
			glGenVertexArrays(1, &*ID);
			validate_opengl(__FILE__, __LINE__, "Fail to generate VertexArray.");
		}

		CLASS_CTOR			VAO(			VAO&&			other) noexcept
			: ID(other.ID)
		{
			other.ID = 0;
		}

		CLASS_DTOR			~VAO()
		{
			dpl::no_except([&]()
			{
				release();
			});	
		}

		VAO&				operator=(		VAO&&			other) noexcept
		{
			ID.swap(other.ID);
			other.release();
			return *this;
		}

	private: // lifecycle (deleted)
		CLASS_CTOR			VAO(			const VAO&		OTHER) = delete;
		VAO&				operator=(		const VAO&		OTHER) = delete;

	public: // functions
		void				bind()
		{
			if(CurrentID::data == ID) return;
			throw_if_invalid_ID();
			glBindVertexArray(ID);
			CurrentID::data = ID;
		}

		static void			bind_default()
		{
			if(CurrentID::data == 0) return;
			glBindVertexArray(0);
			CurrentID::data = 0;
		}

		inline void			enclose_action(	const Action	ACTION)
		{
			bind();
			ACTION();
			validate_opengl(__FILE__, __LINE__, "Action failed.");
			bind_default();
		}

	private: // functions
		void				release()
		{
			if(ID == 0u) return;		
			if(ID == CurrentID::data) bind_default();
			glDeleteVertexArrays(1, &*ID);
			ID = 0u;
		}

		inline void			throw_if_invalid_ID() const
		{
			if(ID != 0u) return;
			throw dpl::GeneralException(this, __LINE__, "Fail to bind. VertexArray is invalid.");
		}
	};
}