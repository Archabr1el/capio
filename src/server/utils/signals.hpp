#ifndef CAPIO_SERVER_HANDLERS_SIGNALS_HPP
#define CAPIO_SERVER_HANDLERS_SIGNALS_HPP

#include <csignal>

void sig_term_handler(int signum, siginfo_t *info, void *ptr) {
    START_LOG(gettid(), "call(signal=[%d] %s)", signum, strsignal(signum));

    std::cout << std::endl
              << CAPIO_SERVER_CLI_LOG_SERVER_WARNING << "shutting down server" << std::endl;

    if (signum == SIGSEGV) {
        std::cout << CAPIO_SERVER_CLI_LOG_SERVER_ERROR << "Segfault detected!" << std::endl;
    }

    // free all the memory used
    for (auto &it : get_capio_fds()) {
        for (auto &fd : it.second) {
            delete_capio_file_from_tid(it.first, fd);
        }
    }
    std::cout << CAPIO_SERVER_CLI_LOG_SERVER_WARNING << "shm cleanup completed" << std::endl;

    for (auto &p : data_buffers) {
        p.second.first->free_shm();
        p.second.second->free_shm();
    }

    std::cout << CAPIO_SERVER_CLI_LOG_SERVER_WARNING << "data_buffers cleanup completed"
              << std::endl;

    destroy_server();
    std::cout << CAPIO_SERVER_CLI_LOG_SERVER << "shutdown completed" << std::endl;
    MPI_Finalize();
    exit(EXIT_SUCCESS);
}

void setup_signal_handlers() {
    START_LOG(gettid(), "call()");
    static struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_sigaction = sig_term_handler;
    sigact.sa_flags     = SA_SIGINFO;
    int res             = sigaction(SIGTERM, &sigact, nullptr);
    res                 = res | sigaction(SIGILL, &sigact, nullptr);
    res                 = res | sigaction(SIGABRT, &sigact, nullptr);
    res                 = res | sigaction(SIGFPE, &sigact, nullptr);
    res                 = res | sigaction(SIGSEGV, &sigact, nullptr);
    res                 = res | sigaction(SIGQUIT, &sigact, nullptr);
    res                 = res | sigaction(SIGPIPE, &sigact, nullptr);
    res                 = res | sigaction(SIGINT, &sigact, nullptr);
    if (res == -1) {
        ERR_EXIT("sigaction for SIGTERM");
    }
}

#endif // CAPIO_SERVER_HANDLERS_SIGNALS_HPP