#ifndef CAPIO_STORAGE_SERVICE_H
#define CAPIO_STORAGE_SERVICE_H

#include "CapioFile/CapioFile.hpp"
#include "CapioFile/CapioMemoryFile.hpp"

class CapioStorageService {

    std::unordered_map<std::string, CapioFile *> *_stored_files;

  public:
    CapioStorageService() {
        START_LOG(gettid(), "call()");
        _stored_files = new std::unordered_map<std::string, CapioFile *>;
        std::cout << CAPIO_SERVER_CLI_LOG_SERVER << " [ " << node_name << " ] "
                  << "CapioStorageService initialization completed." << std::endl;
    }

    ~CapioStorageService() { delete _stored_files; }

    void createFile(const std::string &file_name) const {
        _stored_files->emplace(file_name, new CapioMemoryFile(file_name));
    }

    void deleteFile(const std::string &file_name) const { _stored_files->erase(file_name); }

    [[nodiscard]] auto getFile(const std::string &file_name) const {
        if (_stored_files->find(file_name) == _stored_files->end()) {
            createFile(file_name);
            return _stored_files->at(file_name);
        }
        return _stored_files->at(file_name);
    }

    /**
     * Notify the occurrence of an operation on a given file, for which other servers running at a
     * certain point might be required to know. This function is used to allow CAPIO to function in
     * the event that a distributed file system (or at least a shared directory) is not available
     */
    void notifyEvent(const std::string &event_name, const std::filesystem::path &filename) const {
        // TODO: implement this
    }
};

inline CapioStorageService *storage_service;

#endif // CAPIO_STORAGE_SERVICE_H
