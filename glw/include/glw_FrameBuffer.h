#pragma once


#include "glw_ColorLayer.h"
#include "glw_DepthLayer.h"


namespace glw
{
	class FrameBuffer	: public dpl::Variation<FrameBuffer, FragmentLayer>
	{
	public: // subtypes
		enum	Mode
		{
			eREAD		= GL_READ_FRAMEBUFFER,
			eWRITE		= GL_DRAW_FRAMEBUFFER,
			eREAD_WRITE = GL_FRAMEBUFFER
		};

	private: // subtypes
		static const uint32_t NUM_COLOR_BUFFERS = 8;
		using	DrawBuffers	= std::array<GLenum, NUM_COLOR_BUFFERS>;

		using	MyLayers	= Variation<FrameBuffer, FragmentLayer>;
		using	Resolution	= RenderableTexture::Resolution;

	public: // data
		dpl::ReadOnly<Resolution,	FrameBuffer> resolution;

	private: // data
		dpl::ReadOnly<GLuint,		FrameBuffer> glID;
		mutable dpl::ReadOnly<bool,	FrameBuffer> bNeedsRebuild;

	public: // lifecycle
		CLASS_CTOR					FrameBuffer(				const Resolution						RESOLUTION = RenderableTexture::MIN_RESOLUTION);

		CLASS_CTOR					FrameBuffer(				const FrameBuffer&						OTHER) = delete;

		CLASS_CTOR					FrameBuffer(				FrameBuffer&&							other) noexcept;

		CLASS_DTOR					~FrameBuffer();

		FrameBuffer&				operator=(					const FrameBuffer&						OTHER) = delete;

		FrameBuffer&				operator=(					FrameBuffer&&							other) noexcept;

	public: // functions
		inline void					set_resolution(				const uint32_t							NEW_WIDTH,
																const uint32_t							NEW_HEIGHT)
		{
			if(resolution().x != NEW_WIDTH || resolution().y != NEW_HEIGHT)
			{
				resolution->x	= NEW_WIDTH;
				resolution->y	= NEW_HEIGHT;
				bNeedsRebuild	= true;
			}
		}

		inline void					set_resolution(				const Resolution&						NEW_RESOLUTION)
		{
			set_resolution(NEW_RESOLUTION.x, NEW_RESOLUTION.y);
		}

		/*
			Returns true if width or height is equal 0.
		*/
		inline bool					is_collapsed() const
		{
			return resolution().x == 0 || resolution().y == 0;
		}

		template<typename LayerT>
		inline bool					create_layer()
		{
			if(MyLayers::create_variant<LayerT>()) bNeedsRebuild = true;
			return bNeedsRebuild;
		}

		template<typename LayerT>
		inline bool					destroy_layer()
		{
			bNeedsRebuild = MyLayers::destroy_variant<LayerT>();
			return bNeedsRebuild;
		}

		inline bool					destroy_all_layers()
		{
			bNeedsRebuild = MyLayers::destroy_all_variants();
			return bNeedsRebuild;
		}

		inline FragmentLayer*		find_layer(					const uint32_t							TYPE_ID)
		{
			return MyLayers::find_base_variant(TYPE_ID);
		}

		inline const FragmentLayer*	find_layer(					const uint32_t							TYPE_ID) const
		{
			return MyLayers::find_base_variant(TYPE_ID);
		}

		template<typename LayerT>
		inline LayerT*				find_layer()
		{
			return MyLayers::find_variant<LayerT>();
		}

		template<typename LayerT>
		inline const LayerT*		find_layer() const
		{
			return MyLayers::find_variant<LayerT>();
		}

		template<typename T>
		inline bool					has_layer()
		{
			return find_layer<T>() != nullptr;
		}

		template<typename T1, typename T2, typename... Ts>
		inline bool					has_layer()
		{
			if(!has_layer<T1>()) return false;
			return has_layer<T2, Ts...>();
		}

		template<typename T1, typename T2>
		inline bool					swap_layers()
		{
			if(FragmentLayer* first = find_layer<T1>())
			{
				if(FragmentLayer* second = find_layer<T2>())
				{
					if(first != second)
					{
						first->swap_textures(*second);
						return true;
					}
				}
			}

			return false;
		}

		inline void					read() const
		{
			bind(Mode::eREAD);
		}

		template<typename... Ts>
		inline void					write() const
		{
			bind(Mode::eWRITE);
			draw_buffers<Ts...>();
		}

		template<typename... Ts>
		inline void					read_write() const
		{
			bind(Mode::eREAD_WRITE);
			draw_buffers<Ts...>();
		}

		template<typename T>
		inline void					read_layer(					const Texture::Unit						UNIT) const
		{
			const FragmentLayer* LAYER = find_layer<T>();
			if(!LAYER)
				throw dpl::GeneralException(this, __LINE__, "Given layer is missing: " + std::string(typeid(T).name()));

			LAYER->bind_for_reading(UNIT);
		}

		/*
			Bind render buffer of the window API.
		*/
		static inline void			bind_default(				const Mode								MODE)
		{
			bind_framebuffer(MODE, 0);
		}

		static void					invalidate_bindings();

		void						clear();

	private: // functions
		inline void					bind(						const Mode								MODE) const
		{
			update();
			bind_framebuffer(MODE, glID);
		}

		static void					bind_framebuffer(			const Mode								MODE,
																const GLuint							FRAMEBUFFER_ID);

		template<typename T>
		inline bool					draw_buffer(				DrawBuffers&							drawBuffers) const
		{
			if(const FragmentLayer* LAYER = find_layer<T>())
			{
				if(LAYER->is_color_buffer())
				{
					const uint32_t INDEX = LAYER->attachment() - GL_COLOR_ATTACHMENT0;
					drawBuffers[INDEX] = LAYER->attachment();
					return true;
				}
			}

			return false;
		}

		template<typename T1, typename T2, typename... Ts>
		inline bool					draw_buffer(				DrawBuffers&							drawBuffers) const
		{
			bool bResult = true;
			if(!draw_buffer<T1>(drawBuffers))			bResult = false;
			if(!draw_buffer<T2, Ts...>(drawBuffers))	bResult = false;
			return bResult;
		}

		template<typename... Ts>
		inline bool					draw_buffers() const
		{
			return set_draw_buffers([&](DrawBuffers& drawBuffers)
			{
				return draw_buffer<Ts...>(drawBuffers);
			});
		}

		bool						set_draw_buffers(			const std::function<bool(DrawBuffers&)>	FILL) const;

		void						update() const;

		void						generate();

		void						release_glObj();
	};
}