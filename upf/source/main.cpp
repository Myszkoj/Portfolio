
#define TEST_UPF

#ifdef TEST_UPF

#include "..//include/upf_Tests.h"

int main()
{
	//upf::test_random_objects(100, 100000, upf::HVSize(3.f, 2.f), std::uniform_real_distribution<float>(0.f, 100.f), std::uniform_real_distribution<float>(0.f, 100.f));
	upf::test_moving_objects(100, 100000, upf::HVSize(3.f, 2.f), std::uniform_real_distribution<float>(0.f, 100.f), std::uniform_real_distribution<float>(0.f, 100.f), 0.2f);
	//upf::test_uniform_objects(100);

	return 0;
}
#endif // TEST_UPF