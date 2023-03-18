#pragma once


#include <dpl_Chain.h>
#include "complex_Utilities.h"


namespace complex
{
	class Dockable	: public dpl::Link<Dockable, Dockable>
					, public dpl::Chain<Dockable, Dockable>
	{
	private: // subtypes
		using MyParent		= dpl::Link<Dockable, Dockable>;
		using MyChildren	= dpl::Chain<Dockable, Dockable>;

	public: // data
		dpl::ReadOnly<ImGuiDir_,			Dockable> direction;
		dpl::ReadOnly<float,				Dockable> ratio;
		dpl::ReadOnly<ImGuiDockNodeFlags,	Dockable> flags;

	protected: // lifecycle
		CLASS_CTOR			Dockable()
			: direction(ImGuiDir_None)
			, ratio(1.f)
			, flags(ImGuiDockNodeFlags_None)
		{

		}

		CLASS_CTOR			Dockable(					const Dockable&						OTHER) = delete;

		CLASS_CTOR			Dockable(					Dockable&&							other) noexcept = default;

		Dockable&			operator=(					const Dockable&						OTHER) = delete;

		Dockable&			operator=(					Dockable&&							other) noexcept = default;

	public: // functions
		/*
			Add given dockpanel as a subnode of this one.
		*/
		void				dock_child(					Dockable&							child,
														const ImGuiDir_						DIRECTION,
														const float							RATIO);

	protected: // flags setup
		/*
		*	(Shared)
		*	Don't display the dockspace node but keep it alive. Windows docked into this dockspace node won't be undocked.
		*/
		inline void			set_KeepAliveOnly(			const bool							bVALUE)
		{
			set_node_flag(ImGuiDockNodeFlags_KeepAliveOnly, bVALUE);
		}

		/*
		*	(Shared)
		*	Disable docking inside the Central Node, which will be always kept empty.
		*/
		inline void			set_NoDockingInCentralNode(	const bool							bVALUE)
		{
			set_node_flag(ImGuiDockNodeFlags_NoDockingInCentralNode, bVALUE);
		}

		/*
		*	(Shared)
		*	Enable passthru dockspace: 
		*	1)	DockSpace() will render a ImGuiCol_WindowBg background covering everything excepted the Central Node when empty. 
		*		Meaning the host window should probably use SetNextWindowBgAlpha(0.0f) prior to Begin() when using this. 
		*
		*	2)	When Central Node is empty: let inputs pass-through + won't display a DockingEmptyBg background. See demo for details.
		*/
		inline void			set_PassthruCentralNode(	const bool							bVALUE)
		{
			set_node_flag(ImGuiDockNodeFlags_PassthruCentralNode, bVALUE);
		}

		/*
		*	(Shared)
		*	Disable splitting the node into smaller nodes. 
		*	Useful e.g. when embedding dockspaces into a main root one (the root one may have splitting disabled to reduce confusion). 
		*	Note: when turned off, existing splits will be preserved.
		*/
		inline void			set_NoSplit(				const bool							bVALUE)
		{
			set_node_flag(ImGuiDockNodeFlags_NoSplit, bVALUE);
		}

		/*
		*	(Shared)
		*	Disable resizing node using the splitter/separators. Useful with programatically setup dockspaces.
		*/
		inline void			set_NoDockResize(			const bool							bVALUE)
		{
			set_node_flag(ImGuiDockNodeFlags_NoResize, bVALUE);
		}

		/*
		*	(Shared/Local)
		*	Tab bar will automatically hide when there is a single window in the dock node.
		*/
		inline void			set_AutoHideTabBar(			const bool							bVALUE)
		{
			set_node_flag(ImGuiDockNodeFlags_AutoHideTabBar, bVALUE);
		}

		/*
		*	(Local, Saved)
		*	A dockspace is a node that occupy space within an existing user window. 
		*	Otherwise the node is floating and create its own window.
		*/
		inline void			set_DockSpace(				const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_DockSpace, bVALUE);
		}

		/*
		*	(Local, Saved)
		*	The central node has 2 main properties: stay visible when empty, only use "remaining" spaces from its neighbor.
		*/
		inline void			set_CentralNode(			const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_CentralNode, bVALUE);
		}

		/*
		*	(Local, Saved)
		*	Tab bar is completely unavailable. No triangle in the corner to enable it back(true by default).
		*/
		inline void			set_NoTabBar(				const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_NoTabBar, bVALUE);
		}

		/*
		*	(Local, Saved)
		*	Tab bar is hidden, with a triangle in the corner to show it again 
		*	(NB: actual tab-bar instance may be destroyed as this is only used for single-window tab bar)
		*/
		inline void			set_HiddenTabBar(			const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_HiddenTabBar, bVALUE);
		}

		/*
		*	(Local, Saved)
		*	Disable window/docking menu (that one that appears instead of the collapse button).
		*/
		inline void			set_NoWindowMenuButton(		const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_NoWindowMenuButton, bVALUE);
		}

		/*
		*	(Local, Saved)
		*	Disable close button.
		*/
		inline void			set_NoCloseButton(			const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_NoCloseButton, bVALUE);
		}

		/*
		*	(Local, Saved)  
		*	Disable any form of docking in this dockspace or individual node. 
		*	(On a whole dockspace, this pretty much defeat the purpose of using a dockspace at all). 
		*	Note: when turned on, existing docked nodes will be preserved.
		*/
		inline void			set_NoDocking(				const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_NoDocking, bVALUE);
		}

		/*
		*	[EXPERIMENTAL]
		*	Prevent another window/node from splitting this node.
		*/
		inline void			set_NoDockingSplitMe(		const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_NoDockingSplitMe, bVALUE);
		}

		/*
		*	[EXPERIMENTAL]
		*	Prevent this node from splitting another window/node.
		*/
		inline void			set_NoDockingSplitOther(	const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_NoDockingSplitOther, bVALUE);
		}

		/*
		*	[EXPERIMENTAL]
		*	Prevent another window/node to be docked over this node.
		*/
		inline void			set_NoDockingOverMe(		const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_NoDockingOverMe, bVALUE);
		}

		/*
		*	[EXPERIMENTAL]
		*	Prevent this node to be docked over another window/node.
		*/
		inline void			set_NoDockingOverOther(		const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_NoDockingOverOther, bVALUE);
		}

		/*
		*	[EXPERIMENTAL]
		*/
		inline void			set_NoResizeX(				const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_NoResizeX, bVALUE);
		}

		/*
		*	[EXPERIMENTAL]
		*/
		inline void			set_NoResizeY(				const bool							bVALUE)
		{
			set_private_node_flag(ImGuiDockNodeFlags_NoResizeY, bVALUE);
		}

	protected: // functions
		inline bool			get_node_flag(				const ImGuiDockNodeFlags_			FLAG) const
		{
			return flags() & FLAG;
		}

		inline bool			get_private_node_flag(		const ImGuiDockNodeFlagsPrivate_	FLAG) const
		{
			return flags() & FLAG;
		}

		void				update_docking(				ImGuiID&							dockID);

	private: // functions
		inline void			set_node_flag(				const ImGuiDockNodeFlags_			FLAG,
														const bool							bVALUE)
		{
			bVALUE	? *flags |= FLAG
					: *flags &= ~FLAG;
		}

		inline void			set_private_node_flag(		const ImGuiDockNodeFlagsPrivate_	FLAG,
														const bool							bVALUE)
		{
			bVALUE	? *flags |= FLAG
					: *flags &= ~FLAG;
		}

	private: // interface
		virtual const char*	get_target_name() const = 0;
	};
}