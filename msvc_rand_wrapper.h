#pragma once
class MSVCRandWrapper {
  public:
	MSVCRandWrapper() = default;

	void srand(unsigned int seed);

	int rand();

  private:
	unsigned int seed_;
};

