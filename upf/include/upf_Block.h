#pragma once


#include <sparsehash/dense_hash_map.h>
#include <memory>
#include "upf_Voxel.h"
#include "upf_Series.h"


#pragma pack(push, 1)
namespace upf
{
	class	SpatialDivision;


	/*
		Stores voxels of unique unit type [V][Hy][Hx], where each of those values is either 0 or 1.
		Another way of looking at it, is that block may store voxels with the same combination of even or odd values for each coordinate.
	*/
	class	Block
	{
	public: // relations
		friend SpatialDivision;

	public: // constants
		static const auto MAIN_PROXY		= Voxel::FLAG_A; // If true, proxy does not contain center of the territory.
		static const auto INVALID_INDEX64	= std::numeric_limits<uint64_t>::max();

	public: // subtypes
		class	Proxy
		{
		public: // relations
			friend Block;

		public: // data
			uint32_t	type		: Box::NUM_TYPE_BITS; //<-- Contains type of the proxy and the box.
			uint32_t	nextProxyID : Box::NUM_ID_BITS;

		public: // lifecycle
			CLASS_CTOR			Proxy()
				: type(0)
				, nextProxyID(Box::INVALID_ID)
			{
				
			}

		public: // functions
			inline bool			is_main() const
			{
				return (type & MAIN_PROXY) > 0;
			}

			inline uint32_t		direction() const
			{
				return type & Box::DIRECTION_FLAGS;
			}

			// This function assumes that OTHER proxy is main.
			inline bool			can_pair_with(	const Proxy&	OTHER) const
			{
				return OTHER.is_main()? true : ((this->direction()) < OTHER.direction());
			}
		};

		struct	ProxyList
		{
			uint32_t	mainIndex		= Box::INVALID_ID;
			uint32_t	adjacentIndex	= Box::INVALID_ID;
		};

		using	CoordHash	= uint64_t; // (Voxel::value & m_blockMask)
		using	Proxies		= std::unique_ptr<Proxy[]>;
		using	Buckets		= google::dense_hash_map<CoordHash, ProxyList>;

		class	Iterator
		{
		public: // subtypes
			using ConstPair = std::pair<const CoordHash, ProxyList>;

		private: // data
			const ConstPair* pos;
			const ConstPair* end;

		public: // lifecycle
			CLASS_CTOR			Iterator(	const ConstPair*	POS,
											const ConstPair*	END)
				: pos(POS)
				, end(END)
			{
				skip_empty();
			}

		public: // operators
			inline auto			operator*() const 
			{ 
				return *pos; 
			}

			inline auto			operator->() const 
			{ 
				return pos; 
			}

			inline bool			operator==(	const Iterator&		OTHER) const 
			{ 
				return pos == OTHER.pos; 
			}

			inline bool			operator!=(	const Iterator&		OTHER) const 
			{ 
				return pos != OTHER.pos; 
			}

			inline Iterator&	operator++() 
			{
				++pos;
				skip_empty();
				return *this;
			}

		private: // functions
			inline void			skip_empty()
			{
				while((pos < end) && (pos->first == INVALID_INDEX64)) ++pos;
			}
		};

	private: // data
		dpl::ReadOnly<Voxel,	Block>	blockMask;
		dpl::ReadOnly<Proxies,	Block>	proxies;
		dpl::ReadOnly<Buckets,	Block>	buckets;

	public: // lifecycle
		CLASS_CTOR			Block();

		CLASS_CTOR			Block(			const Block&		OTHER) = delete;

		CLASS_CTOR			Block(			Block&&				other) noexcept;

		Block&				operator=(		const Block&		OTHER) = delete;

		Block&				operator=(		Block&&				other) noexcept;

	public: // functions
		void				initialize(		const Voxel			BLOCK_MASK,
											const Series&		SERIES);

		inline const Proxy* get() const
		{
			return proxies().get();
		}

		inline auto			begin(			const uint32_t		RANGE_BEGIN,
											const uint32_t		RANGE_END) const
		{
			return Iterator(buckets().begin(RANGE_BEGIN).pos, buckets().begin(RANGE_END).pos);
		}

		inline auto			end(			const uint32_t 		RANGE_END) const
		{
			return Iterator(buckets().begin(RANGE_END).pos, buckets().begin(RANGE_END).pos);
		}

		inline uint32_t		numBuckets() const
		{
			return (uint32_t)buckets().bucket_count(); //<-- Should never be larger than uint32_t::max.
		}

		void				update(			const Series&		SERIES);

	private: // functions
		void				release();
	};
}
#pragma pack(pop)