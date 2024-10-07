#ifndef CAPIO_CREATE_HPP
#define CAPIO_CREATE_HPP

/**
 * @brief Handle the create systemcall
 *
 * @param str raw request as read from the shared memory interface stripped of the request number
 * (first parameter of the request)
 */
inline void create_handler(const char *const str) {
    pid_t tid, fd;
    char path[PATH_MAX];
    sscanf(str, "%d %d %s", &tid, &fd, path);
    START_LOG(gettid(), "call(tid=%d, path=%s)", tid, path);
    file_manager->unlockThreadAwaitingCreation(path);
    std::string name(client_manager->get_app_name(tid));
    capio_cl_engine->addProducer(path, name);
}

#endif // CAPIO_CREATE_HPP
