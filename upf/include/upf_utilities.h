#pragma once

// std
#include <atomic>
#include <stdint.h>
#include <limits>
#include <functional>

// dpl
#include <dpl_ReadOnly.h>
#include <dpl_GeneralException.h>
#include <dpl_Values.h>
#include <dpl_ThreadPool.h>

// cml
#include <cml.h>
#pragma comment(lib, "cml.lib")


namespace upf // uniform pair finder
{
	class Voxel;
	class Block;
	class SpatialDivision;
}