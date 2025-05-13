#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;
#define DBG 1
#define DRAM_SIZE (64*1024*1024)
#define CACHE_SIZE (64*1024)
struct CacheLineFA {
	unsigned int tag = 0;
	bool valid = false;
	unsigned int leastrecentuseCounter = 0;
};
enum cacheResType { MISS = 0, HIT = 1 };
static unsigned int   FaLineSize = 64;
static unsigned int FaCacheLines = CACHE_SIZE / FaLineSize;
static CacheLineFA* FaCache = new CacheLineFA[FaCacheLines]{};
static unsigned int   FaAccessCount = 0;

/* The following implements a random number generator */
unsigned int m_w = 0xABABAB55; /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05080902; /* must not be zero, nor 0x9068ffff */
unsigned int rand_()
{
	m_z = 36969 * (m_z & 65535) + (m_z >> 16);
	m_w = 18000 * (m_w & 65535) + (m_w >> 16);
	return (m_z << 16) + m_w; /* 32-bit result */
}
unsigned int memGen1()
{
	static unsigned int addr = 0;
	return (addr++) % (DRAM_SIZE);
}
unsigned int memGen2()
{
	static unsigned int addr = 0;
	return rand_() % (24 * 1024);
}
unsigned int memGen3()
{
	return rand_() % (DRAM_SIZE);
}
unsigned int memGen4()
{
	static unsigned int addr = 0;
	return (addr++) % (4 * 1024);
}
unsigned int memGen5()
{
	static unsigned int addr = 0;
	return (addr++) % (1024 * 64);
}
unsigned int memGen6()
{
	static unsigned int addr = 0;
	return (addr += 32) % (64 * 4 * 1024);
}
// Direct Mapped Cache Simulator
cacheResType cacheSimDM(unsigned int addr)
{
	// This function accepts the memory address for the memory transaction and
	// returns whether it caused a cache miss or a cache hit
	// The current implementation assumes there is no cache; so, every
	//transaction is a miss
		return MISS;
}

// Fully Associative Cache Simulator
cacheResType cacheSimFA(unsigned int addr)
{
	unsigned int lineSize = FaLineSize;
	unsigned int cacheLines = FaCacheLines;
	CacheLineFA* cache = FaCache;
	unsigned int& accessCount = FaAccessCount;

	unsigned int tag = addr / lineSize;
	// Check for hit
	bool hit = false;
	int hitIndex = -1;

	for (int i = 0; i < cacheLines; ++i) {
		if (cache[i].valid && cache[i].tag == tag) {
			hit = true;
			hitIndex = i;
			break;
		}
	}
	if (hit) {
		cache[hitIndex].leastrecentuseCounter = ++accessCount;
		return HIT;
	}
	// Miss
	unsigned int lruValue = UINT_MAX;
	int lruIndex = 0;

	for (int i = 0; i < cacheLines; ++i) {
		if (!cache[i].valid) { // Prioritize invalid entries
			lruIndex = i;
			break;
		}
		if (cache[i].leastrecentuseCounter < lruValue) {
			lruValue = cache[i].leastrecentuseCounter;
			lruIndex = i;
		}
	}
	// Replace entry
	cache[lruIndex].tag = tag;
	cache[lruIndex].valid = true;
	cache[lruIndex].leastrecentuseCounter = ++accessCount;
		return MISS;
}
// Re‑initialize the FA cache for new line size
void resetFACache(unsigned int newLineSize) {
	FaLineSize = newLineSize;
	FaCacheLines = CACHE_SIZE / FaLineSize;
	FaAccessCount = 0;

	delete[] FaCache;
	FaCache = new CacheLineFA[FaCacheLines]();
}


const char* msg[2] = { "Miss","Hit" };
#define NO_OF_Iterations 100 // CHange to 1,000,000
int main()
{
	const unsigned int sizes[] = { 16, 32, 64, 128 };
	const unsigned int SetNum = 4;

	cout << "Experiment 1 for FACacheSim: 4 sets, varying line size"<<endl;

	for (unsigned int L : sizes) {
		resetFACache(L);

	unsigned int totalLines = CACHE_SIZE / L;
	unsigned int ways = totalLines / SetNum;

		cout << "Line size = " << L << " bytes, Ways = " << ways << "\n";
		//cout << "Address (Miss/Hit)\n";

		unsigned long long hits = 0;
		for (unsigned long long i = 0; i < NO_OF_Iterations; ++i) {
			unsigned int addr = memGen2();
			cacheResType r = cacheSimFA(addr);
			if (r == HIT) {
				hits++;
			}

			//cout << "0x"
				//<< setfill('0') << setw(8) << hex << addr
				//<< " (" << msg[r] << ")\n";
		}
		double ratio = 100.0 * hits / NO_OF_Iterations;
		cout << dec << "Hit ratio = " << fixed << setprecision(3) << ratio << "%" << endl;
	}
	cout << endl;

	// cleanup
	delete[] FaCache;
	return 0;
}