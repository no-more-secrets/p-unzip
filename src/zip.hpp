/****************************************************************
 * OO Wrappers for libzip types
 ***************************************************************/
#pragma once

#include "ptr_resource.hpp"
#include "utils.hpp"

#include <vector>
#include <zip.h>
#include <time.h>

/****************************************************************
 * ZipStat
 ***************************************************************/
class ZipStat {

public:
    // This is the zero-based index within the archive of the
    // element represented by this ZipStat.
    zip_uint64_t index()     const;
    // File/folder name of entry.  Folder names end with /
    std::string  name()      const;
    // Uncompressed size of entry.
    zip_uint64_t size()      const;
    // Compressed size of entry.
    zip_uint64_t comp_size() const;
    // Last mode time.  This will be rounded to the nearest
    // two-second boundary and contains no timezone.  Also,
    // zip files do not store timezone.  So the time returned
    // by this function must be interpreted based on the
    // known timezone of the machine that created the zip.
    time_t       mtime()     const;
    // Will return true if the entry represents a folder,
    // which is if the name ends in a forward slash.
    bool         is_folder() const;

    ZipStat( zip_stat_t stat ) : stat(stat) {}

private:
    zip_stat_t stat;

};

/****************************************************************
 * Zip
 ***************************************************************/
class Zip : public PtrRes<zip_t, Zip> {

public:
    Zip( Buffer::SP& zs );

    // This returns the size of the vector of cached ZipStats.
    size_t size() const { return stats.size(); }

    // Access the given element of the archive with a zero-based
    // index and return the ZipStat describing it at the time
    // that the zip was originally opened.
    ZipStat const& at(size_t idx) const;

    // Access the given element of the archive with a zero-based
    // index and return the ZipStat describing it at the time
    // that the zip was originally opened.
    ZipStat const& operator[](size_t idx) const {
        return at( idx );
    }

    // Create a new buffer of the size necessary to hold the
    // uncompressed contents, then do the uncompression and
    // return the buffer.
    Buffer extract( size_t idx ) const;

    // Uncompress file into existing buffer.  Throws if the
    // buffer is not big enough.
    void extract_in( size_t idx, Buffer& buffer ) const;

    typedef std::vector<ZipStat>::const_iterator
            const_iterator;

    // These are to support range-based for, and basically
    // just exposed the iteration properties of the vector.
    const_iterator begin() { return stats.begin(); }
    const_iterator end()   { return stats.end();   }

    void destroyer();

private:
    Buffer::SP b;

    std::vector<ZipStat> stats;

};
