#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <random>

using namespace std;

// Cache Block structure (for each cache line)
struct CacheBlock {
    bool valid;
    unsigned int tag;
    int data;  // Simulated data (optional)
};

// Cache structure for Set-Associative, Fully Associative, or Direct-Mapped
struct Cache {
    int numSets;                // Number of sets (1 for fully associative)
    int associativity;          // 1 for direct-mapped, n for n-way set associative
    int blockSize;              // Cache block size (in bytes)
    vector<vector<CacheBlock>> sets;  // A 2D array of sets and their blocks

    Cache(int numSets, int associativity, int blockSize)
        : numSets(numSets), associativity(associativity), blockSize(blockSize) {
        sets.resize(numSets, vector<CacheBlock>(associativity));
    }
};

enum ReplacementPolicy { FIFO, LRU, RANDOM };

ReplacementPolicy chooseReplacementPolicy() {
    int choice;
    cout << "Choose cache replacement policy:\n";
    cout << "1. FIFO\n";
    cout << "2. LRU\n";
    cout << "3. RANDOM\n";
    cout << "Enter your choice (1-3): ";
    cin >> choice;
    return static_cast<ReplacementPolicy>(choice - 1);
}

int accessCache(Cache& cache, unsigned int address) {
    // Extract the index and tag from the address
    unsigned int blockOffset = log2(cache.blockSize);
    unsigned int index = (address >> blockOffset) % cache.numSets;
    unsigned int tag = address >> (blockOffset + static_cast<int>(log2(cache.numSets)));

    // Search for the tag in the selected set
    for (int i = 0; i < cache.associativity; ++i) {
        CacheBlock& block = cache.sets[index][i];
        if (block.valid && block.tag == tag) {
            // Cache hit
            cout << "Cache HIT at address: " << address << endl;
            return 1;
        }
    }
    // Cache miss
    cout << "Cache MISS at address: " << address << endl;
    return 0;
}

void insertCacheBlock(Cache& cache, unsigned int address, ReplacementPolicy policy, vector<int>& lruTracker) {
    unsigned int blockOffset = log2(cache.blockSize);
    unsigned int index = (address >> blockOffset) % cache.numSets;
    unsigned int tag = address >> static_cast<int>(blockOffset + log2(cache.numSets));

    // Insert based on the chosen replacement policy
    if (policy == FIFO) {
        // FIFO Replacement Policy
        for (int i = 0; i < cache.associativity; ++i) {
            CacheBlock& block = cache.sets[index][i];
            if (!block.valid) {
                // Empty slot found, insert the block
                block.valid = true;
                block.tag = tag;
                cout << "Inserting block with tag " << tag << " at index " << index << endl;
                return;
            }
        }
        // Replace the first block (FIFO)
        CacheBlock& block = cache.sets[index][0];
        block.tag = tag;
        cout << "Replacing block at index " << index << " with tag " << tag << endl;

    } else if (policy == LRU) {
        // LRU Replacement Policy
        for (int i = 0; i < cache.associativity; ++i) {
            CacheBlock& block = cache.sets[index][i];
            if (!block.valid) {
                // Empty slot found, insert the block
                block.valid = true;
                block.tag = tag;
                lruTracker[index] = i; // Update LRU tracker
                cout << "Inserting block with tag " << tag << " at index " << index << endl;
                return;
            }
        }
        // Find the LRU block and replace it
        int lruIndex = min_element(lruTracker.begin(), lruTracker.begin() + cache.associativity) - lruTracker.begin();
        CacheBlock& block = cache.sets[index][lruIndex];
        block.tag = tag;
        lruTracker[index] = lruIndex; // Update LRU tracker
        cout << "Replacing LRU block at index " << index << " with tag " << tag << endl;

    } else if (policy == RANDOM) {
        // Random Replacement Policy
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, cache.associativity - 1);
        int randomIndex = dis(gen);
        CacheBlock& block = cache.sets[index][randomIndex];
        block.valid = true;
        block.tag = tag;
        cout << "Replacing block at index " << index << " with tag " << tag << " (random replacement)" << endl;
    }
}

// void printCacheState(const Cache& cache) {
//     cout << "Cache State (valid blocks):\n";
//     for (int i = 0; i < cache.numSets; ++i) {
//         cout << "Set " << i << ": ";
//         for (int j = 0; j < cache.associativity; ++j) {
//             const CacheBlock& block = cache.sets[i][j];
//             if (block.valid) {
//                 cout << "Block " << j << " (Tag: " << block.tag << ") ";
//             }
//         }
//         cout << endl;
//     }
// }



void simulateCache(Cache& cache, vector<unsigned int>& addressStream, ReplacementPolicy policy) {
    int hits = 0, misses = 0;
    vector<int> lruTracker(cache.numSets, -1); // Initialize LRU tracker

    for (unsigned int address : addressStream) {
        if (accessCache(cache, address)) {
            hits++;
        } else {
            misses++;
            insertCacheBlock(cache, address, policy, lruTracker);
        }
    }

    cout << "Cache Simulation Results:" << endl;
    cout << "Total accesses: " << (hits + misses) << endl;
    cout << "Hits: " << hits << ", Misses: " << misses << endl;
    cout << "Hit Ratio: " << (double)hits / (hits + misses) << endl;

    // Print cache state after simulation
    // printCacheState(cache);
}

int main() {
    int cacheSize, blockSize;

    cout << "Enter total cache size in bytes (e.g., 1024): ";
    cin >> cacheSize;
    cout << "Enter block size in bytes (e.g., 16): ";
    cin >> blockSize;

    int numSets = cacheSize / blockSize;  // Calculate number of sets (assuming direct-mapped)
    int associativity = 1;  // For direct-mapped cache

    Cache directMappedCache(numSets, associativity, blockSize); // Direct-mapped cache

    // User-defined address stream
    vector<unsigned int> addressStream;
    cout << "Enter the number of addresses in the address stream: ";
    int numAddresses;
    cin >> numAddresses;

    cout << "Enter the addresses (in decimal or hexadecimal):\n";
    for (int i = 0; i < numAddresses; ++i) {
        string addressStr;
        cin >> addressStr;  // Read address as a string

        unsigned int address;
        if (addressStr.find("0x") == 0) {
            // If the address starts with '0x', treat it as hexadecimal
            address = stoul(addressStr, nullptr, 16);
        } else {
            // Otherwise, treat it as decimal
            address = stoul(addressStr);
        }
        
        addressStream.push_back(address);
    }

    // Choose replacement policy
    ReplacementPolicy policy = chooseReplacementPolicy();

    // Run the simulation
    simulateCache(directMappedCache, addressStream, policy);

    return 0;
}
