/*
@Author: Jing
@Date: 2020.9.23
@Desc: 服务器类，总体
*/

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "Server.h"
#include "../Utils/Utils.h"
#include "../Log/Logger.h"

Server::Server(Config config, EventLoop* loop):
    m_listen_fd(socket_bind_listen(config.server_port)),
    m_listen_channel(new Channel(m_listen_fd)),
    m_loop(loop),
    m_running(false),
    m_is_nolinger(config.no_linger),
    m_thread_pool(new EventLoopThreadPool(loop, config.num_thread))
{   
    if(!set_sock_non_blocking(m_listen_fd)) abort();
    if(!set_sock_reuse_port(m_listen_fd)) abort();

    m_listen_channel->setReadHandler(std::bind(&Server::readHandler, this));
    m_listen_channel->setConnHandler(std::bind(&Server::connHandler, this));
    m_listen_channel->setEvents(EPOLLIN | EPOLLET); // 读事件 和 边缘触发
}

// 应该退出各个loop
// 关闭各个套接字?
Server::~Server(){
    m_running = false;
    m_loop->quit(); // quit来关闭套接字？
    m_thread_pool->stop();
    close(m_listen_fd);
    // TODO
}


void Server::start(){
    if(m_running) return;
    m_running = true;
    m_thread_pool->start(); // 开启线程池
    m_loop->addToPoller(m_listen_channel);
}

void Server::readHandler(){
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int connfd = -1;
    while((connfd = accept(m_listen_fd, (struct sockaddr*)&client_addr, &addr_len)) >= 0){
        // 打印连接的客户端的IP和端口号
        LOG_INFO("The Client %s : %p Connected to Server.", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        // 设置connfd为非阻塞
        set_sock_non_blocking(connfd);
        // 关闭Nagle算法
        set_sock_nodelay(connfd);
        // 是否优雅关闭？
        if(m_is_nolinger)   set_sock_nolinger(connfd);
        // 获取下一个loop
        EventLoop* loop = m_thread_pool->getNextLoop();
        // 创建HttpData对象
        // 设置HttpData对象中的Channel的Holder
        // TODO
    }
}

void Server::connHandler(){
    // TODO
}