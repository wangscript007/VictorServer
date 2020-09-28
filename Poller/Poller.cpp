/*
@Author: Jing
@Date: 2020.09.27
@Desc: epoll的封装
*/

#include <assert.h>
#include <string.h>
#include <strings.h>
#include <errno.h> // errno每一线程都有一份

#include "Poller.h"
#include "../Log/Logger.h"


const int EPOLL_WAIT_TIME = 10000;

Poller::Poller(int max_events):
    max_events(max_events),
    m_epoll_events(max_events),
    m_epoll_fd(epoll_create1(EPOLL_CLOEXEC)){}

Poller::~Poller(){}

void Poller::poll(std::vector<std::shared_ptr<Channel>>& activeChannels){
    int active_num = epoll_wait(m_epoll_fd, &(*m_epoll_events.begin()), max_events, EPOLL_WAIT_TIME);
    if(active_num > 0){
        LOG_INFO("Epoll Wait: %d events happened.", active_num);
        getActiveChannels(active_num, activeChannels);
    }
    else if(active_num == 0){
        LOG_INFO("Epoll Wait Timeout.");
    }
    else{
        if(errno == EINTR){
            LOG_INFO("Epoll Wait is Interrupted.");
        }
        else{
            LOG_ERROR("Epoll Wait Error: %s", strerror(errno));
        }
    }
}

void Poller::getActiveChannels(int active_num, std::vector<std::shared_ptr<Channel>>& activeChannels){
    assert(active_num <= max_events);
    for(int i = 0; i < active_num; i++){
        struct epoll_event event = m_epoll_events[i];
        Channel* pChannel = static_cast<Channel*>(event.data.ptr);
        pChannel->setRevents(event.events);
        activeChannels.push_back(pChannel->getSelf());
    }
    // 需要清空m_epoll_events吗？muduo没有.
}

void Poller::addChannel(std::shared_ptr<Channel> channel){
    if(channel){
        struct epoll_event event;
        bzero(&event, sizeof(event));
        event.events = channel->getEvents();
        event.data.ptr = channel.get();
        int fd = channel->getFd();
        int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event);
        if(ret < 0){
            LOG_ERROR("Add Channel Error: %s.", strerror(errno));
        }
    }
}

void Poller::delChannel(std::shared_ptr<Channel> channel){
    if(channel){
        int fd = channel->getFd();
        int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
        if(ret < 0){
            LOG_ERROR("Del Channel Error: %s.", strerror(errno));
        }
    }
}

// 主要是为了修改监听的事件
void Poller::modChannel(std::shared_ptr<Channel> channel){
    if(channel){
        // 如果前后想要监听的事件一致，那么跳过
        if(channel->getEvents() != channel->getLastEvents()){
            int fd = channel->getFd();
            struct epoll_event event;
            event.events = channel->getEvents();
            event.data.ptr = channel.get();
            int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &event);
            if(ret < 0){
                LOG_ERROR("Mod Channel Error: %s.", strerror(errno));
            }
        }
    }
}