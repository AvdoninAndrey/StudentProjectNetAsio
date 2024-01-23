#pragma once
#include "net_common.hpp"
#include "net_tcqueue.hpp"
#include "net_message.hpp"
#include "net_connection.hpp"

namespace olc
{
	namespace net
	{
		template<typename T>
		class ServerInterface
		{
		private:
		public:
			//�������������� ��� ������� ������� ���������� � ������� �� ����� ����� ������� ���������� � �� ��� ������������ ipv4
			ServerInterface(uint16_t port) : m_acceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			{
			
			}
			virtual ~ServerInterface() { Stop(); };

			bool Start()
			{
				try
				{
					WaitForConnection(); // ��������� ����������� ������� (����� ������������ ������ ��������)
					thrContext = std::thread([&]() { m_context.run(); })
				}
				catch (const std::exception& ex)
				{
					std::cerr << "SERVER[EXCEPTION]: " << ex.what() << std::endl;
					return false;
				}

				std::cout << "SERVER[STARTED]\n";
				return true;
			}
			void Stop()
			{
				m_context.stop();

				if (thrContext.joinable()) // ���� ����� � �������� ��������� �� ��� ��� ������������� 
					// �������� ����� ����� ��������� ����� (�������� �� ��� �� ������� ��� ��� ���������� ������)
				{
					thrContext.join();
				}
				std::cout << "SERVER[STOPED]\n";
			}
			void WaitForConnection()
			{
				m_acceptor.async_accept(
					[&](std::error_code& error, asio::ip::tcp::socket socket)
					{
						if (!ec)
						{
							std::cout << "SERVER[NEW CONNECTION]" << socket.remote_endpoint() "\n"; //ip-a���� �����������

							//� ����������� ���������� �������� ����, ��� ��� ���������� �� ������� �������
							//������� asio, ����� ������� � ������� �������� ��������� (������� ���� ��� ������� � �������� ����������������)
							std::shared_ptr<Connetion<T>> newConnection = std::make_shared<Connetion<T>>(
								connection<T>::owner::server,
								m_context, std::move(socket), m_qMessagesIn
								);
							if (OnClientConnect(newConnection)) // ���� ������������ �� �������� ����������
							{
								m_deqConnetion.push_back(std::move(newConnection)); //��������� ���������� � ���������
								m_deqConnetion.back()->ConnectToClient(ipCounter++); //���������� ��������� ��������������
								std::cout << "[" << m_deqConnetion.back()->GetID << "] Connetion Aproved\n"; //����������� ��������
							}
							else
							{
								std::cout << "[---] Connection Denied\n"
							}
						}
						else
						{
							std::cout << "SERVER[NEW CONNECTION ERROR]" << error.message() << "\n";
						}
						WaitForConnection();
					}
				);
			}
			//��� �������� ��������� �������
			void SendMessageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg )
			{
				if (client && client->IsConnected()) // ���� ���������� ���������� � ������ ���������
				{
					client->Send(msg);
				}
				else
				{
					OnClientDisconnect(client); // ������ �������������, ��� ������ �������� � ���������� �������� �������� ����������
					client.reset(); // ���������� �������� ������� � ���� ��� ��������� ������ �� ������ ������� ������
					//remove ���������� ��������� �������� � ����� � ���������� �������� �� ��� �������, ����� erase �������
					m_deqConnetion.erase(std::remove(m_deqConnetion.begin(), m_deqConnetion.end(), client), m_deqConnetion.end());
				}
			}
			//��� �������� ��������� ���� ��������
			void SendMessageAllClients(const Message<T>& msg, std::shared_ptr<Connection<T>> clientIgnore = nullptr)
			{
				bool flagInvalidClientExists = false;
				for (auto& client : m_deqConnetion)
				{
					if (client && client->IsConnected())
					{
						if (client != clientIgnore)
						{
							client->Send(msg);
						}
					}
					else
					{
						OnClientDisconnect(client);
						client.reset();
						flagInvalidClientExists = true; // �������� ��� ���� ���������� �������
					}
				}
				if (flagInvalidClientExists) // ���� ���� ���������� �������, �� ������� �� �� ���������� ����������
				{
					m_deqConnetion.erase(std::remove(m_deqConnetion.begin(), m_deqConnetion.end(), nullptr), m_deqConnetion.end());
				}
			}

			void Update(size_t nMaxMessages = -1) // ��� ��� size_t ����������� ���, �� �� ����� ���� ��� ����� ����� ������� ��� ����� 
			{
				size_t nMessageCount = 0;
				while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
				{
					auto msg = m_qMessagesIn.pop_front();
					OnMessage(msg.remote, msg.msg);
					++nMessageCount;
				}
			}
		protected:
			virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client)
			{
				return false;
			}
			virtual bool OnClientDisconnect(std::shared_ptr<Connection<T>> client, Message<T> & msg)
			{

			}

			virtual void OnMessage(std::shared_ptr<Connection<T>> client, Message<T>& msg)
			{

			}
		protected:
			Tsqueue<OwnedMessage<T>> m_qMessagesIn;
			asio::io_context m_context;
			std::thread thrContext;
			std::deque<std::shared_ptr<Connection<T>>> m_deqConnetion; //��������� �� ����������

			asio::ip::tcp::acceptor m_acceptor; // ����������� ������ ��� �������� ������� ��������
			uint32_t ipCounter = 10000; // ���������� �������������� ��������
		};
	}
}

