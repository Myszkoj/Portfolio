#include "..//include/glw_FrameBuffer.h"

#define INVALID_FRAMEBUFFER_ID 0

namespace glw
{
	struct Bindings
	{
		GLuint read		= INVALID_FRAMEBUFFER_ID;
		GLuint write	= INVALID_FRAMEBUFFER_ID;
	} bindings;


//=====> FrameBuffer public: // lifecycle
	CLASS_CTOR		FrameBuffer::FrameBuffer(				const Resolution						RESOLUTION)
		: resolution(RESOLUTION)
		, glID(INVALID_FRAMEBUFFER_ID)
		, bNeedsRebuild(true)
	{
		generate();
	}

	CLASS_CTOR		FrameBuffer::FrameBuffer(				FrameBuffer&&							other) noexcept
		: MyLayers(std::move(other))
		, resolution(other.resolution)
		, glID(other.glID)
		, bNeedsRebuild(other.bNeedsRebuild)
	{
		other.glID = INVALID_FRAMEBUFFER_ID;
	}
	
	CLASS_DTOR		FrameBuffer::~FrameBuffer()
	{
		release_glObj();
	}

	FrameBuffer&	FrameBuffer::operator=(					FrameBuffer&&							other) noexcept
	{
		if(this != &other)
		{
			release_glObj();

			MyLayers::operator=(std::move(other));
			resolution		= other.resolution;
			glID			= other.glID;
			bNeedsRebuild	= other.bNeedsRebuild;

			other.glID = INVALID_FRAMEBUFFER_ID;
		}

		return *this;
	}

//=====> FrameBuffer public: // functions
	void			FrameBuffer::invalidate_bindings()
	{
		bindings.read	= INVALID_FRAMEBUFFER_ID;
		bindings.write	= INVALID_FRAMEBUFFER_ID;
	}

	void			FrameBuffer::clear()
	{
		bind(Mode::eREAD_WRITE);

		set_draw_buffers([&](DrawBuffers&	drawBuffers)
		{
			MyLayers::for_each_variant([&](FragmentLayer& layer)
			{
				if(layer.is_color_buffer())
				{
					const uint32_t INDEX = layer.attachment() - GL_COLOR_ATTACHMENT0;
					drawBuffers[INDEX] = layer.attachment();
				}
			});

			return true;
		});
			
		glViewport(0, 0, resolution().x, resolution().y);

		MyLayers::for_each_variant([](FragmentLayer& layer)
		{
			layer.clear();
		});

		validate_opengl(__FILE__, __LINE__, "Fail to clear.");
	}

//=====> FrameBuffer private: // functions
	void			FrameBuffer::bind_framebuffer(			const Mode								MODE,
															const GLuint							FRAMEBUFFER_ID)
	{
		switch(MODE)
		{
		case Mode::eREAD:		
			if(bindings.read != FRAMEBUFFER_ID)
			{
				bindings.read = FRAMEBUFFER_ID;
				glBindFramebuffer(Mode::eREAD, FRAMEBUFFER_ID);
			}
			break;

		case Mode::eWRITE:		
			if(bindings.write != FRAMEBUFFER_ID)
			{
				bindings.write = FRAMEBUFFER_ID;
				glBindFramebuffer(Mode::eWRITE,	FRAMEBUFFER_ID);
			}
			break;

		case Mode::eREAD_WRITE: 
			if(bindings.read != FRAMEBUFFER_ID || bindings.write != FRAMEBUFFER_ID)
			{
				bindings.read	= FRAMEBUFFER_ID;
				bindings.write	= FRAMEBUFFER_ID;
				glBindFramebuffer(Mode::eREAD_WRITE, FRAMEBUFFER_ID);
			}
			break;

		default: throw dpl::GeneralException(__FILE__, __LINE__, "Unknown mode: " + std::to_string(MODE));
		}

		validate_opengl(__FILE__, __LINE__, "Fail to bind.");
	}

	bool			FrameBuffer::set_draw_buffers(			const std::function<bool(DrawBuffers&)>	FILL) const
	{
		static DrawBuffers	drawBuffers;
							drawBuffers.fill(GL_NONE);

		const bool bALL_BINDED = FILL(drawBuffers);
		glDrawBuffers(NUM_COLOR_BUFFERS, drawBuffers.data());
		validate_opengl(__FILE__, __LINE__, "Fail to draw buffers.");
		return bALL_BINDED;
	}

	void			FrameBuffer::update() const
	{
		if(bNeedsRebuild)
		{
			if(glID() == 0)
				throw dpl::GeneralException(this, __LINE__, "Unable to complete. Framebuffer is invalid.");

			MyLayers::for_each_variant([&](const FragmentLayer& LAYER)
			{
				LAYER.update(resolution());
			});

			glBindFramebuffer(GL_FRAMEBUFFER, glID);

			MyLayers::for_each_variant([&](const FragmentLayer& LAYER)
			{
				LAYER.bind_for_reading(0);

				if(LAYER.type() == GL_TEXTURE_CUBE_MAP)
				{
					if(LAYER.is_depth_attachment())
					{
						glFramebufferTexture(GL_FRAMEBUFFER, LAYER.attachment, LAYER.GL_ID(), 0);
					}
					else
					{
						for (uint32_t n = 0; n < 6; ++n)
							glFramebufferTexture2D(GL_FRAMEBUFFER, LAYER.attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, LAYER.GL_ID(), 0);
					}
				}
				else
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER, LAYER.attachment, LAYER.type(), LAYER.GL_ID(), 0);
				}

				validate_opengl(__FILE__, __LINE__, "Invalid use of glFramebufferTexture functions.");
			});

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			switch(status)
			{
			case GL_FRAMEBUFFER_COMPLETE:
				break;

			case GL_FRAMEBUFFER_UNDEFINED:
				throw dpl::GeneralException(this, __LINE__, "Fail to complete: GL_FRAMEBUFFER_UNDEFINED");
		
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				throw dpl::GeneralException(this, __LINE__, "Fail to complete: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				throw dpl::GeneralException(this, __LINE__, "Fail to complete: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				throw dpl::GeneralException(this, __LINE__, "Fail to complete: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
		
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				throw dpl::GeneralException(this, __LINE__, "Fail to complete: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");

			case GL_FRAMEBUFFER_UNSUPPORTED:
				throw dpl::GeneralException(this, __LINE__, "Fail to complete: GL_FRAMEBUFFER_UNSUPPORTED");

			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				throw dpl::GeneralException(this, __LINE__, "Fail to complete: GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");

			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				throw dpl::GeneralException(this, __LINE__, "Fail to complete: GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");

			default:
				throw dpl::GeneralException(this, __LINE__, "Fail to complete: UNKNOW_ERROR");
			}

			bNeedsRebuild = false;
		}
	}

	void			FrameBuffer::generate()
	{
		if(glID() == 0)
		{
			glGenFramebuffers(1, &*glID);

			if(glID() == 0)
				throw dpl::GeneralException(this, __LINE__, "Fail to create FrameBuffer Object.");
		}
	}

	void			FrameBuffer::release_glObj()
	{
		if(glID() != 0)
		{
			if(bindings.read == glID)	bindings.read	= INVALID_FRAMEBUFFER_ID;
			if(bindings.write == glID)	bindings.write	= INVALID_FRAMEBUFFER_ID;
			glDeleteFramebuffers(1, &*glID);
			glID = 0;
		}
	}
}