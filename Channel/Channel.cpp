/*
@Author: Jing
@Date: 2020.09.27
@Desc: Channel
*/

#include <sys/epoll.h>

#include "Channel.h"


Channel::Channel(){
    m_fd = -1;
    m_events = 0;
    m_revents = 0;
    m_last_events = 0;
}

Channel::Channel(int fd){
    m_fd = fd;
    m_events = 0;
    m_revents = 0;
    m_last_events = 0;
}

Channel::~Channel(){}

void Channel::setFd(int fd){
    m_fd = fd;
}

int Channel::getFd(){
    return m_fd;
}

void Channel::setReadHandler(Callback&& read_handler){
    m_read_handler = read_handler; // 移动拷贝
}

void Channel::setWriteHandler(Callback&& write_handler){
    m_write_handler = write_handler;
}

void Channel::setErrorHandler(Callback&& error_handler){
    m_error_handler = error_handler;
}

void Channel::setConnHandler(Callback&& conn_handler){
    m_conn_handler = conn_handler;
}

void Channel::setHolder(std::shared_ptr<HttpData> holder){
    m_wk_holder = holder;
}

std::shared_ptr<HttpData> Channel::getHolder(){
    return m_wk_holder.lock();
}


void Channel::setEvents(__uint32_t events){
    m_events = events;
}

__uint32_t Channel::getEvents(){
    return m_events;
}

void Channel::setRevents(__uint32_t revents){
    m_revents = revents;
}

__uint32_t Channel::getRevents(){
    return m_revents;
}

void Channel::setLastEvents(__uint32_t last_events){
    m_last_events = last_events;
}

__uint32_t Channel::getLastEvents(){
    return m_last_events;
}


void Channel::handleRead(){
    if(m_read_handler){
        m_read_handler();
    }
}

void Channel::handleWrite(){
    if(m_write_handler){
        m_write_handler();
    }
}

void Channel::handleConn(){
    if(m_conn_handler){
        m_conn_handler();
    }
}

void Channel::handleError(){
    if(m_error_handler){
        m_error_handler();
    }
}

void Channel::handleEvents(){
    // 更新m_last_events;
    m_last_events = m_events;
    // 在下面的处理过程中，更新m_events
    m_events = 0; 

    if(m_revents & (EPOLLHUP | EPOLLERR)){
        handleError();
        return;
    }
    else{
        if(m_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
            handleRead();
        }
        if(m_revents & EPOLLOUT){
            handleWrite();
        }
    }
    // 是否执行handleConn()函数？
    // TODO: 可能还需要修改
}


std::shared_ptr<Channel> Channel::getSelf(){
    std::shared_ptr<Channel> p = shared_from_this();
    return p;
}
