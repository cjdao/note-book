#include <iostream>
#include <cassert>
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include "leveldb/comparator.h"

int main(int argc, char *argv[])
{

    // # Opening A Database
    // ===========================
    // A leveldb database has a name which corresponds to a file system directory. 
    // All of the contents of database are stored in this directory. 
    leveldb::DB *db;

    leveldb::Options options;
    options.create_if_missing = true;

    // If you want to raise an error if the database already exists, 
    // add the following line before the leveldb::DB::Open call
    // options.error_if_exists = true;

    leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
    assert(status.ok());

    // # Status
    // ===========================
    // Values of this type(leveldb::Status) are returned by most functions in leveldb 
    // that may encounter an error. You can check if such a result is ok, 
    // and also print an associated error message: 

    // leveldb::Status s = ...
    // if (!s.ok()) std::cerr << s.ToString() << std::endl;

    // # Slice
    // ===========================
    // Slice is a simple structure that contains a length and a pointer to an external byte array. 
    // Returning a Slice is a cheaper alternative to returning a std::string since we do not need 
    // to copy potentially large keys and values. 
    // In addition, leveldb methods do not return null-terminated C-style strings since leveldb 
    // keys and values are allowed to contain '\0' bytes.

    // C++ strings and null-terminated C-style strings can be easily converted to a Slice: 
    leveldb::Slice s1 = "hello";
    std::string str("world");
    leveldb::Slice s2 = str;

    // A Slice can be easily converted back to a C++ string: 
    str = s1.ToString();
    assert(str == std::string("hello"));

    // Be careful when using Slices since it is up to the caller to ensure that the external 
    // byte array into which the Slice points remains live while the Slice is in use.

    // leveldb::Slice slice;
    // if (...) {
    //     std::string str = ... ;
    //     slice = str;
    // }
    // Use(slice);

    // When the if statement goes out of scope, 
    // str will be destroyed and the backing storage for slice will disappear. 
    


    std::string init_val("Hello world!");
    std::string value1;
    leveldb::Slice key1 = "key1";
    leveldb::Slice key2(std::string("key2"));
    leveldb::Status s;

    // # Reads And Writes
    // ===========================
    // The database provides Put, Delete, and Get methods to modify/query the database.
    s = db->Put(leveldb::WriteOptions(), key1, init_val);
    if (s.ok()) s = db->Get(leveldb::ReadOptions(), key1, &value1);

    assert(s.ok() && (value1==init_val));

    s = db->Put(leveldb::WriteOptions(), key2, value1);
    if (s.ok()) s = db->Delete(leveldb::WriteOptions(), key1);
    if (s.ok()) {
        std::string value2;
        s = db->Get(leveldb::ReadOptions(), key2, &value2);
        assert(s.ok() && (value2==value1));
    }

    // # Atomic Updates
    // ===========================
    // Note that if the process dies after the Put of key2 but before the delete of key1,
    // the same value may be left stored under multiple keys. Such problems can be avoided
    // by using the WriteBatch class to atomically apply a set of updates: 
    s = db->Put(leveldb::WriteOptions(), key1, init_val);
    if (s.ok()) s = db->Get(leveldb::ReadOptions(), key1, &value1);
    if (s.ok()) {
        leveldb::WriteBatch write_batch;
        write_batch.Delete(key1);
        write_batch.Put(key2, value1);
        s = db->Write(leveldb::WriteOptions(), &write_batch);
    }
    assert(s.ok());
    // The WriteBatch holds a sequence of edits to be made to the database, 
    // and these edits within the batch are applied in order. 
    // Note that we called Delete before Put so that if key1 is identical to key2, 
    // we do not end up erroneously dropping the value entirely.
    // Apart from its atomicity benefits, WriteBatch may also be used to speed up 
    // bulk updates by placing lots of individual mutations into the same batch.

    // # Synchronous Writes
    // ===========================
    // By default, each write to leveldb is asynchronous: it returns after pushing 
    // the write from the process into the operating system. The transfer from operating 
    // system memory to the underlying persistent storage happens asynchronously. 
    // The sync flag can be turned on for a particular write to make the write operation 
    // not return until the data being written has been pushed all the way to persistent 
    // storage. (On Posix systems, this is implemented by calling either fsync(...) or fdatasync(...)
    // or msync(..., MS_SYNC) before the write operation returns.)

    // leveldb::WriteOptions write_options;
    // write_options.sync = true;
    // db->Put(write_options,...);
    

    // # Concurrency
    // ===========================
    // A database may only be opened by one process at a time. The leveldb implementation 
    // acquires a lock from the operating system to prevent misuse. Within a single process, 
    // the same leveldb::DB object may be safely shared by multiple concurrent threads. I.e.,
    // different threads may write into or fetch iterators or call Get on the same database 
    // without any external synchronization (the leveldb implementation will automatically 
    // do the required synchronization). However other objects (like Iterator and WriteBatch) 
    // may require external synchronization. If two threads share such an object, they must 
    // protect access to it using their own locking protocol. More details are available in the public header files.
    
    // # Iteration
    // ===========================
    // The following example demonstrates how to print all key,value pairs in a database.
    leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
    }
    assert(it->status().ok());   // Check for any errors found during the scan
    delete it;
    
    // The following variation shows how to process just the keys in the range [start, limit):
    // for ( it->Seek(start); 
    //       it->Valid() && it->key().ToString()<limit; 
    //       it->Next() ) {
    //     // ... 
    // }

    // You can also process entries in reverse orders.(Caveat: reverse iteration maybe somewhat slower than forward iteration.)
    // for ( it->Seek(start); it->Valid(); it->Prev()) {
    //     // ...
    // }
    
    // # Snapshots
    // ===========================
    // Snapshots provide consistent read-only views over the entire state of the key-value store.
    // ReadOptions::snapshot may be non-NULL to indicate that a read should operate on a particular 
    // version of the DB state. If ReadOptions::snapshot is NULL, the read will operate on an implicit
    // snapshot of the current state.
    // snapshot are created by DB::GetSnapshot() method:
    leveldb::ReadOptions read_options;
    read_options.snapshot = db->GetSnapshot();
    // ... apply some updates to do 
    leveldb::Iterator *iter = db->NewIterator(read_options);
    // ... read using iter to view the state when the snapshot was created 
    delete iter;
    // Note that when a snapshot is no longer needed, it should be released using the DB::ReleaseSnapshot interface.
    // This allows the implementation to get rid of state that was being maintained just to support reading as of that snapshot.
    db->ReleaseSnapshot(read_options.snapshot);

    // # Comporators
    // ===========================
    // The preceding examples used the default ordering function for key, which orders bytes lexicographically.
    // You can also supply a custom comparator when opening a database. For example, suppose each database key consists of 
    // two numbers and we should sort by the first number, breaking ties by the second number.
    // First, define a proper subclass of leveldb::Comparator that expresses thes rules:
    class TwoPartComparator : public leveldb::Comparator {
		void Parsekey(const leveldb::Slice &s, int *a, int *b) const{
					
		}
    public:
        // Threa-way comparison function:
        // if a > b : negative result
        // if a < b : positive result
        // else : zero result 
        int Compare(const leveldb::Slice &a, const leveldb::Slice &b) const {
            int a1, a2, b1, b2;
            Parsekey(a, &a1, &a2);
            Parsekey(b, &b1, &b2);
            if (a1 < b1) return -1;
            if (a1 > b1) return +1;
            if (a2 < b2) return -1;
            if (a2 > b2) return +1;
            return 0;
        };

        // Ignore the following methods for now:
        const char* Name() const {return "TwoPartComparator";};
        void FindShortestSeparator(std::string*, const leveldb::Slice&) const { }
        void FindShortSuccessor(std::string*) const { }
    };
    
    // Now, create a database using this custom comparator:
    TwoPartComparator cmp;
    leveldb::DB *db2;
    leveldb::Options options2;
    options2.create_if_missing = true;
    options2.comparator = &cmp; 
    leveldb::Status status2  = leveldb::DB::Open(options2, "/temp/testdb2", &db2);
    assert(status2.ok());


    // # CLosing A Database
    // ===========================
    // When you are done with a database, just delete the database object.
    delete db;

    std::cout << std::endl ;
    std::cout << "=============================" << std::endl;
    std::cout << "LevelDB example finished!" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::endl ;
    return 0;
}
