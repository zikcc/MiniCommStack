#include <csignal>
#include <atomic>

std::atomic<bool> stop_flag(false);

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        stop_flag = true;
    }
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }
    set_nonblocking(listen_fd);

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(12345);
    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }
    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        close(listen_fd);
        return 1;
    }
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = nullptr;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev) < 0) {
        perror("epoll_ctl ADD listen_fd");
        close(listen_fd);
        close(epfd);
        return 1;
    }

    ThreadPool pool(std::thread::hardware_concurrency());
    std::unordered_map<int, Connection*> conns;

    epoll_event events[MAX_EVENTS];
    while (!stop_flag) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, 1000); // 每秒检测一次 stop_flag
        if (n < 0) {
            if (errno == EINTR) continue; // 信号中断，重试
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < n; ++i) {
            auto *conn = static_cast<Connection*>(events[i].data.ptr);
            int fd = conn ? conn->fd : listen_fd;

            if (!conn) {
                while (true) {
                    int cfd = accept(listen_fd, nullptr, nullptr);
                    if (cfd < 0) break;
                    std::cout << "[INFO] New connection: " << cfd << "\n";
                    set_nonblocking(cfd);

                    Connection* c = new Connection{cfd, "", "", {}};
                    conns[cfd] = c;

                    epoll_event cev{};
                    cev.events = EPOLLIN | EPOLLET;
                    cev.data.ptr = c;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &cev) < 0) {
                        perror("epoll_ctl ADD client");
                        close(cfd);
                        delete c;
                        conns.erase(cfd);
                    }
                }
                continue;
            }

            if (events[i].events & EPOLLIN) {
                char buf[4096];
                while (true) {
                    ssize_t r = read(fd, buf, sizeof(buf));
                    if (r > 0) {
                        conn->inbuf.append(buf, r);
                    } else if (r == 0 || (r < 0 && errno != EAGAIN)) {
                        std::cout << "[INFO] Connection closed: " << fd << "\n";
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                        close(fd);
                        delete conn;
                        conns.erase(fd);
                        break;
                    } else break;
                }

                Packet pkt;
                while (try_parse_packet(conn, pkt)) {
                    pool.enqueue([conn, pkt, &epfd]() mutable {
                        Packet resp;
                        if (pkt.payload.rfind("#weather ", 0) == 0) {
                            std::string city = pkt.payload.substr(9);
                            resp.payload = fetch_weather(city);
                        } else {
                            resp.payload = "Echo: " + pkt.payload;
                        }
                        resp.length   = resp.payload.size();
                        resp.checksum = calculate_checksum(
                            std::vector<uint8_t>(resp.payload.begin(), resp.payload.end())
                        );
                        auto bytes = resp.serialize();
                        {
                            std::lock_guard<std::mutex> lock(conn->mutex); // 加锁保护 outbuf
                            conn->outbuf.append(reinterpret_cast<char*>(bytes.data()), bytes.size());
                        }

                        epoll_event wev{};
                        wev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                        wev.data.ptr = conn;
                        if (epoll_ctl(epfd, EPOLL_CTL_MOD, conn->fd, &wev) < 0) {
                            perror("epoll_ctl MOD EPOLLOUT");
                        }
                    });
                }
            }

            if (events[i].events & EPOLLOUT) {
                std::lock_guard<std::mutex> lock(conn->mutex);
                while (!conn->outbuf.empty()) {
                    ssize_t w = write(fd, conn->outbuf.data(), conn->outbuf.size());
                    if (w > 0) {
                        conn->outbuf.erase(0, w);
                    } else if (errno == EAGAIN) {
                        break;
                    } else {
                        std::cout << "[INFO] Connection write error, closing: " << fd << "\n";
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                        close(fd);
                        delete conn;
                        conns.erase(fd);
                        break;
                    }
                }
                if (conns.count(fd) && conn->outbuf.empty()) {
                    epoll_event rev{};
                    rev.events = EPOLLIN | EPOLLET;
                    rev.data.ptr = conn;
                    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &rev) < 0) {
                        perror("epoll_ctl MOD remove EPOLLOUT");
                    }
                }
            }
        }
    }

    // 优雅退出
    std::cout << "[INFO] Server shutting down...\n";
    for (auto& [fd, conn] : conns) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
        close(fd);
        delete conn;
    }
    conns.clear();

    close(listen_fd);
    close(epfd);
    pool.shutdown(); // 假设你的 ThreadPool 支持 shutdown() 等待线程退出

    return 0;
}
