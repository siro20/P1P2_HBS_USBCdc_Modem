#pragma once
#include <inttypes.h>

class ChannelComp
{
	public:
		ChannelComp(void);

		bool Update(int16_t in, int16_t *out);

	private:
		int16_t charge;
		int16_t resistance;
};