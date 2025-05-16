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
static unsigned int NumSets = 4;
#define NO_OF_Iterations 1000000 // CHange to 1,000,000

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
struct GenFunction { unsigned int (*fn)(); const char* name; };
GenFunction Funct[] = {
	{ memGen1, "memGen1 (sequential full DRAM)"},
	{ memGen2, "memGen2 (24 KiB random)" },
	{ memGen3, "memGen3 (global random)" },
	{ memGen4, "memGen4 (4 KiB sequential)" },
	{ memGen5, "memGen5 (64 KiB sequential)"},
	{ memGen6, "memGen6 (stride=32 over 64 KiB)"},
};
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
// Set associative simulator
cacheResType cacheSimSA(unsigned int addr)
{
	unsigned int block = addr / FaLineSize;
	unsigned int setindex = block % NumSets;
	unsigned int tag = block / NumSets;
	unsigned int ways = FaCacheLines / NumSets;
	unsigned int base = setindex * ways;

	// Check for hit
	for (unsigned i = 0; i < ways; ++i) {
		CacheLineFA& ln = FaCache[base + i];
		if (ln.valid && ln.tag == tag) {
			ln.leastrecentuseCounter = ++FaAccessCount;
			return HIT;
		}
	}
	//choose invalid slot or LRU
	unsigned int candidate = base;
	unsigned int leastUsed = UINT_MAX;
	for (unsigned i = 0; i < ways; ++i) {
		CacheLineFA& ln = FaCache[base + i];
		if (!ln.valid) {
			candidate = base + i;
			break;
		}
		if (ln.leastrecentuseCounter < leastUsed) {
			leastUsed = ln.leastrecentuseCounter;
			candidate = base + i;
		}
	}

	//add new line
	CacheLineFA& line = FaCache[candidate];
	line.valid = true;
	line.tag = tag;
	line.leastrecentuseCounter = ++FaAccessCount;
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
void resetSACache(unsigned int newLineSize, unsigned int newNumSets) {
	FaLineSize = newLineSize;
	FaCacheLines = CACHE_SIZE / FaLineSize;
	FaAccessCount = 0;
	NumSets = newNumSets;

	delete[] FaCache;
	FaCache = new CacheLineFA[FaCacheLines]();
}


const char* msg[2] = { "Miss","Hit" };
void FASATestCases() {
	cout << "Test 1: Fully Associative Cache with 3 lines. 4 Addresses A, B, C, and D"<<endl;
	resetSACache(64, 1);
	FaCacheLines = 3;
	delete[] FaCache;
	FaCache = new CacheLineFA[3]();

	unsigned int a = 0x0000;
	unsigned int b = 0x0040; 
	unsigned int c = 0x0080;
	unsigned int d = 0x00C0; 

	cout << "Expected Values for Test1: MISS, MISS, MISS, HIT, HIT, MISS (evict C as it is LRU), MISS (c was evicted) " << endl;

	cout << "Access A (0x0000): " << msg[cacheSimSA(a)] << endl;
	cout << "Access B (0x0040): " << msg[cacheSimSA(b)] << endl;
	cout << "Access C (0x0080): " << msg[cacheSimSA(c)] << endl;
	cout << "Access A again:     " << msg[cacheSimSA(a)] << endl;
	cout << "Access B again:     " << msg[cacheSimSA(b)] << endl;
	cout << "Access D (0x00C0): " << msg[cacheSimSA(d)] << endl;
	cout << "Access C again:     " << msg[cacheSimSA(c)] << endl;

	cout << "Test 2: 2 sets with 2 lines per set and 3 addresses: addr0,addr2,addr4" << endl;
	resetSACache(64, 2);
	FaCacheLines = 4;
	delete[] FaCache;
	FaCache = new CacheLineFA[4]();

	unsigned int addr0 = 0x0000;
	unsigned int addr2 = 0x0080;
	unsigned int addr4 = 0x0100;

	cout << "Expected Value for Test 2: MISS,MISS,HIT,HIT,MISS,MISS" << endl;

	cout << "Access addr0 (0x0000): " << msg[cacheSimSA(addr0)] << endl;
	cout << "Access addr2 (0x0080): " << msg[cacheSimSA(addr2)] << endl;
	cout << "Access addr0 again:    " << msg[cacheSimSA(addr0)] << endl;
	cout << "Access addr2 again:    " << msg[cacheSimSA(addr2)] << endl; 
	cout << "Access addr4 (0x0100): " << msg[cacheSimSA(addr4)] << endl;  
	cout << "Access addr0 again:    " << msg[cacheSimSA(addr0)] << endl; 

	cout << "Test 3: Repeated Access to Same Address"<<endl;
	cout << "Expected: MISS, HIT, HIT, HIT"<<endl;

	resetSACache(64, 1);
	FaCacheLines = 1;
	delete[] FaCache;
	FaCache = new CacheLineFA[1]();

	unsigned int addr = 0x0000;

	cout << "1) Access addr: " << msg[cacheSimSA(addr)] << endl;
	cout << "2) Access addr: " << msg[cacheSimSA(addr)] << endl;
	cout << "3) Access addr: " << msg[cacheSimSA(addr)] << endl;
	cout << "4) Access addr: " << msg[cacheSimSA(addr)] << endl;

	cout << "Test 4: Independent Sets Dont collide"<<endl;
	cout << "Expected: MISS, MISS, HIT (set 0), HIT (set 1)\n";

	resetSACache(64, 2);
	FaCacheLines = 4;
	delete[] FaCache;
	FaCache = new CacheLineFA[4]();

	unsigned int set0 = 0x0000; // set 0
	unsigned int set1 = 0x0040; // set 1

	cout << "1) Access set0: " << msg[cacheSimSA(set0)] << endl;
	cout << "2) Access set1: " << msg[cacheSimSA(set1)] << endl;
	cout << "3) Access set0: " << msg[cacheSimSA(set0)] << endl;
	cout << "4) Access set1: " << msg[cacheSimSA(set1)] << endl;

	cout << "Test 5: LRU Inside a Set" << endl;
	cout << "Expected: MISS, MISS, HIT, MISS (evict oldest), MISS (evicted)\n";

	resetSACache(64, 2);
	FaCacheLines = 4;
	delete[] FaCache;
	FaCache = new CacheLineFA[4]();

	unsigned int x = 0x0000;
	unsigned int y = 0x0080;
	unsigned int z = 0x0100;

	cout << "1) Access x: " << msg[cacheSimSA(x)] << endl;
	cout << "2) Access y: " << msg[cacheSimSA(y)] << endl;
	cout << "3) Access x: " << msg[cacheSimSA(x)] << endl;
	cout << "4) Access z: " << msg[cacheSimSA(z)] << endl;
	cout << "5) Access y: " << msg[cacheSimSA(y)] << endl;

}
void datacollectionSA() {
	cout << " Data Collection for SACacheSim: " << "\n\n\n";

	const unsigned int sizes[] = { 16, 32, 64, 128 };
	const unsigned int SetNum = 4;
	const unsigned int WaysSet[] = { 1, 2, 4, 8, 16, 32, 64 };
	for (int gen = 1; gen <= 6; ++gen) {

		cout << " Generator memGen" << gen << endl;

		// Experiment 1
		cout << "FACacheSim Experiment 1: 4 sets, varying line size"<<endl;
		for (unsigned int L : sizes) {
			resetSACache(L, SetNum);

			unsigned int totalLines = CACHE_SIZE / L;
			unsigned int ways = totalLines / SetNum;

			cout << "Line size = " << L << " bytes, Ways = " << ways << endl;

			unsigned long long hits = 0;
			for (unsigned long long i = 0; i < NO_OF_Iterations; ++i) {
				unsigned int addr;
				switch (gen) {
				case 1: addr = memGen1(); break;
				case 2: addr = memGen2(); break;
				case 3: addr = memGen3(); break;
				case 4: addr = memGen4(); break;
				case 5: addr = memGen5(); break;
				case 6: addr = memGen6(); break;
				}
				if (cacheSimSA(addr) == HIT) {
					hits++;
				}
			}
			double ratio = 100.0 * hits / NO_OF_Iterations;
			cout << dec
				<< "Hit ratio = "
				<< fixed << setprecision(3)
				<< ratio << "%" << endl;
		}

		cout << endl;

		//Experiment 2
		FaLineSize = 64;
		cout << "FACache Experiment 2: Fixed Line Size (64) and varying ways:"<<endl;
		for (unsigned int wayCount : WaysSet) {
			resetSACache(FaLineSize, wayCount);

			unsigned int totalLines = CACHE_SIZE / FaLineSize;
			unsigned int ways = totalLines / wayCount;

			cout << "Number of Lines: " << FaLineSize
				<< " Bytes, Number of Sets = " << wayCount
				<< ", Ways = " << ways << endl;

			unsigned long long hits1 = 0;
			for (unsigned long long i = 0; i < NO_OF_Iterations; ++i) {
				unsigned int addr1;
				switch (gen) {
				case 1: addr1 = memGen1(); break;
				case 2: addr1 = memGen2(); break;
				case 3: addr1 = memGen3(); break;
				case 4: addr1 = memGen4(); break;
				case 5: addr1 = memGen5(); break;
				case 6: addr1 = memGen6(); break;
				}
				if (cacheSimSA(addr1) == HIT) {
					hits1++;
				}
			}
			double ratio1 = 100.0 * hits1 / NO_OF_Iterations;
			cout << "Hit ratio = "
				<< fixed << setprecision(3)
				<< ratio1 << "%" << endl;
		}
	}

	delete[] FaCache;
}

int main()
{
	FASATestCases();
	datacollectionSA();
	return 0;
}
