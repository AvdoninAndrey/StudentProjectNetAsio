#pragma once
#include "net_common.hpp"
#include "net_tcqueue.hpp"
#include "net_message.hpp"

namespace olc
{
	namespace net
	{
		template <typename T>
		class Connection : public std::enable_shared_from_this<Connection<T>>
		{
		public:
			Connection() {};
			virtual ~Connection(){}

			bool ConnectToServer();

			bool IsConnected()
			{
				return true;
			}
			bool Disconnect();

			bool Send(const Message<T>& msg);
		protected:
			asio::ip::tcp::socket m_socket;
			asio::io_context& m_asioContext;

			Tsqueue<Message<T>> m_qMessagesOut;
			Tsqueue<OwnedMessage<T>>& m_qMessagesIn;
		};
	}
}