#include <iostream>
#include <vector>
#include <algorithm>


#ifndef IBF_HPP
#define IBF_HPP


static const int IBF_DEFAULT_SIZE = 50;
static const int IBF_DEFAULT_QTD_HASH_FUNCTIONS = 3;



class InvertibleBloomFilter {
public:

    InvertibleBloomFilter(int size, int hashFunctions)
        : size(size), hashFunctions(hashFunctions), ibf(size, 0), count(0) {}

    InvertibleBloomFilter(int size, int hashFunctions, int count, std::vector<size_t> ibf)
        : size(size), hashFunctions(hashFunctions), ibf(ibf), count(count) {}

    void insert(const std::string& element) {
        for (int i = 0; i < hashFunctions; ++i) {
            size_t hashValue = hash(element, i);
            ibf[hashValue % size] ^= hashValue;
        }
        ++count;
    }

    void remove(const std::string& element) {
        for (int i = 0; i < hashFunctions; ++i) {
            size_t hashValue = hash(element, i);
            ibf[hashValue % size] ^= hashValue;
        }
        --count;
    }

    bool contains(const std::string& element) const {
        for (int i = 0; i < hashFunctions; ++i) {
            size_t hashValue = hash(element, i);
            if ((ibf[hashValue % size] & hashValue) != hashValue) {
                return false;
            }
        }
        return true;
    }

    bool is_empty() const {
        return count == 0;
    }


    int get_count() const {
        return count;
    }

private:
    std::vector<size_t> ibf;
    int size;
    int hashFunctions;
    int count;

    size_t hash(const std::string& element, int index) const {
        return std::hash<std::string>{}(element + std::to_string(index));
    }
};

#endif // IBF_HPP
