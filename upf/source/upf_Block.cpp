#include "..//include/upf_Block.h"

#pragma warning( disable : 26451 ) // arithmetic overflow

// lifecycle
namespace upf
{
	CLASS_CTOR		Block::Block()
	{
		buckets->set_empty_key(INVALID_INDEX64);
	}

	CLASS_CTOR		Block::Block(			Block&&			other) noexcept
		: blockMask(other.blockMask)
		, proxies(std::move(other.proxies))
		, buckets(std::move(other.buckets))
	{
				
	}

	Block&			Block::operator=(		Block&&			other) noexcept
	{
		blockMask	= other.blockMask;
		proxies		= std::move(other.proxies);
		buckets		= std::move(other.buckets);
		return *this;
	}
}

// public functions
namespace upf
{
	void			Block::initialize(		const Voxel		BLOCK_MASK,
											const Series&	SERIES)
	{
		blockMask	= BLOCK_MASK;
		proxies		= std::make_unique<Proxy[]>(SERIES.size());	
		buckets->reserve(SERIES.size());
	}

	void			Block::update(			const Series&	SERIES)
	{
		// We use semi bucket sort to move similar proxies closer together.
		// https://www.bigocheatsheet.com/

		buckets->clear();

		Voxel proxyCoords;

		for(uint32_t boxID = 0; boxID < SERIES.size(); ++boxID)
		{
			auto& proxy	= proxies->get()[boxID];
			const auto& BOX = *reinterpret_cast<const Box*>(SERIES.objects() + SERIES.offset() + boxID * SERIES.stride());
			if(BOX.is_disabled())
			{
				proxy.nextProxyID = Box::INVALID_ID;
				continue;
			}
			
			// Check which coordinates are different.
			const bool	HX_ODD	= BOX.voxel.hx%2;
			const bool	HY_ODD	= BOX.voxel.hy%2;
			const bool	V_ODD	= BOX.voxel.v%2;

			// Calculate new coordinates of the proxy.
			proxyCoords = BOX.voxel;
			if(blockMask().hx%2 != HX_ODD)	proxyCoords.hx += (BOX.voxel.get_flag(Box::HX_GREATER) ? 1 : -1);
			if(blockMask().hy%2 != HY_ODD)	proxyCoords.hy += (BOX.voxel.get_flag(Box::HY_GREATER) ? 1 : -1);
			if(blockMask().v%2 != V_ODD)	proxyCoords.v  += (BOX.voxel.get_flag(Box::V_GREATER) ? 1 : -1);

			// Get coordinates masks.
			const auto	BOX_COORDS_MASK		= BOX.voxel.to_coords_mask();
			const auto	PROXY_COORDS_MASK	= proxyCoords.to_coords_mask();

			// Set proxy flags.
			const bool	IS_MAIN = PROXY_COORDS_MASK == BOX_COORDS_MASK;
			proxy.type	= (IS_MAIN? MAIN_PROXY : 0)
						| (HX_ODD? Box::HX_GREATER : 0)
						| (HY_ODD? Box::HY_GREATER : 0)
						| (V_ODD? Box::V_GREATER : 0);

			// Attach proxy to the coordinates bucket.
			auto& list	= (*buckets)[PROXY_COORDS_MASK];
			auto& index = IS_MAIN ? list.mainIndex : list.adjacentIndex;
			proxy.nextProxyID	= index;
			index				= boxID;
		}
	}
}

// private functions
namespace upf
{
	void			Block::release()
	{
		proxies->reset();
		buckets->resize(0);
	}
}