#pragma once

#ifndef _WIN32

// use the usual shared memory object from boost

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace ext {
    /**
     * Represents a shared memory buffer that will be deleted when no longer referenced by any process.
     */
    class autoremove_shared_memory {
        boost::interprocess::shared_memory_object shm;
        boost::interprocess::mapped_region region;
    public:
        using size_type = boost::interprocess::offset_t;
        template <typename OpenMode = boost::interprocess::open_or_create_t>
        autoremove_shared_memory(const char* name, size_type size, OpenMode open_mode = boost::interprocess::open_or_create, boost::interprocess::mode_t mode = boost::interprocess::read_write) :
            shm(open_mode, name, mode) {
            size_type existing_size = 0;
            shm.get_size(existing_size);
            if (existing_size < size) {
                shm.truncate(size);
            }
            region = boost::interprocess::mapped_region(shm, mode);
        }
        autoremove_shared_memory() {}
        ~autoremove_shared_memory() {
            // unbind the region
            region = boost::interprocess::mapped_region();
            // remove the memory (if possible)
            boost::interprocess::shared_memory_object::remove(shm.get_name());
        }
        void* address() const noexcept {
            return region.get_address();
        }
    };
}

#else

// use the Windows shared memory object on Windows systems, so that it will be backed by kernel instead of filesystem, and have the desired lifetime.

#include <exception>
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace ext {
    /**
    * Represents a shared memory buffer that will be deleted when no longer referenced by any process.
    */
    class autoremove_shared_memory {
        boost::interprocess::windows_shared_memory shm;
        boost::interprocess::mapped_region region;
    public:
        using size_type = std::size_t;
        autoremove_shared_memory(const char* name, size_type size, boost::interprocess::open_or_create_t open_mode = boost::interprocess::open_or_create, boost::interprocess::mode_t mode = boost::interprocess::read_write) :
            shm(open_mode, name, mode, size) {
            if (shm.get_size() < size) {
                throw std::exception("Existing Windows shared memory too small!");
            }
            region = boost::interprocess::mapped_region(shm, mode);
        }
        autoremove_shared_memory(const char* name, size_type size, boost::interprocess::create_only_t open_mode, boost::interprocess::mode_t mode = boost::interprocess::read_write) :
            shm(open_mode, name, mode, size),
            region(shm, mode) {
            // TODO: fix similar issue with open_only mode for POSIX! Also check if open_or_create will throw exception if existing memory too small!
        }
        autoremove_shared_memory(const char* name, size_type size, boost::interprocess::open_only_t open_mode, boost::interprocess::mode_t mode = boost::interprocess::read_write) :
            shm(open_mode, name, mode) {
            if (shm.get_size() < size) {
                throw std::exception("Existing Windows shared memory too small!");
            }
            region = boost::interprocess::mapped_region(shm, mode);
        }
        autoremove_shared_memory() noexcept {}
        ~autoremove_shared_memory() noexcept {}
        void* address() const noexcept {
            return region.get_address();
        }
    };
}

#endif