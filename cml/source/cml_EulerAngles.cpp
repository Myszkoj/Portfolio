#include "..//include/cml_EulerAngles.h"


namespace cml
{
	namespace YPR_to_Quat
	{
		/*
			Stores result of finding formula to transform from yaw->pitch->roll angles(with that order) to quaternion.
		*/
		struct TestResult
		{
			Quat		quat			= Quat(1.f, 0.f, 0.f, 0.f);
			float		smallest_error	= std::numeric_limits<float>::max();

			// XYZ axis swap(0-no_swap, 1-XY, 2-XZ, 3-YZ, 4-WX, 5-WY, 6-WZ).
			uint32_t	swapID	  = 0;

			// Angle Swaps.
			int32_t		angleSwap = -1; // -1(NO_SWAP), 0(YP), 1(PR), 2(RY)

			// Term Factor ID
			float		wl_factor = 1.f;
			float		xl_factor = 1.f;
			float		yl_factor = 1.f;
			float		zl_factor = 1.f;

			float		wr_factor = 1.f;
			float		xr_factor = 1.f;
			float		yr_factor = 1.f;
			float		zr_factor = 1.f;

			uint32_t	numTests		= 0;
		};

		float			find_similarity(		const Quat& ORIGINAL, 
												const Quat& IMITATION)
		{
			return glm::abs(ORIGINAL.w - IMITATION.w)
				 + glm::abs(ORIGINAL.x - IMITATION.x)
				 + glm::abs(ORIGINAL.y - IMITATION.y)
				 + glm::abs(ORIGINAL.z - IMITATION.z);
		}

		float			find_swapped(			const Quat&		ORIGINAL, 
												const Quat&		IMITATION,
												TestResult&		result)
		{
			bool bSuccess = false;

			const Quat XY_SWAP = Quat(IMITATION.w, IMITATION.y, IMITATION.x, IMITATION.z);
			const Quat XZ_SWAP = Quat(IMITATION.w, IMITATION.z, IMITATION.y, IMITATION.x);
			const Quat YZ_SWAP = Quat(IMITATION.w, IMITATION.x, IMITATION.z, IMITATION.y);
			const Quat WX_SWAP = Quat(IMITATION.x, IMITATION.w, IMITATION.y, IMITATION.z);
			const Quat WY_SWAP = Quat(IMITATION.y, IMITATION.x, IMITATION.w, IMITATION.z);
			const Quat WZ_SWAP = Quat(IMITATION.z, IMITATION.x, IMITATION.y, IMITATION.z);

			const float ERROR_0 = find_similarity(ORIGINAL, IMITATION);
			const float ERROR_1 = find_similarity(ORIGINAL, XY_SWAP);
			const float ERROR_2 = find_similarity(ORIGINAL, XZ_SWAP);
			const float ERROR_3 = find_similarity(ORIGINAL, YZ_SWAP);
			const float ERROR_4 = find_similarity(ORIGINAL, WX_SWAP);
			const float ERROR_5 = find_similarity(ORIGINAL, WY_SWAP);
			const float ERROR_6 = find_similarity(ORIGINAL, WZ_SWAP);

			if(ERROR_0 < result.smallest_error)
			{
				result.swapID			= 0;
				result.smallest_error	= ERROR_0;
				result.quat				= IMITATION;
				bSuccess = true;
			}

			if(ERROR_1 < result.smallest_error)
			{
				result.swapID			= 1;
				result.smallest_error	= ERROR_1;
				result.quat				= XY_SWAP;
				bSuccess = true;
			}

			if(ERROR_2 < result.smallest_error)
			{
				result.swapID			= 2;
				result.smallest_error	= ERROR_2;
				result.quat				= XZ_SWAP;
				bSuccess = true;
			}

			if(ERROR_3 < result.smallest_error)
			{
				result.swapID			= 3;
				result.smallest_error	= ERROR_3;
				result.quat				= YZ_SWAP;
				bSuccess = true;
			}

			if(ERROR_4 < result.smallest_error)
			{
				result.swapID			= 4;
				result.smallest_error	= ERROR_4;
				result.quat				= WX_SWAP;
				bSuccess = true;
			}

			if(ERROR_5 < result.smallest_error)
			{
				result.swapID			= 5;
				result.smallest_error	= ERROR_5;
				result.quat				= WY_SWAP;
				bSuccess = true;
			}

			if(ERROR_6 < result.smallest_error)
			{
				result.swapID			= 6;
				result.smallest_error	= ERROR_6;
				result.quat				= WZ_SWAP;
				bSuccess = true;
			}

			result.numTests += 6;

			return bSuccess;
		}

		bool			find_term_factors(		const Quat&		ORIGINAL,
												const Vec3&		YPR,
												TestResult&		result)
		{
			const Vec3 c = glm::cos(YPR * 0.5f);
			const Vec3 s = glm::sin(YPR * 0.5f);
		
			const float W_L = c.x * c.y * c.z;		const float W_R = s.x * s.y * s.z;
			const float X_L = s.x * c.y * c.z;		const float X_R = c.x * s.y * s.z;
			const float Y_L = c.x * s.y * c.z;		const float Y_R = s.x * c.y * s.z;
			const float Z_L = c.x * c.y * s.z;		const float Z_R = s.x * s.y * c.z;

			// ORIGINAL TERMS
			//const float W = W_L + W_R;
			//const float X = X_L - X_R;
			//const float Y = Y_L + Y_R;
			//const float Z = Z_L - Z_R;

			uint32_t numUpdates = 0;

			for(uint32_t wl = 0; wl < 2; ++wl)
			{for(uint32_t wr = 0; wr < 2; ++wr)
			{
				const float wl_value = W_L * ((wl==0)? -1.f : 1.f);
				const float wr_value = W_R * ((wr==0)? -1.f : 1.f);

				for(uint32_t xl = 0; xl < 2; ++xl)
				{for(uint32_t xr = 0; xr < 2; ++xr)
				{
					const float xl_value = X_L * ((xl==0)? -1.f : 1.f);
					const float xr_value = X_R * ((xr==0)? -1.f : 1.f);

					for(uint32_t yl = 0; yl < 2; ++yl)
					{for(uint32_t yr = 0; yr < 2; ++yr)
					{
						const float yl_value = Y_L * ((yl==0)? -1.f : 1.f);
						const float yr_value = Y_R * ((yr==0)? -1.f : 1.f);

						for(uint32_t zl = 0; zl < 2; ++zl)
						{for(uint32_t zr = 0; zr < 2; ++zr)
						{
							const float zl_value = Z_L * ((zl==0)? -1.f : 1.f);
							const float zr_value = Z_R * ((zr==0)? -1.f : 1.f);

							const Quat TEST(	wl_value + wr_value,
												xl_value - xr_value,
												yl_value + yr_value,
												zl_value - zr_value);

							if(find_swapped(ORIGINAL, TEST, result))
							{
								result.wl_factor = ((wl==0)? -1.f : 1.f);
								result.xl_factor = ((xl==0)? -1.f : 1.f);
								result.yl_factor = ((yl==0)? -1.f : 1.f);
								result.zl_factor = ((zl==0)? -1.f : 1.f);

								result.wr_factor = ((wr==0)? -1.f : 1.f);
								result.xr_factor = ((xr==0)? -1.f : 1.f);
								result.yr_factor = ((yr==0)? -1.f : 1.f);
								result.zr_factor = ((zr==0)? -1.f : 1.f);

								++numUpdates;
							}
						}}

					}}

				}}

			}}

			return numUpdates > 0;
		}

		TestResult		find_formula(			const Quat&		ORIGINAL,
												const Vec3&		YPR)
		{
			TestResult result;

			for(int32_t angleSwapID = -1; angleSwapID < 3; ++angleSwapID)
			{
				Vec3 angles = YPR;

				if(angleSwapID >= 0)
				{
					const uint32_t FIRST_ID		= angleSwapID;
					const uint32_t SECOND_ID	= (angleSwapID+1)%3;
					std::swap(angles[FIRST_ID], angles[SECOND_ID]);
				}

				const Quat TEST(angles);

				const float ERROR = find_similarity(ORIGINAL, TEST);
				if(ERROR < result.smallest_error)
				{
					result.quat				= TEST;
					result.smallest_error	= ERROR;
					result.angleSwap		= angleSwapID;
				}

				++result.numTests;

				if(find_term_factors(ORIGINAL, angles, result))
				{
					result.angleSwap = angleSwapID;
				}
			}

			/*
			Vec3		angles(0.f, 0.f, 0.f);
			for(int32_t dYaw = -1; dYaw < 4; ++dYaw)
			{
				angles[0] = YPR[0] + static_cast<float>(dYaw) * (glm::pi<float>() / 2.f);

				for(int32_t dPitch = -1; dPitch < 4; ++dPitch)
				{
					angles[1] = YPR[1] + static_cast<float>(dPitch) * (glm::pi<float>() / 2.f);

					for(int32_t dRoll = -1; dRoll < 4; ++dRoll)
					{
						angles[2] = YPR[2] + static_cast<float>(dRoll) * (glm::pi<float>() / 2.f);

						const Quat TEST(angles);

						bool bUpdateBestYPR = false;
						const float ERROR = find_similarity(ORIGINAL, TEST);
						if(ERROR < result.smallest_error)
						{
							result.quat				= TEST;
							result.smallest_error	= ERROR;
							bUpdateBestYPR = true;
						}

						++result.numTests;

						if(find_term_factors(ORIGINAL, angles, result))
						{
							bUpdateBestYPR = true;
						}

						if(bUpdateBestYPR)
						{
							result.best_dYaw	= dYaw;
							result.best_dPitch	= dPitch;
							result.best_dRoll	= dRoll;
						}
					}
				}
			}
			*/

			return result;
		}
	}

	namespace Quat_to_YPR
	{
		struct TestResult
		{
			Vec3		angles			= Vec3(0.f, 0.f, 0.f);
			float		smallest_error	= std::numeric_limits<float>::max();

			// Term Factors
			float		w_factor = 1.f;
			float		x_factor = 1.f;
			float		y_factor = 1.f;
			float		z_factor = 1.f;

			// Sqr Factors.
			float		w_Sqrfactor = 1.f;
			float		x_Sqrfactor = 1.f;
			float		y_Sqrfactor = 1.f;
			float		z_Sqrfactor = 1.f;

			// XYZ axis swap(0-no_swap, 1-XY, 2-XZ, 3-YZ).
			uint32_t	swapID			= 0;
			uint32_t	numTests		= 0;
		};

		float			calculate_yaw(		const Quat& ROT, 
											const Vec4& SQR_FACTORS)
		{
			const float TOP_VALUE		= 2.f * (ROT.y * ROT.z + ROT.w * ROT.x);
			const float BOTTOM_VALUE	= SQR_FACTORS.w * ROT.w * ROT.w - SQR_FACTORS.x * ROT.x * ROT.x - SQR_FACTORS.y * ROT.y * ROT.y + SQR_FACTORS.z * ROT.z * ROT.z;

			if((TOP_VALUE == 0.f) && (BOTTOM_VALUE == 0.f))
			{
				if((ROT.x == 0.f) && (ROT.y == 0.f))
					return 0.f;

				return 2.f * std::atan2(ROT.x, ROT.w);
			}

			return std::atan2(TOP_VALUE, BOTTOM_VALUE);
		}

		float			calculate_pitch(	const Quat& ROT)
		{
			return std::asin(glm::clamp(-2.f * (ROT.x * ROT.z - ROT.w * ROT.y), -1.f, 1.f));
		}

		float			calculate_roll(		const Quat& ROT, 
											const Vec4& SQR_FACTORS)
		{
			const float TOP_VALUE		= 2.f * (ROT.x * ROT.y + ROT.w * ROT.z);
			const float BOTTOM_VALUE	= SQR_FACTORS.w * ROT.w * ROT.w + SQR_FACTORS.x * ROT.x * ROT.x - SQR_FACTORS.y * ROT.y * ROT.y - SQR_FACTORS.z * ROT.z * ROT.z;

			if((TOP_VALUE == 0.f) && (BOTTOM_VALUE == 0.f))
				return 0.f;

			return std::atan2(TOP_VALUE, BOTTOM_VALUE);
		}

		Vec3			to_yaw_pitch_roll(	const Quat& ROTATION, 
											const Vec4& SQR_FACTORS)
		{
			Vec3 angles;

			const float sqw = SQR_FACTORS.w * ROTATION.w * ROTATION.w;
			const float sqx = SQR_FACTORS.x * ROTATION.x * ROTATION.x;
			const float sqy = SQR_FACTORS.y * ROTATION.y * ROTATION.y;
			const float sqz = SQR_FACTORS.z * ROTATION.z * ROTATION.z;

			const float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
			const float test = ROTATION.x * ROTATION.y + ROTATION.z * ROTATION.w;

			if (test > 0.499f * unit) { // singularity at north pole
				angles[0] = 2.f * atan2(ROTATION.x, ROTATION.w);
				angles[1] = glm::pi<float>() / 2.f;
				angles[2] = 0.f;
				return angles;
			}
			if (test < -0.499f * unit) { // singularity at south pole
				angles[0] = -2.f * atan2(ROTATION.x, ROTATION.w);
				angles[1] = -glm::pi<float>() / 2.f;
				angles[2] = 0.f;
				return angles;
			}

			const float YAW_TOP		= 2.f * (ROTATION.y * ROTATION.w - ROTATION.x * ROTATION.z);
			const float YAW_BOTTOM	= sqw + sqx - sqy - sqz;

			const float ROLL_TOP	= 2.f * (ROTATION.x * ROTATION.w - ROTATION.y * ROTATION.z);
			const float ROLL_BOTTOM = sqw - sqx + sqy - sqz;

			angles[0] = (YAW_TOP == 0.f && YAW_BOTTOM == 0.f) ? 0.f : atan2(YAW_TOP, YAW_BOTTOM);
			angles[1] = asin(2.f * test / unit);
			angles[2] = (ROLL_TOP == 0.f && ROLL_BOTTOM == 0.f) ? 0.f : atan2(ROLL_TOP, ROLL_BOTTOM);

			return angles;
		}

		float			find_similarity(	const Vec3& ORIGINAL, 
											const Vec3& IMITATION)
		{
			return glm::abs(ORIGINAL.x - IMITATION.x)
				 + glm::abs(ORIGINAL.y - IMITATION.y)
				 + glm::abs(ORIGINAL.z - IMITATION.z);
		}

		bool			find_swapped(		const Vec3& TARGET_YAW_PITCH_ROLL,
											const Quat& ROTATION,
											const Vec4& SQR_FACTORS,
											TestResult& result)
		{
			bool bSuccess = false;

			const Vec3 NO_SWAP = to_yaw_pitch_roll(ROTATION, SQR_FACTORS);
			const Vec3 XY_SWAP = to_yaw_pitch_roll(Quat(ROTATION.w, ROTATION.y, ROTATION.x, ROTATION.z), SQR_FACTORS);
			const Vec3 XZ_SWAP = to_yaw_pitch_roll(Quat(ROTATION.w, ROTATION.z, ROTATION.y, ROTATION.x), SQR_FACTORS);
			const Vec3 YZ_SWAP = to_yaw_pitch_roll(Quat(ROTATION.w, ROTATION.x, ROTATION.z, ROTATION.y), SQR_FACTORS);
			const Vec3 WX_SWAP = to_yaw_pitch_roll(Quat(ROTATION.x, ROTATION.w, ROTATION.y, ROTATION.z), SQR_FACTORS);
			const Vec3 WY_SWAP = to_yaw_pitch_roll(Quat(ROTATION.y, ROTATION.x, ROTATION.w, ROTATION.z), SQR_FACTORS);
			const Vec3 WZ_SWAP = to_yaw_pitch_roll(Quat(ROTATION.z, ROTATION.x, ROTATION.y, ROTATION.w), SQR_FACTORS);

			const float ERROR_0 = find_similarity(TARGET_YAW_PITCH_ROLL, NO_SWAP);
			const float ERROR_1 = find_similarity(TARGET_YAW_PITCH_ROLL, XY_SWAP);
			const float ERROR_2 = find_similarity(TARGET_YAW_PITCH_ROLL, XZ_SWAP);
			const float ERROR_3 = find_similarity(TARGET_YAW_PITCH_ROLL, YZ_SWAP);
			const float ERROR_4 = find_similarity(TARGET_YAW_PITCH_ROLL, WX_SWAP);
			const float ERROR_5 = find_similarity(TARGET_YAW_PITCH_ROLL, WY_SWAP);
			const float ERROR_6 = find_similarity(TARGET_YAW_PITCH_ROLL, WZ_SWAP);

			if(ERROR_0 < result.smallest_error)
			{
				result.swapID			= 0;
				result.smallest_error	= ERROR_0;
				result.angles			= NO_SWAP;
				bSuccess = true;
			}

			if(ERROR_1 < result.smallest_error)
			{
				result.swapID			= 1;
				result.smallest_error	= ERROR_1;
				result.angles			= XY_SWAP;
				bSuccess = true;
			}

			if(ERROR_2 < result.smallest_error)
			{
				result.swapID			= 2;
				result.smallest_error	= ERROR_2;
				result.angles			= XZ_SWAP;
				bSuccess = true;
			}

			if(ERROR_3 < result.smallest_error)
			{
				result.swapID			= 3;
				result.smallest_error	= ERROR_3;
				result.angles			= YZ_SWAP;
				bSuccess = true;
			}

			if(ERROR_4 < result.smallest_error)
			{
				result.swapID			= 4;
				result.smallest_error	= ERROR_4;
				result.angles			= WX_SWAP;
				bSuccess = true;
			}

			if(ERROR_5 < result.smallest_error)
			{
				result.swapID			= 5;
				result.smallest_error	= ERROR_5;
				result.angles			= WY_SWAP;
				bSuccess = true;
			}

			if(ERROR_6 < result.smallest_error)
			{
				result.swapID			= 6;
				result.smallest_error	= ERROR_6;
				result.angles			= WZ_SWAP;
				bSuccess = true;
			}

			result.numTests += 6;

			return bSuccess;
		}

		bool			find_sqr_factors(	const Vec3& TARGET_YAW_PITCH_ROLL,
											const Quat& ROTATION,
											TestResult& result)
		{
			bool bSuccess = false;

			for(uint32_t wi = 0; wi < 2; ++wi)
			{
				for(uint32_t xi = 0; xi < 2; ++xi)
				{
					for(uint32_t yi = 0; yi < 2; ++yi)
					{
						for(uint32_t zi = 0; zi < 2; ++zi)
						{
							const Vec4 SQR_FACTORS(((wi==0)? -1.f : 1.f),
													((xi==0)? -1.f : 1.f),
													((yi==0)? -1.f : 1.f),
													((zi==0)? -1.f : 1.f));

							if(find_swapped(TARGET_YAW_PITCH_ROLL, ROTATION, SQR_FACTORS, result))
							{
								result.w_Sqrfactor = SQR_FACTORS.w;
								result.x_Sqrfactor = SQR_FACTORS.x;
								result.y_Sqrfactor = SQR_FACTORS.y;
								result.z_Sqrfactor = SQR_FACTORS.z;
								bSuccess = true;
							}
						}
					}
				}
			}

			return bSuccess;
		}

		TestResult		find_formula(		const Vec3& TARGET_YAW_PITCH_ROLL,
											const Quat& ROTATION)
		{
			TestResult result;

			for(uint32_t wi = 0; wi < 2; ++wi)
			{
				for(uint32_t xi = 0; xi < 2; ++xi)
				{
					for(uint32_t yi = 0; yi < 2; ++yi)
					{
						for(uint32_t zi = 0; zi < 2; ++zi)
						{
							Quat	test;
									test.w = ROTATION.w * ((wi==0)? -1.f : 1.f);
									test.x = ROTATION.x * ((xi==0)? -1.f : 1.f);
									test.y = ROTATION.y * ((yi==0)? -1.f : 1.f);
									test.z = ROTATION.z * ((zi==0)? -1.f : 1.f);

							if(find_sqr_factors(TARGET_YAW_PITCH_ROLL, test, result))
							{
								result.w_factor = ((wi==0)? -1.f : 1.f);
								result.x_factor = ((xi==0)? -1.f : 1.f);
								result.y_factor = ((yi==0)? -1.f : 1.f);
								result.z_factor = ((zi==0)? -1.f : 1.f);
							}
						}
					}
				}
			}

			return result;
		}
	}


//=====> EulerAngles -> public constants
	const float EulerAngles::HALF_PI			= glm::pi<float>() / 2.f;
	const float EulerAngles::MAX_HALF_SIN_PITCH = 0.499f;

//=====> EulerAngles -> public functions
	void		EulerAngles::set_from_direction(	const Vec3&		NORMALIZED_LOCAL_FRONT)
	{
		roll = 0.f;

		if(NORMALIZED_LOCAL_FRONT.x == 0.f && NORMALIZED_LOCAL_FRONT.z == 0.f)
		{
			if(NORMALIZED_LOCAL_FRONT.y > 0.f)
			{
				yaw = 0.f; //glm::pi<float>();
				set_pitch_internal(NORMALIZED_LOCAL_FRONT);
			}
			else if(NORMALIZED_LOCAL_FRONT.y < 0.f)
			{
				yaw = 0.f; //-glm::pi<float>();
				set_pitch_internal(NORMALIZED_LOCAL_FRONT);
			}
			else // ingularity
			{
				yaw		= 0.f;
				pitch	= 0.f;
			}
		}
		else // straight up or down
		{
			set_yaw_internal(NORMALIZED_LOCAL_FRONT);
			set_pitch_internal(NORMALIZED_LOCAL_FRONT);
		}
	}

	void		EulerAngles::set_from_quat(			const Quat&		ROTATION)
	{
		const float SQ_W = ROTATION.w * ROTATION.w;
		const float SQ_X = ROTATION.x * ROTATION.x;
		const float SQ_Y = ROTATION.y * ROTATION.y;
		const float SQ_Z = ROTATION.z * ROTATION.z;

		const float CORRECTION_FACTOR	= SQ_X + SQ_Y + SQ_Z + SQ_W; // if normalised is one, otherwise is correction factor
		const float HALF_SIN_PITCH		= ROTATION.x * ROTATION.y + ROTATION.z * ROTATION.w;
		static const float MAX_HALF_SIN_PITCH = 0.499f;

		if (HALF_SIN_PITCH > MAX_HALF_SIN_PITCH * CORRECTION_FACTOR) 
		{ // singularity at north pole
			yaw		= 2.f * atan2(ROTATION.x, ROTATION.w);
			pitch	= glm::pi<float>() / 2.f;
			roll	= 0.f;
			return;
		}
		if (HALF_SIN_PITCH < -MAX_HALF_SIN_PITCH * CORRECTION_FACTOR) 
		{ // singularity at south pole
			yaw		= -2.f * atan2(ROTATION.x, ROTATION.w);
			pitch	= -glm::pi<float>() / 2.f;
			roll	= 0.f;
		}
		else
		{
			const float YAW_TOP		= 2.f * (ROTATION.y * ROTATION.w - ROTATION.x * ROTATION.z);
			const float YAW_BOTTOM	= SQ_W + SQ_X - SQ_Y - SQ_Z;

			const float ROLL_TOP	= 2.f * (ROTATION.x * ROTATION.w - ROTATION.y * ROTATION.z);
			const float ROLL_BOTTOM = SQ_W - SQ_X + SQ_Y - SQ_Z;

			yaw		= (YAW_TOP == 0.f && YAW_BOTTOM == 0.f) ? 0.f : atan2(YAW_TOP, YAW_BOTTOM);
			pitch	= asin(2.f * HALF_SIN_PITCH / CORRECTION_FACTOR);
			roll	= (ROLL_TOP == 0.f && ROLL_BOTTOM == 0.f) ? 0.f : atan2(ROLL_TOP, ROLL_BOTTOM);
		}
	}

	void		EulerAngles::set_only_yaw(			const Quat&		ROTATION)
	{
		const float SQ_W = ROTATION.w * ROTATION.w;
		const float SQ_X = ROTATION.x * ROTATION.x;
		const float SQ_Y = ROTATION.y * ROTATION.y;
		const float SQ_Z = ROTATION.z * ROTATION.z;

		const float CORRECTION_FACTOR	= SQ_X + SQ_Y + SQ_Z + SQ_W; // if normalised is one, otherwise is correction factor
		const float HALF_SIN_PITCH		= ROTATION.x * ROTATION.y + ROTATION.z * ROTATION.w;
		
		if (HALF_SIN_PITCH > MAX_HALF_SIN_PITCH * CORRECTION_FACTOR) 
		{ 
			// singularity at north pole
			yaw = 2.f * atan2(ROTATION.x, ROTATION.w);
		}
		if (HALF_SIN_PITCH < -MAX_HALF_SIN_PITCH * CORRECTION_FACTOR) 
		{ 
			// singularity at south pole
			yaw = -2.f * atan2(ROTATION.x, ROTATION.w);
		}
		else // general case
		{
			yaw = atan2(2.f * (ROTATION.y * ROTATION.w - ROTATION.x * ROTATION.z), 
						SQ_W + SQ_X - SQ_Y - SQ_Z);
		}
	}

	void		EulerAngles::set_only_pitch(		const Quat&		ROTATION)
	{
		const float SIN_PITCH = 2.f * (ROTATION.x * ROTATION.y + ROTATION.z * ROTATION.w);
		pitch = std::asin(glm::clamp(SIN_PITCH, -1.f, 1.f));
	}

	void		EulerAngles::set_only_roll(			const Quat&		ROTATION)
	{
		const float SQ_W = ROTATION.w * ROTATION.w;
		const float SQ_X = ROTATION.x * ROTATION.x;
		const float SQ_Y = ROTATION.y * ROTATION.y;
		const float SQ_Z = ROTATION.z * ROTATION.z;

		const float CORRECTION_FACTOR	= SQ_X + SQ_Y + SQ_Z + SQ_W; // if normalised is one, otherwise is correction factor
		const float HALF_SIN_PITCH		= ROTATION.x * ROTATION.y + ROTATION.z * ROTATION.w;

		if (HALF_SIN_PITCH > MAX_HALF_SIN_PITCH * CORRECTION_FACTOR) 
		{ 
			// singularity at north pole
			roll = 0.f;
		}
		if (HALF_SIN_PITCH < -MAX_HALF_SIN_PITCH * CORRECTION_FACTOR) 
		{ 
			// singularity at south pole
			roll = 0.f;
		}
		else // general case
		{
			roll = atan2(	2.f * (ROTATION.x * ROTATION.w - ROTATION.y * ROTATION.z), 
							SQ_W - SQ_X + SQ_Y - SQ_Z);
		}
	}

	Quat		EulerAngles::to_quat() const
	{
		// This formula was found with YPR_to_Quat process.
		const float cy = glm::cos(yaw() * 0.5f);
		const float sy = glm::sin(yaw() * 0.5f);
		const float cp = glm::cos(pitch() * 0.5f);
		const float sp = glm::sin(pitch() * 0.5f);
		const float cr = glm::cos(roll() * 0.5f);
		const float sr = glm::sin(roll() * 0.5f);

		Quat	quat;
				quat.w = cy * cr * cp - sy * sr * sp;
				quat.x = cy * sr * cp + sy * cr * sp;
				quat.y = sy * cr * cp + cy * sr * sp;
				quat.z = cy * cr * sp - sy * sr * cp;

		return quat;
	}

//=====> EulerAngles -> private functions
	void		EulerAngles::test_YPR_quat_conversions()
	{
		const EulerAngles ANGLES(	glm::radians(123.f),
									glm::radians(-73.f),
									glm::radians(17.f));

		const Quat FIRST_ROT	= glm::angleAxis(ANGLES.yaw(), Vec3(0.f, 1.f, 0.f));
		const Vec3 LOCAL_RIGHT	= FIRST_ROT * Vec3(0.f, 0.f, 1.f);
		const Quat SECOND_ROT	= glm::angleAxis(ANGLES.pitch(), LOCAL_RIGHT) * FIRST_ROT;
		const Vec3 LOCAL_FRONT  = SECOND_ROT * Vec3(1.f, 0.f, 0.f);
		const Quat FINAL_ROT	= glm::angleAxis(ANGLES.roll(), LOCAL_FRONT) * SECOND_ROT;

		//const auto RESULT0 = YPR_to_Quat::find_formula(FINAL_ROT, ANGLES_RAD);
		//const auto RESULT1 = Quat_to_YPR::find_formula(ANGLES_RAD, FINAL_ROT);

		//const float calculated_YAW		= glm::degrees(calculate_yaw(RESULT0.quat));
		//const float calculated_PITCH	= glm::degrees(calculate_pitch(RESULT0.quat));
		//const float calculated_ROLL		= glm::degrees(calculate_roll(RESULT0.quat));

		//const Vec3 TEST_ANGLES = glm::degrees(to_yaw_pitch_roll(RESULT0.quat));

		int breakpoint = 0;
	}
}