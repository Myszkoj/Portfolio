#pragma once


#include "fqs_Settings.h"
#include "fqs_Queuer.h"
#include "fqs_Section.h"


namespace fqs
{
	class Organizer
	{
	public: // subtypes
		using	Task	= std::function<void()>;

	public: // data
		dpl::ReadOnly<Settings, Organizer>	settings;

	private: // data
		dpl::DynamicArray<Queuer>			queuers;
		dpl::DynamicArray<Section>			sections;

	public: // lifecycle
		CLASS_CTOR				Organizer() = default;

	public: // functions
		inline Queuer*			get_queuers()
		{
			return queuers.data();
		}

		inline const Queuer*	get_queuers() const
		{
			return queuers.data();
		}

		void					initialize(				const Settings&			NEW_SETTINGS);

		void					update(					dpl::ThreadPool&		threadPool);

		void					update(					dpl::ParallelPhase*		threadPool);

	private: // functions
		inline uint32_t			calculate_sectionID(	const uint32_t			DISTANCE_IN_CM) const
		{
			return std::min(settings().sectionsPerQ - 1, DISTANCE_IN_CM / settings().sectionLength);
		}

		inline Section&			get_section(			const uint32_t			QID,
														const uint32_t			SECTION_ID)
		{
			settings().validate_QID(QID);
			settings().validate_SECTION_ID(SECTION_ID);
			return sections.data()[QID * settings().sectionsPerQ + SECTION_ID];
		}

		void					sort_section(			Section&				section);

		void					update_queue(			const uint32_t			QID);

	private: // update steps (standard threadpool)
		void					clear_sections(			dpl::ThreadPool&		threadPool,
														const uint32_t			NUM_SPLITS);

		void					insert_queuers(			dpl::ThreadPool&		threadPool,
														const uint32_t			NUM_SPLITS);

		void					sort_sections(			dpl::ThreadPool&		threadPool,
														const uint32_t			NUM_SPLITS);

		void					update_queues(			dpl::ThreadPool&		threadPool,
														const uint32_t			NUM_SPLITS);

	private: // update steps (parallel phase)
		void					clear_sections(			dpl::ParallelPhase*		threadPool);

		void					insert_queuers(			dpl::ParallelPhase*		threadPool);

		void					sort_sections(			dpl::ParallelPhase*		threadPool);

		void					update_queues(			dpl::ParallelPhase*		threadPool);
	};
}