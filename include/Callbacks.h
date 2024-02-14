#pragma once
#include <memory>
#include <functional>
#include "Timestamp.h"

namespace mymuduo
{
namespace net
{
class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
//
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;

//the data has been red to (buf, len)
using MessageCallback = std::function<void(const TcpConnectionPtr&,
                                        Buffer*,
                                        Timestamp)>;


} // namespace net

} // namespace mymuduo
