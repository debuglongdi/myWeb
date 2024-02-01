# Webserver

[![Build Status](https://travis-ci.org/linyacool/WebServer.svg?branch=master)](https://travis-ci.org/linyacool/WebServer) [![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://opensource.org/licenses/MIT)

## Introduction

本项目实现了一个基于Reactor模式的Web服务器，传输层采用I/O多路复用技术管理TCP连接，同时在应用层搭建了一个http服务器，用于响应Get请求，实现静态资源的访问。最后，本项目还可以在Web服务器的基础上自由扩展应用层服务。

Enjoy it，it's gonna be really fun!!!

## Environment

- OS: Ubuntu 16.04
- Complier: g++ 5.4.0
- Tools: CMake/VScode

## Technical points

* 基于Reactor模式构建网络服务器，编程风格偏向面向过程


## Develop and Fix List

* 

## 开发记录

### 项目构建
### 
###
###
###



## Server Performance

* 使用Ringbuffer减少数据的拷贝拷贝次数，在一分钟内测试1000个并发连接，使用RingBuffer使得QPS提升10%

### Simple Comparison



### Analysis

* 测试结果表明，本项目所实现的服务器与muduo性能相近！
* 在搭建这个服务器的过程中，我的基本框架还有代码的风格(甚至一些直接的代码都是来自Muduo那本书)！但是呢，在慢慢搭建的过程中，我修改了Epoll的工作模式(LT->ET)，改进了缓冲区Buffer为环形缓冲区，还有线程间的异步任务调配我也为了避免锁的开销，改成了无锁结构，相比在这些做工作之前的服务器，我这个版本是有很大的性能提升的，这也是Muduo所没有的。