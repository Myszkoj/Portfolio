#pragma once


#include "upf_Series.h"
#include "upf_Block.h"
#include <array>


namespace upf
{
	/*
		Assigns each person into 3-dimensional cells (8 per person).
	*/
	class	SpatialDivision
	{
	public: // constants
		static const uint32_t	NUM_PROXIES_PER_BOX	= 8;	// Each box is represented by 2x2x2 cluster of proxies.
		
	public: // subtypes
		using	Blocks		= std::array<Block, NUM_PROXIES_PER_BOX>;
		using	TestPair	= std::function<void(void*, void*)>;

	public: // data
		dpl::ReadOnly<Series,	SpatialDivision>	series;
		dpl::ReadOnly<Blocks,	SpatialDivision>	blocks;
		dpl::ReadOnly<bool,		SpatialDivision>	logPairGeneration;

	public: // lifecycle
		CLASS_CTOR		SpatialDivision() 
			: logPairGeneration(false)
		{

		}

	public: // functions
		void			initialize(			const Series&		SERIES,
											const bool			LOG_PAIR_GENERATION = false);

		void			update(				dpl::ParallelPhase*	threadPool,
											const TestPair&		TEST_PAIR);

	private: // update stages
		void			update_boxes(		dpl::ParallelPhase*	threadPool);

		void			update_blocks(		dpl::ParallelPhase*	threadPool);

		void			find_pairs(			dpl::ParallelPhase*	threadPool,
											const TestPair&		TEST_PAIR);
	};
}