/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>
#include <utility>


using namespace std;

// TODO: Implement your own HashMap and Cache classes.

//-----------------------
// Define HashMap classes
//-----------------------
// To use HashMap ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class HashKey
// {
// public:
//    HashKey() {}
//
//    size_t operator() () const { return 0; }
//
//    bool operator == (const HashKey& k) const { return true; }
//
// private:
// };
//
template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap() : _numBuckets(0), _buckets(0) {}
   HashMap(size_t b) : _numBuckets(0), _buckets(0) { init(b); }
   ~HashMap() { reset(); }

   // [Optional] TODO: implement the HashMap<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the HashMap
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   // (_bId, _bnId) range from (0, 0) to (_numBuckets, 0)
   //
   class iterator
   {
      friend class HashMap<HashKey, HashData>;

   public:
      const HashNode& operator * () const {return *subitr;}
      HashNode& operator* () {return *subitr;}
      iterator& operator ++ ()
      {
          if(subitr == bucketlist[nBucket-1].end())
          {
              return (*this);
          }
          if(subitr != bucketlist[n].end()-1)
          {
              subitr++;
              return (*this);
          }
          else
          {
              for(size_t i = n+1; i < nBucket; i++)
              {
                    if(bucketlist[i].size() != 0)
                    {
                        n = i;
                        subitr = bucketlist[n].begin();
                        return (*this);
                    }
              }
              n = nBucket -1;
              subitr = bucketlist[n].end();
              return (*this);
          }
      }

   private:
      size_t n;
      vector<HashNode>* bucketlist;
      size_t nBucket;
      typename vector<HashNode>::iterator subitr;
   };

   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b];
      _numAllBuckets = _numBuckets;}
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
      _numAllBuckets = _numBuckets;
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   bool resize(size_t b)
   {
       if(b>=_numAllBuckets)
           init(b);
       else
       {
           _numBuckets = b;

           for(unsigned i =0; i <b; i++)
           {
               _buckets[i].clear();
           }
       }

   }


   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const { iterator(); }
   // Pass the end
   iterator end() const { iterator(); }
   // return true if no valid data
   bool empty() const { return size() == 0; }
   // number of valid data
   size_t size() const
   {
       size_t count = 0;
       for(size_t i = 0;i != _numBuckets;i++)
       {
           count += _buckets[i].size();
       }
       return count;
   }
   bool retrive(const HashKey& k, HashData& n)
   {
       vector<HashNode>* curbucket = _buckets + ( k() % _numBuckets);
       if(curbucket->empty())
       {
           return false;
       }
       for(typename vector<HashNode>::const_iterator itr = curbucket->begin();
           itr != curbucket->end(); itr++)
       {
           if(itr->first == k)
           {
               n = itr->second;
               return true;
           }
       }
       return false;
   }

   void quickInsert(const HashKey& k, const HashData& d)
   {
       _buckets[k() % _numBuckets].push_back(make_pair(k,d));
   }
   void clear()
   {
       for(unsigned i = 0; i<_numBuckets;i++)
       {
           _buckets[i].clear();
       }
   }

private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   size_t                   _numAllBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//
//    size_t operator() () const { return 0; }
//
//    bool operator == (const CacheKey&) const { return true; }
//
// private:
// };
//
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { }
   void reset() { }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const { return false; }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) { }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
