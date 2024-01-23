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
			//инициалаизруем наш приёмник сокетов контекстом и говорим на каком порту слушать соединение и то что используется ipv4
			ServerInterface(uint16_t port) : m_acceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			{
			
			}
			virtual ~ServerInterface() { Stop(); };

			bool Start()
			{
				try
				{
					WaitForConnection(); // дождаться подключения клиента (чтобы поддерживать работу контекса)
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

				if (thrContext.joinable()) // если поток в активном состоянии то идёт его присоединение 
					// проверка нужна чтобы проверить поток (возможно он уже не рабочий или был присоединён раннее)
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
							std::cout << "SERVER[NEW CONNECTION]" << socket.remote_endpoint() "\n"; //ip-aдрес подключения

							//В конструктор соединения передаётс флаг, что это соединение на стороне сервера
							//контекс asio, сокет клиента и очередь входящих сообщений (которая одна для сервера и является потокобезопасной)
							std::shared_ptr<Connetion<T>> newConnection = std::make_shared<Connetion<T>>(
								connection<T>::owner::server,
								m_context, std::move(socket), m_qMessagesIn
								);
							if (OnClientConnect(newConnection)) // если пользователь не отклонил соединение
							{
								m_deqConnetion.push_back(std::move(newConnection)); //добавляем соединение в контейнер
								m_deqConnetion.back()->ConnectToClient(ipCounter++); //увеличивем возможные идентификаторы
								std::cout << "[" << m_deqConnetion.back()->GetID << "] Connetion Aproved\n"; //подключение одобрено
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
			//для отправки сообщения клиенту
			void SendMessageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg )
			{
				if (client && client->IsConnected()) // если соединение существует и клиент подключён
				{
					client->Send(msg);
				}
				else
				{
					OnClientDisconnect(client); // делаем предположение, что клиент отключён и производим действия закрытия соединения
					client.reset(); // сбрасываем значения объекта и если это последняя ссылка на объект очищаем память
					//remove перемещает найденные элементы в конец и возвращает итератор на эту позицию, затем erase удаляет
					m_deqConnetion.erase(std::remove(m_deqConnetion.begin(), m_deqConnetion.end(), client), m_deqConnetion.end());
				}
			}
			//для отправки сообщения всем клиентам
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
						flagInvalidClientExists = true; // отмечаем что есть невалидные клиенты
					}
				}
				if (flagInvalidClientExists) // если были невалидные клиенты, то удаляем их из контейнера соединений
				{
					m_deqConnetion.erase(std::remove(m_deqConnetion.begin(), m_deqConnetion.end(), nullptr), m_deqConnetion.end());
				}
			}

			void Update(size_t nMaxMessages = -1) // так как size_t беззнаковый тип, то на самом деле тут будет самое большее его число 
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
			std::deque<std::shared_ptr<Connection<T>>> m_deqConnetion; //указатели на соединения

			asio::ip::tcp::acceptor m_acceptor; // специальный объект для хранения сокетов клиентов
			uint32_t ipCounter = 10000; // уникальные идентификаторы клиентов
		};
	}
}

