#include <gtest/gtest.h>

#include "uart.hpp"

#define OVERSAMPLING 16

// Captured waveform from hardware
static int16_t testdata[] = {
3000,
3000,
3000,
3000,
3000,
3000,
1500,
0,
0,
0,
-26,
-30,
-44,
-51,
11,
-149,
-1860,
-3046,
-3029,
-3003,
-2995,
-2997,
-3029,
-2954,
-1440,
-46,
20,
-21,
-5,
-5,
-65,
296,
2112,
3160,
3126,
3102,
3073,
3074,
3130,
2845,
1022,
-97,
-56,
-37,
-46,
-60,
20,
-484,
-2254,
-3063,
-3028,
-3037,
-3023,
-3001,
-3033,
-2915,
-1267,
14,
15,
15,
14,
14,
-70,
563,
2455,
3168,
3123,
3102,
3089,
3090,
3161,
2469,
593,
-80,
-52,
-44,
-36,
-60,
29,
-706,
-2512,
-3053,
-3005,
-3018,
-3003,
-2995,
-3052,
-2755,
-988,
51,
8,
-1,
15,
14,
-85,
756,
2646,
3160,
3119,
3116,
3102,
3073,
3176,
2322,
402,
-97,
-49,
-30,
-39,
-40,
61,
-821,
-2664,
-3053,
-2998,
-2997,
-2997,
-2997,
-3081,
-2392,
-565,
74,
19,
-1,
15,
16,
-101,
840,
2790,
3167,
3118,
3109,
3088,
3092,
3185,
2163,
273,
-80,
-41,
-32,
-39,
-26,
2,
-1186,
-2844,
-3046,
-2995,
-2997,
-2997,
-2997,
-3093,
-2251,
-416,
47,
17,
20,
19,
20,
27,
22,
22,
0,
13,
20,
20,
27,
21,
13,
-1,
13,
21,
13,
-15,
54,
1559,
3037,
3138,
3109,
3094,
3094,
3126,
3057,
1502,
46,
-45,
-26,
-40,
-40,
-39,
-32,
-25,
-18,
-26,
-41,
-26,
-19,
-19,
-26,
-39,
-32,
-39,
-26,
-19,
-19,
11,
-106,
-1732,
-3030,
-3018,
-2995,
-2997,
-2982,
-3005,
-2993,
-1621,
-111,
47,
20,
19,
19,
-28,
179,
1937,
3154,
3126,
3115,
3108,
3094,
3144,
2921,
1142,
-53,
-32,
-19,
-26,
-39,
26,
-308,
-2041,
-3042,
-3005,
-2997,
-2997,
-2997,
-3031,
-2942,
-1339,
-13,
32,
20,
19,
20,
-58,
420,
2273,
3178,
3136,
3114,
3108,
3094,
3172,
2618,
729,
-99,
-52,
-39,
-40,
-40,
42,
-572,
-2377,
-3044,
-3001,
-2997,
-2983,
-2976,
-3029,
-2762,
-1060,
25,
6,
1,
1,
8,
-89,
656,
2546,
3173,
3140,
3127,
3113,
3115,
3203,
2412,
501,
-74,
-42,
-32,
-40,
-12,
98,
-702,
-2543,
-3038,
-3000,
-2997,
-2983,
-2976,
-3050,
-2530,
-739,
71,
21,
13,
20,
19,
20,
27,
21,
28,
14,
13,
20,
18,
34,
35,
13,
13,
20,
19,
20,
27,
22,
22,
0,
13,
21,
13,
14,
30,
14,
-1,
13,
20,
20,
28,
2,
-52,
1134,
2893,
3160,
3109,
3094,
3094,
3127,
3136,
1832,
147,
-78,
-40,
-32,
-39,
-6,
-10,
-1325,
-2873,
-3033,
-2997,
-2983,
-2976,
-2978,
-3079,
-2196,
-331,
80,
24,
13,
-1,
13,
20,
18,
34,
35,
13,
-1,
13,
20,
18,
35,
28,
-1,
13,
20,
20,
-8,
72,
1666,
3071,
3134,
3109,
3094,
3094,
3144,
3061,
1407,
12,
-31,
-40,
-32,
-39,
11,
-86,
-1707,
-3004,
-3014,
-2997,
-2983,
-2976,
-3000,
-2980,
-1627,
-109,
41,
35,
41,
33,
-25,
123,
1867,
3133,
3130,
3117,
3122,
3114,
3168,
2892,
1114,
-56,
-55,
-40,
-26,
-19,
34,
-262,
-2007,
-3047,
-3006,
-2983,
-2976,
-2978,
-3011,
-2930,
-1369,
-13,
43,
24,
22,
0,
-55,
415,
2246,
3158,
3136,
3135,
3127,
3114,
3184,
2638,
793,
-65,
-49,
-26,
-19,
-21,
75,
-537,
-2350,
-3044,
-3002,
-2983,
-2976,
-2978,
-3020,
-2861,
-1152,
46,
40,
35,
18,
19,
20,
27,
20,
20,
27,
20,
19,
20,
27,
20,
20,
27,
20,
19,
20,
41,
41,
39,
33,
18,
20,
27,
20,
20,
27,
20,
18,
34,
34,
20,
27,
-83,
803,
2732,
3184,
3137,
3134,
3127,
3114,
3213,
2297,
371,
-66,
-43,
-39,
-32,
-40,
92,
-791,
-2671,
-3060,
-3019,
-3003,
-2982,
-2976,
-3067,
-2331,
-504,
68,
25,
27,
20,
9,
-44,
1105,
2886,
3175,
3130,
3113,
3129,
3152,
3113,
1751,
164,
-44,
-42,
-39,
-32,
-24,
-25,
-40,
-40,
-39,
-32,
-25,
-18,
-26,
-40,
-40,
-26,
-19,
-21,
-19,
-26,
-14,
-56,
-1482,
-2911,
-3009,
-2997,
-2983,
-2976,
-2948,
-3221,
-2765,
-436,
114,
37,
18,
20,
-6,
64,
1666,
3093,
3154,
3123,
3094,
3094,
3129,
3044,
1429,
18,
-49,
-25,
-24,
-32,
-3,
-86,
-1690,
-2994,
-3019,
-3018,
-3003,
-2982,
-2998,
-2980,
-1627,
-109,
43,
30,
20,
18,
-8,
116,
1851,
3167,
3151,
3122,
3093,
3109,
3164,
2842,
1085,
-38,
-40,
-25,
-9,
-25,
11,
-255,
-1972,
-3040,
-2998,
-2996,
-2997,
-2983,
-3009,
-2930,
-1370,
2,
50,
21,
13,
14,
28,
20,
18,
34,
34,
18,
33,
41,
34,
13,
13,
21,
13,
-1,
27,
41,
39,
33,
20,
13,
13,
20,
20,
27,
20,
19,
18,
34

};

static void genTestData(uint8_t b, enum UART::UART_PARITY p, int16_t data[OVERSAMPLING * 13]) {
	size_t off, toggle, parity;
	for (size_t i = 0; i < OVERSAMPLING * 13; i++) {
		data[i] = 0;
	}

	off = OVERSAMPLING;
	toggle = 1;

	/* Start */
	for (size_t i = 0; i < OVERSAMPLING / 2; i++) {
		data[i + off] = 3300 * toggle;
	}
	toggle *= -1;

	off += OVERSAMPLING;
	parity = 0;

	/* Data LSB first */
	for (ssize_t bit = 0; bit < 8; bit++, off+=OVERSAMPLING) {
		if (!(b & (1 << bit))) {
			for (size_t i = 0; i < OVERSAMPLING / 2; i++) {
				data[i + off] = 3300 * toggle;
			}
			toggle *= -1;
		} else {
			parity++;
		}
	}
	/* Parity */
	if ((p == UART::PARITY_EVEN) && !(parity & 1)) {
		// Parity is even, send 0
		for (size_t i = 0; i < OVERSAMPLING / 2; i++) {
			data[i + off] = 3300 * toggle;
		}
		toggle *= -1;
	}
	if ((p == UART::PARITY_ODD) && ((parity & 1) > 0)) {
		// Parity is odd, send 0
		for (size_t i = 0; i < OVERSAMPLING / 2; i++) {
			data[i + off] = 3300 * toggle;
		}
		toggle *= -1;
	}
	//for (size_t i = 0; i < OVERSAMPLING * 13; i++) {
	//	std::cout << data[i] << " ";
	//}
	//std::cout << std::endl;
}

TEST(UART, TestDecodeParityNone)
{
	UART u(3300/2, UART::PARITY_NONE);
	uint8_t out;
	bool err;
	bool done;
	int16_t p[OVERSAMPLING * 13];
	int uart_active;

	for (uint16_t testbyte = 0; testbyte <= 0xff; testbyte++) {
		genTestData(testbyte, UART::PARITY_NONE, p);

		uart_active = 0;
		for (size_t i = 0; i < OVERSAMPLING * 13; i++) {	
			out = 0;
			err = false;
			done = u.Update(p[i], &out, &err);
			if (done)
				break;
			if (u.Receiving())
				uart_active++;
		}

		EXPECT_GT(uart_active, 9 * OVERSAMPLING);
		EXPECT_EQ(done, true);
		EXPECT_EQ(err, false);
		EXPECT_EQ(testbyte, out);
	}
}

TEST(UART, TestFramingError)
{
	UART u(3300/2, UART::PARITY_NONE);
	uint8_t out;
	bool err;
	bool done;
	int16_t p[OVERSAMPLING * 13];

	for (uint16_t testbyte = 0; testbyte <= 0xff; testbyte++) {
		genTestData(testbyte, UART::PARITY_NONE, p);

		// Break STOP bit
		for (size_t i = (OVERSAMPLING * 10); i < (OVERSAMPLING * 10 + OVERSAMPLING/2); i++)
			p[i] = 3300;

		for (size_t i = 0; i < OVERSAMPLING * 13; i++) {	
			out = 0;
			err = false;
			done = u.Update(p[i], &out, &err);
			if (done)
				break;
		}

		EXPECT_EQ(done, true);
		EXPECT_EQ(err, true);
	}
}

TEST(UART, TestPolarityError)
{
	UART u(3300/2, UART::PARITY_EVEN);
	uint8_t out;
	bool err;
	bool done;
	int16_t p[OVERSAMPLING * 13];

	for (uint16_t testbyte = 0; testbyte <= 0xff; testbyte++) {
		genTestData(testbyte, UART::PARITY_EVEN, p);

		// Invert START bit polarity
		for (size_t i = (OVERSAMPLING * 1); i < (OVERSAMPLING * 1 + OVERSAMPLING/2); i++)
			p[i] = -p[i];

		for (size_t i = 0; i < OVERSAMPLING * 13; i++) {	
			out = 0;
			err = false;
			done = u.Update(p[i], &out, &err);
			if (done)
				break;
		}

		EXPECT_EQ(done, true);
		EXPECT_EQ(err, true);
	}
}

TEST(UART, TestDecodeParityEven)
{
	UART u(3300/2, UART::PARITY_EVEN);
	uint8_t out;
	bool err;
	bool done;
	int16_t p[OVERSAMPLING * 13];
	int uart_active;

	// Test all bytes with correct parity
	for (uint16_t testbyte = 0; testbyte <= 0xff; testbyte++) {
		genTestData(testbyte, UART::PARITY_EVEN, p);

		uart_active = 0;
		for (size_t i = 0; i < OVERSAMPLING * 13; i++) {	
			out = 0;
			err = false;
			done = u.Update(p[i], &out, &err);
			if (done)
				break;
			if (u.Receiving())
				uart_active++;
		}

		EXPECT_GT(uart_active, 10 * OVERSAMPLING);
		EXPECT_EQ(done, true);
		EXPECT_EQ(err, false);
		EXPECT_EQ(testbyte, out);
	}
	// Test all bytes with incorrect parity
	for (uint16_t testbyte = 0; testbyte <= 0xff; testbyte++) {
		genTestData(testbyte, UART::PARITY_ODD, p);

		for (size_t i = 0; i < OVERSAMPLING * 13; i++) {	
			out = 0;
			err = false;
			done = u.Update(p[i], &out, &err);
			if (done)
				break;
		}

		EXPECT_EQ(done, true);
		EXPECT_EQ(err, true);
	}
}

TEST(UART, TestDecodeParityOdd)
{
	UART u(3300/2, UART::PARITY_ODD);
	uint8_t out;
	bool err;
	bool done;
	int16_t p[OVERSAMPLING * 13];
	int uart_active;

	// Test all bytes with correct parity
	for (uint16_t testbyte = 0; testbyte <= 0xff; testbyte++) {
		genTestData(testbyte, UART::PARITY_ODD, p);

		uart_active = 0;
		for (size_t i = 0; i < OVERSAMPLING * 13; i++) {
	
			out = 0;
			err = false;
			done = u.Update(p[i], &out, &err);
			if (done)
				break;
			if (u.Receiving())
				uart_active++;
		}

		EXPECT_GT(uart_active, 10 * OVERSAMPLING);
		EXPECT_EQ(done, true);
		EXPECT_EQ(err, false);
		EXPECT_EQ(testbyte, out);
	}
	// Test all bytes with incorrect parity
	for (uint16_t testbyte = 0; testbyte <= 0xff; testbyte++) {
		genTestData(testbyte, UART::PARITY_EVEN, p);

		for (size_t i = 0; i < OVERSAMPLING * 13; i++) {
	
			out = 0;
			err = false;
			done = u.Update(p[i], &out, &err);
			if (done)
				break;
		}

		EXPECT_EQ(done, true);
		EXPECT_EQ(err, true);
	}
}

TEST(UART, TestPulseNoise)
{
	UART u(3300/2, UART::PARITY_ODD);

	uint8_t out;
	bool err;
	bool done;
	int16_t p[OVERSAMPLING * 13];

	for (int16_t noise = 0; noise < OVERSAMPLING * 13; noise++) {
		genTestData(0xcc, UART::PARITY_ODD, p);

		// Inject spike
		p[noise] += 2500;
		p[noise+1] -= 2500;
		for (size_t i = 0; i < OVERSAMPLING * 13; i++) {
			out = 0;
			err = false;
			done = u.Update(p[i], &out, &err);
			if (done)
				break;
		}
		EXPECT_EQ(done, true);
		EXPECT_EQ(err, false);
		EXPECT_EQ(0xcc, out);
		// Print detailed report
		if (err || out != 0xcc) {
			for (ssize_t i = -32; i < 32; i++) {
				if (noise + i > 0 && noise + i < OVERSAMPLING * 13) {
					std::cerr << "[          ] p[" << noise + i << "] = " << p[noise + i] << std::endl;
				}
			}
			std::cerr << std::endl;
			p[noise] -= 2500;
			p[noise+1] += 2500;
			std::cerr << "[          ] was: " << std::endl;
			for (ssize_t i = -32; i < 32; i++) {
				if (noise + i > 0 && noise + i < OVERSAMPLING * 13)
					std::cerr << "[          ] p[" << noise + i << "] = " << p[noise + i] << std::endl;
			}
			std::cerr << "[          ] noise = " << noise << std::endl;
		}
	}
}

TEST(UART, TestCapturedTestData)
{
	UART u(1400, UART::PARITY_EVEN);

	uint8_t out;
	bool err;
	int count = 0;

	for (size_t i = 0; i < sizeof(testdata)/sizeof(testdata[0]); i++) {
		out = 0xff;
		err = false;
		if (u.Update(testdata[i], &out, &err)) {

			EXPECT_EQ(count, out);
			if (err)
				std::cerr << "[          ] i = " << i << std::endl;
			EXPECT_EQ(err, false);

			count++;

		}
	}

	EXPECT_EQ(count, 3);

}
