#include "Core.h"

int main(int argc, char** argv)
{
	std::unique_ptr<Core> core(std::make_unique<Core>());
	if (core->Init())
		core->Run();
	core->Destroy();

	return 0;
}