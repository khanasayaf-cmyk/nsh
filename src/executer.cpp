#include "executer.hpp"
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "builtins.hpp"

void Executer::execute(std::vector<std::string>& tokens) {
    if (Builtins::handle(tokens)) {
        return;
    }

    std::string outputFile;
    bool background = false;

    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i] == "&") {
            background = true;
            tokens.erase(tokens.begin() + i);
            break;
        }
    }

    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i] == ">") {
            if (i + 1 < tokens.size()) {
                outputFile = tokens[i + 1];
                tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
            }
            break;
        }
    }

    std::vector<const char*> argv;

    for (const std::string& token : tokens) {
        argv.push_back(token.c_str());
    }

    argv.push_back(nullptr);

    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << tokens[0] << ": failed to execute command" << std::endl;
    } else if (pid == 0) {
        if (!outputFile.empty()) {
            int file_fd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(file_fd, STDOUT_FILENO);
            close(file_fd);
        }

        int status = execvp(argv[0], const_cast<char* const*>(argv.data()));

        if (status != 0) {
            std::string msg = "failed to execute command";

            if (errno == ENOENT) {
                msg = "command not found";
            }

            std::cerr << tokens[0] << ": " << msg << std::endl;
        }
    } else {
        if (!background) {
            waitpid(pid, nullptr, 0);
        }
    }
}
