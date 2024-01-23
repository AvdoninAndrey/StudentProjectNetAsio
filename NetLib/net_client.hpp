#pragma once
#include "net_common.hpp"
#include "net_message.hpp"
#include "net_tcqueue.hpp"
#include "net_connection.hpp"


namespace olc
{
	namespace net
	{
		template<typename T>
		class Client
		{
		private:
			Tsqueue<OwnedMessage<T>> m_qMessagesIn;
		protected:
			asio::io_context m_context;
			std::thread thrContext;
			asio::ip::tcp::socket m_socket;
			std::unique_ptr<Connection<T>> m_connection;
			asio::ip::tcp::endpoint m_endpoints;
		public:
			Client() : m_socket(m_context) {};
			virtual ~Client() { Disconnect(); };

			bool Connect(const std::string& host, const uint16_t port)
			{
				try 
				{
					m_connection = std::make_unique<Connection<T>>();
					asio::ip::tcp::resolver resolver(m_context);
					m_endpoints = resolver.resolve(host, std::to_string(port));

					m_connection->ConnectToServer(m_endpoints);
					thrContext = thread([&]()
						{
							m_context.run();
						});
					return true;
				}
				catch (const std::exception& ex)
				{
					std::cout << "Exception client: " << ex.what() << std::endl;
					return false;
				}
			}

			void Disconnect()
			{
				if (IsConnected())
				{
					m_connection->Disconnect();
				}

				m_context.stop();
				if (thrContext.joinable())
				{
					thrContext.join();
				}

				m_connection.release();
			}

			bool IsConnected()
			{
				if (m_connection)
				{
					return m_connection->IsConnected();
				}
				else
				{
					return false;
				}
			}

			Tsqueue<OwnedMessage<T>>& Incoming()
			{
				return m_qMessagesIn;
			}
		};
	}
}